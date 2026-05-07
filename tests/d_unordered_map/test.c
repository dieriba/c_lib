#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "d_test.h"
#include "d_types.h"
#include "d_unordered_map.h"
#include "d_general_lib.h"

static usize g_destructor_calls = 0;
static usize g_hash_calls = 0;
static usize g_cmp_calls = 0;

typedef struct PairKey
{
    int bucket;
    int id;
} PairKey;

typedef struct BigValue
{
    uint64_t a;
    uint32_t b;
    char bytes[37];
    int tail;
} BigValue;

typedef struct BinaryValue
{
    unsigned char bytes[64];
} BinaryValue;

static void reset_counters(void)
{
    g_destructor_calls = 0;
    g_hash_calls = 0;
    g_cmp_calls = 0;
}

static usize hash_int_key(void *key)
{
    int value = *(int *)key;
    uint32_t x = (uint32_t)value;

    g_hash_calls++;
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return (usize)x;
}

static bool cmp_int_key(void *lhs, void *rhs)
{
    g_cmp_calls++;
    return *(int *)lhs == *(int *)rhs;
}

static usize constant_hash_key(void *key)
{
    (void)key;
    g_hash_calls++;
    return 0x1234UL;
}

static usize same_group_different_h2_hash(void *key)
{
    PairKey *pair = (PairKey *)key;

    g_hash_calls++;
    return ((usize)pair->bucket << 7) | ((usize)pair->id & 0x7FUL);
}

static bool cmp_pair_key(void *lhs, void *rhs)
{
    PairKey *a = (PairKey *)lhs;
    PairKey *b = (PairKey *)rhs;

    g_cmp_calls++;
    return a->bucket == b->bucket && a->id == b->id;
}

static void counting_free(void *elem)
{
    (void)elem;
    g_destructor_calls++;
}

static BigValue make_big_value(int seed)
{
    BigValue value;

    memset(&value, 0, sizeof(value));
    value.a = 0xabcddcba00000000ULL + (uint64_t)seed;
    value.b = (uint32_t)(seed * 97);
    for (usize i = 0; i < sizeof(value.bytes); i++)
        value.bytes[i] = (char)('A' + ((seed + (int)i) % 26));
    value.tail = -seed * 13;
    return value;
}

static void assert_big_value_eq(const BigValue *actual, const BigValue *expected)
{
    D_TEST_EXPR(actual->a == expected->a);
    D_TEST_EXPR(actual->b == expected->b);
    D_TEST_MEM_EQ(actual->bytes, expected->bytes, sizeof(expected->bytes));
    D_TEST_EXPR(actual->tail == expected->tail);
}

static DUnorderedMap *make_int_map(usize value_size, usize capacity, FnPtrFreeElem free_fn)
{
    DUnorderedMap *map = NULL;

    D_TEST_EXPR(d_unordered_map_new(&map, sizeof(int), value_size, capacity, hash_int_key, cmp_int_key, free_fn) == D_OK);
    D_TEST_NOT_NULL(map);
    return map;
}

static DUnorderedMap *make_constant_hash_int_map(usize value_size, usize capacity, FnPtrFreeElem free_fn)
{
    DUnorderedMap *map = NULL;

    D_TEST_EXPR(d_unordered_map_new(&map, sizeof(int), value_size, capacity, constant_hash_key, cmp_int_key, free_fn) == D_OK);
    D_TEST_NOT_NULL(map);
    return map;
}

static DUnorderedMap *make_pair_map(usize value_size, usize capacity)
{
    DUnorderedMap *map = NULL;

    D_TEST_EXPR(d_unordered_map_new(&map, sizeof(PairKey), value_size, capacity, same_group_different_h2_hash, cmp_pair_key, NULL) == D_OK);
    D_TEST_NOT_NULL(map);
    return map;
}

static void expect_size(DUnorderedMap *map, usize expected)
{
    usize size = (usize)-1;

    D_TEST_EXPR(d_unordered_map_get_size(map, &size) == D_OK);
    D_TEST_EXPR(size == expected);
}

static void expect_capacity_at_least(DUnorderedMap *map, usize expected_min)
{
    usize capacity = 0;

    D_TEST_EXPR(d_unordered_map_get_capacity(map, &capacity) == D_OK);
    D_TEST_EXPR(capacity >= expected_min);
}

static void insert_int_value(DUnorderedMap *map, int key, int value)
{
    D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
}

static int *get_int_value(DUnorderedMap *map, int key)
{
    return (int *)d_unordered_map_get(map, &key);
}

static void expect_int_value(DUnorderedMap *map, int key, int expected)
{
    int *got = get_int_value(map, key);

    D_TEST_NOT_NULL(got);
    D_TEST_EXPR(*got == expected);
}

static void destroy_map(DUnorderedMap **map)
{
    d_unordered_map_destroy(map);
    D_TEST_NULL(*map);
}

static void test_new_creates_empty_map_with_default_capacity(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);

    expect_size(map, 0);
    expect_capacity_at_least(map, 16);
    destroy_map(&map);
}

static void test_new_with_small_capacity_rounds_to_group_capacity(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 1, NULL);

    expect_size(map, 0);
    expect_capacity_at_least(map, 16);
    destroy_map(&map);
}

static void test_new_with_explicit_capacity_keeps_enough_capacity(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 64, NULL);

    expect_size(map, 0);
    expect_capacity_at_least(map, 64);
    destroy_map(&map);
}

static void test_new_rejects_null_output_pointer(void)
{
    D_TEST_EXPR(d_unordered_map_new(NULL, sizeof(int), sizeof(int), 16, hash_int_key, cmp_int_key, NULL) == D_ERR_INVALID_ARG);
}

