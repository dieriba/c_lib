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

static void make_string(DDynString *s, const char *str)
{
    D_TEST_EXPR(d_dyn_string_init_from_c_string(s, str) == D_OK);
}

static void test_init_returns_ok(void)
{
    DDynString s;

    D_TEST_EXPR(d_dyn_string_init(&s) == D_OK);
    d_dyn_string_destroy(&s);
}

static void test_init_creates_empty_string(void)
{
    DDynString s;

    d_dyn_string_init(&s);
    expect_c_string(&s, "");
    d_dyn_string_destroy(&s);
}

static void test_init_creates_size_zero(void)
{
    DDynString s;

    d_dyn_string_init(&s);
    expect_size(&s, 0);
    d_dyn_string_destroy(&s);
}

static void test_init_creates_zero_terminated_string(void)
{
    DDynString s;

    d_dyn_string_init(&s);
    expect_zero_terminated(&s);
    d_dyn_string_destroy(&s);
}

static void test_init_rejects_null_pointer(void)
{
    D_TEST_EXPR(d_dyn_string_init(NULL) == D_ERR_INVALID_ARG);
}

static void test_init_with_capacity_returns_ok(void)
{
    DDynString s;

    D_TEST_EXPR(d_dyn_string_init_with_capacity(&s, 64) == D_OK);
    d_dyn_string_destroy(&s);
}

static void test_init_with_capacity_sets_capacity(void)
{
    DDynString s;

    d_dyn_string_init_with_capacity(&s, 64);
    expect_capacity_at_least(&s, 64);
    d_dyn_string_destroy(&s);
}

static void test_init_with_capacity_creates_empty_string(void)
{
    DDynString s;

    d_dyn_string_init_with_capacity(&s, 64);
    expect_string(&s, "");
    d_dyn_string_destroy(&s);
}

static void test_init_with_capacity_rejects_null_pointer(void)
{
    D_TEST_EXPR(d_dyn_string_init_with_capacity(NULL, 64) == D_ERR_INVALID_ARG);
}

static void test_init_with_capacity_accepts_zero_capacity(void)
{
    DDynString s;

    D_TEST_EXPR(d_dyn_string_init_with_capacity(&s, 0) == D_OK);
    expect_string(&s, "");
    d_dyn_string_destroy(&s);
}

static void test_init_from_c_string_returns_ok(void)
{
    DDynString s;

    D_TEST_EXPR(d_dyn_string_init_from_c_string(&s, "hello") == D_OK);
    d_dyn_string_destroy(&s);
}

static void test_init_from_c_string_copies_content(void)
{
    DDynString s;

    d_dyn_string_init_from_c_string(&s, "hello");
    expect_string(&s, "hello");
    d_dyn_string_destroy(&s);
}

static void test_init_from_c_string_does_not_alias_source(void)
{
    DDynString s;
    char source[] = "hello";

    d_dyn_string_init_from_c_string(&s, source);
    source[0] = 'H';
    expect_string(&s, "hello");
    d_dyn_string_destroy(&s);
}

static void test_init_from_c_string_accepts_empty_string(void)
{
    DDynString s;

    D_TEST_EXPR(d_dyn_string_init_from_c_string(&s, "") == D_OK);
    expect_string(&s, "");
    d_dyn_string_destroy(&s);
}

static void test_init_from_c_string_rejects_null_pointer(void)
{
    D_TEST_EXPR(d_dyn_string_init_from_c_string(NULL, "abc") == D_ERR_INVALID_ARG);
}

static void test_init_from_c_string_rejects_null_string(void)
{
    DDynString s;

    D_TEST_EXPR(d_dyn_string_init_from_c_string(&s, NULL) == D_ERR_INVALID_ARG);
}

static void test_init_with_sub_string_copies_middle(void)
{
    DDynString s;

    D_TEST_EXPR(d_dyn_string_init_with_sub_string(&s, "hello world", 6, 5) == D_OK);
    expect_string(&s, "world");
    d_dyn_string_destroy(&s);
}

