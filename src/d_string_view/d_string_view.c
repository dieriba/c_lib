#include <string.h>
#include <stdlib.h>
#include "d_string_view.h"
#include "d_dyn_array.h"
#include "raw_buffer.h"
#include "d_general_lib.h"

#define dstring_view_get_char_at(view, pos) ((view).data[(pos)])

typedef bool (*char_match_fn)(char ch, const void *ctx);

static bool match_char(char c1, const void *c2)
{
    return c1 == *(const char *)c2;
}

static bool not_match_char(char c1, const void *c2)
{
    return !match_char(c1, c2);
}

static bool char_in_set(char c, const void *set)
{
    return strchr((const char *)set, c) != NULL;
}

static bool not_char_in_set(char c, const void *set)
{
    return !char_in_set(c, set);
}

static bool match_predicate(char c, const void *ctx)
{
    match fn = (match)ctx;
    return fn(c) != 0;
}

static bool not_match_predicate(char c, const void *ctx)
{
    return !match_predicate(c, ctx);
}

static usize find_first_from_index(DStringView view, usize pos, char_match_fn matches, const void *ctx)
{
    if (view.data == NULL || pos >= view.size)
        return MAX_SIZE_T_VALUE;

    for (usize i = pos; i < view.size; ++i)
    {
        if (matches(view.data[i], ctx))
            return i;
    }
    return MAX_SIZE_T_VALUE;
}

static usize find_last_from_index(DStringView view, usize pos, char_match_fn matches, const void *ctx)
{
    if (view.data == NULL || view.size == 0)
        return MAX_SIZE_T_VALUE;

    pos = pos >= view.size ? view.size - 1 : pos;
    usize i = pos;

    while (1)
    {
        if (matches(view.data[i], ctx))
            return i;
        if (i == 0)
            break;
        --i;
    }

    return MAX_SIZE_T_VALUE;
}

char *d_string_view_substr(DStringView view, usize pos, usize size)
{
    return d_substr(view.data, pos, size);
}

DStringView d_string_view_from_parts(const char *data, usize size)
{
    DStringView view = {
        .data = data,
        .size = (data == NULL) ? 0 : size};

    return view;
}

DStringView d_string_view_from_c_string(const char *c_str)
{
    DStringView view = {
        .data = c_str,
        .size = (c_str == NULL) ? 0 : strlen(c_str)};

    return view;
}

DStringView d_string_view_from_dyn_string(const DDynString *dstring)
{
    if (dstring == NULL)
        return d_string_view_from_parts(NULL, 0);
    usize size;
    d_dyn_string_get_size(dstring, &size);
    return d_string_view_from_parts(d_dyn_string_get_string(dstring), size);
}

bool d_string_view_is_empty(DStringView view)
{
    return view.size == 0;
}

usize d_string_view_len(DStringView view)
{
    return view.size;
}

const char *d_string_view_data(DStringView view)
{
    return view.data;
}

char d_string_view_get_char_at(DStringView view, usize index)
{
#ifdef BOUNDARY_CHECK
    if (index >= view.size)
    {
        /* stop program execution */
    }
#endif
    return view.data[index];
}

DStringView d_string_view_subview(DStringView view, usize pos, usize size)
{
    if (view.data == NULL || pos > view.size)
        return d_string_view_from_parts(NULL, 0);

    size = (pos + size > view.size) ? (view.size - pos) : size;
    return d_string_view_from_parts(view.data + pos, size);
}

int d_string_view_compare(DStringView view1, DStringView view2)
{
    return strcmp(view1.data, view2.data);
}

int d_string_view_compare_against_c_string(DStringView view, const char *c_str)
{
    return d_string_view_compare(view, d_string_view_from_c_string(c_str));
}

bool d_string_view_equals(DStringView view1, DStringView view2)
{
    return d_string_view_compare(view1, view2) == 0;
}

bool d_string_view_equals_c_string(DStringView view, const char *c_str)
{
    return d_string_view_compare_against_c_string(view, c_str) == 0;
}

bool d_string_view_starts_with_char(DStringView view, char c)
{
    return view.size == 0 ? false : dstring_view_get_char_at(view, 0) == c;
}

bool d_string_view_ends_with_char(DStringView view, char c)
{
    return view.size == 0 ? false : dstring_view_get_char_at(view, view.size - 1) == c;
}

bool d_string_view_starts_with_view(DStringView view, DStringView prefix)
{
    if (prefix.size > view.size)
        return false;
    return memcmp(view.data, prefix.data, prefix.size) == 0;
}

bool d_string_view_ends_with_view(DStringView view, DStringView suffix)
{
    if (suffix.size > view.size)
        return false;
    return memcmp(view.data + (view.size - suffix.size), suffix.data, suffix.size) == 0;
}

bool d_string_view_starts_with_c_string(DStringView view, const char *prefix)
{
    return d_string_view_starts_with_view(view, d_string_view_from_c_string(prefix));
}

bool d_string_view_ends_with_c_string(DStringView view, const char *suffix)
{
    return d_string_view_ends_with_view(view, d_string_view_from_c_string(suffix));
}

