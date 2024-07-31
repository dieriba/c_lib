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
    if ((dstring = malloc(sizeof(DRealString)) == NULL))
        return NULL;
    dstring -> len = 0;
    dstring -> capacity = CAPACITY;
    if ((dstring -> string = malloc(sizeof(char) * (CAPACITY + 1))== NULL))
        return NULL;
    return (DString*)dstring;
}

DString* 	d_string_new_from_c_string(const char* str)
{
    DRealString* dstring;
    if ((dstring = malloc(sizeof(DRealString)) == NULL))
        return NULL;
    usize len = strlen(str);
    dstring -> len = len;
    dstring -> capacity = CAPACITY;
    if ((dstring -> string = malloc(sizeof(char) * (len + CAPACITY + 1))== NULL))
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
    if ((dstring = malloc(sizeof(DRealString)) == NULL))
        return NULL;
    dstring -> len = 0;
    dstring -> capacity = reserve;
    if ((dstring -> string = malloc(sizeof(char) * (reserve + 1))== NULL))
        return NULL;
    return (DString*)dstring;
}


DString* 	d_string_new_with_substring(const char* str, usize from, usize len)
{
    usize str_len = strlen(str);
    if (from > str_len)
        return NULL;
    DRealString* dstring;
    if ((dstring = malloc(sizeof(DRealString)) == NULL))
        return NULL;
    if ((dstring -> string = malloc(sizeof(char) * (len + CAPACITY + 1))== NULL))
    {
        free(dstring);
        return NULL;
    }
    memcpy(dstring -> string, str + from, len);
    dstring -> len = len;
    dstring -> string[len] = '\0';
    dstring -> capacity = CAPACITY;
    return (DString*)dstring;
}

usize		d_string_get_capacity(DString* dstring)
{
    if (dstring == NULL)
        return NULL;
    DRealString* rdstring = (DRealString*)dstring;
    return rdstring -> capacity;
}

DString* 	d_string_resize(DString* dstring, usize len)
{
    if (dstring == NULL)
        return NULL;
    DRealString* rdstring = (DRealString*)dstring;
    if (len >= rdstring -> len + rdstring -> capacity && d_string_modify_capacity(dstring, len * 2) == NULL)
        return NULL;
    rdstring -> len = len;
    rdstring -> string[len] = '\0';
    return dstring;
}

DString* 	d_string_modify_capacity(DString* dstring, usize len)
{
    if (dstring == NULL)
        return NULL;
    DRealString* rdstring = (DRealString*)dstring;
    usize capacity = rdstring -> capacity + (rdstring -> capacity % 2 == 1);
    capacity += len + (2 * (capacity)) + 1; // adding 1 for the null bytes as each string will have a null bytes at the end so stdlib function can be used on them
    rdstring -> string = realloc(rdstring -> string, rdstring -> len + rdstring -> capacity);
    if (rdstring -> string == NULL)
        return NULL;
    return dstring;
}

DString* 	d_string_push_char(DString* dstring, char c)
{
    if (dstring == NULL)
        return NULL;
    DRealString* rdstring = (DRealString*)dstring;
    if (rdstring -> capacity == 0 || rdstring -> capacity - 1 == 0)
        d_string_modify_capacity(dstring, rdstring -> len * 2);
    rdstring->string[rdstring->len++] = c;
    rdstring->string[rdstring->len] = '\0';
    --rdstring->capacity;
    return dstring;
}

DString* 	d_string_push_str_with_len(DString* dstring, const char *str_to_append, usize len)
{
    if (dstring == NULL)
        return NULL;
    DRealString* rdstring = (DRealString*)dstring;
    if (rdstring -> capacity <= len && d_string_modify_capacity(dstring, len * 2) == NULL)
        return NULL;
    memcpy(rdstring -> string + rdstring -> len, str_to_append, len);
    rdstring -> string[rdstring -> len] = '\0';
    rdstring -> len += len;
    rdstring -> capacity -= len;
    return dstring;
}

