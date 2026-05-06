#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "d_test.h"
#include "d_string_view.h"
#include "d_dyn_string.h"
#include "d_dyn_array.h"
#include "d_types.h"

#define ARRAY_DEFAULT_OPTS ((BufferOpts)0)
#define NOT_FOUND MAX_SIZE_T_VALUE

static DStringView view_lit(const char *s)
{
    return d_string_view_from_c_string(s);
}

static DStringView view_mem(const char *s, usize n)
{
    return d_string_view_from_parts(s, n);
}

static void expect_view(DStringView view, const char *expected, usize expected_size)
{
    D_TEST_EXPR(d_string_view_len(view) == expected_size);
    if (expected_size == 0)
        return;
    D_TEST_NOT_NULL(d_string_view_data(view));
    D_TEST_MEM_EQ(d_string_view_data(view), expected, expected_size);
}

static void expect_empty_view(DStringView view)
{
    D_TEST_EXPR(d_string_view_len(view) == 0);
}

static void expect_view_ptr_and_size(DStringView view, const char *expected_ptr, usize expected_size)
{
    D_TEST_EXPR(d_string_view_data(view) == expected_ptr);
    D_TEST_EXPR(d_string_view_len(view) == expected_size);
}

static usize ptr_array_size(DDynArray *arr)
{
    usize size = 0;
    D_TEST_EXPR(d_dyn_array_get_size(arr, &size) == D_OK);
    return size;
}

static char *ptr_array_get_str(DDynArray *arr, usize index)
{
    char *out = NULL;
    D_TEST_EXPR(d_dyn_array_get_elem_at(arr, index, &out) == D_OK);
    return out;
}

static bool is_space_match(char c)
{
    return c == ' ' || c == '\t' || c == '\n';
}

static bool is_digit_match(char c)
{
    return c >= '0' && c <= '9';
}

static bool is_lower_a_to_f(char c)
{
    return c >= 'a' && c <= 'f';
}

static void test_from_parts_keeps_pointer_and_size(void)
{
    const char data[] = {'a', 'b', 'c', 'd'};
    DStringView view = d_string_view_from_parts(data, 4);

    expect_view_ptr_and_size(view, data, 4);
    D_TEST_MEM_EQ(view.data, data, 4);
}

static void test_from_parts_preserves_embedded_nul(void)
{
    const char data[] = {'a', 'b', '\0', 'c', 'd'};
    DStringView view = d_string_view_from_parts(data, sizeof(data));

    expect_view_ptr_and_size(view, data, sizeof(data));
    D_TEST_EXPR(d_string_view_get_char_at(view, 2) == '\0');
    D_TEST_EXPR(d_string_view_get_char_at(view, 3) == 'c');
}

static void test_from_parts_null_data_forces_empty_view(void)
{
    DStringView view = d_string_view_from_parts(NULL, 99);
    expect_view(view, "", 0);
    D_TEST_EXPR(view.size == 0);
    D_TEST_EXPR(d_string_view_is_empty(view));
}

static void test_from_c_string_sets_strlen_size(void)
{
    DStringView view = d_string_view_from_c_string("hello");

    expect_view(view, "hello", 5);
}

static void test_from_c_string_empty_string_is_empty_but_data_valid(void)
{
    const char *s = "";
    DStringView view = d_string_view_from_c_string(s);

    expect_view_ptr_and_size(view, s, 0);
    D_TEST_EXPR(d_string_view_is_empty(view));
}

static void test_from_c_string_null_returns_null_empty(void)
{
    DStringView view = d_string_view_from_c_string(NULL);

    D_TEST_NULL(view.data);
    D_TEST_EXPR(view.size == 0);
}

static void test_from_dyn_string_null_returns_empty(void)
{
    DStringView view = d_string_view_from_dyn_string(NULL);

    D_TEST_NULL(view.data);
    D_TEST_EXPR(view.size == 0);
}

static void test_from_dyn_string_views_dyn_string_buffer(void)
{
    DDynString *s = NULL;

    D_TEST_EXPR(d_dyn_string_new_from_c_string(&s, "dynamic") == D_OK);
    DStringView view = d_string_view_from_dyn_string(s);
    expect_view(view, "dynamic", 7);
    D_TEST_EXPR(view.data == d_dyn_string_get_string(s));

    d_dyn_string_destroy(&s);
}