static void test_init_with_sub_string_clamps_size_past_end(void)
{
    DDynString s;

    D_TEST_EXPR(d_dyn_string_init_with_sub_string(&s, "abcdef", 4, 100) == D_OK);
    expect_string(&s, "ef");
    d_dyn_string_destroy(&s);
}

static void test_init_with_sub_string_accepts_pos_equal_len(void)
{
    DDynString s;

    D_TEST_EXPR(d_dyn_string_init_with_sub_string(&s, "abc", 3, 10) == D_OK);
    expect_string(&s, "");
    d_dyn_string_destroy(&s);
}

static void test_init_with_sub_string_rejects_null_pointer(void)
{
    D_TEST_EXPR(d_dyn_string_init_with_sub_string(NULL, "abc", 0, 1) == D_ERR_INVALID_ARG);
}

static void test_init_with_sub_string_rejects_null_source(void)
{
    DDynString s;

    D_TEST_EXPR(d_dyn_string_init_with_sub_string(&s, NULL, 0, 1) == D_ERR_INVALID_ARG);
}

static void test_init_with_sub_string_rejects_pos_after_len(void)
{
    DDynString s;

    D_TEST_EXPR(d_dyn_string_init_with_sub_string(&s, "abc", 4, 1) == D_ERR_INVALID_ARG);
}

static void test_init_with_sub_string_copies_prefix(void)
{
    DDynString s;

    D_TEST_EXPR(d_dyn_string_init_with_sub_string(&s, "abcdef", 0, 3) == D_OK);
    expect_string(&s, "abc");
    d_dyn_string_destroy(&s);
}

static void test_init_with_sub_string_zero_size_returns_empty(void)
{
    DDynString s;

    D_TEST_EXPR(d_dyn_string_init_with_sub_string(&s, "abcdef", 2, 0) == D_OK);
    expect_string(&s, "");
    d_dyn_string_destroy(&s);
}

static void test_init_from_dstring_copies_content(void)
{
    DDynString src;
    DDynString copy;

    make_string(&src, "copy me");
    D_TEST_EXPR(d_dyn_string_init_from_dstring(&copy, &src) == D_OK);
    expect_string(&copy, "copy me");
    d_dyn_string_destroy(&copy);
    d_dyn_string_destroy(&src);
}

static void test_init_from_dstring_does_not_alias_source(void)
{
    DDynString src;
    DDynString copy;

    make_string(&src, "copy me");
    d_dyn_string_init_from_dstring(&copy, &src);
    d_dyn_string_replace_from_str(&src, "changed");
    expect_string(&copy, "copy me");
    d_dyn_string_destroy(&copy);
    d_dyn_string_destroy(&src);
}

static void test_init_from_dstring_rejects_null_source(void)
{
    DDynString copy;

    D_TEST_EXPR(d_dyn_string_init_from_dstring(&copy, NULL) == D_ERR_INVALID_ARG);
}

static void test_init_from_dstring_rejects_null_pointer(void)
{
    DDynString src;

    make_string(&src, "abc");
    D_TEST_EXPR(d_dyn_string_init_from_dstring(NULL, &src) == D_ERR_INVALID_ARG);
    d_dyn_string_destroy(&src);
}

static void test_push_char_appends_to_empty_string(void)
{
    DDynString s;

    make_string(&s, "");
    D_TEST_EXPR(d_dyn_string_push_char(&s, 'a') == D_OK);
    expect_string(&s, "a");
    d_dyn_string_destroy(&s);
}

static void test_push_char_appends_after_existing_content(void)
{
    DDynString s;

    make_string(&s, "ab");
    D_TEST_EXPR(d_dyn_string_push_char(&s, 'c') == D_OK);
    expect_string(&s, "abc");
    d_dyn_string_destroy(&s);
}

static void test_push_c_str_appends_to_empty_string(void)
{
    DDynString s;

    make_string(&s, "");
    D_TEST_EXPR(d_dyn_string_push_c_str(&s, "abc") == D_OK);
    expect_string(&s, "abc");
    d_dyn_string_destroy(&s);
}

static void test_push_c_str_appends_after_existing_content(void)
{
    DDynString s;

    make_string(&s, "hello");
    D_TEST_EXPR(d_dyn_string_push_c_str(&s, " world") == D_OK);
    expect_string(&s, "hello world");
    d_dyn_string_destroy(&s);
}

