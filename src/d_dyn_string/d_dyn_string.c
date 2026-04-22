#include <string.h>
#include <stdlib.h>
#include "d_types.h"
#include "d_dyn_string.h"
#include "container.h"
#include "d_general_lib.h"

struct _DynString
{
    Container str
};

DynString *d_dyn_string_new(void)
{
    DynString *dstring = malloc(sizeof(DynString));
    if (dstring == NULL)
        return NULL;

    char *str = malloc(DEFAULT_CAPACITY + 1);
    str = dstring->str.data;
    if (str == NULL)
    {
        free(dstring);
        return NULL;
    }
    dstring->str.data = str;
    dstring->str.elem_size = sizeof(char);
    dstring->str.len = 0;
    dstring->str.capacity = DEFAULT_CAPACITY;
    str[0] = '\0';
    return dstring;
}

DynString *d_dyn_string_new_from_c_string(const char *str)
{
    if (str == NULL)
        return d_dyn_string_new();

    usize len = strlen(str);
    usize capacity = len + DEFAULT_CAPACITY;

    DynString *dstring = malloc(sizeof(DynString));
    if (dstring == NULL)
        return NULL;

    dstring->str.data = malloc(capacity + 1);
    if ((char *)(dstring->str.data) == NULL)
    {
        free(dstring);
        return NULL;
    }

    memcpy(dstring->str.data, str, len + 1);
    dstring->str.len = len;
    dstring->str.elem_size = sizeof(char);
    dstring->str.capacity = capacity;
    return dstring;
}

DynString *d_dyn_string_new_from_dstring(DynString *dstring)
{
    if (dstring == NULL)
        return NULL;
    return d_dyn_string_new_with_sub_string(dstring->str.data, 0, dstring->str.len);
}

DynString *d_dyn_string_new_with_reserve(usize reserve)
{
    DynString *dstring = malloc(sizeof(DynString));
    if (dstring == NULL)
        return NULL;

    reserve = (reserve == 0) ? DEFAULT_CAPACITY : reserve;
    char *str = malloc(reserve + 1);
    dstring->str.data = str;
    if (str == NULL)
    {
        free(dstring);
        return NULL;
    }

    dstring->str.elem_size = sizeof(char);
    dstring->str.len = 0;
    dstring->str.capacity = reserve;
    str[0] = '\0';
    return dstring;
}

DynString *d_dyn_string_new_with_sub_string(const char *str, usize pos, usize len)
{
    if (str == NULL)
        return NULL;

    usize str_len = strlen(str);
    if (pos > str_len)
        return NULL;

    len = (pos + len > str_len) ? (str_len - pos) : len;
    usize capacity = len + DEFAULT_CAPACITY;

    DynString *dstring = malloc(sizeof(DynString));
    if (dstring == NULL)
        return NULL;

    dstring->str.data = malloc(capacity + 1);
    if (dstring->str.data == NULL)
    {
        free(dstring);
        return NULL;
    }

    if (len != 0)
        memcpy(dstring->str.data, str + pos, len);

    dstring->str.elem_size = sizeof(char);
    dstring->str.len = len;
    dstring->str.capacity = capacity;
    ((char *)(dstring->str.data))[len] = '\0';
    return dstring;
}

char *d_dyn_string_get_string(DynString *dstring)
{
    return dstring->str.data;
}

usize d_dyn_string_get_len(DynString *dstring)
{
    return dstring->str.len;
}

DynString *d_dyn_string_sub_string_in_place(DynString *dstring, usize pos, usize len)
{
    if (dstring == NULL || pos > dstring->str.len)
        return NULL;

    len = (pos + len > dstring->str.len) ? (dstring->str.len - pos) : len;
    memmove(dstring->str.data, (char *)(dstring->str.data) + pos, len);
    dstring->str.len = len;
    ((char *)(dstring->str.data))[len] = '\0';
    return dstring;
}

usize d_dyn_string_get_capacity(DynString *dstring)
{
    return dstring->str.capacity;
}

