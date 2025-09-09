/* This module contains the parse_line function which parses a single assembly line, objects related to the data/errors found during the parsing,
   and other related utilities/constants */
#ifndef _MMN14_PARSER_H_
#define _MMN14_PARSER_H_

#include "bool.h"
#include "instructions.h"
#include "directives.h"
#include "utils.h"

/* the size of a word in the CPU*/
#define WORD_SIZE 24
/* amount of reserved bits in extra words (3 for the A,R,E field) */
#define EXTRA_WORD_RESERVED_BITS 3
/* The biggest integer you can use as an immediate or in a .data directive (maximum of WORD_SIZE - EXTRA_WORD_RESERVED_BITS bits signed integer)*/
#define MAX_INTEGER ((2 << (WORD_SIZE - EXTRA_WORD_RESERVED_BITS - 1)) - 1)
/* The smallest integer you can use as an immediate or in a .data directive (minimum of WORD_SIZE - EXTRA_WORD_RESERVED_BITS bits signed integer)*/
#define MIN_INTEGER (-1 * (2 << (WORD_SIZE - EXTRA_WORD_RESERVED_BITS - 1)))
/* The amount of registers the CPU has. */
#define REGISTER_COUNT 8
/* The label end character */
#define LABEL_END_CHAR ':'

/* The result of parsing a symbol. A symbol is defined as a non-empty string of characters which has some end indicator (e.g. ':' or null terminator).
   A symbol may be valid or not depending on some rules. read ParseSymbolErrorType for more information about that.
   When parsing a symbol there are 3 possiblities:
   1. Its end indicator is found and the symbol is valid
   2. The end indicator is not found and thus there is no symbol
   3. The end indicator is found but the symbol is invalid.  */
typedef enum
{
    HAS_SYMBOL,           /* there is a symbol and it is valid */
    DOES_NOT_HAVE_SYMBOL, /* there is no symbol */
    SYMBOL_PARSE_ERROR    /* the end indicator has been found but symbol is invalid */

} ParseSymbolResult;

/* The type of error which can occur when parsing a symbol */
typedef enum
{
    SYMBOL_STARTS_WITH_NON_ALPHABETHIC_CHARACTER, /* there is a symbol but the first character is a non-alphabethic character */
    INVALID_CHARACTER_IN_SYMBOL,                  /* there is a symbol but it has an character which is not numeric nor alphabethic (e.g. '$')*/
    BUFFER_TOO_SMALL,                             /* there is a symbol but the current buffer is too small to fit it */
    SYMBOL_EMPTY,                                 /* A symbol can be empty if it has a defined end, that end is found but the symbol is not. e.g. a label is a
    symbol which ends with a ':', so an empty label would be ":". */
    SYMBOL_IS_A_DIRECTIVE,                        /* the symbol has the name of a directive */
    SYMBOL_IS_AN_INSTRUCTION,                     /* the symbol has the name of an instruction */
    SYMBOL_IS_A_REGISTER                          /* the symbol has the name of a register */
} ParseSymbolErrorType;

/* Represents an error while parsing a label */
typedef struct
{
    /* the type of error */
    ParseSymbolErrorType type;
    /* the value of the error. Changes depending on type. */
    union
    {
        /* This is valid only when type is INVALID_CHARACTER_IN_SYMBOL */
        struct
        {
            /* the offending symbol */
            char symbol[MAX_LABEL_SIZE + 1];
            /* the offending character */
            char invalid_char;
            /* the position of the offending character */
            int position;
        } invalid_char_in_symbol;
        /* This is valid only when type is SYMBOL_STARTS_WITH_NON_ALPHABETHIC_CHARACTER*/
        struct
        {
            /* the offending label */
            char symbol[MAX_LABEL_SIZE + 1];
            /* the offending non-alphabethic character */
            char non_alphabethic_char;
        } symbol_starts_with_non_alphabethic_char;
        /* This is valid only when type is BUFFER_TOO_SMALL. It is the length of the symbol which didn't fit. */
        uint32 symbol_length;
        /* only valid when type is SYMBOL_IS_AN_INSTRUCTION or SYMBOL_IS_AN_DIRECTIVE. This is the symbol*/
        char symbol[MAX_LABEL_SIZE + 1];
    } val;
} ParseSymbolError;

/* The data of a symbol parse. A symbol is defined as a string of characters which has some end indicator (e.g. ':' or null terminator).
   Read ParseSymbolResult and ParseSymbolError for more detail.*/
