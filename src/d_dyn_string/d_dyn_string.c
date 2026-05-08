#include <string.h>
#include <stdlib.h>
#include "d_types.h"
#include "container.h"
#include "d_dyn_string.h"
#include "raw_buffer.h"
#include "d_general_lib.h"
#include "d_string_view.h"

struct DDynString
{
    RawBuffer str;
};

static DDynString *d_dyn_string_new_raw()
{
    return malloc(sizeof(DDynString));
}

DResult d_dyn_string_new_with_capacity(DDynString **new_dyn_string, usize reserve)
{
    if (new_dyn_string == NULL)
        return D_ERR_INVALID_ARG;
    else if ((*new_dyn_string = d_dyn_string_new_raw()) == NULL)
        return D_ERR_ALLOC;
    DResult op_result;
    if ((op_result = raw_buffer_init((RawBuffer *)*new_dyn_string, sizeof(char), reserve, RAW_BUF_OPT_ZERO_SENTINEL)) != D_OK)
    {
        d_dyn_string_destroy(new_dyn_string);
        return op_result;
    }
    return D_OK;
}

DResult d_dyn_string_new(DDynString **dyn_string)
{
    return d_dyn_string_new_with_capacity(dyn_string, DEFAULT_CAPACITY);
}

DResult d_dyn_string_new_from_c_string(DDynString **dyn_string, const char *str)
{
    if (dyn_string == NULL || str == NULL)
        return D_ERR_INVALID_ARG;

    usize size = strlen(str);
    if ((*dyn_string = d_dyn_string_new_raw()) == NULL)
        return D_ERR_ALLOC;
    DResult op_result;
    if ((op_result = raw_buffer_init_with_data((RawBuffer *)*dyn_string, sizeof(char), str, size, RAW_BUF_OPT_ZERO_SENTINEL)) != D_OK)
    {
        d_dyn_string_destroy(dyn_string);
        return op_result;
    }
    return D_OK;
}

DResult d_dyn_string_new_with_sub_string(DDynString **dyn_string, const char *str, usize pos, usize size)
{
    if (dyn_string == NULL || str == NULL)
        return D_ERR_INVALID_ARG;

    usize str_len = strlen(str);
    if (pos > str_len)
        return D_ERR_INVALID_ARG;

    size = (pos + size > str_len) ? (str_len - pos) : size;

    if ((*dyn_string = d_dyn_string_new_raw()) == NULL)
        return D_ERR_ALLOC;

    DResult op_result;

    if ((op_result = raw_buffer_init_with_data((RawBuffer *)*dyn_string, sizeof(char), str + pos, size, RAW_BUF_OPT_ZERO_SENTINEL)) != D_OK)
    {
        d_dyn_string_destroy(dyn_string);
        return op_result;
    }
    return D_OK;
}

DResult d_dyn_string_new_from_dstring(DDynString **new_dyn_string, DDynString *dstring)
{
    if (dstring == NULL)
        return D_ERR_INVALID_ARG;
    return d_dyn_string_new_with_sub_string(new_dyn_string, dstring->str.data, 0, dstring->str.size);
}

DCompareResult d_dyn_string_compare(DDynString *d1, DDynString *d2)
{
    if (d1 == NULL || d2 == NULL)
        return D_COMPARE_ERROR;
    return d_string_view_compare(d_string_view_from_dyn_string(d1), d_string_view_from_dyn_string(d2));
}

const char *d_dyn_string_get_string(const DDynString *dstring)
{
    return raw_buffer_get_data((RawBuffer *)dstring);
}

DResult d_dyn_string_get_size(const DDynString *dstring, usize *size)
{
    return raw_buffer_get_size((RawBuffer *)dstring, size);
}

DResult d_dyn_string_get_capacity(const DDynString *dstring, usize *capacity)
{
    return raw_buffer_get_capacity((RawBuffer *)dstring, capacity);
}

DResult d_dyn_string_sub_string_in_place(DDynString *dstring, usize pos, usize size)
{
    DResult op_result;
    usize cnt_size;
    if ((op_result = raw_buffer_get_size((RawBuffer *)dstring, &cnt_size)) != D_OK)
        return op_result;
    if (pos >= cnt_size)
        return D_ERR_INVALID_ARG;
    size = pos + size > cnt_size ? cnt_size - pos : size;
    const char *s = raw_buffer_get_data((RawBuffer *)dstring);
    return raw_buffer_replace_data_at(&dstring->str, 0, s + pos, size);
}

DResult d_dyn_string_get_char_at(const DDynString *dstring, usize i, void *out_elem)
{
    return raw_buffer_get_elem_at((RawBuffer *)dstring, i, out_elem);
}

DResult d_dyn_string_resize(DDynString *dstring, usize size, char c)
{
    return raw_buffer_resize((RawBuffer *)dstring, size, &c);
}

DResult d_dyn_string_push_char(DDynString *dstring, char c)
{
    return raw_buffer_push((RawBuffer *)dstring, &c);
}

DResult d_dyn_string_push_str_with_len(DDynString *dstring, const char *str_to_append, usize size)
{
    return raw_buffer_append_data((RawBuffer *)dstring, str_to_append, size);
}

DResult d_dyn_string_push_c_str(DDynString *dstring, const char *str_to_append)
{
    if (str_to_append == NULL)
        return D_ERR_INVALID_ARG;
    return d_dyn_string_push_str_with_len(dstring, str_to_append, strlen(str_to_append));
}

DResult d_dyn_string_merge(DDynString *dstring1, DDynString *dstring2)
{
    if (dstring1 == NULL || dstring2 == NULL)
        return D_ERR_INVALID_ARG;
    return d_dyn_string_push_str_with_len(dstring1, dstring2->str.data, dstring2->str.size);
}

DResult d_dyn_string_replace_from_str(DDynString *dstring, const char *str)
{
    if (dstring == NULL || str == NULL)
        return D_ERR_INVALID_ARG;

    return raw_buffer_replace_data_at(&dstring->str, 0, str, strlen(str));
}

DResult d_dyn_string_replace_from_dstring(DDynString *dstring, const DDynString *to_copy)
{
    return raw_buffer_replace_with((RawBuffer *)dstring, (RawBuffer *)to_copy, 0);
}

void d_dyn_string_destroy(DDynString **dstring)
{
    if (dstring == NULL || *dstring == NULL)
        return;
    DDynString *dstr = *dstring;
    raw_buffer_free((RawBuffer *)dstr);
    free(dstr);
    *dstring = NULL;
}