static void test_len_data_and_is_empty_are_simple_accessors(void)
{
    DStringView empty = view_lit("");
    DStringView non_empty = view_lit("x");

    D_TEST_EXPR(d_string_view_len(empty) == 0);
    D_TEST_EXPR(d_string_view_is_empty(empty));
    D_TEST_EXPR(!d_string_view_is_empty(non_empty));
    D_TEST_EXPR(d_string_view_data(non_empty)[0] == 'x');
}

static void test_get_char_at_reads_inside_bounds(void)
{
    DStringView view = view_lit("abc");

    D_TEST_EXPR(d_string_view_get_char_at(view, 0) == 'a');
    D_TEST_EXPR(d_string_view_get_char_at(view, 2) == 'c');
}

static void test_subview_middle(void)
{
    DStringView view = view_lit("hello world");
    DStringView sub = d_string_view_subview(view, 6, 5);

    expect_view(sub, "world", 5);
    D_TEST_EXPR(sub.data == view.data + 6);
}

static void test_subview_clamps_size_past_end(void)
{
    DStringView view = view_lit("abcdef");
    DStringView sub = d_string_view_subview(view, 4, 100);

    expect_view(sub, "ef", 2);
}

static void test_subview_pos_equal_size_returns_empty_at_end(void)
{
    DStringView view = view_lit("abc");
    DStringView sub = d_string_view_subview(view, 3, 99);
    expect_empty_view(sub);
    D_TEST_EXPR(sub.size == 0);
}

static void test_subview_pos_after_size_returns_null_empty(void)
{
    DStringView view = view_lit("abc");
    DStringView sub = d_string_view_subview(view, 4, 1);

    D_TEST_NULL(sub.data);
    D_TEST_EXPR(sub.size == 0);
}

static void test_subview_null_view_returns_null_empty(void)
{
    DStringView view = d_string_view_from_parts(NULL, 0);
    DStringView sub = d_string_view_subview(view, 0, 1);

    D_TEST_NULL(sub.data);
    D_TEST_EXPR(sub.size == 0);
}

static void test_subview_zero_size_inside_view_keeps_position(void)
{
    DStringView view = view_lit("abc");
    DStringView sub = d_string_view_subview(view, 1, 0);

    D_TEST_EXPR(sub.data == view.data + 1);
    D_TEST_EXPR(sub.size == 0);
}

static void test_substr_middle_allocates_c_string(void)
{
    char *s = d_string_view_substr(view_lit("hello world"), 6, 5);

    D_TEST_NOT_NULL(s);
    D_TEST_STR_EQ(s, "world");
    free(s);
}

static void test_substr_clamps_size_past_end(void)
{
    char *s = d_string_view_substr(view_lit("abcdef"), 4, 100);

    D_TEST_NOT_NULL(s);
    D_TEST_STR_EQ(s, "ef");
    free(s);
}

static void test_compare_equal_full_c_strings(void)
{
    D_TEST_EXPR(d_string_view_compare(view_lit("abc"), view_lit("abc")) == 0);
    D_TEST_EXPR(d_string_view_equals(view_lit("abc"), view_lit("abc")));
}

static void test_compare_orders_c_strings(void)
{
    D_TEST_EXPR(d_string_view_compare(view_lit("abc"), view_lit("abd")) < 0);
    D_TEST_EXPR(d_string_view_compare(view_lit("abd"), view_lit("abc")) > 0);
}

static void test_compare_should_use_view_size_not_strcmp(void)
{
    const char a[] = {'a', 'b', 'c', 'X', '\0'};
    const char b[] = {'a', 'b', 'c', 'Y', '\0'};
    DStringView left = d_string_view_from_parts(a, 3);
    DStringView right = d_string_view_from_parts(b, 3);

    D_TEST_EXPR(d_string_view_compare(left, right) == 0);
    D_TEST_EXPR(d_string_view_equals(left, right));
}

static void test_compare_embedded_nul_should_use_full_size(void)
{
    const char a[] = {'a', '\0', 'x'};
    const char b[] = {'a', '\0', 'y'};
    DStringView left = d_string_view_from_parts(a, 3);
    DStringView right = d_string_view_from_parts(b, 3);

    D_TEST_EXPR(d_string_view_compare(left, right) < 0);
    D_TEST_EXPR(!d_string_view_equals(left, right));
}

static void test_equals_c_string_respects_size(void)
{
    const char data[] = {'a', 'b', 'c', 'X', '\0'};
    DStringView view = d_string_view_from_parts(data, 3);

    D_TEST_EXPR(d_string_view_equals_c_string(view, "abc"));
    D_TEST_EXPR(!d_string_view_equals_c_string(view, "abcd"));
}