static void test_new_rejects_zero_key_size(void)
{
    DUnorderedMap *map = NULL;

    D_TEST_EXPR(d_unordered_map_new(&map, 0, sizeof(int), 16, hash_int_key, cmp_int_key, NULL) != D_OK);
    D_TEST_NULL(map);
}

static void test_new_rejects_zero_value_size(void)
{
    DUnorderedMap *map = NULL;

    D_TEST_EXPR(d_unordered_map_new(&map, sizeof(int), 0, 16, hash_int_key, cmp_int_key, NULL) != D_OK);
    D_TEST_NULL(map);
}

static void test_get_size_rejects_null_map(void)
{
    usize size = 123;

    D_TEST_EXPR(d_unordered_map_get_size(NULL, &size) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(size == 123);
}

static void test_get_size_rejects_null_output(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);

    D_TEST_EXPR(d_unordered_map_get_size(map, NULL) == D_ERR_INVALID_ARG);
    destroy_map(&map);
}

static void test_get_capacity_rejects_null_map(void)
{
    usize capacity = 123;

    D_TEST_EXPR(d_unordered_map_get_capacity(NULL, &capacity) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(capacity == 123);
}

static void test_get_capacity_rejects_null_output(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);

    D_TEST_EXPR(d_unordered_map_get_capacity(map, NULL) == D_ERR_INVALID_ARG);
    destroy_map(&map);
}

static void test_insert_get_single_key(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);

    insert_int_value(map, 42, 9001);
    expect_size(map, 1);
    expect_int_value(map, 42, 9001);
    destroy_map(&map);
}

static void test_get_missing_key_returns_null(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);
    int key = 7;

    D_TEST_NULL(d_unordered_map_get(map, &key));
    expect_size(map, 0);
    destroy_map(&map);
}

static void test_insert_rejects_null_map(void)
{
    int key = 1;
    int value = 2;

    D_TEST_EXPR(d_unordered_map_insert(NULL, &key, &value) == D_ERR_INVALID_ARG);
}

static void test_insert_rejects_null_key(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);
    int value = 2;

    D_TEST_EXPR(d_unordered_map_insert(map, NULL, &value) == D_ERR_INVALID_ARG);
    expect_size(map, 0);
    destroy_map(&map);
}

static void test_insert_rejects_null_value(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);
    int key = 1;

    D_TEST_EXPR(d_unordered_map_insert(map, &key, NULL) == D_ERR_INVALID_ARG);
    expect_size(map, 0);
    D_TEST_NULL(d_unordered_map_get(map, &key));
    destroy_map(&map);
}

static void test_get_rejects_null_map(void)
{
    int key = 1;

    D_TEST_NULL(d_unordered_map_get(NULL, &key));
}

static void test_get_rejects_null_key(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);

    D_TEST_NULL(d_unordered_map_get(map, NULL));
    destroy_map(&map);
}

static void test_insert_copies_key_and_value_not_source_addresses(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);
    int key = 10;
    int value = 20;

    D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    key = 99;
    value = 88;
    expect_int_value(map, 10, 20);
    D_TEST_NULL(d_unordered_map_get(map, &key));
    destroy_map(&map);
}

static void test_duplicate_insert_updates_value_without_growing_size(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);

    insert_int_value(map, 5, 10);
    insert_int_value(map, 5, 20);
    insert_int_value(map, 5, 30);
    expect_size(map, 1);
    expect_int_value(map, 5, 30);
    destroy_map(&map);
}

static void test_many_unique_keys_are_retrievable_after_insertions(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 128, NULL);

    for (int i = 0; i < 80; i++)
        insert_int_value(map, i, i * 100 + 7);
    expect_size(map, 80);
    for (int i = 0; i < 80; i++)
        expect_int_value(map, i, i * 100 + 7);
    destroy_map(&map);
}

static void test_negative_and_large_integer_keys_round_trip(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 64, NULL);
    int keys[] = {0, -1, -2, INT32_MIN, INT32_MAX, 123456789, -987654321};

    for (usize i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
    {
        int value = (int)i * 111;
        D_TEST_EXPR(d_unordered_map_insert(map, &keys[i], &value) == D_OK);
    }
    for (usize i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
        expect_int_value(map, keys[i], (int)i * 111);
    destroy_map(&map);
}

static void test_constant_hash_collision_cluster_all_keys_retrievable(void)
{
    DUnorderedMap *map = make_constant_hash_int_map(sizeof(int), 64, NULL);

    for (int i = 0; i < 32; i++)
        insert_int_value(map, i, 1000 + i);
    expect_size(map, 32);
    for (int i = 0; i < 32; i++)
        expect_int_value(map, i, 1000 + i);
    destroy_map(&map);
}

static void test_collision_cluster_updates_existing_key_not_first_h2_match(void)
{
    DUnorderedMap *map = make_constant_hash_int_map(sizeof(int), 64, NULL);

    for (int i = 0; i < 20; i++)
        insert_int_value(map, i, i);
    insert_int_value(map, 13, 4242);
    expect_size(map, 20);
    for (int i = 0; i < 20; i++)
        expect_int_value(map, i, i == 13 ? 4242 : i);
    destroy_map(&map);
}

static void test_same_group_different_h2_values_are_retrievable(void)
{
    DUnorderedMap *map = make_pair_map(sizeof(int), 64);

    for (int i = 0; i < 48; i++)
    {
        PairKey key = {.bucket = 0, .id = i};
        int value = i * 3;
        D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    }
    expect_size(map, 48);
    for (int i = 0; i < 48; i++)
    {
        PairKey key = {.bucket = 0, .id = i};
        int *value = (int *)d_unordered_map_get(map, &key);
        D_TEST_NOT_NULL(value);
        D_TEST_EXPR(*value == i * 3);
    }
    destroy_map(&map);
}

static void test_remove_existing_key_returns_value_and_decrements_size(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);
    int out = 0;

    insert_int_value(map, 1, 11);
    insert_int_value(map, 2, 22);
    D_TEST_EXPR(d_unordered_map_remove(map, &(int){1}, &out) == D_OK);
    D_TEST_EXPR(out == 11);
    expect_size(map, 1);
    D_TEST_NULL(d_unordered_map_get(map, &(int){1}));
    expect_int_value(map, 2, 22);
    destroy_map(&map);
}

