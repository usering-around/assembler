#include <ctype.h>
#include <string.h>
#include "parser.h"

/* Trim any characters found after the first space. Returns the string back. */
char *trim_after_space(char *str)
{
    char *str_start = str;
    while (!isspace((unsigned char)*str) && *str != 0)
    {
        str++;
    }
    *str = 0;
    return str_start;
}

/* turn a character which represents a digit into an integer */
#define CHAR_DIGIT_TO_INT(c) ((int)((c) - '0'))

/* maximum digits a 32 bits number has in base 10 */
#define BITS_32_INT_MAX_DIGITS 10

/* Parse a single 32 bits signed integer from str, assuming it is represented in base 10.
   Returns the integer in the out parameter integer, along with the amount of characters the integer occupies in the string, chars_read,
   and whether or not the integer is negative.
   Returns TRUE if the parse was successful, FALSE otherwise. A parse may fail if there are no digits at all, or if the integer overflows a signed 32 bit integer.
   In the case of a failure, the out parameter integer will contain garbage and the is_negative parameter will work as expected.
   In the former case of failure, chars_read will be 0, and in the latter it will be the
   amount of characters the integer occupies. */
bool parse_int32_base10(const char *str, int32 *integer, int *chars_read, bool *is_negative)
{
    uint32 digit_count = 0;
    int bits_32_max_int_digits[] = {2, 1, 4, 7, 4, 8, 3, 6, 4, 7};
    int i;
    int last_digit;
    *integer = 0;
    *chars_read = 0;
    *is_negative = FALSE;
    if (*str == '-')
    {
        *is_negative = TRUE;
        str++;
        *chars_read += 1;
    }

    while (*str >= '0' && *str <= '9' && *str != 0)
    {
        *integer *= 10;
        *integer += CHAR_DIGIT_TO_INT(*str);
        digit_count++;
        str++;
    }
    *chars_read += digit_count;

    if (digit_count > BITS_32_INT_MAX_DIGITS)
    {
        return FALSE;
    }
    else if (digit_count == 0)
    {
        *chars_read = 0;
        return FALSE;
    }
    else if (digit_count == BITS_32_INT_MAX_DIGITS)
    {
        /* we need to check that the number does not go over the limit */
        str--; /* to get back to the integer */
        last_digit = CHAR_DIGIT_TO_INT(*str);
        str--; /* skip last digit */
        /* -2 to skip the last digit */
        for (i = BITS_32_INT_MAX_DIGITS - 2; i >= 0; --i)
        {
            if (CHAR_DIGIT_TO_INT(*str) > bits_32_max_int_digits[i])
            {
                return FALSE;
            }
            str--;
        }
        /* check the last digit - negative numbers allow for 1 more in the last digit
           we don't need to ensure the last digit is less than 9. To see this, we note that numbers of the form 2^n never end in 0,
           and thus numbers of the form (2^n) -1 never end in 9. Since numbers which represent the maximum limit of a signed integer are always
           of the latter form, the last digit is always less than  9. */
        if (is_negative)
        {
            if (last_digit > (bits_32_max_int_digits[BITS_32_INT_MAX_DIGITS - 1] + 1))
            {
                return FALSE;
            }
        }
        else
        {
            if (last_digit > bits_32_max_int_digits[BITS_32_INT_MAX_DIGITS - 1])
            {
                return FALSE;
            }
        }
    }

    if (*is_negative)
    {
        *integer *= -1;
    }
    return TRUE;
}

/* returns c == 0 */
bool default_symbol_end_indicator(char c)
{
    return c == 0;
}
/* Parse a symbol from line, given a callback symbol_end_indicator which indicates the end of a symbol.
   parse_symbol_data is an out parameter which will be filled with values corresponding to the result of the parsing.
   Read its documentation for more information. */
