#include <string.h>
#include "first_pass.h"
#include "parser.h"

/* Takes a pointer to a Directive and a U32Vector and updates data_vec in accordance with the directive.
   For .data directive, it will push in data_vec each number.
   For .string directive, it will push each character into data_vec.
   This function does not handle any other kind of directive.
   The function returns the amount of data written to data_vec. */
uint32 handle_data_and_string_directive(Directive *directive, U32Vector *data_vec, bool *alloc_fail)
{
    uint32 i;
    int integer;
    char *str;
    uint32 amount_of_data = 0;
    if (directive->type == DIRECTIVE_DATA)
    {
        for (i = 0; i < directive->val.data.amount_of_integers; ++i)
        {
            integer = directive->val.data.integers[i];
            if ((*alloc_fail = !u32_vec_push(data_vec, integer)))
            {
                return 0;
            }
            amount_of_data += 1;
        }
    }
    else if (directive->type == DIRECTIVE_STRING)
    {
        str = directive->val.string;
        while (*str != 0)
        {
            if ((*alloc_fail = !u32_vec_push(data_vec, *str)))
            {
                return 0;
            }
            str++;
            amount_of_data += 1;
        }
        if ((*alloc_fail = !u32_vec_push(data_vec, 0)))
        {
            return 0;
        }
        amount_of_data += 1;
    }
    return amount_of_data;
}

