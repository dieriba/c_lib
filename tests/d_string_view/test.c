#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "d_test.h"
#include "d_string_view.h"
#include "d_dyn_string.h"
#include "d_dyn_array.h"
#include "d_types.h"

#ifndef MAX_SIZE_T_VALUE
#define MAX_SIZE_T_VALUE ((usize) - 1)
#endif

#define ARRAY_DEFAULT_OPTS 0

static bool is_space_like(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static bool is_digit_like(char c)
{
    return c >= '0' && c <= '9';
}

static bool is_lower_like(char c)
{
    return c >= 'a' && c <= 'z';
}

static void expect_view(DStringView view, const char *data, usize size)
{
    D_TEST_EXPR(d_string_view_len(view) == size);
    D_TEST_NOT_NULL(view.data);
    if (size == 0)
    {
        D_TEST_EXPR(view.size == 0);
        return;
    }
    D_TEST_MEM_EQ(view.data, data, size);
}

static void expect_view_cstr(DStringView view, const char *expected)
{
    expect_view(view, expected, strlen(expected));
}

static void expect_split_elem(DDynArray *arr, usize index, const char *expected)
{
    char *elem = NULL;
    D_TEST_EXPR(d_dyn_array_get_elem_at(arr, index, &elem) == D_OK);
    D_TEST_NOT_NULL(elem);
    D_TEST_STR_EQ(elem, expected);
}

static void expect_split_size(DDynArray *arr, usize expected)
{
    usize size = 0;
    D_TEST_EXPR(d_dyn_array_get_size(arr, &size) == D_OK);
    D_TEST_EXPR(size == expected);
}

static void expect_split_view_elem(DDynArray *arr, usize index, const char *expected)
{
    DStringView tok = {0};
    D_TEST_EXPR(d_dyn_array_get_elem_at(arr, index, &tok) == D_OK);
    D_TEST_EXPR(tok.size == strlen(expected));
    D_TEST_MEM_EQ(tok.data, expected, tok.size);
}

/* construction / basic invariants */
static void test_from_parts_keeps_pointer_and_size_for_valid_buffer(void)
{
    const char buf[] = {'a', 'b', '\0', 'c', 'd'};
    DStringView view = d_string_view_from_parts(buf, sizeof(buf));
    D_TEST_EXPR(view.data == buf);
    D_TEST_EXPR(view.size == sizeof(buf));
}

static void test_from_parts_null_normalizes_to_empty_view(void)
{
    DStringView view = d_string_view_from_parts(NULL, 1234);
    D_TEST_NOT_NULL(view.data);
    D_TEST_EXPR(view.size == 0);
    D_TEST_EXPR(d_string_view_is_empty(view));
}

static void test_from_c_string_null_returns_empty_view(void)
{
    DStringView view = d_string_view_from_c_string(NULL);
    D_TEST_NOT_NULL(view.data);
    D_TEST_EXPR(view.size == 0);
}

static void test_from_c_string_stops_at_first_nul(void)
{
    const char buf[] = {'a', 'b', '\0', 'c'};
    DStringView view = d_string_view_from_c_string(buf);
    D_TEST_EXPR(view.data == buf);
    D_TEST_EXPR(view.size == 2);
}

static void test_from_dyn_string_null_returns_empty_view(void)
{
    DStringView view = d_string_view_from_dyn_string(NULL);
    D_TEST_NOT_NULL(view.data);
    D_TEST_EXPR(view.size == 0);
}

static void test_from_dyn_string_points_to_dyn_string_storage(void)
{
    DDynString s;
    D_TEST_EXPR(d_dyn_string_init_from_c_string(&s, "hello") == D_OK);
    DStringView view = d_string_view_from_dyn_string(&s);
    D_TEST_EXPR(view.data == d_dyn_string_get_string(&s));
    expect_view_cstr(view, "hello");
    d_dyn_string_destroy(&s);
}

static void test_from_dyn_string_with_empty_string(void)
{
    DDynString s;
    D_TEST_EXPR(d_dyn_string_init_from_c_string(&s, "") == D_OK);
    DStringView view = d_string_view_from_dyn_string(&s);
    D_TEST_EXPR(view.size == 0);
    d_dyn_string_destroy(&s);
}

static void test_len_data_and_is_empty_are_consistent(void)
{
    DStringView empty = d_string_view_from_c_string("");
    DStringView non_empty = d_string_view_from_c_string("x");
    D_TEST_EXPR(d_string_view_is_empty(empty));
    D_TEST_EXPR(!d_string_view_is_empty(non_empty));
    D_TEST_EXPR(d_string_view_len(non_empty) == 1);
    D_TEST_EXPR(d_string_view_data(non_empty)[0] == 'x');
}

static void test_get_char_at_reads_embedded_nul_and_edges(void)
{
    const char buf[] = {'A', '\0', 'B'};
    DStringView view = d_string_view_from_parts(buf, sizeof(buf));
    char out;
    D_TEST_EXPR(d_string_view_get_char_at(view, 0, &out) == D_OK);
    D_TEST_EXPR(out == 'A');
    D_TEST_EXPR(d_string_view_get_char_at(view, 1, &out) == D_OK);
    D_TEST_EXPR(out == '\0');
    D_TEST_EXPR(d_string_view_get_char_at(view, 2, &out) == D_OK);
    D_TEST_EXPR(out == 'B');
    D_TEST_EXPR(d_string_view_get_char_at(view, 3, &out) == D_ERR_INVALID_ARG);
}

static void test_subview_middle_does_not_allocate_and_points_inside_original(void)
{
    const char *s = "0123456789";
    DStringView view = d_string_view_from_c_string(s);
    DStringView sub = d_string_view_subview(view, 3, 4);
    D_TEST_EXPR(sub.data == s + 3);
    expect_view_cstr(sub, "3456");
}

static void test_subview_clamps_size_past_end(void)
{
    DStringView view = d_string_view_from_c_string("abcdef");
    expect_view_cstr(d_string_view_subview(view, 4, 999), "ef");
}

static void test_subview_pos_equal_len_returns_valid_empty_string_pointer(void)
{
    DStringView view = d_string_view_from_c_string("abc");
    DStringView sub = d_string_view_subview(view, 3, 99);
    D_TEST_NOT_NULL(sub.data);
    D_TEST_EXPR(sub.size == 0);
    D_TEST_STR_EQ(sub.data, "");
}

static void test_subview_pos_greater_than_len_returns_empty_view(void)
{
    DStringView view = d_string_view_from_c_string("abc");
    DStringView sub = d_string_view_subview(view, 4, 1);
    D_TEST_NOT_NULL(sub.data);
    D_TEST_EXPR(sub.size == 0);
}

static void test_subview_empty_view_at_pos_zero_returns_empty_view(void)
{
    DStringView view = d_string_view_from_parts(NULL, 0);
    DStringView sub = d_string_view_subview(view, 0, 1);
    D_TEST_NOT_NULL(sub.data);
    D_TEST_EXPR(sub.size == 0);
}

static void test_subview_zero_size_in_middle_keeps_position_pointer(void)
{
    const char *s = "abcdef";
    DStringView view = d_string_view_from_c_string(s);
    DStringView sub = d_string_view_subview(view, 2, 0);
    D_TEST_EXPR(sub.data == s + 2);
    D_TEST_EXPR(sub.size == 0);
}

static void test_substr_allocates_copy_and_terminates(void)
{
    DStringView view = d_string_view_from_c_string("abcdef");
    char *sub = d_string_view_substr(view, 2, 3);
    D_TEST_NOT_NULL(sub);
    D_TEST_STR_EQ(sub, "cde");
    D_TEST_EXPR(sub[3] == '\0');
    free(sub);
}

static void test_substr_pos_at_len_returns_allocated_empty_string(void)
{
    DStringView view = d_string_view_from_c_string("abc");
    char *sub = d_string_view_substr(view, 3, 9);
    D_TEST_NOT_NULL(sub);
    D_TEST_STR_EQ(sub, "");
    free(sub);
}

static void test_substr_pos_past_len_returns_null(void)
{
    DStringView view = d_string_view_from_c_string("abc");
    char *sub = d_string_view_substr(view, 4, 1);
    D_TEST_NULL(sub);
}

static void test_substr_empty_view_at_pos_zero_returns_allocated_empty_string(void)
{
    DStringView view = d_string_view_from_parts(NULL, 0);
    char *sub = d_string_view_substr(view, 0, 1);
    D_TEST_NOT_NULL(sub);
    D_TEST_STR_EQ(sub, "");
    free(sub);
}

static void test_substr_zero_len_allocates_empty_string(void)
{
    DStringView view = d_string_view_from_c_string("abc");
    char *sub = d_string_view_substr(view, 1, 0);
    D_TEST_NOT_NULL(sub);
    D_TEST_STR_EQ(sub, "");
    free(sub);
}

static void test_substr_should_respect_view_size_with_embedded_nul_before_pos(void)
{
    const char buf[] = {'a', '\0', 'b', 'c', 'd'};
    DStringView view = d_string_view_from_parts(buf, sizeof(buf));
    char *sub = d_string_view_substr(view, 2, 3);
    D_TEST_NOT_NULL(sub);
    if (sub != NULL)
    {
        D_TEST_MEM_EQ(sub, "bcd", 3);
        D_TEST_EXPR(sub[3] == '\0');
        free(sub);
    }
}

static void test_substr_should_copy_embedded_nul_inside_result(void)
{
    const char buf[] = {'x', 'a', '\0', 'b', 'y'};
    DStringView view = d_string_view_from_parts(buf, sizeof(buf));
    char *sub = d_string_view_substr(view, 1, 3);
    D_TEST_NOT_NULL(sub);
    if (sub != NULL)
    {
        D_TEST_EXPR(sub[0] == 'a');
        D_TEST_EXPR(sub[1] == '\0');
        free(sub);
    }
}

/* compare / equality */
static void test_compare_equal_views_returns_true(void)
{
    D_TEST_EXPR(d_string_view_compare(d_string_view_from_c_string("abc"), d_string_view_from_c_string("abc")) == true);
}

static void test_compare_same_prefix_different_lengths_returns_false(void)
{
    D_TEST_EXPR(d_string_view_compare(d_string_view_from_c_string("abc"), d_string_view_from_c_string("abcd")) == false);
}

static void test_compare_different_same_length_views_return_false(void)
{
    D_TEST_EXPR(d_string_view_compare(d_string_view_from_c_string("abc"), d_string_view_from_c_string("abd")) == false);
    D_TEST_EXPR(d_string_view_compare(d_string_view_from_c_string("abd"), d_string_view_from_c_string("abc")) == false);
}

static void test_compare_handles_embedded_nul_by_size(void)
{
    const char a[] = {'a', '\0', 'b'};
    const char b[] = {'a', '\0', 'c'};
    D_TEST_EXPR(d_string_view_compare(d_string_view_from_parts(a, 3), d_string_view_from_parts(b, 3)) == false);
}

static void test_compare_against_c_string_equal_and_not_equal(void)
{
    DStringView view = d_string_view_from_c_string("hello");
    D_TEST_EXPR(d_string_view_compare_against_c_string(view, "hello") == true);
    D_TEST_EXPR(d_string_view_compare_against_c_string(view, "world") == false);
    D_TEST_EXPR(d_string_view_compare_against_c_string(view, "hell")  == false);
}

static void test_equals_views_with_embedded_nul(void)
{
    const char a[] = {'x', '\0', 'y'};
    const char b[] = {'x', '\0', 'y'};
    D_TEST_EXPR(d_string_view_compare(d_string_view_from_parts(a, 3), d_string_view_from_parts(b, 3)) == true);
}

static void test_equals_c_string_rejects_view_with_extra_embedded_data(void)
{
    const char a[] = {'a', 'b', '\0', 'c'};
    D_TEST_EXPR(d_string_view_compare_against_c_string(d_string_view_from_parts(a, 4), "ab") == false);
}

static void test_empty_view_equals_empty_c_string(void)
{
    D_TEST_EXPR(d_string_view_compare_against_c_string(d_string_view_from_parts(NULL, 0), "") == true);
}

/* starts/ends */
static void test_starts_and_ends_with_char_basic_edges(void)
{
    DStringView view = d_string_view_from_c_string("abc");
    D_TEST_EXPR(d_string_view_starts_with_char(view, 'a'));
    D_TEST_EXPR(!d_string_view_starts_with_char(view, 'b'));
    D_TEST_EXPR(d_string_view_ends_with_char(view, 'c'));
    D_TEST_EXPR(!d_string_view_ends_with_char(view, 'b'));
    D_TEST_EXPR(!d_string_view_starts_with_char(d_string_view_from_c_string(""), 'x'));
    D_TEST_EXPR(!d_string_view_ends_with_char(d_string_view_from_c_string(""), 'x'));
}

static void test_starts_and_ends_with_embedded_nul_views(void)
{
    const char data[] = {'a', '\0', 'b', 'c'};
    const char prefix[] = {'a', '\0', 'b'};
    const char suffix[] = {'\0', 'b', 'c'};
    DStringView view = d_string_view_from_parts(data, 4);
    D_TEST_EXPR(d_string_view_starts_with_view(view, d_string_view_from_parts(prefix, 3)));
    D_TEST_EXPR(d_string_view_ends_with_view(view, d_string_view_from_parts(suffix, 3)));
}

static void test_starts_ends_empty_prefix_suffix_are_true(void)
{
    DStringView view = d_string_view_from_c_string("abc");
    DStringView empty = d_string_view_from_c_string("");
    D_TEST_EXPR(d_string_view_starts_with_view(view, empty));
    D_TEST_EXPR(d_string_view_ends_with_view(view, empty));
    D_TEST_EXPR(d_string_view_starts_with_c_string(view, ""));
    D_TEST_EXPR(d_string_view_ends_with_c_string(view, ""));
}

static void test_starts_ends_prefix_suffix_longer_than_view_are_false(void)
{
    DStringView view = d_string_view_from_c_string("abc");
    D_TEST_EXPR(!d_string_view_starts_with_c_string(view, "abcd"));
    D_TEST_EXPR(!d_string_view_ends_with_c_string(view, "zabc"));
}

/* char find */
static void test_find_first_matching_char_from_index(void)
{
    DStringView view = d_string_view_from_c_string("abacad");
    D_TEST_EXPR(d_string_view_find_first_matching_char_from_index(view, 'a', 0) == 0);
    D_TEST_EXPR(d_string_view_find_first_matching_char_from_index(view, 'a', 1) == 2);
    D_TEST_EXPR(d_string_view_find_first_matching_char_from_index(view, 'x', 0) == MAX_SIZE_T_VALUE);
}

static void test_find_first_matching_char_pos_at_or_past_end_returns_not_found(void)
{
    DStringView view = d_string_view_from_c_string("abc");
    D_TEST_EXPR(d_string_view_find_first_matching_char_from_index(view, 'a', 3) == MAX_SIZE_T_VALUE);
    D_TEST_EXPR(d_string_view_find_first_matching_char_from_index(view, 'a', 999) == MAX_SIZE_T_VALUE);
}

static void test_find_first_not_matching_char(void)
{
    DStringView view = d_string_view_from_c_string("aaab");
    D_TEST_EXPR(d_string_view_find_first_not_matching_char_from_start(view, 'a') == 3);
    D_TEST_EXPR(d_string_view_find_first_not_matching_char_from_start(d_string_view_from_c_string("aaaa"), 'a') == MAX_SIZE_T_VALUE);
}

static void test_find_last_matching_char_clamps_large_pos_to_end(void)
{
    DStringView view = d_string_view_from_c_string("abacad");
    D_TEST_EXPR(d_string_view_find_last_matching_char_from_index(view, 'a', 999) == 4);
    D_TEST_EXPR(d_string_view_find_last_matching_char_from_index(view, 'a', 3) == 2);
    D_TEST_EXPR(d_string_view_find_last_matching_char_from_index(view, 'x', 999) == MAX_SIZE_T_VALUE);
}

static void test_find_last_not_matching_char(void)
{
    DStringView view = d_string_view_from_c_string("baaa");
    D_TEST_EXPR(d_string_view_find_last_not_matching_char_from_end(view, 'a') == 0);
    D_TEST_EXPR(d_string_view_find_last_not_matching_char_from_end(d_string_view_from_c_string("aaaa"), 'a') == MAX_SIZE_T_VALUE);
}

static void test_find_char_handles_embedded_nul(void)
{
    const char data[] = {'a', '\0', 'b', '\0'};
    DStringView view = d_string_view_from_parts(data, sizeof(data));
    D_TEST_EXPR(d_string_view_find_first_matching_char_from_start(view, '\0') == 1);
    D_TEST_EXPR(d_string_view_find_last_matching_char_from_end(view, '\0') == 3);
}

static void test_find_char_on_empty_views_returns_not_found(void)
{
    D_TEST_EXPR(d_string_view_find_first_matching_char_from_start(d_string_view_from_parts(NULL, 0), 'x') == MAX_SIZE_T_VALUE);
    D_TEST_EXPR(d_string_view_find_last_matching_char_from_end(d_string_view_from_parts(NULL, 0), 'x') == MAX_SIZE_T_VALUE);
    D_TEST_EXPR(d_string_view_find_first_matching_char_from_start(d_string_view_from_c_string(""), 'x') == MAX_SIZE_T_VALUE);
    D_TEST_EXPR(d_string_view_find_last_matching_char_from_end(d_string_view_from_c_string(""), 'x') == MAX_SIZE_T_VALUE);
}

/* view find */
static void test_find_first_matching_view_basic_and_overlapping(void)
{
    DStringView view = d_string_view_from_c_string("aaaa");
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_start(view, D_STRING_VIEW_FROM_LITERAL("aa")) == 0);
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_index(view, D_STRING_VIEW_FROM_LITERAL("aa"), 1) == 1);
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_index(view, D_STRING_VIEW_FROM_LITERAL("aa"), 3) == MAX_SIZE_T_VALUE);
}

