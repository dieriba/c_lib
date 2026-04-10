#include <string.h>
#include <stdlib.h>
#include "d_types.h"
#include "d_dyn_string.h"
#include "d_general_lib.h"

#define DYN_STRING_DEFAULT_CAPACITY 0x10

struct _DynString
{
    char *string;
    usize len;
    usize capacity;
};

#define dstring_get_char_at(dstring, pos) dstring->string[pos]
#define GROWTH_POLICTY 2
#define LIMIT (MAX_SIZE_T_VALUE / GROWTH_POLICTY)
DynString *d_dyn_string_new(void)
{
    struct _DynString *dstring;
    if ((dstring = malloc(sizeof(struct _DynString))) == NULL)
        return NULL;
    dstring->len = 0;
    if ((dstring->string = malloc(sizeof(char) * DYN_STRING_DEFAULT_CAPACITY + 1)) == NULL)
    {
        free(dstring);
        return NULL;
    }
    dstring->string[0] = 0;
    dstring->capacity = DYN_STRING_DEFAULT_CAPACITY;
    return (DynString *)dstring;
}

DynString *d_dyn_string_new_from_c_string(const char *str)
{
    struct _DynString *dstring;
    if ((dstring = malloc(sizeof(struct _DynString))) == NULL)
        return NULL;
    usize len = strlen(str);
    if ((dstring->string = malloc(sizeof(char) * (len + DYN_STRING_DEFAULT_CAPACITY + 1))) == NULL)
    {
        free(dstring);
        return NULL;
    }
    dstring->len = len;
    dstring->capacity = len + DYN_STRING_DEFAULT_CAPACITY;
    memcpy(dstring->string, str, len + 1);
    return (DynString *)dstring;
}

char *d_dyn_string_substr(DynString *dstring, usize pos, usize len)
{
    return d_substr(dstring->string, pos, len);
}

DynString *d_dyn_string_new_with_reserve(usize reserve)
{
    struct _DynString *dstring;
    if ((dstring = malloc(sizeof(struct _DynString))) == NULL)
        return NULL;
    reserve = reserve == 0 ? DYN_STRING_DEFAULT_CAPACITY : reserve;
    if ((dstring->string = malloc(sizeof(char) * reserve + 1)) == NULL)
    {
        free(dstring);
        return NULL;
    }
    dstring->len = 0;
    dstring->string[0] = 0;
    dstring->capacity = reserve;
    return (DynString *)dstring;
}

DynString *d_dyn_string_new_with_sub_string(const char *str, usize pos, usize len)
{
    usize str_len;
    if (str == NULL)
        return NULL;
    else if (pos > (str_len = strlen(str)))
        return NULL;
    len = pos + len > str_len ? str_len - pos : len;
    struct _DynString *dstring;
    if ((dstring = malloc(sizeof(struct _DynString))) == NULL)
        return NULL;
    if ((dstring->string = malloc(sizeof(char) * (len + DYN_STRING_DEFAULT_CAPACITY + 1))) == NULL)
    {
        free(dstring);
        return NULL;
    }
    if (len != 0)
        memcpy(dstring->string, str + pos, len);
    dstring->len = len;
    dstring->capacity = len + DYN_STRING_DEFAULT_CAPACITY;
    dstring->string[len] = 0;
    return (DynString *)dstring;
}

DynString *d_dyn_string_new_from_dstring(DynString *dstring)
{
    if (dstring == NULL)
        return NULL;
    return d_dyn_string_new_with_sub_string(dstring->string, 0, dstring->len);
}

DynString *d_dyn_string_sub_string_in_place(DynString *dstring, usize pos, usize len)
{
    if (pos > dstring->len)
        return NULL;
    struct _DynString *rdstring = (struct _DynString *)dstring;
    usize string_len = rdstring->len;
    len = pos + len > string_len ? string_len - pos : len;
    char *string = dstring->string;
    memmove(string, string + pos, len);
    rdstring->len = len;
    rdstring->string[len] = 0;
    return dstring;
}

usize d_dyn_string_get_capacity(DynString *dstring)
{
    struct _DynString *rdstring = (struct _DynString *)dstring;
    return rdstring->capacity;
}

