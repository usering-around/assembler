/* Thie module contains the directive struct and some other useful utilities that have to do with directives. */

#ifndef _MMN14_DIRECTIVES_H_
#define _MMN14_DIRECTIVES_H_
#include "bool.h"
#include "utils.h" /* for MAX_LINE_LENGTH and the integer types  */

/* The type of directive */
typedef enum
{
    /* .extern directive */
    DIRECTIVE_EXTERN,
    /* .entry directive */
    DIRECTIVE_ENTRY,
    /* .data directive */
    DIRECTIVE_DATA,
    /* .string directive */
    DIRECTIVE_STRING
} DirectiveType;

/* Represents a directive statement */
typedef struct
{
    /* The type of directive */
    DirectiveType type;
    /* The value of the directive. Changes depending on type. */
    union
    {
        /* Only valid when type is DIRECTIVE_ENTRY. It represents the symbol the .entry directive got */
        char *entry_symbol;
        /* Only valid when type is DIRECTIVE_EXTERN. It represents the symbol the .extern directive got */
        char *extern_symbol;
        /* Only valid when type is DIRECTIVE_DATA. It represents the list of integers that the .data directive got */
        struct
        {
            /* The list of integers. MAX_LINE_LENGTH/2 comes from the fact that you must put a ',' after each number, limiting the maximum amount of numbers you can put. */
            int integers[MAX_LINE_LENGTH / 2];
            /* The amount of integers in the list. */
            uint32 amount_of_integers;
        } data;
        /* Only valid when type is DIRECTIVE_STRING. It represents the string that the .string directive got */
        char string[MAX_LINE_LENGTH];
    } val;
} Directive;

/**
 * @brief Attempts to parse a string as a directive. Note: this function will only attemps to parse as much as necessary,
 * so for example a string like "data blah blah" would get flagged as a data directive.
 * @param str the string to parse.
 * @param type out parameter. If the string is a directive, this parameter is set to be the corresponding DirectiveType.
 * @return TRUE if the string does start with the name of a directive, FALSE otherwise.
 */
bool str_to_directive_type(const char *str, DirectiveType *type);

/**
 * @brief Check if a string exactly matches a directive's name. Note: this does exact matching,
 * so for example someting like "data blah blah" would get flagged as false, however "data" would be flagged as true.
 * @param str the str to check.
 * @return TRUE if the str exactly matches a directive's name, FALSE otherwise.
 */
bool is_a_directive(const char *str);

/**
 * @brief Get the length of the name of a directive
 * @param type the directive of which to get the length of
 * @return the length of the directive's name
 */
uint32 directive_name_len(DirectiveType type);

#endif