#include "dstring.h"
#include <string.h>
#include <stdlib.h>

#define CAPACITY 8

typedef struct _DRealString DRealString;

//Real String Interface
struct _DRealString {
    char    *string;
    usize     len;
    usize     capacity;
};


DString* d_string_new(void)
{
    DRealString* dstring;
    if ((dstring = malloc(sizeof(DRealString))) == NULL)
        return NULL;
    dstring -> len = 0;
    dstring -> capacity = CAPACITY;
    if ((dstring -> string = malloc(sizeof(char) * (CAPACITY + 1))) == NULL)
        return NULL;
    return (DString*)dstring;
}

DString* 	d_string_new_from_c_string(const char* str)
{
    DRealString* dstring;
    if ((dstring = malloc(sizeof(DRealString))) == NULL)
        return NULL;
    usize len = strlen(str);
    dstring -> len = len;
    dstring -> capacity = CAPACITY;
    if ((dstring -> string = malloc(sizeof(char) * (len + CAPACITY + 1))) == NULL)
        return NULL;
    memcpy(dstring -> string, str, len + 1);
    return (DString*)dstring;
}

DString* 	d_string_new_from_dstring(DString* dstring)
{
    if (dstring == NULL)
        return d_string_new();
    return d_string_new_from_c_string(dstring->string);
}

DString* 	d_string_new_with_reserve(usize reserve)
{
    DRealString* dstring;
    if ((dstring = malloc(sizeof(DRealString))) == NULL)
        return NULL;
    dstring -> len = 0;
    dstring -> capacity = reserve;
    if ((dstring -> string = malloc(sizeof(char) * (reserve + 1))) == NULL)
        return NULL;
    return (DString*)dstring;
}


DString* 	d_string_new_with_substring(const char* str, usize pos, usize len)
{
    usize str_len;
    if (str == NULL)
        return d_string_new();
    else if (pos > (str_len = strlen(str)))
        return NULL;
    len = len > str_len ? 
            str_len - pos : pos + len > str_len //if len > str_len set let to str_len - pos so it has the correct num of char
            ?
            str_len - pos : len; // if len < str_len then check if pos + len exceed str boundary if so then len = str - pos
    DRealString* dstring;
    if ((dstring = malloc(sizeof(DRealString))) == NULL)
        return NULL;
    if ((dstring -> string = malloc(sizeof(char) * (len + CAPACITY + 1))) == NULL)
    {
        free(dstring);
        return NULL;
    }
    if (len != 0)
        memcpy(dstring -> string, str + pos, len);
    dstring -> len = len;
    dstring -> string[len] = '\0';
    dstring -> capacity = CAPACITY;
    return (DString*)dstring;
}

usize		d_string_get_capacity(DString* dstring)
{
    DRealString* rdstring = (DRealString*)dstring;
    return rdstring -> capacity;
}

DString* 	d_string_resize(DString* dstring, usize len)
{
    DRealString* rdstring = (DRealString*)dstring;
    if (len >= rdstring -> len + rdstring -> capacity && d_string_modify_capacity(dstring, len * 2) == NULL)
        return NULL;
    rdstring -> len = len;
    rdstring -> string[len] = '\0';
    return dstring;
}

DString* 	d_string_modify_capacity(DString* dstring, usize new_capacity)
{
    DRealString* rdstring = (DRealString*)dstring;
    if ((rdstring -> string = realloc(rdstring -> string, rdstring -> len + new_capacity + 1)) == NULL)
        return NULL;
    rdstring -> capacity = new_capacity;
    return dstring;
}

DString* 	d_string_push_char(DString* dstring, char c)
{
    DRealString* rdstring = (DRealString*)dstring;
    if (rdstring -> capacity <= 1 && d_string_modify_capacity(dstring, rdstring -> len * 2) == NULL)
        return NULL;
    rdstring->string[rdstring->len++] = c;
    rdstring->string[rdstring->len] = '\0';
    --rdstring->capacity;
    return dstring;
}

DString* 	d_string_push_str_with_len(DString* dstring, const char *str_to_append, usize len)
{
    DRealString* rdstring = (DRealString*)dstring;
    if (len >= rdstring -> capacity && d_string_modify_capacity(dstring, len * 2) == NULL)
        return NULL;
    memcpy(rdstring -> string + rdstring -> len, str_to_append, len);
    rdstring -> len += len;
    rdstring -> string[rdstring -> len] = '\0';
    rdstring -> capacity -= len;
    return dstring;
}

DString* 	d_string_push_c_str(DString* dstring, const char *str_to_append)
{
    return d_string_push_str_with_len(dstring, str_to_append, strlen(str_to_append));
}

DString* 	d_string_replace_from_str(DString** dstring, const char* str)
{
    DRealString* rdstring = (DRealString*)*dstring;
    usize len = str == NULL ? 0 : strlen(str);
    usize capacity = len + CAPACITY + 1;
    
    if ((len >= rdstring -> len + rdstring -> capacity) && (rdstring -> string = realloc(rdstring -> string, len + capacity)) == NULL)
    {
        d_string_destroy(dstring);
        return NULL;
    }
    if (len != 0)
        memcpy(rdstring -> string, str, len);
    rdstring -> len = len;
    rdstring -> string[len] = 0;
    return *dstring;
}

