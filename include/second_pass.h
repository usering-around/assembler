/* This module contains the second_pass function which is used to turn an assembly file into an .ob, .ent and .ext files after running the first_pass */

#ifndef _MMN14_SECOND_PASS_H_
#define _MMN14_SECOND_PASS_H_
#include <stdio.h>
#include "vector.h"
#include "first_pass.h"
#include "errors.h"
#include "bool.h"

/* The result of the second pass */
typedef struct SecondPassResult
{
   /* The binary image of all the instructions in the file. Each 24 bit value is represented in 32 bits, which means that negative numbers overflow
    24 bits. */
   U32Vector *instruction_image;
   /* The binary image of all the data in the file. Each 24 bit value is represented in 32 bits, which means that negative numbers overflow
    24 bits. */
   U32Vector *data_image;

   /* All the entry symbols along with the address at which they are defined.
      e.g. if instructions start at address 0,
      .entry hi
      hi: inc r3
      then 'hi' will be in the symbol vector with address 0.
      Note: this contains refrences from symbol_table, and as such, should not be used after you free the symbol table. */
   SymbolVector *entry_symbols;
   /* All the external symbols along with the address in which they are used inside an instruction.
      So, to replace the symbols with actual values in the instruction image, you simply need to put the value in the external symbol's address.
      e.g. if instructions start at address 0,
      .extern external
      add external, r1
      then 'external' will be in the symbol vector with address 1 (since it is encoded right after the instruction)
      Note: this contains refrences from symbol_table, and as such, should not be used after you free the symbol table. */
   SymbolVector *external_symbols;
   /* The symbol table the first pass built up.
      It contains the following 3 things:
      1. Labels found before instructions and the address they should have in the object file
      2. Labels found before .data and .string directives and the address they should have in the object file
      3. Symbols found in .extern directives and the address they should have in the .ext file */
   SymbolTable symbol_table;
   /* Whether or not we encountered an error. If we did NOT encounter an error, then all the above is guranteed to be valid.
      Otherwise only the parts which overlap with first_pass_result are guranteed to be valid with the same gurantee as the first_pass_result.
      (so essentially number 3 in the symbol table gurantees may not hold in this case). */
   bool encountered_error;

   /* whether or not we encountered an allocation failure during the pass.
      Note: the pass stops immediately when an allocation fails */
   bool alloc_fail;
} SecondPassResult;

/**
 * @brief Runs a second pass on an input file. If any errors occurs during it, err_callback will be called with the appropriate error.
 * In short, this function builds the instruction image for .ob file, and collects entry/external symbols for .ent and .ext files.
 * Read the documentation of the SecondPassResult object to see the exact gurantees this function makes and what it returns.
 * @param input The assembly file to perform second_pass on. This function assumes that this is an assembly file with no extensions (e.g. macros)
 * @param first_pass_result The result from the first pass. This function should only be called after the first_pass or something equivalent returned a FirstPassResult object
 * @param err_callback The callback to call each time there is an error
 * @return SecondPassResult object. See its documentation for more information.
 */
SecondPassResult second_pass(FILE *input, FirstPassResult first_pass_result, ErrorCallback err_callback);

/**
 * @brief Free all dynamic data structures which are held by the SecondPassResult object.
 * Note: there is no need to free the FirstPassResult object which was given to second_pass.
 * @param second_pass_result the SecondPassResult object
 */
void free_second_pass_result(SecondPassResult second_pass_result);
#endif