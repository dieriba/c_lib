#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "d_test.h"
#include "d_de_queue.h"
#include "d_types.h"



static DDeQueue *make_int_deque(usize capacity)
{
    DDeQueue *deque = NULL;

    D_TEST_EXPR(d_de_queue_new(&deque, capacity, sizeof(int)) == D_OK);
    D_TEST_NOT_NULL(deque);
    return deque;
}

static void expect_size(DDeQueue *deque, usize expected)
{
    usize size = (usize)-1;

    D_TEST_EXPR(d_de_queue_get_size(deque, &size) == D_OK);
    D_TEST_EXPR(size == expected);
}

static void expect_capacity_at_least(DDeQueue *deque, usize min_capacity)
{
    usize capacity = 0;

    D_TEST_EXPR(d_de_queue_get_capacity(deque, &capacity) == D_OK);
    D_TEST_EXPR(capacity >= min_capacity);
}

static void expect_empty(DDeQueue *deque, bool expected)
{
    bool is_empty = !expected;

    D_TEST_EXPR(d_de_queue_is_empty(deque, &is_empty) == D_OK);
    D_TEST_EXPR(is_empty == expected);
}

static void push_front_int(DDeQueue *deque, int value)
{
    D_TEST_EXPR(d_de_queue_push_front(deque, &value) == D_OK);
}

static void push_back_int(DDeQueue *deque, int value)
{
    D_TEST_EXPR(d_de_queue_push_back(deque, &value) == D_OK);
}

static int pop_front_int(DDeQueue *deque)
{
    int value = 0x7fffffff;

    D_TEST_EXPR(d_de_queue_pop_front(deque, &value) == D_OK);
    return value;
}

static int pop_back_int(DDeQueue *deque)
{
    int value = 0x7fffffff;

    D_TEST_EXPR(d_de_queue_pop_back(deque, &value) == D_OK);
    return value;
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
    LargeStruct value;

    memset(&value, 0, sizeof(value));
    value.a = 0x1234000000000000ULL + (uint64_t)seed;
    value.b = (uint32_t)(seed * 2654435761u);
    for (usize i = 0; i < sizeof(value.bytes); i++)
        value.bytes[i] = (char)('A' + ((seed + (int)i) % 26));
    value.tail = -seed;
    return value;
}

static void assert_large_struct_eq(const LargeStruct *actual, const LargeStruct *expected)
{
    D_TEST_EXPR(actual->a == expected->a);
    D_TEST_EXPR(actual->b == expected->b);
    D_TEST_MEM_EQ(actual->bytes, expected->bytes, sizeof(expected->bytes));
    D_TEST_EXPR(actual->tail == expected->tail);
}

