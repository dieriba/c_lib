#include <string.h>
#include "d_test.h"
#include "d_dyn_string.h"
#include <stdio.h>
#include "d_types.h"

static void expect_size(DDynString *s, usize expected)
{
    usize size = 0;

    D_TEST_EXPR(d_dyn_string_get_size(s, &size) == D_OK);
    D_TEST_EXPR(size == expected);
}

static void expect_capacity_at_least(DDynString *s, usize expected_min)
{
    usize capacity = 0;

    D_TEST_EXPR(d_dyn_string_get_capacity(s, &capacity) == D_OK);
    D_TEST_EXPR(capacity >= expected_min);
}

static void expect_c_string(DDynString *s, const char *expected)
{
    D_TEST_STR_EQ(d_dyn_string_get_string(s), expected);
}

static void expect_zero_terminated(DDynString *s)
{
    usize size = 0;
    const char *str = NULL;

    D_TEST_EXPR(d_dyn_string_get_size(s, &size) == D_OK);
    str = d_dyn_string_get_string(s);
    D_TEST_NOT_NULL(str);
    D_TEST_EXPR(str[size] == '\0');
}

static void expect_string(DDynString *s, const char *expected)
{
    D_TEST_NOT_NULL(s);
    expect_size(s, strlen(expected));
    expect_c_string(s, expected);
    expect_capacity_at_least(s, strlen(expected));
    expect_zero_terminated(s);
}

static DDynString *make_string(const char *str)
{
    DDynString *s = NULL;

    D_TEST_EXPR(d_dyn_string_new_from_c_string(&s, str) == D_OK);
    return s;
}

static void test_new_returns_ok(void)
{
    DDynString *s = NULL;

    D_TEST_EXPR(d_dyn_string_new(&s) == D_OK);

    d_dyn_string_destroy(&s);
}

static void test_new_sets_output_pointer(void)
{
    DDynString *s = NULL;

    d_dyn_string_new(&s);
    D_TEST_NOT_NULL(s);

    d_dyn_string_destroy(&s);
}

static void test_new_creates_empty_string(void)
{
    DDynString *s = NULL;

    d_dyn_string_new(&s);
    expect_c_string(s, "");

    d_dyn_string_destroy(&s);
}

static void test_new_creates_size_zero(void)
{
    DDynString *s = NULL;

    d_dyn_string_new(&s);
    expect_size(s, 0);

    d_dyn_string_destroy(&s);
}

static void test_new_creates_zero_terminated_string(void)
{
    DDynString *s = NULL;

    d_dyn_string_new(&s);
    expect_zero_terminated(s);

    d_dyn_string_destroy(&s);
}

static void test_new_rejects_null_output_pointer(void)
{
    D_TEST_EXPR(d_dyn_string_new(NULL) == D_ERR_INVALID_ARG);
}

static void test_new_with_capacity_returns_ok(void)
{
    DDynString *s = NULL;

    D_TEST_EXPR(d_dyn_string_new_with_capacity(&s, 64) == D_OK);

    d_dyn_string_destroy(&s);
}

static void test_new_with_capacity_sets_capacity(void)
{
    DDynString *s = NULL;

    d_dyn_string_new_with_capacity(&s, 64);
    expect_capacity_at_least(s, 64);

    d_dyn_string_destroy(&s);
}

static void test_new_with_capacity_creates_empty_string(void)
{
    DDynString *s = NULL;

    d_dyn_string_new_with_capacity(&s, 64);
    expect_string(s, "");

    d_dyn_string_destroy(&s);
}

static void test_new_with_capacity_rejects_null_output_pointer(void)
{
    D_TEST_EXPR(d_dyn_string_new_with_capacity(NULL, 64) == D_ERR_INVALID_ARG);
}

static void test_new_from_c_string_returns_ok(void)
{
    DDynString *s = NULL;

    D_TEST_EXPR(d_dyn_string_new_from_c_string(&s, "hello") == D_OK);

    d_dyn_string_destroy(&s);
}

static void test_new_from_c_string_copies_content(void)
{
    DDynString *s = NULL;

    d_dyn_string_new_from_c_string(&s, "hello");
    expect_string(s, "hello");

    d_dyn_string_destroy(&s);
}

static void test_new_from_c_string_does_not_alias_source(void)
{
    DDynString *s = NULL;
    char source[] = "hello";

    d_dyn_string_new_from_c_string(&s, source);
    source[0] = 'H';
    expect_string(s, "hello");

    d_dyn_string_destroy(&s);
}