typedef struct
{
    /* the result of the parsing. */
    ParseSymbolResult result;

    /* the length of the symbol or 0 if there is no symbol. Note: does not include the end indiactor character. */
    int symbol_length;
    union
    {
        /* An error if there is any. Only valid when result is SYMBOL_PARSE_ERROR */
        ParseSymbolError symbol_parse_error;
        /* the symbol itself. It is null terminated.
         It is valid only if the result is HAS_SYMBOL */
        char symbol_buffer[MAX_LABEL_SIZE + 1];
    } val;
} ParseSymbolData;

/* The possible types of parse errors (label errors excluded) */
typedef enum
{
    /* We have a label but there is nothing after it */
    PARSE_ERROR_EXPECTED_INSTRUCTION_OR_DIRECTIVE_AFTER_LABEL,
    /* We have a label but no space after it */
    PARSE_ERROR_EXPECTED_A_SPACE_AFTER_LABEL,
    /* The directive is not recognized */
    PARSE_ERROR_INVALID_DIRECTIVE,
    /* The .data directive got empty data */
    PARSE_ERROR_DATA_DIRECTIVE_EMPTY_DATA,
    /* The .data directive got something which is not an integer*/
    PARSE_ERROR_DATA_DIRECTIVE_NOT_AN_INTEGER,
    /* The .data directive got an invalid character after an integer */
    PARSE_ERROR_DATA_DIRECTIVE_INVALID_CHARACTER_AFTER_INTEGER,
    /* The .data directive got a comma after the last integer */
    PARSE_ERROR_DATA_DIRECTIVE_COMMA_AFTER_LAST_INTEGER,
    /* The .data directive got an integer which is bigger than the limit (see MAX_INTEGER)*/
    PARSE_ERROR_DATA_DIRECTIVE_INTEGER_BIGGER_THAN_LIMIT,
    /* The .data directive got an integer which is smaller than the limit (see MIN_INTEGER)*/
    PARSE_ERROR_DATA_DIRECTIVE_INTEGER_SMALLER_THAN_LIMIT,
    /* The .string directive got a string which does not start with a quote */
    PARSE_ERROR_STRING_DIRECTIVE_DOES_NOT_START_WITH_QUOTE,
    /* The .string directive got a string which does not end with a quote */
    PARSE_ERROR_STRING_DIRECTIVE_DOES_NOT_END_WITH_QUOTE,
    /* Entry directive didn't get a symbol */
    PARSE_ERROR_ENTRY_DIRECTIVE_GOT_NO_SYMBOL,
    /* Entry directive got an invalid symbol */
    PARSE_ERROR_ENTRY_DIRECTIVE_GOT_INVALID_SYMBOL,
    /* Extern directive got no symbol */
    PARSE_ERROR_EXTERN_DIRECTIVE_GOT_NO_SYMBOL,
    /* Extern directive got an invalid symbol */
    PARSE_ERROR_EXTERN_DIRECTIVE_GOT_INVALID_SYMBOL,
    /* We got an invalid instruction. Anything which is not a directive, comment, or a label is assumed to be an instruction */
    PARSE_ERROR_INVALID_INSTRUCTION,
    /* The operand of the instruction has an # but no integer after it, which is invalid since we're expecting an immediate integer */
    PARSE_ERROR_OPERAND_NO_INTEGER_AFTER_HASHTAG,
    /* The immediate integer is too big (see MAX_INTEGER)*/
    PARSE_ERROR_OPERAND_IMMEDIATE_INTEGER_TOO_BIG,
    /* The immediate integer is too small (see MIN_INTEGER)*/
    PARSE_ERROR_OPERAND_IMMEDIATE_INTEGER_TOO_SMALL,

    /* We got an unexpected character after the operand */
    PARSE_ERROR_OPERAND_INVALID_CHARACTER_AFTER_OPERAND,
    /* Operand is an invalid symbol */
    PARSE_ERROR_OPERAND_INVALID_SYMBOL,
    /* We got too many operands for the instruction */
    PARSE_ERROR_INSTRUCTION_TOO_MANY_OPERANDS,
    /* We got too little operands for the instruction */
    PARSE_ERROR_INSTRUCTION_TOO_LITTLE_OPERANDS,
    /* We got a comma after the final operand */
    PARSE_ERROR_INSTRUCTION_COMMA_AFTER_FINAL_OPERAND,
    /* We expected a different operand type for this instruction */
    PARSE_ERROR_INSTRUCTION_EXPECTED_A_DIFFERENT_OPERAND_TYPE,
    /* The first operand of the instruction is missing (called in situations like add , r2)*/
    PARSE_ERROR_INSTRUCTION_FIRST_OPERAND_EMPTY
} ParseErrorType;

