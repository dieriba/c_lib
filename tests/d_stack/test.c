#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "d_test.h"
#include "d_stack.h"
#include "d_types.h"

static void make_int_stack(DStack *stack, usize capacity)
{
    D_TEST_EXPR(d_stack_init(stack, capacity, sizeof(int), NULL, NULL) == D_OK);
}

static void expect_size(DStack *stack, usize expected)
{
    D_TEST_EXPR(d_stack_get_size(stack) == expected);
}

static void expect_capacity_at_least(DStack *stack, usize expected_min)
{
    D_TEST_EXPR(d_stack_get_capacity(stack) >= expected_min);
}

static void expect_empty(DStack *stack, bool expected)
{
    D_TEST_EXPR(d_stack_is_empty(stack) == expected);
}

static void push_int(DStack *stack, int value)
{
    D_TEST_EXPR(d_stack_push(stack, &value) == D_OK);
}

static int pop_int(DStack *stack)
{
    int value = 0x7fffffff;
    D_TEST_EXPR(d_stack_pop(stack, &value) == D_OK);
    return value;
}

static char *owned_string(const char *s)
{
    size_t len = strlen(s);
    char *copy = malloc(len + 1);
    D_TEST_NOT_NULL(copy);
    if (copy != NULL)
        memcpy(copy, s, len + 1);
    return copy;
}

static void string_ptr_free(void *elem_slot)
{
    free(*(char **)elem_slot);
}

static void *string_ptr_copy(void *elem_slot)
{
    char **slot = elem_slot;
    char **new_slot = malloc(sizeof(char *));
    if (new_slot == NULL)
        return NULL;
    *new_slot = strdup(*slot);
    if (*new_slot == NULL)
    {
        free(new_slot);
        return NULL;
    }
    return new_slot;
}

static int g_destroy_count = 0;
static int g_destroy_sum = 0;

static void reset_destroy_tracking(void)
{
    g_destroy_count = 0;
    g_destroy_sum = 0;
}

static void int_destroy_counter(void *elem_slot)
{
    int *value = elem_slot;

    g_destroy_count++;
    g_destroy_sum += *value;
}

typedef struct LargeStruct
{
    uint64_t a;
    uint32_t b;
    char bytes[37];
    int tail;
} LargeStruct;

static LargeStruct make_large_struct(int seed)
{
    LargeStruct value = {0};
    value.a = 0xabcdef0000000000ULL + (uint64_t)seed;
    value.b = (uint32_t)(seed * 17);
    for (usize i = 0; i < sizeof(value.bytes); i++)
        value.bytes[i] = (char)('A' + ((seed + (int)i) % 26));
    value.tail = -seed;
    return value;
}

static void test_init_creates_empty_stack(void)
{
    DStack stack;
    D_TEST_EXPR(d_stack_init(&stack, 0, sizeof(int), NULL, NULL) == D_OK);
    expect_size(&stack, 0);
    expect_empty(&stack, true);
    d_stack_destroy(&stack);
}

static void test_init_with_capacity_sets_capacity(void)
{
    DStack stack;
    D_TEST_EXPR(d_stack_init(&stack, 8, sizeof(int), NULL, NULL) == D_OK);
    expect_capacity_at_least(&stack, 8);
    expect_size(&stack, 0);
    d_stack_destroy(&stack);
}

static void test_destroy_null_pointer_is_safe(void)
{
    d_stack_destroy(NULL);
}

static void test_destroy_empty_stack_is_safe(void)
{
    DStack stack;
    make_int_stack(&stack, 4);
    d_stack_destroy(&stack);
}

static void test_destroy_can_be_called_twice_safely(void)
{
    DStack stack;
    make_int_stack(&stack, 1);
    push_int(&stack, 42);
    d_stack_destroy(&stack);
    d_stack_destroy(&stack);
}

static void test_push_single_value_updates_size_and_empty_flag(void)
{
    DStack stack;
    make_int_stack(&stack, 1);
    push_int(&stack, 42);
    expect_size(&stack, 1);
    expect_empty(&stack, false);
    d_stack_destroy(&stack);
}