void parse_symbol(char *line, bool (*symbol_end_indicator)(char), ParseSymbolData *parse_symbol_data)
{
    bool is_symbol = FALSE; /* whether or not the string we're parsing is indeed a symbol (i.e. it has an end indicator)*/
    char *copy_to;          /* where to copy the symbol to */
    int character_position = 0;
    char c = line[character_position], temp;
    uint32 symbol_len = 0;
    char *line_start = line;

    parse_symbol_data->result = DOES_NOT_HAVE_SYMBOL;

    /* put the default symbol_end_indicator if we were given NULL */
    if (symbol_end_indicator == NULL)
    {
        symbol_end_indicator = default_symbol_end_indicator;
    }

    /* handle the edge case of an empty label */
    if (symbol_end_indicator(*line))
    {
        parse_symbol_data->val.symbol_parse_error.type = SYMBOL_EMPTY;
        parse_symbol_data->result = SYMBOL_PARSE_ERROR;
        parse_symbol_data->symbol_length = 0;
        return;
    }
    if (symbol_end_indicator(0))
    {
        /* since every string has a null terminator in it, we get that line is has a symbol in this case */
        is_symbol = TRUE;
        parse_symbol_data->result = HAS_SYMBOL;
    }
    /* check that the first character is an alphabethic one */
    if (!isalpha(c))
    {
        parse_symbol_data->val.symbol_parse_error.type = SYMBOL_STARTS_WITH_NON_ALPHABETHIC_CHARACTER;
        parse_symbol_data->val.symbol_parse_error.val.symbol_starts_with_non_alphabethic_char.non_alphabethic_char = c;
        parse_symbol_data->result = SYMBOL_PARSE_ERROR;
    }
    character_position++;
    symbol_len += 1;

    while (line[character_position] != 0)
    {
        c = line[character_position];
        if (symbol_end_indicator(c))
        {
            is_symbol = TRUE;
            if (parse_symbol_data->result == DOES_NOT_HAVE_SYMBOL)
            {
                /* if there were no errors then we have a valid symbol*/
                parse_symbol_data->result = HAS_SYMBOL;
            }
            break;
        }
        else if (!isalnum(c))
        {

            parse_symbol_data->val.symbol_parse_error.type = INVALID_CHARACTER_IN_SYMBOL;
            parse_symbol_data->val.symbol_parse_error.val.invalid_char_in_symbol.invalid_char = c;
            parse_symbol_data->val.symbol_parse_error.val.invalid_char_in_symbol.position = character_position;
            parse_symbol_data->result = SYMBOL_PARSE_ERROR;
        }
        character_position++;
        symbol_len++;
    }

    if (!is_symbol)
    {
        parse_symbol_data->result = DOES_NOT_HAVE_SYMBOL;
        parse_symbol_data->symbol_length = 0;
        return;
    }
    parse_symbol_data->symbol_length = symbol_len;

    /* +1 for null termination */
    if (symbol_len + 1 > sizeof(parse_symbol_data->val.symbol_buffer))
    {
        parse_symbol_data->result = SYMBOL_PARSE_ERROR;
        parse_symbol_data->val.symbol_parse_error.type = BUFFER_TOO_SMALL;
        parse_symbol_data->val.symbol_parse_error.val.symbol_length = symbol_len;
        return;
    }

    /* copy the label to the right place and ensure it is does not conflict with a reserved keyword*/
    if (parse_symbol_data->result == HAS_SYMBOL)
    {
        /* temporary measure to ensure that line_start only contains our symbol */
        temp = line_start[symbol_len];
        line_start[symbol_len] = 0;
        if (is_a_directive(line_start))
        {
            parse_symbol_data->result = SYMBOL_PARSE_ERROR;
            parse_symbol_data->val.symbol_parse_error.type = SYMBOL_IS_A_DIRECTIVE;
            copy_to = parse_symbol_data->val.symbol_parse_error.val.symbol;
        }
        else if (is_an_instruction(line_start))
        {
            parse_symbol_data->result = SYMBOL_PARSE_ERROR;
            parse_symbol_data->val.symbol_parse_error.type = SYMBOL_IS_AN_INSTRUCTION;
            copy_to = parse_symbol_data->val.symbol_parse_error.val.symbol;
        }
        else if (is_a_register(line_start))
        {
            parse_symbol_data->result = SYMBOL_PARSE_ERROR;
            parse_symbol_data->val.symbol_parse_error.type = SYMBOL_IS_A_REGISTER;
            copy_to = parse_symbol_data->val.symbol_parse_error.val.symbol;
        }
        else
        {
            copy_to = parse_symbol_data->val.symbol_buffer;
        }
        /* restore the line */
        line_start[symbol_len] = temp;
    }
    else if (parse_symbol_data->val.symbol_parse_error.type == SYMBOL_STARTS_WITH_NON_ALPHABETHIC_CHARACTER)
    {
        copy_to = parse_symbol_data->val.symbol_parse_error.val.symbol_starts_with_non_alphabethic_char.symbol;
    }
    else
    {
        /* otherwise we have an invalid_char_in_symbol error */
        copy_to = parse_symbol_data->val.symbol_parse_error.val.invalid_char_in_symbol.symbol;
    }
    strncpy(copy_to, line_start, symbol_len);
    copy_to[symbol_len] = 0;
    return;
}