static void test_push_c_str_accepts_empty_append(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_push_c_str(&s, "") == D_OK);
    expect_string(&s, "abc");
    d_dyn_string_destroy(&s);
}

static void test_push_str_with_len_appends_exact_len(void)
{
    DDynString s;

    make_string(&s, "ab");
    D_TEST_EXPR(d_dyn_string_push_str_with_len(&s, "cdef", 2) == D_OK);
    expect_string(&s, "abcd");
    d_dyn_string_destroy(&s);
}

static void test_push_str_with_len_can_append_zero_bytes(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_push_str_with_len(&s, "xyz", 0) == D_OK);
    expect_string(&s, "abc");
    d_dyn_string_destroy(&s);
}

static void test_merge_appends_right_to_left(void)
{
    DDynString left;
    DDynString right;

    make_string(&left, "hello");
    make_string(&right, " world");
    D_TEST_EXPR(d_dyn_string_merge(&left, &right) == D_OK);
    expect_string(&left, "hello world");
    d_dyn_string_destroy(&right);
    d_dyn_string_destroy(&left);
}

static void test_merge_does_not_modify_right(void)
{
    DDynString left;
    DDynString right;

    make_string(&left, "hello");
    make_string(&right, " world");
    d_dyn_string_merge(&left, &right);
    expect_string(&right, " world");
    d_dyn_string_destroy(&right);
    d_dyn_string_destroy(&left);
}

static void test_merge_rejects_null_left(void)
{
    DDynString right;

    make_string(&right, "abc");
    D_TEST_EXPR(d_dyn_string_merge(NULL, &right) == D_ERR_INVALID_ARG);
    d_dyn_string_destroy(&right);
}

static void test_merge_rejects_null_right(void)
{
    DDynString left;

    make_string(&left, "abc");
    D_TEST_EXPR(d_dyn_string_merge(&left, NULL) == D_ERR_INVALID_ARG);
    d_dyn_string_destroy(&left);
}

static void test_replace_from_str_replaces_shorter_content(void)
{
    DDynString s;

    make_string(&s, "old long value");
    D_TEST_EXPR(d_dyn_string_replace_from_str(&s, "new") == D_OK);
    expect_string(&s, "new");
    d_dyn_string_destroy(&s);
}

static void test_replace_from_str_replaces_longer_content(void)
{
    DDynString s;

    make_string(&s, "old");
    D_TEST_EXPR(d_dyn_string_replace_from_str(&s, "a longer new value") == D_OK);
    expect_string(&s, "a longer new value");
    d_dyn_string_destroy(&s);
}

static void test_replace_from_str_accepts_empty_string(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_replace_from_str(&s, "") == D_OK);
    expect_string(&s, "");
    d_dyn_string_destroy(&s);
}

static void test_replace_from_str_rejects_null_dstring(void)
{
    D_TEST_EXPR(d_dyn_string_replace_from_str(NULL, "abc") == D_ERR_INVALID_ARG);
}

static void test_replace_from_str_rejects_null_source(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_replace_from_str(&s, NULL) == D_ERR_INVALID_ARG);
    d_dyn_string_destroy(&s);
}

static void test_replace_from_dstring_replaces_content(void)
{
    DDynString dst;
    DDynString src;

    make_string(&dst, "dst");
    make_string(&src, "source value");
    D_TEST_EXPR(d_dyn_string_replace_from_dstring(&dst, &src) == D_OK);
    expect_string(&dst, "source value");
    d_dyn_string_destroy(&src);
    d_dyn_string_destroy(&dst);
}

static void test_replace_from_dstring_does_not_alias_source(void)
{
    DDynString dst;
    DDynString src;

    make_string(&dst, "dst");
    make_string(&src, "source value");
    d_dyn_string_replace_from_dstring(&dst, &src);
    d_dyn_string_replace_from_str(&src, "changed");
    expect_string(&dst, "source value");
    d_dyn_string_destroy(&src);
    d_dyn_string_destroy(&dst);
}

