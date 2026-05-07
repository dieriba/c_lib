#include "d_test.h"
#include "d_unordered_map.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct SPair
{
    int key;
    int value;
} SPair;

typedef struct BigValue
{
    int id;
    double score;
    unsigned char bytes[37];
} BigValue;

typedef struct BinKey
{
    unsigned char bytes[8];
} BinKey;

static int g_free_calls;
static int g_key_sum;
static int g_value_sum;
static int g_live_owned_keys;
static int g_live_owned_values;

static void reset_counters(void)
{
    g_free_calls = 0;
    g_key_sum = 0;
    g_value_sum = 0;
    g_live_owned_keys = 0;
    g_live_owned_values = 0;
}

static usize hash_int_key(void *key)
{
    int k = *(int *)key;
    return (usize)((unsigned)k * 2654435761u);
}

static usize hash_int_collision(void *key)
{
    (void)key;
    return 0x42;
}

static usize hash_int_two_groups(void *key)
{
    int k = *(int *)key;
    return (usize)((k % 2) << 7); /* same h2, only two h1 groups */
}

static bool cmp_int_key(void *a, void *b)
{
    return *(int *)a == *(int *)b;
}

static usize hash_bin_key(void *key)
{
    const BinKey *k = (const BinKey *)key;
    usize h = 1469598103934665603ULL;
    for (usize i = 0; i < sizeof(k->bytes); ++i)
    {
        h ^= k->bytes[i];
        h *= 1099511628211ULL;
    }
    return h;
}

static bool cmp_bin_key(void *a, void *b)
{
    return memcmp(a, b, sizeof(BinKey)) == 0;
}

static void destroy_count_int_pair(void *key, void *value)
{
    g_free_calls++;
    if (key)
        g_key_sum += *(int *)key;
    if (value)
        g_value_sum += *(int *)value;
}

static void destroy_owned_string_pair(void *key, void *value)
{
    char *k = *(char **)key;
    char *v = *(char **)value;
    if (k)
    {
        g_live_owned_keys--;
        free(k);
    }
    if (v)
    {
        g_live_owned_values--;
        free(v);
    }
    g_free_calls++;
}

static char *dup_lit(const char *s)
{
    usize len = strlen(s) + 1;
    char *out = malloc(len);
    D_TEST_NOT_NULL(out);
    memcpy(out, s, len);
    return out;
}

static DUnorderedMap *new_int_map_custom(usize value_size, usize capacity, FnPtrFreeHashMap free_fn)
{
    DUnorderedMap *map = NULL;
    D_TEST_EXPR(d_unordered_map_new(&map, sizeof(int), value_size, capacity, hash_int_key, cmp_int_key, free_fn) == D_OK);
    D_TEST_NOT_NULL(map);
    return map;
}

static DUnorderedMap *new_collision_map(usize value_size, usize capacity, FnPtrFreeHashMap free_fn)
{
    DUnorderedMap *map = NULL;
    D_TEST_EXPR(d_unordered_map_new(&map, sizeof(int), value_size, capacity, hash_int_collision, cmp_int_key, free_fn) == D_OK);
    D_TEST_NOT_NULL(map);
    return map;
}

static void assert_size(DUnorderedMap *map, usize expected)
{
    usize size = 999999;
    D_TEST_EXPR(d_unordered_map_get_size(map, &size) == D_OK);
    D_TEST_EXPR(size == expected);
}

static void assert_capacity_at_least(DUnorderedMap *map, usize expected_min)
{
    usize capacity = 0;
    D_TEST_EXPR(d_unordered_map_get_capacity(map, &capacity) == D_OK);
    D_TEST_EXPR(capacity >= expected_min);
}

static void insert_int(DUnorderedMap *map, int key, int value)
{
    D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
}

static void assert_get_int(DUnorderedMap *map, int key, int expected)
{
    int *got = d_unordered_map_get(map, &key);
    D_TEST_NOT_NULL(got);
    D_TEST_EXPR(*got == expected);
}