/* Represents an parsing error (label errors excluded)*/
typedef struct
{
    /* the type of error */
    ParseErrorType type;
    /* the value of the error. Changes depending on type */
    union
    {
        /* Only valid when type is PARSE_ERROR_INVALID_DIRECTIVE. This is the invalid directive which was given */
        char *invalid_directive;
        /* Only valid when type is PARSE_ERROR_INVALID_INSTRUCTION. This is the invalid instruction which was given */
        char *invalid_instruction;
        /* Only valid when type is PARSE_ERROR_DATA_DIRECTIVE_INVALID_CHARACTER_AFTER_INTEGER or PARSE_ERROR_OPERAND_INVALID_CHARACTER_AFTER_OPERAND.
        This is the invalid character which was given. */
        char invalid_character;
        /* Only valid when type is PARSE_ERROR_DATA_DIRECTIVE_INTEGER_BIGGER_THAN_LIMIT or PARSE_ERROR_DATA_DIRECTIVE_INTEGER_SMALLER_THAN_LIMIT
        PARSE_ERROR_OPERAND_IMMEDIATE_INTEGER_TOO_BIG or PARSE_ERROR_OPERAND_IMMEDIATE_INTEGER_TOO_SMALL.
        It represents the big integer which was given if represntable in 32bits, otherwise 0. */
        int32 overflown_integer;
        /* Only valid when type is PARSE_ERROR_INSTRUCTION_TOO_MANY_OPERANDS or PARSE_ERROR_INSTRUCTION_TOO_LITTLE_OPERANDS.
        It represents the amount of operands which the instruction has expected. */
        uint32 expected_amount_of_operands;
        /* Only valid when type is PARSE_ERROR_ENTRY_DIRECTIVE_GOT_INVALID_SYMBOL or PARSE_ERROR_EXTERN_DIRECTIVE_GOT_INVALID_SYMBOL or PARSE_ERROR_OPERAND_INVALID_SYMBOL.
           It is the invalid symbol which is found.*/
        ParseSymbolError invalid_symbol;

        /* Only valid when type is PARSE_ERROR_INSTRUCTION_EXPECTED_A_DIFFERENT_OPERAND_TYPE.
           It is an array containing the expected operand type as well as the op type which was bad. */
        struct
        {
            /* the index of the operand which is not acceptable by the instruction */
            int op_index;
            /* the operand_type which is not acceptable by the instruction */
            OperandType bad_op_type;
            /* An array of the operand type which the function accepts as the op_index operand */
            const OperandType *acceptable_operands;
            /* the length of the operand type array */
            uint32 len;

        } expected_operands_type;
    } val;
} ParseError;

/* The type of line the parsing found */
typedef enum
{
    /* A line which has an error in it (not accounting for label errors)*/
    PARSE_LINE_ERROR,
    /* A line with an instruction */
    PARSE_LINE_INSTRUCTION,
    /* A line with a directive */
    PARSE_LINE_DIRECTIVE,
    /* A line with a comment*/
    PARSE_LINE_COMMENT,
    /* An empty line */
    PARSE_LINE_EMPTY
} ParseLineType;

/* The data of parsing 1 line.*/
typedef struct
{
    /* The data of the label parsing */
    ParseSymbolData parse_label_data;
    /* The type of line we got */
    ParseLineType type;
    /* The data about the line. Changes depending on type.*/
    union
    {
        /* This is valid only when type is PARSE_LINE_ERROR. It represents the error in the parsing. See the ParseError type. */
        ParseError parse_error;
        /* This is valid only when type is PARSE_LINE_DIRECTIVE. It represents that we parsed a directive. See the Directive type. */
        Directive directive;
        /* This is valid only when type is PARSE_LINE_INSTRUCTION. It represents that we parsed an instruction. See the Instruction type. */
        Instruction instruction;
    } val;
} ParseLineData;

/* Parse 1 line of an assembly file. Assumes that there are no macros. ParseLineData* is an out parameter which this function fills out. See the ParseLineData type.
   Note: this function may modify the line */

/**
 * @brief Parse a single line of an assembly file. This function assumes that there are no macros or other extensions to the assembly language.
 * @param line the line to parse. Note: this line may be modified by the function
 * @param parse_line_data out parameter. This function fills its fields in accordance with the parsing of the line. Read ParseLineData type for more information.
 */
void parse_line(char *line, ParseLineData *parse_line_data);

/**
 * @brief Check if a string exactly matches the name of a register. For example it returns TRUE if str = "r4", but will return false if str="r9" or str="r4 random"
 * @param str the string to check
 * @return TRUE if the string matches a register name, FALSE otherwise.
 */
bool is_a_register(const char *str);

#endif