usize d_string_view_find_first_matching_char_from_index(DStringView view, char c, usize pos)
{
    return find_first_from_index(view, pos, match_char, &c);
}

usize d_string_view_find_first_matching_char_from_start(DStringView view, char c)
{
    return d_string_view_find_first_matching_char_from_index(view, c, 0);
}

usize d_string_view_find_first_not_matching_char_from_index(DStringView view, char c, usize pos)
{
    return find_first_from_index(view, pos, not_match_char, &c);
}

usize d_string_view_find_first_not_matching_char_from_start(DStringView view, char c)
{
    return d_string_view_find_first_not_matching_char_from_index(view, c, 0);
}

usize d_string_view_find_last_matching_char_from_index(DStringView view, char c, usize pos)
{
    return find_last_from_index(view, pos, match_char, &c);
}

usize d_string_view_find_last_matching_char_from_end(DStringView view, char c)
{
    return d_string_view_find_last_matching_char_from_index(view, c, view.size);
}

usize d_string_view_find_last_not_matching_char_from_index(DStringView view, char c, usize pos)
{
    return find_last_from_index(view, pos, not_match_char, &c);
}

usize d_string_view_find_last_not_matching_char_from_end(DStringView view, char c)
{
    return d_string_view_find_last_not_matching_char_from_index(view, c, view.size);
}

usize d_string_view_find_first_matching_view_from_index(DStringView view, DStringView to_find, usize pos)
{
    if (pos >= view.size)
        return MAX_SIZE_T_VALUE;
    if (to_find.size == 0)
        return pos;
    if (to_find.size > view.size - pos)
        return MAX_SIZE_T_VALUE;

    for (usize i = pos; i < view.size; ++i)
    {
        if (view.data[i] == to_find.data[0] &&
            view.size - i >= to_find.size &&
            memcmp(view.data + i, to_find.data, to_find.size) == 0)
            return i;
    }
    return MAX_SIZE_T_VALUE;
}

usize d_string_view_find_first_matching_view_from_start(DStringView view, DStringView to_find)
{
    return d_string_view_find_first_matching_view_from_index(view, to_find, 0);
}

usize d_string_view_find_first_matching_c_string_from_index(DStringView view, const char *str, usize pos)
{
    return d_string_view_find_first_matching_view_from_index(view, d_string_view_from_c_string(str), pos);
}

usize d_string_view_find_first_matching_c_string_from_start(DStringView view, const char *str)
{
    return d_string_view_find_first_matching_c_string_from_index(view, str, 0);
}

usize d_string_view_find_last_matching_view_from_index(DStringView view, DStringView to_find, usize pos)
{
    if (view.size == 0)
        return MAX_SIZE_T_VALUE;
    if (to_find.size == 0)
        return pos > view.size ? view.size : pos;

    pos = pos >= view.size ? view.size - 1 : pos;

    while (1)
    {
        if (view.data[pos] == to_find.data[0] &&
            view.size - pos >= to_find.size &&
            memcmp(view.data + pos, to_find.data, to_find.size) == 0)
            return pos;

        if (pos == 0)
            break;
        --pos;
    }
    return MAX_SIZE_T_VALUE;
}

usize d_string_view_find_last_matching_view_from_end(DStringView view, DStringView to_find)
{
    return d_string_view_find_last_matching_view_from_index(view, to_find, view.size);
}

usize d_string_view_find_last_matching_c_string_from_index(DStringView view, const char *str, usize pos)
{
    return d_string_view_find_last_matching_view_from_index(view, d_string_view_from_c_string(str), pos);
}

usize d_string_view_find_last_matching_c_string_from_end(DStringView view, const char *str)
{
    return d_string_view_find_last_matching_c_string_from_index(view, str, view.size);
}

usize d_string_view_find_first_char_in_set_from_index(DStringView view, const char *set, usize pos)
{
    return find_first_from_index(view, pos, char_in_set, set);
}

usize d_string_view_find_first_char_in_set_from_start(DStringView view, const char *set)
{
    return d_string_view_find_first_char_in_set_from_index(view, set, 0);
}

usize d_string_view_find_first_char_not_in_set_from_index(DStringView view, const char *set, usize pos)
{
    return find_first_from_index(view, pos, not_char_in_set, set);
}

usize d_string_view_find_first_char_not_in_set_from_start(DStringView view, const char *set)
{
    return d_string_view_find_first_char_not_in_set_from_index(view, set, 0);
}

usize d_string_view_find_last_char_in_set_from_index(DStringView view, const char *set, usize pos)
{
    return find_last_from_index(view, pos, char_in_set, set);
}

usize d_string_view_find_last_char_in_set_from_end(DStringView view, const char *set)
{
    return d_string_view_find_last_char_in_set_from_index(view, set, view.size);
}

usize d_string_view_find_last_char_not_in_set_from_index(DStringView view, const char *set, usize pos)
{
    return find_last_from_index(view, pos, not_char_in_set, set);
}

usize d_string_view_find_last_char_not_in_set_from_end(DStringView view, const char *set)
{
    return d_string_view_find_last_char_not_in_set_from_index(view, set, view.size);
}