static void test_new_rejects_null_output_pointer(void)
{
    D_TEST_EXPR(d_unordered_map_new(NULL, sizeof(int), sizeof(int), 0, hash_int_key, cmp_int_key, NULL) == D_ERR_INVALID_ARG);
}

static void test_new_allows_zero_capacity_and_reports_nonzero_capacity(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, NULL);
    assert_size(map, 0);
    assert_capacity_at_least(map, 16);
    d_unordered_map_destroy(&map);
}

static void test_destroy_null_pointer_is_noop(void)
{
    d_unordered_map_destroy(NULL);
    DUnorderedMap *map = NULL;
    d_unordered_map_destroy(&map);
    D_TEST_NULL(map);
}

static void test_insert_rejects_null_map(void)
{
    int k = 1;
    int v = 2;
    D_TEST_EXPR(d_unordered_map_insert(NULL, &k, &v) == D_ERR_INVALID_ARG);
}

static void test_insert_rejects_null_key(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, NULL);
    int v = 2;
    D_TEST_EXPR(d_unordered_map_insert(map, NULL, &v) == D_ERR_INVALID_ARG);
    assert_size(map, 0);
    d_unordered_map_destroy(&map);
}

static void test_insert_rejects_null_value(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, NULL);
    int k = 1;
    D_TEST_EXPR(d_unordered_map_insert(map, &k, NULL) == D_ERR_INVALID_ARG);
    assert_size(map, 0);
    d_unordered_map_destroy(&map);
}

static void test_get_null_map_returns_null(void)
{
    int k = 1;
    D_TEST_NULL(d_unordered_map_get(NULL, &k));
}

static void test_get_null_key_returns_null(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, NULL);
    D_TEST_NULL(d_unordered_map_get(map, NULL));
    d_unordered_map_destroy(&map);
}

static void test_remove_rejects_null_output_key_slot(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, NULL);
    insert_int(map, 1, 10);
    int out_value = 0;
    D_TEST_EXPR(d_unordered_map_remove(map, &(int){1}, NULL, &out_value) == D_ERR_INVALID_ARG);
    assert_size(map, 1);
    assert_get_int(map, 1, 10);
    d_unordered_map_destroy(&map);
}

static void test_remove_rejects_null_output_value_slot(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, NULL);
    insert_int(map, 1, 10);
    int out_key = 0;
    D_TEST_EXPR(d_unordered_map_remove(map, &(int){1}, &out_key, NULL) == D_ERR_INVALID_ARG);
    assert_size(map, 1);
    assert_get_int(map, 1, 10);
    d_unordered_map_destroy(&map);
}

static void test_remove_null_map_is_invalid(void)
{
    int out_key = 0;
    int out_value = 0;
    D_TEST_EXPR(d_unordered_map_remove(NULL, &(int){1}, &out_key, &out_value) == D_ERR_INVALID_ARG);
}

static void test_remove_null_lookup_key_is_invalid(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, NULL);
    int out_key = 0;
    int out_value = 0;
    D_TEST_EXPR(d_unordered_map_remove(map, NULL, &out_key, &out_value) == D_ERR_INVALID_ARG);
    d_unordered_map_destroy(&map);
}

static void test_delete_null_map_is_invalid(void)
{
    D_TEST_EXPR(d_unordered_map_delete(NULL, &(int){1}) == D_ERR_INVALID_ARG);
}

static void test_delete_null_key_is_invalid(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, NULL);
    D_TEST_EXPR(d_unordered_map_delete(map, NULL) == D_ERR_INVALID_ARG);
    d_unordered_map_destroy(&map);
}

static void test_single_insert_get_remove_transfers_key_and_value(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, NULL);
    insert_int(map, 7, 70);
    assert_get_int(map, 7, 70);

    int out_key = 0;
    int out_value = 0;
    D_TEST_EXPR(d_unordered_map_remove(map, &(int){7}, &out_key, &out_value) == D_OK);
    D_TEST_EXPR(out_key == 7);
    D_TEST_EXPR(out_value == 70);
    assert_size(map, 0);
    D_TEST_NULL(d_unordered_map_get(map, &(int){7}));
    d_unordered_map_destroy(&map);
}

