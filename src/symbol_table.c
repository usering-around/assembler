#include <string.h>
#include "symbol_table.h"

VECTOR_IMPL(Symbol, SymbolVector, symbol)

bool symbol_table_init(SymbolTable *symbol_table)
{
    symbol_table->inner = symbol_vec_create();
    if (symbol_table->inner == NULL)
    {
        return FALSE;
    }
    return TRUE;
}

void symbol_table_free(SymbolTable symbol_table)
{
    uint32 i;
    /* first we free each name inside the symbol table */
    for (i = 0; i < symbol_table.inner->len; ++i)
    {
        Symbol symbol = symbol_vec_get(symbol_table.inner, i);
        free(symbol.name);
    }

    symbol_vec_free(symbol_table.inner);
}

/* Searches the SymbolTable via a simple linear search */
Symbol *symbol_table_search(SymbolTable symbol_table, char *symbol_name)
{
    uint32 i;
    SymbolVector *vec = symbol_table.inner;

    for (i = 0; i < vec->len; ++i)
    {
        Symbol *symbol = symbol_vec_get_ptr(vec, i);
        if (strcmp(symbol->name, symbol_name) == 0)
        {
            return symbol;
        }
    }
    return NULL;
}

bool symbol_table_insert(SymbolTable symbol_table, const char *symbol_name, uint32 addr, SymbolContext ctx, int line_num)
{
    Symbol symbol;
    symbol.name = strdup(symbol_name);
    if (symbol.name == NULL)
    {
        return FALSE;
    }
    symbol.context = ctx;
    symbol.addr = addr;
    symbol.line = line_num;
    return symbol_vec_push(symbol_table.inner, symbol);
}

SymbolTableIterator symbol_table_iter(SymbolTable symbol_table)
{
    SymbolTableIterator iter;
    iter.table = symbol_table;
    iter.position = 0;
    return iter;
}

Symbol *symbol_table_iter_next(SymbolTableIterator *iter)
{
    Symbol *symbol;
    if (iter->position >= iter->table.inner->len)
    {
        return NULL;
    }
    symbol = symbol_vec_get_ptr(iter->table.inner, iter->position);
    iter->position++;
    return symbol;
}