static void test_new_from_c_string_accepts_empty_string(void)
{
    DDynString *s = NULL;
    D_TEST_EXPR(d_dyn_string_new_from_c_string(&s, "") == D_OK);
    expect_string(s, "");

    d_dyn_string_destroy(&s);
}

static void test_new_from_c_string_rejects_null_output_pointer(void)
{
    D_TEST_EXPR(d_dyn_string_new_from_c_string(NULL, "abc") == D_ERR_INVALID_ARG);
}

static void test_new_from_c_string_rejects_null_string(void)
{
    DDynString *s = NULL;

    D_TEST_EXPR(d_dyn_string_new_from_c_string(&s, NULL) == D_ERR_INVALID_ARG);
    D_TEST_NULL(s);
}

static void test_new_with_sub_string_copies_middle(void)
{
    DDynString *s = NULL;

    D_TEST_EXPR(d_dyn_string_new_with_sub_string(&s, "hello world", 6, 5) == D_OK);
    expect_string(s, "world");

    d_dyn_string_destroy(&s);
}

static void test_new_with_sub_string_clamps_size_past_end(void)
{
    DDynString *s = NULL;

    D_TEST_EXPR(d_dyn_string_new_with_sub_string(&s, "abcdef", 4, 100) == D_OK);
    expect_string(s, "ef");

    d_dyn_string_destroy(&s);
}

static void test_new_with_sub_string_accepts_pos_equal_len(void)
{
    DDynString *s = NULL;

    D_TEST_EXPR(d_dyn_string_new_with_sub_string(&s, "abc", 3, 10) == D_OK);
    expect_string(s, "");

    d_dyn_string_destroy(&s);
}

static void test_new_with_sub_string_rejects_null_output_pointer(void)
{
    D_TEST_EXPR(d_dyn_string_new_with_sub_string(NULL, "abc", 0, 1) == D_ERR_INVALID_ARG);
}

static void test_new_with_sub_string_rejects_null_source(void)
{
    DDynString *s = NULL;

    D_TEST_EXPR(d_dyn_string_new_with_sub_string(&s, NULL, 0, 1) == D_ERR_INVALID_ARG);
    D_TEST_NULL(s);
}

static void test_new_with_sub_string_rejects_pos_after_len(void)
{
    DDynString *s = NULL;

    D_TEST_EXPR(d_dyn_string_new_with_sub_string(&s, "abc", 4, 1) == D_ERR_INVALID_ARG);
    D_TEST_NULL(s);
}

static void test_new_from_dstring_copies_content(void)
{
    DDynString *src = make_string("copy me");
    DDynString *copy = NULL;

    D_TEST_EXPR(d_dyn_string_new_from_dstring(&copy, src) == D_OK);
    expect_string(copy, "copy me");

    d_dyn_string_destroy(&copy);
    d_dyn_string_destroy(&src);
}

static void test_new_from_dstring_does_not_alias_source(void)
{
    DDynString *src = make_string("copy me");
    DDynString *copy = NULL;

    d_dyn_string_new_from_dstring(&copy, src);
    d_dyn_string_replace_from_str(src, "changed");
    expect_string(copy, "copy me");

    d_dyn_string_destroy(&copy);
    d_dyn_string_destroy(&src);
}

static void test_new_from_dstring_rejects_null_source(void)
{
    DDynString *copy = NULL;

    D_TEST_EXPR(d_dyn_string_new_from_dstring(&copy, NULL) == D_ERR_INVALID_ARG);
    D_TEST_NULL(copy);
}

static void test_push_char_appends_to_empty_string(void)
{
    DDynString *s = make_string("");

    D_TEST_EXPR(d_dyn_string_push_char(s, 'a') == D_OK);
    expect_string(s, "a");

    d_dyn_string_destroy(&s);
}

static void test_push_char_appends_after_existing_content(void)
{
    DDynString *s = make_string("ab");

    D_TEST_EXPR(d_dyn_string_push_char(s, 'c') == D_OK);
    expect_string(s, "abc");

    d_dyn_string_destroy(&s);
}

static void test_push_c_str_appends_to_empty_string(void)
{
    DDynString *s = make_string("");

    D_TEST_EXPR(d_dyn_string_push_c_str(s, "abc") == D_OK);
    expect_string(s, "abc");

    d_dyn_string_destroy(&s);
}

