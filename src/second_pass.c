
#include "second_pass.h"
#include "parser.h"
#include "utils.h" /* int types */
/* The bit at which the operand's data starts in the extra information word. (The first 3 bits are used for the 'A,R,E' field) */
#define OPERAND_WORD_START_BIT 3

/* Encodes an operand's information word if necessary. Returns 0 if the operand does not need an information word (i.e. if it is a register) */
uint32 encode_operand(Operand *operand, SymbolTable *symbol_table, uint32 current_instruction_addr)
{
    uint32 encoding = 0;
    int offset;
    Symbol *symbol;
    if (operand->type == OPERAND_IMMEDIATE)
    {
        encoding |= (operand->value.immediate << OPERAND_WORD_START_BIT);
        encoding |= 0x4; /* 'A' in the A,R,E, field */
    }
    else if (operand->type == OPERAND_SYMBOL)
    {
        symbol = symbol_table_search(*symbol_table, operand->value.symbol_name);

        encoding |= (symbol->addr << OPERAND_WORD_START_BIT);
        if (symbol->context == SYMBOL_CONTEXT_EXTERNAL)
        {
            encoding |= 0x1; /* 'E' in the A,R,E field*/
        }
        else
        {
            encoding |= 0x2; /* 'R' in the A,R,E field*/
        }
    }
    else if (operand->type == OPERAND_ADDRESS)
    {
        symbol = symbol_table_search(*symbol_table, operand->value.symbol_name);

        /* calculate the offset between the address of the symbol and the current instruction */
        offset = symbol->addr - current_instruction_addr;

        encoding |= (offset << OPERAND_WORD_START_BIT);
        encoding |= 0x4; /* 'A' in the A,R,E field*/
    }

    return encoding;
}

/* Write an instruction onto instruction_image. Returns the amount of words written to the instruction image. */
uint32 write_instruction(Instruction *instruction, U32Vector *instruction_image, SymbolTable *symbol_table, uint32 IC, bool *alloc_fail)
{
    uint32 words_written = 0; /* amount of words we wrote to the instruction_image vector */
    uint32 encoding = encode_instruction(instruction);

    *alloc_fail = FALSE;
    /* write the instruction to the instruction image */
    if ((*alloc_fail = !u32_vec_push(instruction_image, encoding)))
    {
        return 0;
    }
    words_written += 1;
    if (instruction->operand_amount >= 1)
    {
        /* encode the operand and write it to the vector if it is necessary */

        encoding = encode_operand(&instruction->operand1, symbol_table, IC);
        if (encoding != 0)
        {
            if ((*alloc_fail = !u32_vec_push(instruction_image, encoding)))
            {
                return 0;
            }
            words_written += 1;
        }
    }
    if (instruction->operand_amount >= 2)
    {
        encoding = encode_operand(&instruction->operand2, symbol_table, IC);
        if (encoding != 0)
        {
            if ((*alloc_fail = !u32_vec_push(instruction_image, encoding)))
            {
                return 0;
            }
            words_written += 1;
        }
    }
    return words_written;
}