static void test_starts_and_ends_with_char(void)
{
    DStringView view = view_lit("abc");

    D_TEST_EXPR(d_string_view_starts_with_char(view, 'a'));
    D_TEST_EXPR(!d_string_view_starts_with_char(view, 'b'));
    D_TEST_EXPR(d_string_view_ends_with_char(view, 'c'));
    D_TEST_EXPR(!d_string_view_ends_with_char(view, 'b'));
}

static void test_starts_and_ends_with_char_empty_false(void)
{
    DStringView view = view_lit("");

    D_TEST_EXPR(!d_string_view_starts_with_char(view, '\0'));
    D_TEST_EXPR(!d_string_view_ends_with_char(view, '\0'));
}

static void test_starts_with_view_cases(void)
{
    DStringView view = view_lit("abcdef");

    D_TEST_EXPR(d_string_view_starts_with_view(view, view_lit("abc")));
    D_TEST_EXPR(d_string_view_starts_with_view(view, view_lit("")));
    D_TEST_EXPR(!d_string_view_starts_with_view(view, view_lit("abd")));
    D_TEST_EXPR(!d_string_view_starts_with_view(view_lit("ab"), view_lit("abc")));
}

static void test_ends_with_view_cases(void)
{
    DStringView view = view_lit("abcdef");

    D_TEST_EXPR(d_string_view_ends_with_view(view, view_lit("def")));
    D_TEST_EXPR(d_string_view_ends_with_view(view, view_lit("")));
    D_TEST_EXPR(!d_string_view_ends_with_view(view, view_lit("cef")));
    D_TEST_EXPR(!d_string_view_ends_with_view(view_lit("ef"), view_lit("def")));
}

static void test_starts_ends_with_embedded_nul_view(void)
{
    const char data[] = {'a', '\0', 'b', 'c'};
    const char prefix[] = {'a', '\0', 'b'};
    const char suffix[] = {'\0', 'b', 'c'};

    D_TEST_EXPR(d_string_view_starts_with_view(view_mem(data, 4), view_mem(prefix, 3)));
    D_TEST_EXPR(d_string_view_ends_with_view(view_mem(data, 4), view_mem(suffix, 3)));
}

static void test_find_first_matching_char(void)
{
    DStringView view = view_lit("banana");

    D_TEST_EXPR(d_string_view_find_first_matching_char_from_start(view, 'a') == 1);
    D_TEST_EXPR(d_string_view_find_first_matching_char_from_index(view, 'a', 2) == 3);
    D_TEST_EXPR(d_string_view_find_first_matching_char_from_index(view, 'z', 0) == NOT_FOUND);
    D_TEST_EXPR(d_string_view_find_first_matching_char_from_index(view, 'a', 99) == NOT_FOUND);
}

static void test_find_first_not_matching_char(void)
{
    DStringView view = view_lit("aaabc");

    D_TEST_EXPR(d_string_view_find_first_not_matching_char_from_start(view, 'a') == 3);
    D_TEST_EXPR(d_string_view_find_first_not_matching_char_from_index(view, 'a', 4) == 4);
    D_TEST_EXPR(d_string_view_find_first_not_matching_char_from_start(view_lit("aaa"), 'a') == NOT_FOUND);
}

static void test_find_last_matching_char(void)
{
    DStringView view = view_lit("banana");

    D_TEST_EXPR(d_string_view_find_last_matching_char_from_end(view, 'a') == 5);
    D_TEST_EXPR(d_string_view_find_last_matching_char_from_index(view, 'a', 4) == 3);
    D_TEST_EXPR(d_string_view_find_last_matching_char_from_index(view, 'z', 5) == NOT_FOUND);
    D_TEST_EXPR(d_string_view_find_last_matching_char_from_index(view, 'b', 99) == 0);
}

static void test_find_last_not_matching_char(void)
{
    DStringView view = view_lit("abccc");

    D_TEST_EXPR(d_string_view_find_last_not_matching_char_from_end(view, 'c') == 1);
    D_TEST_EXPR(d_string_view_find_last_not_matching_char_from_index(view, 'c', 0) == 0);
    D_TEST_EXPR(d_string_view_find_last_not_matching_char_from_end(view_lit("ccc"), 'c') == NOT_FOUND);
}