DString* 	d_string_replace_from_dstring(DString** dstring, const DString* to_copy)
{
    return d_string_replace_from_str(dstring, to_copy -> string);
}

int32		d_string_compare(DString* dstring1, DString* dstring2)
{
    return strcmp(dstring1 -> string, dstring2 -> string);
}

usize		d_string_find_first_not_of_from(DString* dstring, char c, usize pos)
{
    usize str_len;
    if (pos >= (str_len = dstring -> len))
        return MAX_SIZE_T_VALUE;
    char *str = dstring -> string;
    for (size_t i = pos; i < str_len; i++)
    {
        if (str[i] != c)
            return i;
    }
    return MAX_SIZE_T_VALUE;
}

usize		d_string_find_first_not_of(DString* dstring, char c)
{
    return d_string_find_first_not_of_from(dstring, c, 0);
}

usize		d_string_find_first_not_of_str_from(DString* dstring, char* str, usize pos)
{
    usize dstring_len;
    if (pos >= (dstring_len = dstring -> len))
        return MAX_SIZE_T_VALUE;
    char *dstring_str = dstring -> string;
    usize str_len = strlen(str);
    for (size_t i = pos; i < dstring_len; i++)
    {
        if (memchr(str, (int)dstring_str[i], str_len) == NULL)
            return i;
    }
    return MAX_SIZE_T_VALUE;
}

usize		d_string_find_first_not_of_str(DString* dstring, char* str)
{
    return d_string_find_first_not_of_str_from(dstring, str, 0);
}

usize		d_string_rfind_first_not_of_from(DString* dstring, char c, usize pos)
{
    pos = pos >= dstring -> len ? dstring -> len - 1 : pos;
    char *str = dstring -> string;
    while (1)
    {
        if (str[pos] != c)
            return pos;
        if (pos == 0)
            break;
        --pos;
    }
    return MAX_SIZE_T_VALUE;
}

usize		d_string_rfind_first_not_of(DString* dstring, char c)
{
    if (dstring -> len == 0)
        return MAX_SIZE_T_VALUE;
    return d_string_rfind_first_not_of_from(dstring, c, dstring -> len - 1);
}

usize		d_string_rfind_first_not_of_str_from(DString* dstring, char* str, usize pos)
{
    pos = pos >= dstring -> len ? dstring -> len - 1 : pos;
    char *dstring_str = dstring -> string;
    usize str_len = strlen(str);
    while (1)
    {
        if (memchr(str, (int)dstring_str[pos], str_len) == NULL)
            return pos;
        if (pos == 0)
            break;
        --pos;
    }
    return MAX_SIZE_T_VALUE;
}

usize		d_string_rfind_first_not_of_str(DString* dstring, char* str)
{
    if (dstring -> len == 0)
        return MAX_SIZE_T_VALUE;
    return d_string_rfind_first_not_of_str_from(dstring, str, dstring -> len - 1);
}

usize		d_string_find_char_from(DString* dstring, char c, usize pos)
{
    if (pos > dstring -> len)
        return MAX_SIZE_T_VALUE;
    char* base_address = dstring -> string;
    char* needle = memchr(base_address + pos, (int)c, strlen(base_address));
    return needle == NULL ? MAX_SIZE_T_VALUE : (usize)(needle - base_address);
}

usize		d_string_find_char(DString* dstring, char c)
{
    return d_string_find_char_from(dstring, c, 0);
}

usize		d_string_find_str_from(DString* dstring, const char *str, usize pos)
{
    if (pos > dstring -> len)
        return MAX_SIZE_T_VALUE;
    char* string = dstring -> string;
    usize len = strlen(str);
    usize string_len = dstring -> len;
    for (usize i = pos; i < string_len; ++i)
    {
        if (string[i] == str[0] && string_len - i >= len && memcmp(string + i, str, len) == 0)
            return (string + i) - string;
    }
    return MAX_SIZE_T_VALUE;
}

usize		d_string_find_str(DString* dstring, const char *str)
{
    return d_string_find_str_from(dstring, str, 0);
}

usize		d_string_find_by_predicate_from(DString* dstring, match fn, usize pos)
{
    if (pos > dstring -> len)
        return MAX_SIZE_T_VALUE;
    char* string = dstring -> string;
    usize string_len = dstring -> len;
    for (usize i = pos; i < string_len; ++i)
    {
        if (fn(string[i]) != 0)
            return (string + i) - string;
    }
    return MAX_SIZE_T_VALUE;    
}

usize		d_string_find_by_predicate(DString* dstring, match fn)
{
    return d_string_find_by_predicate_from(dstring, fn, 0);
}

usize		d_string_rfind_char_from(DString* dstring, char c, usize pos)
{
    if (pos > dstring -> len)
        return MAX_SIZE_T_VALUE;
    usize i = pos;
    char* s = dstring -> string;
    while (1)
    {
        if (s[i] == c)
            return i;
        else if (i == 0)
            break;
        --i;
    }

    return MAX_SIZE_T_VALUE;
}