/* Algorithm:
   for each line in the file we do the following:
    if the line is empty, has a comment or has a syntax error we skip it (it is not our job to handle syntax errors)

    if the line has a .entry directive, we check the symbol which is after the entry directive. If it is in the symbol table, we put it in the entry symbols array.
    If it is not in the symbol table, we call err_callback with the appropriate error.
    We ignore any other kind of directives if found.

    If the line has an instruction, we check the operands of the instruction. If any operand is an external symbol,
    we update their address to the appropriate address in the instruction image and put them inside the external symbols array.
    After putting the external symbols (if there are any), we encode and write the instruction to the instruction image,
    as well as raise IC by the amount of words the instruction took.

  once we're done reading the file, we return the symbol table, the data image, the instruction image, the entry symbols array and external symbols array.
*/
SecondPassResult second_pass(FILE *input, FirstPassResult first_pass_result, ErrorCallback err_callback)
{
    char buf[MAX_LINE_LENGTH + 2];                        /* instruction buffer; +2 for null termination and newline character */
    Error error;                                          /* error we call err_callback with */
    uint32 IC = 100;                                      /* Instruction count */
    U32Vector *instruction_image = u32_vec_create();      /* The instruction image we return*/
    SymbolVector *entry_symbols = symbol_vec_create();    /* the entry symbols vector we return */
    SymbolVector *external_symbols = symbol_vec_create(); /* the extern symbols vector we return */
    SecondPassResult second_pass_result;                  /* the second pass result we return */
    bool instruction_has_invalid_operand;                 /* whether or not an instruction we're encoding has an invalid operand*/
    bool alloc_fail = FALSE;                              /* whether or not we encounterd an allocation failure */
    ParseLineData parse_line_data;
    LineInfo line_info;
    Symbol *symbol, symbol_copy;
    Instruction *instruction;
    uint32 i;

    /* initialize second_pass_result*/
    second_pass_result.symbol_table = first_pass_result.symbol_table;
    second_pass_result.data_image = first_pass_result.data_image;
    second_pass_result.instruction_image = instruction_image;
    second_pass_result.entry_symbols = entry_symbols;
    second_pass_result.external_symbols = external_symbols;
    second_pass_result.encountered_error = FALSE;
    second_pass_result.alloc_fail = FALSE;

    /* we check after initializing second_pass_result so that if the caller decides to free second_pass_result it won't try to free garbage*/
    if (instruction_image == NULL || entry_symbols == NULL || external_symbols == NULL)
    {
        second_pass_result.alloc_fail = TRUE;
        second_pass_result.encountered_error = TRUE;
        return second_pass_result;
    }

    /* initialize line_info */
    line_info.line = buf;
    line_info.line_num = 0;

    while (fgets(buf, sizeof(buf), input))
    {
        line_info.line_num++;
        error.line_info = line_info; /* update error's line info */
        parse_line(buf, &parse_line_data);
        if (parse_line_data.type == PARSE_LINE_COMMENT || parse_line_data.type == PARSE_LINE_COMMENT || parse_line_data.type == PARSE_LINE_ERROR)
        {
            /* not our job to handle these */
            continue;
        }
        else if (parse_line_data.type == PARSE_LINE_DIRECTIVE)
        {
            /* we only need to handle entry directives by ensuring their symbol is defined inside the file
               and if so, by pushing them onto the entry symbol vector */
            if (parse_line_data.val.directive.type == DIRECTIVE_ENTRY)
            {
                symbol = symbol_table_search(first_pass_result.symbol_table, parse_line_data.val.directive.val.entry_symbol);
                if (symbol == NULL)
                {
                    error.type = ERROR_TYPE_SYMBOL_NOT_DEFINED;
                    error.val.symbol_name = parse_line_data.val.directive.val.entry_symbol;
                    err(err_callback, error);
                    second_pass_result.encountered_error = TRUE;
                }
                else if (symbol->context == SYMBOL_CONTEXT_EXTERNAL)
                {
                    /* we cannot have a .entry to an external symbol */
                    error.type = ERROR_TYPE_EXTERNAL_SYMBOL_USED_IN_ENTRY_DIRECTIVE;
                    error.val.symbol = symbol;
                    err(err_callback, error);
                    second_pass_result.encountered_error = TRUE;
                }
                else
                {
                    /* ensure that we haven't already insereted the entry symbol
                    (since using .entry twice is allowed, but we only need to write it once to the entry file)*/
                    for (i = 0; i < entry_symbols->len; ++i)
                    {
                        if (symbol_vec_get_ptr(entry_symbols, i)->name == symbol->name)
                        {
                            continue;
                        }
                    }
                    if (!symbol_vec_push(entry_symbols, *symbol))
                    {
                        second_pass_result.alloc_fail = TRUE;
                        second_pass_result.encountered_error = TRUE;
                        return second_pass_result;
                    }
                }
            }
        }
        else if (parse_line_data.type == PARSE_LINE_INSTRUCTION)
        {
            instruction = &parse_line_data.val.instruction;
            instruction_has_invalid_operand = FALSE;

            /* Check operands for any external symbols.
               If there are any, we change their address to their fitting place in the instruction image and push them in the external symbols vector. */
            if (instruction->operand_amount >= 1 && (instruction->operand1.type == OPERAND_SYMBOL || instruction->operand1.type == OPERAND_ADDRESS))
            {

                symbol = symbol_table_search(first_pass_result.symbol_table, instruction->operand1.value.symbol_name);
                if (symbol == NULL)
                {
                    /* error - undefined symbol */
                    error.type = ERROR_TYPE_SYMBOL_NOT_DEFINED;
                    error.val.symbol_name = instruction->operand1.value.symbol_name;
                    err(err_callback, error);
                    instruction_has_invalid_operand = TRUE;
                }
                else if (symbol->context == SYMBOL_CONTEXT_EXTERNAL)
                {
                    symbol_copy = *symbol;
                    /* update the address of the symbol to its address in the instruction image */
                    symbol_copy.addr = IC + 1;
                    if (!symbol_vec_push(external_symbols, symbol_copy))
                    {
                        second_pass_result.alloc_fail = TRUE;
                        second_pass_result.encountered_error = TRUE;
                        return second_pass_result;
                    }
                }
            }
            if (instruction->operand_amount >= 2 && (instruction->operand2.type == OPERAND_SYMBOL || instruction->operand2.type == OPERAND_ADDRESS))
            {
                symbol = symbol_table_search(first_pass_result.symbol_table, instruction->operand2.value.symbol_name);
                if (symbol == NULL)
                {
                    /* error - undefined symbol */
                    error.type = ERROR_TYPE_SYMBOL_NOT_DEFINED;
                    error.val.symbol_name = instruction->operand2.value.symbol_name;
                    err(err_callback, error);
                    instruction_has_invalid_operand = TRUE;
                }
                else if (symbol->context == SYMBOL_CONTEXT_EXTERNAL)
                {
                    symbol_copy = *symbol;
                    /* update the address of the symbol to its address in the instruction image */
                    symbol_copy.addr = IC + 2;
                    if (!symbol_vec_push(external_symbols, symbol_copy))
                    {
                        second_pass_result.alloc_fail = TRUE;
                        second_pass_result.encountered_error = TRUE;
                        return second_pass_result;
                    }
                }
            }
            if (instruction_has_invalid_operand)
            {
                /* we already reported the error */
                second_pass_result.encountered_error = TRUE;
            }
            else
            {
                /* write the instruction to the instruction_image vector and raise IC by the amount of words we wrote*/
                IC += write_instruction(&parse_line_data.val.instruction, instruction_image, &first_pass_result.symbol_table, IC, &alloc_fail);
                if (alloc_fail)
                {
                    second_pass_result.alloc_fail = TRUE;
                    second_pass_result.encountered_error = TRUE;
                    return second_pass_result;
                }
            }
        }
    }

    return second_pass_result;
}

void free_second_pass_result(SecondPassResult second_pass_result)
{
    u32_vec_free(second_pass_result.data_image);
    u32_vec_free(second_pass_result.instruction_image);
    symbol_vec_free(second_pass_result.entry_symbols);
    symbol_vec_free(second_pass_result.external_symbols);
    symbol_table_free(second_pass_result.symbol_table);
}