static void test_find_char_on_empty_and_null_views_returns_not_found(void)
{
    DStringView empty = view_lit("");
    DStringView null_view = d_string_view_from_parts(NULL, 10);

    D_TEST_EXPR(d_string_view_find_first_matching_char_from_start(empty, 'x') == NOT_FOUND);
    D_TEST_EXPR(d_string_view_find_last_matching_char_from_end(empty, 'x') == NOT_FOUND);
    D_TEST_EXPR(d_string_view_find_first_matching_char_from_start(null_view, 'x') == NOT_FOUND);
    D_TEST_EXPR(d_string_view_find_last_matching_char_from_end(null_view, 'x') == NOT_FOUND);
}

static void test_find_first_matching_view(void)
{
    DStringView view = view_lit("abc abc abc");

    D_TEST_EXPR(d_string_view_find_first_matching_view_from_start(view, view_lit("abc")) == 0);
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_index(view, view_lit("abc"), 1) == 4);
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_index(view, view_lit("abc"), 5) == 8);
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_start(view, view_lit("xyz")) == NOT_FOUND);
}

static void test_find_first_matching_view_empty_needle(void)
{
    DStringView view = view_lit("abc");

    D_TEST_EXPR(d_string_view_find_first_matching_view_from_index(view, view_lit(""), 0) == 0);
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_index(view, view_lit(""), 2) == 2);
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_index(view, view_lit(""), 3) == NOT_FOUND);
}

static void test_find_first_matching_view_overlapping(void)
{
    DStringView view = view_lit("aaaa");

    D_TEST_EXPR(d_string_view_find_first_matching_view_from_start(view, view_lit("aa")) == 0);
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_index(view, view_lit("aa"), 1) == 1);
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_index(view, view_lit("aaa"), 2) == NOT_FOUND);
}

static void test_find_view_with_embedded_nul(void)
{
    const char hay[] = {'a', '\0', 'b', 'a', '\0', 'c'};
    const char needle[] = {'a', '\0', 'c'};

    D_TEST_EXPR(d_string_view_find_first_matching_view_from_start(view_mem(hay, sizeof(hay)), view_mem(needle, sizeof(needle))) == 3);
    D_TEST_EXPR(d_string_view_find_last_matching_view_from_end(view_mem(hay, sizeof(hay)), view_mem(needle, sizeof(needle))) == 3);
}

static void test_find_last_matching_view(void)
{
    DStringView view = view_lit("abc abc abc");

    D_TEST_EXPR(d_string_view_find_last_matching_view_from_end(view, view_lit("abc")) == 8);
    D_TEST_EXPR(d_string_view_find_last_matching_view_from_index(view, view_lit("abc"), 7) == 4);
    D_TEST_EXPR(d_string_view_find_last_matching_view_from_index(view, view_lit("abc"), 3) == 0);
    D_TEST_EXPR(d_string_view_find_last_matching_view_from_end(view, view_lit("xyz")) == NOT_FOUND);
}

static void test_find_last_matching_view_empty_needle(void)
{
    DStringView view = view_lit("abc");

    D_TEST_EXPR(d_string_view_find_last_matching_view_from_index(view, view_lit(""), 0) == 0);
    D_TEST_EXPR(d_string_view_find_last_matching_view_from_index(view, view_lit(""), 2) == 2);
    D_TEST_EXPR(d_string_view_find_last_matching_view_from_index(view, view_lit(""), 99) == 3);
}

static void test_find_c_string_wrappers(void)
{
    DStringView view = view_lit("one two one");

    D_TEST_EXPR(d_string_view_find_first_matching_c_string_from_start(view, "two") == 4);
    D_TEST_EXPR(d_string_view_find_first_matching_c_string_from_index(view, "one", 1) == 8);
    D_TEST_EXPR(d_string_view_find_last_matching_c_string_from_end(view, "one") == 8);
    D_TEST_EXPR(d_string_view_find_last_matching_c_string_from_index(view, "one", 7) == 0);
}

static void test_find_char_in_set(void)
{
    DStringView view = view_lit("abc123");

    D_TEST_EXPR(d_string_view_find_first_char_in_set_from_start(view, "0123456789") == 3);
    D_TEST_EXPR(d_string_view_find_first_char_in_set_from_index(view, "abc", 2) == 2);
    D_TEST_EXPR(d_string_view_find_first_char_in_set_from_start(view, "xyz") == NOT_FOUND);
}

static void test_find_char_not_in_set(void)
{
    DStringView view = view_lit("   abc");

    D_TEST_EXPR(d_string_view_find_first_char_not_in_set_from_start(view, " \t\n") == 3);
    D_TEST_EXPR(d_string_view_find_first_char_not_in_set_from_start(view_lit("111"), "1") == NOT_FOUND);
}

