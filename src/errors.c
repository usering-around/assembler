#include <stdarg.h>
#include <stdio.h>
#include "errors.h"
#include "utils.h" /* int types */

/* a function similar to sprintf which writes nice formatted output to a buf, given LineInfo.
   Note: err_fmt and the variadic arguements work just like sprintf. See sprintf documentation for more info on how to use it. */
void process_error(char *buf, LineInfo line_info, char *err_fmt, ...)
{
    va_list args;
    int chars_written;

    va_start(args, err_fmt);
    chars_written = sprintf(buf, "%serror in line %s%d:\n%sline: %s%s\n%sinfo:%s ", ANSI_RED, ANSI_YELLOW, line_info.line_num,
                            ANSI_CYAN, ANSI_YELLOW, trim_end(line_info.line), ANSI_CYAN, ANSI_RED);
    buf += chars_written;
    chars_written = vsprintf(buf, err_fmt, args);
    buf += chars_written;
    sprintf(buf, ANSI_NORMAL);
}

/* Convert an expand macro error to a string, given a buffer and LineInfo */
void macro_error_to_string(ExpandMacroError *error, char *buf, LineInfo line_info)
{
    switch (error->type)
    {
    case EXPAND_MACRO_ERROR_LINE_TOO_LONG:
    {
        process_error(buf, line_info, "line is too big! expected %d characters, got %d", error->val.is_too_long.expected_len, error->val.is_too_long.len);
        break;
    }

    case EXPAND_MACRO_EXPECTED_MACRO_NAME:
    {
        process_error(buf, line_info, "Expected a macro name after macro declaration");
        break;
    }

    case EXPAND_MACRO_ERROR_STARTS_WITH_INVALID_CHARACTER:
    {
        process_error(buf, line_info, "macro name starts with an invalid character '%c'. Expected character to be alphabethic or '_'",
                      error->val.starts_with_invalid_character);
        break;
    }

    case EXPAND_MACRO_ERROR_IS_AN_INSTRUCTION:
    {
        process_error(buf, line_info, "macro name cannot be an instruction");
        break;
    }

    case EXPAND_MACRO_ERROR_IS_A_DIRECTIVE:
    {
        process_error(buf, line_info, "macro name cannot be a directive");
        break;
    }

    case EXPAND_MACRO_ERROR_IS_A_REGISTER:
    {
        process_error(buf, line_info, "macro name cannot be a register. Note: symbols r0,r1,...,r%d are reserved for registers", REGISTER_COUNT);
        break;
    }

    case EXPAND_MACRO_ERROR_INVALID_CHARACTER:
    {
        process_error(buf, line_info, "macro name has invalid character '%c' in position %d", error->val.invalid_character.invalid_character,
                      error->val.invalid_character.position);
        break;
    }

    case EXPAND_MACRO_ERROR_NAME_IS_TOO_LONG:
    {
        process_error(buf, line_info, "macro name is too long; expected %d characters, got %d", error->val.is_too_long.expected_len, error->val.is_too_long.len);
        break;
    }

    case EXPAND_MACRO_ERROR_MACRO_DEFINED_AS_LABEL:
    {
        process_error(buf, line_info, "\"%s\" is a macro; its name should not be used for a label", error->val.macro_name);
        break;
    }
    }
}

/* Convert an parse symbol error to a string, given a buffer and LineInfo */
void parse_symbol_error_to_string(ParseSymbolError *error, char *buf, LineInfo line_info)
{
    switch (error->type)
    {
    case INVALID_CHARACTER_IN_SYMBOL:
    {
        process_error(buf, line_info, "symbol \"%s\" has invalid character '%c' at position %d. Symbols may only contain numeric and alphabethic characters",
                      error->val.invalid_char_in_symbol.symbol,
                      error->val.invalid_char_in_symbol.invalid_char, error->val.invalid_char_in_symbol.position);
        break;
    }

    case SYMBOL_STARTS_WITH_NON_ALPHABETHIC_CHARACTER:
    {
        process_error(buf, line_info, "symbol \"%s\" starts with non-alphabethic character '%c'", error->val.symbol_starts_with_non_alphabethic_char.symbol,
                      error->val.symbol_starts_with_non_alphabethic_char.non_alphabethic_char);
        break;
    }

    case BUFFER_TOO_SMALL:
    {
        process_error(buf, line_info, "symbol is too big, expected %d characters but got %d", MAX_LABEL_SIZE, error->val.symbol_length);
        break;
    }

    case SYMBOL_EMPTY:
    {
        process_error(buf, line_info, "expected a symbol");
        break;
    }

    case SYMBOL_IS_A_DIRECTIVE:
    {
        process_error(buf, line_info, "symbol \"%s\" has the same name as a directive", error->val.symbol);
        break;
    }

    case SYMBOL_IS_AN_INSTRUCTION:
    {
        process_error(buf, line_info, "symbol \"%s\" has the same name as an instruction", error->val.symbol);
        break;
    }

    case SYMBOL_IS_A_REGISTER:
    {
        process_error(buf, line_info, "symbol \"%s\" has the same name as a register. Note: symbols r0,r1,...,r%d are reserved for registers",
                      error->val.symbol, REGISTER_COUNT - 1);
        break;
    }
    }
}