/* Algorithm:
   We start IC to 100 and DC to 0, create a symbol table and an array which represents the data image.
   For each line in the file we do the following:
    If the line has a valid label, insert it to the symbol table with the address being DC if the line has a directive or IC if the line has an instruction.
    If the label has been defined before already, we call err_callback with the appropriate error.
    If it does not have a valid label, call err_callback with the appropriate error.
    If there is no label, we simply continue to the next step.

    Afterwards we check the rest of the line after the label.
    If there is an error there, we report it.

    Otherwise If we find a directive, we do different things depending on the directive.
    .entry directive: we do nothing since it is not handled by the first pass.
    .extern directive: we insert the symbol into the symbol table as long as it hasn't been defined before.
                        if it has been defined before, we call err_callback with the appropriate error.
    .data directive: we push each integer into the data image, raising DC by 1 for each integer in the process.
    .string directive: we push each character of the string into the data image, raising DC by 1 for each integer in the process.

    Otherwise there is an instruction.
    If the instruction is invalid, we call err_callback with the appropriate error.
    If the instruction is valid, we raise IC by the amount of words necessary to encode the instruction.

    Once we read the entire file, we return the symbol table, the data image and a flag representing whether or not we encountered any errors.
*/
FirstPassResult first_pass(FILE *input, ErrorCallback err_callback)
{
    uint32 IC = INSTRUCTION_MEMORY_START, DC = 0;  /* instruction counter, data counter */
    char instruction_buf[MAX_LINE_LENGTH + 2];     /* +2 for newline + null termination */
    char instruction_dup[sizeof(instruction_buf)]; /* duplicate instruction buffer for use in line_info */
    LineInfo line_info;                            /* information about the line which is passed to error */
    FirstPassResult first_pass_result;             /* the result we return  */
    U32Vector *data_vec = u32_vec_create();        /* the data image */
    Error error;                                   /* error used for err_callback */
    bool should_skip_table_insertion;              /* whether or not we should not skip inserting a label into a the table*/
    bool alloc_fail = FALSE;                       /* whether or not we failed a emory allocation */
    bool memory_overflown = FALSE;                 /* whether or not we have overflowed the address space */
    LineInfo mem_overflow_line_info;               /* line information about a line which caused a memory overflow*/
    Symbol *symbol;
    uint32 addr;
    SymbolContext symbol_ctx;
    char *label;
    ParseLineData parse_line_data;
    SymbolTableIterator symbol_table_iterator;

    /* initialize first_pass_result */
    first_pass_result.encountered_error = FALSE;
    first_pass_result.alloc_fail = FALSE;
    first_pass_result.data_image = data_vec;
    if (!symbol_table_init(&first_pass_result.symbol_table) || data_vec == NULL)
    {
        first_pass_result.alloc_fail = TRUE;
        first_pass_result.encountered_error = TRUE;
        return first_pass_result;
    }

    /* initialize line_info */
    line_info.line_num = 0;
    line_info.line = instruction_dup;

    while (fgets(instruction_buf, sizeof(instruction_buf), input))
    {
        line_info.line_num++;
        error.line_info = line_info; /* update error's line info */
        memcpy(line_info.line, instruction_buf, sizeof(instruction_buf));

        parse_line(instruction_buf, &parse_line_data);
        if (parse_line_data.type == PARSE_LINE_COMMENT || parse_line_data.type == PARSE_LINE_EMPTY)
        {
            /* we don't care about empty lines/comments*/
            continue;
        }

        if (parse_line_data.parse_label_data.result == HAS_SYMBOL)
        {
            /* insert label to table if there are no errors in the parsing */
            label = parse_line_data.parse_label_data.val.symbol_buffer;
            should_skip_table_insertion = FALSE;
            if (parse_line_data.type != PARSE_LINE_ERROR)
            {
                if (parse_line_data.type == PARSE_LINE_DIRECTIVE)
                {
                    if (parse_line_data.val.directive.type == DIRECTIVE_DATA || parse_line_data.val.directive.type == DIRECTIVE_STRING)
                    {
                        symbol_ctx = SYMBOL_CONTEXT_DATA;
                        addr = DC;
                    }
                    else
                    {
                        /* we ignore .entry and .extern directives */
                        should_skip_table_insertion = TRUE;
                    }
                }
                else
                {
                    symbol_ctx = SYMBOL_CONTEXT_CODE;
                    addr = IC;
                }

                if (!should_skip_table_insertion)
                {
                    if ((symbol = symbol_table_search(first_pass_result.symbol_table, label)) != NULL)
                    {
                        error.type = ERROR_TYPE_SYMBOL_ALREADY_DEFINED;
                        error.val.symbol = symbol;
                        err(err_callback, error);
                        first_pass_result.encountered_error = TRUE;
                    }
                    else
                    {
                        if (!symbol_table_insert(first_pass_result.symbol_table, label, addr, symbol_ctx, line_info.line_num))
                        {
                            first_pass_result.alloc_fail = TRUE;
                            first_pass_result.encountered_error = TRUE;
                            return first_pass_result;
                        }
                    }
                }
            }
        }
        else if (parse_line_data.parse_label_data.result == SYMBOL_PARSE_ERROR)
        {
            /* we have an error with the label */
            error.type = ERROR_TYPE_SYMBOL_PARSE;
            error.val.symbol_parse_err = &parse_line_data.parse_label_data.val.symbol_parse_error;
            err(err_callback, error);
            first_pass_result.encountered_error = TRUE;
        }

        if (parse_line_data.type == PARSE_LINE_ERROR)
        {
            error.type = ERROR_TYPE_PARSE;
            error.val.parse_err = &parse_line_data.val.parse_error;
            err(err_callback, error);
            first_pass_result.encountered_error = TRUE;
        }
        else if (parse_line_data.type == PARSE_LINE_DIRECTIVE)
        {

            if (parse_line_data.val.directive.type == DIRECTIVE_EXTERN)
            {
                /* insert extern directive symbol into the symbol table is if it not already defined */
                if ((symbol = symbol_table_search(first_pass_result.symbol_table, parse_line_data.val.directive.val.extern_symbol)) != NULL)
                {
                    error.type = ERROR_TYPE_SYMBOL_ALREADY_DEFINED;
                    error.val.symbol = symbol;
                    err(err_callback, error);
                    first_pass_result.encountered_error = TRUE;
                }
                else
                {
                    if (!symbol_table_insert(first_pass_result.symbol_table, parse_line_data.val.directive.val.extern_symbol, 0, SYMBOL_CONTEXT_EXTERNAL, line_info.line_num))
                    {
                        first_pass_result.alloc_fail = TRUE;
                        first_pass_result.encountered_error = TRUE;
                        return first_pass_result;
                    }
                }
            }
            else
            {
                /* insert the directive data into the data image and increase DC appropriately */
                DC += handle_data_and_string_directive(&parse_line_data.val.directive, data_vec, &alloc_fail);
                if (alloc_fail)
                {
                    first_pass_result.alloc_fail = TRUE;
                    first_pass_result.encountered_error = TRUE;
                    return first_pass_result;
                }
            }
            /* .entry directive are handled by the second pass */
        }
        else if (parse_line_data.type == PARSE_LINE_INSTRUCTION)
        {
            /* increase IC by the amount of words an instruction takes */
            IC += instruction_encoding_word_count(&parse_line_data.val.instruction);
        }
        if (!memory_overflown && (IC + data_vec->len > MAX_ADDRESS))
        {
            /* We've overflown, save info for later so that we can report it after reporting any other error found in the file */
            memory_overflown = TRUE;
            mem_overflow_line_info.line_num = line_info.line_num;
            mem_overflow_line_info.line = strdup(line_info.line);
        }
    }

    /* report memory overflown */
    if (memory_overflown)
    {
        error.type = ERROR_TYPE_MEMORY_OVERFLOWN;
        error.line_info = mem_overflow_line_info;
        error.val.memory_overflown.expected_max_address = MAX_ADDRESS;
        error.val.memory_overflown.max_address = IC + data_vec->len;
        err(err_callback, error);
        first_pass_result.encountered_error = TRUE;
        /* free the duplicated string */
        free(mem_overflow_line_info.line);
    }

    /* add IC to every data symbol  */
    symbol_table_iterator = symbol_table_iter(first_pass_result.symbol_table);
    for (symbol = symbol_table_iter_next(&symbol_table_iterator); symbol != NULL; symbol = symbol_table_iter_next(&symbol_table_iterator))
    {
        if (symbol->context == SYMBOL_CONTEXT_DATA)
        {
            symbol->addr += IC;
        }
    }

    return first_pass_result;
}

void free_first_pass_result(FirstPassResult first_pass_result)
{
    u32_vec_free(first_pass_result.data_image);
    symbol_table_free(first_pass_result.symbol_table);
}
