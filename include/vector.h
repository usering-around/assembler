/* This module consists of a dynamic array data structure called Vector. It includes a useful macro to generate code for a vector for any type.
   To create a vector for some type, use
   VECTOR_HEADER(type, vec_type_name, prefix)
   in this header file, and in vector.c use:
   VECTOR_IMPL(type, vec_type_name, prefix)

   For user created types (i.e. structs/typedefs) do the same thing but in your own module
   (e.g. in a module for an object called someObject, in someObject.h use VECTOR_HEADER and in someObject.c use VECTOR_IMPL)

   The header and implementation macros are separated to prevent code duplication. */
#ifndef _MMN14_VECTOR_H_
#define _MMN14_VECTOR_H_

#include <malloc.h>
#include <assert.h>
#include "bool.h"
#include "utils.h" /* for int types */

/* implement the vector container header (dynamic array) for some type.
   Note: this only implements the header, meaning you should use IMPL_VECTOR with the same parameters in the implementation file to
   fully implement the vector for the type.
   type - type to implement it for,
   vec_type_name - the name of the vector type,
   prefix - the prefix to the vector functions
   Read the macro's body for documentation of available methods. */
#define VECTOR_HEADER(type, vec_type_name, prefix)                                                                             \
    /* Len is the length of the vector. Consider the other two fields as private fields. */                                    \
    typedef struct                                                                                                             \
    {                                                                                                                          \
        type *array;                                                                                                           \
        uint32 len;                                                                                                            \
        uint32 capacity;                                                                                                       \
    } vec_type_name;                                                                                                           \
    /* Create a new vector.                                                                                                    \
     Returns a pointer to the newly allocated vector if successfull, and NULL if the allocation failed.  */                    \
    vec_type_name *prefix##_vec_create();                                                                                      \
    /* Push an item into the last place of the vector. A push may be unsuccessfull if allocation for more dynamic memory fail. \
       Returns TRUE if the push was successfull, FALSE otherwise.   */                                                         \
    bool prefix##_vec_push(vec_type_name *vec, type item);                                                                     \
    /* Free the vector. The vector should not be used after calling this.                                                      \
       Note: if the vector's items are pointers to allocated objects, it is your responsibility to free them. */               \
    void prefix##_vec_free(vec_type_name *vec);                                                                                \
    /* Get a copy of a value which is in a certain position in the vector */                                                   \
    type prefix##_vec_get(const vec_type_name *vec, uint32 position);                                                          \
    /* Get a pointer to a value which is in a certain position in the vector */                                                \
    type *prefix##_vec_get_ptr(const vec_type_name *vec, uint32 position);

/* Implement the vector for some type. Do not use in a HEADER file, as that will result in multiple definitions of the same functions.
   The implementation is fairly straightforward: it's basically an array, except once it reaches its capacity, it grows its size by 2. */
#define VECTOR_IMPL(type, vec_type_name, prefix)                                             \
    vec_type_name *prefix##_vec_create()                                                     \
    {                                                                                        \
        vec_type_name *vec = calloc(1, sizeof(vec_type_name));                               \
        return vec;                                                                          \
    }                                                                                        \
                                                                                             \
    bool prefix##_vec_push(vec_type_name *vec, type item)                                    \
    {                                                                                        \
        type *new_alloc;                                                                     \
        if (vec->capacity == 0)                                                              \
        {                                                                                    \
            vec->array = malloc(sizeof(type));                                               \
            if (vec->array == NULL)                                                          \
            {                                                                                \
                return FALSE;                                                                \
            }                                                                                \
            vec->capacity = 1;                                                               \
        }                                                                                    \
        else if (vec->capacity == vec->len)                                                  \
        {                                                                                    \
            /* increase the size of the allocation by 2 once the len reaches the capacity */ \
            new_alloc = realloc(vec->array, sizeof(type) * vec->len * 2);                    \
            if (new_alloc != NULL)                                                           \
            {                                                                                \
                vec->array = new_alloc;                                                      \
                vec->capacity = vec->len * 2;                                                \
            }                                                                                \
            else                                                                             \
            {                                                                                \
                return FALSE;                                                                \
            }                                                                                \
        }                                                                                    \
        vec->array[vec->len] = item;                                                         \
        vec->len++;                                                                          \
        return TRUE;                                                                         \
    }                                                                                        \
                                                                                             \
    void prefix##_vec_free(vec_type_name *vec)                                               \
    {                                                                                        \
        free(vec->array);                                                                    \
        free(vec);                                                                           \
    }                                                                                        \
                                                                                             \
    type prefix##_vec_get(const vec_type_name *vec, uint32 position)                         \
    {                                                                                        \
        assert(position < vec->len);                                                         \
        return vec->array[position];                                                         \
    }                                                                                        \
                                                                                             \
    type *prefix##_vec_get_ptr(const vec_type_name *vec, uint32 position)                    \
    {                                                                                        \
        assert(position < vec->len);                                                         \
        return &vec->array[position];                                                        \
    }

VECTOR_HEADER(uint32, U32Vector, u32)
VECTOR_HEADER(char, CharVector, char)
#endif