/* Parse a .data's directive integer list from an str. Returns TRUE and fills parse_error if we encountered an error,
   otherwise returns FALSE and fills directive with integer list of the .data directive */
bool parse_data_directive(char *str, Directive *directive, ParseError *parse_error)
{
    int chars_read;
    int32 integer;
    bool encountered_error = FALSE, is_negative;

    directive->val.data.amount_of_integers = 0;
    if (*str == 0)
    {
        parse_error->type = PARSE_ERROR_DATA_DIRECTIVE_EMPTY_DATA;
        encountered_error = TRUE;
        return encountered_error;
    }

    /* scan the integer list */
    while (*str != 0)
    {
        encountered_error = !parse_int32_base10(str, &integer, &chars_read, &is_negative);
        if (chars_read == 0)
        {
            parse_error->type = PARSE_ERROR_DATA_DIRECTIVE_NOT_AN_INTEGER;
            return encountered_error;
        }
        /* if encountered_error is TRUE then we have overflown a 32 bit signed integer*/
        else if (integer < MIN_INTEGER || (encountered_error && is_negative))
        {
            parse_error->type = PARSE_ERROR_DATA_DIRECTIVE_INTEGER_SMALLER_THAN_LIMIT;

            if (encountered_error)
            {
                parse_error->val.overflown_integer = 0;
            }
            else
            {
                parse_error->val.overflown_integer = integer;
            }
            encountered_error = TRUE;
            return encountered_error;
        }
        else if (integer > MAX_INTEGER || encountered_error)
        {
            parse_error->type = PARSE_ERROR_DATA_DIRECTIVE_INTEGER_BIGGER_THAN_LIMIT;
            if (encountered_error)
            {
                parse_error->val.overflown_integer = 0;
            }
            else
            {
                parse_error->val.overflown_integer = integer;
            }
            encountered_error = TRUE;
            return encountered_error;
        }
        directive->val.data.integers[directive->val.data.amount_of_integers] = integer;
        directive->val.data.amount_of_integers++;
        str += chars_read;
        str = skip_space(str);
        if (*str != 0 && *str != ',')
        {
            /* error - invalid character after the integer*/
            parse_error->type = PARSE_ERROR_DATA_DIRECTIVE_INVALID_CHARACTER_AFTER_INTEGER;
            parse_error->val.invalid_character = *str;
            encountered_error = TRUE;
            return encountered_error;
        }
        else if (*str == ',')
        {
            str += 1;
            str = skip_space(str);
            /* check if the ',' is after the last number. */
            if (*str == 0)
            {
                /* error - ',' after a number */
                parse_error->type = PARSE_ERROR_DATA_DIRECTIVE_COMMA_AFTER_LAST_INTEGER;
                encountered_error = TRUE;
                return encountered_error;
            }
        }
    }
    return encountered_error;
}

/* Parse the string of a .string directive. Returns TRUE and fills parse_error if there was an error,
   otherwise returns FALSE and fills directive with the string found */
bool parse_string_directive(char *str, Directive *directive, ParseError *parse_error)
{
    char *end;         /* end of the quoted region */
    char *start = str; /* start of the quoted region */
    uint32 i;
    bool encountered_error = FALSE;

    if (*start != '"')
    {
        parse_error->type = PARSE_ERROR_STRING_DIRECTIVE_DOES_NOT_START_WITH_QUOTE;
        encountered_error = TRUE;
        return encountered_error;
    }
    str++;

    /* get to the end of the line */
    while (*str != 0)
    {
        str++;
    }
    /* go back to the last quote */
    while (*str != '"')
    {
        str--;
    }
    end = str;

    if (start == end)
    {
        /* we know that the string starts with a quote - thus it must mean that there is no ending quote */
        parse_error->type = PARSE_ERROR_STRING_DIRECTIVE_DOES_NOT_END_WITH_QUOTE;
        encountered_error = TRUE;
        return encountered_error;
    }

    /* otherwise we have a valid string, we copy everything from the start to end to the directive's string buffer */
    i = 0;
    for (str = start + 1; str != end; ++str)
    {
        directive->val.string[i] = *str;
        ++i;
    }
    directive->val.string[i] = 0;
    return encountered_error;
}

