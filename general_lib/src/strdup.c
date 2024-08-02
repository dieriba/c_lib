#include <general_lib.h>

char*   d_strdup(const char* s)
{
    return d_substr(s, 0, strlen(s));
}