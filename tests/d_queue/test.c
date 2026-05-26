#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "d_test.h"
#include "d_queue.h"
#include "d_types.h"

static void make_int_queue(DQueue *queue, usize capacity)
{
    D_TEST_EXPR(d_queue_init(queue, capacity, sizeof(int), NULL, NULL) == D_OK);
}

static void expect_size(DQueue *queue, usize expected)
{
    usize size = (usize)-1;
    D_TEST_EXPR(d_queue_get_size(queue, &size) == D_OK);
    D_TEST_EXPR(size == expected);
}

static void expect_capacity_at_least(DQueue *queue, usize expected_min)
{
    usize capacity = 0;
    D_TEST_EXPR(d_queue_get_capacity(queue, &capacity) == D_OK);
    D_TEST_EXPR(capacity >= expected_min);
}

static void expect_empty(DQueue *queue, bool expected)
{
    bool is_empty = !expected;
    D_TEST_EXPR(d_queue_is_empty(queue, &is_empty) == D_OK);
    D_TEST_EXPR(is_empty == expected);
}

static void push_int(DQueue *queue, int value)
{
    D_TEST_EXPR(d_queue_push(queue, &value) == D_OK);
}

static int pop_int(DQueue *queue)
{
    int value = 0x7fffffff;
    D_TEST_EXPR(d_queue_pop(queue, &value) == D_OK);
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
    value.a = 0xfeed000000000000ULL + (uint64_t)seed;
    value.b = (uint32_t)(seed * 31);
    for (usize i = 0; i < sizeof(value.bytes); i++)
        value.bytes[i] = (char)('a' + ((seed + (int)i) % 26));
    value.tail = -seed;
    return value;
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

static void assert_large_struct_eq(const LargeStruct *actual, const LargeStruct *expected)
{
    D_TEST_EXPR(actual->a == expected->a);
    D_TEST_EXPR(actual->b == expected->b);
    D_TEST_MEM_EQ(actual->bytes, expected->bytes, sizeof(expected->bytes));
    D_TEST_EXPR(actual->tail == expected->tail);
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

static void test_init_creates_empty_queue(void)
{
    DQueue queue;
    D_TEST_EXPR(d_queue_init(&queue, 0, sizeof(int), NULL, NULL) == D_OK);
    expect_size(&queue, 0);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_init_with_capacity_sets_capacity(void)
{
    DQueue queue;
    D_TEST_EXPR(d_queue_init(&queue, 8, sizeof(int), NULL, NULL) == D_OK);
    expect_capacity_at_least(&queue, 8);
    expect_size(&queue, 0);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_push_single_value_updates_size_and_empty_flag(void)
{
    DQueue queue;
    make_int_queue(&queue, 1);
    push_int(&queue, 42);
    expect_size(&queue, 1);
    expect_empty(&queue, false);
    d_queue_destroy(&queue);
}

static void test_push_copies_value_not_source_address(void)
{
    DQueue queue;
    make_int_queue(&queue, 1);
    int value = 123;
    D_TEST_EXPR(d_queue_push(&queue, &value) == D_OK);
    value = 999;
    D_TEST_EXPR(pop_int(&queue) == 123);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_pop_returns_first_pushed_value(void)
{
    DQueue queue;
    make_int_queue(&queue, 2);
    push_int(&queue, 10);
    push_int(&queue, 20);
    D_TEST_EXPR(pop_int(&queue) == 10);
    D_TEST_EXPR(pop_int(&queue) == 20);
    expect_size(&queue, 0);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_pop_many_values_are_fifo(void)
{
    DQueue queue;
    make_int_queue(&queue, 1);
    for (int i = 0; i < 256; i++)
        push_int(&queue, i);
    expect_size(&queue, 256);
    expect_capacity_at_least(&queue, 256);
    for (int i = 0; i < 256; i++)
        D_TEST_EXPR(pop_int(&queue) == i);
    expect_size(&queue, 0);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_pop_empty_queue_fails_and_keeps_output_unchanged(void)
{
    DQueue queue;
    make_int_queue(&queue, 1);
    int out = 0x55555555;
    D_TEST_EXPR(d_queue_pop(&queue, &out) != D_OK);
    D_TEST_EXPR(out == 0x55555555);
    expect_size(&queue, 0);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_pop_until_empty_then_push_again(void)
{
    DQueue queue;
    make_int_queue(&queue, 2);
    push_int(&queue, 1);
    push_int(&queue, 2);
    D_TEST_EXPR(pop_int(&queue) == 1);
    D_TEST_EXPR(pop_int(&queue) == 2);
    expect_empty(&queue, true);
    push_int(&queue, 3);
    push_int(&queue, 4);
    D_TEST_EXPR(pop_int(&queue) == 3);
    D_TEST_EXPR(pop_int(&queue) == 4);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_interleaved_push_pop_preserves_fifo_order(void)
{
    DQueue queue;
    make_int_queue(&queue, 3);
    push_int(&queue, 1);
    push_int(&queue, 2);
    D_TEST_EXPR(pop_int(&queue) == 1);
    push_int(&queue, 3);
    push_int(&queue, 4);
    D_TEST_EXPR(pop_int(&queue) == 2);
    D_TEST_EXPR(pop_int(&queue) == 3);
    push_int(&queue, 5);
    D_TEST_EXPR(pop_int(&queue) == 4);
    D_TEST_EXPR(pop_int(&queue) == 5);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_wraparound_without_growth_preserves_order(void)
{
    DQueue queue;
    make_int_queue(&queue, 4);
    push_int(&queue, 1);
    push_int(&queue, 2);
    push_int(&queue, 3);
    D_TEST_EXPR(pop_int(&queue) == 1);
    D_TEST_EXPR(pop_int(&queue) == 2);
    push_int(&queue, 4);
    push_int(&queue, 5);
    push_int(&queue, 6);
    D_TEST_EXPR(pop_int(&queue) == 3);
    D_TEST_EXPR(pop_int(&queue) == 4);
    D_TEST_EXPR(pop_int(&queue) == 5);
    D_TEST_EXPR(pop_int(&queue) == 6);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_growth_while_wrapped_preserves_order(void)
{
    DQueue queue;
    make_int_queue(&queue, 4);
    for (int i = 0; i < 4; i++)
        push_int(&queue, i);
    D_TEST_EXPR(pop_int(&queue) == 0);
    D_TEST_EXPR(pop_int(&queue) == 1);
    for (int i = 4; i < 32; i++)
        push_int(&queue, i);
    expect_size(&queue, 30);
    expect_capacity_at_least(&queue, 32);
    for (int i = 2; i < 32; i++)
        D_TEST_EXPR(pop_int(&queue) == i);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_capacity_grows_from_zero_capacity(void)
{
    DQueue queue;
    make_int_queue(&queue, 0);
    push_int(&queue, 7);
    expect_size(&queue, 1);
    expect_capacity_at_least(&queue, 1);
    D_TEST_EXPR(pop_int(&queue) == 7);
    d_queue_destroy(&queue);
}

static void test_capacity_does_not_shrink_after_pops(void)
{
    DQueue queue;
    make_int_queue(&queue, 1);
    usize before = 0, after = 0;
    for (int i = 0; i < 64; i++)
        push_int(&queue, i);
    D_TEST_EXPR(d_queue_get_capacity(&queue, &before) == D_OK);
    for (int i = 0; i < 64; i++)
        D_TEST_EXPR(pop_int(&queue) == i);
    D_TEST_EXPR(d_queue_get_capacity(&queue, &after) == D_OK);
    D_TEST_EXPR(after == before);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_queue_stores_pointer_values_fifo(void)
{
    DQueue queue;
    char a[] = "alpha";
    char b[] = "beta";
    char *pa = a, *pb = b, *out = NULL;
    D_TEST_EXPR(d_queue_init(&queue, 1, sizeof(char *), NULL, NULL) == D_OK);
    D_TEST_EXPR(d_queue_push(&queue, &pa) == D_OK);
    D_TEST_EXPR(d_queue_push(&queue, &pb) == D_OK);
    D_TEST_EXPR(d_queue_pop(&queue, &out) == D_OK);
    D_TEST_EXPR(out == pa);
    D_TEST_STR_EQ(out, "alpha");
    D_TEST_EXPR(d_queue_pop(&queue, &out) == D_OK);
    D_TEST_EXPR(out == pb);
    D_TEST_STR_EQ(out, "beta");
    d_queue_destroy(&queue);
}

static void test_queue_can_store_null_pointer_value_when_wrapped_in_slot(void)
{
    DQueue queue;
    char *ptr = NULL;
    char *out = (char *)0x1;
    D_TEST_EXPR(d_queue_init(&queue, 1, sizeof(char *), NULL, NULL) == D_OK);
    D_TEST_EXPR(d_queue_push(&queue, &ptr) == D_OK);
    expect_size(&queue, 1);
    D_TEST_EXPR(d_queue_pop(&queue, &out) == D_OK);
    D_TEST_NULL(out);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_large_struct_elements_are_copied_and_fifo(void)
{
    DQueue queue;
    LargeStruct values[16];
    D_TEST_EXPR(d_queue_init(&queue, 1, sizeof(LargeStruct), NULL, NULL) == D_OK);
    for (int i = 0; i < 16; i++)
    {
        values[i] = make_large_struct(i);
        D_TEST_EXPR(d_queue_push(&queue, &values[i]) == D_OK);
    }
    memset(values[3].bytes, 'X', sizeof(values[3].bytes));
    values[3].tail = 9999;
    for (int i = 0; i < 16; i++)
    {
        LargeStruct expected = make_large_struct(i);
        LargeStruct out;
        memset(&out, 0, sizeof(out));
        D_TEST_EXPR(d_queue_pop(&queue, &out) == D_OK);
        assert_large_struct_eq(&out, &expected);
    }
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_binary_data_with_zero_bytes_round_trips(void)
{
    DQueue queue;
    unsigned char data[8] = {0, 1, 2, 0, 4, 255, 0, 8};
    unsigned char out[8];
    memset(out, 0xaa, sizeof(out));
    D_TEST_EXPR(d_queue_init(&queue, 1, sizeof(data), NULL, NULL) == D_OK);
    D_TEST_EXPR(d_queue_push(&queue, data) == D_OK);
    memset(data, 0xbb, sizeof(data));
    D_TEST_EXPR(d_queue_pop(&queue, out) == D_OK);
    D_TEST_EXPR(out[0] == 0 && out[1] == 1 && out[2] == 2 && out[3] == 0);
    D_TEST_EXPR(out[4] == 4 && out[5] == 255 && out[6] == 0 && out[7] == 8);
    d_queue_destroy(&queue);
}

static void test_mutating_after_failed_pop_is_still_valid(void)
{
    DQueue queue;
    make_int_queue(&queue, 1);
    int out = 111;
    D_TEST_EXPR(d_queue_pop(&queue, &out) != D_OK);
    D_TEST_EXPR(out == 111);
    push_int(&queue, 42);
    D_TEST_EXPR(pop_int(&queue) == 42);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_many_alternating_operations_leave_consistent_state(void)
{
    DQueue queue;
    make_int_queue(&queue, 2);
    int expected_front = 0, next_value = 0;
    for (int round = 0; round < 100; round++)
    {
        push_int(&queue, next_value++);
        push_int(&queue, next_value++);
        D_TEST_EXPR(pop_int(&queue) == expected_front++);
        push_int(&queue, next_value++);
        D_TEST_EXPR(pop_int(&queue) == expected_front++);
    }
    while (expected_front < next_value)
        D_TEST_EXPR(pop_int(&queue) == expected_front++);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_multiple_queues_are_independent(void)
{
    DQueue a, b;
    make_int_queue(&a, 1);
    make_int_queue(&b, 1);
    push_int(&a, 1);
    push_int(&a, 2);
    push_int(&b, 10);
    push_int(&b, 20);
    D_TEST_EXPR(pop_int(&a) == 1);
    D_TEST_EXPR(pop_int(&b) == 10);
    push_int(&a, 3);
    D_TEST_EXPR(pop_int(&a) == 2);
    D_TEST_EXPR(pop_int(&a) == 3);
    D_TEST_EXPR(pop_int(&b) == 20);
    expect_empty(&a, true);
    expect_empty(&b, true);
    d_queue_destroy(&a);
    d_queue_destroy(&b);
}

static void test_fifo_order_after_repeated_full_empty_cycles(void)
{
    DQueue queue;
    make_int_queue(&queue, 3);
    for (int cycle = 0; cycle < 50; cycle++)
    {
        push_int(&queue, cycle * 3 + 0);
        push_int(&queue, cycle * 3 + 1);
        push_int(&queue, cycle * 3 + 2);
        D_TEST_EXPR(pop_int(&queue) == cycle * 3 + 0);
        D_TEST_EXPR(pop_int(&queue) == cycle * 3 + 1);
        D_TEST_EXPR(pop_int(&queue) == cycle * 3 + 2);
        expect_empty(&queue, true);
    }
    d_queue_destroy(&queue);
}

static void test_size_tracks_interleaved_operations_exactly(void)
{
    DQueue queue;
    make_int_queue(&queue, 2);
    expect_size(&queue, 0);
    push_int(&queue, 1); expect_size(&queue, 1);
    push_int(&queue, 2); expect_size(&queue, 2);
    D_TEST_EXPR(pop_int(&queue) == 1); expect_size(&queue, 1);
    push_int(&queue, 3); expect_size(&queue, 2);
    D_TEST_EXPR(pop_int(&queue) == 2); expect_size(&queue, 1);
    D_TEST_EXPR(pop_int(&queue) == 3); expect_size(&queue, 0);
    d_queue_destroy(&queue);
}

static void test_destroy_null_pointer_is_safe(void)
{
    d_queue_destroy(NULL);
}

static void test_destroy_empty_queue_is_safe(void)
{
    DQueue queue;
    make_int_queue(&queue, 4);
    d_queue_destroy(&queue);
}

static void test_destroy_non_empty_queue_is_safe(void)
{
    DQueue queue;
    make_int_queue(&queue, 2);
    push_int(&queue, 10);
    push_int(&queue, 20);
    d_queue_destroy(&queue);
}

static void test_destroy_after_wraparound_is_safe(void)
{
    DQueue queue;
    make_int_queue(&queue, 4);
    push_int(&queue, 1);
    push_int(&queue, 2);
    push_int(&queue, 3);
    D_TEST_EXPR(pop_int(&queue) == 1);
    D_TEST_EXPR(pop_int(&queue) == 2);
    push_int(&queue, 4);
    push_int(&queue, 5);
    push_int(&queue, 6);
    d_queue_destroy(&queue);
}

static void test_destroy_after_growth_is_safe(void)
{
    DQueue queue;
    make_int_queue(&queue, 1);
    for (int i = 0; i < 128; i++)
        push_int(&queue, i);
    d_queue_destroy(&queue);
}

static void test_destroy_can_be_called_twice_safely(void)
{
    DQueue queue;
    make_int_queue(&queue, 1);
    push_int(&queue, 42);
    d_queue_destroy(&queue);
    d_queue_destroy(&queue);
}

static void test_destroy_pointer_queue_does_not_free_pointed_values(void)
{
    DQueue queue;
    char *owned = malloc(6);
    D_TEST_NOT_NULL(owned);
    memcpy(owned, "hello", 6);
    D_TEST_EXPR(d_queue_init(&queue, 1, sizeof(char *), NULL, NULL) == D_OK);
    D_TEST_EXPR(d_queue_push(&queue, &owned) == D_OK);
    d_queue_destroy(&queue);
    D_TEST_STR_EQ(owned, "hello");
    free(owned);
}

/* --- extra edge-case tests --- */

static void test_push_exactly_at_pow2_boundary_then_one_more(void)
{
    DQueue queue;
    make_int_queue(&queue, 8);
    for (int i = 0; i < 8; i++)
        push_int(&queue, i);
    push_int(&queue, 8);
    expect_size(&queue, 9);
    for (int i = 0; i < 9; i++)
        D_TEST_EXPR(pop_int(&queue) == i);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_head_equals_tail_after_half_drain_then_refill(void)
{
    DQueue queue;
    make_int_queue(&queue, 4);
    for (int i = 0; i < 4; i++) push_int(&queue, i * 10);
    for (int i = 0; i < 4; i++) D_TEST_EXPR(pop_int(&queue) == i * 10);
    for (int i = 0; i < 4; i++) push_int(&queue, i * 100);
    for (int i = 0; i < 4; i++) D_TEST_EXPR(pop_int(&queue) == i * 100);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_growth_triggered_at_exact_capacity_while_wrapped(void)
{
    DQueue queue;
    make_int_queue(&queue, 4);
    for (int i = 0; i < 4; i++) push_int(&queue, i);
    D_TEST_EXPR(pop_int(&queue) == 0);
    D_TEST_EXPR(pop_int(&queue) == 1);
    push_int(&queue, 4);
    push_int(&queue, 5);
    /* head > 0, tail wrapped — queue is now physically: [4,5,2,3] */
    /* push one more to trigger growth */
    push_int(&queue, 6);
    expect_size(&queue, 5);
    D_TEST_EXPR(pop_int(&queue) == 2);
    D_TEST_EXPR(pop_int(&queue) == 3);
    D_TEST_EXPR(pop_int(&queue) == 4);
    D_TEST_EXPR(pop_int(&queue) == 5);
    D_TEST_EXPR(pop_int(&queue) == 6);
    expect_empty(&queue, true);
    d_queue_destroy(&queue);
}

static void test_destroy_calls_destructor_for_each_remaining_element(void)
{
    DQueue queue;
    int values[] = {7, 8, 9};

    reset_destroy_tracking();
    D_TEST_EXPR(d_queue_init(&queue, 0, sizeof(int), int_destroy_counter, NULL) == D_OK);
    for (int i = 0; i < 3; i++)
        push_int(&queue, values[i]);
    d_queue_destroy(&queue);
    D_TEST_EXPR(g_destroy_count == 3);
    D_TEST_EXPR(g_destroy_sum == 24);
}

static void test_copy_with_copy_fn_deep_copies_pointer_elements(void)
{
    DQueue src, dst;
    char *s_hello = owned_string("hello");
    char *s_world = owned_string("world");
    char *src_first = NULL, *src_second = NULL, *dst_first = NULL, *dst_second = NULL;

    D_TEST_EXPR(d_queue_init(&src, 0, sizeof(char *), string_ptr_free, string_ptr_copy) == D_OK);
    D_TEST_EXPR(d_queue_init(&dst, 0, sizeof(char *), string_ptr_free, string_ptr_copy) == D_OK);
    D_TEST_EXPR(d_queue_push(&src, &s_hello) == D_OK);
    D_TEST_EXPR(d_queue_push(&src, &s_world) == D_OK);
    D_TEST_EXPR(d_queue_copy(&dst, &src) == D_OK);
    D_TEST_EXPR(d_queue_pop(&src, &src_first) == D_OK);   /* FIFO: "hello" */
    D_TEST_EXPR(d_queue_pop(&src, &src_second) == D_OK);  /* "world" */
    D_TEST_EXPR(d_queue_pop(&dst, &dst_first) == D_OK);
    D_TEST_EXPR(d_queue_pop(&dst, &dst_second) == D_OK);
    D_TEST_EXPR(strcmp(src_first, dst_first) == 0);
    D_TEST_EXPR(strcmp(src_second, dst_second) == 0);
    D_TEST_EXPR(src_first != dst_first);
    D_TEST_EXPR(src_second != dst_second);
    free(src_first); free(src_second); free(dst_first); free(dst_second);
    d_queue_destroy(&src);
    d_queue_destroy(&dst);
}

static void test_copy_without_copy_fn_pointer_array_is_shallow(void)
{
    DQueue src, dst;
    char *s = owned_string("shallow");
    char *src_ptr = NULL, *dst_ptr = NULL;

    D_TEST_EXPR(d_queue_init(&src, 0, sizeof(char *), NULL, NULL) == D_OK);
    D_TEST_EXPR(d_queue_init(&dst, 0, sizeof(char *), NULL, NULL) == D_OK);
    D_TEST_EXPR(d_queue_push(&src, &s) == D_OK);
    D_TEST_EXPR(d_queue_copy(&dst, &src) == D_OK);
    D_TEST_EXPR(d_queue_pop(&src, &src_ptr) == D_OK);
    D_TEST_EXPR(d_queue_pop(&dst, &dst_ptr) == D_OK);
    D_TEST_EXPR(src_ptr == s);
    D_TEST_EXPR(dst_ptr == s);
    free(s);
    d_queue_destroy(&src);
    d_queue_destroy(&dst);
}

int main(void)
{
    DTest tests[] = {
        D_TEST_GENERATE_TEST(test_init_creates_empty_queue),
        D_TEST_GENERATE_TEST(test_init_with_capacity_sets_capacity),
        D_TEST_GENERATE_TEST(test_destroy_null_pointer_is_safe),
        D_TEST_GENERATE_TEST(test_destroy_empty_queue_is_safe),
        D_TEST_GENERATE_TEST(test_destroy_non_empty_queue_is_safe),
        D_TEST_GENERATE_TEST(test_destroy_after_wraparound_is_safe),
        D_TEST_GENERATE_TEST(test_destroy_after_growth_is_safe),
        D_TEST_GENERATE_TEST(test_destroy_can_be_called_twice_safely),
        D_TEST_GENERATE_TEST(test_destroy_pointer_queue_does_not_free_pointed_values),
        D_TEST_GENERATE_TEST(test_push_single_value_updates_size_and_empty_flag),
        D_TEST_GENERATE_TEST(test_push_copies_value_not_source_address),
        D_TEST_GENERATE_TEST(test_pop_returns_first_pushed_value),
        D_TEST_GENERATE_TEST(test_pop_many_values_are_fifo),
        D_TEST_GENERATE_TEST(test_pop_empty_queue_fails_and_keeps_output_unchanged),
        D_TEST_GENERATE_TEST(test_pop_until_empty_then_push_again),
        D_TEST_GENERATE_TEST(test_interleaved_push_pop_preserves_fifo_order),
        D_TEST_GENERATE_TEST(test_wraparound_without_growth_preserves_order),
        D_TEST_GENERATE_TEST(test_growth_while_wrapped_preserves_order),
        D_TEST_GENERATE_TEST(test_capacity_grows_from_zero_capacity),
        D_TEST_GENERATE_TEST(test_capacity_does_not_shrink_after_pops),
        D_TEST_GENERATE_TEST(test_queue_stores_pointer_values_fifo),
        D_TEST_GENERATE_TEST(test_queue_can_store_null_pointer_value_when_wrapped_in_slot),
        D_TEST_GENERATE_TEST(test_large_struct_elements_are_copied_and_fifo),
        D_TEST_GENERATE_TEST(test_binary_data_with_zero_bytes_round_trips),
        D_TEST_GENERATE_TEST(test_mutating_after_failed_pop_is_still_valid),
        D_TEST_GENERATE_TEST(test_many_alternating_operations_leave_consistent_state),
        D_TEST_GENERATE_TEST(test_multiple_queues_are_independent),
        D_TEST_GENERATE_TEST(test_fifo_order_after_repeated_full_empty_cycles),
        D_TEST_GENERATE_TEST(test_size_tracks_interleaved_operations_exactly),
        D_TEST_GENERATE_TEST(test_push_exactly_at_pow2_boundary_then_one_more),
        D_TEST_GENERATE_TEST(test_head_equals_tail_after_half_drain_then_refill),
        D_TEST_GENERATE_TEST(test_growth_triggered_at_exact_capacity_while_wrapped),
        D_TEST_GENERATE_TEST(test_destroy_calls_destructor_for_each_remaining_element),
        D_TEST_GENERATE_TEST(test_copy_with_copy_fn_deep_copies_pointer_elements),
        D_TEST_GENERATE_TEST(test_copy_without_copy_fn_pointer_array_is_shallow),
    };
    D_TEST_RUN_TESTS(tests);
    return 0;
}