/*  Parses a single directive statement. It assumes that the str starts with the name of the directive without any addition (e.g. a '.' at the start).
    So for example a valid value for str would be "data 1, 2,3", however it would parse ".data 1,2,3" as invalid directive.
    Returns TRUE and fills the parse_error if an error was encounterd during the parsing.
    Otherwise returns FALSE and fills the directive object with data.
    Note: this function modifes the str */
bool parse_directive(char *str, Directive *directive, ParseError *parse_error)
{
    bool encountered_error = FALSE;
    ParseSymbolData parse_symbol_data;
    DirectiveType directive_type;
    char *str_start = str;
    /* ensure the str starts with a valid directive name */
    if (!str_to_directive_type(str, &directive_type))
    {
        parse_error->type = PARSE_ERROR_INVALID_DIRECTIVE;
        parse_error->val.invalid_directive = trim_after_space(str_start);
        encountered_error = TRUE;
        return encountered_error;
    }
    str += directive_name_len(directive_type);

    if (!isspace(*str) && *str != 0)
    {
        /* if there is no space after the directive and we're not at the end of the string - we have an invalid directive */
        parse_error->type = PARSE_ERROR_INVALID_DIRECTIVE;
        parse_error->val.invalid_directive = trim_after_space(str_start);
        encountered_error = TRUE;
        return encountered_error;
    }
    directive->type = directive_type;
    str = skip_space(str);

    /* parse each directive's data */
    switch (directive_type)
    {
    case DIRECTIVE_DATA:
    {
        encountered_error = parse_data_directive(str, directive, parse_error);
        break;
    }

    case DIRECTIVE_STRING:
    {
        encountered_error = parse_string_directive(str, directive, parse_error);
        break;
    }

    case DIRECTIVE_ENTRY:
    {
        /* ensure the symbol is a valid symbol */
        trim_end(str);
        parse_symbol(str, NULL, &parse_symbol_data);
        if (parse_symbol_data.result == HAS_SYMBOL)
        {
            directive->val.entry_symbol = str;
        }
        else if (parse_symbol_data.result == DOES_NOT_HAVE_SYMBOL)
        {
            parse_error->type = PARSE_ERROR_ENTRY_DIRECTIVE_GOT_NO_SYMBOL;
            encountered_error = TRUE;
        }
        else if (parse_symbol_data.result == SYMBOL_PARSE_ERROR)
        {
            if (parse_symbol_data.val.symbol_parse_error.type == SYMBOL_EMPTY)
            {
                /* special message for empty symbols */
                parse_error->type = PARSE_ERROR_ENTRY_DIRECTIVE_GOT_NO_SYMBOL;
            }
            else
            {
                parse_error->type = PARSE_ERROR_ENTRY_DIRECTIVE_GOT_INVALID_SYMBOL;
                parse_error->val.invalid_symbol = parse_symbol_data.val.symbol_parse_error;
            }
            encountered_error = TRUE;
        }
        break;
    }

    case DIRECTIVE_EXTERN:
    {
        trim_end(str);
        /* ensure the symbol is a valid symbol */
        parse_symbol(str, NULL, &parse_symbol_data);
        if (parse_symbol_data.result == HAS_SYMBOL)
        {
            directive->val.extern_symbol = str;
        }
        else if (parse_symbol_data.result == DOES_NOT_HAVE_SYMBOL)
        {
            parse_error->type = PARSE_ERROR_EXTERN_DIRECTIVE_GOT_NO_SYMBOL;
            encountered_error = TRUE;
        }
        else if (parse_symbol_data.result == SYMBOL_PARSE_ERROR)
        {
            if (parse_symbol_data.val.symbol_parse_error.type == SYMBOL_EMPTY)
            {
                /* special message for empty symbols */
                parse_error->type = PARSE_ERROR_EXTERN_DIRECTIVE_GOT_NO_SYMBOL;
            }
            else
            {
                parse_error->type = PARSE_ERROR_EXTERN_DIRECTIVE_GOT_INVALID_SYMBOL;
                parse_error->val.invalid_symbol = parse_symbol_data.val.symbol_parse_error;
            }
            encountered_error = TRUE;
        }
        break;
    }
    }
    return encountered_error;
}

