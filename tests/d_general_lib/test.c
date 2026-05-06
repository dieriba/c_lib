#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include "d_test.h"
#include "d_general_lib.h"
#include "d_dyn_array.h"
#include "d_string_view.h"
#include "d_types.h"

#define ARRAY_DEFAULT_OPTS ((BufferOpts)0)

static usize arr_size(DDynArray *arr)
{
    usize size = 0;
    D_TEST_EXPR(d_dyn_array_get_size(arr, &size) == D_OK);
    return size;
}

static char *arr_get_str(DDynArray *arr, usize index)
{
    char *out = NULL;
    D_TEST_EXPR(d_dyn_array_get_elem_at(arr, index, &out) == D_OK);
    return out;
}

static void expect_alloc_str(char *s, const char *expected)
{
    D_TEST_NOT_NULL(s);
    D_TEST_STR_EQ(s, expected);
    free(s);
}

static void expect_buf_str(char *ret, char *buf, const char *expected)
{
    D_TEST_EXPR(ret == buf);
    D_TEST_STR_EQ(buf, expected);
}

static void test_substr_middle_copies_expected_slice(void)
{
    expect_alloc_str(d_substr("hello world", 6, 5), "world");
}

static void test_substr_from_start(void)
{
    expect_alloc_str(d_substr("abcdef", 0, 3), "abc");
}

static void test_substr_size_larger_than_remaining_is_clamped(void)
{
    expect_alloc_str(d_substr("abcdef", 4, 100), "ef");
}

static void test_substr_zero_size_inside_string_returns_allocated_empty_string(void)
{
    expect_alloc_str(d_substr("abcdef", 2, 0), "");
}

static void test_substr_pos_equal_strlen_returns_allocated_empty_string(void)
{
    expect_alloc_str(d_substr("abcdef", 6, 10), "");
}

static void test_substr_pos_after_strlen_returns_null(void)
{
    D_TEST_NULL(d_substr("abcdef", 7, 1));
}

static void test_substr_null_string_returns_null(void)
{
    D_TEST_NULL(d_substr(NULL, 0, 1));
}

static void test_substr_does_not_alias_source(void)
{
    char src[] = "abcdef";
    char *sub = d_substr(src, 1, 3);

    D_TEST_NOT_NULL(sub);
    src[1] = 'X';
    D_TEST_STR_EQ(sub, "bcd");
    free(sub);
}

static void test_substr_result_is_nul_terminated_after_clamp(void)
{
    char *sub = d_substr("abc", 1, 99);

    D_TEST_NOT_NULL(sub);
    D_TEST_EXPR(sub[2] == '\0');
    free(sub);
}

static void test_substr_source_with_embedded_nul_uses_c_string_length(void)
{
    const char src[] = {'a', 'b', '\0', 'c', 'd', '\0'};
    expect_alloc_str(d_substr(src, 0, sizeof(src)), "ab");
}

static void test_substr_huge_size_from_valid_pos_does_not_overread(void)
{
    expect_alloc_str(d_substr("abc", 1, MAX_SIZE_T_VALUE), "bc");
}

static void test_substr_huge_pos_returns_null(void)
{
    D_TEST_NULL(d_substr("abc", MAX_SIZE_T_VALUE, 1));
}

static void test_strdup_copies_string(void)
{
    expect_alloc_str(d_strdup("hello"), "hello");
}

static void test_strdup_empty_string_returns_allocated_empty_string(void)
{
    expect_alloc_str(d_strdup(""), "");
}

static void test_strdup_does_not_alias_source(void)
{
    char src[] = "mutable";
    char *copy = d_strdup(src);

    D_TEST_NOT_NULL(copy);
    src[0] = 'M';
    D_TEST_STR_EQ(copy, "mutable");
    free(copy);
}

static void test_strdup_preserves_full_string_until_first_nul(void)
{
    const char src[] = {'a', 'b', '\0', 'c', '\0'};
    expect_alloc_str(d_strdup(src), "ab");
}

static void test_strdup_null_input_should_return_null_not_crash(void)
{
    /* This test intentionally exposes the current strlen(NULL) bug if present. */
    D_TEST_NULL(d_strdup(NULL));
}

static void test_memfill_rejects_null_dst(void)
{
    int filler = 42;
    D_TEST_NULL(memfill(NULL, &filler, sizeof(filler), 3));
}

static void test_memfill_rejects_null_filler(void)
{
    int dst[3] = {0};
    D_TEST_NULL(memfill(dst, NULL, sizeof(int), 3));
}