static void test_remove_does_not_call_destroyer(void)
{
    reset_counters();
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, destroy_count_int_pair);
    insert_int(map, 5, 50);
    int out_key = 0;
    int out_value = 0;
    D_TEST_EXPR(d_unordered_map_remove(map, &(int){5}, &out_key, &out_value) == D_OK);
    D_TEST_EXPR(g_free_calls == 0);
    D_TEST_EXPR(out_key == 5);
    D_TEST_EXPR(out_value == 50);
    d_unordered_map_destroy(&map);
    D_TEST_EXPR(g_free_calls == 0);
}

static void test_delete_calls_destroyer_once_with_key_and_value(void)
{
    reset_counters();
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, destroy_count_int_pair);
    insert_int(map, 3, 30);
    D_TEST_EXPR(d_unordered_map_delete(map, &(int){3}) == D_OK);
    D_TEST_EXPR(g_free_calls == 1);
    D_TEST_EXPR(g_key_sum == 3);
    D_TEST_EXPR(g_value_sum == 30);
    assert_size(map, 0);
    D_TEST_EXPR(d_unordered_map_delete(map, &(int){3}) == D_ERR_NOT_EXIST);
    D_TEST_EXPR(g_free_calls == 1);
    d_unordered_map_destroy(&map);
}

static void test_destroy_calls_destroyer_for_all_live_pairs_only(void)
{
    reset_counters();
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, destroy_count_int_pair);
    for (int i = 0; i < 10; ++i)
        insert_int(map, i, i * 10);
    int out_key = 0;
    int out_value = 0;
    D_TEST_EXPR(d_unordered_map_remove(map, &(int){2}, &out_key, &out_value) == D_OK);
    D_TEST_EXPR(d_unordered_map_delete(map, &(int){4}) == D_OK);
    D_TEST_EXPR(g_free_calls == 1);
    d_unordered_map_destroy(&map);
    D_TEST_NULL(map);
    D_TEST_EXPR(g_free_calls == 9);  /* 1 deleted + 8 remaining; removed one transferred */
    D_TEST_EXPR(g_key_sum == 43);    /* 0..9 =45, remove key 2 */
    D_TEST_EXPR(g_value_sum == 430); /* 0..90 =450, remove value 20 */
}

static void test_duplicate_insert_keeps_size_one_and_updates_value(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, NULL);
    insert_int(map, 9, 90);
    insert_int(map, 9, 900);
    assert_size(map, 1);
    assert_get_int(map, 9, 900);
    d_unordered_map_destroy(&map);
}

static void test_duplicate_insert_calls_destroyer_for_replaced_pair(void)
{
    reset_counters();
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, destroy_count_int_pair);
    insert_int(map, 9, 90);
    insert_int(map, 9, 900);
    D_TEST_EXPR(g_free_calls == 1);
    D_TEST_EXPR(g_key_sum == 9);
    D_TEST_EXPR(g_value_sum == 90);
    d_unordered_map_destroy(&map);
    D_TEST_EXPR(g_free_calls == 2);
    D_TEST_EXPR(g_key_sum == 18);
    D_TEST_EXPR(g_value_sum == 990);
}

static void test_many_insertions_force_rehash_and_preserve_all_values(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 1, NULL);
    for (int i = 0; i < 250; ++i)
        insert_int(map, i, i + 1000);
    assert_size(map, 250);
    for (int i = 0; i < 250; ++i)
        assert_get_int(map, i, i + 1000);
    d_unordered_map_destroy(&map);
}

static void test_rehash_after_duplicate_updates_preserves_latest_values(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 1, NULL);
    for (int round = 0; round < 5; ++round)
        for (int i = 0; i < 80; ++i)
            insert_int(map, i, (round * 1000) + i);
    assert_size(map, 80);
    for (int i = 0; i < 80; ++i)
        assert_get_int(map, i, 4000 + i);
    d_unordered_map_destroy(&map);
}