static void test_find_last_matching_view_basic_and_overlapping(void)
{
    DStringView view = d_string_view_from_c_string("aaaa");
    D_TEST_EXPR(d_string_view_find_last_matching_view_from_end(view, D_STRING_VIEW_FROM_LITERAL("aa")) == 2);
    D_TEST_EXPR(d_string_view_find_last_matching_view_from_index(view, D_STRING_VIEW_FROM_LITERAL("aa"), 1) == 1);
}

static void test_find_matching_view_with_embedded_nul_needle(void)
{
    const char hay[] = {'a', '\0', 'b', 'x', 'a', '\0', 'b'};
    const char needle[] = {'a', '\0', 'b'};
    DStringView view = d_string_view_from_parts(hay, sizeof(hay));
    DStringView ndl = d_string_view_from_parts(needle, sizeof(needle));
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_start(view, ndl) == 0);
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_index(view, ndl, 1) == 4);
    D_TEST_EXPR(d_string_view_find_last_matching_view_from_end(view, ndl) == 4);
}

static void test_find_first_empty_needle_at_start_and_middle(void)
{
    DStringView view = d_string_view_from_c_string("abc");
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_index(view, D_STRING_VIEW_FROM_LITERAL(""), 0) == 0);
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_index(view, D_STRING_VIEW_FROM_LITERAL(""), 2) == 2);
}