bool parse_operand_symbol_end_indicator(char c)
{
    return isspace(c) || (c == ',') || (c == 0);
}
/* Parses a single operand from an str. If the parse is successfull it puts the length of the operand in the string into operand_length
   and fills the Operand object with the parsed operand. Otherwise it fills ParseError with details about the error.
   Returns TRUE if the parse is unsuccessful, FALSE otherwise.
   Note: This function assumes that the str is not empty and does not start with a space or a ','*/
bool parse_operand(char *str, uint32 *operand_length, Operand *operand, ParseError *parse_error)
{
    int32 integer;
    int chars_read;
    bool encountered_error = FALSE, is_negative;
    ParseSymbolData parse_symbol_data;

    if (*str == '#')
    {
        operand->type = OPERAND_IMMEDIATE;
        str++;
        *operand_length = 1;
        encountered_error = !parse_int32_base10(str, &integer, &chars_read, &is_negative);
        if (chars_read == 0)
        {
            parse_error->type = PARSE_ERROR_OPERAND_NO_INTEGER_AFTER_HASHTAG;
            return encountered_error;
        }
        else if (integer < MIN_INTEGER || (encountered_error && is_negative))
        {
            parse_error->type = PARSE_ERROR_OPERAND_IMMEDIATE_INTEGER_TOO_SMALL;
            if (encountered_error)
            {
                parse_error->val.overflown_integer = 0;
            }
            else
            {
                parse_error->val.overflown_integer = integer;
            }
            encountered_error = TRUE;
            return encountered_error;
        }
        else if (integer > MAX_INTEGER || encountered_error)
        {
            parse_error->type = PARSE_ERROR_OPERAND_IMMEDIATE_INTEGER_TOO_BIG;
            if (encountered_error)
            {
                parse_error->val.overflown_integer = 0;
            }
            else
            {
                parse_error->val.overflown_integer = integer;
            }
            encountered_error = TRUE;
            return encountered_error;
        }

        operand->value.immediate = integer;
        str += chars_read;
        *operand_length += chars_read;
        return encountered_error;
    }
    else if (*str == 'r' && parse_int32_base10(str + 1, &integer, &chars_read, &is_negative) && integer < REGISTER_COUNT && !is_negative)
    {
        operand->type = OPERAND_REGISTER;
        operand->value.register_num = integer;
        str += chars_read + 1; /* +1 for the 'r' character */
        *operand_length = chars_read + 1;
        return encountered_error;
    }
    else
    {
        if (*str == '&')
        {
            operand->type = OPERAND_ADDRESS;
            str++;
        }
        else
        {
            operand->type = OPERAND_SYMBOL;
        }
        *operand_length = 0;
        parse_symbol(str, parse_operand_symbol_end_indicator, &parse_symbol_data);
        if (parse_symbol_data.result == HAS_SYMBOL)
        {
            *operand_length += parse_symbol_data.symbol_length;
            memcpy(operand->value.symbol_name, parse_symbol_data.val.symbol_buffer, (size_t)*operand_length);
        }
        else if (parse_symbol_data.result == SYMBOL_PARSE_ERROR)
        {
            parse_error->type = PARSE_ERROR_OPERAND_INVALID_SYMBOL;
            parse_error->val.invalid_symbol = parse_symbol_data.val.symbol_parse_error;
            encountered_error = TRUE;
            return encountered_error;
        }
        operand->value.symbol_name[*operand_length] = 0;
        if (operand->type == OPERAND_ADDRESS)
        {
            *operand_length += 1; /* to add-in for the &*/
        }
        return encountered_error;
    }
}