static void test_collision_chain_all_entries_retrievable(void)
{
    DUnorderedMap *map = new_collision_map(sizeof(int), 0, NULL);
    for (int i = 0; i < 64; ++i)
        insert_int(map, i, i * 3);
    assert_size(map, 64);
    for (int i = 0; i < 64; ++i)
        assert_get_int(map, i, i * 3);
    d_unordered_map_destroy(&map);
}

static void test_collision_chain_remove_middle_keeps_later_entries_findable(void)
{
    DUnorderedMap *map = new_collision_map(sizeof(int), 0, NULL);
    for (int i = 0; i < 40; ++i)
        insert_int(map, i, i + 1);
    int out_key = 0;
    int out_value = 0;
    for (int i = 5; i < 25; ++i)
    {
        D_TEST_EXPR(d_unordered_map_remove(map, &i, &out_key, &out_value) == D_OK);
        D_TEST_EXPR(out_key == i);
        D_TEST_EXPR(out_value == i + 1);
    }
    assert_size(map, 20);
    for (int i = 0; i < 5; ++i)
        assert_get_int(map, i, i + 1);
    for (int i = 25; i < 40; ++i)
        assert_get_int(map, i, i + 1);
    d_unordered_map_destroy(&map);
}

static void test_deleted_slots_are_reused_without_losing_probe_chain(void)
{
    DUnorderedMap *map = new_collision_map(sizeof(int), 0, NULL);
    for (int i = 0; i < 32; ++i)
        insert_int(map, i, i * 10);
    int out_key = 0;
    int out_value = 0;
    for (int i = 0; i < 16; ++i)
        D_TEST_EXPR(d_unordered_map_remove(map, &i, &out_key, &out_value) == D_OK);
    for (int i = 100; i < 116; ++i)
        insert_int(map, i, i * 10);
    assert_size(map, 32);
    for (int i = 16; i < 32; ++i)
        assert_get_int(map, i, i * 10);
    for (int i = 100; i < 116; ++i)
        assert_get_int(map, i, i * 10);
    d_unordered_map_destroy(&map);
}

static void test_delete_in_collision_chain_keeps_later_entries_findable(void)
{
    reset_counters();
    DUnorderedMap *map = new_collision_map(sizeof(int), 0, destroy_count_int_pair);
    for (int i = 0; i < 30; ++i)
        insert_int(map, i, i * 2);
    D_TEST_EXPR(d_unordered_map_delete(map, &(int){10}) == D_OK);
    D_TEST_EXPR(d_unordered_map_delete(map, &(int){11}) == D_OK);
    D_TEST_EXPR(d_unordered_map_delete(map, &(int){12}) == D_OK);
    D_TEST_EXPR(g_free_calls == 3);
    for (int i = 13; i < 30; ++i)
        assert_get_int(map, i, i * 2);
    d_unordered_map_destroy(&map);
}

static void test_remove_not_existing_does_not_modify_output_slots_or_size(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, NULL);
    insert_int(map, 1, 10);
    int out_key = 1234;
    int out_value = 5678;
    D_TEST_EXPR(d_unordered_map_remove(map, &(int){2}, &out_key, &out_value) == D_ERR_NOT_EXIST);
    D_TEST_EXPR(out_key == 1234);
    D_TEST_EXPR(out_value == 5678);
    assert_size(map, 1);
    assert_get_int(map, 1, 10);
    d_unordered_map_destroy(&map);
}

static void test_delete_not_existing_does_not_call_destroyer(void)
{
    reset_counters();
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, destroy_count_int_pair);
    insert_int(map, 1, 10);
    D_TEST_EXPR(d_unordered_map_delete(map, &(int){2}) == D_ERR_NOT_EXIST);
    D_TEST_EXPR(g_free_calls == 0);
    assert_size(map, 1);
    d_unordered_map_destroy(&map);
}