DynString *d_dyn_string_resize(DynString *dstring, usize len)
{
    struct _DynString *rdstring = (struct _DynString *)dstring;
    usize string_len = rdstring->len;
    if (len > string_len)
    {
        usize count = len - string_len;
        if (d_dyn_string_increase_capacity_if_needed(dstring, count) == NULL)
            return NULL;
        memset(rdstring->string + string_len, 0, count + 1);
    }
    rdstring->len = len;
    return dstring;
}

DynString *d_dyn_string_increase_capacity_if_needed(DynString *dstring, usize to_copy)
{
    struct _DynString *rdstring = (struct _DynString *)dstring;

    if (MAX_SIZE_T_VALUE - rdstring->len < to_copy)
        return NULL;
    usize needed = rdstring->len + to_copy;
    if (needed <= rdstring->capacity)
        return dstring;

    if (needed > LIMIT)
        return NULL;
    usize new_capacity = needed * GROWTH_POLICTY;
    char *tmp = realloc(rdstring->string, new_capacity + 1);
    if (tmp == NULL)
        return NULL;

    rdstring->string = tmp;
    rdstring->capacity = new_capacity;
    return dstring;
}

DynString *d_dyn_string_push_char(DynString *dstring, char c)
{
    struct _DynString *rdstring = (struct _DynString *)dstring;
    if (d_dyn_string_increase_capacity_if_needed(dstring, 1) == NULL)
        return NULL;
    rdstring->string[rdstring->len++] = c;
    rdstring->string[rdstring->len] = 0;
    return dstring;
}

char d_dyn_string_get_char_at(DynString *dstring, usize i)
{
#ifdef BOUNDARY_CHECK
    if (i >= dstring->len)
    {
        // halt prog execution
    }
#endif

    return dstring->string[i];
}

DynString *d_dyn_string_push_str_with_len(DynString *dstring, const char *str_to_append, usize len)
{
    struct _DynString *rdstring = (struct _DynString *)dstring;
    if (d_dyn_string_increase_capacity_if_needed(dstring, len) == NULL)
        return NULL;
    memcpy(rdstring->string + rdstring->len, str_to_append, len);
    rdstring->len += len;
    rdstring->string[rdstring->len] = 0;
    return dstring;
}

DynString *d_dyn_string_push_c_str(DynString *dstring, const char *str_to_append)
{
    return d_dyn_string_push_str_with_len(dstring, str_to_append, strlen(str_to_append));
}

DynString *d_dyn_string_merge(DynString *dstring1, DynString *dstring2)
{
    return d_dyn_string_push_str_with_len(dstring1, dstring2->string, dstring2->len);
}

DynString *d_dyn_string_replace_from_str(DynString *dstring, const char *str)
{
    struct _DynString *rdstring = (struct _DynString *)dstring;
    usize to_copy = str == NULL ? 0 : strlen(str);
    if (to_copy > dstring->len && d_dyn_string_increase_capacity_if_needed(dstring, to_copy - dstring->len) == NULL)
        return NULL;
    if (to_copy != 0)
        memcpy(rdstring->string, str, to_copy);
    rdstring->len = to_copy;
    rdstring->string[rdstring->len] = 0;
    return dstring;
}

DynString *d_dyn_string_replace_from_dstring(DynString *dstring, const DynString *to_copy)
{
    return d_dyn_string_replace_from_str(dstring, to_copy->string);
}

int32 d_dyn_string_compare(DynString *dstring1, DynString *dstring2)
{
    return strcmp(dstring1->string, dstring2->string);
}

int32 d_dyn_string_compare_against_c_string(DynString *dstring, const char *c_str)
{
    return strcmp(dstring->string, c_str);
}

bool d_dyn_string_starts_with_char(DynString *dstring, char c)
{
    return dstring->len == 0 ? false : dstring_get_char_at(dstring, 0) == c;
}

bool d_dyn_string_ends_with_char(DynString *dstring, char c)
{
    return dstring->len == 0 ? false : dstring_get_char_at(dstring, dstring->len - 1) == c;
}

bool d_dyn_string_starts_with_str(DynString *dstring, const char *str_to_find)
{
    usize len_str_to_find = strlen(str_to_find);
    if (len_str_to_find > dstring->len)
        return false;
    return memcmp(dstring->string, str_to_find, len_str_to_find) == 0;
}

