#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "macros.h"
#include "first_pass.h"
#include "second_pass.h"
#include "errors.h"
#include "utils.h"

/* trunecate x to 24 bits by making any bits above bit 23 equal 0 */
#define TO_24_BITS(x) ((x) & 0xffffff)

/* Exit code for an allocation failure */
#define ALLOC_ERROR_EXIT_CODE 1

/* Exit code for when the user calls this binary in a wrong manner  */
#define BAD_USAGE_EXIT_CODE 2

/* the biggest length out of all the file extensions we create */
#define MAX_FILE_EXTENSION_LENGTH 3

/* Our error_callback function which gets called each time there is an error in the assembly file.
   prints an error with nice colors in the format:
   filename: error\n\n
   */
void error_callback(Error error, void *filename)
{
    char buf[ERROR_TO_STRING_BUF_SIZE_UPPER_BOUND];
    error_to_string(error, buf);
    printf("%s%s:%s ", ANSI_CYAN, (char *)filename, ANSI_NORMAL);
    printf("%s\n\n", buf);
}

/* Write each symbol in a SymbolVector to a file with name filename in the format:
   symbol address
   returns TRUE if it successfully opened and wrote to the file, FALSE otherwise */
bool symbol_vec_write_to_file(char *filename, SymbolVector *vec)
{
    FILE *file;
    uint32 i;
    Symbol *symbol;
    if ((file = fopen(filename, "w")) == NULL)
    {
        return FALSE;
    }
    else
    {
        for (i = 0; i < vec->len; ++i)
        {
            symbol = symbol_vec_get_ptr(vec, i);
            fprintf(file, "%s %07u\n", symbol->name, symbol->addr);
        }
    }
    fclose(file);
    return TRUE;
}

void exit_due_to_alloc_failure()
{
    printf("exiting early due to an allocation failure");
    exit(ALLOC_ERROR_EXIT_CODE);
}