static void test_find_last_empty_needle_clamps_to_len(void)
{
    DStringView view = d_string_view_from_c_string("abc");
    D_TEST_EXPR(d_string_view_find_last_matching_view_from_index(view, D_STRING_VIEW_FROM_LITERAL(""), 0) == 0);
    D_TEST_EXPR(d_string_view_find_last_matching_view_from_index(view, D_STRING_VIEW_FROM_LITERAL(""), 2) == 2);
    D_TEST_EXPR(d_string_view_find_last_matching_view_from_index(view, D_STRING_VIEW_FROM_LITERAL(""), 999) == 3);
}

static void test_find_view_on_empty_haystack(void)
{
    DStringView empty = d_string_view_from_c_string("");
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_start(empty, D_STRING_VIEW_FROM_LITERAL("a")) == MAX_SIZE_T_VALUE);
    D_TEST_EXPR(d_string_view_find_last_matching_view_from_end(empty, D_STRING_VIEW_FROM_LITERAL("a")) == MAX_SIZE_T_VALUE);
}

/* set find */
static void test_find_first_char_in_set_and_not_in_set(void)
{
    DStringView view = d_string_view_from_c_string("abc123");
    D_TEST_EXPR(d_string_view_find_first_char_in_set_from_start(view, D_STRING_VIEW_FROM_LITERAL("0123456789")) == 3);
    D_TEST_EXPR(d_string_view_find_first_char_not_in_set_from_start(view, D_STRING_VIEW_FROM_LITERAL("abc")) == 3);
}