DString* 	d_string_push_c_str(DString* dstring, const char *str_to_append)
{
    return d_string_push_str_with_len(dstring, str_to_append, strlen(str_to_append));
}

DString* 	d_string_replace_from_str(DString** _dstring, const char* str)
{
    if (_dstring == NULL || *_dstring == NULL)
        return NULL;

    DRealString* dstring = *_dstring;
    usize len = strlen(str);
    usize capacity = len + CAPACITY + 1;
    
    if ((len >= dstring -> len + dstring -> capacity) && (dstring -> string = realloc(dstring -> string, len + capacity)) == NULL)
    {
        *_dstring = NULL;
        return NULL;
    }
    memcpy(dstring -> string, str, len + 1);
    return dstring;
}

DString* 	d_string_replace_from_dstring(DString** dstring, const DString* to_copy)
{
    if (dstring == NULL)
        return NULL;
    return d_string_replace_from_str(dstring, to_copy -> string);
}

int32		d_string_compare(DString* dstring1, DString* dstring2)
{
    return strcmp(dstring1 -> string, dstring2 -> string);
}

usize		d_string_find_char_from(DString* dstring, char c, usize pos)
{
    if (pos > dstring -> len)
        return MAX_SIZE_T_VALUE;
    char* base_address = dstring -> string;
    char* needle = memchr(base_address + pos, (int)c, strlen(base_address));
    return needle == NULL ? MAX_SIZE_T_VALUE : needle - base_address;
}

usize		d_string_find_char(DString* dstring, char c)
{
    return d_string_find_char_from(dstring, c, 0);
}


usize		d_string_find_str_from(DString* dstring, const char *str, usize pos)
{
    if (pos > dstring -> len)
        return MAX_SIZE_T_VALUE;
    char* s = &dstring -> string[pos];
    usize len = strlen(str);
    usize str_len = dstring -> len;
    for (usize i = 0; i < str_len; i++)
    {
        if (s[i] == str[0] && str_len - i >= len && memcmp(s + i, str, len) == 0)
            return i;
    }
    return MAX_SIZE_T_VALUE;
}

usize		d_string_find_str(DString* dstring, const char *str)
{
    return d_string_find_str_from(dstring, str, 0);
}

usize		d_string_find_by_predicate_from(DString* dstring, match_predicate fn, usize pos)
{
    if (pos > dstring -> len || fn == NULL)
        return MAX_SIZE_T_VALUE;
    char* s = &dstring -> string[pos];
    usize str_len = dstring -> len;
    for (usize i = 0; i < str_len; i++)
    {
        if (fn(s[i]) == true)
            return i;
    }
    return MAX_SIZE_T_VALUE;    
}

usize		d_string_find_by_predicate(DString* dstring, match_predicate fn)
{
    return d_string_find_by_predicate_from(dstring, fn, 0);
}

usize		d_string_rfind_char_from(DString* dstring, char c, usize pos)
{
    if (pos > dstring -> len)
        return MAX_SIZE_T_VALUE;
    char* base_address = dstring -> string;
    char* needle = memrchr(base_address + pos, (int)c, strlen(base_address));
    return needle == NULL ? MAX_SIZE_T_VALUE : needle - base_address;
}

usize		d_string_rfind_char(DString* dstring, char c)
{
    return d_string_rfind_char_from(dstring, c, 0);
}

usize		d_string_rfind_str_from(DString* dstring, const char *str, usize pos)
{
  
}

usize		d_string_rfind_str(DString* dstring, const char *str)
{
    return d_string_rfind_str_from(dstring, str, 0);
}

usize		d_string_rfind_by_predicate_from(DString* dstring, match_predicate fn, usize pos)
{
        
}

usize		d_string_rfind_by_predicate(DString* dstring, match_predicate fn)
{
    return d_string_rfind_by_predicate_from(dstring, fn, 0);
}