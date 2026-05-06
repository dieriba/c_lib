#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "d_test.h"
#include "d_stack.h"
#include "d_types.h"

static DStack *make_int_stack(usize capacity)
{
    DStack *stack = NULL;

    D_TEST_EXPR(d_stack_new(&stack, capacity, sizeof(int)) == D_OK);
    D_TEST_NOT_NULL(stack);
    return stack;
}

static void expect_size(DStack *stack, usize expected)
{
    usize size = (usize)-1;

    D_TEST_EXPR(d_stack_get_size(stack, &size) == D_OK);
    D_TEST_EXPR(size == expected);
}

static void expect_capacity_at_least(DStack *stack, usize expected_min)
{
    usize capacity = 0;

    D_TEST_EXPR(d_stack_get_capacity(stack, &capacity) == D_OK);
    D_TEST_EXPR(capacity >= expected_min);
}

static void expect_empty(DStack *stack, bool expected)
{
    bool is_empty = !expected;

    D_TEST_EXPR(d_stack_is_empty(stack, &is_empty) == D_OK);
    D_TEST_EXPR(is_empty == expected);
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

static void test_new_creates_empty_stack(void)
{
    DStack *stack = NULL;

    D_TEST_EXPR(d_stack_new(&stack, 0, sizeof(int)) == D_OK);
    D_TEST_NOT_NULL(stack);
    expect_size(stack, 0);
    expect_empty(stack, true);

    d_stack_destroy(&stack);
}

static void test_new_with_capacity_sets_capacity(void)
{
    DStack *stack = NULL;

    D_TEST_EXPR(d_stack_new(&stack, 8, sizeof(int)) == D_OK);
    expect_capacity_at_least(stack, 8);
    expect_size(stack, 0);

    d_stack_destroy(&stack);
}

static void test_new_rejects_null_output_pointer(void)
{
    D_TEST_EXPR(d_stack_new(NULL, 4, sizeof(int)) == D_ERR_INVALID_ARG);
}

static void test_new_rejects_zero_elem_size(void)
{
    DStack *stack = NULL;

    D_TEST_EXPR(d_stack_new(&stack, 4, 0) == D_ERR_INVALID_ARG);
    D_TEST_NULL(stack);
}

static void test_destroy_null_output_pointer_is_safe(void)
{
    d_stack_destroy(NULL);
}

static void test_destroy_null_stack_is_safe(void)
{
    DStack *stack = NULL;

    d_stack_destroy(&stack);
    D_TEST_NULL(stack);
}

static void test_destroy_sets_pointer_to_null(void)
{
    DStack *stack = make_int_stack(4);

    d_stack_destroy(&stack);
    D_TEST_NULL(stack);
}

static void test_push_single_value_updates_size_and_empty_flag(void)
{
    DStack *stack = make_int_stack(1);

    push_int(stack, 42);
    expect_size(stack, 1);
    expect_empty(stack, false);

    d_stack_destroy(&stack);
}

static void test_push_rejects_null_stack(void)
{
    int value = 1;

    D_TEST_EXPR(d_stack_push(NULL, &value) == D_ERR_INVALID_ARG);
}

static void test_push_rejects_null_elem(void)
{
    DStack *stack = make_int_stack(1);

    D_TEST_EXPR(d_stack_push(stack, NULL) == D_ERR_INVALID_ARG);
    expect_size(stack, 0);
    expect_empty(stack, true);

    d_stack_destroy(&stack);
}

static void test_push_copies_value_not_source_address(void)
{
    DStack *stack = make_int_stack(1);
    int value = 123;

    D_TEST_EXPR(d_stack_push(stack, &value) == D_OK);
    value = 999;
    D_TEST_EXPR(pop_int(stack) == 123);

    d_stack_destroy(&stack);
}

static void test_pop_returns_last_pushed_value(void)
{
    DStack *stack = make_int_stack(2);

    push_int(stack, 10);
    push_int(stack, 20);
    D_TEST_EXPR(pop_int(stack) == 20);
    D_TEST_EXPR(pop_int(stack) == 10);
    expect_size(stack, 0);
    expect_empty(stack, true);

    d_stack_destroy(&stack);
}

static void test_pop_many_values_are_lifo(void)
{
    DStack *stack = make_int_stack(1);

    for (int i = 0; i < 128; i++)
        push_int(stack, i);
    expect_size(stack, 128);
    expect_capacity_at_least(stack, 128);

    for (int i = 127; i >= 0; i--)
        D_TEST_EXPR(pop_int(stack) == i);
    expect_size(stack, 0);
    expect_empty(stack, true);

    d_stack_destroy(&stack);
}

static void test_pop_rejects_null_stack(void)
{
    int out = 0x12345678;

    D_TEST_EXPR(d_stack_pop(NULL, &out) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(out == 0x12345678);
}

static void test_pop_rejects_null_output_pointer(void)
{
    DStack *stack = make_int_stack(1);

    push_int(stack, 77);
    D_TEST_EXPR(d_stack_pop(stack, NULL) == D_ERR_INVALID_ARG);
    expect_size(stack, 1);
    D_TEST_EXPR(pop_int(stack) == 77);

    d_stack_destroy(&stack);
}

static void test_pop_empty_stack_fails_and_keeps_output_unchanged(void)
{
    DStack *stack = make_int_stack(1);
    int out = 0x12345678;

    D_TEST_EXPR(d_stack_pop(stack, &out) != D_OK);
    D_TEST_EXPR(out == 0x12345678);
    expect_size(stack, 0);
    expect_empty(stack, true);

    d_stack_destroy(&stack);
}

static void test_pop_until_empty_then_push_again(void)
{
    DStack *stack = make_int_stack(2);

    push_int(stack, 1);
    push_int(stack, 2);
    D_TEST_EXPR(pop_int(stack) == 2);
    D_TEST_EXPR(pop_int(stack) == 1);
    expect_empty(stack, true);

    push_int(stack, 3);
    push_int(stack, 4);
    D_TEST_EXPR(pop_int(stack) == 4);
    D_TEST_EXPR(pop_int(stack) == 3);
    expect_empty(stack, true);

    d_stack_destroy(&stack);
}

static void test_interleaved_push_pop_preserves_lifo_order(void)
{
    DStack *stack = make_int_stack(2);

    push_int(stack, 1);
    push_int(stack, 2);
    D_TEST_EXPR(pop_int(stack) == 2);
    push_int(stack, 3);
    push_int(stack, 4);
    D_TEST_EXPR(pop_int(stack) == 4);
    D_TEST_EXPR(pop_int(stack) == 3);
    D_TEST_EXPR(pop_int(stack) == 1);
    expect_size(stack, 0);

    d_stack_destroy(&stack);
}

static void test_size_rejects_null_stack(void)
{
    usize size = 111;

    D_TEST_EXPR(d_stack_get_size(NULL, &size) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(size == 111);
}

static void test_size_rejects_null_output_pointer(void)
{
    DStack *stack = make_int_stack(1);

    D_TEST_EXPR(d_stack_get_size(stack, NULL) == D_ERR_INVALID_ARG);

    d_stack_destroy(&stack);
}

static void test_capacity_rejects_null_stack(void)
{
    usize capacity = 111;

    D_TEST_EXPR(d_stack_get_capacity(NULL, &capacity) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(capacity == 111);
}

static void test_capacity_rejects_null_output_pointer(void)
{
    DStack *stack = make_int_stack(1);

    D_TEST_EXPR(d_stack_get_capacity(stack, NULL) == D_ERR_INVALID_ARG);

    d_stack_destroy(&stack);
}

static void test_is_empty_rejects_null_stack(void)
{
    bool is_empty = false;

    D_TEST_EXPR(d_stack_is_empty(NULL, &is_empty) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(is_empty == false);
}

static void test_is_empty_rejects_null_output_pointer(void)
{
    DStack *stack = make_int_stack(1);

    D_TEST_EXPR(d_stack_is_empty(stack, NULL) == D_ERR_INVALID_ARG);

    d_stack_destroy(&stack);
}

static void test_stack_stores_pointer_values(void)
{
    DStack *stack = NULL;
    char *a = owned_string("alpha");
    char *b = owned_string("beta");
    char *out = NULL;

    D_TEST_EXPR(d_stack_new(&stack, 1, sizeof(char *)) == D_OK);
    D_TEST_EXPR(d_stack_push(stack, &a) == D_OK);
    D_TEST_EXPR(d_stack_push(stack, &b) == D_OK);

    D_TEST_EXPR(d_stack_pop(stack, &out) == D_OK);
    D_TEST_EXPR(out == b);
    D_TEST_STR_EQ(out, "beta");
    free(out);

    out = NULL;
    D_TEST_EXPR(d_stack_pop(stack, &out) == D_OK);
    D_TEST_EXPR(out == a);
    D_TEST_STR_EQ(out, "alpha");
    free(out);

    d_stack_destroy(&stack);
}

static void test_stack_can_store_null_pointer_value_when_wrapped_in_slot(void)
{
    DStack *stack = NULL;
    char *ptr = NULL;
    char *out = (char *)0x1;

    D_TEST_EXPR(d_stack_new(&stack, 1, sizeof(char *)) == D_OK);
    D_TEST_EXPR(d_stack_push(stack, &ptr) == D_OK);
    expect_size(stack, 1);

    D_TEST_EXPR(d_stack_pop(stack, &out) == D_OK);
    D_TEST_NULL(out);
    expect_empty(stack, true);

    d_stack_destroy(&stack);
}

static void test_large_struct_elements_are_copied_exactly(void)
{
    DStack *stack = NULL;
    LargeStruct values[16];
    LargeStruct out;

    D_TEST_EXPR(d_stack_new(&stack, 1, sizeof(LargeStruct)) == D_OK);
    for (int i = 0; i < 16; i++)
    {
        values[i] = make_large_struct(i);
        D_TEST_EXPR(d_stack_push(stack, &values[i]) == D_OK);
    }

    memset(values, 0, sizeof(values));
    for (int i = 15; i >= 0; i--)
    {
        LargeStruct expected = make_large_struct(i);
        memset(&out, 0, sizeof(out));
        D_TEST_EXPR(d_stack_pop(stack, &out) == D_OK);
        D_TEST_MEM_EQ(&out, &expected, sizeof(LargeStruct));
    }

    d_stack_destroy(&stack);
}

static void test_binary_data_with_zero_bytes_round_trips(void)
{
    DStack *stack = NULL;
    unsigned char data[8] = {0, 1, 2, 0, 4, 5, 0, 7};
    unsigned char out[8] = {0};

    D_TEST_EXPR(d_stack_new(&stack, 1, sizeof(data)) == D_OK);
    D_TEST_EXPR(d_stack_push(stack, data) == D_OK);
    memset(data, 0xff, sizeof(data));

    D_TEST_EXPR(d_stack_pop(stack, out) == D_OK);
    {
        unsigned char expected[8] = {0, 1, 2, 0, 4, 5, 0, 7};
        D_TEST_MEM_EQ(out, expected, sizeof(expected));
    }

    d_stack_destroy(&stack);
}

static void test_capacity_grows_from_zero_capacity(void)
{
    DStack *stack = NULL;

    D_TEST_EXPR(d_stack_new(&stack, 0, sizeof(int)) == D_OK);
    for (int i = 0; i < 32; i++)
        push_int(stack, i);

    expect_size(stack, 32);
    expect_capacity_at_least(stack, 32);
    for (int i = 31; i >= 0; i--)
        D_TEST_EXPR(pop_int(stack) == i);

    d_stack_destroy(&stack);
}

static void test_capacity_does_not_shrink_after_pops(void)
{
    DStack *stack = make_int_stack(1);
    usize capacity_before = 0;
    usize capacity_after = 0;

    for (int i = 0; i < 64; i++)
        push_int(stack, i);
    D_TEST_EXPR(d_stack_get_capacity(stack, &capacity_before) == D_OK);
    for (int i = 0; i < 64; i++)
        (void)pop_int(stack);
    D_TEST_EXPR(d_stack_get_capacity(stack, &capacity_after) == D_OK);
    D_TEST_EXPR(capacity_after == capacity_before);

    d_stack_destroy(&stack);
}

static void test_mutating_after_failed_pop_is_still_valid(void)
{
    DStack *stack = make_int_stack(1);
    int out = 0;

    D_TEST_EXPR(d_stack_pop(stack, &out) != D_OK);
    push_int(stack, 1234);
    D_TEST_EXPR(pop_int(stack) == 1234);
    expect_empty(stack, true);

    d_stack_destroy(&stack);
}

static void test_mutating_after_failed_null_push_is_still_valid(void)
{
    DStack *stack = make_int_stack(1);

    D_TEST_EXPR(d_stack_push(stack, NULL) == D_ERR_INVALID_ARG);
    push_int(stack, 55);
    D_TEST_EXPR(pop_int(stack) == 55);
    expect_empty(stack, true);

    d_stack_destroy(&stack);
}

static void test_many_alternating_operations_leave_consistent_state(void)
{
    DStack *stack = make_int_stack(4);

    for (int round = 0; round < 100; round++)
    {
        push_int(stack, round * 3 + 0);
        push_int(stack, round * 3 + 1);
        D_TEST_EXPR(pop_int(stack) == round * 3 + 1);
        push_int(stack, round * 3 + 2);
        D_TEST_EXPR(pop_int(stack) == round * 3 + 2);
        D_TEST_EXPR(pop_int(stack) == round * 3 + 0);
        expect_size(stack, 0);
        expect_empty(stack, true);
    }

    d_stack_destroy(&stack);
}

static void test_multiple_stacks_are_independent(void)
{
    DStack *a = make_int_stack(1);
    DStack *b = make_int_stack(1);

    push_int(a, 1);
    push_int(a, 2);
    push_int(b, 10);
    push_int(b, 20);

    D_TEST_EXPR(pop_int(a) == 2);
    D_TEST_EXPR(pop_int(b) == 20);
    push_int(a, 3);
    D_TEST_EXPR(pop_int(a) == 3);
    D_TEST_EXPR(pop_int(a) == 1);
    D_TEST_EXPR(pop_int(b) == 10);

    d_stack_destroy(&a);
    d_stack_destroy(&b);
}

int main(void)
{
    DTest tests[] = {
        D_TEST_GENERATE_TEST(test_new_creates_empty_stack),
        D_TEST_GENERATE_TEST(test_new_with_capacity_sets_capacity),
        D_TEST_GENERATE_TEST(test_new_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_new_rejects_zero_elem_size),
        D_TEST_GENERATE_TEST(test_destroy_null_output_pointer_is_safe),
        D_TEST_GENERATE_TEST(test_destroy_null_stack_is_safe),
        D_TEST_GENERATE_TEST(test_destroy_sets_pointer_to_null),
        D_TEST_GENERATE_TEST(test_push_single_value_updates_size_and_empty_flag),
        D_TEST_GENERATE_TEST(test_push_rejects_null_stack),
        D_TEST_GENERATE_TEST(test_push_rejects_null_elem),
        D_TEST_GENERATE_TEST(test_push_copies_value_not_source_address),
        D_TEST_GENERATE_TEST(test_pop_returns_last_pushed_value),
        D_TEST_GENERATE_TEST(test_pop_many_values_are_lifo),
        D_TEST_GENERATE_TEST(test_pop_rejects_null_stack),
        D_TEST_GENERATE_TEST(test_pop_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_pop_empty_stack_fails_and_keeps_output_unchanged),
        D_TEST_GENERATE_TEST(test_pop_until_empty_then_push_again),
        D_TEST_GENERATE_TEST(test_interleaved_push_pop_preserves_lifo_order),
        D_TEST_GENERATE_TEST(test_size_rejects_null_stack),
        D_TEST_GENERATE_TEST(test_size_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_capacity_rejects_null_stack),
        D_TEST_GENERATE_TEST(test_capacity_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_is_empty_rejects_null_stack),
        D_TEST_GENERATE_TEST(test_is_empty_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_stack_stores_pointer_values),
        D_TEST_GENERATE_TEST(test_stack_can_store_null_pointer_value_when_wrapped_in_slot),
        D_TEST_GENERATE_TEST(test_large_struct_elements_are_copied_exactly),
        D_TEST_GENERATE_TEST(test_binary_data_with_zero_bytes_round_trips),
        D_TEST_GENERATE_TEST(test_capacity_grows_from_zero_capacity),
        D_TEST_GENERATE_TEST(test_capacity_does_not_shrink_after_pops),
        D_TEST_GENERATE_TEST(test_mutating_after_failed_pop_is_still_valid),
        D_TEST_GENERATE_TEST(test_mutating_after_failed_null_push_is_still_valid),
        D_TEST_GENERATE_TEST(test_many_alternating_operations_leave_consistent_state),
        D_TEST_GENERATE_TEST(test_multiple_stacks_are_independent),
    };

    D_TEST_RUN_TESTS(tests);
    return 0;
}