static void test_memfill_rejects_zero_elem_size(void)
{
    int dst[3] = {0};
    int filler = 42;
    D_TEST_NULL(memfill(dst, &filler, 0, 3));
}

static void test_memfill_zero_slots_is_noop_and_returns_dst(void)
{
    unsigned char dst[4] = {1, 2, 3, 4};
    unsigned char filler = 9;
    void *ret = memfill(dst, &filler, sizeof(filler), 0);

    D_TEST_EXPR(ret == dst);
    D_TEST_EXPR(dst[0] == 1 && dst[1] == 2 && dst[2] == 3 && dst[3] == 4);
}

static void test_memfill_one_byte_many_slots(void)
{
    unsigned char dst[32];
    unsigned char filler = 0xAB;

    memset(dst, 0, sizeof(dst));
    D_TEST_EXPR(memfill(dst, &filler, sizeof(filler), sizeof(dst)) == dst);
    for (usize i = 0; i < sizeof(dst); ++i)
        D_TEST_EXPR(dst[i] == 0xAB);
}

static void test_memfill_int_values(void)
{
    int dst[17];
    int filler = 0x12345678;

    memset(dst, 0, sizeof(dst));
    D_TEST_EXPR(memfill(dst, &filler, sizeof(filler), 17) == dst);
    for (usize i = 0; i < 17; ++i)
        D_TEST_EXPR(dst[i] == filler);
}

static void test_memfill_struct_values_compare_fields_not_padding(void)
{
    typedef struct s_item
    {
        int id;
        char tag;
        long score;
    } t_item;
    t_item dst[9];
    t_item filler = {0};

    filler.id = 12;
    filler.tag = 'x';
    filler.score = 999;
    memset(dst, 0, sizeof(dst));
    D_TEST_EXPR(memfill(dst, &filler, sizeof(filler), 9) == dst);
    for (usize i = 0; i < 9; ++i)
    {
        D_TEST_EXPR(dst[i].id == filler.id);
        D_TEST_EXPR(dst[i].tag == filler.tag);
        D_TEST_EXPR(dst[i].score == filler.score);
    }
}

static void test_memfill_fills_non_power_of_two_slot_count(void)
{
    uint16_t dst[37];
    uint16_t filler = 0xBEEF;

    memset(dst, 0, sizeof(dst));
    D_TEST_EXPR(memfill(dst, &filler, sizeof(filler), 37) == dst);
    for (usize i = 0; i < 37; ++i)
        D_TEST_EXPR(dst[i] == filler);
}

static void test_memfill_binary_blob_element(void)
{
    unsigned char dst[5][3];
    unsigned char filler[3] = {0, 255, 42};

    memset(dst, 0, sizeof(dst));
    D_TEST_EXPR(memfill(dst, filler, sizeof(filler), 5) == dst);
    for (usize i = 0; i < 5; ++i)
        D_TEST_MEM_EQ(dst[i], filler, sizeof(filler));
}

static void test_itoa_ll_zero(void)
{
    expect_alloc_str(d_itoa_ll(0), "0");
}

static void test_itoa_ll_positive_single_digit(void)
{
    expect_alloc_str(d_itoa_ll(7), "7");
}

static void test_itoa_ll_positive_multi_digit(void)
{
    expect_alloc_str(d_itoa_ll(1234567890LL), "1234567890");
}

static void test_itoa_ll_negative_single_digit(void)
{
    expect_alloc_str(d_itoa_ll(-7), "-7");
}

static void test_itoa_ll_negative_multi_digit(void)
{
    expect_alloc_str(d_itoa_ll(-1234567890LL), "-1234567890");
}

static void test_itoa_ll_int_min(void)
{
    expect_alloc_str(d_itoa_ll((int64)INT_MIN), "-2147483648");
}

static void test_itoa_ll_int_max(void)
{
    expect_alloc_str(d_itoa_ll((int64)INT_MAX), "2147483647");
}

static void test_itoa_ll_long_long_large_positive(void)
{
    expect_alloc_str(d_itoa_ll(9223372036854775807LL), "9223372036854775807");
}

static void test_itoa_ll_long_long_min_should_work(void)
{
    expect_alloc_str(d_itoa_ll((int64)LLONG_MIN), "-9223372036854775808");
}

static void test_itoa_ll_no_alloc_returns_same_buffer_and_writes_zero(void)
{
    char buf[64];
    memset(buf, 'X', sizeof(buf));
    expect_buf_str(d_itoa_ll_no_alloc(0, buf), buf, "0");
}

static void test_itoa_ll_no_alloc_negative(void)
{
    char buf[64];
    memset(buf, 'X', sizeof(buf));
    expect_buf_str(d_itoa_ll_no_alloc(-424242, buf), buf, "-424242");
}