static void test_replace_from_dstring_rejects_null_destination(void)
{
    DDynString src;

    make_string(&src, "abc");
    D_TEST_EXPR(d_dyn_string_replace_from_dstring(NULL, &src) == D_ERR_INVALID_ARG);
    d_dyn_string_destroy(&src);
}

static void test_replace_from_dstring_rejects_null_source(void)
{
    DDynString dst;

    make_string(&dst, "abc");
    D_TEST_EXPR(d_dyn_string_replace_from_dstring(&dst, NULL) == D_ERR_INVALID_ARG);
    d_dyn_string_destroy(&dst);
}

static void test_sub_string_in_place_keeps_middle(void)
{
    DDynString s;

    make_string(&s, "hello world");
    D_TEST_EXPR(d_dyn_string_sub_string_in_place(&s, 6, 5) == D_OK);
    expect_string(&s, "world");
    d_dyn_string_destroy(&s);
}

static void test_sub_string_in_place_clamps_size_past_end(void)
{
    DDynString s;

    make_string(&s, "abcdef");
    D_TEST_EXPR(d_dyn_string_sub_string_in_place(&s, 2, 100) == D_OK);
    expect_string(&s, "cdef");
    d_dyn_string_destroy(&s);
}

static void test_sub_string_in_place_rejects_pos_equal_size(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_sub_string_in_place(&s, 3, 1) == D_ERR_INVALID_ARG);
    expect_string(&s, "abc");
    d_dyn_string_destroy(&s);
}

static void test_sub_string_in_place_rejects_pos_after_size(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_sub_string_in_place(&s, 4, 1) == D_ERR_INVALID_ARG);
    expect_string(&s, "abc");
    d_dyn_string_destroy(&s);
}

static void test_get_char_at_reads_first_character(void)
{
    DDynString s;
    char c = 0;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_get_char_at(&s, 0, &c) == D_OK);
    D_TEST_EXPR(c == 'a');
    d_dyn_string_destroy(&s);
}

static void test_get_char_at_reads_middle_character(void)
{
    DDynString s;
    char c = 0;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_get_char_at(&s, 1, &c) == D_OK);
    D_TEST_EXPR(c == 'b');
    d_dyn_string_destroy(&s);
}

static void test_resize_grow_fills_new_slots(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_resize(&s, 6, 'x') == D_OK);
    expect_string(&s, "abcxxx");
    d_dyn_string_destroy(&s);
}

static void test_resize_shrink_truncates_content(void)
{
    DDynString s;

    make_string(&s, "abcdef");
    D_TEST_EXPR(d_dyn_string_resize(&s, 3, 'x') == D_OK);
    expect_string(&s, "abc");
    d_dyn_string_destroy(&s);
}

static void test_resize_to_zero_makes_empty_string(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_resize(&s, 0, 'x') == D_OK);
    expect_string(&s, "");
    d_dyn_string_destroy(&s);
}

static void test_destroy_accepts_null_pointer(void)
{
    d_dyn_string_destroy(NULL);
    D_TEST_EXPR(true);
}

static void test_destroy_is_safe(void)
{
    DDynString s;

    make_string(&s, "abc");
    d_dyn_string_destroy(&s);
    D_TEST_EXPR(true);
}

static void test_destroy_empty_string_is_safe(void)
{
    DDynString s;

    D_TEST_EXPR(d_dyn_string_init(&s) == D_OK);
    d_dyn_string_destroy(&s);
    D_TEST_EXPR(true);
}

static void test_get_string_rejects_null_dstring_safely(void)
{
    D_TEST_NULL(d_dyn_string_get_string(NULL));
}

static void test_get_size_rejects_null_dstring(void)
{
    usize size = 123;

    D_TEST_EXPR(d_dyn_string_get_size(NULL, &size) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(size == 123);
}

static void test_get_size_rejects_null_output_pointer(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_get_size(&s, NULL) == D_ERR_INVALID_ARG);
    d_dyn_string_destroy(&s);
}

static void test_get_capacity_rejects_null_dstring(void)
{
    usize capacity = 123;

    D_TEST_EXPR(d_dyn_string_get_capacity(NULL, &capacity) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(capacity == 123);
}

static void test_get_capacity_rejects_null_output_pointer(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_get_capacity(&s, NULL) == D_ERR_INVALID_ARG);
    d_dyn_string_destroy(&s);
}