static void test_find_last_char_in_set_and_not_in_set(void)
{
    DStringView view = d_string_view_from_c_string("abc123abc");
    D_TEST_EXPR(d_string_view_find_last_char_in_set_from_end(view, D_STRING_VIEW_FROM_LITERAL("0123456789")) == 5);
    D_TEST_EXPR(d_string_view_find_last_char_not_in_set_from_end(view, D_STRING_VIEW_FROM_LITERAL("abc")) == 5);
}

static void test_find_char_in_empty_set_returns_not_found(void)
{
    DStringView view = d_string_view_from_c_string("abc");
    D_TEST_EXPR(d_string_view_find_first_char_in_set_from_start(view, D_STRING_VIEW_FROM_LITERAL("")) == MAX_SIZE_T_VALUE);
    D_TEST_EXPR(d_string_view_find_last_char_in_set_from_end(view, D_STRING_VIEW_FROM_LITERAL("")) == MAX_SIZE_T_VALUE);
}

static void test_find_char_not_in_empty_set_returns_edges(void)
{
    DStringView view = d_string_view_from_c_string("abc");
    D_TEST_EXPR(d_string_view_find_first_char_not_in_set_from_start(view, D_STRING_VIEW_FROM_LITERAL("")) == 0);
    D_TEST_EXPR(d_string_view_find_last_char_not_in_set_from_end(view, D_STRING_VIEW_FROM_LITERAL("")) == 2);
}

/* predicate find */
static void test_find_first_matching_predicate(void)
{
    DStringView view = d_string_view_from_c_string("abc123");
    D_TEST_EXPR(d_string_view_find_first_matching_predicate_from_start(view, is_digit_like) == 3);
    D_TEST_EXPR(d_string_view_find_first_not_matching_predicate_from_start(view, is_lower_like) == 3);
}

static void test_find_last_matching_predicate(void)
{
    DStringView view = d_string_view_from_c_string("abc123xyz");
    D_TEST_EXPR(d_string_view_find_last_matching_predicate_from_end(view, is_digit_like) == 5);
    D_TEST_EXPR(d_string_view_find_last_not_matching_predicate_from_end(view, is_lower_like) == 5);
}

static void test_predicate_search_on_empty_view_returns_not_found(void)
{
    DStringView view = d_string_view_from_c_string("");
    D_TEST_EXPR(d_string_view_find_first_matching_predicate_from_start(view, is_digit_like) == MAX_SIZE_T_VALUE);
    D_TEST_EXPR(d_string_view_find_last_matching_predicate_from_end(view, is_digit_like) == MAX_SIZE_T_VALUE);
}

/* trim */
static void test_trim_left_and_right_by_char_basic(void)
{
    DStringView view = d_string_view_from_c_string("---abc---");
    expect_view_cstr(d_string_view_trim_left_by_char(view, '-'), "abc---");
    expect_view_cstr(d_string_view_trim_right_by_char(view, '-'), "---abc");
}

static void test_trim_both_sides_by_char(void)
{
    DStringView view = d_string_view_from_c_string("***hello***");
    DStringView left = d_string_view_trim_left_by_char(view, '*');
    DStringView both = d_string_view_trim_right_by_char(left, '*');
    expect_view_cstr(both, "hello");
}

static void test_trim_all_matching_chars_returns_empty_view_at_end_or_start(void)
{
    DStringView view = d_string_view_from_c_string("xxxxx");
    DStringView left = d_string_view_trim_left_by_char(view, 'x');
    DStringView right = d_string_view_trim_right_by_char(view, 'x');
    D_TEST_EXPR(left.size == 0);
    D_TEST_NOT_NULL(left.data);
    D_TEST_EXPR(right.size == 0);
}

static void test_trim_no_matching_chars_returns_original_content(void)
{
    DStringView view = d_string_view_from_c_string("abc");
    DStringView left = d_string_view_trim_left_by_char(view, 'x');
    DStringView right = d_string_view_trim_right_by_char(view, 'x');
    D_TEST_EXPR(left.data == view.data);
    D_TEST_EXPR(right.data == view.data);
    expect_view_cstr(left, "abc");
    expect_view_cstr(right, "abc");
}

static void test_trim_by_predicate_spaces(void)
{
    DStringView view = d_string_view_from_c_string(" \t\nabc\r\n ");
    expect_view_cstr(d_string_view_trim_left_by_predicate(view, is_space_like), "abc\r\n ");
    expect_view_cstr(d_string_view_trim_right_by_predicate(view, is_space_like), " \t\nabc");
}

static void test_trim_predicate_all_matching_returns_empty(void)
{
    DStringView view = d_string_view_from_c_string("12345");
    D_TEST_EXPR(d_string_view_trim_left_by_predicate(view, is_digit_like).size == 0);
    D_TEST_EXPR(d_string_view_trim_right_by_predicate(view, is_digit_like).size == 0);
}

static void test_trim_empty_and_null_views_are_safe(void)
{
    DStringView empty = d_string_view_from_c_string("");
    DStringView nullv = d_string_view_from_parts(NULL, 0);
    D_TEST_EXPR(d_string_view_trim_left_by_char(empty, 'x').size == 0);
    D_TEST_EXPR(d_string_view_trim_right_by_char(empty, 'x').size == 0);
    D_TEST_EXPR(d_string_view_trim_left_by_char(nullv, 'x').size == 0);
    D_TEST_EXPR(d_string_view_trim_right_by_char(nullv, 'x').size == 0);
}

static void test_split_owned_by_char_basic_skips_empty_tokens(void)
{
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_owned(&arr, d_string_view_from_c_string("a,b,c"), ARRAY_DEFAULT_OPTS, ',') == D_OK);
    expect_split_size(&arr, 3);
    expect_split_elem(&arr, 0, "a");
    expect_split_elem(&arr, 1, "b");
    expect_split_elem(&arr, 2, "c");
    d_dyn_array_destroy(&arr);
}