static void test_push_c_str_appends_after_existing_content(void)
{
    DDynString *s = make_string("hello");

    D_TEST_EXPR(d_dyn_string_push_c_str(s, " world") == D_OK);
    expect_string(s, "hello world");

    d_dyn_string_destroy(&s);
}

static void test_push_c_str_accepts_empty_append(void)
{
    DDynString *s = make_string("abc");

    D_TEST_EXPR(d_dyn_string_push_c_str(s, "") == D_OK);
    expect_string(s, "abc");

    d_dyn_string_destroy(&s);
}

static void test_push_str_with_len_appends_exact_len(void)
{
    DDynString *s = make_string("ab");

    D_TEST_EXPR(d_dyn_string_push_str_with_len(s, "cdef", 2) == D_OK);
    expect_string(s, "abcd");

    d_dyn_string_destroy(&s);
}

static void test_push_str_with_len_can_append_zero_bytes(void)
{
    DDynString *s = make_string("abc");

    D_TEST_EXPR(d_dyn_string_push_str_with_len(s, "xyz", 0) == D_OK);
    expect_string(s, "abc");

    d_dyn_string_destroy(&s);
}

static void test_merge_appends_right_to_left(void)
{
    DDynString *left = make_string("hello");
    DDynString *right = make_string(" world");

    D_TEST_EXPR(d_dyn_string_merge(left, right) == D_OK);
    expect_string(left, "hello world");

    d_dyn_string_destroy(&right);
    d_dyn_string_destroy(&left);
}

static void test_merge_does_not_modify_right(void)
{
    DDynString *left = make_string("hello");
    DDynString *right = make_string(" world");

    d_dyn_string_merge(left, right);
    expect_string(right, " world");

    d_dyn_string_destroy(&right);
    d_dyn_string_destroy(&left);
}

static void test_merge_rejects_null_left(void)
{
    DDynString *right = make_string("abc");

    D_TEST_EXPR(d_dyn_string_merge(NULL, right) == D_ERR_INVALID_ARG);

    d_dyn_string_destroy(&right);
}

static void test_merge_rejects_null_right(void)
{
    DDynString *left = make_string("abc");

    D_TEST_EXPR(d_dyn_string_merge(left, NULL) == D_ERR_INVALID_ARG);

    d_dyn_string_destroy(&left);
}

static void test_replace_from_str_replaces_shorter_content(void)
{
    DDynString *s = make_string("old long value");

    D_TEST_EXPR(d_dyn_string_replace_from_str(s, "new") == D_OK);
    expect_string(s, "new");

    d_dyn_string_destroy(&s);
}

static void test_replace_from_str_replaces_longer_content(void)
{
    DDynString *s = make_string("old");

    D_TEST_EXPR(d_dyn_string_replace_from_str(s, "a longer new value") == D_OK);
    expect_string(s, "a longer new value");

    d_dyn_string_destroy(&s);
}

static void test_replace_from_str_accepts_empty_string(void)
{
    DDynString *s = make_string("abc");

    D_TEST_EXPR(d_dyn_string_replace_from_str(s, "") == D_OK);
    expect_string(s, "");

    d_dyn_string_destroy(&s);
}

static void test_replace_from_str_rejects_null_dstring(void)
{
    D_TEST_EXPR(d_dyn_string_replace_from_str(NULL, "abc") == D_ERR_INVALID_ARG);
}

static void test_replace_from_str_rejects_null_source(void)
{
    DDynString *s = make_string("abc");

    D_TEST_EXPR(d_dyn_string_replace_from_str(s, NULL) == D_ERR_INVALID_ARG);

    d_dyn_string_destroy(&s);
}

static void test_replace_from_dstring_replaces_content(void)
{
    DDynString *dst = make_string("dst");
    DDynString *src = make_string("source value");

    D_TEST_EXPR(d_dyn_string_replace_from_dstring(dst, src) == D_OK);
    expect_string(dst, "source value");

    d_dyn_string_destroy(&src);
    d_dyn_string_destroy(&dst);
}

static void test_replace_from_dstring_does_not_alias_source(void)
{
    DDynString *dst = make_string("dst");
    DDynString *src = make_string("source value");

    d_dyn_string_replace_from_dstring(dst, src);
    d_dyn_string_replace_from_str(src, "changed");
    expect_string(dst, "source value");

    d_dyn_string_destroy(&src);
    d_dyn_string_destroy(&dst);
}