static void test_push_char_many_times_preserves_order_and_terminator(void)
{
    DDynString s;
    char expected[129];

    make_string(&s, "");
    for (usize i = 0; i < 128; i++)
    {
        char c = (char)('a' + (i % 26));
        expected[i] = c;
        D_TEST_EXPR(d_dyn_string_push_char(&s, c) == D_OK);
        expect_size(&s, i + 1);
        expect_zero_terminated(&s);
    }
    expected[128] = '\0';
    expect_string(&s, expected);
    d_dyn_string_destroy(&s);
}

static void test_push_char_rejects_null_dstring(void)
{
    D_TEST_EXPR(d_dyn_string_push_char(NULL, 'x') == D_ERR_INVALID_ARG);
}

static void test_push_c_str_rejects_null_dstring(void)
{
    D_TEST_EXPR(d_dyn_string_push_c_str(NULL, "abc") == D_ERR_INVALID_ARG);
}

static void test_push_c_str_rejects_null_source(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_push_c_str(&s, NULL) == D_ERR_INVALID_ARG);
    expect_string(&s, "abc");
    d_dyn_string_destroy(&s);
}

static void expect_memory_prefix(DDynString *s, const char *expected, usize expected_size)
{
    const char *data = NULL;

    data = d_dyn_string_get_string(s);
    D_TEST_NOT_NULL(data);
    expect_size(s, expected_size);
    D_TEST_MEM_EQ(data, expected, expected_size);
    D_TEST_EXPR(data[expected_size] == '\0');
}

static void test_push_str_with_len_accepts_embedded_nul_bytes(void)
{
    DDynString s;
    const char payload[] = {'c', '\0', 'd', 'e'};
    const char expected[] = {'a', 'b', 'c', '\0', 'd', 'e'};
    char c = 0;

    make_string(&s, "ab");
    D_TEST_EXPR(d_dyn_string_push_str_with_len(&s, payload, sizeof(payload)) == D_OK);
    expect_memory_prefix(&s, expected, sizeof(expected));
    D_TEST_EXPR(d_dyn_string_get_char_at(&s, 3, &c) == D_OK);
    D_TEST_EXPR(c == '\0');
    D_TEST_EXPR(d_dyn_string_get_char_at(&s, 5, &c) == D_OK);
    D_TEST_EXPR(c == 'e');
    d_dyn_string_destroy(&s);
}

static void test_push_str_with_len_rejects_null_dstring(void)
{
    D_TEST_EXPR(d_dyn_string_push_str_with_len(NULL, "abc", 3) == D_ERR_INVALID_ARG);
}

static void test_push_str_with_len_rejects_null_source_when_size_nonzero(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_push_str_with_len(&s, NULL, 3) == D_ERR_INVALID_ARG);
    expect_string(&s, "abc");
    d_dyn_string_destroy(&s);
}

static void test_push_str_with_len_rejects_null_source_when_size_zero(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_push_str_with_len(&s, NULL, 0) == D_ERR_INVALID_ARG);
    expect_string(&s, "abc");
    d_dyn_string_destroy(&s);
}

static void test_merge_with_empty_right_keeps_left(void)
{
    DDynString left;
    DDynString right;

    make_string(&left, "abc");
    make_string(&right, "");
    D_TEST_EXPR(d_dyn_string_merge(&left, &right) == D_OK);
    expect_string(&left, "abc");
    expect_string(&right, "");
    d_dyn_string_destroy(&right);
    d_dyn_string_destroy(&left);
}

static void test_merge_empty_left_gets_right_content(void)
{
    DDynString left;
    DDynString right;

    make_string(&left, "");
    make_string(&right, "abc");
    D_TEST_EXPR(d_dyn_string_merge(&left, &right) == D_OK);
    expect_string(&left, "abc");
    expect_string(&right, "abc");
    d_dyn_string_destroy(&right);
    d_dyn_string_destroy(&left);
}