/* Parse a single instruction from an str. If successfull, returns FALSE and fills instruction with information about the parsed instruction.
   Otherwise returns TRUE and fills parse_error with infromation about the error encountered during parsing.
   Note: this function modifies the str */
bool parse_instruction(char *str, Instruction *instruction, ParseError *parse_error)
{
    const OperandType *acceptable_operands; /* acceptable operands for an instruction */
    int acceptable_operands_amount, i;      /* amount of acceptable operands in the acceptable_operands array */
    bool is_acceptable_operand;             /* whether or not the current parsed operand is acceptible by the function */
    uint32 operand_amount, operand_len;     /* amount of operands of the instruction, the length the current parsed operand occupies in the string */
    bool encountered_error = FALSE;
    char *str_start = str;
    InstructionType instruction_type;

    if (!str_to_instruction_type(str, &instruction_type))
    {
        parse_error->type = PARSE_ERROR_INVALID_INSTRUCTION;
        parse_error->val.invalid_instruction = trim_after_space(str);
        encountered_error = TRUE;
        return encountered_error;
    }
    instruction->type = instruction_type;
    str += instruction_name_len(instruction_type);

    /* str_to_instruction only checks as much characters as necessary. Thus for the instruction to be valid, we need to make sure that after it comes space or the end*/
    if (!isspace(*str) && *str != 0)
    {
        parse_error->type = PARSE_ERROR_INVALID_INSTRUCTION;
        parse_error->val.invalid_instruction = trim_after_space(str_start); /* we use str_start since we already moved away with str */
        encountered_error = TRUE;
        return encountered_error;
    }
    str = skip_space(str);
    operand_amount = instruction_operand_amount(instruction_type);
    instruction->operand_amount = operand_amount;
    /* We have 4 cases:
       case 1: the instruction needs 0 operands but we have text after it (error)
       case 2: the instruction needs at least 1 operand, but we don't have any text after it (error)
       case 3: the instruction needs 0 operands and we have no text left (we're done)
       case 4: the instruction has at least 1 operand and we have text after it (we need to investigate the instruction further)*/
    if (operand_amount == 0 && *str != 0)
    {
        parse_error->type = PARSE_ERROR_INSTRUCTION_TOO_MANY_OPERANDS;
        parse_error->val.expected_amount_of_operands = operand_amount;
        encountered_error = TRUE;
        return encountered_error;
    }
    else if (*str == 0 && operand_amount > 0)
    {
        parse_error->type = PARSE_ERROR_INSTRUCTION_TOO_LITTLE_OPERANDS;
        parse_error->val.expected_amount_of_operands = operand_amount;
        encountered_error = TRUE;
        return encountered_error;
    }
    else if (*str == 0 && operand_amount == 0)
    {
        /* we got zero operands as expected */
        return encountered_error;
    }
    /* otherwise operand_amount > 0 and *str != 0*/

    /* check if the operand is empty */
    if (*str == ',')
    {
        parse_error->type = PARSE_ERROR_INSTRUCTION_FIRST_OPERAND_EMPTY;
        encountered_error = TRUE;
        return encountered_error;
    }

    /* parse the first operand */
    encountered_error = parse_operand(str, &operand_len, &instruction->operand1, parse_error);
    if (encountered_error)
    {
        return encountered_error;
    }
    str += operand_len;
    str = skip_space(str);

    /* We have 6 cases:
        case 1: after the first operand we found a character which is not null terminator or ',' or a space (error)
        case 2: we found no text after the first operand but operand amount is greater than 1 (error)
        case 3: operand amount is 1 and there is a ',' after the operand (with nothing else after it) (error)
        case 4: we have 1 operand and the text ends (good)
        case 5: we have 1 operand and there is a ',' with more stuff after it (error)
        case 6: we have 2 operands and there is a ',' after the first operand (need to investigate the instruction further)
    */
    if (*str != 0 && *str != ',')
    {
        parse_error->type = PARSE_ERROR_OPERAND_INVALID_CHARACTER_AFTER_OPERAND;
        parse_error->val.invalid_character = *str;
        encountered_error = TRUE;
        return encountered_error;
    }
    else if (*str == 0 && operand_amount > 1)
    {
        parse_error->type = PARSE_ERROR_INSTRUCTION_TOO_LITTLE_OPERANDS;
        parse_error->val.expected_amount_of_operands = operand_amount;
        encountered_error = TRUE;
        return encountered_error;
    }
    else if (*str == ',' && operand_amount == 1 && *skip_space(str + 1) == 0)
    {
        parse_error->type = PARSE_ERROR_INSTRUCTION_COMMA_AFTER_FINAL_OPERAND;
        encountered_error = TRUE;
        return encountered_error;
    }
    else if (*str == 0 && operand_amount == 1)
    {
        /* we got 1 operands as expected, now we check that this operand is expected for this instruction */
        is_acceptable_operand = FALSE;
        acceptable_operands = acceptable_dest_operands(instruction_type, &acceptable_operands_amount);
        for (i = 0; i < acceptable_operands_amount; ++i)
        {
            if (instruction->operand1.type == acceptable_operands[i])
            {
                is_acceptable_operand = TRUE;
            }
        }
        if (!is_acceptable_operand)
        {
            /* note: this must be first, since instruction and parse_error could point to the same place (union) */
            parse_error->val.expected_operands_type.bad_op_type = instruction->operand1.type;
            parse_error->type = PARSE_ERROR_INSTRUCTION_EXPECTED_A_DIFFERENT_OPERAND_TYPE;
            parse_error->val.expected_operands_type.acceptable_operands = acceptable_operands;
            parse_error->val.expected_operands_type.len = acceptable_operands_amount;
            parse_error->val.expected_operands_type.op_index = 1;
            encountered_error = TRUE;
            return encountered_error;
        }
        return encountered_error;
    }
    else if (operand_amount == 1)
    {
        parse_error->type = PARSE_ERROR_INSTRUCTION_TOO_MANY_OPERANDS;
        parse_error->val.expected_amount_of_operands = operand_amount;
        encountered_error = TRUE;
        return encountered_error;
    }
    /* otherwise operand_amount > 1 and *str == ',' */
    str++; /* skip ',' */
    str = skip_space(str);
    if (*str == 0)
    {
        parse_error->type = PARSE_ERROR_INSTRUCTION_TOO_LITTLE_OPERANDS;
        parse_error->val.expected_amount_of_operands = operand_amount;
        encountered_error = TRUE;
        return encountered_error;
    }

    /* parse the second operand */
    encountered_error = parse_operand(str, &operand_len, &instruction->operand2, parse_error);
    if (encountered_error)
    {
        return encountered_error;
    }
    str += operand_len;
    str = skip_space(str);
    /* we have 4 cases:
        case 1: The text isn't over and there is no ',' (error)
        case 2: The text isn't over and there is a ',' with nothing else after it (error)
        case 3: the text is over (good)
        case 4: There is a ',' and the text is not over (error since there can only be 2 operands)*/
    if (*str != 0 && *str != ',')
    {
        parse_error->type = PARSE_ERROR_OPERAND_INVALID_CHARACTER_AFTER_OPERAND;
        parse_error->val.invalid_character = *str;
        encountered_error = TRUE;
        return encountered_error;
    }
    else if (*str == ',' && *skip_space(str + 1) == 0)
    {
        parse_error->type = PARSE_ERROR_INSTRUCTION_COMMA_AFTER_FINAL_OPERAND;
        encountered_error = TRUE;
        return encountered_error;
    }
    else if (*str == 0)
    {
        /* we got 2 operands as expected, we need to check that their types matches the expected */
        is_acceptable_operand = FALSE;
        acceptable_operands = acceptable_src_operands(instruction->type, &acceptable_operands_amount);
        for (i = 0; i < acceptable_operands_amount; ++i)
        {
            if (instruction->operand1.type == acceptable_operands[i])
            {
                is_acceptable_operand = TRUE;
            }
        }
        if (!is_acceptable_operand)
        {
            /* note: this must be first, since instruction and parse_error could point to the same place (union) */

            parse_error->type = PARSE_ERROR_INSTRUCTION_EXPECTED_A_DIFFERENT_OPERAND_TYPE;
            parse_error->val.expected_operands_type.bad_op_type = instruction->operand1.type;
            parse_error->val.expected_operands_type.acceptable_operands = acceptable_operands;
            parse_error->val.expected_operands_type.len = acceptable_operands_amount;
            parse_error->val.expected_operands_type.op_index = 1;
            encountered_error = TRUE;
            return encountered_error;
        }

        is_acceptable_operand = FALSE;
        acceptable_operands = acceptable_dest_operands(instruction->type, &acceptable_operands_amount);
        for (i = 0; i < acceptable_operands_amount; ++i)
        {
            if (instruction->operand2.type == acceptable_operands[i])
            {
                is_acceptable_operand = TRUE;
            }
        }
        if (!is_acceptable_operand)
        {
            /* note: this must be first, since instruction and parse_error could point to the same place (union) */
            parse_error->val.expected_operands_type.bad_op_type = instruction->operand2.type;

            parse_error->type = PARSE_ERROR_INSTRUCTION_EXPECTED_A_DIFFERENT_OPERAND_TYPE;
            parse_error->val.expected_operands_type.acceptable_operands = acceptable_operands;
            parse_error->val.expected_operands_type.len = acceptable_operands_amount;
            parse_error->val.expected_operands_type.op_index = 2;
            encountered_error = TRUE;
            return encountered_error;
        }
        return encountered_error;
    }
    /* otherwise - we're in a position to parse another operand. However, there can only be 2 operands! */
    parse_error->type = PARSE_ERROR_INSTRUCTION_TOO_MANY_OPERANDS;
    parse_error->val.expected_amount_of_operands = operand_amount;
    encountered_error = TRUE;
    return encountered_error;
}