static void test_split_owned_by_char_collapses_consecutive_separators(void)
{
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_owned(&arr, d_string_view_from_c_string(",,a,,,b,,"), ARRAY_DEFAULT_OPTS, ',') == D_OK);
    expect_split_size(&arr, 2);
    expect_split_elem(&arr, 0, "a");
    expect_split_elem(&arr, 1, "b");
    d_dyn_array_destroy(&arr);
}

static void test_split_owned_by_char_no_separator_returns_original_copy(void)
{
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_owned(&arr, d_string_view_from_c_string("abc"), ARRAY_DEFAULT_OPTS, ',') == D_OK);
    expect_split_size(&arr, 1);
    expect_split_elem(&arr, 0, "abc");
    d_dyn_array_destroy(&arr);
}

static void test_split_owned_by_char_all_separators_returns_empty_array(void)
{
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_owned(&arr, d_string_view_from_c_string(",,,,"), ARRAY_DEFAULT_OPTS, ',') == D_OK);
    expect_split_size(&arr, 0);
    d_dyn_array_destroy(&arr);
}

static void test_split_owned_by_char_empty_view_returns_empty_array(void)
{
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_owned(&arr, d_string_view_from_c_string(""), ARRAY_DEFAULT_OPTS, ',') == D_OK);
    expect_split_size(&arr, 0);
    d_dyn_array_destroy(&arr);
}

static void test_split_owned_by_char_rejects_null_output_pointer(void)
{
    D_TEST_EXPR(d_string_view_split_by_char_owned(NULL, d_string_view_from_c_string("a,b"), ARRAY_DEFAULT_OPTS, ',') == D_ERR_INVALID_ARG);
}

static void test_split_owned_by_char_of_str_basic_multiple_separators(void)
{
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_of_str_owned(&arr, d_string_view_from_c_string("a,b;c::d"), ARRAY_DEFAULT_OPTS, D_STRING_VIEW_FROM_LITERAL(",;:")) == D_OK);
    expect_split_size(&arr, 4);
    expect_split_elem(&arr, 0, "a");
    expect_split_elem(&arr, 1, "b");
    expect_split_elem(&arr, 2, "c");
    expect_split_elem(&arr, 3, "d");
    d_dyn_array_destroy(&arr);
}

static void test_split_owned_by_char_of_str_collapses_consecutive_separators(void)
{
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_of_str_owned(&arr, d_string_view_from_c_string(";;a,,b::"), ARRAY_DEFAULT_OPTS, D_STRING_VIEW_FROM_LITERAL(",;:")) == D_OK);
    expect_split_size(&arr, 2);
    expect_split_elem(&arr, 0, "a");
    expect_split_elem(&arr, 1, "b");
    d_dyn_array_destroy(&arr);
}

static void test_split_owned_by_char_of_str_empty_separator_set_returns_whole_string(void)
{
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_of_str_owned(&arr, d_string_view_from_c_string("abc"), ARRAY_DEFAULT_OPTS, D_STRING_VIEW_FROM_LITERAL("")) == D_OK);
    expect_split_size(&arr, 1);
    expect_split_elem(&arr, 0, "abc");
    d_dyn_array_destroy(&arr);
}

static void test_split_owned_by_char_of_str_rejects_null_output_pointer(void)
{
    D_TEST_EXPR(d_string_view_split_by_char_of_str_owned(NULL, d_string_view_from_c_string("a"), ARRAY_DEFAULT_OPTS, D_STRING_VIEW_FROM_LITERAL(",")) == D_ERR_INVALID_ARG);
}

static void test_split_owned_by_char_should_respect_embedded_nul_in_view(void)
{
    const char data[] = {'a', ',', 'b', '\0', 'c', ',', 'd'};
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_owned(&arr, d_string_view_from_parts(data, sizeof(data)), ARRAY_DEFAULT_OPTS, ',') == D_OK);
    expect_split_size(&arr, 3);
    expect_split_elem(&arr, 0, "a");
    /* token 1 contains b, NUL, c; current char* API cannot show it with strcmp, so validate bytes */
    char *elem = NULL;
    D_TEST_EXPR(d_dyn_array_get_elem_at(&arr, 1, &elem) == D_OK);
    D_TEST_NOT_NULL(elem);
    D_TEST_EXPR(elem[0] == 'b');
    D_TEST_EXPR(elem[1] == '\0');
    expect_split_elem(&arr, 2, "d");
    d_dyn_array_destroy(&arr);
}

/* dyn string conversion */
static void test_dyn_string_init_from_string_view_basic(void)
{
    DDynString s;
    D_TEST_EXPR(d_dyn_string_init_from_string_view(&s, d_string_view_from_c_string("hello")) == D_OK);
    D_TEST_STR_EQ(d_dyn_string_get_string(&s), "hello");
    d_dyn_string_destroy(&s);
}

static void test_dyn_string_init_from_string_view_subview(void)
{
    DDynString s;
    DStringView view = d_string_view_subview(d_string_view_from_c_string("012345"), 2, 3);
    D_TEST_EXPR(d_dyn_string_init_from_string_view(&s, view) == D_OK);
    D_TEST_STR_EQ(d_dyn_string_get_string(&s), "234");
    d_dyn_string_destroy(&s);
}

static void test_dyn_string_init_from_string_view_empty_view(void)
{
    DDynString s;
    D_TEST_EXPR(d_dyn_string_init_from_string_view(&s, d_string_view_from_c_string("")) == D_OK);
    D_TEST_STR_EQ(d_dyn_string_get_string(&s), "");
    d_dyn_string_destroy(&s);
}

static void test_dyn_string_init_from_string_view_rejects_null_dstring(void)
{
    D_TEST_EXPR(d_dyn_string_init_from_string_view(NULL, d_string_view_from_c_string("abc")) == D_ERR_INVALID_ARG);
}

static void test_dyn_string_init_from_string_view_from_null_parts_succeeds(void)
{
    DDynString s;
    D_TEST_EXPR(d_dyn_string_init_from_string_view(&s, d_string_view_from_parts(NULL, 0)) == D_OK);
    D_TEST_STR_EQ(d_dyn_string_get_string(&s), "");
    d_dyn_string_destroy(&s);
}

static void test_dyn_string_init_from_string_view_preserves_embedded_nul(void)
{
    const char data[] = {'a', '\0', 'b'};
    DDynString s;
    usize size = 0;
    D_TEST_EXPR(d_dyn_string_init_from_string_view(&s, d_string_view_from_parts(data, sizeof(data))) == D_OK);
    D_TEST_EXPR(d_dyn_string_get_size(&s, &size) == D_OK);
    D_TEST_EXPR(size == sizeof(data));
    D_TEST_MEM_EQ(d_dyn_string_get_string(&s), data, sizeof(data));
    D_TEST_EXPR(d_dyn_string_get_string(&s)[sizeof(data)] == '\0');
    d_dyn_string_destroy(&s);
}

