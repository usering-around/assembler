#include <string.h>
#include <ctype.h>
#include "macros.h"
#include "instructions.h"
#include "directives.h"
#include "parser.h"
#include "utils.h"

VECTOR_IMPL(Macro, MacroVector, macro)

/* write a string str to a CharVec vec. A write may fail if allocation for the CharVec fails.
   Returns TRUE if the write was successfull, FALSE otherwise. */
bool char_vec_write_str(CharVector *vec, char *str)
{
    while (*str != 0)
    {
        if (!char_vec_push(vec, *str))
        {
            return FALSE;
        }
        str++;
    }
    return TRUE;
}

/* Initialize a new macro table.
   macro_table is an out parameter. It is the MacroTable to initialize. And initialization may fail if alloacting memory for it fails.
   Returns TRUE if initialization was successfull, FALSE otherwise. */
bool macro_table_init(MacroTable *macro_table)
{
    macro_table->inner = macro_vec_create();
    if (macro_table->inner == NULL)
    {
        return FALSE;
    }
    return TRUE;
}

/* free the macro table */
void macro_table_free(MacroTable macro_table)
{
    macro_vec_free(macro_table.inner);
}

/* Search for a macro inside the table. The search is implemented via a simple linear search.
   Returns a pointer to the Macro if it was found, NULL otherwise. */
Macro *macro_table_search(MacroTable *table, char *name)
{
    uint32 i;
    for (i = 0; i < table->inner->len; ++i)
    {
        if (strcmp(macro_vec_get_ptr(table->inner, i)->name, name) == 0)
        {
            return macro_vec_get_ptr(table->inner, i);
        }
    }
    return NULL;
}

/* Insert a macro with a certain name to the table. An insertion may fail if allocation was unsuccesfull
   Returns TRUE if the insertion was successfull, FALSE otherwise. */
bool macro_table_insert(MacroTable *table, char *name)
{
    Macro macro;
    memcpy(macro.name, name, MAX_MACRO_NAME_LENGTH);
    macro.name[MAX_MACRO_NAME_LENGTH] = 0;
    macro.data = char_vec_create();
    if (macro.data == NULL)
    {
        return FALSE;
    }
    return macro_vec_push(table->inner, macro);
}

/* Write data to a macro inside the table. Writing multiple times will result in the data being concatenated.
   Attempting to write on an entry which does not exist will do nothing
   Writing may fail if allocation for new memory fails.
   Returns TRUE if the write was successful, FALSE otherwise.*/
bool macro_table_write(MacroTable *table, char *name, char *data)
{
    Macro *macro = macro_table_search(table, name);
    if (macro != NULL)
    {
        return char_vec_write_str(macro->data, data);
    }
    return TRUE;
}

/* Check if a macro name has invalid characters in it, i.e. characters which are not numbers, alphabethic or the '_' character.
   This function returns TRUE if it found an invalid character, FALSE otherwise.
   If the function returns TRUE, then the two out parameters invalid_char and invalid_char_pos will be set to be
   the invalid character found and its position respectively. */
bool has_invalid_characters(const char *mcro_name, char *invalid_char, int *invalid_char_pos)
{
    int pos = 1;
    while (isalnum(*mcro_name) || *mcro_name == '_')
    {
        mcro_name++;
        pos++;
    }
    if (*mcro_name == 0)
    {
        return FALSE;
    }
    else
    {
        *invalid_char = *mcro_name;
        *invalid_char_pos = pos;
        return TRUE;
    }
}

/* Expand macros in a file. The algorithm works as follows:
   we set a flag IS_IN_MACRO = FALSE representing whether or not we're currently in a macro definition's body
   We go through each line:
    if the line is too big we call err_callback with the appropriate error.
    else if IS_IN_MACRO:
        if the line consists of just "mcroend", we pt IS_IN_MACRO = FALSE.
        otherwise we copy the current line into the body of the macro that is being defined inside the macro table
    else if the line starts with "mcro":
        we set IS_IN_MACRO = TRUE and parse the macro name and
        call err_callback with an approrpiate error if it is a reserved keyword (i.e. instruction, directive, register names),
        or if it contains invalid characters in it.
        If there is no error in the parsing, we set IS_IN_MACRO = TRUE and put the macro's name as an entry in the macro table.
    else if the line contains a macro's name:
        we paste the macro into the output file
    else:
        we paste the current line into the output file

  Once we reach EOF, we do another pass on the file to check that none of the macros have been defined as a label,
  and call err_callback with the approrpiate error if they have.
  At the end we return MacroExpansionResult with the result that we got.
     */
