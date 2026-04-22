#include <string.h>
#include <stdlib.h>
#include "d_types.h"
#include "d_dyn_string.h"
#include "container.h"
#include "d_general_lib.h"

struct _DynString
{
    char *string;
    usize len;
    usize capacity;
};

DynString *d_dyn_string_new(void)
{
    DynString *dstring = malloc(sizeof(DynString));
    if (dstring == NULL)
        return NULL;

    dstring->string = malloc(DEFAULT_CAPACITY + 1);
    if (dstring->string == NULL)
    {
        free(dstring);
        return NULL;
    }

    dstring->len = 0;
    dstring->capacity = DEFAULT_CAPACITY;
    dstring->string[0] = '\0';
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

    dstring->string = malloc(capacity + 1);
    if (dstring->string == NULL)
    {
        free(dstring);
        return NULL;
    }

    memcpy(dstring->string, str, len + 1);
    dstring->len = len;
    dstring->capacity = capacity;
    return dstring;
}

DynString *d_dyn_string_new_from_dstring(DynString *dstring)
{
    if (dstring == NULL)
        return NULL;
    return d_dyn_string_new_with_sub_string(dstring->string, 0, dstring->len);
}

DynString *d_dyn_string_new_with_reserve(usize reserve)
{
    DynString *dstring = malloc(sizeof(DynString));
    if (dstring == NULL)
        return NULL;

    reserve = (reserve == 0) ? DEFAULT_CAPACITY : reserve;

    dstring->string = malloc(reserve + 1);
    if (dstring->string == NULL)
    {
        free(dstring);
        return NULL;
    }

    dstring->len = 0;
    dstring->capacity = reserve;
    dstring->string[0] = '\0';
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

    dstring->string = malloc(capacity + 1);
    if (dstring->string == NULL)
    {
        free(dstring);
        return NULL;
    }

    if (len != 0)
        memcpy(dstring->string, str + pos, len);

    dstring->len = len;
    dstring->capacity = capacity;
    dstring->string[len] = '\0';
    return dstring;
}

char *d_dyn_string_get_string(DynString *dstring)
{
    return dstring->string;
}

usize d_dyn_string_get_len(DynString *dstring)
{
    return dstring->len;
}

char *d_dyn_string_substr(DynString *dstring, usize pos, usize len)
{
    return d_substr(dstring->string, pos, len);
}

DynString *d_dyn_string_sub_string_in_place(DynString *dstring, usize pos, usize len)
{
    if (dstring == NULL || pos > dstring->len)
        return NULL;

    len = (pos + len > dstring->len) ? (dstring->len - pos) : len;
    memmove(dstring->string, dstring->string + pos, len);
    dstring->len = len;
    dstring->string[len] = '\0';
    return dstring;
}

usize d_dyn_string_get_capacity(DynString *dstring)
{
    return dstring->capacity;
}

char d_dyn_string_get_char_at(DynString *dstring, usize i)
{
#ifdef BOUNDARY_CHECK
    if (i >= dstring->len)
    {
        /* halt program or handle error */
    }
#endif
    return dstring->string[i];
}

static void *increase_d_dyn_string_capacity_if_needed(void *ptr_data, usize *capacity, usize nb_elem, usize elem_size, usize nb_elem_to_copy)
{
    return container_increase_capacity_if_needed(ptr_data, capacity, nb_elem, elem_size, nb_elem_to_copy, true);
}

DynString *d_dyn_string_resize(DynString *dstring, usize len)
{
    if (dstring == NULL)
        return NULL;

    if (len > dstring->len)
    {
        usize count = len - dstring->len;
        void *data = increase_d_dyn_string_capacity_if_needed(dstring->string, &dstring->capacity, dstring->len, sizeof(char), count);
        if (data == NULL)
            return NULL;
        dstring->string = data;
        memset(dstring->string + dstring->len, 0, count + 1);
    }
    else
    {
        dstring->string[len] = '\0';
    }

    dstring->len = len;
    return dstring;
}

DynString *d_dyn_string_push_char(DynString *dstring, char c)
{
    void *data = increase_d_dyn_string_capacity_if_needed(dstring->string, &dstring->capacity, dstring->len, sizeof(char), 1);
    if (data == NULL)
        return NULL;
    dstring->string = data;
    dstring->string[dstring->len++] = c;
    dstring->string[dstring->len] = '\0';
    return dstring;
}

DynString *d_dyn_string_push_str_with_len(DynString *dstring, const char *str_to_append, usize len)
{
    if (dstring == NULL || str_to_append == NULL)
        return NULL;

    void *data = increase_d_dyn_string_capacity_if_needed(dstring->string, &dstring->capacity, dstring->len, sizeof(char), len);
    if (data == NULL)
        return NULL;
    dstring->string = data;
    memcpy(dstring->string + dstring->len, str_to_append, len);
    dstring->len += len;
    dstring->string[dstring->len] = '\0';
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
    return d_dyn_string_push_str_with_len(dstring1, dstring2->string, dstring2->len);
}

DynString *d_dyn_string_replace_from_str(DynString *dstring, const char *str)
{
    if (dstring == NULL)
        return NULL;

    usize to_copy = (str == NULL) ? 0 : strlen(str);

    if (to_copy > dstring->len)
    {
        void *data = increase_d_dyn_string_capacity_if_needed(dstring->string, &dstring->capacity, dstring->len, sizeof(char), to_copy - dstring->len);
        if (data == NULL)
            return NULL;
        dstring->string = data;
    }

    if (to_copy != 0)
        memcpy(dstring->string, str, to_copy);

    dstring->len = to_copy;
    dstring->string[dstring->len] = '\0';
    return dstring;
}

DynString *d_dyn_string_replace_from_dstring(DynString *dstring, const DynString *to_copy)
{
    if (to_copy == NULL)
        return d_dyn_string_replace_from_str(dstring, NULL);

    if (dstring == NULL)
        return NULL;

    if (to_copy->len > dstring->len)
    {
        void *data = increase_d_dyn_string_capacity_if_needed(dstring->string, &dstring->capacity, dstring->len, sizeof(char), to_copy->len - dstring->len);
        if (data == NULL)
            return NULL;
        dstring->string = data;
    }

    if (to_copy->len != 0)
        memcpy(dstring->string, to_copy->string, to_copy->len);

    dstring->len = to_copy->len;
    dstring->string[dstring->len] = '\0';
    return dstring;
}

void d_dyn_string_destroy(DynString **dstring)
{
    if (dstring == NULL || *dstring == NULL)
        return;

    free((*dstring)->string);
    free(*dstring);
    *dstring = NULL;
}