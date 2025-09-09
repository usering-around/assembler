#ifndef _MMN14_MACROS_H_
#define _MMN14_MACROS_H_
#include <stdio.h>
#include "errors.h"
#include "vector.h"
#include "bool.h"

/* the maximum name of a macro */
#define MAX_MACRO_NAME_LENGTH 31

/* Represents a macro which is pasted upon using its name in an assembly file */
typedef struct
{
    /* The name of the macro */
    char name[MAX_MACRO_NAME_LENGTH + 1];
    /* The body of the macro */
    CharVector *data;
} Macro;

VECTOR_HEADER(Macro, MacroVector, macro)

/* A map between a macro's name to its representation. This type acts as a pointer, meaning it is fine to return it by value as long as it is not freed */
typedef struct
{
    MacroVector *inner;
} MacroTable;

/**
 * @brief Search the table for a macro with a specified name
 * @param table the macro table to search in
 * @param name the name of the macro to search
 * @return NULL if the macro was not found, a valid pointer to a Macro object if found.
 */
Macro *macro_table_search(MacroTable *table, char *name);

/* result of a macro expansion operation. */
typedef struct
{
    /* whether or not we encountered an error during the expansion. */
    bool encountered_error;
    /* whether or not we encountered an allocation failure during the expansion.
       Note: upon encountering an allocation failure, the expansion immediately stops. */
    bool alloc_fail;
} MacroExpansionResult;

/**
 * @brief Expand macros in an assembly file to some other file.
 * If any errors are found during the process, err_callback will be called with the appropriate error.
 * Note: assumes that macros are always defined before they're used, and that all macro definition have a corresponding mcroend.
 * @param in the input file
 * @param out the output file
 * @param err_callback the callback to call upon an error
 * @return MacroExpansionResult object containing information gathered during the expansion. Read its documentaiton for more information.
 */
MacroExpansionResult expand_macros(FILE *in, FILE *out, ErrorCallback err_callback);

#endif