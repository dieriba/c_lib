#include "d_general_lib.h"
char *d_substr(const char *str, usize pos, usize size)
{
    usize str_len;
    if (pos > (str_len = strlen(str)))
        return NULL;

    usize cpyable_char = str_len - pos;
    if (size > cpyable_char)
        size = cpyable_char;
    char *sub_str = malloc(sizeof(char) * (size + 1));
    if (sub_str == NULL)
        return NULL;
    if (size != 0)
        memcpy(sub_str, str + pos, size);
    sub_str[size] = '\0';
    return sub_str;
}