static void test_itoa_ll_no_alloc_large_positive(void)
{
    char buf[64];
    memset(buf, 'X', sizeof(buf));
    expect_buf_str(d_itoa_ll_no_alloc(9223372036854775807LL, buf), buf, "9223372036854775807");
}

static void test_itoa_usize_zero(void)
{
    expect_alloc_str(d_itoa_usize((usize)0), "0");
}

static void test_itoa_usize_one(void)
{
    expect_alloc_str(d_itoa_usize((usize)1), "1");
}

static void test_itoa_usize_multi_digit(void)
{
    expect_alloc_str(d_itoa_usize((usize)123456789), "123456789");
}

static void test_itoa_usize_max_value(void)
{
    char expected[64];

    snprintf(expected, sizeof(expected), "%zu", (size_t)MAX_SIZE_T_VALUE);
    expect_alloc_str(d_itoa_usize(MAX_SIZE_T_VALUE), expected);
}

static void test_itoa_usize_no_alloc_returns_same_buffer(void)
{
    char buf[64];
    memset(buf, 'X', sizeof(buf));
    expect_buf_str(d_itoa_usize_no_alloc((usize)987654321, buf), buf, "987654321");
}

static void test_itoa_usize_no_alloc_max_value(void)
{
    char expected[64];
    char buf[64];

    snprintf(expected, sizeof(expected), "%zu", (size_t)MAX_SIZE_T_VALUE);
    memset(buf, 'X', sizeof(buf));
    expect_buf_str(d_itoa_usize_no_alloc(MAX_SIZE_T_VALUE, buf), buf, expected);
}