MacroExpansionResult expand_macros(FILE *in, FILE *out, ErrorCallback err_callback)
{
    char line[MAX_LINE_LENGTH + 2];                     /* the buffer for the line in the file */
    char line_copy[sizeof(line)];                       /* a copy of the buffer, used for line_info */
    char *mcro_name, *line_ptr;                         /* the name of the macro and a pointer to the line buffer*/
    char current_macro_name[MAX_MACRO_NAME_LENGTH + 1]; /* the name of the current macro that is being defined */
    bool is_in_macro = FALSE;                           /* whether or not we're currently in a macro definition */
    MacroTable macro_table;                             /* the table which holds all the macros and their definition */
    Macro *macro;                                       /* a pointer to a macro in the macro table */
    LineInfo line_info;                                 /* information about the current line */
    bool encountered_error = FALSE;                     /* whether or not we encountered an error */
    ExpandMacroError expand_macro_err;                  /* an error we encountered during macro expansion, if we find any*/
    Error error;                                        /* error we return for err_callback */
    MacroExpansionResult macro_expansion_result;        /* the result we return */
    uint32 i;
    char invalid_character;
    int invalid_character_pos;
    char c;

    /* initialize the macro table */
    if (!macro_table_init(&macro_table))
    {
        /* we encountered an allocation failure - immediately return */
        macro_expansion_result.alloc_fail = TRUE;
        macro_expansion_result.encountered_error = TRUE;
        return macro_expansion_result;
    }
    /* initialize line_info */
    line_info.line = line_copy;
    line_info.line_num = 0;

    /* we only have one error type here - predefine error */
    error.type = ERROR_TYPE_MACRO;
    error.val.expand_macro_err = &expand_macro_err;

    /* initialize macro expansion result */
    macro_expansion_result.encountered_error = FALSE;
    macro_expansion_result.alloc_fail = FALSE;

    while (fgets(line, sizeof(line), in))
    {
        line_info.line_num++;
        if (strlen(line) == (sizeof(line) - 1) && line[sizeof(line) - 1] != '\n')
        {
            /* we're at a line which is longer than MAX_LINE_LENGTH  */
            expand_macro_err.type = EXPAND_MACRO_ERROR_LINE_TOO_LONG;
            expand_macro_err.val.is_too_long.expected_len = MAX_LINE_LENGTH;
            /* count the length of the line */
            expand_macro_err.val.is_too_long.len = sizeof(line) - 1;
            while ((c = getc(in)) != '\n' && c != EOF)
            {
                expand_macro_err.val.is_too_long.len++;
            }
            /* we duplicate this string since it may be modified by err_callback (could result in a seg fault)*/
            error.line_info.line = strdup("line is too long to be displayed");
            error.line_info.line_num = line_info.line_num;
            err(err_callback, error);
            free(error.line_info.line);
            encountered_error = TRUE;
            continue;
        }
        strcpy(line_copy, line);
        error.line_info = line_info; /* update error's line info */

        line_ptr = skip_space(line);

        if (is_in_macro)
        {
            /* we copy each line of the macro until we reach mcroend*/
            if (strncmp(line_ptr, "mcroend", 7) == 0 && *skip_space(line_ptr + 7) == 0)
            {
                is_in_macro = FALSE;
            }
            else
            {
                if (!macro_table_write(&macro_table, current_macro_name, line))
                {
                    /* we couldn't alloacte memory - return */
                    macro_expansion_result.encountered_error = TRUE;
                    macro_expansion_result.alloc_fail = TRUE;
                    return macro_expansion_result;
                }
            }
        }
        else if (strncmp(line_ptr, "mcro", 4) == 0)
        {
            /* we found a macro definition */
            is_in_macro = TRUE;
            mcro_name = skip_space(line_ptr + 4);
            trim_end(mcro_name);

            if (*mcro_name == 0)
            {
                /* error - empty macro name */
                expand_macro_err.type = EXPAND_MACRO_EXPECTED_MACRO_NAME;
                err(err_callback, error);
            }
            else if (!isalpha(*mcro_name) && *mcro_name != '_')
            {
                /* error - macro does not start with alphabethic character and does not start with _ */
                expand_macro_err.type = EXPAND_MACRO_ERROR_STARTS_WITH_INVALID_CHARACTER;
                expand_macro_err.val.starts_with_invalid_character = *mcro_name;
                err(err_callback, error);
                encountered_error = TRUE;
            }
            else if (is_an_instruction(mcro_name))
            {
                /* error - macro is an instruction */
                expand_macro_err.type = EXPAND_MACRO_ERROR_IS_AN_INSTRUCTION;
                err(err_callback, error);
                encountered_error = TRUE;
            }
            else if (is_a_directive(mcro_name))
            {
                /* error - macro is an directive */
                expand_macro_err.type = EXPAND_MACRO_ERROR_IS_A_DIRECTIVE;
                err(err_callback, error);
                encountered_error = TRUE;
            }
            else if (is_a_register(mcro_name))
            {
                /* error - macro has a name of a register */
                expand_macro_err.type = EXPAND_MACRO_ERROR_IS_A_REGISTER;
                err(err_callback, error);
                encountered_error = TRUE;
            }
            else if (strlen(mcro_name) > MAX_MACRO_NAME_LENGTH)
            {
                /* error - name is too long */
                expand_macro_err.type = EXPAND_MACRO_ERROR_NAME_IS_TOO_LONG;
                expand_macro_err.val.is_too_long.expected_len = MAX_MACRO_NAME_LENGTH;
                expand_macro_err.val.is_too_long.len = strlen(mcro_name);
                err(err_callback, error);
                encountered_error = TRUE;
            }
            else if (has_invalid_characters(mcro_name, &invalid_character, &invalid_character_pos))
            {
                /* error - macro has invalid charcaters in it */
                expand_macro_err.type = EXPAND_MACRO_ERROR_INVALID_CHARACTER;
                expand_macro_err.val.invalid_character.invalid_character = invalid_character;
                expand_macro_err.val.invalid_character.position = invalid_character_pos;
                err(err_callback, error);
                encountered_error = TRUE;
            }
            else
            {
                /* we have a macro with a valid name - update the current macro name and insert the macro into the table */
                strncpy(current_macro_name, mcro_name, MAX_MACRO_NAME_LENGTH);
                current_macro_name[MAX_MACRO_NAME_LENGTH] = 0;
                if (!macro_table_insert(&macro_table, current_macro_name))
                {
                    /* couldn't allocate memory */
                    macro_expansion_result.alloc_fail = TRUE;
                    macro_expansion_result.encountered_error = TRUE;
                    return macro_expansion_result;
                }
            }
        }
        else if ((macro = macro_table_search(&macro_table, trim_end(line_ptr))))
        {
            /* paste the macro */
            for (i = 0; i < macro->data->len; ++i)
            {
                fputc(char_vec_get(macro->data, i), out);
            }
        }
        else
        {
            /* write the line to the output file. We use line_copy instead of line since line may have been modified by the above if condition */
            fwrite(line_copy, sizeof(*line_copy), strlen(line_copy), out);
        }
    }

    /* ensure that no macro has been defined as a label */
    line_info.line_num = 0;
    fseek(in, 0, SEEK_SET);
    while (fgets(line, sizeof(line), in))
    {
        line_info.line_num++;
        strcpy(line_copy, line);
        error.line_info = line_info; /* update error's line info */

        /* check if the line is a label*/
        line_ptr = skip_space(line);
        while (*line_ptr != LABEL_END_CHAR && *line_ptr != 0)
        {
            line_ptr++;
        }
        if (*line_ptr == LABEL_END_CHAR)
        {
            /* the line is a label - check if the label has been defined as a macro */
            *line_ptr = 0;
            if ((macro = macro_table_search(&macro_table, skip_space(line))) != NULL)
            {
                /* error - the macro has been defined as a label */
                expand_macro_err.type = EXPAND_MACRO_ERROR_MACRO_DEFINED_AS_LABEL;
                expand_macro_err.val.macro_name = macro->name;
                err(err_callback, error);
                encountered_error = TRUE;
            }
        }
    }

    macro_table_free(macro_table);
    macro_expansion_result.encountered_error = encountered_error;
    return macro_expansion_result;
}