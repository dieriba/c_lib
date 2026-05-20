#include <stdio.h>
#include "container.h"
#include "d_string_view.h"
#include "d_math.h"
#include "d_general_lib.h"

#define dstring_view_get_char_at(view, pos) ((view).data[(pos)])

typedef bool (*char_match_fn)(char ch, const void *ctx);
typedef usize (*TokenBoundFn)(DStringView, void *ctx, usize);
typedef DResult (*PushBackFn)(DDynArray *, DStringView, usize, usize);

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
    const DStringView *set_view = set;
    return memchr(set_view->data, c, set_view->size) != NULL;
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
    if (pos >= view.size || ctx == NULL)
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
    if (view.size == 0 || ctx == NULL)
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
    if (pos > view.size)
        return NULL;
    return d_substr(view.data + pos, 0, size);
}

DStringView d_string_view_from_parts(const char *data, usize size)
{
    return (DStringView){
        .data = data == NULL ? "" : data,
        .size = data == NULL ? 0 : size,
    };
}

DStringView d_string_view_from_c_string(const char *c_str)
{
    if (c_str == NULL)
        return (DStringView){.data = "", .size = 0};
    return (DStringView){.data = c_str, .size = strlen(c_str)};
}

DStringView d_string_view_from_dyn_string(const DDynString *dstring)
{
    if (dstring == NULL)
        return (DStringView){.data = "", .size = 0};
    return d_string_view_from_parts(d_dyn_string_get_string(dstring), dstring->str.size);
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

DResult d_string_view_get_char_at(DStringView view, usize index, char *out)
{
    if (index >= view.size || out == NULL)
        return D_ERR_INVALID_ARG;
    *out = view.data[index];
    return D_OK;
}

DStringView d_string_view_subview(DStringView view, usize pos, usize size)
{
    if (pos >= view.size)
        return (DStringView){.data = "", .size = 0};

    usize chr_viewable = view.size - pos;
    size = chr_viewable < size ? chr_viewable : size;
    return d_string_view_from_parts(view.data + pos, size);
}

bool d_string_view_compare(DStringView view1, DStringView view2)
{
    if (view1.size != view2.size)
        return false;
    return memcmp(view1.data, view2.data, view1.size) == 0;
}

bool d_string_view_compare_against_c_string(DStringView view, const char *c_str)
{
    return d_string_view_compare(view, d_string_view_from_c_string(c_str));
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
    if (view.size - pos < to_find.size)
        return MAX_SIZE_T_VALUE;
    usize stop = view.size - to_find.size;
    for (usize i = pos; i <= stop; ++i)
    {
        if (view.data[i] == to_find.data[0] && memcmp(view.data + i, to_find.data, to_find.size) == 0)
            return i;
    }
    return MAX_SIZE_T_VALUE;
}

usize d_string_view_find_first_matching_view_from_start(DStringView view, DStringView to_find)
{
    return d_string_view_find_first_matching_view_from_index(view, to_find, 0);
}

usize d_string_view_find_last_matching_view_from_index(DStringView view, DStringView to_find, usize pos)
{
    if (view.size == 0 || view.size < to_find.size)
        return MAX_SIZE_T_VALUE;

    if (to_find.size == 0)
        return pos > view.size ? view.size : pos;

    pos = pos >= view.size ? view.size - 1 : pos;
    usize minimal_start_pos = view.size - to_find.size;
    if (pos > minimal_start_pos)
        pos = minimal_start_pos;
    while (true)
    {
        if (view.data[pos] == to_find.data[0] && memcmp(view.data + pos, to_find.data, to_find.size) == 0)
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

usize d_string_view_find_first_char_in_set_from_index(DStringView view, DStringView set, usize pos)
{
    return find_first_from_index(view, pos, char_in_set, &set);
}

usize d_string_view_find_first_char_in_set_from_start(DStringView view, DStringView set)
{
    return d_string_view_find_first_char_in_set_from_index(view, set, 0);
}

usize d_string_view_find_first_char_not_in_set_from_index(DStringView view, DStringView set, usize pos)
{
    return find_first_from_index(view, pos, not_char_in_set, &set);
}

usize d_string_view_find_first_char_not_in_set_from_start(DStringView view, DStringView set)
{
    return d_string_view_find_first_char_not_in_set_from_index(view, set, 0);
}

usize d_string_view_find_last_char_in_set_from_index(DStringView view, DStringView set, usize pos)
{
    return find_last_from_index(view, pos, char_in_set, &set);
}

usize d_string_view_find_last_char_in_set_from_end(DStringView view, DStringView set)
{
    return d_string_view_find_last_char_in_set_from_index(view, set, view.size);
}

usize d_string_view_find_last_char_not_in_set_from_index(DStringView view, DStringView set, usize pos)
{
    return find_last_from_index(view, pos, not_char_in_set, &set);
}

usize d_string_view_find_last_char_not_in_set_from_end(DStringView view, DStringView set)
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

DResult d_dyn_string_init_from_string_view(DDynString *new_dyn_string, DStringView view)
{
    if (new_dyn_string == NULL)
        return D_ERR_INVALID_ARG;
    return raw_buffer_init_with_data((RawBuffer *)new_dyn_string, sizeof(char), view.data, view.size, NULL, RAW_BUF_OPT_ZERO_SENTINEL);
}

static DResult push_back_sub_view_fn(DDynArray *d_dyn_array, DStringView view, usize start_pos, usize size)
{
    DStringView subview = d_string_view_subview(view, start_pos, size);
    return d_dyn_array_push_back(d_dyn_array, &subview);
}

static DResult push_back_owned_sub_view_fn(DDynArray *d_dyn_array, DStringView view, usize start_pos, usize size)
{
    char *subview = d_string_view_substr(view, start_pos, size);
    if (subview == NULL)
        return D_ERR_ALLOC;
    DResult op_result = d_dyn_array_push_back_ptr(d_dyn_array, subview);
    if (op_result != D_OK)
        free(subview);
    return op_result;
}

static usize token_start_char(DStringView view, void *ctx, usize pos)
{
    return d_string_view_find_first_not_matching_char_from_index(view, *(char *)ctx, pos);
}

static usize token_end_char(DStringView view, void *ctx, usize pos)
{
    return d_string_view_find_first_matching_char_from_index(view, *(char *)ctx, pos);
}

static usize token_start_char_set(DStringView view, void *ctx, usize pos)
{
    return d_string_view_find_first_char_not_in_set_from_index(view, *(DStringView *)ctx, pos);
}

static usize token_end_char_set(DStringView view, void *ctx, usize pos)
{
    return d_string_view_find_first_char_in_set_from_index(view, *(DStringView *)ctx, pos);
}

static DResult split_string_view(DDynArray *new_dyn_array, DStringView view, void *ctx, TokenBoundFn token_start_idx_fn, TokenBoundFn token_end_idx_fn, PushBackFn push_back_token)
{
    usize i = 0;
    while (i != MAX_SIZE_T_VALUE)
    {
        i = token_start_idx_fn(view, ctx, i);
        if (i != MAX_SIZE_T_VALUE)
        {
            usize j = token_end_idx_fn(view, ctx, i);
            DResult op_result = push_back_token(new_dyn_array, view, i, j == MAX_SIZE_T_VALUE ? MAX_SIZE_T_VALUE : j - i);
            if (op_result != D_OK)
            {
                d_dyn_array_destroy(new_dyn_array);
                return op_result;
            }
            i = j;
        }
    }
    return D_OK;
}

static DResult split_string_view_owned(DDynArray *new_dyn_array, DStringView view, void *ctx, TokenBoundFn token_start_idx_fn, TokenBoundFn token_end_idx_fn, BufferOpts opts)
{
    DResult op_result = d_dyn_array_init_ptr_arr(new_dyn_array, DEFAULT_CAPACITY, _free_str, opts);
    if (op_result != D_OK)
        return op_result;
    return split_string_view(new_dyn_array, view, ctx, token_start_idx_fn, token_end_idx_fn, push_back_owned_sub_view_fn);
}

static DResult split_string_view_not_owned(DDynArray *new_dyn_array, DStringView view, void *ctx, TokenBoundFn token_start_idx_fn, TokenBoundFn token_end_idx_fn, BufferOpts opts)
{
    DResult op_result = d_dyn_array_init(new_dyn_array, sizeof(DStringView), DEFAULT_CAPACITY, NULL, opts);
    if (op_result != D_OK)
        return op_result;
    return split_string_view(new_dyn_array, view, ctx, token_start_idx_fn, token_end_idx_fn, push_back_sub_view_fn);
}

DResult d_string_view_split_by_char_of_str_owned(DDynArray *new_dyn_array, DStringView view, BufferOpts opts, DStringView set)
{
    return split_string_view_owned(new_dyn_array, view, &set, token_start_char_set, token_end_char_set, opts);
}

DResult d_string_view_split_by_char_owned(DDynArray *new_dyn_array, DStringView view, BufferOpts opts, char c)
{
    return split_string_view_owned(new_dyn_array, view, &c, token_start_char, token_end_char, opts);
}

DResult d_string_view_split_by_char_of_str_not_owned(DDynArray *new_dyn_array, DStringView view, BufferOpts opts, DStringView set)
{
    return split_string_view_not_owned(new_dyn_array, view, &set, token_start_char_set, token_end_char_set, opts);
}

DResult d_string_view_split_by_char_not_owned(DDynArray *new_dyn_array, DStringView view, BufferOpts opts, char c)
{
    return split_string_view_not_owned(new_dyn_array, view, &c, token_start_char, token_end_char, opts);
}

void d_string_view_dbg_print(DStringView *view)
{
    if (view == NULL)
    {
        printf("DStringView: NULL\n");
        return;
    }
    printf("DStringView { data: \"%.*s\", size: %zu }\n",
           (int)view->size, view->data, view->size);
}