/* stress / invariants */
static void test_repeated_subviews_across_every_position_and_length(void)
{
    const char *s = "abcdefghijklmnopqrstuvwxyz";
    DStringView view = d_string_view_from_c_string(s);
    for (usize pos = 0; pos <= view.size; ++pos)
    {
        for (usize len = 0; len <= view.size + 5; ++len)
        {
            DStringView sub = d_string_view_subview(view, pos, len);
            usize expected = 0;
            if (pos < view.size)
            {
                expected = view.size - pos;
                if (expected > len)
                    expected = len;
                D_TEST_EXPR(sub.data == s + pos);
                D_TEST_EXPR(sub.size == expected);
                D_TEST_MEM_EQ(sub.data, s + pos, expected);
            }
            else if (pos == view.size)
            {
                D_TEST_EXPR(sub.size == 0);
                D_TEST_NOT_NULL(sub.data);
            }
        }
    }
}

static void test_search_every_character_matches_memchr_expectations(void)
{
    const char data[] = "the quick brown fox jumps over the lazy dog";
    DStringView view = d_string_view_from_c_string(data);
    for (int ch = 1; ch < 127; ++ch)
    {
        const char *first = memchr(data, ch, view.size);
        usize got_first = d_string_view_find_first_matching_char_from_start(view, (char)ch);
        if (first == NULL)
            D_TEST_EXPR(got_first == MAX_SIZE_T_VALUE);
        else
            D_TEST_EXPR(got_first == (usize)(first - data));

        usize expected_last = MAX_SIZE_T_VALUE;
        for (usize i = 0; i < view.size; ++i)
            if (data[i] == (char)ch)
                expected_last = i;
        D_TEST_EXPR(d_string_view_find_last_matching_char_from_end(view, (char)ch) == expected_last);
    }
}

static void test_find_substring_many_overlapping_positions(void)
{
    DStringView view = d_string_view_from_c_string("abababababab");
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_start(view, D_STRING_VIEW_FROM_LITERAL("abab")) == 0);
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_index(view, D_STRING_VIEW_FROM_LITERAL("abab"), 1) == 2);
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_index(view, D_STRING_VIEW_FROM_LITERAL("abab"), 3) == 4);
    D_TEST_EXPR(d_string_view_find_last_matching_view_from_end(view, D_STRING_VIEW_FROM_LITERAL("abab")) == 8);
}

static void test_split_owned_long_string_many_tokens(void)
{
    char buffer[512];
    usize pos = 0;
    for (int i = 0; i < 80; ++i)
    {
        buffer[pos++] = 'x';
        buffer[pos++] = ',';
    }
    buffer[pos++] = 'z';
    buffer[pos] = '\0';

    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_owned(&arr, d_string_view_from_c_string(buffer), ARRAY_DEFAULT_OPTS, ',') == D_OK);
    expect_split_size(&arr, 81);
    for (usize i = 0; i < 80; ++i)
        expect_split_elem(&arr, i, "x");
    expect_split_elem(&arr, 80, "z");
    d_dyn_array_destroy(&arr);
}

static void test_split_not_owned_by_char_basic(void)
{
    const char *src = "a,b,c";
    DStringView view = d_string_view_from_c_string(src);
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_not_owned(&arr, view, ARRAY_DEFAULT_OPTS, ',') == D_OK);
    expect_split_size(&arr, 3);
    expect_split_view_elem(&arr, 0, "a");
    expect_split_view_elem(&arr, 1, "b");
    expect_split_view_elem(&arr, 2, "c");
    DStringView t0 = {0}, t1 = {0}, t2 = {0};
    d_dyn_array_get_elem_at(&arr, 0, &t0);
    d_dyn_array_get_elem_at(&arr, 1, &t1);
    d_dyn_array_get_elem_at(&arr, 2, &t2);
    D_TEST_EXPR(t0.data == src);
    D_TEST_EXPR(t1.data == src + 2);
    D_TEST_EXPR(t2.data == src + 4);
    d_dyn_array_destroy(&arr);
}

static void test_split_not_owned_by_char_collapses_consecutive_separators(void)
{
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_not_owned(&arr, d_string_view_from_c_string(",,a,,,b,,"), ARRAY_DEFAULT_OPTS, ',') == D_OK);
    expect_split_size(&arr, 2);
    expect_split_view_elem(&arr, 0, "a");
    expect_split_view_elem(&arr, 1, "b");
    d_dyn_array_destroy(&arr);
}

static void test_split_not_owned_by_char_no_separator_returns_whole(void)
{
    const char *src = "abc";
    DStringView view = d_string_view_from_c_string(src);
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_not_owned(&arr, view, ARRAY_DEFAULT_OPTS, ',') == D_OK);
    expect_split_size(&arr, 1);
    expect_split_view_elem(&arr, 0, "abc");
    DStringView tok = {0};
    d_dyn_array_get_elem_at(&arr, 0, &tok);
    D_TEST_EXPR(tok.data == src);
    d_dyn_array_destroy(&arr);
}

static void test_split_not_owned_by_char_all_separators_returns_empty_array(void)
{
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_not_owned(&arr, d_string_view_from_c_string(",,,,"), ARRAY_DEFAULT_OPTS, ',') == D_OK);
    expect_split_size(&arr, 0);
    d_dyn_array_destroy(&arr);
}

static void test_split_not_owned_by_char_empty_view_returns_empty_array(void)
{
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_not_owned(&arr, d_string_view_from_c_string(""), ARRAY_DEFAULT_OPTS, ',') == D_OK);
    expect_split_size(&arr, 0);
    d_dyn_array_destroy(&arr);
}

static void test_split_not_owned_by_char_rejects_null_output_pointer(void)
{
    D_TEST_EXPR(d_string_view_split_by_char_not_owned(NULL, d_string_view_from_c_string("a,b"), ARRAY_DEFAULT_OPTS, ',') == D_ERR_INVALID_ARG);
}

static void test_split_not_owned_by_char_views_do_not_allocate(void)
{
    const char src[] = "hello world foo";
    DStringView view = d_string_view_from_parts(src, sizeof(src) - 1);
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_not_owned(&arr, view, ARRAY_DEFAULT_OPTS, ' ') == D_OK);
    expect_split_size(&arr, 3);
    usize size = 0;
    d_dyn_array_get_size(&arr, &size);
    for (usize i = 0; i < size; ++i)
    {
        DStringView tok = {0};
        d_dyn_array_get_elem_at(&arr, i, &tok);
        D_TEST_EXPR(tok.data >= src && tok.data < src + sizeof(src));
    }
    d_dyn_array_destroy(&arr);
}


