#include <general_lib.h>

#include <stdio.h>

char*   d_substr(const char* str, usize pos, usize len)
{
    usize str_len;
    if (str == NULL || pos >= (str_len = strlen(str)))
        return NULL;
    len = len > str_len ? str_len - pos : pos + len > str_len ? str_len - pos : len;
    char* sub_str = malloc(sizeof(char) * (len + 1));
    if (sub_str == NULL)
        return NULL;
    if (len != 0)
        memcpy(sub_str, str + pos, len);
    sub_str[len] = '\0';
    return sub_str;
}