static void test_find_last_char_in_set(void)
{
    DStringView view = view_lit("abc123");

    D_TEST_EXPR(d_string_view_find_last_char_in_set_from_end(view, "0123456789") == 5);
    D_TEST_EXPR(d_string_view_find_last_char_in_set_from_index(view, "abc", 4) == 2);
    D_TEST_EXPR(d_string_view_find_last_char_in_set_from_end(view, "xyz") == NOT_FOUND);
}

static void test_find_last_char_not_in_set(void)
{
    DStringView view = view_lit("abc   ");

    D_TEST_EXPR(d_string_view_find_last_char_not_in_set_from_end(view, " \t\n") == 2);
    D_TEST_EXPR(d_string_view_find_last_char_not_in_set_from_end(view_lit("111"), "1") == NOT_FOUND);
}

static void test_find_with_empty_set(void)
{
    DStringView view = view_lit("abc");

    D_TEST_EXPR(d_string_view_find_first_char_in_set_from_start(view, "") == NOT_FOUND);
    D_TEST_EXPR(d_string_view_find_first_char_not_in_set_from_start(view, "") == 0);
    D_TEST_EXPR(d_string_view_find_last_char_in_set_from_end(view, "") == NOT_FOUND);
    D_TEST_EXPR(d_string_view_find_last_char_not_in_set_from_end(view, "") == 2);
}

static void test_find_predicate_first(void)
{
    DStringView view = view_lit("abc123");

    D_TEST_EXPR(d_string_view_find_first_matching_predicate_from_start(view, is_digit_match) == 3);
    D_TEST_EXPR(d_string_view_find_first_matching_predicate_from_index(view, is_digit_match, 4) == 4);
    D_TEST_EXPR(d_string_view_find_first_not_matching_predicate_from_start(view_lit("123a"), is_digit_match) == 3);
    D_TEST_EXPR(d_string_view_find_first_matching_predicate_from_start(view_lit("abc"), is_digit_match) == NOT_FOUND);
}

static void test_find_predicate_last(void)
{
    DStringView view = view_lit("abc123def");

    D_TEST_EXPR(d_string_view_find_last_matching_predicate_from_end(view, is_digit_match) == 5);
    D_TEST_EXPR(d_string_view_find_last_matching_predicate_from_index(view, is_digit_match, 4) == 4);
    D_TEST_EXPR(d_string_view_find_last_not_matching_predicate_from_end(view_lit("abc123"), is_digit_match) == 2);
    D_TEST_EXPR(d_string_view_find_last_matching_predicate_from_end(view_lit("abc"), is_digit_match) == NOT_FOUND);
}

static void test_trim_left_by_char(void)
{
    expect_view(d_string_view_trim_left_by_char(view_lit("---abc--"), '-'), "abc--", 5);
    expect_view(d_string_view_trim_left_by_char(view_lit("abc"), '-'), "abc", 3);
    expect_view(d_string_view_trim_left_by_char(view_lit("---"), '-'), "", 0);
}

static void test_trim_right_by_char(void)
{
    expect_view(d_string_view_trim_right_by_char(view_lit("--abc---"), '-'), "--abc", 5);
    expect_view(d_string_view_trim_right_by_char(view_lit("abc"), '-'), "abc", 3);
    expect_view(d_string_view_trim_right_by_char(view_lit("---"), '-'), "", 0);
}

static void test_trim_by_predicate(void)
{
    expect_view(d_string_view_trim_left_by_predicate(view_lit(" \t\nabc "), is_space_match), "abc ", 4);
    expect_view(d_string_view_trim_right_by_predicate(view_lit(" abc \t\n"), is_space_match), " abc", 4);
    expect_view(d_string_view_trim_left_by_predicate(view_lit("123"), is_digit_match), "", 0);
    expect_view(d_string_view_trim_right_by_predicate(view_lit("123"), is_digit_match), "", 0);
}

static void test_trim_empty_and_null_views(void)
{
    DStringView null_view = d_string_view_from_parts(NULL, 0);

    D_TEST_EXPR(d_string_view_trim_left_by_char(view_lit(""), 'x').size == 0);
    D_TEST_EXPR(d_string_view_trim_right_by_char(view_lit(""), 'x').size == 0);
    D_TEST_EXPR(d_string_view_trim_left_by_char(null_view, 'x').size == 0);
    D_TEST_EXPR(d_string_view_trim_right_by_char(null_view, 'x').size == 0);
}