usize		d_string_rfind_char(DString* dstring, char c)
{
    return d_string_rfind_char_from(dstring, c, dstring -> len);
}

usize		d_string_rfind_str_from(DString* dstring, const char *str, usize pos)
{
    if (pos > dstring -> len)
        return MAX_SIZE_T_VALUE;
    usize string_len = dstring -> len;
    char* string = dstring -> string;
    usize str_len = strlen(str);
    usize i = pos;
    while (1)
    {
        if (string[i] == str[0] && string_len - i >= str_len && memcmp(string + i, str, str_len) == 0)
            return (string + i) - string;
        else if (i == 0)
            break;
        --i;
    }
    return MAX_SIZE_T_VALUE;
}

usize		d_string_rfind_str(DString* dstring, const char *str)
{
    return d_string_rfind_str_from(dstring, str, dstring -> len);
}

usize		d_string_rfind_by_predicate_from(DString* dstring, match fn, usize pos)
{
    if (pos > dstring -> len)
        return MAX_SIZE_T_VALUE;
    char* string = dstring -> string;
    usize i = pos;
    while (1)
    {
        if (fn(string[i]) != 0)
            return (string + i) - string;
        else if (i == 0)
            break;
        --i;
    }
    return MAX_SIZE_T_VALUE;  
}

usize		d_string_rfind_by_predicate(DString* dstring, match fn)
{
    return d_string_rfind_by_predicate_from(dstring, fn, dstring -> len - 1);
}

DString*	d_string_sub_string_in_place(DString* dstring, usize pos, usize len)
{
    if (pos > dstring -> len)
        return NULL;
    DRealString* rdstring = (DRealString*)dstring;
    usize string_len = rdstring -> len;
    if (pos + len > dstring -> len)
        len = string_len - pos;
    char* string = dstring -> string;
    memmove(string, string + pos, len);
    rdstring -> string[len] = 0;
    rdstring -> capacity += string_len - len;
    rdstring -> len = len;
    return dstring;
}


DString*	d_string_trim_left_by_char_in_place(DString* dstring, char c)
{
    usize i = d_string_find_first_not_of(dstring, c);
    if (i == MAX_SIZE_T_VALUE)
        return dstring;
    return d_string_sub_string_in_place(dstring, i, dstring -> len);
}

DString*	d_string_trim_left_by_predicate_in_place(DString* dstring, match fn)
{
    usize len = dstring -> len;
    char* str = dstring -> string;
    usize i = 0;
    for (; i < len; ++i)
    {
        if (fn(str[i]) == 0)
            break;
    }
    if (i == len)
        return dstring;
    return d_string_sub_string_in_place(dstring, i, dstring -> len);
}

DString*	d_string_trim_left_by_char_new(DString* dstring, char c)
{
    usize i = d_string_find_first_not_of(dstring, c);
    i = i == MAX_SIZE_T_VALUE ? 0 : i;
    return d_string_new_with_substring(dstring -> string, i, dstring -> len);
}

DString*	d_string_trim_left_by_predicate_new(DString* dstring, match fn)
{
    usize len = dstring -> len;
    char* str = dstring -> string;
    usize i = 0;
    for (; i < len; ++i)
    {
        if (fn(str[i]) == 0)
            break;
    }
    if (i == len)
        return dstring;
    return d_string_new_with_substring(dstring -> string, i, dstring -> len);
}

DString*	d_string_trim_right_by_char_in_place(DString* dstring, char c)
{
    usize i = d_string_rfind_first_not_of(dstring, c);
    if (i == MAX_SIZE_T_VALUE)
        return dstring;
    return d_string_sub_string_in_place(dstring, 0, i + 1);
}

DString*	d_string_trim_right_by_predicate_in_place(DString* dstring, match fn)
{
    usize i;
    if ((i = dstring -> len) == 0)
        return dstring;
    --i;
    char* str = dstring -> string;
    while (1)
    {
        if (fn(str[i]) == 0)
            break;
        else if (i == 0)
            return d_string_sub_string_in_place(dstring, 0, 0);
        --i;
    }
    return d_string_sub_string_in_place(dstring, 0, i + 1);
}

DString*	d_string_trim_right_by_char_new(DString* dstring, char c)
{
    usize i = d_string_rfind_first_not_of(dstring, c);
    i = i == MAX_SIZE_T_VALUE ? dstring -> len : i;
    return d_string_new_with_substring(dstring -> string, 0, i + 1);
}

DString*	d_string_trim_right_by_predicate_new(DString* dstring, match fn)
{
    usize i;
    if ((i = dstring -> len) == 0)
        return dstring;
    --i;
    char* str = dstring -> string;
    while (1)
    {
        if (fn(str[i]) == 0)
            break;
        else if (i == 0)
            return d_string_new_with_substring(dstring -> string, 0, 0);
        --i;
    }
    return d_string_new_with_substring(dstring -> string, 0, i + 1);
}

void		d_string_destroy(DString** dstring)
{
    DRealString* rdstring = ((DRealString*)*dstring);
    free(rdstring -> string);
    free(rdstring);
    *dstring = NULL;
}