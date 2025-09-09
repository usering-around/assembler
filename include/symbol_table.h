/* This module contains the Symbol and SymbolTable objects (and related methods) which can be used to store symbols of an assembly file */

#ifndef _MMN14_SYMBOL_TABLE_H_
#define _MMN14_SYMBOL_TABLE_H_
#include "vector.h"
#include "utils.h"

/* The context in which the symbol was defined */
typedef enum
{
   /* The symbol is defined in code context, i.e. label before an instruction*/
   SYMBOL_CONTEXT_CODE,
   /* The symbol is defined in data context, i.e. label before a directive */
   SYMBOL_CONTEXT_DATA,
   /* The symbol is defined in external context, i.e. in a .extern directive */
   SYMBOL_CONTEXT_EXTERNAL
} SymbolContext;

/* Represents a Symbol in the SymbolTable */
typedef struct Symbol
{
   /* the address of the symbol in memory*/
   uint32 addr;
   /* Context in which a symbol was defined */
   SymbolContext context;
   /* The name of the symbol. Do not modify. */
   char *name;
   /* The line in which the symbol was defined. Used for error messages */
   int line;
} Symbol;

VECTOR_HEADER(Symbol, SymbolVector, symbol)

/*  This type represents a map between a symbol's name to itself. Read Symbol struct above and the methods below.
    Note: the symbol table acts as a pointer, meaning it is fine to return it by value. */
typedef struct
{
   SymbolVector *inner;
} SymbolTable;

/* This type represents an iterator over a SymbolTable. Read symbol_table_iter for more info.
   Note: This object should be considered invalid the moment you use any modifying method (e.g. symbol_table_insert)
   on the underlying SymbolTable object. It is fine however to modify the symbols while iterating. */
typedef struct symbol_table_iterator
{
   SymbolTable table;
   uint32 position;
} SymbolTableIterator;

/**
 * @brief Attempt to initialize a symbol table
 * @param symbol_table out parameter - a pointer a SymbolTable to initialize.
 * Note: Free the symbol table after you're done using it.
 * @return TRUE if the initialization was successful, FALSE otherwise. Initialization will fail if allocation of memory fail.
 */
bool symbol_table_init(SymbolTable *symbol_table);

/**
 * @brief Free any dynamic memory a SymbolTable object is holding
 * @param symbol_table the SymbolTable to free
 */
void symbol_table_free(SymbolTable symbol_table);

/**
 * @brief Search for a symbol in SymbolTable
 * @param symbol_table the SymbolTable to search
 * @param symbol the symbol's name
 * @return A pointer to the Symbol object if found, NULL otherwise.
 */
Symbol *symbol_table_search(SymbolTable symbol_table, char *symbol);

/**
 * @brief Attempt to insert a symbol into the SymbolTable
 * @param symbol_table The SymbolTable object to insert the symbol to
 * @param symbol_name The name of the symbol. Note: the name will be copied
 * @param addr The address of the symbol
 * @param region The region of the symbol
 * @param line_num The number in which the symbol was defined
 * @return TRUE if the insertion was successful, FALSE otherwise. Insertion will fail if allocation of memory fails.
 */
bool symbol_table_insert(SymbolTable symbol_table, const char *symbol_name, uint32 addr, SymbolContext ctx, int line_num);

/**
 * @brief Iterate over all of the symbols in the symbol table
 * @param symbol_table The symbol table to iterate on
 * @return SymbolTableIterator object. Use symbol_table_iter_next to on this object until it is null to iterate.
 */
SymbolTableIterator symbol_table_iter(SymbolTable symbol_table);

/**
 * @brief Iterate once on a SymbolTableIterator object.
 * @param iter The SymbolTableIterator object. Note: ensure that this iterator is valid. See SymbolTableIterator for more information.
 * @return NULL if this iterator already iterated over all of the values in the SymbolTable, otherwise a pointer to the next symbol in the iteration
 */
Symbol *symbol_table_iter_next(SymbolTableIterator *iter);

#endif
