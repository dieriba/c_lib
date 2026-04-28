#include <string.h>
#include <stdlib.h>
#include "d_types.h"
#include "d_dyn_string.h"
#include "buffer.h"
#include "d_general_lib.h"

struct _DynString
{
    Buffer str
};

static DynString *d_dyn_string_new_raw()
{
    return malloc(sizeof(DynString));
}

DynString *d_dyn_string_new(void)
{
    DynString *dstring = d_dyn_string_new_raw();
    if (dstring == NULL)
        return NULL;
    if (buffer_init(&dstring->str, sizeof(char), DEFAULT_CAPACITY, CNT_OPT_ZERO_SENTINEL) == ERROR)
    {
        free(dstring);
        return NULL;
    }
    return dstring;
}

DynString *d_dyn_string_new_from_c_string(const char *str)
{
    if (str == NULL)
        return NULL;

    usize len = strlen(str);

    DynString *dstring = d_dyn_string_new_raw();
    if (dstring == NULL)
        return NULL;

    if (buffer_init_with_data(&dstring->str, sizeof(char), str, len, CNT_OPT_ZERO_SENTINEL) == ERROR)
    {
        d_dyn_string_destroy(&dstring);
        return NULL;
    }
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

    DynString *dstring = d_dyn_string_new_raw();
    if (dstring == NULL)
        return NULL;

    if (buffer_init_with_data(&dstring->str, sizeof(char), str + pos, len, CNT_OPT_ZERO_SENTINEL) == ERROR)
    {
        d_dyn_string_destroy(&dstring);
        return NULL;
    }
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
    DynString *dstring = d_dyn_string_new_raw();
    if (dstring == NULL)
        return NULL;

    if (buffer_init(&dstring->str, sizeof(char), reserve, CNT_OPT_ZERO_SENTINEL) == ERROR)
    {
        d_dyn_string_destroy(&dstring);
        return NULL;
    }
    return dstring;
}

const char const *d_dyn_string_get_string(DynString *dstring)
{
    return dstring->str.data;
}

usize d_dyn_string_get_len(DynString *dstring)
{
    return dstring->str.len;
}

DynString *d_dyn_string_sub_string_in_place(DynString *dstring, usize pos, usize len)
{
    if (dstring == NULL)
        return NULL;
    usize cnt_len = buffer_get_len(&dstring->str);
    if (pos >= cnt_len)
        return NULL;
    len = pos + len > cnt_len ? cnt_len - pos : len;
    char *s = buffer_get_data(dstring);
    if (buffer_replace_data_trunc(&dstring->str, 0, s + pos, len) == ERROR)
        return NULL;
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

DynString *d_dyn_string_resize(DynString *dstring, usize len, char c)
{
    if (dstring == NULL || buffer_resize(&dstring->str, len, &c) == ERROR)
        return NULL;
    return dstring;
}

DynString *d_dyn_string_push_char(DynString *dstring, char c)
{
    if (dstring == NULL || buffer_push(&dstring->str, &c) == ERROR)
        return NULL;
    return dstring;
}

DynString *d_dyn_string_push_str_with_len(DynString *dstring, const char *str_to_append, usize len)
{
    if (dstring == NULL || buffer_append_data(&dstring->str, str_to_append, len) == ERROR)
        return NULL;
    return dstring;
}

DynString *d_dyn_string_push_c_str(DynString *dstring, const char *str_to_append)
{
    if (str_to_append == NULL)
        return NULL;
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
    if (dstring == NULL || str == NULL || buffer_replace_data_trunc(&dstring->str, 0, str, strlen(str)) == ERROR)
        return NULL;

    return dstring;
}

DynString *d_dyn_string_replace_from_dstring(DynString *dstring, const DynString *to_copy)
{
    if (dstring == NULL || to_copy == NULL)
        return NULL;
    if (buffer_replace_buffer_trunc(&dstring->str, &to_copy->str) == ERROR)
        return NULL;
    return dstring;
}

void d_dyn_string_destroy(DynString **dstring)
{
    if (dstring == NULL || *dstring == NULL)
        return;
    DynString *dstr = *dstring;
    buffer_destroy(&dstr->str);
    free(dstr);
    *dstring = NULL;
}