bool d_dyn_string_ends_with_str(DynString *dstring, const char *str_to_find)
{
    usize len_str_to_find = strlen(str_to_find);
    if (len_str_to_find > dstring->len)
        return false;
    char *addr = (dstring->string + dstring->len) - len_str_to_find;
    return memcmp(addr, str_to_find, len_str_to_find) == 0;
}

typedef bool (*char_match_fn)(char ch, const void *ctx);

bool match_char(char c1, const void *c2)
{
    return c1 == *(char *)c2;
}

bool not_match_char(char c1, const void *c2)
{
    return !match_char(c1, c2);
}

bool char_in_set(char c, const void *set)
{
    return strchr((const char *)set, c) != NULL;
}

bool not_char_in_set(char c, const void *set)
{
    return !char_in_set(c, set);
}

bool match_predicate(char c, const void *ctx)
{
    match fn = (match)ctx;
    return fn(c);
}

bool not_match_predicate(char c, const void *ctx)
{
    return !match_predicate(c, ctx);
}

static usize find_first_from_index(const DynString *dstring, usize pos, char_match_fn matches, const void *ctx)
{
    if (dstring == NULL || pos >= dstring->len)
        return MAX_SIZE_T_VALUE;

    for (usize i = pos; i < dstring->len; ++i)
    {
        if (matches(dstring->string[i], ctx))
            return i;
    }
    return MAX_SIZE_T_VALUE;
}

static usize find_last_from_index(const DynString *dstring, usize pos, char_match_fn matches, const void *ctx)
{
    if (dstring == NULL || dstring->len == 0)
        return MAX_SIZE_T_VALUE;
    pos = pos >= dstring->len ? dstring->len - 1 : pos;
    usize i = pos;
    while (1)
    {
        if (matches(dstring->string[i], ctx))
            return i;
        else if (i == 0)
            break;
        --i;
    }

    return MAX_SIZE_T_VALUE;
}

usize d_dyn_string_find_first_matching_char_from_index(DynString *dstring, char c, usize pos)
{
    return find_first_from_index(dstring, pos, match_char, &c);
}

usize d_dyn_string_find_first_matching_char_from_start(DynString *dstring, char c)
{
    return d_dyn_string_find_first_matching_char_from_index(dstring, c, 0);
}

usize d_dyn_string_find_first_not_matching_char_from_index(DynString *dstring, char c, usize pos)
{
    return find_first_from_index(dstring, pos, not_match_char, &c);
}

usize d_dyn_string_find_first_not_matching_char_from_start(DynString *dstring, char c)
{
    return d_dyn_string_find_first_not_matching_char_from_index(dstring, c, 0);
}

usize d_dyn_string_find_last_matching_char_from_index(DynString *dstring, char c, usize pos)
{
    return find_last_from_index(dstring, pos, match_char, &c);
}

usize d_dyn_string_find_last_matching_char_from_end(DynString *dstring, char c)
{
    return d_dyn_string_find_last_matching_char_from_index(dstring, c, dstring->len);
}

usize d_dyn_string_find_last_not_matching_char_from_index(DynString *dstring, char c, usize pos)
{
    return find_last_from_index(dstring, pos, not_match_char, &c);
}

usize d_dyn_string_find_last_not_matching_char_from_end(DynString *dstring, char c)
{
    return d_dyn_string_find_last_not_matching_char_from_index(dstring, c, dstring->len);
}

usize d_dyn_string_find_first_matching_str_from_index(DynString *dstring, const char *str, usize pos)
{
    if (pos >= dstring->len)
        return MAX_SIZE_T_VALUE;
    char *string = dstring->string;
    usize len = strlen(str);
    usize string_len = dstring->len;
    for (usize i = pos; i < string_len; ++i)
    {
        if (string[i] == str[0] && string_len - i >= len && memcmp(string + i, str, len) == 0)
            return (string + i) - string;
    }
    return MAX_SIZE_T_VALUE;
}

usize d_dyn_string_find_first_matching_str_from_start(DynString *dstring, const char *str)
{
    return d_dyn_string_find_first_matching_str_from_index(dstring, str, 0);
}