static void test_dyn_string_new_from_string_view_copies_exact_size(void)
{
    DDynString *s = NULL;
    const char data[] = {'a', 'b', 'c', 'X', '\0'};
    DStringView view = d_string_view_from_parts(data, 3);

    D_TEST_EXPR(d_dyn_string_new_from_string_view(&s, view) == D_OK);
    D_TEST_STR_EQ(d_dyn_string_get_string(s), "abc");
    d_dyn_string_destroy(&s);
}

static void test_dyn_string_new_from_string_view_rejects_null_output(void)
{
    D_TEST_EXPR(d_dyn_string_new_from_string_view(NULL, view_lit("abc")) == D_ERR_INVALID_ARG);
}

static void test_split_by_char_basic(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_string_view_split_by_char(&arr, view_lit("a,b,c"), ARRAY_DEFAULT_OPTS, ',') == D_OK);
    D_TEST_NOT_NULL(arr);
    D_TEST_EXPR(ptr_array_size(arr) == 3);
    D_TEST_STR_EQ(ptr_array_get_str(arr, 0), "a");
    D_TEST_STR_EQ(ptr_array_get_str(arr, 1), "b");
    D_TEST_STR_EQ(ptr_array_get_str(arr, 2), "c");
    d_dyn_array_destroy(&arr);
}

static void test_split_by_char_skips_empty_tokens(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_string_view_split_by_char(&arr, view_lit(",,a,,b,,"), ARRAY_DEFAULT_OPTS, ',') == D_OK);
    D_TEST_EXPR(ptr_array_size(arr) == 2);
    D_TEST_STR_EQ(ptr_array_get_str(arr, 0), "a");
    D_TEST_STR_EQ(ptr_array_get_str(arr, 1), "b");
    d_dyn_array_destroy(&arr);
}

static void test_split_by_char_no_separator_returns_whole_string(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_string_view_split_by_char(&arr, view_lit("abc"), ARRAY_DEFAULT_OPTS, ',') == D_OK);
    D_TEST_EXPR(ptr_array_size(arr) == 1);
    D_TEST_STR_EQ(ptr_array_get_str(arr, 0), "abc");
    d_dyn_array_destroy(&arr);
}

static void test_split_by_char_empty_and_only_separators_return_empty_array(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_string_view_split_by_char(&arr, view_lit(""), ARRAY_DEFAULT_OPTS, ',') == D_OK);
    D_TEST_EXPR(ptr_array_size(arr) == 0);
    d_dyn_array_destroy(&arr);

    D_TEST_EXPR(d_string_view_split_by_char(&arr, view_lit(",,,"), ARRAY_DEFAULT_OPTS, ',') == D_OK);
    D_TEST_EXPR(ptr_array_size(arr) == 0);
    d_dyn_array_destroy(&arr);
}

static void test_split_by_char_rejects_null_output_pointer(void)
{
    D_TEST_EXPR(d_string_view_split_by_char(NULL, view_lit("a,b"), ARRAY_DEFAULT_OPTS, ',') == D_ERR_INVALID_ARG);
}

static void test_split_by_char_of_str_basic(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_string_view_split_by_char_of_str(&arr, view_lit("a,b;c"), ARRAY_DEFAULT_OPTS, ",;") == D_OK);
    D_TEST_EXPR(ptr_array_size(arr) == 3);
    D_TEST_STR_EQ(ptr_array_get_str(arr, 0), "a");
    D_TEST_STR_EQ(ptr_array_get_str(arr, 1), "b");
    D_TEST_STR_EQ(ptr_array_get_str(arr, 2), "c");
    d_dyn_array_destroy(&arr);
}

static void test_split_by_char_of_str_skips_empty_tokens(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_string_view_split_by_char_of_str(&arr, view_lit(",;a;;b,,"), ARRAY_DEFAULT_OPTS, ",;") == D_OK);
    D_TEST_EXPR(ptr_array_size(arr) == 2);
    D_TEST_STR_EQ(ptr_array_get_str(arr, 0), "a");
    D_TEST_STR_EQ(ptr_array_get_str(arr, 1), "b");
    d_dyn_array_destroy(&arr);
}

static void test_split_by_char_of_str_empty_separator_set_returns_whole_string(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_string_view_split_by_char_of_str(&arr, view_lit("abc"), ARRAY_DEFAULT_OPTS, "") == D_OK);
    D_TEST_EXPR(ptr_array_size(arr) == 1);
    D_TEST_STR_EQ(ptr_array_get_str(arr, 0), "abc");
    d_dyn_array_destroy(&arr);
}

