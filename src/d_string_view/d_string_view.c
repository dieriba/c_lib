#include <string.h>

#include "d_string_view.h"

typedef struct _DStringView
{
    const char *data;
    usize len;
} DStringView;

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
    if (view.data == NULL || pos >= view.len)
        return MAX_SIZE_T_VALUE;

    for (usize i = pos; i < view.len; ++i)
    {
        if (matches(view.data[i], ctx))
            return i;
    }
    return MAX_SIZE_T_VALUE;
}

static usize find_last_from_index(DStringView view, usize pos, char_match_fn matches, const void *ctx)
{
    if (view.data == NULL || view.len == 0)
        return MAX_SIZE_T_VALUE;

    pos = pos >= view.len ? view.len - 1 : pos;
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

DStringView d_string_view_from_parts(const char *data, usize len)
{
    DStringView view;

    view.data = data;
    view.len = (data == NULL) ? 0 : len;
    return view;
}

DStringView d_string_view_from_c_string(const char *c_str)
{
    DStringView view;

    view.data = c_str;
    view.len = (c_str == NULL) ? 0 : strlen(c_str);
    return view;
}

DStringView d_string_view_from_dyn_string(const DynString *dstring)
{
    if (dstring == NULL)
        return d_string_view_from_parts(NULL, 0);

    return d_string_view_from_parts(d_dyn_string_get_string(dstring), d_dyn_string_get_len(dstring));
}

bool d_string_view_is_empty(DStringView view)
{
    return view.len == 0;
}

usize d_string_view_len(DStringView view)
{
    return view.len;
}

const char *d_string_view_data(DStringView view)
{
    return view.data;
}

char d_string_view_get_char_at(DStringView view, usize index)
{
#ifdef BOUNDARY_CHECK
    if (index >= view.len)
    {
        /* stop program execution */
    }
#endif
    return view.data[index];
}

/*
** Slicing
*/

DStringView d_string_view_subview(DStringView view, usize pos, usize len)
{
    if (view.data == NULL || pos > view.len)
        return d_string_view_from_parts(NULL, 0);

    len = (pos + len > view.len) ? (view.len - pos) : len;
    return d_string_view_from_parts(view.data + pos, len);
}

int32 d_string_view_compare(DStringView view1, DStringView view2)
{

    return 0;
}

int32 d_string_view_compare_against_c_string(DStringView view, const char *c_str)
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
    return view.len == 0 ? false : dstring_view_get_char_at(view, 0) == c;
}

bool d_string_view_ends_with_char(DStringView view, char c)
{
    return view.len == 0 ? false : dstring_view_get_char_at(view, view.len - 1) == c;
}

bool d_string_view_starts_with_view(DStringView view, DStringView prefix)
{
    if (prefix.len > view.len)
        return false;
    return memcmp(view.data, prefix.data, prefix.len) == 0;
}

bool d_string_view_ends_with_view(DStringView view, DStringView suffix)
{
    if (suffix.len > view.len)
        return false;
    return memcmp(view.data + (view.len - suffix.len), suffix.data, suffix.len) == 0;
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
    return d_string_view_find_last_matching_char_from_index(view, c, view.len);
}

usize d_string_view_find_last_not_matching_char_from_index(DStringView view, char c, usize pos)
{
    return find_last_from_index(view, pos, not_match_char, &c);
}

usize d_string_view_find_last_not_matching_char_from_end(DStringView view, char c)
{
    return d_string_view_find_last_not_matching_char_from_index(view, c, view.len);
}

usize d_string_view_find_first_matching_view_from_index(DStringView view, DStringView to_find, usize pos)
{
    if (pos >= view.len)
        return MAX_SIZE_T_VALUE;
    if (to_find.len == 0)
        return pos;
    if (to_find.len > view.len - pos)
        return MAX_SIZE_T_VALUE;

    for (usize i = pos; i < view.len; ++i)
    {
        if (view.data[i] == to_find.data[0] &&
            view.len - i >= to_find.len &&
            memcmp(view.data + i, to_find.data, to_find.len) == 0)
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
    if (view.len == 0)
        return MAX_SIZE_T_VALUE;
    if (to_find.len == 0)
        return pos > view.len ? view.len : pos;

    pos = pos >= view.len ? view.len - 1 : pos;

    while (1)
    {
        if (view.data[pos] == to_find.data[0] &&
            view.len - pos >= to_find.len &&
            memcmp(view.data + pos, to_find.data, to_find.len) == 0)
            return pos;

        if (pos == 0)
            break;
        --pos;
    }
    return MAX_SIZE_T_VALUE;
}

usize d_string_view_find_last_matching_view_from_end(DStringView view, DStringView to_find)
{
    return d_string_view_find_last_matching_view_from_index(view, to_find, view.len);
}

usize d_string_view_find_last_matching_c_string_from_index(DStringView view, const char *str, usize pos)
{
    return d_string_view_find_last_matching_view_from_index(view, d_string_view_from_c_string(str), pos);
}

usize d_string_view_find_last_matching_c_string_from_end(DStringView view, const char *str)
{
    return d_string_view_find_last_matching_c_string_from_index(view, str, view.len);
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
    return d_string_view_find_last_char_in_set_from_index(view, set, view.len);
}

usize d_string_view_find_last_char_not_in_set_from_index(DStringView view, const char *set, usize pos)
{
    return find_last_from_index(view, pos, not_char_in_set, set);
}

usize d_string_view_find_last_char_not_in_set_from_end(DStringView view, const char *set)
{
    return d_string_view_find_last_char_not_in_set_from_index(view, set, view.len);
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
    return d_string_view_find_last_matching_predicate_from_index(view, fn, view.len);
}

usize d_string_view_find_last_not_matching_predicate_from_index(DStringView view, match fn, usize pos)
{
    return find_last_from_index(view, pos, not_match_predicate, fn);
}

usize d_string_view_find_last_not_matching_predicate_from_end(DStringView view, match fn)
{
    return d_string_view_find_last_not_matching_predicate_from_index(view, fn, view.len);
}

DStringView d_string_view_trim_left_by_char(DStringView view, char c)
{
    usize i = d_string_view_find_first_not_matching_char_from_start(view, c);
    if (i == MAX_SIZE_T_VALUE)
        i = view.len;
    return d_string_view_subview(view, i, view.len - i);
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
        i = view.len;
    return d_string_view_subview(view, i, view.len - i);
}

DStringView d_string_view_trim_right_by_predicate(DStringView view, match fn)
{
    usize i = d_string_view_find_last_not_matching_predicate_from_end(view, fn);
    return d_string_view_subview(view, 0, i == MAX_SIZE_T_VALUE ? 0 : i + 1);
}

DynString *d_dyn_string_new_from_string_view(DStringView view)
{
    return d_dyn_string_new_with_sub_string(view.data, 0, view.len);
}