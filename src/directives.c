#include <string.h>
#include "directives.h"

bool str_to_directive_type(const char *str, DirectiveType *type)
{
    if (strncmp("data", str, 4) == 0)
    {
        *type = DIRECTIVE_DATA;
    }
    else if (strncmp("string", str, 6) == 0)
    {
        *type = DIRECTIVE_STRING;
    }
    else if (strncmp("entry", str, 5) == 0)
    {
        *type = DIRECTIVE_ENTRY;
    }
    else if (strncmp("extern", str, 6) == 0)
    {
        *type = DIRECTIVE_EXTERN;
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

bool is_a_directive(const char *str)
{
    DirectiveType dummy;
    if (str_to_directive_type(str, &dummy) && (*(str + directive_name_len(dummy)) == 0))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

uint32 directive_name_len(DirectiveType type)
{
    switch (type)
    {
    case DIRECTIVE_DATA:
    {
        return 4;
    }

    case DIRECTIVE_ENTRY:
    {
        return 5;
    }

    case DIRECTIVE_EXTERN:
    case DIRECTIVE_STRING:
    {
        return 6;
    }
    }

    return 0; /* should be unreachable*/
}