static void test_merge_self_duplicates_content(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_merge(&s, &s) == D_OK);
    expect_string(&s, "abcabc");
    d_dyn_string_destroy(&s);
}

static void test_replace_from_str_after_embedded_nul_restores_c_string_invariant(void)
{
    DDynString s;
    const char payload[] = {'c', '\0', 'd'};

    make_string(&s, "ab");
    D_TEST_EXPR(d_dyn_string_push_str_with_len(&s, payload, sizeof(payload)) == D_OK);
    D_TEST_EXPR(d_dyn_string_replace_from_str(&s, "xyz") == D_OK);
    expect_string(&s, "xyz");
    d_dyn_string_destroy(&s);
}

static void test_replace_from_dstring_accepts_empty_source(void)
{
    DDynString dst;
    DDynString src;

    make_string(&dst, "abc");
    make_string(&src, "");
    D_TEST_EXPR(d_dyn_string_replace_from_dstring(&dst, &src) == D_OK);
    expect_string(&dst, "");
    d_dyn_string_destroy(&src);
    d_dyn_string_destroy(&dst);
}

static void test_replace_from_dstring_self_is_noop(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_replace_from_dstring(&s, &s) == D_OK);
    expect_string(&s, "abc");
    d_dyn_string_destroy(&s);
}

static void test_sub_string_in_place_keeps_prefix(void)
{
    DDynString s;

    make_string(&s, "abcdef");
    D_TEST_EXPR(d_dyn_string_sub_string_in_place(&s, 0, 3) == D_OK);
    expect_string(&s, "abc");
    d_dyn_string_destroy(&s);
}

static void test_sub_string_in_place_zero_size_returns_empty(void)
{
    DDynString s;

    make_string(&s, "abcdef");
    D_TEST_EXPR(d_dyn_string_sub_string_in_place(&s, 2, 0) == D_OK);
    expect_string(&s, "");
    d_dyn_string_destroy(&s);
}

static void test_sub_string_in_place_rejects_null_dstring(void)
{
    D_TEST_EXPR(d_dyn_string_sub_string_in_place(NULL, 0, 1) == D_ERR_INVALID_ARG);
}

static void test_sub_string_in_place_on_empty_rejects_without_crash(void)
{
    DDynString s;

    make_string(&s, "");
    D_TEST_EXPR(d_dyn_string_sub_string_in_place(&s, 0, 0) == D_ERR_INVALID_ARG);
    expect_string(&s, "");
    d_dyn_string_destroy(&s);
}

static void test_get_char_at_reads_last_character(void)
{
    DDynString s;
    char c = 0;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_get_char_at(&s, 2, &c) == D_OK);
    D_TEST_EXPR(c == 'c');
    d_dyn_string_destroy(&s);
}

static void test_get_char_at_rejects_index_equal_size(void)
{
    DDynString s;
    char c = 'x';

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_get_char_at(&s, 3, &c) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(c == 'x');
    d_dyn_string_destroy(&s);
}

static void test_get_char_at_rejects_null_dstring(void)
{
    char c = 'x';

    D_TEST_EXPR(d_dyn_string_get_char_at(NULL, 0, &c) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(c == 'x');
}

static void test_get_char_at_rejects_null_output_pointer(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_get_char_at(&s, 0, NULL) == D_ERR_INVALID_ARG);
    d_dyn_string_destroy(&s);
}

static void test_resize_same_size_keeps_content(void)
{
    DDynString s;

    make_string(&s, "abc");
    D_TEST_EXPR(d_dyn_string_resize(&s, 3, 'x') == D_OK);
    expect_string(&s, "abc");
    d_dyn_string_destroy(&s);
}

static void test_resize_empty_grow_fills_all_slots(void)
{
    DDynString s;

    make_string(&s, "");
    D_TEST_EXPR(d_dyn_string_resize(&s, 4, 'z') == D_OK);
    expect_string(&s, "zzzz");
    d_dyn_string_destroy(&s);
}

static void test_resize_rejects_null_dstring(void)
{
    D_TEST_EXPR(d_dyn_string_resize(NULL, 3, 'x') == D_ERR_INVALID_ARG);
}