static void test_split_not_owned_by_char_of_str_basic(void)
{
    const char *src = "a,b;c::d";
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_of_str_not_owned(&arr, d_string_view_from_c_string(src), ARRAY_DEFAULT_OPTS, D_STRING_VIEW_FROM_LITERAL(",;:")) == D_OK);
    expect_split_size(&arr, 4);
    expect_split_view_elem(&arr, 0, "a");
    expect_split_view_elem(&arr, 1, "b");
    expect_split_view_elem(&arr, 2, "c");
    expect_split_view_elem(&arr, 3, "d");
    DStringView t0 = {0};
    d_dyn_array_get_elem_at(&arr, 0, &t0);
    D_TEST_EXPR(t0.data == src);
    d_dyn_array_destroy(&arr);
}

static void test_split_not_owned_by_char_of_str_collapses_consecutive(void)
{
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_of_str_not_owned(&arr, d_string_view_from_c_string(";;a,,b::"), ARRAY_DEFAULT_OPTS, D_STRING_VIEW_FROM_LITERAL(",;:")) == D_OK);
    expect_split_size(&arr, 2);
    expect_split_view_elem(&arr, 0, "a");
    expect_split_view_elem(&arr, 1, "b");
    d_dyn_array_destroy(&arr);
}

static void test_split_not_owned_by_char_of_str_empty_set_returns_whole(void)
{
    const char *src = "abc";
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_of_str_not_owned(&arr, d_string_view_from_c_string(src), ARRAY_DEFAULT_OPTS, D_STRING_VIEW_FROM_LITERAL("")) == D_OK);
    expect_split_size(&arr, 1);
    expect_split_view_elem(&arr, 0, "abc");
    DStringView tok = {0};
    d_dyn_array_get_elem_at(&arr, 0, &tok);
    D_TEST_EXPR(tok.data == src);
    d_dyn_array_destroy(&arr);
}

static void test_split_not_owned_by_char_of_str_all_delims_returns_empty_array(void)
{
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_of_str_not_owned(&arr, d_string_view_from_c_string(";,;,"), ARRAY_DEFAULT_OPTS, D_STRING_VIEW_FROM_LITERAL(",;")) == D_OK);
    expect_split_size(&arr, 0);
    d_dyn_array_destroy(&arr);
}

static void test_split_not_owned_by_char_of_str_rejects_null_output_pointer(void)
{
    D_TEST_EXPR(d_string_view_split_by_char_of_str_not_owned(NULL, d_string_view_from_c_string("a"), ARRAY_DEFAULT_OPTS, D_STRING_VIEW_FROM_LITERAL(",")) == D_ERR_INVALID_ARG);
}

static void test_split_not_owned_by_char_of_str_views_point_into_source(void)
{
    const char src[] = "one two,three";
    DStringView view = d_string_view_from_parts(src, sizeof(src) - 1);
    DDynArray arr;
    D_TEST_EXPR(d_string_view_split_by_char_of_str_not_owned(&arr, view, ARRAY_DEFAULT_OPTS, D_STRING_VIEW_FROM_LITERAL(" ,")) == D_OK);
    expect_split_size(&arr, 3);
    usize size = 0;
    d_dyn_array_get_size(&arr, &size);
    for (usize i = 0; i < size; ++i)
    {
        DStringView tok = {0};
        d_dyn_array_get_elem_at(&arr, i, &tok);
        D_TEST_EXPR(tok.data >= src && tok.data < src + sizeof(src));
    }
    d_dyn_array_destroy(&arr);
}

static void test_view_remains_non_owning_after_dyn_string_mutation_pointer_may_be_invalid_contract(void)
{
    DDynString s;
    D_TEST_EXPR(d_dyn_string_init_from_c_string(&s, "abc") == D_OK);
    DStringView view = d_string_view_from_dyn_string(&s);
    D_TEST_EXPR(view.size == 3);
    D_TEST_MEM_EQ(view.data, "abc", 3);
    d_dyn_string_destroy(&s);
    /* This test documents non-owning semantics: do not dereference view.data after owner destruction. */
    D_TEST_EXPR(view.size == 3);
}