/* returns c == LABEL_END_CHAR */
bool parse_line_label_end_indicator(char c)
{
    return c == LABEL_END_CHAR;
}

void parse_line(char *line, ParseLineData *parse_line_data)
{
    char *line_ptr = line;
    ParseSymbolData parse_symbol_data;
    bool encountered_error;

    if (line[0] == '\n' || line[0] == 0)
    {
        parse_line_data->type = PARSE_LINE_EMPTY;
        return;
    }
    else if (line[0] == ';')
    {
        parse_line_data->type = PARSE_LINE_COMMENT;
        return;
    }
    line_ptr = skip_space(line_ptr);
    /* try to parse a label */
    parse_symbol(line_ptr, parse_line_label_end_indicator, &parse_symbol_data);
    parse_line_data->parse_label_data = parse_symbol_data;

    line_ptr += parse_symbol_data.symbol_length;

    if (parse_symbol_data.result != DOES_NOT_HAVE_SYMBOL)
    {
        line_ptr++; /* if we have a label, even if invalid, we need to consider the ':' character */
        if (!isspace(*line_ptr))
        {
            /* error - we have no space after the label */
            parse_line_data->type = PARSE_LINE_ERROR;
            parse_line_data->val.parse_error.type = PARSE_ERROR_EXPECTED_A_SPACE_AFTER_LABEL;
            return;
        }
    }

    /* skip any space which might be found after the end of the label */
    line_ptr = skip_space(line_ptr);

    if (*line_ptr == 0)
    {
        /* we got nothing after a label... error */
        parse_line_data->type = PARSE_LINE_ERROR;
        parse_line_data->val.parse_error.type = PARSE_ERROR_EXPECTED_INSTRUCTION_OR_DIRECTIVE_AFTER_LABEL;
    }
    else if (*line_ptr == '.')
    {
        /* we have a directive! */
        parse_line_data->type = PARSE_LINE_DIRECTIVE;
        line_ptr++; /* skip the '.' */
        encountered_error = parse_directive(line_ptr, &parse_line_data->val.directive, &parse_line_data->val.parse_error);
        if (encountered_error)
        {
            parse_line_data->type = PARSE_LINE_ERROR;
        }
    }
    else
    {
        parse_line_data->type = PARSE_LINE_INSTRUCTION;
        encountered_error = parse_instruction(line_ptr, &parse_line_data->val.instruction, &parse_line_data->val.parse_error);
        if (encountered_error)
        {
            parse_line_data->type = PARSE_LINE_ERROR;
        }
    }
}

bool is_a_register(const char *str)
{
    int register_num, chars_read;
    bool is_negative;
    if (*str == 'r' && parse_int32_base10(str + 1, &register_num, &chars_read, &is_negative) && *(str + 1 + chars_read) == 0 && register_num < REGISTER_COUNT && !is_negative)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