static void test_push_char_then_replace_then_push_again(void)
{
    DDynString s;

    D_TEST_EXPR(d_dyn_string_init(&s) == D_OK);
    for (int i = 0; i < 64; i++)
        D_TEST_EXPR(d_dyn_string_push_char(&s, 'a') == D_OK);
    D_TEST_EXPR(d_dyn_string_replace_from_str(&s, "reset") == D_OK);
    expect_string(&s, "reset");
    D_TEST_EXPR(d_dyn_string_push_c_str(&s, "!") == D_OK);
    expect_string(&s, "reset!");
    d_dyn_string_destroy(&s);
}

static void test_init_from_dstring_then_mutate_both_independently(void)
{
    DDynString src;
    DDynString copy;

    make_string(&src, "original");
    D_TEST_EXPR(d_dyn_string_init_from_dstring(&copy, &src) == D_OK);
    D_TEST_EXPR(d_dyn_string_push_c_str(&src, "-src") == D_OK);
    D_TEST_EXPR(d_dyn_string_push_c_str(&copy, "-copy") == D_OK);
    expect_string(&src, "original-src");
    expect_string(&copy, "original-copy");
    d_dyn_string_destroy(&copy);
    d_dyn_string_destroy(&src);
}

static void test_merge_chain_three_strings(void)
{
    DDynString a;
    DDynString b;
    DDynString c;

    make_string(&a, "foo");
    make_string(&b, "bar");
    make_string(&c, "baz");
    D_TEST_EXPR(d_dyn_string_merge(&a, &b) == D_OK);
    D_TEST_EXPR(d_dyn_string_merge(&a, &c) == D_OK);
    expect_string(&a, "foobarbaz");
    d_dyn_string_destroy(&c);
    d_dyn_string_destroy(&b);
    d_dyn_string_destroy(&a);
}

static void test_resize_grow_shrink_grow_round_trip(void)
{
    DDynString s;

    D_TEST_EXPR(d_dyn_string_init(&s) == D_OK);
    D_TEST_EXPR(d_dyn_string_resize(&s, 10, 'A') == D_OK);
    expect_string(&s, "AAAAAAAAAA");
    D_TEST_EXPR(d_dyn_string_resize(&s, 3, 'X') == D_OK);
    expect_string(&s, "AAA");
    D_TEST_EXPR(d_dyn_string_resize(&s, 6, 'B') == D_OK);
    expect_string(&s, "AAABBB");
    d_dyn_string_destroy(&s);
}