static void test_split_string_by_char_basic(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_split_string_by_char(&arr, "a,b,c", ',', ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(arr_size(arr) == 3);
    D_TEST_STR_EQ(arr_get_str(arr, 0), "a");
    D_TEST_STR_EQ(arr_get_str(arr, 1), "b");
    D_TEST_STR_EQ(arr_get_str(arr, 2), "c");
    d_dyn_array_destroy(&arr);
}

static void test_split_string_by_char_skips_empty_tokens(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_split_string_by_char(&arr, ",,a,,b,,", ',', ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(arr_size(arr) == 2);
    D_TEST_STR_EQ(arr_get_str(arr, 0), "a");
    D_TEST_STR_EQ(arr_get_str(arr, 1), "b");
    d_dyn_array_destroy(&arr);
}

static void test_split_string_by_char_no_delimiter_returns_whole_string(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_split_string_by_char(&arr, "abc", ',', ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(arr_size(arr) == 1);
    D_TEST_STR_EQ(arr_get_str(arr, 0), "abc");
    d_dyn_array_destroy(&arr);
}

static void test_split_string_by_char_empty_string_returns_empty_array(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_split_string_by_char(&arr, "", ',', ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(arr_size(arr) == 0);
    d_dyn_array_destroy(&arr);
}

static void test_split_string_by_char_only_delimiters_returns_empty_array(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_split_string_by_char(&arr, ",,,,,", ',', ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(arr_size(arr) == 0);
    d_dyn_array_destroy(&arr);
}

static void test_split_string_by_char_rejects_null_output(void)
{
    D_TEST_EXPR(d_split_string_by_char(NULL, "a,b", ',', ARRAY_DEFAULT_OPTS) == D_ERR_INVALID_ARG);
}

static void test_split_string_by_char_null_string_should_reject_not_crash(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_split_string_by_char(&arr, NULL, ',', ARRAY_DEFAULT_OPTS) == D_ERR_INVALID_ARG);
    D_TEST_NULL(arr);
}

static void test_split_string_by_char_consecutive_and_edge_delims_stress(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_split_string_by_char(&arr, ",alpha,,beta,gamma,,,delta,", ',', ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(arr_size(arr) == 4);
    D_TEST_STR_EQ(arr_get_str(arr, 0), "alpha");
    D_TEST_STR_EQ(arr_get_str(arr, 1), "beta");
    D_TEST_STR_EQ(arr_get_str(arr, 2), "gamma");
    D_TEST_STR_EQ(arr_get_str(arr, 3), "delta");
    d_dyn_array_destroy(&arr);
}

static void test_split_string_by_char_many_tokens_growth(void)
{
    DDynArray *arr = NULL;
    char input[512];
    usize pos = 0;

    for (usize i = 0; i < 80; ++i)
    {
        input[pos++] = 'x';
        input[pos++] = ',';
    }
    input[pos] = '\0';
    D_TEST_EXPR(d_split_string_by_char(&arr, input, ',', ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(arr_size(arr) == 80);
    for (usize i = 0; i < 80; ++i)
        D_TEST_STR_EQ(arr_get_str(arr, i), "x");
    d_dyn_array_destroy(&arr);
}

static void test_split_string_by_char_of_str_basic_multiple_delims(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_split_string_by_char_of_str(&arr, "a,b;c|d", ",;|", ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(arr_size(arr) == 4);
    D_TEST_STR_EQ(arr_get_str(arr, 0), "a");
    D_TEST_STR_EQ(arr_get_str(arr, 1), "b");
    D_TEST_STR_EQ(arr_get_str(arr, 2), "c");
    D_TEST_STR_EQ(arr_get_str(arr, 3), "d");
    d_dyn_array_destroy(&arr);
}

static void test_split_string_by_char_of_str_skips_empty_tokens(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_split_string_by_char_of_str(&arr, ",;a;;b,,", ",;", ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(arr_size(arr) == 2);
    D_TEST_STR_EQ(arr_get_str(arr, 0), "a");
    D_TEST_STR_EQ(arr_get_str(arr, 1), "b");
    d_dyn_array_destroy(&arr);
}

static void test_split_string_by_char_of_str_empty_delims_returns_whole_string(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_split_string_by_char_of_str(&arr, "abc", "", ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(arr_size(arr) == 1);
    D_TEST_STR_EQ(arr_get_str(arr, 0), "abc");
    d_dyn_array_destroy(&arr);
}

static void test_split_string_by_char_of_str_all_delims_returns_empty_array(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_split_string_by_char_of_str(&arr, ";,;,", ",;", ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(arr_size(arr) == 0);
    d_dyn_array_destroy(&arr);
}

static void test_split_string_by_char_of_str_rejects_null_output(void)
{
    D_TEST_EXPR(d_split_string_by_char_of_str(NULL, "a,b", ",", ARRAY_DEFAULT_OPTS) == D_ERR_INVALID_ARG);
}

static void test_split_string_by_char_of_str_rejects_null_delims(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_split_string_by_char_of_str(&arr, "a,b", NULL, ARRAY_DEFAULT_OPTS) == D_ERR_INVALID_ARG);
    D_TEST_NULL(arr);
}

static void test_split_string_by_char_of_str_null_string_should_reject_not_crash(void)
{
    DDynArray *arr = NULL;

    D_TEST_EXPR(d_split_string_by_char_of_str(&arr, NULL, ",", ARRAY_DEFAULT_OPTS) == D_ERR_INVALID_ARG);
    D_TEST_NULL(arr);
}

static void test_split_string_results_are_independent_allocations(void)
{
    DDynArray *arr = NULL;
    char input[] = "left,right";
    char *first = NULL;
    char *second = NULL;

    D_TEST_EXPR(d_split_string_by_char(&arr, input, ',', ARRAY_DEFAULT_OPTS) == D_OK);
    first = arr_get_str(arr, 0);
    second = arr_get_str(arr, 1);
    input[0] = 'X';
    D_TEST_STR_EQ(first, "left");
    D_TEST_STR_EQ(second, "right");
    D_TEST_EXPR(first != input);
    D_TEST_EXPR(second != input + 5);
    d_dyn_array_destroy(&arr);
}

int main(void)
{
    DTest tests[] = {
        D_TEST_GENERATE_TEST(test_substr_middle_copies_expected_slice),
        D_TEST_GENERATE_TEST(test_substr_from_start),
        D_TEST_GENERATE_TEST(test_substr_size_larger_than_remaining_is_clamped),
        D_TEST_GENERATE_TEST(test_substr_zero_size_inside_string_returns_allocated_empty_string),
        D_TEST_GENERATE_TEST(test_substr_pos_equal_strlen_returns_allocated_empty_string),
        D_TEST_GENERATE_TEST(test_substr_pos_after_strlen_returns_null),
        D_TEST_GENERATE_TEST(test_substr_null_string_returns_null),
        D_TEST_GENERATE_TEST(test_substr_does_not_alias_source),
        D_TEST_GENERATE_TEST(test_substr_result_is_nul_terminated_after_clamp),
        D_TEST_GENERATE_TEST(test_substr_source_with_embedded_nul_uses_c_string_length),
        D_TEST_GENERATE_TEST(test_substr_huge_size_from_valid_pos_does_not_overread),
        D_TEST_GENERATE_TEST(test_substr_huge_pos_returns_null),
        D_TEST_GENERATE_TEST(test_strdup_copies_string),
        D_TEST_GENERATE_TEST(test_strdup_empty_string_returns_allocated_empty_string),
        D_TEST_GENERATE_TEST(test_strdup_does_not_alias_source),
        D_TEST_GENERATE_TEST(test_strdup_preserves_full_string_until_first_nul),
        D_TEST_GENERATE_TEST(test_strdup_null_input_should_return_null_not_crash),
        D_TEST_GENERATE_TEST(test_memfill_rejects_null_dst),
        D_TEST_GENERATE_TEST(test_memfill_rejects_null_filler),
        D_TEST_GENERATE_TEST(test_memfill_rejects_zero_elem_size),
        D_TEST_GENERATE_TEST(test_memfill_zero_slots_is_noop_and_returns_dst),
        D_TEST_GENERATE_TEST(test_memfill_one_byte_many_slots),
        D_TEST_GENERATE_TEST(test_memfill_int_values),
        D_TEST_GENERATE_TEST(test_memfill_struct_values_compare_fields_not_padding),
        D_TEST_GENERATE_TEST(test_memfill_fills_non_power_of_two_slot_count),
        D_TEST_GENERATE_TEST(test_memfill_binary_blob_element),
        D_TEST_GENERATE_TEST(test_itoa_ll_zero),
        D_TEST_GENERATE_TEST(test_itoa_ll_positive_single_digit),
        D_TEST_GENERATE_TEST(test_itoa_ll_positive_multi_digit),
        D_TEST_GENERATE_TEST(test_itoa_ll_negative_single_digit),
        D_TEST_GENERATE_TEST(test_itoa_ll_negative_multi_digit),
        D_TEST_GENERATE_TEST(test_itoa_ll_int_min),
        D_TEST_GENERATE_TEST(test_itoa_ll_int_max),
        D_TEST_GENERATE_TEST(test_itoa_ll_long_long_large_positive),
        D_TEST_GENERATE_TEST(test_itoa_ll_long_long_min_should_work),
        D_TEST_GENERATE_TEST(test_itoa_ll_no_alloc_returns_same_buffer_and_writes_zero),
        D_TEST_GENERATE_TEST(test_itoa_ll_no_alloc_negative),
        D_TEST_GENERATE_TEST(test_itoa_ll_no_alloc_large_positive),
        D_TEST_GENERATE_TEST(test_itoa_usize_zero),
        D_TEST_GENERATE_TEST(test_itoa_usize_one),
        D_TEST_GENERATE_TEST(test_itoa_usize_multi_digit),
        D_TEST_GENERATE_TEST(test_itoa_usize_max_value),
        D_TEST_GENERATE_TEST(test_itoa_usize_no_alloc_returns_same_buffer),
        D_TEST_GENERATE_TEST(test_itoa_usize_no_alloc_max_value),
        D_TEST_GENERATE_TEST(test_split_string_by_char_basic),
        D_TEST_GENERATE_TEST(test_split_string_by_char_skips_empty_tokens),
        D_TEST_GENERATE_TEST(test_split_string_by_char_no_delimiter_returns_whole_string),
        D_TEST_GENERATE_TEST(test_split_string_by_char_empty_string_returns_empty_array),
        D_TEST_GENERATE_TEST(test_split_string_by_char_only_delimiters_returns_empty_array),
        D_TEST_GENERATE_TEST(test_split_string_by_char_rejects_null_output),
        D_TEST_GENERATE_TEST(test_split_string_by_char_null_string_should_reject_not_crash),
        D_TEST_GENERATE_TEST(test_split_string_by_char_consecutive_and_edge_delims_stress),
        D_TEST_GENERATE_TEST(test_split_string_by_char_many_tokens_growth),
        D_TEST_GENERATE_TEST(test_split_string_by_char_of_str_basic_multiple_delims),
        D_TEST_GENERATE_TEST(test_split_string_by_char_of_str_skips_empty_tokens),
        D_TEST_GENERATE_TEST(test_split_string_by_char_of_str_empty_delims_returns_whole_string),
        D_TEST_GENERATE_TEST(test_split_string_by_char_of_str_all_delims_returns_empty_array),
        D_TEST_GENERATE_TEST(test_split_string_by_char_of_str_rejects_null_output),
        D_TEST_GENERATE_TEST(test_split_string_by_char_of_str_rejects_null_delims),
        D_TEST_GENERATE_TEST(test_split_string_by_char_of_str_null_string_should_reject_not_crash),
        D_TEST_GENERATE_TEST(test_split_string_results_are_independent_allocations),
    };

    D_TEST_RUN_TESTS(tests);
    return 0;
}