char d_dyn_string_get_char_at(DynString *dstring, usize i)
{
#ifdef BOUNDARY_CHECK
    if (i >= dstring->str.len)
    {
        /* halt program or handle error */
    }
#endif
    return ((char *)(dstring->str.data))[i];
}

static void *increase_d_dyn_string_capacity_if_needed(DynString *dstring, usize nb_elem_to_copy)
{
    return container_increase_capacity_if_needed(&dstring->str, nb_elem_to_copy, true);
}

DynString *d_dyn_string_resize(DynString *dstring, usize len)
{
    if (dstring == NULL)
        return NULL;

    if (len > dstring->str.len)
    {
        usize count = len - dstring->str.len;
        if (increase_d_dyn_string_capacity_if_needed(dstring, count) == ERROR)
            return NULL;
        memset(dstring->str.data + dstring->str.len, 0, count + 1);
    }
    else
    {
        ((char *)(dstring->str.data))[len] = '\0';
    }

    dstring->str.len = len;
    return dstring;
}

DynString *d_dyn_string_push_char(DynString *dstring, char c)
{
    if (increase_d_dyn_string_capacity_if_needed(dstring, 1) == ERROR)
        return NULL;
    char *str = dstring->str.data;
    str[dstring->str.len++] = c;
    str[dstring->str.len] = '\0';
    return dstring;
}

DynString *d_dyn_string_push_str_with_len(DynString *dstring, const char *str_to_append, usize len)
{
    if (dstring == NULL || str_to_append == NULL)
        return NULL;

    if (increase_d_dyn_string_capacity_if_needed(dstring, len) == ERROR)
        return NULL;
    char *str = dstring->str.data;

    memcpy(str + dstring->str.len, str_to_append, len);
    dstring->str.len += len;
    str[dstring->str.len] = '\0';
    return dstring;
}

DynString *d_dyn_string_push_c_str(DynString *dstring, const char *str_to_append)
{
    if (str_to_append == NULL)
        return dstring;
    return d_dyn_string_push_str_with_len(dstring, str_to_append, strlen(str_to_append));
}

DynString *d_dyn_string_merge(DynString *dstring1, DynString *dstring2)
{
    if (dstring1 == NULL || dstring2 == NULL)
        return NULL;
    return d_dyn_string_push_str_with_len(dstring1, dstring2->str.data, dstring2->str.len);
}

DynString *d_dyn_string_replace_from_str(DynString *dstring, const char *str)
{
    if (dstring == NULL)
        return NULL;

    usize to_copy = (str == NULL) ? 0 : strlen(str);

    if (to_copy > dstring->str.len)
    {
        if (increase_d_dyn_string_capacity_if_needed(dstring, to_copy - dstring->str.len) == ERROR)
            return NULL;
    }

    if (to_copy != 0)
        memcpy(dstring->str.data, str, to_copy);

    dstring->str.len = to_copy;
    ((char *)(dstring->str.data))[dstring->str.len] = '\0';
    return dstring;
}

DynString *d_dyn_string_replace_from_dstring(DynString *dstring, const DynString *to_copy)
{
    if (to_copy == NULL)
        return d_dyn_string_replace_from_str(dstring, NULL);

    if (dstring == NULL)
        return NULL;

    if (to_copy->str.len > dstring->str.len)
    {
        if (increase_d_dyn_string_capacity_if_needed(dstring, to_copy->str.len - dstring->str.len) == ERROR)
            return NULL;
    }

    if (to_copy->str.len != 0)
        memcpy(dstring->str.data, to_copy->str.data, to_copy->str.len);

    dstring->str.len = to_copy->str.len;
    ((char *)(dstring->str.data))[dstring->str.len] = '\0';
    return dstring;
}

void d_dyn_string_destroy(DynString **dstring)
{
    if (dstring == NULL || *dstring == NULL)
        return;

    free((*dstring)->str.data);
    free(*dstring);
    *dstring = NULL;
}