int main(void)
{
    DTest tests[] = {
        D_TEST_GENERATE_TEST(test_from_parts_keeps_pointer_and_size_for_valid_buffer),
        D_TEST_GENERATE_TEST(test_from_parts_null_normalizes_to_empty_view),
        D_TEST_GENERATE_TEST(test_from_c_string_null_returns_empty_view),
        D_TEST_GENERATE_TEST(test_from_c_string_stops_at_first_nul),
        D_TEST_GENERATE_TEST(test_from_dyn_string_null_returns_empty_view),
        D_TEST_GENERATE_TEST(test_from_dyn_string_points_to_dyn_string_storage),
        D_TEST_GENERATE_TEST(test_from_dyn_string_with_empty_string),
        D_TEST_GENERATE_TEST(test_len_data_and_is_empty_are_consistent),
        D_TEST_GENERATE_TEST(test_get_char_at_reads_embedded_nul_and_edges),
        D_TEST_GENERATE_TEST(test_subview_middle_does_not_allocate_and_points_inside_original),
        D_TEST_GENERATE_TEST(test_subview_clamps_size_past_end),
        D_TEST_GENERATE_TEST(test_subview_pos_equal_len_returns_valid_empty_string_pointer),
        D_TEST_GENERATE_TEST(test_subview_pos_greater_than_len_returns_empty_view),
        D_TEST_GENERATE_TEST(test_subview_empty_view_at_pos_zero_returns_empty_view),
        D_TEST_GENERATE_TEST(test_subview_zero_size_in_middle_keeps_position_pointer),
        D_TEST_GENERATE_TEST(test_substr_allocates_copy_and_terminates),
        D_TEST_GENERATE_TEST(test_substr_pos_at_len_returns_allocated_empty_string),
        D_TEST_GENERATE_TEST(test_substr_pos_past_len_returns_null),
        D_TEST_GENERATE_TEST(test_substr_empty_view_at_pos_zero_returns_allocated_empty_string),
        D_TEST_GENERATE_TEST(test_substr_zero_len_allocates_empty_string),
        D_TEST_GENERATE_TEST(test_substr_should_respect_view_size_with_embedded_nul_before_pos),
        D_TEST_GENERATE_TEST(test_substr_should_copy_embedded_nul_inside_result),
        D_TEST_GENERATE_TEST(test_compare_equal_views_returns_true),
        D_TEST_GENERATE_TEST(test_compare_same_prefix_different_lengths_returns_false),
        D_TEST_GENERATE_TEST(test_compare_different_same_length_views_return_false),
        D_TEST_GENERATE_TEST(test_compare_handles_embedded_nul_by_size),
        D_TEST_GENERATE_TEST(test_compare_against_c_string_equal_and_not_equal),
        D_TEST_GENERATE_TEST(test_equals_views_with_embedded_nul),
        D_TEST_GENERATE_TEST(test_equals_c_string_rejects_view_with_extra_embedded_data),
        D_TEST_GENERATE_TEST(test_empty_view_equals_empty_c_string),
        D_TEST_GENERATE_TEST(test_starts_and_ends_with_char_basic_edges),
        D_TEST_GENERATE_TEST(test_starts_and_ends_with_embedded_nul_views),
        D_TEST_GENERATE_TEST(test_starts_ends_empty_prefix_suffix_are_true),
        D_TEST_GENERATE_TEST(test_starts_ends_prefix_suffix_longer_than_view_are_false),
        D_TEST_GENERATE_TEST(test_find_first_matching_char_from_index),
        D_TEST_GENERATE_TEST(test_find_first_matching_char_pos_at_or_past_end_returns_not_found),
        D_TEST_GENERATE_TEST(test_find_first_not_matching_char),
        D_TEST_GENERATE_TEST(test_find_last_matching_char_clamps_large_pos_to_end),
        D_TEST_GENERATE_TEST(test_find_last_not_matching_char),
        D_TEST_GENERATE_TEST(test_find_char_handles_embedded_nul),
        D_TEST_GENERATE_TEST(test_find_char_on_empty_views_returns_not_found),
        D_TEST_GENERATE_TEST(test_find_first_matching_view_basic_and_overlapping),
        D_TEST_GENERATE_TEST(test_find_last_matching_view_basic_and_overlapping),
        D_TEST_GENERATE_TEST(test_find_matching_view_with_embedded_nul_needle),
        D_TEST_GENERATE_TEST(test_find_first_empty_needle_at_start_and_middle),
        D_TEST_GENERATE_TEST(test_find_last_empty_needle_clamps_to_len),
        D_TEST_GENERATE_TEST(test_find_view_on_empty_haystack),
        D_TEST_GENERATE_TEST(test_find_first_char_in_set_and_not_in_set),
        D_TEST_GENERATE_TEST(test_find_last_char_in_set_and_not_in_set),
        D_TEST_GENERATE_TEST(test_find_char_in_empty_set_returns_not_found),
        D_TEST_GENERATE_TEST(test_find_char_not_in_empty_set_returns_edges),
        D_TEST_GENERATE_TEST(test_find_first_matching_predicate),
        D_TEST_GENERATE_TEST(test_find_last_matching_predicate),
        D_TEST_GENERATE_TEST(test_predicate_search_on_empty_view_returns_not_found),
        D_TEST_GENERATE_TEST(test_trim_left_and_right_by_char_basic),
        D_TEST_GENERATE_TEST(test_trim_both_sides_by_char),
        D_TEST_GENERATE_TEST(test_trim_all_matching_chars_returns_empty_view_at_end_or_start),
        D_TEST_GENERATE_TEST(test_trim_no_matching_chars_returns_original_content),
        D_TEST_GENERATE_TEST(test_trim_by_predicate_spaces),
        D_TEST_GENERATE_TEST(test_trim_predicate_all_matching_returns_empty),
        D_TEST_GENERATE_TEST(test_trim_empty_and_null_views_are_safe),
        D_TEST_GENERATE_TEST(test_split_owned_by_char_basic_skips_empty_tokens),
        D_TEST_GENERATE_TEST(test_split_owned_by_char_collapses_consecutive_separators),
        D_TEST_GENERATE_TEST(test_split_owned_by_char_no_separator_returns_original_copy),
        D_TEST_GENERATE_TEST(test_split_owned_by_char_all_separators_returns_empty_array),
        D_TEST_GENERATE_TEST(test_split_owned_by_char_empty_view_returns_empty_array),
        D_TEST_GENERATE_TEST(test_split_owned_by_char_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_split_owned_by_char_of_str_basic_multiple_separators),
        D_TEST_GENERATE_TEST(test_split_owned_by_char_of_str_collapses_consecutive_separators),
        D_TEST_GENERATE_TEST(test_split_owned_by_char_of_str_empty_separator_set_returns_whole_string),
        D_TEST_GENERATE_TEST(test_split_owned_by_char_of_str_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_split_owned_by_char_should_respect_embedded_nul_in_view),
        D_TEST_GENERATE_TEST(test_dyn_string_init_from_string_view_basic),
        D_TEST_GENERATE_TEST(test_dyn_string_init_from_string_view_subview),
        D_TEST_GENERATE_TEST(test_dyn_string_init_from_string_view_empty_view),
        D_TEST_GENERATE_TEST(test_dyn_string_init_from_string_view_rejects_null_dstring),
        D_TEST_GENERATE_TEST(test_dyn_string_init_from_string_view_from_null_parts_succeeds),
        D_TEST_GENERATE_TEST(test_dyn_string_init_from_string_view_preserves_embedded_nul),
        D_TEST_GENERATE_TEST(test_repeated_subviews_across_every_position_and_length),
        D_TEST_GENERATE_TEST(test_search_every_character_matches_memchr_expectations),
        D_TEST_GENERATE_TEST(test_find_substring_many_overlapping_positions),
        D_TEST_GENERATE_TEST(test_split_owned_long_string_many_tokens),
        D_TEST_GENERATE_TEST(test_split_not_owned_by_char_basic),
        D_TEST_GENERATE_TEST(test_split_not_owned_by_char_collapses_consecutive_separators),
        D_TEST_GENERATE_TEST(test_split_not_owned_by_char_no_separator_returns_whole),
        D_TEST_GENERATE_TEST(test_split_not_owned_by_char_all_separators_returns_empty_array),
        D_TEST_GENERATE_TEST(test_split_not_owned_by_char_empty_view_returns_empty_array),
        D_TEST_GENERATE_TEST(test_split_not_owned_by_char_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_split_not_owned_by_char_views_do_not_allocate),
        D_TEST_GENERATE_TEST(test_split_not_owned_by_char_of_str_basic),
        D_TEST_GENERATE_TEST(test_split_not_owned_by_char_of_str_collapses_consecutive),
        D_TEST_GENERATE_TEST(test_split_not_owned_by_char_of_str_empty_set_returns_whole),
        D_TEST_GENERATE_TEST(test_split_not_owned_by_char_of_str_all_delims_returns_empty_array),
        D_TEST_GENERATE_TEST(test_split_not_owned_by_char_of_str_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_split_not_owned_by_char_of_str_views_point_into_source),
        D_TEST_GENERATE_TEST(test_view_remains_non_owning_after_dyn_string_mutation_pointer_may_be_invalid_contract),
    };

    D_TEST_RUN_TESTS(tests);
    return 0;
}