usize d_string_view_find_first_matching_predicate_from_index(DStringView view, match fn, usize pos)
{
    return find_first_from_index(view, pos, match_predicate, fn);
}

usize d_string_view_find_first_matching_predicate_from_start(DStringView view, match fn)
{
    return d_string_view_find_first_matching_predicate_from_index(view, fn, 0);
}

usize d_string_view_find_first_not_matching_predicate_from_index(DStringView view, match fn, usize pos)
{
    return find_first_from_index(view, pos, not_match_predicate, fn);
}

usize d_string_view_find_first_not_matching_predicate_from_start(DStringView view, match fn)
{
    return d_string_view_find_first_not_matching_predicate_from_index(view, fn, 0);
}

usize d_string_view_find_last_matching_predicate_from_index(DStringView view, match fn, usize pos)
{
    return find_last_from_index(view, pos, match_predicate, fn);
}

usize d_string_view_find_last_matching_predicate_from_end(DStringView view, match fn)
{
    return d_string_view_find_last_matching_predicate_from_index(view, fn, view.size);
}

usize d_string_view_find_last_not_matching_predicate_from_index(DStringView view, match fn, usize pos)
{
    return find_last_from_index(view, pos, not_match_predicate, fn);
}

usize d_string_view_find_last_not_matching_predicate_from_end(DStringView view, match fn)
{
    return d_string_view_find_last_not_matching_predicate_from_index(view, fn, view.size);
}

DStringView d_string_view_trim_left_by_char(DStringView view, char c)
{
    usize i = d_string_view_find_first_not_matching_char_from_start(view, c);
    if (i == MAX_SIZE_T_VALUE)
        i = view.size;
    return d_string_view_subview(view, i, view.size - i);
}

DStringView d_string_view_trim_right_by_char(DStringView view, char c)
{
    usize i = d_string_view_find_last_not_matching_char_from_end(view, c);
    return d_string_view_subview(view, 0, i == MAX_SIZE_T_VALUE ? 0 : i + 1);
}

DStringView d_string_view_trim_left_by_predicate(DStringView view, match fn)
{
    usize i = d_string_view_find_first_not_matching_predicate_from_start(view, fn);
    if (i == MAX_SIZE_T_VALUE)
        i = view.size;
    return d_string_view_subview(view, i, view.size - i);
}

DStringView d_string_view_trim_right_by_predicate(DStringView view, match fn)
{
    usize i = d_string_view_find_last_not_matching_predicate_from_end(view, fn);
    return d_string_view_subview(view, 0, i == MAX_SIZE_T_VALUE ? 0 : i + 1);
}

DResult d_dyn_string_new_from_string_view(DDynString **new_dyn_string, DStringView view)
{
    return d_dyn_string_new_with_sub_string(new_dyn_string, view.data, 0, view.size);
}

static void _free_str(void *elem)
{
    free(*((void **)elem));
}

DResult d_string_view_split_by_char_of_str(DDynArray **new_dyn_array, DStringView view, BufferOpts opts, char *str)
{
    if (new_dyn_array == NULL || str == NULL)
        return D_ERR_INVALID_ARG;
    DResult op_result;
    usize size = view.size;
    if ((op_result = d_dyn_array_new_ptr_arr(new_dyn_array, size, _free_str, opts)) != D_OK)
        return op_result;
    const char *string = view.data;
    DDynArray *dyn_array = *new_dyn_array;
    usize str_len = strlen(str);
    for (usize i = 0; i < size;)
    {
        while (i < size && memchr(str, (int)string[i], str_len) != NULL)
            ++i;
        if (i != size)
        {
            usize j = d_string_view_find_first_char_in_set_from_index(view, str, i);
            char *str = d_string_view_substr(view, i, j == MAX_SIZE_T_VALUE ? MAX_SIZE_T_VALUE : j - i);
            if ((op_result = d_dyn_array_push_back_ptr(dyn_array, str)) != D_OK)
            {
                d_dyn_array_destroy(new_dyn_array);
                return op_result;
            }
            i = j;
        }
        else
            break;
    }
    return D_OK;
}

DResult d_string_view_split_by_char(DDynArray **new_dyn_array, DStringView view, BufferOpts opts, char c)
{
    if (new_dyn_array == NULL)
        return D_ERR_INVALID_ARG;
    DResult op_result;
    usize size = view.size;
    if ((op_result = d_dyn_array_new_ptr_arr(new_dyn_array, size, _free_str, opts)) != D_OK)
        return op_result;
    const char *str = view.data;
    DDynArray *dyn_array = *new_dyn_array;
    for (usize i = 0; i < size;)
    {
        while (i < size && str[i] == c)
            ++i;
        if (i != size)
        {
            usize j = d_string_view_find_first_matching_char_from_index(view, c, i);
            char *str = d_string_view_substr(view, i, j == MAX_SIZE_T_VALUE ? MAX_SIZE_T_VALUE : j - i);
            if ((op_result = d_dyn_array_push_back_ptr(dyn_array, str)) != D_OK)
            {
                d_dyn_array_destroy(new_dyn_array);
                return op_result;
            }
            i = j;
        }
        else
            break;
    }
    return D_OK;
}