int main(int argc, char **argv)
{
    char *filename_base; /* the base of the filename (e.g. it would be "example" for "example.asm")*/
    char *filename;      /* actual filename with an extension */
    FILE *input_file,
        *macro_expand_out, *ob_file; /* .as file, .am file, .ob file */
    MacroExpansionResult macro_expansion_result;
    FirstPassResult first_pass_result;
    SecondPassResult second_pass_result;
    uint32 IC, word, DC;
    ErrorCallback err_callback;
    int i;

    if (argc == 1)
    {
        printf("usage: assembler [file1] [file2] [file3] ...\nNote: files should be without extension, i.e. you should enter \"file\" instead of \"file.as\"\n");
        return BAD_USAGE_EXIT_CODE;
    }
    for (i = 1; i < argc; ++i)
    {
        /* get the base filename from command line args*/
        filename_base = argv[i];

        /* allocate enough memory for filename - +1 for null termination */
        filename = malloc(strlen(filename_base) + MAX_FILE_EXTENSION_LENGTH + 1);
        if (filename == NULL)
        {
            exit_due_to_alloc_failure();
        }

        /* initialize our print error callback */
        err_callback.callback = error_callback;
        err_callback.data = filename;

        /* open the .as file for reading */
        sprintf(filename, "%s.as", filename_base);
        if ((input_file = fopen(filename, "r")) == NULL)
        {
            printf("error: could not open file %s for reading\n", filename);
            free(filename);
            continue;
        }
        /* open the .am file for reading & writing*/
        filename[0] = 0;
        sprintf(filename, "%s.am", filename_base);
        if ((macro_expand_out = fopen(filename, "w+")) == NULL)
        {
            printf("error: could not open file %s for write & read\n", filename);
            fclose(input_file);
            free(filename);
            continue;
        }
        printf("assembling %s\n", filename_base);
        /* expand macros */
        macro_expansion_result = expand_macros(input_file, macro_expand_out, err_callback);
        if (macro_expansion_result.encountered_error)
        {
            /* we have errors in the expand macro stage, delete the .am file */
            fclose(macro_expand_out);
            remove(filename);

            if (macro_expansion_result.alloc_fail)
            {
                free(filename);
                exit_due_to_alloc_failure();
            }
            printf("%s: macro expansion failed; moving to next file\n", filename);
            free(filename);

            continue;
        }
        /* we no longer need the input file */
        fclose(input_file);

        /* read the .am file from the start and run first_pass on it */
        fseek(macro_expand_out, 0, SEEK_SET);
        first_pass_result = first_pass(macro_expand_out, err_callback);
        if (first_pass_result.encountered_error)
        {

            if (first_pass_result.alloc_fail)
            {
                free_first_pass_result(first_pass_result);
                fclose(macro_expand_out);
                free(filename);
                exit_due_to_alloc_failure();
            }
            else
            {
                /* run the second pass to obtain more errors */
                fseek(macro_expand_out, 0, SEEK_SET);
                second_pass_result = second_pass(macro_expand_out, first_pass_result, err_callback);
                free_second_pass_result(second_pass_result);
                fclose(macro_expand_out);
                if (second_pass_result.alloc_fail)
                {
                    free(filename);
                    exit_due_to_alloc_failure();
                }
            }

            printf("%s: first pass failed; moving to next file\n", filename);
            free(filename);
            continue;
        }

        /* read the .am file from the start and run second_pass on it  */
        fseek(macro_expand_out, 0, SEEK_SET);
        second_pass_result = second_pass(macro_expand_out, first_pass_result, err_callback);
        if (second_pass_result.encountered_error)
        {
            free_second_pass_result(second_pass_result);
            fclose(macro_expand_out);

            if (second_pass_result.alloc_fail)
            {
                free(filename);
                exit_due_to_alloc_failure();
            }
            printf("%s: second pass failed; moving to next file\n", filename);
            free(filename);
            continue;
        }
        fclose(macro_expand_out);

        /* now there were no errors and we're in position to create the files!
           we first create the object file (if necessary) */
        if (second_pass_result.instruction_image->len > 0 || second_pass_result.data_image->len > 0)
        {
            filename[0] = 0;
            sprintf(filename, "%s.ob", filename_base);
            if ((ob_file = fopen(filename, "w")) == NULL)
            {
                printf("error: could not open file %s for writing\n", filename);
            }
            else
            {
                /* first line of the object file is the length of the instruction image and data image*/
                fprintf(ob_file, "%7d %d\n", second_pass_result.instruction_image->len, second_pass_result.data_image->len);
                /* write the instruction image */
                for (IC = INSTRUCTION_MEMORY_START; IC < second_pass_result.instruction_image->len + INSTRUCTION_MEMORY_START; ++IC)
                {
                    word = u32_vec_get(second_pass_result.instruction_image, IC - INSTRUCTION_MEMORY_START);
                    fprintf(ob_file, "%07d %06x\n", IC, TO_24_BITS(word));
                }
                /* write the data imgage */
                for (DC = IC; DC < IC + second_pass_result.data_image->len; ++DC)
                {
                    word = u32_vec_get(second_pass_result.data_image, DC - IC);
                    fprintf(ob_file, "%07d %06x\n", DC, TO_24_BITS(word));
                }
                fclose(ob_file);
            }
        }

        /* create .ent file if necessary*/
        if (second_pass_result.entry_symbols->len > 0)
        {
            filename[0] = 0;
            sprintf(filename, "%s.ent", filename_base);
            if (!symbol_vec_write_to_file(filename, second_pass_result.entry_symbols))
            {
                printf("error: could not open file %s for writing\n", filename);
            }
        }
        /* create .ext file if necessary */
        if (second_pass_result.external_symbols->len > 0)
        {
            filename[0] = 0;
            sprintf(filename, "%s.ext", filename_base);
            if (!symbol_vec_write_to_file(filename, second_pass_result.external_symbols))
            {
                printf("error: could not open file %s for writing\n", filename);
            }
        }

        free_second_pass_result(second_pass_result);
        free(filename);

        printf("assembled %s successfully\n", filename_base);
    }
    printf("assembler done; exiting\n");
    return 0;
}