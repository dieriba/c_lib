#include "d_general_lib.h"

char *d_strdup(const char *s)
{
    if (s == NULL)
        return NULL;
    return d_substr(s, 0, strlen(s));
}