static void test_split_by_char_of_str_rejects_null_args(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_string_view_split_by_char_of_str(NULL, view_lit("a,b"), ARRAY_DEFAULT_OPTS, ",") == D_ERR_INVALID_ARG);
    D_TEST_EXPR(d_string_view_split_by_char_of_str(&arr, view_lit("a,b"), ARRAY_DEFAULT_OPTS, NULL) == D_ERR_INVALID_ARG);
    D_TEST_NULL(arr);
}

static void test_find_prefix_suffix_should_not_read_null_for_empty_pattern(void)
{
    DStringView null_empty = d_string_view_from_parts(NULL, 0);

    D_TEST_EXPR(d_string_view_starts_with_view(view_lit("abc"), null_empty));
    D_TEST_EXPR(d_string_view_ends_with_view(view_lit("abc"), null_empty));
}

static void test_predicate_on_empty_view_does_not_call_null_fn(void)
{
    DStringView empty = view_lit("");

    D_TEST_EXPR(d_string_view_find_first_matching_predicate_from_start(empty, NULL) == NOT_FOUND);
    D_TEST_EXPR(d_string_view_find_last_matching_predicate_from_end(empty, NULL) == NOT_FOUND);
}

static void test_set_search_on_empty_view_does_not_read_null_set(void)
{
    DStringView empty = view_lit("");

    D_TEST_EXPR(d_string_view_find_first_char_in_set_from_start(empty, NULL) == NOT_FOUND);
    D_TEST_EXPR(d_string_view_find_last_char_in_set_from_end(empty, NULL) == NOT_FOUND);
}

static void test_find_predicate_not_matching_all_chars(void)
{
    DStringView view = view_lit("abcdef");

    D_TEST_EXPR(d_string_view_find_first_not_matching_predicate_from_start(view, is_lower_a_to_f) == NOT_FOUND);
    D_TEST_EXPR(d_string_view_find_last_not_matching_predicate_from_end(view, is_lower_a_to_f) == NOT_FOUND);
}

static void test_all_find_wrappers_on_one_char_view(void)
{
    DStringView view = view_lit("x");

    D_TEST_EXPR(d_string_view_find_first_matching_char_from_start(view, 'x') == 0);
    D_TEST_EXPR(d_string_view_find_last_matching_char_from_end(view, 'x') == 0);
    D_TEST_EXPR(d_string_view_find_first_not_matching_char_from_start(view, 'x') == NOT_FOUND);
    D_TEST_EXPR(d_string_view_find_last_not_matching_char_from_end(view, 'x') == NOT_FOUND);
    D_TEST_EXPR(d_string_view_find_first_matching_view_from_start(view, view_lit("x")) == 0);
    D_TEST_EXPR(d_string_view_find_last_matching_view_from_end(view, view_lit("x")) == 0);
}

static void test_subview_huge_size_no_overflow_expected(void)
{
    DStringView view = view_lit("abcdef");
    DStringView sub = d_string_view_subview(view, 2, MAX_SIZE_T_VALUE);

    expect_view(sub, "cdef", 4);
}