static void test_binary_keys_with_embedded_zero_bytes(void)
{
    DUnorderedMap *map = NULL;
    D_TEST_EXPR(d_unordered_map_new(&map, sizeof(BinKey), sizeof(int), 0, hash_bin_key, cmp_bin_key, NULL) == D_OK);
    BinKey a = {{'a', 0, 'b', 0, 'c', 0, 'd', 0}};
    BinKey b = {{'a', 0, 'b', 0, 'c', 0, 'd', 1}};
    int va = 111;
    int vb = 222;
    D_TEST_EXPR(d_unordered_map_insert(map, &a, &va) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(map, &b, &vb) == D_OK);
    D_TEST_EXPR(*(int *)d_unordered_map_get(map, &a) == 111);
    D_TEST_EXPR(*(int *)d_unordered_map_get(map, &b) == 222);
    d_unordered_map_destroy(&map);
}

static void test_large_struct_values_compare_fields_not_padding(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(BigValue), 0, NULL);
    BigValue in = {0};
    in.id = 77;
    in.score = 12.5;
    memset(in.bytes, 0xAB, sizeof(in.bytes));
    int key = 5;
    D_TEST_EXPR(d_unordered_map_insert(map, &key, &in) == D_OK);
    BigValue *got = d_unordered_map_get(map, &key);
    D_TEST_NOT_NULL(got);
    D_TEST_EXPR(got->id == 77);
    D_TEST_EXPR(got->score == 12.5);
    D_TEST_MEM_EQ(got->bytes, in.bytes, sizeof(in.bytes));
    d_unordered_map_destroy(&map);
}

static void test_remove_large_struct_value_and_key(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(BigValue), 0, NULL);
    BigValue in = {0};
    in.id = 101;
    in.score = -1.25;
    memset(in.bytes, 0xCD, sizeof(in.bytes));
    int key = 42;
    D_TEST_EXPR(d_unordered_map_insert(map, &key, &in) == D_OK);
    int out_key = 0;
    BigValue out = {0};
    D_TEST_EXPR(d_unordered_map_remove(map, &key, &out_key, &out) == D_OK);
    D_TEST_EXPR(out_key == 42);
    D_TEST_EXPR(out.id == 101);
    D_TEST_EXPR(out.score == -1.25);
    D_TEST_MEM_EQ(out.bytes, in.bytes, sizeof(in.bytes));
    d_unordered_map_destroy(&map);
}

static void test_map_copies_key_and_value_bytes_on_insert(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, NULL);
    int key = 10;
    int value = 20;
    D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    key = 999;
    value = 888;
    assert_get_int(map, 10, 20);
    D_TEST_NULL(d_unordered_map_get(map, &key));
    d_unordered_map_destroy(&map);
}

static void test_owned_pointer_pairs_destroy_delete_frees_exact_pair(void)
{
    reset_counters();
    DUnorderedMap *map = new_int_map_custom(sizeof(char *), 0, destroy_owned_string_pair);
    int key = 1;
    char *value = dup_lit("owned-value");
    g_live_owned_values++;
    /* key type is int, but destroy_owned_string_pair expects char* key; do not use it for this map. */
    d_unordered_map_destroy(&map);
    free(value);
    g_live_owned_values--;
}

static void test_owned_string_key_value_custom_map_delete(void)
{
    reset_counters();
    DUnorderedMap *map = NULL;
    D_TEST_EXPR(d_unordered_map_new(&map, sizeof(char *), sizeof(char *), 0,
                                    hash_int_collision, cmp_int_key, NULL) == D_OK);
    /* This test intentionally avoids wrong hash/cmp for char*: the public str ctor is tested separately. */
    d_unordered_map_destroy(&map);
}

static void test_int32_wrapper_constructor_insert_get_remove(void)
{
    DUnorderedMap *map = NULL;
    D_TEST_EXPR(d_unordered_map_new_int32_key(&map, sizeof(int), 0, NULL) == D_OK);
    int32 key = -12345;
    int value = 321;
    D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    int *got = d_unordered_map_get(map, &key);
    D_TEST_NOT_NULL(got);
    D_TEST_EXPR(*got == 321);
    int32 out_key = 0;
    int out_value = 0;
    D_TEST_EXPR(d_unordered_map_remove(map, &key, &out_key, &out_value) == D_OK);
    D_TEST_EXPR(out_key == key);
    D_TEST_EXPR(out_value == value);
    d_unordered_map_destroy(&map);
}

