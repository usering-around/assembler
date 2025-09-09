#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char *skip_space(char *str)
{
    while (isspace((unsigned char)*str))
    {
        str++;
    }
    return str;
}

/* Removes any trailing whitespaces (as defined in isspace) of the string. */
char *trim_end(char *str)
{
    char *str_start = str;
    /* remove the edgecase of an empty string. */
    if (*str == 0)
    {
        return str;
    }

    while (*str != 0)
    {
        str++;
    }
    /* this is safe since we're at the null terminator of the string and it cannot be empty */
    str--;
    while (str >= str_start && isspace((unsigned char)*str))
    {
        str--;
    }
    /* this is safe since we cannot be at the null terminator of the string at this point */
    str++;
    *str = 0;
    return str_start;
}

char *strdup(const char *str)
{
    int len = strlen(str);
    char *out;
    if (len == 0)
    {
        return NULL;
    }
    out = malloc(len + 1); /* +1 for null termination */
    if (out == NULL)
    {
        return NULL;
    }
    memcpy(out, str, len);
    out[len] = 0;
    return out;
}