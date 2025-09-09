/* This module contains the error and error callback types that the whole app uses, as well as a method to convert an error to a string. */
#ifndef _MMN14_ERRORS_H_
#define _MMN14_ERRORS_H_
#include "parser.h"
#include "symbol_table.h"

/* An upper bound to the size necessary to turn an error to a string in error_to_string.
   This bound is obtained by the fact that the biggest line which can be displayed is 80 characters,
   the biggest error is one with less than 300 characters (including format arguements), all the colors together in the message take less than 70 characters,
   and all the extra words (e.g. "error:", "info:", etc.) takes less than 50 characters,
   giving us an upper bound of 500 characters. */
#define ERROR_TO_STRING_BUF_SIZE_UPPER_BOUND 500

/* Type of macro expansion error */
typedef enum
{
    /* Found a line which is over MAX_LINE_LENGTH long */
    EXPAND_MACRO_ERROR_LINE_TOO_LONG,
    /* Got a macro definition without a name for the macro */
    EXPAND_MACRO_EXPECTED_MACRO_NAME,
    /* Macro starts with an invalid character */
    EXPAND_MACRO_ERROR_STARTS_WITH_INVALID_CHARACTER,
    /* The macro has the name of an instruction */
    EXPAND_MACRO_ERROR_IS_AN_INSTRUCTION,
    /* The macro has the name of a directive */
    EXPAND_MACRO_ERROR_IS_A_DIRECTIVE,
    /* The macro has the name of a register */
    EXPAND_MACRO_ERROR_IS_A_REGISTER,
    /* The macro has an invalid character in its name */
    EXPAND_MACRO_ERROR_INVALID_CHARACTER,
    /* The name of the macro is bigger than MAX_MACRO_NAME_LENGTH */
    EXPAND_MACRO_ERROR_NAME_IS_TOO_LONG,
    /* After defining the macro it was also found to be defined as a label */
    EXPAND_MACRO_ERROR_MACRO_DEFINED_AS_LABEL

} ExpandMacroErrorType;

/* Represents an error which happened during macro expansion*/
typedef struct
{
    /* type of error*/
    ExpandMacroErrorType type;
    /* value of the error*/
    union
    {
        /* Only valid when type is EXPAND_MACRO_ERROR_STARTS_WITH_INVALID_CHARACTER. Represents the invalid character which the macro name starts with */
        char starts_with_invalid_character;
        /* Only valid when type is EXPAND_MACRO_ERROR_NAME_IS_TOO_LONG. */
        struct
        {
            /* the length of the macro name */
            int len;
            /* the expected (max) length of the macro name */
            int expected_len;
        } is_too_long;

        /* Only valid when type is EXPAND_MACRO_ERROR_INVALID_CHARACTER */
        struct
        {
            /* the invalid character */
            char invalid_character;
            /* the position of the invalid character in the macro name */
            int position;
        } invalid_character;

        /* only valid when type is EXPAND_MACRO_ERROR_MACRO_DEFINED_AS_LABEL. It is the name of the macro which has also been defined as a label */
        char *macro_name;

    } val;
} ExpandMacroError;

/* Contains a line along with related information (e.g. the line number )*/
typedef struct
{
    /* The number of the line in the file */
    int line_num;
    /* Pointer to a buffer which contains the line */
    char *line;
} LineInfo;

/* The type of error */
typedef enum
{
    /* Error while expanding macros */
    ERROR_TYPE_MACRO,
    /* error while parsing a symbol */
    ERROR_TYPE_SYMBOL_PARSE,
    /* error while parsing the line (labels excluded)*/
    ERROR_TYPE_PARSE,
    /* A symbol has been defined again despite already being defined */
    ERROR_TYPE_SYMBOL_ALREADY_DEFINED,
    /* The file is using too much of the address space of the machine */
    ERROR_TYPE_MEMORY_OVERFLOWN,
    /* A symbol used in the file in .entry directive or an instruction is not defined anywhere */
    ERROR_TYPE_SYMBOL_NOT_DEFINED,
    /* A symbol is defined in a .extern directive but is also used in a .entry directive */
    ERROR_TYPE_EXTERNAL_SYMBOL_USED_IN_ENTRY_DIRECTIVE

} ErrorType;

/* An error type which encapsulates all the errors. See error_to_string to turn this into a string.
   This type acts as a pointer, and as such, it should not be assumed to live forever. */
typedef struct
{
    /* the type of error */
    ErrorType type;
    /* Information about the line in which the error has occured */
    LineInfo line_info;
    /* The value of the error. Changes depending on the type of error */
    union
    {
        /* Only valid when type is ERROR_TYPE_MACRO. Has information about the error which happened during macro expansion */
        ExpandMacroError *expand_macro_err;
        /* Only valid when type is ERROR_TYPE_SYMBOL_PARSE. Has information about the error which happened during the parsing of a symbol
           Check parser.h for more info */
        ParseSymbolError *symbol_parse_err;
        /* Only valid when type is ERROR_TYPE_PARSE. Has information about the error which happened during the parsing of a line (label excluded)
           Check parser.h for more info */
        ParseError *parse_err;
        /* Only valid when type is ERROR_TYPE_SYMBOL_ALREADY_DEFINED or ERROR_TYPE_EXTERNAL_SYMBOL_USED_IN_ENTRY_DIRECTIVE.
          This is the symbol which has been defined twice/The external symbol which was used in an entry directive.  */
        Symbol *symbol;
        /* Only valid when type is ERROR_TYPE_SYMBOL_ALREADY_DEFINED_AS_MACRO. This is the name of the macro which has been defined again as a symbol */
        char *macro_name;
        /* Only valid when type is ERROR_TYPE_MEMORY_OVERFLOWN */
        struct
        {
            int max_address;
            int expected_max_address;
        } memory_overflown;

        /* Only valid when type is ERROR_TYPE_SYMBOL_NOT_DEFINED. This is the symbol which has not been defined. */
        char *symbol_name;
    } val;
} Error;

/**
 * @brief turn an error into a displayable string
 * @param error the error you wish to turn into a string
 * @param buf a buffer to hold the characters of the string. Note: See ERROR_TO_STRING_BUF_SIZE_UPPER_BOUND for sufficient buffer size.
 * Providing a buffer of a smaller size may result in undefined behvaior
 */
void error_to_string(Error error, char *buf);

/* Represents an callback which is called each time there is an error in one of the passes, along with some local data which you can bring along with it. */
typedef struct
{
    /* Callback which takes an error and some local data. This callback is called each time there is an error in one of the three passes.
       It is called with the error along with the data field of this struct, which you may use for providing local data (for example, file name) to the callback */
    void (*callback)(Error, void *);
    /* Local data which is provided to the callback each time it is called */
    void *data;
} ErrorCallback;

/**
 * @brief call an error callback with a specified error
 * @param err_callback the error callback to call
 * @param err the specified error
 */
void err(ErrorCallback err_callback, Error err);

#endif