static void test_push_copies_value_not_source_address(void)
{
    DStack stack;
    make_int_stack(&stack, 1);
    int value = 123;
    D_TEST_EXPR(d_stack_push(&stack, &value) == D_OK);
    value = 999;
    D_TEST_EXPR(pop_int(&stack) == 123);
    d_stack_destroy(&stack);
}

static void test_pop_returns_last_pushed_value(void)
{
    DStack stack;
    make_int_stack(&stack, 2);
    push_int(&stack, 10);
    push_int(&stack, 20);
    D_TEST_EXPR(pop_int(&stack) == 20);
    D_TEST_EXPR(pop_int(&stack) == 10);
    expect_size(&stack, 0);
    expect_empty(&stack, true);
    d_stack_destroy(&stack);
}

static void test_pop_many_values_are_lifo(void)
{
    DStack stack;
    make_int_stack(&stack, 1);
    for (int i = 0; i < 128; i++)
        push_int(&stack, i);
    expect_size(&stack, 128);
    expect_capacity_at_least(&stack, 128);
    for (int i = 127; i >= 0; i--)
        D_TEST_EXPR(pop_int(&stack) == i);
    expect_size(&stack, 0);
    expect_empty(&stack, true);
    d_stack_destroy(&stack);
}

static void test_pop_empty_stack_fails_and_keeps_output_unchanged(void)
{
    DStack stack;
    make_int_stack(&stack, 1);
    int out = 0x12345678;
    D_TEST_EXPR(d_stack_pop(&stack, &out) != D_OK);
    D_TEST_EXPR(out == 0x12345678);
    expect_size(&stack, 0);
    expect_empty(&stack, true);
    d_stack_destroy(&stack);
}

static void test_pop_until_empty_then_push_again(void)
{
    DStack stack;
    make_int_stack(&stack, 2);
    push_int(&stack, 1);
    push_int(&stack, 2);
    D_TEST_EXPR(pop_int(&stack) == 2);
    D_TEST_EXPR(pop_int(&stack) == 1);
    expect_empty(&stack, true);
    push_int(&stack, 3);
    push_int(&stack, 4);
    D_TEST_EXPR(pop_int(&stack) == 4);
    D_TEST_EXPR(pop_int(&stack) == 3);
    expect_empty(&stack, true);
    d_stack_destroy(&stack);
}

static void test_interleaved_push_pop_preserves_lifo_order(void)
{
    DStack stack;
    make_int_stack(&stack, 2);
    push_int(&stack, 1);
    push_int(&stack, 2);
    D_TEST_EXPR(pop_int(&stack) == 2);
    push_int(&stack, 3);
    push_int(&stack, 4);
    D_TEST_EXPR(pop_int(&stack) == 4);
    D_TEST_EXPR(pop_int(&stack) == 3);
    D_TEST_EXPR(pop_int(&stack) == 1);
    expect_size(&stack, 0);
    d_stack_destroy(&stack);
}

static void test_stack_stores_pointer_values(void)
{
    DStack stack;
    char *a = owned_string("alpha");
    char *b = owned_string("beta");
    char *out = NULL;
    D_TEST_EXPR(d_stack_init(&stack, 1, sizeof(char *), NULL, NULL) == D_OK);
    D_TEST_EXPR(d_stack_push(&stack, &a) == D_OK);
    D_TEST_EXPR(d_stack_push(&stack, &b) == D_OK);
    D_TEST_EXPR(d_stack_pop(&stack, &out) == D_OK);
    D_TEST_EXPR(out == b);
    D_TEST_STR_EQ(out, "beta");
    free(out);
    out = NULL;
    D_TEST_EXPR(d_stack_pop(&stack, &out) == D_OK);
    D_TEST_EXPR(out == a);
    D_TEST_STR_EQ(out, "alpha");
    free(out);
    d_stack_destroy(&stack);
}