static void test_remove_existing_key_allows_null_out_elem(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);

    insert_int_value(map, 1, 11);
    D_TEST_EXPR(d_unordered_map_remove(map, &(int){1}, NULL) == D_OK);
    expect_size(map, 0);
    D_TEST_NULL(d_unordered_map_get(map, &(int){1}));
    destroy_map(&map);
}

static void test_remove_missing_key_fails_and_preserves_output(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);
    int out = 0x12345678;

    insert_int_value(map, 1, 11);
    D_TEST_EXPR(d_unordered_map_remove(map, &(int){2}, &out) == D_ERR_NOT_EXIST);
    D_TEST_EXPR(out == 0x12345678);
    expect_size(map, 1);
    expect_int_value(map, 1, 11);
    destroy_map(&map);
}

static void test_remove_rejects_null_map(void)
{
    int key = 1;
    int out = 123;

    D_TEST_EXPR(d_unordered_map_remove(NULL, &key, &out) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(out == 123);
}

static void test_remove_rejects_null_key(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);
    int out = 123;

    D_TEST_EXPR(d_unordered_map_remove(map, NULL, &out) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(out == 123);
    destroy_map(&map);
}

static void test_remove_twice_fails_second_time(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);
    int out = 0;

    insert_int_value(map, 1, 11);
    D_TEST_EXPR(d_unordered_map_remove(map, &(int){1}, &out) == D_OK);
    out = 0x77777777;
    D_TEST_EXPR(d_unordered_map_remove(map, &(int){1}, &out) == D_ERR_NOT_EXIST);
    D_TEST_EXPR(out == 0x77777777);
    expect_size(map, 0);
    destroy_map(&map);
}

static void test_remove_from_collision_cluster_does_not_break_later_gets(void)
{
    DUnorderedMap *map = make_constant_hash_int_map(sizeof(int), 64, NULL);
    int out = 0;

    for (int i = 0; i < 24; i++)
        insert_int_value(map, i, i * 10);
    D_TEST_EXPR(d_unordered_map_remove(map, &(int){0}, &out) == D_OK);
    D_TEST_EXPR(d_unordered_map_remove(map, &(int){7}, &out) == D_OK);
    D_TEST_EXPR(d_unordered_map_remove(map, &(int){15}, &out) == D_OK);
    expect_size(map, 21);
    for (int i = 0; i < 24; i++)
    {
        int *value = get_int_value(map, i);
        if (i == 0 || i == 7 || i == 15)
            D_TEST_NULL(value);
        else
        {
            D_TEST_NOT_NULL(value);
            D_TEST_EXPR(*value == i * 10);
        }
    }
    destroy_map(&map);
}

static void test_deleted_slots_are_reused_without_growing_size_wrongly(void)
{
    DUnorderedMap *map = make_constant_hash_int_map(sizeof(int), 64, NULL);
    int out = 0;

    for (int i = 0; i < 24; i++)
        insert_int_value(map, i, i);
    for (int i = 0; i < 12; i++)
        D_TEST_EXPR(d_unordered_map_remove(map, &i, &out) == D_OK);
    expect_size(map, 12);
    for (int i = 100; i < 112; i++)
        insert_int_value(map, i, i * 2);
    expect_size(map, 24);
    for (int i = 12; i < 24; i++)
        expect_int_value(map, i, i);
    for (int i = 100; i < 112; i++)
        expect_int_value(map, i, i * 2);
    destroy_map(&map);
}

static void test_remove_all_then_reinsert_many_keys(void)
{
    DUnorderedMap *map = make_constant_hash_int_map(sizeof(int), 64, NULL);
    int out = 0;

    for (int i = 0; i < 32; i++)
        insert_int_value(map, i, i + 1);
    for (int i = 0; i < 32; i++)
        D_TEST_EXPR(d_unordered_map_remove(map, &i, &out) == D_OK);
    expect_size(map, 0);
    for (int i = 0; i < 32; i++)
        D_TEST_NULL(d_unordered_map_get(map, &i));
    for (int i = 0; i < 32; i++)
        insert_int_value(map, i + 1000, i + 2000);
    expect_size(map, 32);
    for (int i = 0; i < 32; i++)
        expect_int_value(map, i + 1000, i + 2000);
    destroy_map(&map);
}

static void test_rehash_preserves_existing_keys_and_inserts_new_key(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 16, NULL);
    usize old_capacity = 0;
    usize new_capacity = 0;

    D_TEST_EXPR(d_unordered_map_get_capacity(map, &old_capacity) == D_OK);
    for (int i = 0; i < 200; i++)
        insert_int_value(map, i, i * 5);
    D_TEST_EXPR(d_unordered_map_get_capacity(map, &new_capacity) == D_OK);
    D_TEST_EXPR(new_capacity > old_capacity);
    expect_size(map, 200);
    for (int i = 0; i < 200; i++)
        expect_int_value(map, i, i * 5);
    destroy_map(&map);
}