int main(void)
{
    DTest tests[] = {
        D_TEST_GENERATE_TEST(test_from_parts_keeps_pointer_and_size),
        D_TEST_GENERATE_TEST(test_from_parts_preserves_embedded_nul),
        D_TEST_GENERATE_TEST(test_from_parts_null_data_forces_empty_view),
        D_TEST_GENERATE_TEST(test_from_c_string_sets_strlen_size),
        D_TEST_GENERATE_TEST(test_from_c_string_empty_string_is_empty_but_data_valid),
        D_TEST_GENERATE_TEST(test_from_c_string_null_returns_null_empty),
        D_TEST_GENERATE_TEST(test_from_dyn_string_null_returns_empty),
        D_TEST_GENERATE_TEST(test_from_dyn_string_views_dyn_string_buffer),
        D_TEST_GENERATE_TEST(test_len_data_and_is_empty_are_simple_accessors),
        D_TEST_GENERATE_TEST(test_get_char_at_reads_inside_bounds),
        D_TEST_GENERATE_TEST(test_subview_middle),
        D_TEST_GENERATE_TEST(test_subview_clamps_size_past_end),
        D_TEST_GENERATE_TEST(test_subview_pos_equal_size_returns_empty_at_end),
        D_TEST_GENERATE_TEST(test_subview_pos_after_size_returns_null_empty),
        D_TEST_GENERATE_TEST(test_subview_null_view_returns_null_empty),
        D_TEST_GENERATE_TEST(test_subview_zero_size_inside_view_keeps_position),
        D_TEST_GENERATE_TEST(test_substr_middle_allocates_c_string),
        D_TEST_GENERATE_TEST(test_substr_clamps_size_past_end),
        D_TEST_GENERATE_TEST(test_compare_equal_full_c_strings),
        D_TEST_GENERATE_TEST(test_compare_orders_c_strings),
        D_TEST_GENERATE_TEST(test_compare_should_use_view_size_not_strcmp),
        D_TEST_GENERATE_TEST(test_compare_embedded_nul_should_use_full_size),
        D_TEST_GENERATE_TEST(test_equals_c_string_respects_size),
        D_TEST_GENERATE_TEST(test_starts_and_ends_with_char),
        D_TEST_GENERATE_TEST(test_starts_and_ends_with_char_empty_false),
        D_TEST_GENERATE_TEST(test_starts_with_view_cases),
        D_TEST_GENERATE_TEST(test_ends_with_view_cases),
        D_TEST_GENERATE_TEST(test_starts_ends_with_embedded_nul_view),
        D_TEST_GENERATE_TEST(test_find_first_matching_char),
        D_TEST_GENERATE_TEST(test_find_first_not_matching_char),
        D_TEST_GENERATE_TEST(test_find_last_matching_char),
        D_TEST_GENERATE_TEST(test_find_last_not_matching_char),
        D_TEST_GENERATE_TEST(test_find_char_on_empty_and_null_views_returns_not_found),
        D_TEST_GENERATE_TEST(test_find_first_matching_view),
        D_TEST_GENERATE_TEST(test_find_first_matching_view_empty_needle),
        D_TEST_GENERATE_TEST(test_find_first_matching_view_overlapping),
        D_TEST_GENERATE_TEST(test_find_view_with_embedded_nul),
        D_TEST_GENERATE_TEST(test_find_last_matching_view),
        D_TEST_GENERATE_TEST(test_find_last_matching_view_empty_needle),
        D_TEST_GENERATE_TEST(test_find_c_string_wrappers),
        D_TEST_GENERATE_TEST(test_find_char_in_set),
        D_TEST_GENERATE_TEST(test_find_char_not_in_set),
        D_TEST_GENERATE_TEST(test_find_last_char_in_set),
        D_TEST_GENERATE_TEST(test_find_last_char_not_in_set),
        D_TEST_GENERATE_TEST(test_find_with_empty_set),
        D_TEST_GENERATE_TEST(test_find_predicate_first),
        D_TEST_GENERATE_TEST(test_find_predicate_last),
        D_TEST_GENERATE_TEST(test_trim_left_by_char),
        D_TEST_GENERATE_TEST(test_trim_right_by_char),
        D_TEST_GENERATE_TEST(test_trim_by_predicate),
        D_TEST_GENERATE_TEST(test_trim_empty_and_null_views),
        D_TEST_GENERATE_TEST(test_dyn_string_new_from_string_view_copies_exact_size),
        D_TEST_GENERATE_TEST(test_dyn_string_new_from_string_view_rejects_null_output),
        D_TEST_GENERATE_TEST(test_split_by_char_basic),
        D_TEST_GENERATE_TEST(test_split_by_char_skips_empty_tokens),
        D_TEST_GENERATE_TEST(test_split_by_char_no_separator_returns_whole_string),
        D_TEST_GENERATE_TEST(test_split_by_char_empty_and_only_separators_return_empty_array),
        D_TEST_GENERATE_TEST(test_split_by_char_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_split_by_char_of_str_basic),
        D_TEST_GENERATE_TEST(test_split_by_char_of_str_skips_empty_tokens),
        D_TEST_GENERATE_TEST(test_split_by_char_of_str_empty_separator_set_returns_whole_string),
        D_TEST_GENERATE_TEST(test_split_by_char_of_str_rejects_null_args),
        D_TEST_GENERATE_TEST(test_find_prefix_suffix_should_not_read_null_for_empty_pattern),
        D_TEST_GENERATE_TEST(test_predicate_on_empty_view_does_not_call_null_fn),
        D_TEST_GENERATE_TEST(test_set_search_on_empty_view_does_not_read_null_set),
        D_TEST_GENERATE_TEST(test_find_predicate_not_matching_all_chars),
        D_TEST_GENERATE_TEST(test_all_find_wrappers_on_one_char_view),
        D_TEST_GENERATE_TEST(test_subview_huge_size_no_overflow_expected),
    };

    D_TEST_RUN_TESTS(tests);
    return 0;
}