static void test_stack_can_store_null_pointer_value_when_wrapped_in_slot(void)
{
    DStack stack;
    char *ptr = NULL;
    char *out = (char *)0x1;
    D_TEST_EXPR(d_stack_init(&stack, 1, sizeof(char *), NULL, NULL) == D_OK);
    D_TEST_EXPR(d_stack_push(&stack, &ptr) == D_OK);
    expect_size(&stack, 1);
    D_TEST_EXPR(d_stack_pop(&stack, &out) == D_OK);
    D_TEST_NULL(out);
    expect_empty(&stack, true);
    d_stack_destroy(&stack);
}

static void test_large_struct_elements_are_copied_exactly(void)
{
    DStack stack;
    LargeStruct values[16];
    LargeStruct out;
    D_TEST_EXPR(d_stack_init(&stack, 1, sizeof(LargeStruct), NULL, NULL) == D_OK);
    for (int i = 0; i < 16; i++)
    {
        values[i] = make_large_struct(i);
        D_TEST_EXPR(d_stack_push(&stack, &values[i]) == D_OK);
    }
    memset(values, 0, sizeof(values));
    for (int i = 15; i >= 0; i--)
    {
        LargeStruct expected = make_large_struct(i);
        memset(&out, 0, sizeof(out));
        D_TEST_EXPR(d_stack_pop(&stack, &out) == D_OK);
        D_TEST_MEM_EQ(&out, &expected, sizeof(LargeStruct));
    }
    d_stack_destroy(&stack);
}

static void test_binary_data_with_zero_bytes_round_trips(void)
{
    DStack stack;
    unsigned char data[8] = {0, 1, 2, 0, 4, 5, 0, 7};
    unsigned char out[8] = {0};
    D_TEST_EXPR(d_stack_init(&stack, 1, sizeof(data), NULL, NULL) == D_OK);
    D_TEST_EXPR(d_stack_push(&stack, data) == D_OK);
    memset(data, 0xff, sizeof(data));
    D_TEST_EXPR(d_stack_pop(&stack, out) == D_OK);
    {
        unsigned char expected[8] = {0, 1, 2, 0, 4, 5, 0, 7};
        D_TEST_MEM_EQ(out, expected, sizeof(expected));
    }
    d_stack_destroy(&stack);
}

static void test_capacity_grows_from_zero_capacity(void)
{
    DStack stack;
    D_TEST_EXPR(d_stack_init(&stack, 0, sizeof(int), NULL, NULL) == D_OK);
    for (int i = 0; i < 32; i++)
        push_int(&stack, i);
    expect_size(&stack, 32);
    expect_capacity_at_least(&stack, 32);
    for (int i = 31; i >= 0; i--)
        D_TEST_EXPR(pop_int(&stack) == i);
    d_stack_destroy(&stack);
}

static void test_capacity_does_not_shrink_after_pops(void)
{
    DStack stack;
    make_int_stack(&stack, 1);
    usize capacity_before = 0, capacity_after = 0;
    for (int i = 0; i < 64; i++)
        push_int(&stack, i);
    capacity_before = d_stack_get_capacity(&stack);
    for (int i = 0; i < 64; i++)
        (void)pop_int(&stack);
    capacity_after = d_stack_get_capacity(&stack);
    D_TEST_EXPR(capacity_after == capacity_before);
    d_stack_destroy(&stack);
}

static void test_mutating_after_failed_pop_is_still_valid(void)
{
    DStack stack;
    make_int_stack(&stack, 1);
    int out = 0;
    D_TEST_EXPR(d_stack_pop(&stack, &out) != D_OK);
    push_int(&stack, 1234);
    D_TEST_EXPR(pop_int(&stack) == 1234);
    expect_empty(&stack, true);
    d_stack_destroy(&stack);
}