static void test_replace_from_dstring_rejects_null_destination(void)
{
    DDynString *src = make_string("abc");

    D_TEST_EXPR(d_dyn_string_replace_from_dstring(NULL, src) == D_ERR_INVALID_ARG);

    d_dyn_string_destroy(&src);
}

static void test_replace_from_dstring_rejects_null_source(void)
{
    DDynString *dst = make_string("abc");

    D_TEST_EXPR(d_dyn_string_replace_from_dstring(dst, NULL) == D_ERR_INVALID_ARG);

    d_dyn_string_destroy(&dst);
}

static void test_sub_string_in_place_keeps_middle(void)
{
    DDynString *s = make_string("hello world");

    D_TEST_EXPR(d_dyn_string_sub_string_in_place(s, 6, 5) == D_OK);
    expect_string(s, "world");

    d_dyn_string_destroy(&s);
}

static void test_sub_string_in_place_clamps_size_past_end(void)
{
    DDynString *s = make_string("abcdef");

    D_TEST_EXPR(d_dyn_string_sub_string_in_place(s, 2, 100) == D_OK);
    expect_string(s, "cdef");

    d_dyn_string_destroy(&s);
}

static void test_sub_string_in_place_rejects_pos_equal_size(void)
{
    DDynString *s = make_string("abc");

    D_TEST_EXPR(d_dyn_string_sub_string_in_place(s, 3, 1) == D_ERR_INVALID_ARG);
    expect_string(s, "abc");

    d_dyn_string_destroy(&s);
}

static void test_sub_string_in_place_rejects_pos_after_size(void)
{
    DDynString *s = make_string("abc");

    D_TEST_EXPR(d_dyn_string_sub_string_in_place(s, 4, 1) == D_ERR_INVALID_ARG);
    expect_string(s, "abc");

    d_dyn_string_destroy(&s);
}

static void test_get_char_at_reads_first_character(void)
{
    DDynString *s = make_string("abc");
    char c = 0;

    D_TEST_EXPR(d_dyn_string_get_char_at(s, 0, &c) == D_OK);
    D_TEST_EXPR(c == 'a');

    d_dyn_string_destroy(&s);
}

static void test_get_char_at_reads_middle_character(void)
{
    DDynString *s = make_string("abc");
    char c = 0;

    D_TEST_EXPR(d_dyn_string_get_char_at(s, 1, &c) == D_OK);
    D_TEST_EXPR(c == 'b');

    d_dyn_string_destroy(&s);
}

static void test_resize_grow_fills_new_slots(void)
{
    DDynString *s = make_string("abc");

    D_TEST_EXPR(d_dyn_string_resize(s, 6, 'x') == D_OK);
    expect_string(s, "abcxxx");

    d_dyn_string_destroy(&s);
}

static void test_resize_shrink_truncates_content(void)
{
    DDynString *s = make_string("abcdef");

    D_TEST_EXPR(d_dyn_string_resize(s, 3, 'x') == D_OK);
    expect_string(s, "abc");

    d_dyn_string_destroy(&s);
}

static void test_resize_to_zero_makes_empty_string(void)
{
    DDynString *s = make_string("abc");

    D_TEST_EXPR(d_dyn_string_resize(s, 0, 'x') == D_OK);
    expect_string(s, "");

    d_dyn_string_destroy(&s);
}

static void test_destroy_accepts_null_double_pointer(void)
{
    d_dyn_string_destroy(NULL);
    D_TEST_EXPR(1);
}

static void test_destroy_accepts_null_string_pointer(void)
{
    DDynString *s = NULL;

    d_dyn_string_destroy(&s);
    D_TEST_NULL(s);
}

static void test_destroy_sets_pointer_to_null(void)
{
    DDynString *s = make_string("abc");

    d_dyn_string_destroy(&s);
    D_TEST_NULL(s);
}