usize d_dyn_string_find_last_matching_str_from_index(DynString *dstring, const char *str_to_match, usize pos)
{
    usize len_dstring;
    if ((len_dstring = dstring->len) == 0)
        return MAX_SIZE_T_VALUE;
    pos = pos >= len_dstring ? len_dstring - 1 : pos;
    char *string = dstring->string;
    usize len_str_to_match = strlen(str_to_match);
    while (1)
    {
        if ((string[pos] == str_to_match[0]) && (len_dstring - pos >= len_str_to_match) && memcmp(string + pos, str_to_match, len_str_to_match) == 0)
            return pos;
        else if (pos == 0)
            break;
        --pos;
    }
    return MAX_SIZE_T_VALUE;
}

usize d_dyn_string_find_last_matching_str_from_end(DynString *dstring, const char *str)
{
    return d_dyn_string_find_last_matching_str_from_index(dstring, str, dstring->len);
}

usize d_dyn_string_find_first_char_in_set_from_index(DynString *dstring, const char *set, usize pos)
{
    return find_first_from_index(dstring, pos, char_in_set, set);
}

usize d_dyn_string_find_first_char_in_str_from_start(DynString *dstring, const char *str)
{
    return d_dyn_string_find_first_char_in_set_from_index(dstring, str, 0);
}

usize d_dyn_string_find_first_char_not_in_set_from_index(DynString *dstring, const char *set, usize pos)
{
    return find_first_from_index(dstring, pos, not_char_in_set, set);
}

usize d_dyn_string_find_first_char_not_in_str_from_start(DynString *dstring, const char *str)
{
    return d_dyn_string_find_first_char_not_in_set_from_index(dstring, str, 0);
}

usize d_dyn_string_find_last_char_in_set_from_index(DynString *dstring, const char *set, usize pos)
{
    return find_last_from_index(dstring, pos, char_in_set, set);
}

usize d_dyn_string_find_last_char_in_str_from_end(DynString *dstring, const char *str)
{
    return d_dyn_string_find_last_char_in_set_from_index(dstring, str, dstring->len);
}

usize d_dyn_string_find_last_char_not_in_str_from_index(DynString *dstring, const char *set, usize pos)
{
    return find_last_from_index(dstring, pos, not_char_in_set, set);
}

usize d_dyn_string_find_last_char_not_in_str_from_end(DynString *dstring, const char *str)
{
    return d_dyn_string_find_last_char_not_in_str_from_index(dstring, str, dstring->len);
}

usize d_dyn_string_find_first_matching_predicate_from_index(DynString *dstring, match fn, usize pos)
{
    return find_first_from_index(dstring, pos, match_predicate, fn);
}

usize d_dyn_string_find_first_matching_predicate_from_start(DynString *dstring, match fn)
{
    return d_dyn_string_find_first_matching_predicate_from_index(dstring, fn, 0);
}

usize d_dyn_string_find_last_matching_predicate_from_index(DynString *dstring, match fn, usize pos)
{
    return find_last_from_index(dstring, pos, match_predicate, fn);
}

usize d_dyn_string_find_last_matching_predicate_from_end(DynString *dstring, match fn)
{
    return d_dyn_string_find_last_matching_predicate_from_index(dstring, fn, dstring->len);
}

usize d_dyn_string_find_first_not_matching_predicate_from_index(DynString *dstring, match fn, usize pos)
{
    return find_first_from_index(dstring, pos, not_match_predicate, fn);
}

usize d_dyn_string_find_first_not_matching_predicate_from_start(DynString *dstring, match fn)
{
    return d_dyn_string_find_first_not_matching_predicate_from_index(dstring, fn, 0);
}

usize d_dyn_string_find_last_not_matching_predicate_from_index(DynString *dstring, match fn, usize pos)
{
    return find_last_from_index(dstring, pos, not_match_predicate, fn);
}

usize d_dyn_string_find_last_not_matching_predicate_from_end(DynString *dstring, match fn)
{
    return d_dyn_string_find_last_not_matching_predicate_from_index(dstring, fn, dstring->len);
}

DynString *d_dyn_string_trim_left_by_char_in_place(DynString *dstring, char c)
{
    usize i = d_dyn_string_find_first_not_matching_char_from_start(dstring, c);
    if (i == MAX_SIZE_T_VALUE)
        i = dstring->len;
    return d_dyn_string_sub_string_in_place(dstring, i, dstring->len - i);
}

DynString *d_dyn_string_trim_left_by_char_new(DynString *dstring, char c)
{
    usize i = d_dyn_string_find_first_not_matching_char_from_start(dstring, c);
    if (i == MAX_SIZE_T_VALUE)
        i = dstring->len;
    return d_dyn_string_new_with_sub_string(dstring->string, i, dstring->len - i);
}