static void test_new_creates_empty_deque_with_zero_capacity_request(void)
{
    DDeQueue *deque = NULL;

    D_TEST_EXPR(d_de_queue_new(&deque, 0, sizeof(int)) == D_OK);
    D_TEST_NOT_NULL(deque);
    expect_size(deque, 0);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_new_with_capacity_sets_capacity_and_empty_state(void)
{
    DDeQueue *deque = NULL;

    D_TEST_EXPR(d_de_queue_new(&deque, 8, sizeof(int)) == D_OK);
    D_TEST_NOT_NULL(deque);
    expect_capacity_at_least(deque, 8);
    expect_size(deque, 0);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_new_rejects_null_output_pointer(void)
{
    D_TEST_EXPR(d_de_queue_new(NULL, 4, sizeof(int)) == D_ERR_INVALID_ARG);
}

static void test_new_rejects_zero_elem_size_and_leaves_output_null(void)
{
    DDeQueue *deque = NULL;

    D_TEST_EXPR(d_de_queue_new(&deque, 4, 0) == D_ERR_INVALID_ARG);
    D_TEST_NULL(deque);
}

static void test_destroy_accepts_null_double_pointer(void)
{
    d_de_queue_destroy(NULL);
    D_TEST_EXPR(true);
}

static void test_destroy_accepts_pointer_to_null(void)
{
    DDeQueue *deque = NULL;

    d_de_queue_destroy(&deque);
    D_TEST_NULL(deque);
}

static void test_destroy_sets_pointer_to_null(void)
{
    DDeQueue *deque = make_int_deque(2);

    d_de_queue_destroy(&deque);
    D_TEST_NULL(deque);
}

static void test_push_front_single_value_updates_state(void)
{
    DDeQueue *deque = make_int_deque(1);

    push_front_int(deque, 42);
    expect_size(deque, 1);
    expect_empty(deque, false);
    D_TEST_EXPR(pop_front_int(deque) == 42);
    d_de_queue_destroy(&deque);
}

static void test_push_back_single_value_updates_state(void)
{
    DDeQueue *deque = make_int_deque(1);

    push_back_int(deque, 42);
    expect_size(deque, 1);
    expect_empty(deque, false);
    D_TEST_EXPR(pop_back_int(deque) == 42);
    d_de_queue_destroy(&deque);
}

static void test_push_front_then_pop_front_is_lifo_from_front_side(void)
{
    DDeQueue *deque = make_int_deque(2);

    push_front_int(deque, 1);
    push_front_int(deque, 2);
    push_front_int(deque, 3);
    expect_size(deque, 3);
    D_TEST_EXPR(pop_front_int(deque) == 3);
    D_TEST_EXPR(pop_front_int(deque) == 2);
    D_TEST_EXPR(pop_front_int(deque) == 1);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_push_back_then_pop_front_is_fifo(void)
{
    DDeQueue *deque = make_int_deque(2);

    push_back_int(deque, 1);
    push_back_int(deque, 2);
    push_back_int(deque, 3);
    D_TEST_EXPR(pop_front_int(deque) == 1);
    D_TEST_EXPR(pop_front_int(deque) == 2);
    D_TEST_EXPR(pop_front_int(deque) == 3);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_push_back_then_pop_back_is_lifo_from_back_side(void)
{
    DDeQueue *deque = make_int_deque(2);

    push_back_int(deque, 1);
    push_back_int(deque, 2);
    push_back_int(deque, 3);
    D_TEST_EXPR(pop_back_int(deque) == 3);
    D_TEST_EXPR(pop_back_int(deque) == 2);
    D_TEST_EXPR(pop_back_int(deque) == 1);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_push_front_then_pop_back_is_fifo_relative_to_front_pushes(void)
{
    DDeQueue *deque = make_int_deque(2);

    push_front_int(deque, 1);
    push_front_int(deque, 2);
    push_front_int(deque, 3);
    D_TEST_EXPR(pop_back_int(deque) == 1);
    D_TEST_EXPR(pop_back_int(deque) == 2);
    D_TEST_EXPR(pop_back_int(deque) == 3);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_mixed_pushes_preserve_expected_order_from_front(void)
{
    DDeQueue *deque = make_int_deque(2);

    push_back_int(deque, 10); /* [10] */
    push_front_int(deque, 5); /* [5, 10] */
    push_back_int(deque, 20); /* [5, 10, 20] */
    push_front_int(deque, 1); /* [1, 5, 10, 20] */

    D_TEST_EXPR(pop_front_int(deque) == 1);
    D_TEST_EXPR(pop_front_int(deque) == 5);
    D_TEST_EXPR(pop_front_int(deque) == 10);
    D_TEST_EXPR(pop_front_int(deque) == 20);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_mixed_pushes_preserve_expected_order_from_back(void)
{
    DDeQueue *deque = make_int_deque(2);

    push_back_int(deque, 10); /* [10] */
    push_front_int(deque, 5); /* [5, 10] */
    push_back_int(deque, 20); /* [5, 10, 20] */
    push_front_int(deque, 1); /* [1, 5, 10, 20] */

    D_TEST_EXPR(pop_back_int(deque) == 20);
    D_TEST_EXPR(pop_back_int(deque) == 10);
    D_TEST_EXPR(pop_back_int(deque) == 5);
    D_TEST_EXPR(pop_back_int(deque) == 1);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_push_rejects_null_deque(void)
{
    int value = 1;

    D_TEST_EXPR(d_de_queue_push_front(NULL, &value) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(d_de_queue_push_back(NULL, &value) == D_ERR_INVALID_ARG);
}

static void test_push_rejects_null_elem_and_does_not_mutate(void)
{
    DDeQueue *deque = make_int_deque(1);

    D_TEST_EXPR(d_de_queue_push_front(deque, NULL) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(d_de_queue_push_back(deque, NULL) == D_ERR_INVALID_ARG);
    expect_size(deque, 0);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_pop_rejects_null_deque_and_keeps_output(void)
{
    int out = 0x12345678;

    D_TEST_EXPR(d_de_queue_pop_front(NULL, &out) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(out == 0x12345678);
    D_TEST_EXPR(d_de_queue_pop_back(NULL, &out) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(out == 0x12345678);
}

static void test_pop_rejects_null_output_and_does_not_remove_front(void)
{
    DDeQueue *deque = make_int_deque(1);

    push_back_int(deque, 11);
    D_TEST_EXPR(d_de_queue_pop_front(deque, NULL) == D_ERR_INVALID_ARG);
    expect_size(deque, 1);
    D_TEST_EXPR(pop_front_int(deque) == 11);
    d_de_queue_destroy(&deque);
}

static void test_pop_rejects_null_output_and_does_not_remove_back(void)
{
    DDeQueue *deque = make_int_deque(1);

    push_back_int(deque, 11);
    D_TEST_EXPR(d_de_queue_pop_back(deque, NULL) == D_ERR_INVALID_ARG);
    expect_size(deque, 1);
    D_TEST_EXPR(pop_back_int(deque) == 11);
    d_de_queue_destroy(&deque);
}

static void test_pop_empty_front_fails_and_keeps_output_unchanged(void)
{
    DDeQueue *deque = make_int_deque(1);
    int out = 0x55555555;

    D_TEST_EXPR(d_de_queue_pop_front(deque, &out) != D_OK);
    D_TEST_EXPR(out == 0x55555555);
    expect_size(deque, 0);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_pop_empty_back_fails_and_keeps_output_unchanged(void)
{
    DDeQueue *deque = make_int_deque(1);
    int out = 0x55555555;

    D_TEST_EXPR(d_de_queue_pop_back(deque, &out) != D_OK);
    D_TEST_EXPR(out == 0x55555555);
    expect_size(deque, 0);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_getters_reject_null_deque(void)
{
    usize n = 0;
    bool b = false;

    D_TEST_EXPR(d_de_queue_get_size(NULL, &n) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(d_de_queue_get_capacity(NULL, &n) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(d_de_queue_is_empty(NULL, &b) == D_ERR_INVALID_ARG);
}

static void test_getters_reject_null_output_pointer(void)
{
    DDeQueue *deque = make_int_deque(1);

    D_TEST_EXPR(d_de_queue_get_size(deque, NULL) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(d_de_queue_get_capacity(deque, NULL) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(d_de_queue_is_empty(deque, NULL) == D_ERR_INVALID_ARG);
    d_de_queue_destroy(&deque);
}

static void test_values_are_copied_not_aliasing_source_stack_variable(void)
{
    DDeQueue *deque = make_int_deque(1);
    int value = 123;

    D_TEST_EXPR(d_de_queue_push_back(deque, &value) == D_OK);
    value = 999;
    D_TEST_EXPR(pop_front_int(deque) == 123);
    d_de_queue_destroy(&deque);
}

static void test_binary_payload_with_zero_bytes_round_trips(void)
{
    typedef struct BinaryPayload
    {
        unsigned char bytes[16];
    } BinaryPayload;

    DDeQueue *deque = NULL;
    BinaryPayload in;
    BinaryPayload out;

    for (usize i = 0; i < sizeof(in.bytes); i++)
        in.bytes[i] = (unsigned char)(i * 17u);
    memset(&out, 0xff, sizeof(out));

    D_TEST_EXPR(d_de_queue_new(&deque, 1, sizeof(BinaryPayload)) == D_OK);
    D_TEST_EXPR(d_de_queue_push_back(deque, &in) == D_OK);
    memset(&in, 0, sizeof(in));
    D_TEST_EXPR(d_de_queue_pop_front(deque, &out) == D_OK);

    for (usize i = 0; i < sizeof(out.bytes); i++)
        D_TEST_EXPR(out.bytes[i] == (unsigned char)(i * 17u));
    d_de_queue_destroy(&deque);
}

static void test_pointer_values_are_stored_as_values_not_deep_copied(void)
{
    DDeQueue *deque = NULL;
    int a = 10;
    int b = 20;
    int *pa = &a;
    int *pb = &b;
    int *out = NULL;

    D_TEST_EXPR(d_de_queue_new(&deque, 1, sizeof(int *)) == D_OK);
    D_TEST_EXPR(d_de_queue_push_back(deque, &pa) == D_OK);
    D_TEST_EXPR(d_de_queue_push_front(deque, &pb) == D_OK);

    D_TEST_EXPR(d_de_queue_pop_front(deque, &out) == D_OK);
    D_TEST_EXPR(out == &b);
    D_TEST_EXPR(*out == 20);
    D_TEST_EXPR(d_de_queue_pop_back(deque, &out) == D_OK);
    D_TEST_EXPR(out == &a);
    D_TEST_EXPR(*out == 10);
    d_de_queue_destroy(&deque);
}

static void test_large_struct_round_trips_from_front_without_padding_memcmp(void)
{
    DDeQueue *deque = NULL;
    LargeStruct in = make_large_struct(7);
    LargeStruct out;

    memset(&out, 0xcc, sizeof(out));
    D_TEST_EXPR(d_de_queue_new(&deque, 1, sizeof(LargeStruct)) == D_OK);
    D_TEST_EXPR(d_de_queue_push_front(deque, &in) == D_OK);
    memset(&in, 0, sizeof(in));
    in = make_large_struct(7);
    D_TEST_EXPR(d_de_queue_pop_front(deque, &out) == D_OK);
    assert_large_struct_eq(&out, &in);
    d_de_queue_destroy(&deque);
}

static void test_large_struct_round_trips_from_back_without_padding_memcmp(void)
{
    DDeQueue *deque = NULL;
    LargeStruct in = make_large_struct(99);
    LargeStruct expected = in;
    LargeStruct out;

    memset(&out, 0xcc, sizeof(out));
    D_TEST_EXPR(d_de_queue_new(&deque, 1, sizeof(LargeStruct)) == D_OK);
    D_TEST_EXPR(d_de_queue_push_back(deque, &in) == D_OK);
    memset(&in, 0, sizeof(in));
    D_TEST_EXPR(d_de_queue_pop_back(deque, &out) == D_OK);
    assert_large_struct_eq(&out, &expected);
    d_de_queue_destroy(&deque);
}

static void test_grows_from_tiny_capacity_with_back_pushes(void)
{
    DDeQueue *deque = make_int_deque(1);

    for (int i = 0; i < 512; i++)
        push_back_int(deque, i);
    expect_size(deque, 512);
    expect_capacity_at_least(deque, 512);
    for (int i = 0; i < 512; i++)
        D_TEST_EXPR(pop_front_int(deque) == i);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_grows_from_tiny_capacity_with_front_pushes(void)
{
    DDeQueue *deque = make_int_deque(1);

    for (int i = 0; i < 512; i++)
        push_front_int(deque, i);
    expect_size(deque, 512);
    expect_capacity_at_least(deque, 512);
    for (int i = 511; i >= 0; i--)
        D_TEST_EXPR(pop_front_int(deque) == i);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_wraparound_after_front_pops_then_back_pushes(void)
{
    DDeQueue *deque = make_int_deque(4);

    for (int i = 0; i < 4; i++)
        push_back_int(deque, i);
    D_TEST_EXPR(pop_front_int(deque) == 0);
    D_TEST_EXPR(pop_front_int(deque) == 1);
    push_back_int(deque, 4);
    push_back_int(deque, 5);

    for (int expected = 2; expected <= 5; expected++)
        D_TEST_EXPR(pop_front_int(deque) == expected);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_wraparound_after_back_pops_then_front_pushes(void)
{
    DDeQueue *deque = make_int_deque(4);

    for (int i = 0; i < 4; i++)
        push_front_int(deque, i); /* [3,2,1,0] */
    D_TEST_EXPR(pop_back_int(deque) == 0);
    D_TEST_EXPR(pop_back_int(deque) == 1);
    push_front_int(deque, 4); /* [4,3,2] */
    push_front_int(deque, 5); /* [5,4,3,2] */

    D_TEST_EXPR(pop_front_int(deque) == 5);
    D_TEST_EXPR(pop_front_int(deque) == 4);
    D_TEST_EXPR(pop_front_int(deque) == 3);
    D_TEST_EXPR(pop_front_int(deque) == 2);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_growth_while_wrapped_preserves_order_from_front(void)
{
    DDeQueue *deque = make_int_deque(4);

    for (int i = 0; i < 4; i++)
        push_back_int(deque, i); /* [0,1,2,3] */
    D_TEST_EXPR(pop_front_int(deque) == 0);
    D_TEST_EXPR(pop_front_int(deque) == 1); /* [2,3], head is wrapped after next pushes */
    push_back_int(deque, 4);
    push_back_int(deque, 5); /* [2,3,4,5], physically wrapped */
    push_back_int(deque, 6); /* must grow while wrapped */
    push_back_int(deque, 7);

    for (int expected = 2; expected <= 7; expected++)
        D_TEST_EXPR(pop_front_int(deque) == expected);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_growth_while_wrapped_preserves_order_from_back(void)
{
    DDeQueue *deque = make_int_deque(4);

    for (int i = 0; i < 4; i++)
        push_front_int(deque, i); /* [3,2,1,0] */
    D_TEST_EXPR(pop_back_int(deque) == 0);
    D_TEST_EXPR(pop_back_int(deque) == 1); /* [3,2] */
    push_front_int(deque, 4);
    push_front_int(deque, 5); /* [5,4,3,2], physically wrapped */
    push_front_int(deque, 6); /* must grow while wrapped */
    push_front_int(deque, 7);

    D_TEST_EXPR(pop_front_int(deque) == 7);
    D_TEST_EXPR(pop_front_int(deque) == 6);
    D_TEST_EXPR(pop_front_int(deque) == 5);
    D_TEST_EXPR(pop_front_int(deque) == 4);
    D_TEST_EXPR(pop_front_int(deque) == 3);
    D_TEST_EXPR(pop_front_int(deque) == 2);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_alternating_push_back_pop_front_keeps_reusable_empty_state(void)
{
    DDeQueue *deque = make_int_deque(2);

    for (int i = 0; i < 1000; i++)
    {
        push_back_int(deque, i);
        expect_size(deque, 1);
        D_TEST_EXPR(pop_front_int(deque) == i);
        expect_size(deque, 0);
        expect_empty(deque, true);
    }
    d_de_queue_destroy(&deque);
}

static void test_alternating_push_front_pop_back_keeps_reusable_empty_state(void)
{
    DDeQueue *deque = make_int_deque(2);

    for (int i = 0; i < 1000; i++)
    {
        push_front_int(deque, i);
        expect_size(deque, 1);
        D_TEST_EXPR(pop_back_int(deque) == i);
        expect_size(deque, 0);
        expect_empty(deque, true);
    }
    d_de_queue_destroy(&deque);
}

static void test_reuse_after_draining_from_front_then_back(void)
{
    DDeQueue *deque = make_int_deque(2);

    for (int i = 0; i < 10; i++)
        push_back_int(deque, i);
    for (int i = 0; i < 10; i++)
        D_TEST_EXPR(pop_front_int(deque) == i);
    expect_empty(deque, true);

    for (int i = 100; i < 110; i++)
        push_front_int(deque, i);
    for (int i = 100; i < 110; i++)
        D_TEST_EXPR(pop_back_int(deque) == i);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_failed_pop_does_not_break_later_pushes(void)
{
    DDeQueue *deque = make_int_deque(1);
    int out = -1;

    D_TEST_EXPR(d_de_queue_pop_front(deque, &out) != D_OK);
    D_TEST_EXPR(d_de_queue_pop_back(deque, &out) != D_OK);
    push_back_int(deque, 12);
    push_front_int(deque, 11);
    D_TEST_EXPR(pop_front_int(deque) == 11);
    D_TEST_EXPR(pop_front_int(deque) == 12);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_failed_null_push_does_not_break_later_pushes(void)
{
    DDeQueue *deque = make_int_deque(1);

    D_TEST_EXPR(d_de_queue_push_back(deque, NULL) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(d_de_queue_push_front(deque, NULL) == D_ERR_INVALID_ARG);
    push_back_int(deque, 1);
    push_front_int(deque, 0);
    D_TEST_EXPR(pop_front_int(deque) == 0);
    D_TEST_EXPR(pop_front_int(deque) == 1);
    d_de_queue_destroy(&deque);
}

static void test_pop_front_and_back_until_middle_value(void)
{
    DDeQueue *deque = make_int_deque(4);

    for (int i = 0; i < 9; i++)
        push_back_int(deque, i); /* [0..8] */

    D_TEST_EXPR(pop_front_int(deque) == 0);
    D_TEST_EXPR(pop_back_int(deque) == 8);
    D_TEST_EXPR(pop_front_int(deque) == 1);
    D_TEST_EXPR(pop_back_int(deque) == 7);
    D_TEST_EXPR(pop_front_int(deque) == 2);
    D_TEST_EXPR(pop_back_int(deque) == 6);
    D_TEST_EXPR(pop_front_int(deque) == 3);
    D_TEST_EXPR(pop_back_int(deque) == 5);
    D_TEST_EXPR(pop_front_int(deque) == 4);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_interleaved_pattern_known_order(void)
{
    DDeQueue *deque = make_int_deque(3);

    push_back_int(deque, 1);                /* [1] */
    push_back_int(deque, 2);                /* [1,2] */
    push_front_int(deque, 0);               /* [0,1,2] */
    D_TEST_EXPR(pop_back_int(deque) == 2);  /* [0,1] */
    push_back_int(deque, 3);                /* [0,1,3] */
    D_TEST_EXPR(pop_front_int(deque) == 0); /* [1,3] */
    push_front_int(deque, -1);              /* [-1,1,3] */
    push_back_int(deque, 4);                /* [-1,1,3,4] grow/wrap possible */

    D_TEST_EXPR(pop_front_int(deque) == -1);
    D_TEST_EXPR(pop_front_int(deque) == 1);
    D_TEST_EXPR(pop_front_int(deque) == 3);
    D_TEST_EXPR(pop_front_int(deque) == 4);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_many_mixed_operations_against_simple_model(void)
{
    DDeQueue *deque = make_int_deque(1);
    int model[4096];
    int start = 2048;
    int end = 2048;

    for (int step = 0; step < 1500; step++)
    {
        int value = step * 3 - 1000;

        if (step % 5 == 0)
        {
            push_front_int(deque, value);
            model[--start] = value;
        }
        else if (step % 5 == 1)
        {
            push_back_int(deque, value);
            model[end++] = value;
        }
        else if (step % 5 == 2 && start < end)
        {
            D_TEST_EXPR(pop_front_int(deque) == model[start++]);
        }
        else if (step % 5 == 3 && start < end)
        {
            D_TEST_EXPR(pop_back_int(deque) == model[--end]);
        }
        else
        {
            push_back_int(deque, value);
            model[end++] = value;
        }
        expect_size(deque, (usize)(end - start));
    }

    while (start < end)
        D_TEST_EXPR(pop_front_int(deque) == model[start++]);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_zero_capacity_queue_can_still_push_front_and_back(void)
{
    DDeQueue *deque = make_int_deque(0);

    push_front_int(deque, 2);
    push_back_int(deque, 3);
    push_front_int(deque, 1);
    D_TEST_EXPR(pop_front_int(deque) == 1);
    D_TEST_EXPR(pop_front_int(deque) == 2);
    D_TEST_EXPR(pop_front_int(deque) == 3);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_capacity_never_shrinks_after_pops(void)
{
    DDeQueue *deque = make_int_deque(1);
    usize capacity_after_growth = 0;
    usize capacity_after_drain = 0;

    for (int i = 0; i < 128; i++)
        push_back_int(deque, i);
    D_TEST_EXPR(d_de_queue_get_capacity(deque, &capacity_after_growth) == D_OK);
    D_TEST_EXPR(capacity_after_growth >= 128);
    for (int i = 0; i < 128; i++)
        D_TEST_EXPR(pop_front_int(deque) == i);
    D_TEST_EXPR(d_de_queue_get_capacity(deque, &capacity_after_drain) == D_OK);
    D_TEST_EXPR(capacity_after_drain == capacity_after_growth);
    d_de_queue_destroy(&deque);
}

static void test_capacity_stable_after_failed_operations(void)
{
    DDeQueue *deque = make_int_deque(4);
    usize before = 0;
    usize after = 0;
    int out = 0;

    D_TEST_EXPR(d_de_queue_get_capacity(deque, &before) == D_OK);
    D_TEST_EXPR(d_de_queue_push_front(deque, NULL) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(d_de_queue_push_back(deque, NULL) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(d_de_queue_pop_front(deque, &out) != D_OK);
    D_TEST_EXPR(d_de_queue_pop_back(deque, &out) != D_OK);
    D_TEST_EXPR(d_de_queue_get_capacity(deque, &after) == D_OK);
    D_TEST_EXPR(after == before);
    expect_size(deque, 0);
    d_de_queue_destroy(&deque);
}

static void test_single_remaining_element_can_be_popped_from_front_or_back(void)
{
    DDeQueue *deque = make_int_deque(2);

    push_back_int(deque, 1);
    push_back_int(deque, 2);
    D_TEST_EXPR(pop_front_int(deque) == 1);
    D_TEST_EXPR(pop_back_int(deque) == 2);
    expect_empty(deque, true);

    push_front_int(deque, 3);
    push_front_int(deque, 4);
    D_TEST_EXPR(pop_back_int(deque) == 3);
    D_TEST_EXPR(pop_front_int(deque) == 4);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_alternating_opposite_end_push_pop_known_values(void)
{
    DDeQueue *deque = make_int_deque(2);

    for (int i = 0; i < 300; i++)
    {
        push_front_int(deque, i);
        push_back_int(deque, -i);
        D_TEST_EXPR(pop_back_int(deque) == -i);
        D_TEST_EXPR(pop_front_int(deque) == i);
        expect_empty(deque, true);
    }
    d_de_queue_destroy(&deque);
}

static void test_push_back_pop_back_after_many_wraps(void)
{
    DDeQueue *deque = make_int_deque(8);

    for (int round = 0; round < 200; round++)
    {
        for (int i = 0; i < 8; i++)
            push_back_int(deque, round * 100 + i);
        for (int i = 7; i >= 0; i--)
            D_TEST_EXPR(pop_back_int(deque) == round * 100 + i);
        expect_empty(deque, true);
    }
    d_de_queue_destroy(&deque);
}

static void test_push_front_pop_front_after_many_wraps(void)
{
    DDeQueue *deque = make_int_deque(8);

    for (int round = 0; round < 200; round++)
    {
        for (int i = 0; i < 8; i++)
            push_front_int(deque, round * 100 + i);
        for (int i = 7; i >= 0; i--)
            D_TEST_EXPR(pop_front_int(deque) == round * 100 + i);
        expect_empty(deque, true);
    }
    d_de_queue_destroy(&deque);
}

static void test_front_and_back_pops_on_same_physical_slot(void)
{
    DDeQueue *deque = make_int_deque(1);

    push_back_int(deque, 77);
    D_TEST_EXPR(pop_front_int(deque) == 77);
    expect_empty(deque, true);

    push_front_int(deque, 88);
    D_TEST_EXPR(pop_back_int(deque) == 88);
    expect_empty(deque, true);
    d_de_queue_destroy(&deque);
}

static void test_multiple_data_types_char_values(void)
{
    DDeQueue *deque = NULL;
    char c;

    D_TEST_EXPR(d_de_queue_new(&deque, 2, sizeof(char)) == D_OK);
    c = 'a';
    D_TEST_EXPR(d_de_queue_push_back(deque, &c) == D_OK);
    c = '\0';
    D_TEST_EXPR(d_de_queue_push_back(deque, &c) == D_OK);
    c = 'z';
    D_TEST_EXPR(d_de_queue_push_front(deque, &c) == D_OK);

    c = '?';
    D_TEST_EXPR(d_de_queue_pop_front(deque, &c) == D_OK);
    D_TEST_EXPR(c == 'z');
    c = '?';
    D_TEST_EXPR(d_de_queue_pop_front(deque, &c) == D_OK);
    D_TEST_EXPR(c == 'a');
    c = '?';
    D_TEST_EXPR(d_de_queue_pop_front(deque, &c) == D_OK);
    D_TEST_EXPR(c == '\0');
    d_de_queue_destroy(&deque);
}

int main(void)
{
    DTest tests[] = {
        D_TEST_GENERATE_TEST(test_new_creates_empty_deque_with_zero_capacity_request),
        D_TEST_GENERATE_TEST(test_new_with_capacity_sets_capacity_and_empty_state),
        D_TEST_GENERATE_TEST(test_new_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_new_rejects_zero_elem_size_and_leaves_output_null),
        D_TEST_GENERATE_TEST(test_destroy_accepts_null_double_pointer),
        D_TEST_GENERATE_TEST(test_destroy_accepts_pointer_to_null),
        D_TEST_GENERATE_TEST(test_destroy_sets_pointer_to_null),
        D_TEST_GENERATE_TEST(test_push_front_single_value_updates_state),
        D_TEST_GENERATE_TEST(test_push_back_single_value_updates_state),
        D_TEST_GENERATE_TEST(test_push_front_then_pop_front_is_lifo_from_front_side),
        D_TEST_GENERATE_TEST(test_push_back_then_pop_front_is_fifo),
        D_TEST_GENERATE_TEST(test_push_back_then_pop_back_is_lifo_from_back_side),
        D_TEST_GENERATE_TEST(test_push_front_then_pop_back_is_fifo_relative_to_front_pushes),
        D_TEST_GENERATE_TEST(test_mixed_pushes_preserve_expected_order_from_front),
        D_TEST_GENERATE_TEST(test_mixed_pushes_preserve_expected_order_from_back),
        D_TEST_GENERATE_TEST(test_push_rejects_null_deque),
        D_TEST_GENERATE_TEST(test_push_rejects_null_elem_and_does_not_mutate),
        D_TEST_GENERATE_TEST(test_pop_rejects_null_deque_and_keeps_output),
        D_TEST_GENERATE_TEST(test_pop_rejects_null_output_and_does_not_remove_front),
        D_TEST_GENERATE_TEST(test_pop_rejects_null_output_and_does_not_remove_back),
        D_TEST_GENERATE_TEST(test_pop_empty_front_fails_and_keeps_output_unchanged),
        D_TEST_GENERATE_TEST(test_pop_empty_back_fails_and_keeps_output_unchanged),
        D_TEST_GENERATE_TEST(test_getters_reject_null_deque),
        D_TEST_GENERATE_TEST(test_getters_reject_null_output_pointer),
        D_TEST_GENERATE_TEST(test_values_are_copied_not_aliasing_source_stack_variable),
        D_TEST_GENERATE_TEST(test_binary_payload_with_zero_bytes_round_trips),
        D_TEST_GENERATE_TEST(test_pointer_values_are_stored_as_values_not_deep_copied),
        D_TEST_GENERATE_TEST(test_large_struct_round_trips_from_front_without_padding_memcmp),
        D_TEST_GENERATE_TEST(test_large_struct_round_trips_from_back_without_padding_memcmp),
        D_TEST_GENERATE_TEST(test_grows_from_tiny_capacity_with_back_pushes),
        D_TEST_GENERATE_TEST(test_grows_from_tiny_capacity_with_front_pushes),
        D_TEST_GENERATE_TEST(test_wraparound_after_front_pops_then_back_pushes),
        D_TEST_GENERATE_TEST(test_wraparound_after_back_pops_then_front_pushes),
        D_TEST_GENERATE_TEST(test_growth_while_wrapped_preserves_order_from_front),
        D_TEST_GENERATE_TEST(test_growth_while_wrapped_preserves_order_from_back),
        D_TEST_GENERATE_TEST(test_alternating_push_back_pop_front_keeps_reusable_empty_state),
        D_TEST_GENERATE_TEST(test_alternating_push_front_pop_back_keeps_reusable_empty_state),
        D_TEST_GENERATE_TEST(test_reuse_after_draining_from_front_then_back),
        D_TEST_GENERATE_TEST(test_failed_pop_does_not_break_later_pushes),
        D_TEST_GENERATE_TEST(test_failed_null_push_does_not_break_later_pushes),
        D_TEST_GENERATE_TEST(test_pop_front_and_back_until_middle_value),
        D_TEST_GENERATE_TEST(test_interleaved_pattern_known_order),
        D_TEST_GENERATE_TEST(test_many_mixed_operations_against_simple_model),
        D_TEST_GENERATE_TEST(test_zero_capacity_queue_can_still_push_front_and_back),
        D_TEST_GENERATE_TEST(test_capacity_never_shrinks_after_pops),
        D_TEST_GENERATE_TEST(test_capacity_stable_after_failed_operations),
        D_TEST_GENERATE_TEST(test_single_remaining_element_can_be_popped_from_front_or_back),
        D_TEST_GENERATE_TEST(test_alternating_opposite_end_push_pop_known_values),
        D_TEST_GENERATE_TEST(test_push_back_pop_back_after_many_wraps),
        D_TEST_GENERATE_TEST(test_push_front_pop_front_after_many_wraps),
        D_TEST_GENERATE_TEST(test_front_and_back_pops_on_same_physical_slot),
        D_TEST_GENERATE_TEST(test_multiple_data_types_char_values),
    };

    D_TEST_RUN_TESTS(tests);
    return 0;
}