static void test_rehash_after_deletions_preserves_live_keys_only(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 16, NULL);
    int out = 0;

    for (int i = 0; i < 80; i++)
        insert_int_value(map, i, i + 10);
    for (int i = 0; i < 40; i += 2)
        D_TEST_EXPR(d_unordered_map_remove(map, &i, &out) == D_OK);
    for (int i = 80; i < 220; i++)
        insert_int_value(map, i, i + 10);
    for (int i = 0; i < 220; i++)
    {
        int *value = get_int_value(map, i);
        if (i < 40 && i % 2 == 0)
            D_TEST_NULL(value);
        else
        {
            D_TEST_NOT_NULL(value);
            D_TEST_EXPR(*value == i + 10);
        }
    }
    destroy_map(&map);
}

static void test_string_key_constructor_basic_insert_get(void)
{
    DUnorderedMap *map = NULL;
    char *key = "hello";
    int value = 42;

    D_TEST_EXPR(d_unordered_map_new_str(&map, sizeof(int), 0, NULL) == D_OK);
    D_TEST_NOT_NULL(map);
    D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    D_TEST_EXPR(*(int *)d_unordered_map_get(map, &key) == 42);
    destroy_map(&map);
}

static void test_string_keys_compare_by_content_not_pointer_address(void)
{
    DUnorderedMap *map = NULL;
    char key_a_buf[] = "same-key";
    char key_b_buf[] = "same-key";
    char *key_a = key_a_buf;
    char *key_b = key_b_buf;
    int value = 77;

    D_TEST_EXPR(d_unordered_map_new_str(&map, sizeof(int), 0, NULL) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(map, &key_a, &value) == D_OK);
    D_TEST_EXPR(*(int *)d_unordered_map_get(map, &key_b) == 77);
    destroy_map(&map);
}

static void test_string_duplicate_key_updates_value(void)
{
    DUnorderedMap *map = NULL;
    char *key1 = "dup";
    char key2_buf[] = "dup";
    char *key2 = key2_buf;
    int one = 1;
    int two = 2;

    D_TEST_EXPR(d_unordered_map_new_str(&map, sizeof(int), 0, NULL) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(map, &key1, &one) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(map, &key2, &two) == D_OK);
    expect_size(map, 1);
    D_TEST_EXPR(*(int *)d_unordered_map_get(map, &key1) == 2);
    destroy_map(&map);
}

static void test_string_key_source_pointer_can_change_after_insert(void)
{
    DUnorderedMap *map = NULL;
    char *key = "first";
    char *saved = "first";
    int value = 11;

    D_TEST_EXPR(d_unordered_map_new_str(&map, sizeof(int), 0, NULL) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    key = "second";
    D_TEST_EXPR(*(int *)d_unordered_map_get(map, &saved) == 11);
    D_TEST_NULL(d_unordered_map_get(map, &key));
    destroy_map(&map);
}

static void test_map_stores_pointer_values_without_dereferencing_them(void)
{
    DUnorderedMap *map = make_int_map(sizeof(char *), 0, NULL);
    int key = 1;
    char *value = "payload";
    char **got = NULL;

    D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    got = (char **)d_unordered_map_get(map, &key);
    D_TEST_NOT_NULL(got);
    D_TEST_EXPR(*got == value);
    destroy_map(&map);
}

static void test_map_can_store_null_pointer_as_wrapped_value(void)
{
    DUnorderedMap *map = make_int_map(sizeof(char *), 0, NULL);
    int key = 1;
    char *value = NULL;
    char **got = NULL;

    D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    got = (char **)d_unordered_map_get(map, &key);
    D_TEST_NOT_NULL(got);
    D_TEST_NULL(*got);
    destroy_map(&map);
}

static void test_remove_pointer_value_copies_pointer_to_out_elem(void)
{
    DUnorderedMap *map = make_int_map(sizeof(char *), 0, NULL);
    int key = 1;
    char *value = "owned-by-test";
    char *out = NULL;

    D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    D_TEST_EXPR(d_unordered_map_remove(map, &key, &out) == D_OK);
    D_TEST_EXPR(out == value);
    expect_size(map, 0);
    destroy_map(&map);
}

static void test_big_value_round_trips_field_wise(void)
{
    DUnorderedMap *map = make_int_map(sizeof(BigValue), 0, NULL);
    int key = 777;
    BigValue value = make_big_value(77);
    BigValue *got = NULL;

    D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    got = (BigValue *)d_unordered_map_get(map, &key);
    D_TEST_NOT_NULL(got);
    assert_big_value_eq(got, &value);
    destroy_map(&map);
}

static void test_big_values_in_collision_cluster_round_trip(void)
{
    DUnorderedMap *map = make_constant_hash_int_map(sizeof(BigValue), 64, NULL);

    for (int i = 0; i < 30; i++)
    {
        BigValue value = make_big_value(i);
        D_TEST_EXPR(d_unordered_map_insert(map, &i, &value) == D_OK);
    }
    for (int i = 0; i < 30; i++)
    {
        BigValue expected = make_big_value(i);
        BigValue *got = (BigValue *)d_unordered_map_get(map, &i);
        D_TEST_NOT_NULL(got);
        assert_big_value_eq(got, &expected);
    }
    destroy_map(&map);
}

static void test_binary_value_with_zero_bytes_round_trips(void)
{
    DUnorderedMap *map = make_int_map(sizeof(BinaryValue), 0, NULL);
    BinaryValue value;
    int key = 1;
    BinaryValue *got = NULL;

    for (usize i = 0; i < sizeof(value.bytes); i++)
        value.bytes[i] = (unsigned char)((i * 17) & 0xff);
    value.bytes[0] = 0;
    value.bytes[13] = 0;
    value.bytes[63] = 0;
    D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    got = (BinaryValue *)d_unordered_map_get(map, &key);
    D_TEST_NOT_NULL(got);
    D_TEST_MEM_EQ(got->bytes, value.bytes, sizeof(value.bytes));
    destroy_map(&map);
}

static void test_destroy_accepts_null_outer_pointer(void)
{
    d_unordered_map_destroy(NULL);
}

static void test_destroy_accepts_null_map_pointer(void)
{
    DUnorderedMap *map = NULL;

    d_unordered_map_destroy(&map);
    D_TEST_NULL(map);
}

static void test_destroy_sets_pointer_to_null(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);

    d_unordered_map_destroy(&map);
    D_TEST_NULL(map);
}