static void test_many_alternating_operations_leave_consistent_state(void)
{
    DStack stack;
    make_int_stack(&stack, 4);
    for (int round = 0; round < 100; round++)
    {
        push_int(&stack, round * 3 + 0);
        push_int(&stack, round * 3 + 1);
        D_TEST_EXPR(pop_int(&stack) == round * 3 + 1);
        push_int(&stack, round * 3 + 2);
        D_TEST_EXPR(pop_int(&stack) == round * 3 + 2);
        D_TEST_EXPR(pop_int(&stack) == round * 3 + 0);
        expect_size(&stack, 0);
        expect_empty(&stack, true);
    }
    d_stack_destroy(&stack);
}

static void test_multiple_stacks_are_independent(void)
{
    DStack a, b;
    make_int_stack(&a, 1);
    make_int_stack(&b, 1);
    push_int(&a, 1);
    push_int(&a, 2);
    push_int(&b, 10);
    push_int(&b, 20);
    D_TEST_EXPR(pop_int(&a) == 2);
    D_TEST_EXPR(pop_int(&b) == 20);
    push_int(&a, 3);
    D_TEST_EXPR(pop_int(&a) == 3);
    D_TEST_EXPR(pop_int(&a) == 1);
    D_TEST_EXPR(pop_int(&b) == 10);
    d_stack_destroy(&a);
    d_stack_destroy(&b);
}

/* --- extra edge-case tests --- */

static void test_push_exactly_at_pow2_boundary_then_one_more(void)
{
    DStack stack;
    make_int_stack(&stack, 8);
    for (int i = 0; i < 8; i++)
        push_int(&stack, i);
    push_int(&stack, 8);
    expect_size(&stack, 9);
    for (int i = 8; i >= 0; i--)
        D_TEST_EXPR(pop_int(&stack) == i);
    expect_empty(&stack, true);
    d_stack_destroy(&stack);
}

static void test_push_pop_alternating_many_times_keeps_lifo(void)
{
    DStack stack;
    make_int_stack(&stack, 2);
    for (int i = 0; i < 500; i++)
    {
        push_int(&stack, i);
        D_TEST_EXPR(pop_int(&stack) == i);
        expect_empty(&stack, true);
    }
    d_stack_destroy(&stack);
}

static void test_destroy_calls_destructor_for_each_remaining_element(void)
{
    DStack stack;
    int values[] = {10, 20, 30};

    reset_destroy_tracking();
    D_TEST_EXPR(d_stack_init(&stack, 0, sizeof(int), int_destroy_counter, NULL) == D_OK);
    for (int i = 0; i < 3; i++)
        push_int(&stack, values[i]);
    d_stack_destroy(&stack);
    D_TEST_EXPR(g_destroy_count == 3);
    D_TEST_EXPR(g_destroy_sum == 60);
}

static void test_copy_with_copy_fn_deep_copies_pointer_elements(void)
{
    DStack src, dst;
    char *s_hello = owned_string("hello");
    char *s_world = owned_string("world");
    char *src_top = NULL, *src_bot = NULL, *dst_top = NULL, *dst_bot = NULL;

    D_TEST_EXPR(d_stack_init(&src, 0, sizeof(char *), string_ptr_free, string_ptr_copy) == D_OK);
    D_TEST_EXPR(d_stack_init(&dst, 0, sizeof(char *), string_ptr_free, string_ptr_copy) == D_OK);
    D_TEST_EXPR(d_stack_push(&src, &s_hello) == D_OK);
    D_TEST_EXPR(d_stack_push(&src, &s_world) == D_OK);
    D_TEST_EXPR(d_stack_copy(&dst, &src) == D_OK);
    D_TEST_EXPR(d_stack_pop(&src, &src_top) == D_OK);  /* LIFO: "world" */
    D_TEST_EXPR(d_stack_pop(&src, &src_bot) == D_OK);  /* "hello" */
    D_TEST_EXPR(d_stack_pop(&dst, &dst_top) == D_OK);
    D_TEST_EXPR(d_stack_pop(&dst, &dst_bot) == D_OK);
    D_TEST_EXPR(strcmp(src_top, dst_top) == 0);
    D_TEST_EXPR(strcmp(src_bot, dst_bot) == 0);
    D_TEST_EXPR(src_top != dst_top);
    D_TEST_EXPR(src_bot != dst_bot);
    free(src_top); free(src_bot); free(dst_top); free(dst_bot);
    d_stack_destroy(&src);
    d_stack_destroy(&dst);
}

