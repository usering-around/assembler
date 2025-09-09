/* This module consists of functions/definitions which are used in 2 or more files, do not fit in any of the modules, and do not deserve their own module */
#ifndef _MMN14_UTILS_H_
#define _MMN14_UTILS_H_
#include <limits.h>

/* the maximum size of a label */
#define MAX_LABEL_SIZE 31
/* max line length not including '\n' and null termination */
#define MAX_LINE_LENGTH 80

/* Ansi text color red */
#define ANSI_RED "\033[1;31m"
/* Ansi text color yellow */
#define ANSI_YELLOW "\033[0;93m"
/* Ansi text color cyan */
#define ANSI_CYAN "\033[1;36m"
/* Ansi return text color to normal */
#define ANSI_NORMAL "\033[0m"

/* ensure that int32 is at least 32 bits */
#if INT_MAX >= 2147483647
/* represents an integer with a width of at least 32 bits */
typedef int int32;
#else
#error "The regular int type does not have a width of at least 32 bits. Please change the type in the typedef of int32 to an integer type with at least 32 bits"
#endif

/* ensure that uint32 is at least 32 bits */
#if UINT_MAX >= 4294967295
/* represents an unsigned integer with a width of at least 32 bits */
typedef unsigned int uint32;
#else
#error "The regular unsigned integer type does not have a width of at least 32 bits. Please change the type in the typedef of uint32 to an unsigned type with at least 32 bits."
#endif

/* I believe char is guranteed to be 8 bits long but just in case */
#if UCHAR_MAX >= 255
/* represents an unsigned integer with a width of at least 8 bits */
typedef unsigned char uint8;
#else
#error "The regular unsigned char type doesn ot have a width of at least 8 bits. Please change the type in the typedef of uint8 to an unsigned type with at least 8 bits."
#endif

/**
 * @brief Skip the spaces (as defined in isspace) in a string
 * @param str The string to skip spaces in
 * @return A pointer to the same string after skipping all the spaces.
 */
char *skip_space(char *str);

/**
 * @brief Removes any trailing whitespaces (as defined in isspace) of a string
 * @param str The string to remove trailing whitespaces from
 * @return The string which was given (for convenience)
 */
char *trim_end(char *str);

/**
 * @brief Duplicate a string
 * @param str The string to duplicate
 * @return NULL if it was not possible to allocate the string, otherwise a pointer to the string. Note: you're responsible for freeing this string.
 */
char *strdup(const char *str);

#endif