/* Convert an parse error to a string, given a buffer and LineInfo */
void parse_error_to_string(ParseError *error, char *buf, LineInfo line_info)
{
    char *ptr, *temp_str;
    uint32 i;
    switch (error->type)
    {

    case PARSE_ERROR_EXPECTED_INSTRUCTION_OR_DIRECTIVE_AFTER_LABEL:
    {
        process_error(buf, line_info, "expected an instruction or a directive after label");
        break;
    }

    case PARSE_ERROR_EXPECTED_A_SPACE_AFTER_LABEL:
    {
        process_error(buf, line_info, "expected a space after label");
        break;
    }
    case PARSE_ERROR_INVALID_DIRECTIVE:
    {
        process_error(buf, line_info, "invalid directive \"%s\", expected one of \".data\", \".string\", \".entry\", \".extern\"", error->val.invalid_directive);
        break;
    }

    case PARSE_ERROR_DATA_DIRECTIVE_EMPTY_DATA:
    {
        process_error(buf, line_info, "expected a list of integers (e.g. 1, 2, 3) after .data directive");
        break;
    }

    case PARSE_ERROR_DATA_DIRECTIVE_NOT_AN_INTEGER:
    {
        process_error(buf, line_info, "expected an 21 bit signed integer");
        break;
    }

    case PARSE_ERROR_DATA_DIRECTIVE_INVALID_CHARACTER_AFTER_INTEGER:
    {
        process_error(buf, line_info, "invalid character '%c' after integer", error->val.invalid_character);
        break;
    }

    case PARSE_ERROR_DATA_DIRECTIVE_COMMA_AFTER_LAST_INTEGER:
    {
        process_error(buf, line_info, "comma is not allowed after the final integer");
        break;
    }

    case PARSE_ERROR_DATA_DIRECTIVE_INTEGER_BIGGER_THAN_LIMIT:
    {
        if (error->val.overflown_integer == 0)
        {
            process_error(buf, line_info, "one of the given integers is too big for a 21 bit signed integer (max is %d)", MAX_INTEGER);
        }
        else
        {
            process_error(buf, line_info, "integer %d is too big for a 21 bit signed integer (max is %d)", error->val.overflown_integer, MAX_INTEGER);
        }
        break;
    }

    case PARSE_ERROR_DATA_DIRECTIVE_INTEGER_SMALLER_THAN_LIMIT:
    {
        if (error->val.overflown_integer == 0)
        {
            process_error(buf, line_info, "one of the given integers is too small for a 21 bit signed integer (min is %d)", MIN_INTEGER);
        }
        else
        {
            process_error(buf, line_info, "integer %d is too small for a 21 bit signed integer (min is %d)", error->val.overflown_integer, MIN_INTEGER);
        }
        break;
    }

    case PARSE_ERROR_STRING_DIRECTIVE_DOES_NOT_START_WITH_QUOTE:
    {
        process_error(buf, line_info, "string should start with a \"");
        break;
    }

    case PARSE_ERROR_STRING_DIRECTIVE_DOES_NOT_END_WITH_QUOTE:
    {
        process_error(buf, line_info, "string should end with a \"");
        break;
    }

    case PARSE_ERROR_INVALID_INSTRUCTION:
    {
        process_error(buf, line_info, "invalid instruction \"%s\"", error->val.invalid_instruction);
        break;
    }

    case PARSE_ERROR_OPERAND_NO_INTEGER_AFTER_HASHTAG:
    {
        process_error(buf, line_info, "expected an integer after #");
        break;
    }

    case PARSE_ERROR_OPERAND_IMMEDIATE_INTEGER_TOO_BIG:
    {
        if (error->val.overflown_integer == 0)
        {
            process_error(buf, line_info, "immediate integer is too big, max is %d", MAX_INTEGER);
        }
        else
        {
            process_error(buf, line_info, "immediate integer is too big: got %d, max is %d", error->val.overflown_integer, MAX_INTEGER);
        }
        break;
    }

    case PARSE_ERROR_OPERAND_IMMEDIATE_INTEGER_TOO_SMALL:
    {
        if (error->val.overflown_integer == 0)
        {
            process_error(buf, line_info, "immediate integer is too small, min is %d", MIN_INTEGER);
        }
        else
        {
            process_error(buf, line_info, "immediate integer is too small: got %d, min is %d", error->val.overflown_integer, MIN_INTEGER);
        }
        break;
    }

    case PARSE_ERROR_OPERAND_INVALID_CHARACTER_AFTER_OPERAND:
    {
        process_error(buf, line_info, "invalid character '%c' after operand", error->val.invalid_character);
        break;
    }

    case PARSE_ERROR_INSTRUCTION_TOO_MANY_OPERANDS:
    {
        process_error(buf, line_info, "instruction got too many operands; expected %d operands", error->val.expected_amount_of_operands);
        break;
    }

    case PARSE_ERROR_INSTRUCTION_TOO_LITTLE_OPERANDS:
    {
        process_error(buf, line_info, "instruction got too few operands; expected %d operands", error->val.expected_amount_of_operands);
        break;
    }

    case PARSE_ERROR_INSTRUCTION_COMMA_AFTER_FINAL_OPERAND:
    {
        process_error(buf, line_info, "cannot have a ',' after the final operand");
        break;
    }

    case PARSE_ERROR_INSTRUCTION_EXPECTED_A_DIFFERENT_OPERAND_TYPE:
    {
        /* We could prevent this allocation by using the fact that there are only 4 type of operands and creating 8 variables,
           4 for the strings of the operand type and 4 for ',' in between them and then do something like
           process_error(buf, line_info, "%s%s%s%s%s%s%s%s", first_str, first_comma, second_str, second_comma, ...)
           and setting first_str, first_comma, second_str, second_comma, ...
           accordingly by going through the expected operand array, but that would be really messy so I chose this method instead*/
        temp_str = malloc(error->val.expected_operands_type.len * MAX_OPERAND_TYPE_STR_LENGTH + 6);
        ptr = temp_str;
        ptr[0] = 0;
        for (i = 0; i < error->val.expected_operands_type.len - 1; ++i)
        {
            ptr += sprintf(ptr, "%s, ", operand_type_name(error->val.expected_operands_type.acceptable_operands[i]));
        }
        sprintf(ptr, "%s", operand_type_name(error->val.expected_operands_type.acceptable_operands[error->val.expected_operands_type.len - 1]));
        process_error(buf, line_info, "operand %d is of unexpected type for this instruction; its type is %s, expected one of: %s",
                      error->val.expected_operands_type.op_index,
                      operand_type_name(error->val.expected_operands_type.bad_op_type), temp_str);
        free(temp_str);
        break;
    }

    case PARSE_ERROR_ENTRY_DIRECTIVE_GOT_NO_SYMBOL:
    {
        process_error(buf, line_info, "expected a symbol after .entry directive");
        break;
    }
    case PARSE_ERROR_EXTERN_DIRECTIVE_GOT_NO_SYMBOL:
    {
        process_error(buf, line_info, "expected a symbol after .extern directive");
        break;
    }
    case PARSE_ERROR_OPERAND_INVALID_SYMBOL:
    case PARSE_ERROR_ENTRY_DIRECTIVE_GOT_INVALID_SYMBOL:
    case PARSE_ERROR_EXTERN_DIRECTIVE_GOT_INVALID_SYMBOL:
    {
        parse_symbol_error_to_string(&error->val.invalid_symbol, buf, line_info);
        break;
    }

    case PARSE_ERROR_INSTRUCTION_FIRST_OPERAND_EMPTY:
    {
        process_error(buf, line_info, "first operand is empty");
        break;
    }
    }
}