int main(void)
{
    DTest tests[] = {
        D_TEST_GENERATE_TEST(test_init_returns_ok),
        D_TEST_GENERATE_TEST(test_init_creates_empty_string),
        D_TEST_GENERATE_TEST(test_init_creates_size_zero),
        D_TEST_GENERATE_TEST(test_init_creates_zero_terminated_string),
        D_TEST_GENERATE_TEST(test_init_rejects_null_pointer),
        D_TEST_GENERATE_TEST(test_init_with_capacity_returns_ok),
        D_TEST_GENERATE_TEST(test_init_with_capacity_sets_capacity),
        D_TEST_GENERATE_TEST(test_init_with_capacity_creates_empty_string),
        D_TEST_GENERATE_TEST(test_init_with_capacity_rejects_null_pointer),
        D_TEST_GENERATE_TEST(test_init_with_capacity_accepts_zero_capacity),
        D_TEST_GENERATE_TEST(test_init_from_c_string_returns_ok),
        D_TEST_GENERATE_TEST(test_init_from_c_string_copies_content),
        D_TEST_GENERATE_TEST(test_init_from_c_string_does_not_alias_source),
        D_TEST_GENERATE_TEST(test_init_from_c_string_accepts_empty_string),
        D_TEST_GENERATE_TEST(test_init_from_c_string_rejects_null_pointer),
        D_TEST_GENERATE_TEST(test_init_from_c_string_rejects_null_string),
        D_TEST_GENERATE_TEST(test_init_with_sub_string_copies_middle),
        D_TEST_GENERATE_TEST(test_init_with_sub_string_clamps_size_past_end),
        D_TEST_GENERATE_TEST(test_init_with_sub_string_accepts_pos_equal_len),
        D_TEST_GENERATE_TEST(test_init_with_sub_string_rejects_null_pointer),
        D_TEST_GENERATE_TEST(test_init_with_sub_string_rejects_null_source),
        D_TEST_GENERATE_TEST(test_init_with_sub_string_rejects_pos_after_len),
        D_TEST_GENERATE_TEST(test_init_with_sub_string_copies_prefix),
        D_TEST_GENERATE_TEST(test_init_with_sub_string_zero_size_returns_empty),
        D_TEST_GENERATE_TEST(test_init_from_dstring_copies_content),
        D_TEST_GENERATE_TEST(test_init_from_dstring_does_not_alias_source),
        D_TEST_GENERATE_TEST(test_init_from_dstring_rejects_null_source),
        D_TEST_GENERATE_TEST(test_init_from_dstring_rejects_null_pointer),
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
        D_TEST_GENERATE_TEST(test_destroy_accepts_null_pointer),
        D_TEST_GENERATE_TEST(test_destroy_is_safe),
        D_TEST_GENERATE_TEST(test_destroy_empty_string_is_safe),
        D_TEST_GENERATE_TEST(test_get_string_rejects_null_dstring_safely),
        D_TEST_GENERATE_TEST(test_get_size_rejects_null_dstring),
        D_TEST_GENERATE_TEST(test_get_size_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_get_capacity_rejects_null_dstring),
        D_TEST_GENERATE_TEST(test_get_capacity_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_push_char_many_times_preserves_order_and_terminator),
        D_TEST_GENERATE_TEST(test_push_char_rejects_null_dstring),
        D_TEST_GENERATE_TEST(test_push_c_str_rejects_null_dstring),
        D_TEST_GENERATE_TEST(test_push_c_str_rejects_null_source),
        D_TEST_GENERATE_TEST(test_push_str_with_len_accepts_embedded_nul_bytes),
        D_TEST_GENERATE_TEST(test_push_str_with_len_rejects_null_dstring),
        D_TEST_GENERATE_TEST(test_push_str_with_len_rejects_null_source_when_size_nonzero),
        D_TEST_GENERATE_TEST(test_push_str_with_len_rejects_null_source_when_size_zero),
        D_TEST_GENERATE_TEST(test_merge_with_empty_right_keeps_left),
        D_TEST_GENERATE_TEST(test_merge_empty_left_gets_right_content),
        D_TEST_GENERATE_TEST(test_merge_self_duplicates_content),
        D_TEST_GENERATE_TEST(test_replace_from_str_after_embedded_nul_restores_c_string_invariant),
        D_TEST_GENERATE_TEST(test_replace_from_dstring_accepts_empty_source),
        D_TEST_GENERATE_TEST(test_replace_from_dstring_self_is_noop),
        D_TEST_GENERATE_TEST(test_sub_string_in_place_keeps_prefix),
        D_TEST_GENERATE_TEST(test_sub_string_in_place_zero_size_returns_empty),
        D_TEST_GENERATE_TEST(test_sub_string_in_place_rejects_null_dstring),
        D_TEST_GENERATE_TEST(test_sub_string_in_place_on_empty_rejects_without_crash),
        D_TEST_GENERATE_TEST(test_get_char_at_reads_last_character),
        D_TEST_GENERATE_TEST(test_get_char_at_rejects_index_equal_size),
        D_TEST_GENERATE_TEST(test_get_char_at_rejects_null_dstring),
        D_TEST_GENERATE_TEST(test_get_char_at_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_resize_same_size_keeps_content),
        D_TEST_GENERATE_TEST(test_resize_empty_grow_fills_all_slots),
        D_TEST_GENERATE_TEST(test_resize_rejects_null_dstring),
        D_TEST_GENERATE_TEST(test_push_char_then_replace_then_push_again),
        D_TEST_GENERATE_TEST(test_init_from_dstring_then_mutate_both_independently),
        D_TEST_GENERATE_TEST(test_merge_chain_three_strings),
        D_TEST_GENERATE_TEST(test_resize_grow_shrink_grow_round_trip),
    };

    D_TEST_RUN_TESTS(tests);
    return 0;
}