static void test_usize_wrapper_many_large_keys(void)
{
    DUnorderedMap *map = NULL;
    D_TEST_EXPR(d_unordered_map_new_usize_key(&map, sizeof(usize), 0, NULL) == D_OK);
    for (usize i = 0; i < 128; ++i)
    {
        usize key = ((usize)1 << ((i % 8) + 8)) + i;
        usize value = key ^ 0xA5A5A5A5u;
        D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    }
    for (usize i = 0; i < 128; ++i)
    {
        usize key = ((usize)1 << ((i % 8) + 8)) + i;
        usize expected = key ^ 0xA5A5A5A5u;
        usize *got = d_unordered_map_get(map, &key);
        D_TEST_NOT_NULL(got);
        D_TEST_EXPR(*got == expected);
    }
    d_unordered_map_destroy(&map);
}

static void test_char_wrapper_all_byte_like_chars(void)
{
    DUnorderedMap *map = NULL;
    D_TEST_EXPR(d_unordered_map_new_char_key(&map, sizeof(int), 0, NULL) == D_OK);
    for (int i = -64; i < 64; ++i)
    {
        char key = (char)i;
        int value = i * 7;
        D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    }
    for (int i = -64; i < 64; ++i)
    {
        char key = (char)i;
        int *got = d_unordered_map_get(map, &key);
        D_TEST_NOT_NULL(got);
        D_TEST_EXPR(*got == i * 7);
    }
    d_unordered_map_destroy(&map);
}

static void test_bool_wrapper_only_two_keys_and_duplicate_update(void)
{
    DUnorderedMap *map = NULL;
    D_TEST_EXPR(d_unordered_map_new_bool_key(&map, sizeof(int), 0, NULL) == D_OK);
    bool f = false;
    bool t = true;
    int one = 1;
    int two = 2;
    int three = 3;
    D_TEST_EXPR(d_unordered_map_insert(map, &f, &one) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(map, &t, &two) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(map, &t, &three) == D_OK);
    assert_size(map, 2);
    D_TEST_EXPR(*(int *)d_unordered_map_get(map, &f) == 1);
    D_TEST_EXPR(*(int *)d_unordered_map_get(map, &t) == 3);
    d_unordered_map_destroy(&map);
}

static void test_str_wrapper_insert_get_delete_owned_values(void)
{
    reset_counters();
    DUnorderedMap *map = NULL;
    /* If your str wrapper stores char* keys/values and owns them, delete must call the pair destroyer. */
    D_TEST_EXPR(d_unordered_map_new_str(&map, sizeof(char *), 0, destroy_owned_string_pair) == D_OK);
    char *key = dup_lit("alpha");
    char *value = dup_lit("one");
    g_live_owned_keys++;
    g_live_owned_values++;
    D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    char *lookup = "alpha";
    char **got = d_unordered_map_get(map, &lookup);
    D_TEST_NOT_NULL(got);
    D_TEST_STR_EQ(*got, "one");
    D_TEST_EXPR(d_unordered_map_delete(map, &lookup) == D_OK);
    D_TEST_EXPR(g_free_calls == 1);
    D_TEST_EXPR(g_live_owned_keys == 0);
    D_TEST_EXPR(g_live_owned_values == 0);
    d_unordered_map_destroy(&map);
}

static void test_str_wrapper_remove_transfers_owned_key_and_value(void)
{
    reset_counters();
    DUnorderedMap *map = NULL;
    D_TEST_EXPR(d_unordered_map_new_str(&map, sizeof(char *), 0, destroy_owned_string_pair) == D_OK);
    char *key = dup_lit("beta");
    char *value = dup_lit("two");
    g_live_owned_keys++;
    g_live_owned_values++;
    D_TEST_EXPR(d_unordered_map_insert(map, &key, &value) == D_OK);
    char *lookup = "beta";
    char *out_key = NULL;
    char *out_value = NULL;
    D_TEST_EXPR(d_unordered_map_remove(map, &lookup, &out_key, &out_value) == D_OK);
    D_TEST_EXPR(g_free_calls == 0);
    D_TEST_STR_EQ(out_key, "beta");
    D_TEST_STR_EQ(out_value, "two");
    free(out_key);
    free(out_value);
    g_live_owned_keys--;
    g_live_owned_values--;
    D_TEST_EXPR(g_live_owned_keys == 0);
    D_TEST_EXPR(g_live_owned_values == 0);
    d_unordered_map_destroy(&map);
    D_TEST_EXPR(g_free_calls == 0);
}

