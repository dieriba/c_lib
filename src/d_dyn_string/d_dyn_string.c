#include <string.h>
#include <stdlib.h>
#include "d_types.h"
#include "container.h"
#include "d_dyn_string.h"
#include "raw_buffer.h"
#include "d_general_lib.h"

struct _DynString
{
    RawBuffer str
};

static DynString *d_dyn_string_new_raw()
{
    return malloc(sizeof(DynString));
}

DResult d_dyn_string_new_with_capacity(DynString **new_dyn_string, usize reserve)
{
    if (new_dyn_string == NULL)
        return D_ERR_INVALID_ARG;
    else if ((*new_dyn_string = d_dyn_string_new_raw()) == NULL)
        return D_ERR_ALLOC;
    DResult op_result;
    if ((op_result = buffer_init((DynString *)*new_dyn_string, sizeof(char), reserve, RAW_BUF_OPT_ZERO_SENTINEL)) != D_OK)
    {
        d_dyn_string_destroy(new_dyn_string);
        return op_result;
    }
    return D_OK;
}

DResult d_dyn_string_new(DynString **dyn_string)
{
    return d_dyn_string_new_with_capacity(dyn_string, DEFAULT_CAPACITY);
}

DResult d_dyn_string_new_from_c_string(DynString **dyn_string, const char *str)
{
    if (dyn_string == NULL || str == NULL)
        return D_ERR_INVALID_ARG;

    usize len = strlen(str);
    if ((*dyn_string = d_dyn_string_new_raw()) == NULL)
        return D_ERR_ALLOC;
    DResult op_result;
    if ((op_result = buffer_init_with_data((DynString *)*dyn_string, sizeof(char), str, len, RAW_BUF_OPT_ZERO_SENTINEL)) != D_OK)
    {
        d_dyn_string_destroy(dyn_string);
        return op_result;
    }
    return D_OK;
}

DResult d_dyn_string_new_with_sub_string(DynString **dyn_string, const char *str, usize pos, usize len)
{
    if (dyn_string == NULL || str == NULL)
        return D_ERR_INVALID_ARG;

    usize str_len = strlen(str);
    if (pos > str_len)
        return D_ERR_INVALID_ARG;

    len = (pos + len > str_len) ? (str_len - pos) : len;

    if ((*dyn_string = d_dyn_string_new_raw()) == NULL)
        return D_ERR_ALLOC;

    DResult op_result;

    if ((op_result = buffer_init_with_data((DynString *)dyn_string, sizeof(char), str + pos, len, RAW_BUF_OPT_ZERO_SENTINEL)) != D_OK)
    {
        d_dyn_string_destroy(dyn_string);
        return op_result;
    }
    return D_OK;
}

DResult d_dyn_string_new_from_dstring(DynString **new_dyn_string, DynString *dstring)
{
    if (dstring == NULL)
        return D_ERR_INVALID_ARG;
    return d_dyn_string_new_with_sub_string(new_dyn_string, dstring->str.data, 0, dstring->str.size);
}

const char const *d_dyn_string_get_string(DynString *dstring)
{
    return buffer_get_data((RawBuffer *)dstring);
}

DResult d_dyn_string_get_size(DynString *dstring, usize *size)
{
    return buffer_get_size((RawBuffer *)dstring, size);
}

DResult d_dyn_string_get_capacity(DynString *dstring, usize *capacity)
{
    return buffer_get_capacity((RawBuffer *)dstring, capacity);
}

DResult d_dyn_string_sub_string_in_place(DynString *dstring, usize pos, usize len)
{
    DResult op_result;
    usize cnt_size;
    if ((op_result = buffer_get_size((RawBuffer *)dstring, &cnt_size)) != D_OK)
        return op_result;
    if (pos >= cnt_size)
        return D_ERR_INVALID_ARG;
    len = pos + len > cnt_size ? cnt_size - pos : len;
    char *s = buffer_get_data(dstring);
    return buffer_replace_data_trunc(&dstring->str, 0, s + pos, len);
}

char *d_dyn_string_get_char_at(DynString *dstring, usize i)
{
    return buffer_get_elem_at((RawBuffer *)dstring, i);
}

DResult d_dyn_string_resize(DynString *dstring, usize len, char c)
{
    return buffer_resize((RawBuffer *)dstring, len, &c);
}

DResult d_dyn_string_push_char(DynString *dstring, char c)
{
    return buffer_push((RawBuffer *)dstring, &c);
}

DResult d_dyn_string_push_str_with_len(DynString *dstring, const char *str_to_append, usize len)
{
    return buffer_append_data((RawBuffer *)dstring, str_to_append, len);
}

DResult d_dyn_string_push_c_str(DynString *dstring, const char *str_to_append)
{
    return d_dyn_string_push_str_with_len(dstring, str_to_append, strlen(str_to_append));
}

DResult d_dyn_string_merge(DynString *dstring1, DynString *dstring2)
{
    if (dstring1 == NULL || dstring2 == NULL)
        return D_ERR_INVALID_ARG;
    return d_dyn_string_push_str_with_len(dstring1, dstring2->str.data, dstring2->str.size);
}

DResult d_dyn_string_replace_from_str(DynString *dstring, const char *str)
{
    if (dstring == NULL || str == NULL)
        return D_ERR_INVALID_ARG;

    return buffer_replace_data_trunc(&dstring->str, 0, str, strlen(str));
}

DResult d_dyn_string_replace_from_dstring(DynString *dstring, const DynString *to_copy)
{
    if (dstring == NULL || to_copy == NULL)
        return D_ERR_INVALID_ARG;
    return buffer_replace_buffer_trunc(&dstring->str, &to_copy->str);
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