static void test_copy_without_copy_fn_pointer_array_is_shallow(void)
{
    DStack src, dst;
    char *s = owned_string("shallow");
    char *src_ptr = NULL, *dst_ptr = NULL;

    D_TEST_EXPR(d_stack_init(&src, 0, sizeof(char *), NULL, NULL) == D_OK);
    D_TEST_EXPR(d_stack_init(&dst, 0, sizeof(char *), NULL, NULL) == D_OK);
    D_TEST_EXPR(d_stack_push(&src, &s) == D_OK);
    D_TEST_EXPR(d_stack_copy(&dst, &src) == D_OK);
    D_TEST_EXPR(d_stack_pop(&src, &src_ptr) == D_OK);
    D_TEST_EXPR(d_stack_pop(&dst, &dst_ptr) == D_OK);
    D_TEST_EXPR(src_ptr == s);
    D_TEST_EXPR(dst_ptr == s);
    free(s);
    d_stack_destroy(&src);
    d_stack_destroy(&dst);
}

int main(void)
{
    DTest tests[] = {
        D_TEST_GENERATE_TEST(test_init_creates_empty_stack),
        D_TEST_GENERATE_TEST(test_init_with_capacity_sets_capacity),
        D_TEST_GENERATE_TEST(test_destroy_null_pointer_is_safe),
        D_TEST_GENERATE_TEST(test_destroy_empty_stack_is_safe),
        D_TEST_GENERATE_TEST(test_destroy_can_be_called_twice_safely),
        D_TEST_GENERATE_TEST(test_push_single_value_updates_size_and_empty_flag),
        D_TEST_GENERATE_TEST(test_push_copies_value_not_source_address),
        D_TEST_GENERATE_TEST(test_pop_returns_last_pushed_value),
        D_TEST_GENERATE_TEST(test_pop_many_values_are_lifo),
        D_TEST_GENERATE_TEST(test_pop_empty_stack_fails_and_keeps_output_unchanged),
        D_TEST_GENERATE_TEST(test_pop_until_empty_then_push_again),
        D_TEST_GENERATE_TEST(test_interleaved_push_pop_preserves_lifo_order),
        D_TEST_GENERATE_TEST(test_stack_stores_pointer_values),
        D_TEST_GENERATE_TEST(test_stack_can_store_null_pointer_value_when_wrapped_in_slot),
        D_TEST_GENERATE_TEST(test_large_struct_elements_are_copied_exactly),
        D_TEST_GENERATE_TEST(test_binary_data_with_zero_bytes_round_trips),
        D_TEST_GENERATE_TEST(test_capacity_grows_from_zero_capacity),
        D_TEST_GENERATE_TEST(test_capacity_does_not_shrink_after_pops),
        D_TEST_GENERATE_TEST(test_mutating_after_failed_pop_is_still_valid),
        D_TEST_GENERATE_TEST(test_many_alternating_operations_leave_consistent_state),
        D_TEST_GENERATE_TEST(test_multiple_stacks_are_independent),
        D_TEST_GENERATE_TEST(test_push_exactly_at_pow2_boundary_then_one_more),
        D_TEST_GENERATE_TEST(test_push_pop_alternating_many_times_keeps_lifo),
        D_TEST_GENERATE_TEST(test_destroy_calls_destructor_for_each_remaining_element),
        D_TEST_GENERATE_TEST(test_copy_with_copy_fn_deep_copies_pointer_elements),
        D_TEST_GENERATE_TEST(test_copy_without_copy_fn_pointer_array_is_shallow),
    };
    D_TEST_RUN_TESTS(tests);
    return 0;
}