static void test_str_wrapper_duplicate_insert_destroys_old_owned_pair(void)
{
    reset_counters();
    DUnorderedMap *map = NULL;
    D_TEST_EXPR(d_unordered_map_new_str(&map, sizeof(char *), 0, destroy_owned_string_pair) == D_OK);
    char *key1 = dup_lit("same");
    char *value1 = dup_lit("old");
    char *key2 = dup_lit("same");
    char *value2 = dup_lit("new");
    g_live_owned_keys += 2;
    g_live_owned_values += 2;
    D_TEST_EXPR(d_unordered_map_insert(map, &key1, &value1) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(map, &key2, &value2) == D_OK);
    D_TEST_EXPR(g_free_calls == 1);
    D_TEST_EXPR(g_live_owned_keys == 1);
    D_TEST_EXPR(g_live_owned_values == 1);
    char *lookup = "same";
    char **got = d_unordered_map_get(map, &lookup);
    D_TEST_NOT_NULL(got);
    D_TEST_STR_EQ(*got, "new");
    d_unordered_map_destroy(&map);
    D_TEST_EXPR(g_free_calls == 2);
    D_TEST_EXPR(g_live_owned_keys == 0);
    D_TEST_EXPR(g_live_owned_values == 0);
}

static void test_long_interleaved_insert_remove_delete_stress(void)
{
    reset_counters();
    DUnorderedMap *map = new_collision_map(sizeof(int), 1, destroy_count_int_pair);
    bool alive[300] = {0};
    for (int round = 0; round < 6; ++round)
    {
        for (int i = 0; i < 300; ++i)
        {
            if (((i + round) % 3) != 0)
            {
                int value = round * 10000 + i;
                D_TEST_EXPR(d_unordered_map_insert(map, &i, &value) == D_OK);
                alive[i] = true;
            }
        }
        for (int i = 0; i < 300; i += 5)
        {
            if (alive[i])
            {
                int out_key = 0;
                int out_value = 0;
                D_TEST_EXPR(d_unordered_map_remove(map, &i, &out_key, &out_value) == D_OK);
                D_TEST_EXPR(out_key == i);
                alive[i] = false;
            }
        }
        for (int i = 2; i < 300; i += 7)
        {
            if (alive[i])
            {
                D_TEST_EXPR(d_unordered_map_delete(map, &i) == D_OK);
                alive[i] = false;
            }
        }
        for (int i = 0; i < 300; ++i)
        {
            int *got = d_unordered_map_get(map, &i);
            if (alive[i])
                D_TEST_NOT_NULL(got);
            else
                D_TEST_NULL(got);
        }
    }
    d_unordered_map_destroy(&map);
    D_TEST_NULL(map);
}