int main(void)
{
    DTest tests[] = {
        D_TEST_GENERATE_TEST(test_new_returns_ok),
        D_TEST_GENERATE_TEST(test_new_sets_output_pointer),
        D_TEST_GENERATE_TEST(test_new_creates_empty_string),
        D_TEST_GENERATE_TEST(test_new_creates_size_zero),
        D_TEST_GENERATE_TEST(test_new_creates_zero_terminated_string),
        D_TEST_GENERATE_TEST(test_new_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_new_with_capacity_returns_ok),
        D_TEST_GENERATE_TEST(test_new_with_capacity_sets_capacity),
        D_TEST_GENERATE_TEST(test_new_with_capacity_creates_empty_string),
        D_TEST_GENERATE_TEST(test_new_with_capacity_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_new_from_c_string_returns_ok),
        D_TEST_GENERATE_TEST(test_new_from_c_string_copies_content),
        D_TEST_GENERATE_TEST(test_new_from_c_string_does_not_alias_source),
        D_TEST_GENERATE_TEST(test_new_from_c_string_accepts_empty_string),
        D_TEST_GENERATE_TEST(test_new_from_c_string_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_new_from_c_string_rejects_null_string),
        D_TEST_GENERATE_TEST(test_new_with_sub_string_copies_middle),
        D_TEST_GENERATE_TEST(test_new_with_sub_string_clamps_size_past_end),
        D_TEST_GENERATE_TEST(test_new_with_sub_string_accepts_pos_equal_len),
        D_TEST_GENERATE_TEST(test_new_with_sub_string_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_new_with_sub_string_rejects_null_source),
        D_TEST_GENERATE_TEST(test_new_with_sub_string_rejects_pos_after_len),
        D_TEST_GENERATE_TEST(test_new_from_dstring_copies_content),
        D_TEST_GENERATE_TEST(test_new_from_dstring_does_not_alias_source),
        D_TEST_GENERATE_TEST(test_new_from_dstring_rejects_null_source),
        D_TEST_GENERATE_TEST(test_push_char_appends_to_empty_string),
        D_TEST_GENERATE_TEST(test_push_char_appends_after_existing_content),
        D_TEST_GENERATE_TEST(test_push_c_str_appends_to_empty_string),
        D_TEST_GENERATE_TEST(test_push_c_str_appends_after_existing_content),
        D_TEST_GENERATE_TEST(test_push_c_str_accepts_empty_append),
        D_TEST_GENERATE_TEST(test_push_str_with_len_appends_exact_len),
        D_TEST_GENERATE_TEST(test_push_str_with_len_can_append_zero_bytes),
        D_TEST_GENERATE_TEST(test_merge_appends_right_to_left),
        D_TEST_GENERATE_TEST(test_merge_does_not_modify_right),
        D_TEST_GENERATE_TEST(test_merge_rejects_null_left),
        D_TEST_GENERATE_TEST(test_merge_rejects_null_right),
        D_TEST_GENERATE_TEST(test_replace_from_str_replaces_shorter_content),
        D_TEST_GENERATE_TEST(test_replace_from_str_replaces_longer_content),
        D_TEST_GENERATE_TEST(test_replace_from_str_accepts_empty_string),
        D_TEST_GENERATE_TEST(test_replace_from_str_rejects_null_dstring),
        D_TEST_GENERATE_TEST(test_replace_from_str_rejects_null_source),
        D_TEST_GENERATE_TEST(test_replace_from_dstring_replaces_content),
        D_TEST_GENERATE_TEST(test_replace_from_dstring_does_not_alias_source),
        D_TEST_GENERATE_TEST(test_replace_from_dstring_rejects_null_destination),
        D_TEST_GENERATE_TEST(test_replace_from_dstring_rejects_null_source),
        D_TEST_GENERATE_TEST(test_sub_string_in_place_keeps_middle),
        D_TEST_GENERATE_TEST(test_sub_string_in_place_clamps_size_past_end),
        D_TEST_GENERATE_TEST(test_sub_string_in_place_rejects_pos_equal_size),
        D_TEST_GENERATE_TEST(test_sub_string_in_place_rejects_pos_after_size),
        D_TEST_GENERATE_TEST(test_get_char_at_reads_first_character),
        D_TEST_GENERATE_TEST(test_get_char_at_reads_middle_character),
        D_TEST_GENERATE_TEST(test_resize_grow_fills_new_slots),
        D_TEST_GENERATE_TEST(test_resize_shrink_truncates_content),
        D_TEST_GENERATE_TEST(test_resize_to_zero_makes_empty_string),
        D_TEST_GENERATE_TEST(test_destroy_accepts_null_double_pointer),
        D_TEST_GENERATE_TEST(test_destroy_accepts_null_string_pointer),
        D_TEST_GENERATE_TEST(test_destroy_sets_pointer_to_null),
    };

    D_TEST_RUN_TESTS(tests);
    return 0;
}