void error_to_string(Error error, char *buf)
{
    buf[0] = 0;
    switch (error.type)
    {
    case ERROR_TYPE_MACRO:
    {
        macro_error_to_string(error.val.expand_macro_err, buf, error.line_info);
        break;
    }

    case ERROR_TYPE_SYMBOL_PARSE:
    {
        parse_symbol_error_to_string(error.val.symbol_parse_err, buf, error.line_info);
        break;
    }

    case ERROR_TYPE_PARSE:
    {
        parse_error_to_string(error.val.parse_err, buf, error.line_info);
        break;
    }

    case ERROR_TYPE_SYMBOL_ALREADY_DEFINED:
    {
        process_error(buf, error.line_info, "symbol \"%s\" has already been defined in line %d", error.val.symbol->name, error.val.symbol->line);
        break;
    }

    case ERROR_TYPE_MEMORY_OVERFLOWN:
    {
        process_error(buf, error.line_info, "Memory has overflown; max address is %d but the file fills up to address %d. The line shown here is the first line in which memory has overflown", error.val.memory_overflown.expected_max_address,
                      error.val.memory_overflown.max_address);
        break;
    }

    case ERROR_TYPE_SYMBOL_NOT_DEFINED:
    {
        process_error(buf, error.line_info, "symbol \"%s\" is not defined anywhere in this file.", error.val.symbol_name);
        break;
    }

    case ERROR_TYPE_EXTERNAL_SYMBOL_USED_IN_ENTRY_DIRECTIVE:
    {
        process_error(buf, error.line_info, "symbol \"%s\" was defined as external in line %d; external symbols may not be used in a .entry directive",
                      error.val.symbol->name, error.val.symbol->line);
        break;
    }
    }
}

void err(ErrorCallback err_callback, Error err)
{
    err_callback.callback(err, err_callback.data);
}