static void test_destroy_calls_free_fn_for_each_live_element(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 64, counting_free);

    reset_counters();
    for (int i = 0; i < 20; i++)
        insert_int_value(map, i, i * 10);
    d_unordered_map_destroy(&map);
    D_TEST_NULL(map);
    D_TEST_EXPR(g_destructor_calls == 20);
}

static void test_destroy_does_not_call_free_fn_for_removed_elements(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 64, counting_free);
    int out = 0;

    reset_counters();
    for (int i = 0; i < 20; i++)
        insert_int_value(map, i, i);
    for (int i = 0; i < 5; i++)
        D_TEST_EXPR(d_unordered_map_remove(map, &i, &out) == D_OK);
    d_unordered_map_destroy(&map);
    D_TEST_NULL(map);
    D_TEST_EXPR(g_destructor_calls == 15);
}

static void test_remove_does_not_call_free_fn_when_out_elem_takes_ownership(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, counting_free);
    int out = 0;

    reset_counters();
    insert_int_value(map, 1, 2);
    D_TEST_EXPR(d_unordered_map_remove(map, &(int){1}, &out) == D_OK);
    D_TEST_EXPR(g_destructor_calls == 0);
    d_unordered_map_destroy(&map);
    D_TEST_EXPR(g_destructor_calls == 0);
}

static void test_duplicate_insert_with_free_fn_does_not_destroy_live_new_value(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, counting_free);

    reset_counters();
    insert_int_value(map, 1, 10);
    insert_int_value(map, 1, 20);
    expect_size(map, 1);
    expect_int_value(map, 1, 20);
    D_TEST_EXPR(g_destructor_calls <= 1);
    d_unordered_map_destroy(&map);
}

static void test_hash_and_compare_are_used_for_operations(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);

    reset_counters();
    insert_int_value(map, 1, 10);
    expect_int_value(map, 1, 10);
    D_TEST_EXPR(g_hash_calls >= 2);
    D_TEST_EXPR(g_cmp_calls >= 1);
    destroy_map(&map);
}

static void test_interleaved_insert_remove_update_sequence(void)
{
    DUnorderedMap *map = make_constant_hash_int_map(sizeof(int), 128, NULL);
    int out = 0;

    for (int round = 0; round < 20; round++)
    {
        for (int i = 0; i < 50; i++)
            insert_int_value(map, i, round * 1000 + i);
        for (int i = 0; i < 50; i += 3)
            D_TEST_EXPR(d_unordered_map_remove(map, &i, &out) == D_OK);
        for (int i = 0; i < 50; i += 3)
            insert_int_value(map, i, round * 2000 + i);
        expect_size(map, 50);
        for (int i = 0; i < 50; i++)
            expect_int_value(map, i, (i % 3 == 0) ? round * 2000 + i : round * 1000 + i);
    }
    destroy_map(&map);
}

static void test_failed_insert_null_value_does_not_poison_future_insert(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);
    int key = 1;
    int value = 10;

    D_TEST_EXPR(d_unordered_map_insert(map, &key, NULL) == D_ERR_INVALID_ARG);
    expect_size(map, 0);
    D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    expect_size(map, 1);
    expect_int_value(map, 1, 10);
    destroy_map(&map);
}

static void test_failed_remove_missing_does_not_poison_future_insert(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);
    int out = 0;

    D_TEST_EXPR(d_unordered_map_remove(map, &(int){42}, &out) == D_ERR_NOT_EXIST);
    insert_int_value(map, 42, 99);
    expect_int_value(map, 42, 99);
    destroy_map(&map);
}

static void test_multiple_maps_are_independent(void)
{
    DUnorderedMap *a = make_int_map(sizeof(int), 0, NULL);
    DUnorderedMap *b = make_int_map(sizeof(int), 0, NULL);

    insert_int_value(a, 1, 10);
    insert_int_value(b, 1, 20);
    expect_int_value(a, 1, 10);
    expect_int_value(b, 1, 20);
    destroy_map(&a);
    destroy_map(&b);
}

static void test_many_maps_create_destroy_without_cross_state(void)
{
    for (int round = 0; round < 100; round++)
    {
        DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);
        insert_int_value(map, round, round * 2);
        expect_int_value(map, round, round * 2);
        destroy_map(&map);
    }
}

static void test_capacity_query_after_many_mutations_is_still_valid(void)
{
    DUnorderedMap *map = make_constant_hash_int_map(sizeof(int), 32, NULL);
    usize capacity = 0;
    int out = 0;

    for (int i = 0; i < 100; i++)
        insert_int_value(map, i, i);
    for (int i = 0; i < 50; i++)
        D_TEST_EXPR(d_unordered_map_remove(map, &i, &out) == D_OK);
    D_TEST_EXPR(d_unordered_map_get_capacity(map, &capacity) == D_OK);
    D_TEST_EXPR(capacity >= 32);
    expect_size(map, 50);
    destroy_map(&map);
}