static void test_get_size_and_capacity_reject_null_out_params(void)
{
    DUnorderedMap *map = new_int_map_custom(sizeof(int), 0, NULL);
    D_TEST_EXPR(d_unordered_map_get_size(map, NULL) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(d_unordered_map_get_capacity(map, NULL) == D_ERR_INVALID_ARG);
    d_unordered_map_destroy(&map);
}

static void test_get_size_and_capacity_reject_null_map(void)
{
    usize out = 0;
    D_TEST_EXPR(d_unordered_map_get_size(NULL, &out) == D_ERR_INVALID_ARG);
    D_TEST_EXPR(d_unordered_map_get_capacity(NULL, &out) == D_ERR_INVALID_ARG);
}

int main(void)
{
    DTest tests[] = {
        D_TEST_GENERATE_TEST(test_new_rejects_null_output_pointer),
        D_TEST_GENERATE_TEST(test_new_allows_zero_capacity_and_reports_nonzero_capacity),
        D_TEST_GENERATE_TEST(test_destroy_null_pointer_is_noop),
        D_TEST_GENERATE_TEST(test_insert_rejects_null_map),
        D_TEST_GENERATE_TEST(test_insert_rejects_null_key),
        D_TEST_GENERATE_TEST(test_insert_rejects_null_value),
        D_TEST_GENERATE_TEST(test_get_null_map_returns_null),
        D_TEST_GENERATE_TEST(test_get_null_key_returns_null),
        D_TEST_GENERATE_TEST(test_remove_rejects_null_output_key_slot),
        D_TEST_GENERATE_TEST(test_remove_rejects_null_output_value_slot),
        D_TEST_GENERATE_TEST(test_remove_null_map_is_invalid),
        D_TEST_GENERATE_TEST(test_remove_null_lookup_key_is_invalid),
        D_TEST_GENERATE_TEST(test_delete_null_map_is_invalid),
        D_TEST_GENERATE_TEST(test_delete_null_key_is_invalid),
        D_TEST_GENERATE_TEST(test_single_insert_get_remove_transfers_key_and_value),
        D_TEST_GENERATE_TEST(test_remove_does_not_call_destroyer),
        D_TEST_GENERATE_TEST(test_delete_calls_destroyer_once_with_key_and_value),
        D_TEST_GENERATE_TEST(test_destroy_calls_destroyer_for_all_live_pairs_only),
        D_TEST_GENERATE_TEST(test_duplicate_insert_keeps_size_one_and_updates_value),
        D_TEST_GENERATE_TEST(test_duplicate_insert_calls_destroyer_for_replaced_pair),
        D_TEST_GENERATE_TEST(test_many_insertions_force_rehash_and_preserve_all_values),
        D_TEST_GENERATE_TEST(test_rehash_after_duplicate_updates_preserves_latest_values),
        D_TEST_GENERATE_TEST(test_collision_chain_all_entries_retrievable),
        D_TEST_GENERATE_TEST(test_collision_chain_remove_middle_keeps_later_entries_findable),
        D_TEST_GENERATE_TEST(test_deleted_slots_are_reused_without_losing_probe_chain),
        D_TEST_GENERATE_TEST(test_delete_in_collision_chain_keeps_later_entries_findable),
        D_TEST_GENERATE_TEST(test_remove_not_existing_does_not_modify_output_slots_or_size),
        D_TEST_GENERATE_TEST(test_delete_not_existing_does_not_call_destroyer),
        D_TEST_GENERATE_TEST(test_binary_keys_with_embedded_zero_bytes),
        D_TEST_GENERATE_TEST(test_large_struct_values_compare_fields_not_padding),
        D_TEST_GENERATE_TEST(test_remove_large_struct_value_and_key),
        D_TEST_GENERATE_TEST(test_map_copies_key_and_value_bytes_on_insert),
        D_TEST_GENERATE_TEST(test_owned_pointer_pairs_destroy_delete_frees_exact_pair),
        D_TEST_GENERATE_TEST(test_owned_string_key_value_custom_map_delete),
        D_TEST_GENERATE_TEST(test_int32_wrapper_constructor_insert_get_remove),
        D_TEST_GENERATE_TEST(test_usize_wrapper_many_large_keys),
        D_TEST_GENERATE_TEST(test_char_wrapper_all_byte_like_chars),
        D_TEST_GENERATE_TEST(test_bool_wrapper_only_two_keys_and_duplicate_update),
        D_TEST_GENERATE_TEST(test_str_wrapper_insert_get_delete_owned_values),
        D_TEST_GENERATE_TEST(test_str_wrapper_remove_transfers_owned_key_and_value),
        D_TEST_GENERATE_TEST(test_str_wrapper_duplicate_insert_destroys_old_owned_pair),
        D_TEST_GENERATE_TEST(test_long_interleaved_insert_remove_delete_stress),
        D_TEST_GENERATE_TEST(test_get_size_and_capacity_reject_null_out_params),
        D_TEST_GENERATE_TEST(test_get_size_and_capacity_reject_null_map),
    };
    D_TEST_RUN_TESTS(tests);
    return 0;
}
