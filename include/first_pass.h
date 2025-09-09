/* This module contains the first_pass_function and its FirstPassResult object,
   which are used to make certain gurantees about an assembly file (read FirstPassResult for the exact gurantees)
   as well as to collect symbols and build data image for the object file */
#ifndef _MMN14_FIRST_PASS_H_
#define _MMN14_FIRST_PASS_H_
#include <stdio.h>
#include "bool.h"
#include "vector.h"
#include "symbol_table.h"
#include "errors.h"
#include "utils.h" /* int types */

/* The starting point of instruction memory */
#define INSTRUCTION_MEMORY_START (100)
/* The biggest possible address in memory */
#define MAX_ADDRESS ((1 << 21) - 1)

/* Result of the first pass. */
typedef struct
{
   /* Whether or not we encountered an error the pass. If this is false then it is guranteed that calling parse_line from parse.h
      will never give a result of type PARSE_LINE_ERROR or LABEL_PARSE_ERROR (meaning there are no syntax errors).
      Additionally, it is guranteed that no label is defined twice in this file.
      It does not however gurantee that a .entry directive has its symbol in the file,
      and it does not gurantee anything about symbols in instructions other than the fact that they're valid syntactically.
      Both these things are checked by the second pass. */
   bool encountered_error;
   /* whether or not we encountered an allocation failure during the pass.
      Note: the pass stops immediately when an allocation fails */
   bool alloc_fail;
   /* The symbol table the first pass built up.
      It contains the following 3 things:
      1. Labels found before instructions and the address they should have in the object file
      2. Labels found before .data and .string directives and the address they should have in the object file
      3. Symbols found in .extern directives. However, the address for these entries will be invalid (0). */
   SymbolTable symbol_table;
   /* Memory image of data from directives. Each uint32 number in the vector represents a 24-bit word.
      Note: each word is represented as a 32bit unsigned number. As such, signed numbers will not have 24-bit values (due to being represented in two's complement),
      meaning you should mask the values here to 24 bit before encoding them to a file. */
   U32Vector *data_image;
} FirstPassResult;

/*
 */

/**
 * @brief Runs a first pass on an input file. If any errors occurs during it, err_callback will be called with the appropriate error.
 * In short, this function ensures that there are no syntax and other certain errors,
   collects all the symbols from the file, and builds the data image of data in the file.
   Read the documentation of the FirstPassResult object to see the exact gurantees this function makes and what it returns.
 * @param input the file you wish to perform first_pass on. This function assumes that this is an assembly file with no extensions (e.g. macros)
 * @param err_callback a callback function which will be called each time there is an error.
 * Note: the error received by err_callback will be invalid when exiting the callback. This means that if you wish to pass
 * data from the error, you should duplicate the data first.
 * @return FirstPassResult object. Read its documentaion for more info.
 */
FirstPassResult first_pass(FILE *input, ErrorCallback err_callback);

/**
 * @brief Free dynamic memory held by a FirstPassResult object
 * @param first_pass_result the FirstPassResult object
 */
void free_first_pass_result(FirstPassResult first_pass_result);

#endif