static void test_remove_last_remaining_key_then_insert_same_key_again(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);
    int out = 0;

    insert_int_value(map, 9, 90);
    D_TEST_EXPR(d_unordered_map_remove(map, &(int){9}, &out) == D_OK);
    expect_size(map, 0);
    insert_int_value(map, 9, 91);
    expect_size(map, 1);
    expect_int_value(map, 9, 91);
    destroy_map(&map);
}

static void test_all_keys_survive_several_rehash_boundaries(void)
{
    DUnorderedMap *map = make_int_map(sizeof(int), 0, NULL);

    for (int i = 0; i < 1000; i++)
        insert_int_value(map, i * 17, i + 123);
    expect_size(map, 1000);
    for (int i = 0; i < 1000; i++)
        expect_int_value(map, i * 17, i + 123);
    destroy_map(&map);
}

static void test_default_int32_constructor_round_trips_negative_zero_positive_keys(void)
{
    DUnorderedMap *map = NULL;
    int32 keys[] = {0, -1, 1, -2147483647 - 1, 2147483647};

    D_TEST_EXPR(d_unordered_map_new_int32_key(&map, sizeof(int), 0, NULL) == D_OK);
    D_TEST_NOT_NULL(map);
    for (usize i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
    {
        int value = (int)(1000 + i);
        D_TEST_EXPR(d_unordered_map_insert(map, &keys[i], &value) == D_OK);
    }
    expect_size(map, sizeof(keys) / sizeof(keys[0]));
    for (usize i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
    {
        int *got = (int *)d_unordered_map_get(map, &keys[i]);
        D_TEST_NOT_NULL(got);
        D_TEST_EXPR(*got == (int)(1000 + i));
    }
    destroy_map(&map);
}

static void test_default_int32_duplicate_updates_and_remove_works(void)
{
    DUnorderedMap *map = NULL;
    int32 key = -1234567;
    int one = 1;
    int two = 2;
    int out = 0;

    D_TEST_EXPR(d_unordered_map_new_int32_key(&map, sizeof(int), 0, NULL) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(map, &key, &one) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(map, &key, &two) == D_OK);
    expect_size(map, 1);
    D_TEST_EXPR(*(int *)d_unordered_map_get(map, &key) == 2);
    D_TEST_EXPR(d_unordered_map_remove(map, &key, &out) == D_OK);
    D_TEST_EXPR(out == 2);
    expect_size(map, 0);
    D_TEST_NULL(d_unordered_map_get(map, &key));
    destroy_map(&map);
}

static void test_default_int64_constructor_handles_wide_bit_patterns(void)
{
    DUnorderedMap *map = NULL;
    int64 keys[] = {
        (int64)0,
        (int64)-1,
        ((int64)1 << 40),
        -((int64)1 << 40),
        (int64)0x7fffffffffffffffLL,
        (int64)(-0x7fffffffffffffffLL - 1LL),
    };

    D_TEST_EXPR(d_unordered_map_new_int64_key(&map, sizeof(int64), 0, NULL) == D_OK);
    D_TEST_NOT_NULL(map);
    for (usize i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
    {
        int64 value = keys[i] ^ (int64)0x55aa55aa55aa55aaLL;
        D_TEST_EXPR(d_unordered_map_insert(map, &keys[i], &value) == D_OK);
    }
    expect_size(map, sizeof(keys) / sizeof(keys[0]));
    for (usize i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
    {
        int64 expected = keys[i] ^ (int64)0x55aa55aa55aa55aaLL;
        int64 *got = (int64 *)d_unordered_map_get(map, &keys[i]);
        D_TEST_NOT_NULL(got);
        D_TEST_EXPR(*got == expected);
    }
    destroy_map(&map);
}

static void test_default_usize_constructor_handles_large_values_and_zero(void)
{
    DUnorderedMap *map = NULL;
    usize keys[] = {0, 1, 15, 16, 17, ((usize)-1), ((usize)-1) / 2};

    D_TEST_EXPR(d_unordered_map_new_usize_key(&map, sizeof(usize), 0, NULL) == D_OK);
    D_TEST_NOT_NULL(map);
    for (usize i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
    {
        usize value = keys[i] + (usize)12345;
        D_TEST_EXPR(d_unordered_map_insert(map, &keys[i], &value) == D_OK);
    }
    expect_size(map, sizeof(keys) / sizeof(keys[0]));
    for (usize i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
    {
        usize expected = keys[i] + (usize)12345;
        usize *got = (usize *)d_unordered_map_get(map, &keys[i]);
        D_TEST_NOT_NULL(got);
        D_TEST_EXPR(*got == expected);
    }
    destroy_map(&map);
}

static void test_default_usize_many_insertions_survive_rehash(void)
{
    DUnorderedMap *map = NULL;

    D_TEST_EXPR(d_unordered_map_new_usize_key(&map, sizeof(usize), 0, NULL) == D_OK);
    D_TEST_NOT_NULL(map);
    for (usize i = 0; i < 600; i++)
    {
        usize key = i * (usize)2654435761UL;
        usize value = i ^ (usize)0xdeadbeefUL;
        D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    }
    expect_size(map, 600);
    for (usize i = 0; i < 600; i++)
    {
        usize key = i * (usize)2654435761UL;
        usize expected = i ^ (usize)0xdeadbeefUL;
        usize *got = (usize *)d_unordered_map_get(map, &key);
        D_TEST_NOT_NULL(got);
        D_TEST_EXPR(*got == expected);
    }
    destroy_map(&map);
}

static void test_default_u32_constructor_remove_even_keys_keeps_odd_keys(void)
{
    DUnorderedMap *map = NULL;

    D_TEST_EXPR(d_unordered_map_new_u32_key(&map, sizeof(u32), 0, NULL) == D_OK);
    D_TEST_NOT_NULL(map);
    for (u32 i = 0; i < 200; i++)
    {
        u32 key = i * 3U;
        u32 value = i + 9000U;
        D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    }
    for (u32 i = 0; i < 200; i += 2)
    {
        u32 key = i * 3U;
        u32 out = 0;
        D_TEST_EXPR(d_unordered_map_remove(map, &key, &out) == D_OK);
        D_TEST_EXPR(out == i + 9000U);
    }
    expect_size(map, 100);
    for (u32 i = 1; i < 200; i += 2)
    {
        u32 key = i * 3U;
        u32 *got = (u32 *)d_unordered_map_get(map, &key);
        D_TEST_NOT_NULL(got);
        D_TEST_EXPR(*got == i + 9000U);
    }
    destroy_map(&map);
}

static void test_default_char_constructor_distinguishes_all_byte_like_char_keys(void)
{
    DUnorderedMap *map = NULL;

    D_TEST_EXPR(d_unordered_map_new_char_key(&map, sizeof(int), 0, NULL) == D_OK);
    D_TEST_NOT_NULL(map);
    for (int i = 0; i < 96; i++)
    {
        char key = (char)(32 + i);
        int value = i * 11;
        D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    }
    expect_size(map, 96);
    for (int i = 0; i < 96; i++)
    {
        char key = (char)(32 + i);
        int *got = (int *)d_unordered_map_get(map, &key);
        D_TEST_NOT_NULL(got);
        D_TEST_EXPR(*got == i * 11);
    }
    destroy_map(&map);
}

static void test_default_bool_constructor_has_only_two_keys_and_updates(void)
{
    DUnorderedMap *map = NULL;
    bool t = true;
    bool f = false;
    int one = 1;
    int two = 2;
    int three = 3;

    D_TEST_EXPR(d_unordered_map_new_bool_key(&map, sizeof(int), 0, NULL) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(map, &t, &one) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(map, &f, &two) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(map, &t, &three) == D_OK);
    expect_size(map, 2);
    D_TEST_EXPR(*(int *)d_unordered_map_get(map, &t) == 3);
    D_TEST_EXPR(*(int *)d_unordered_map_get(map, &f) == 2);
    destroy_map(&map);
}

static void test_default_str_constructor_many_strings_survive_rehash_and_deletions(void)
{
    DUnorderedMap *map = NULL;
    char storage[160][24];

    D_TEST_EXPR(d_unordered_map_new_str(&map, sizeof(int), 0, NULL) == D_OK);
    D_TEST_NOT_NULL(map);
    for (int i = 0; i < 160; i++)
    {
        snprintf(storage[i], sizeof(storage[i]), "key-%03d", i);
        char *key = storage[i];
        int value = i * 7;
        D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    }
    for (int i = 0; i < 160; i += 3)
    {
        char *key = storage[i];
        int out = -1;
        D_TEST_EXPR(d_unordered_map_remove(map, &key, &out) == D_OK);
        D_TEST_EXPR(out == i * 7);
    }
    for (int i = 0; i < 160; i++)
    {
        char *key = storage[i];
        int *got = (int *)d_unordered_map_get(map, &key);
        if (i % 3 == 0)
            D_TEST_NULL(got);
        else
        {
            D_TEST_NOT_NULL(got);
            D_TEST_EXPR(*got == i * 7);
        }
    }
    destroy_map(&map);
}

static void test_default_str_constructor_empty_string_and_similar_prefixes_are_distinct(void)
{
    DUnorderedMap *map = NULL;
    char *keys[] = {"", "a", "aa", "aaa", "aaaa", "aaab", "aab", "ab"};

    D_TEST_EXPR(d_unordered_map_new_str(&map, sizeof(int), 0, NULL) == D_OK);
    D_TEST_NOT_NULL(map);
    for (usize i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
    {
        int value = (int)i + 50;
        D_TEST_EXPR(d_unordered_map_insert(map, &keys[i], &value) == D_OK);
    }
    expect_size(map, sizeof(keys) / sizeof(keys[0]));
    for (usize i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
    {
        int *got = (int *)d_unordered_map_get(map, &keys[i]);
        D_TEST_NOT_NULL(got);
        D_TEST_EXPR(*got == (int)i + 50);
    }
    destroy_map(&map);
}

int main(void)
{
    DTest tests[] = {
        D_TEST_GENERATE_TEST(test_new_creates_empty_map_with_default_capacity),
        D_TEST_GENERATE_TEST(test_new_with_small_capacity_rounds_to_group_capacity),
        D_TEST_GENERATE_TEST(test_new_with_explicit_capacity_keeps_enough_capacity),
        D_TEST_GENERATE_TEST(test_new_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_new_rejects_zero_key_size),
        D_TEST_GENERATE_TEST(test_new_rejects_zero_value_size),
        D_TEST_GENERATE_TEST(test_get_size_rejects_null_map),
        D_TEST_GENERATE_TEST(test_get_size_rejects_null_output),
        D_TEST_GENERATE_TEST(test_get_capacity_rejects_null_map),
        D_TEST_GENERATE_TEST(test_get_capacity_rejects_null_output),
        D_TEST_GENERATE_TEST(test_insert_get_single_key),
        D_TEST_GENERATE_TEST(test_get_missing_key_returns_null),
        D_TEST_GENERATE_TEST(test_insert_rejects_null_map),
        D_TEST_GENERATE_TEST(test_insert_rejects_null_key),
        D_TEST_GENERATE_TEST(test_insert_rejects_null_value),
        D_TEST_GENERATE_TEST(test_get_rejects_null_map),
        D_TEST_GENERATE_TEST(test_get_rejects_null_key),
        D_TEST_GENERATE_TEST(test_insert_copies_key_and_value_not_source_addresses),
        D_TEST_GENERATE_TEST(test_duplicate_insert_updates_value_without_growing_size),
        D_TEST_GENERATE_TEST(test_many_unique_keys_are_retrievable_after_insertions),
        D_TEST_GENERATE_TEST(test_negative_and_large_integer_keys_round_trip),
        D_TEST_GENERATE_TEST(test_constant_hash_collision_cluster_all_keys_retrievable),
        D_TEST_GENERATE_TEST(test_collision_cluster_updates_existing_key_not_first_h2_match),
        D_TEST_GENERATE_TEST(test_same_group_different_h2_values_are_retrievable),
        D_TEST_GENERATE_TEST(test_remove_existing_key_returns_value_and_decrements_size),
        D_TEST_GENERATE_TEST(test_remove_existing_key_allows_null_out_elem),
        D_TEST_GENERATE_TEST(test_remove_missing_key_fails_and_preserves_output),
        D_TEST_GENERATE_TEST(test_remove_rejects_null_map),
        D_TEST_GENERATE_TEST(test_remove_rejects_null_key),
        D_TEST_GENERATE_TEST(test_remove_twice_fails_second_time),
        D_TEST_GENERATE_TEST(test_remove_from_collision_cluster_does_not_break_later_gets),
        D_TEST_GENERATE_TEST(test_deleted_slots_are_reused_without_growing_size_wrongly),
        D_TEST_GENERATE_TEST(test_remove_all_then_reinsert_many_keys),
        D_TEST_GENERATE_TEST(test_rehash_preserves_existing_keys_and_inserts_new_key),
        D_TEST_GENERATE_TEST(test_rehash_after_deletions_preserves_live_keys_only),
        D_TEST_GENERATE_TEST(test_string_key_constructor_basic_insert_get),
        D_TEST_GENERATE_TEST(test_string_keys_compare_by_content_not_pointer_address),
        D_TEST_GENERATE_TEST(test_string_duplicate_key_updates_value),
        D_TEST_GENERATE_TEST(test_string_key_source_pointer_can_change_after_insert),
        D_TEST_GENERATE_TEST(test_map_stores_pointer_values_without_dereferencing_them),
        D_TEST_GENERATE_TEST(test_map_can_store_null_pointer_as_wrapped_value),
        D_TEST_GENERATE_TEST(test_remove_pointer_value_copies_pointer_to_out_elem),
        D_TEST_GENERATE_TEST(test_big_value_round_trips_field_wise),
        D_TEST_GENERATE_TEST(test_big_values_in_collision_cluster_round_trip),
        D_TEST_GENERATE_TEST(test_binary_value_with_zero_bytes_round_trips),
        D_TEST_GENERATE_TEST(test_destroy_accepts_null_outer_pointer),
        D_TEST_GENERATE_TEST(test_destroy_accepts_null_map_pointer),
        D_TEST_GENERATE_TEST(test_destroy_sets_pointer_to_null),
        D_TEST_GENERATE_TEST(test_destroy_calls_free_fn_for_each_live_element),
        D_TEST_GENERATE_TEST(test_destroy_does_not_call_free_fn_for_removed_elements),
        D_TEST_GENERATE_TEST(test_remove_does_not_call_free_fn_when_out_elem_takes_ownership),
        D_TEST_GENERATE_TEST(test_duplicate_insert_with_free_fn_does_not_destroy_live_new_value),
        D_TEST_GENERATE_TEST(test_hash_and_compare_are_used_for_operations),
        D_TEST_GENERATE_TEST(test_interleaved_insert_remove_update_sequence),
        D_TEST_GENERATE_TEST(test_failed_insert_null_value_does_not_poison_future_insert),
        D_TEST_GENERATE_TEST(test_failed_remove_missing_does_not_poison_future_insert),
        D_TEST_GENERATE_TEST(test_multiple_maps_are_independent),
        D_TEST_GENERATE_TEST(test_many_maps_create_destroy_without_cross_state),
        D_TEST_GENERATE_TEST(test_capacity_query_after_many_mutations_is_still_valid),
        D_TEST_GENERATE_TEST(test_remove_last_remaining_key_then_insert_same_key_again),
        D_TEST_GENERATE_TEST(test_all_keys_survive_several_rehash_boundaries),
        D_TEST_GENERATE_TEST(test_default_int32_constructor_round_trips_negative_zero_positive_keys),
        D_TEST_GENERATE_TEST(test_default_int32_duplicate_updates_and_remove_works),
        D_TEST_GENERATE_TEST(test_default_int64_constructor_handles_wide_bit_patterns),
        D_TEST_GENERATE_TEST(test_default_usize_constructor_handles_large_values_and_zero),
        D_TEST_GENERATE_TEST(test_default_usize_many_insertions_survive_rehash),
        D_TEST_GENERATE_TEST(test_default_u32_constructor_remove_even_keys_keeps_odd_keys),
        D_TEST_GENERATE_TEST(test_default_char_constructor_distinguishes_all_byte_like_char_keys),
        D_TEST_GENERATE_TEST(test_default_bool_constructor_has_only_two_keys_and_updates),
        D_TEST_GENERATE_TEST(test_default_str_constructor_many_strings_survive_rehash_and_deletions),
        D_TEST_GENERATE_TEST(test_default_str_constructor_empty_string_and_similar_prefixes_are_distinct),
    };

    D_TEST_RUN_TESTS(tests);
    return 0;
}