DynString *d_dyn_string_trim_left_by_predicate_in_place(DynString *dstring, match fn)
{
    usize i = d_dyn_string_find_first_not_matching_predicate_from_start(dstring, fn);
    if (i == MAX_SIZE_T_VALUE)
        i = dstring->len;
    return d_dyn_string_sub_string_in_place(dstring, i, dstring->len - i);
}

DynString *d_dyn_string_trim_left_by_predicate_new(DynString *dstring, match fn)
{
    usize i = d_dyn_string_find_first_not_matching_predicate_from_start(dstring, fn);
    if (i == MAX_SIZE_T_VALUE)
        i = dstring->len;
    return d_dyn_string_new_with_sub_string(dstring->string, i, dstring->len - i);
}

DynString *d_dyn_string_trim_right_by_char_in_place(DynString *dstring, char c)
{
    usize i = d_dyn_string_find_last_not_matching_char_from_end(dstring, c);
    return d_dyn_string_sub_string_in_place(dstring, 0, i == MAX_SIZE_T_VALUE ? 0 : i + 1);
}

DynString *d_dyn_string_trim_right_by_char_new(DynString *dstring, char c)
{
    usize i = d_dyn_string_find_last_not_matching_char_from_end(dstring, c);
    return d_dyn_string_new_with_sub_string(dstring->string, 0, i == MAX_SIZE_T_VALUE ? 0 : i + 1);
}

DynString *d_dyn_string_trim_right_by_predicate_in_place(DynString *dstring, match fn)
{
    usize i = d_dyn_string_find_last_not_matching_predicate_from_end(dstring, fn);
    return d_dyn_string_sub_string_in_place(dstring, 0, i == MAX_SIZE_T_VALUE ? 0 : i + 1);
}

DynString *d_dyn_string_trim_right_by_predicate_new(DynString *dstring, match fn)
{
    usize i = d_dyn_string_find_last_not_matching_predicate_from_end(dstring, fn);
    return d_dyn_string_new_with_sub_string(dstring->string, 0, i == MAX_SIZE_T_VALUE ? 0 : i + 1);
}

DPointerArray *d_dyn_string_split_by_char_of_str(DynString *dstring, const char *str)
{
    DPointerArray *vec = d_pointer_array_new(1, true, free);
    if (vec == NULL)
        return NULL;

    usize len = dstring->len;
    char *string = dstring->string;

    usize str_len = strlen(str);
    for (usize i = 0; i < len;)
    {
        while (i < len && memchr(str, (int)string[i], str_len) != NULL)
            ++i;
        if (i != len)
        {
            usize j = d_dyn_string_find_first_char_in_set_from_index(dstring, str, i);
            char *str = d_dyn_string_substr(dstring, i, j == MAX_SIZE_T_VALUE ? MAX_SIZE_T_VALUE : j - i);
            if (str == NULL || d_pointer_array_push_back(vec, str) == NULL)
            {
                d_pointer_array_destroy(&vec);
                return NULL;
            }
            i = j;
        }
        else
            break;
    }
    return vec;
}

DPointerArray *d_dyn_string_split_by_char(DynString *dstring, char c)
{
    DPointerArray *vec = d_pointer_array_new(10, true, free);
    if (vec == NULL)
        return NULL;
    usize len = dstring->len;
    char *str = dstring->string;
    for (usize i = 0; i < len;)
    {
        while (i < len && str[i] == c)
            ++i;
        if (i != len)
        {
            usize j = d_dyn_string_find_first_matching_char_from_index(dstring, c, i);
            char *str = d_dyn_string_substr(dstring, i, j == MAX_SIZE_T_VALUE ? MAX_SIZE_T_VALUE : j - i);
            if (str == NULL || d_pointer_array_push_back(vec, str) == NULL)
            {
                d_pointer_array_destroy(&vec);
                return NULL;
            }
            i = j;
        }
        else
            break;
    }
    return vec;
}

void d_dyn_string_destroy(DynString **dstring)
{
    if (dstring == NULL || *dstring == NULL)
        return;
    struct _DynString *rdstring = ((struct _DynString *)*dstring);
    free(rdstring->string);
    free(rdstring);
    *dstring = NULL;
}