#include "raw_map.h"
#include "d_unordered_map.h"
#include "d_test.h"
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

static usize local_hash_str(void *key)
{
    const char *s = *(const char **)key;
    usize h = 5381;
    while (*s)
        h = h * 33 + (unsigned char)*s++;
    return h;
}

static bool local_cmp_str(void *a, void *b)
{
    return strcmp(*(const char **)a, *(const char **)b) == 0;
}

/* Single-arg key destructor: counts entries freed and accumulates key sum */
static void count_key_destroy_int(void *elem)
{
    g_free_calls++;
    g_key_sum += *(int *)elem;
}

/* Single-arg value destructor: accumulates value sum */
static void count_value_destroy_int(void *elem)
{
    g_value_sum += *(int *)elem;
}

static void free_owned_str_key(void *elem)
{
    char *k = *(char **)elem;
    if (k)
    {
        g_live_owned_keys--;
        free(k);
    }
    g_free_calls++;
}

static void free_owned_str_value(void *elem)
{
    char *v = *(char **)elem;
    if (v)
    {
        g_live_owned_values--;
        free(v);
    }
}

static char *dup_lit(const char *s)
{
    usize len = strlen(s) + 1;
    char *out = malloc(len);
    D_TEST_NOT_NULL(out);
    memcpy(out, s, len);
    return out;
}

static void init_int_map(DUnorderedMap *map, usize value_size, usize capacity, DestroyElemFn key_fn, DestroyElemFn value_fn)
{
    D_TEST_EXPR(d_unordered_map_init(map, sizeof(int), value_size, capacity, hash_int_key, cmp_int_key, key_fn, value_fn) == D_OK);
}

static void init_collision_map(DUnorderedMap *map, usize value_size, usize capacity, DestroyElemFn key_fn, DestroyElemFn value_fn)
{
    D_TEST_EXPR(d_unordered_map_init(map, sizeof(int), value_size, capacity, hash_int_collision, cmp_int_key, key_fn, value_fn) == D_OK);
}

static void assert_size(DUnorderedMap *map, usize expected)
{
    D_TEST_EXPR(d_unordered_map_get_size(map) == expected);
}

static void assert_capacity_at_least(DUnorderedMap *map, usize expected_min)
{
    D_TEST_EXPR(d_unordered_map_get_capacity(map) >= expected_min);
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

static void test_init_allows_zero_capacity_and_reports_nonzero_capacity(void)
{
    DUnorderedMap map;

    init_int_map(&map, sizeof(int), 0, NULL, NULL);
    assert_size(&map, 0);
    assert_capacity_at_least(&map, 16);
    d_unordered_map_destroy(&map);
}

static void test_destroy_null_pointer_is_noop(void)
{
    d_unordered_map_destroy(NULL);
    D_TEST_EXPR(true);
}

static void test_destroy_empty_map_is_safe(void)
{
    DUnorderedMap map;

    init_int_map(&map, sizeof(int), 0, NULL, NULL);
    d_unordered_map_destroy(&map);
    D_TEST_EXPR(true);
}

static void test_single_insert_get_remove_transfers_key_and_value(void)
{
    DUnorderedMap map;
    int out_key = 0;
    int out_value = 0;

    init_int_map(&map, sizeof(int), 0, NULL, NULL);
    insert_int(&map, 7, 70);
    assert_get_int(&map, 7, 70);
    D_TEST_EXPR(d_unordered_map_remove(&map, &(int){7}, &out_key, &out_value) == D_OK);
    D_TEST_EXPR(out_key == 7);
    D_TEST_EXPR(out_value == 70);
    assert_size(&map, 0);
    D_TEST_NULL(d_unordered_map_get(&map, &(int){7}));
    d_unordered_map_destroy(&map);
}

static void test_remove_does_not_call_destroyer(void)
{
    DUnorderedMap map;
    int out_key = 0;
    int out_value = 0;

    reset_counters();
    init_int_map(&map, sizeof(int), 0, count_key_destroy_int, count_value_destroy_int);
    insert_int(&map, 5, 50);
    D_TEST_EXPR(d_unordered_map_remove(&map, &(int){5}, &out_key, &out_value) == D_OK);
    D_TEST_EXPR(g_free_calls == 0);
    D_TEST_EXPR(out_key == 5);
    D_TEST_EXPR(out_value == 50);
    d_unordered_map_destroy(&map);
    D_TEST_EXPR(g_free_calls == 0);
}

static void test_delete_calls_destroyer_once_with_key_and_value(void)
{
    DUnorderedMap map;

    reset_counters();
    init_int_map(&map, sizeof(int), 0, count_key_destroy_int, count_value_destroy_int);
    insert_int(&map, 3, 30);
    D_TEST_EXPR(d_unordered_map_delete(&map, &(int){3}) == D_OK);
    D_TEST_EXPR(g_free_calls == 1);
    D_TEST_EXPR(g_key_sum == 3);
    D_TEST_EXPR(g_value_sum == 30);
    assert_size(&map, 0);
    D_TEST_EXPR(d_unordered_map_delete(&map, &(int){3}) == D_ERR_NOT_EXIST);
    D_TEST_EXPR(g_free_calls == 1);
    d_unordered_map_destroy(&map);
}

static void test_destroy_calls_destroyer_for_all_live_pairs_only(void)
{
    DUnorderedMap map;
    int out_key = 0;
    int out_value = 0;

    reset_counters();
    init_int_map(&map, sizeof(int), 0, count_key_destroy_int, count_value_destroy_int);
    for (int i = 0; i < 10; ++i)
        insert_int(&map, i, i * 10);
    D_TEST_EXPR(d_unordered_map_remove(&map, &(int){2}, &out_key, &out_value) == D_OK);
    D_TEST_EXPR(d_unordered_map_delete(&map, &(int){4}) == D_OK);
    D_TEST_EXPR(g_free_calls == 1);
    d_unordered_map_destroy(&map);
    D_TEST_EXPR(g_free_calls == 9);  /* 1 deleted + 8 remaining; removed one transferred */
    D_TEST_EXPR(g_key_sum == 43);    /* 0..9 =45, minus removed key 2 */
    D_TEST_EXPR(g_value_sum == 430); /* 0..90 =450, minus removed value 20 */
}

static void test_duplicate_insert_keeps_size_one_and_updates_value(void)
{
    DUnorderedMap map;

    init_int_map(&map, sizeof(int), 0, NULL, NULL);
    insert_int(&map, 9, 90);
    insert_int(&map, 9, 900);
    assert_size(&map, 1);
    assert_get_int(&map, 9, 900);
    d_unordered_map_destroy(&map);
}

static void test_duplicate_insert_calls_destroyer_for_replaced_pair(void)
{
    DUnorderedMap map;

    reset_counters();
    init_int_map(&map, sizeof(int), 0, count_key_destroy_int, count_value_destroy_int);
    insert_int(&map, 9, 90);
    insert_int(&map, 9, 900);
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
    DUnorderedMap map;

    init_int_map(&map, sizeof(int), 1, NULL, NULL);
    for (int i = 0; i < 250; ++i)
        insert_int(&map, i, i + 1000);
    assert_size(&map, 250);
    for (int i = 0; i < 250; ++i)
        assert_get_int(&map, i, i + 1000);
    d_unordered_map_destroy(&map);
}

static void test_rehash_after_duplicate_updates_preserves_latest_values(void)
{
    DUnorderedMap map;

    init_int_map(&map, sizeof(int), 1, NULL, NULL);
    for (int round = 0; round < 5; ++round)
        for (int i = 0; i < 80; ++i)
            insert_int(&map, i, (round * 1000) + i);
    assert_size(&map, 80);
    for (int i = 0; i < 80; ++i)
        assert_get_int(&map, i, 4000 + i);
    d_unordered_map_destroy(&map);
}

static void test_collision_chain_all_entries_retrievable(void)
{
    DUnorderedMap map;

    init_collision_map(&map, sizeof(int), 0, NULL, NULL);
    for (int i = 0; i < 64; ++i)
        insert_int(&map, i, i * 3);
    assert_size(&map, 64);
    for (int i = 0; i < 64; ++i)
        assert_get_int(&map, i, i * 3);
    d_unordered_map_destroy(&map);
}

static void test_collision_chain_remove_middle_keeps_later_entries_findable(void)
{
    DUnorderedMap map;
    int out_key = 0;
    int out_value = 0;

    init_collision_map(&map, sizeof(int), 0, NULL, NULL);
    for (int i = 0; i < 40; ++i)
        insert_int(&map, i, i + 1);
    for (int i = 5; i < 25; ++i)
    {
        D_TEST_EXPR(d_unordered_map_remove(&map, &i, &out_key, &out_value) == D_OK);
        D_TEST_EXPR(out_key == i);
        D_TEST_EXPR(out_value == i + 1);
    }
    assert_size(&map, 20);
    for (int i = 0; i < 5; ++i)
        assert_get_int(&map, i, i + 1);
    for (int i = 25; i < 40; ++i)
        assert_get_int(&map, i, i + 1);
    d_unordered_map_destroy(&map);
}

static void test_deleted_slots_are_reused_without_losing_probe_chain(void)
{
    DUnorderedMap map;
    int out_key = 0;
    int out_value = 0;

    init_collision_map(&map, sizeof(int), 0, NULL, NULL);
    for (int i = 0; i < 32; ++i)
        insert_int(&map, i, i * 10);
    for (int i = 0; i < 16; ++i)
        D_TEST_EXPR(d_unordered_map_remove(&map, &i, &out_key, &out_value) == D_OK);
    for (int i = 100; i < 116; ++i)
        insert_int(&map, i, i * 10);
    assert_size(&map, 32);
    for (int i = 16; i < 32; ++i)
        assert_get_int(&map, i, i * 10);
    for (int i = 100; i < 116; ++i)
        assert_get_int(&map, i, i * 10);
    d_unordered_map_destroy(&map);
}

static void test_delete_in_collision_chain_keeps_later_entries_findable(void)
{
    DUnorderedMap map;

    reset_counters();
    init_collision_map(&map, sizeof(int), 0, count_key_destroy_int, count_value_destroy_int);
    for (int i = 0; i < 30; ++i)
        insert_int(&map, i, i * 2);
    D_TEST_EXPR(d_unordered_map_delete(&map, &(int){10}) == D_OK);
    D_TEST_EXPR(d_unordered_map_delete(&map, &(int){11}) == D_OK);
    D_TEST_EXPR(d_unordered_map_delete(&map, &(int){12}) == D_OK);
    D_TEST_EXPR(g_free_calls == 3);
    for (int i = 13; i < 30; ++i)
        assert_get_int(&map, i, i * 2);
    d_unordered_map_destroy(&map);
}

static void test_remove_not_existing_does_not_modify_output_slots_or_size(void)
{
    DUnorderedMap map;
    int out_key = 1234;
    int out_value = 5678;

    init_int_map(&map, sizeof(int), 0, NULL, NULL);
    insert_int(&map, 1, 10);
    D_TEST_EXPR(d_unordered_map_remove(&map, &(int){2}, &out_key, &out_value) == D_ERR_NOT_EXIST);
    D_TEST_EXPR(out_key == 1234);
    D_TEST_EXPR(out_value == 5678);
    assert_size(&map, 1);
    assert_get_int(&map, 1, 10);
    d_unordered_map_destroy(&map);
}

static void test_delete_not_existing_does_not_call_destroyer(void)
{
    DUnorderedMap map;

    reset_counters();
    init_int_map(&map, sizeof(int), 0, count_key_destroy_int, count_value_destroy_int);
    insert_int(&map, 1, 10);
    D_TEST_EXPR(d_unordered_map_delete(&map, &(int){2}) == D_ERR_NOT_EXIST);
    D_TEST_EXPR(g_free_calls == 0);
    assert_size(&map, 1);
    d_unordered_map_destroy(&map);
}

static void test_binary_keys_with_embedded_zero_bytes(void)
{
    DUnorderedMap map;
    BinKey a = {{'a', 0, 'b', 0, 'c', 0, 'd', 0}};
    BinKey b = {{'a', 0, 'b', 0, 'c', 0, 'd', 1}};
    int va = 111;
    int vb = 222;

    D_TEST_EXPR(d_unordered_map_init(&map, sizeof(BinKey), sizeof(int), 0, hash_bin_key, cmp_bin_key, NULL, NULL) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(&map, &a, &va) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(&map, &b, &vb) == D_OK);
    D_TEST_EXPR(*(int *)d_unordered_map_get(&map, &a) == 111);
    D_TEST_EXPR(*(int *)d_unordered_map_get(&map, &b) == 222);
    d_unordered_map_destroy(&map);
}

static void test_large_struct_values_compare_fields_not_padding(void)
{
    DUnorderedMap map;
    BigValue in = {0};
    int key = 5;
    BigValue *got;

    init_int_map(&map, sizeof(BigValue), 0, NULL, NULL);
    in.id = 77;
    in.score = 12.5;
    memset(in.bytes, 0xAB, sizeof(in.bytes));
    D_TEST_EXPR(d_unordered_map_insert(&map, &key, &in) == D_OK);
    got = d_unordered_map_get(&map, &key);
    D_TEST_NOT_NULL(got);
    D_TEST_EXPR(got->id == 77);
    D_TEST_EXPR(got->score == 12.5);
    D_TEST_MEM_EQ(got->bytes, in.bytes, sizeof(in.bytes));
    d_unordered_map_destroy(&map);
}

static void test_remove_large_struct_value_and_key(void)
{
    DUnorderedMap map;
    BigValue in = {0};
    int key = 42;
    int out_key = 0;
    BigValue out = {0};

    init_int_map(&map, sizeof(BigValue), 0, NULL, NULL);
    in.id = 101;
    in.score = -1.25;
    memset(in.bytes, 0xCD, sizeof(in.bytes));
    D_TEST_EXPR(d_unordered_map_insert(&map, &key, &in) == D_OK);
    D_TEST_EXPR(d_unordered_map_remove(&map, &key, &out_key, &out) == D_OK);
    D_TEST_EXPR(out_key == 42);
    D_TEST_EXPR(out.id == 101);
    D_TEST_EXPR(out.score == -1.25);
    D_TEST_MEM_EQ(out.bytes, in.bytes, sizeof(in.bytes));
    d_unordered_map_destroy(&map);
}

static void test_map_copies_key_and_value_bytes_on_insert(void)
{
    DUnorderedMap map;
    int key = 10;
    int value = 20;

    init_int_map(&map, sizeof(int), 0, NULL, NULL);
    D_TEST_EXPR(d_unordered_map_insert(&map, &key, &value) == D_OK);
    key = 999;
    value = 888;
    assert_get_int(&map, 10, 20);
    D_TEST_NULL(d_unordered_map_get(&map, &key));
    d_unordered_map_destroy(&map);
}

static void test_int32_wrapper_constructor_insert_get_remove(void)
{
    DUnorderedMap map;
    int32 key = -12345;
    int value = 321;
    int32 out_key = 0;
    int out_value = 0;

    D_TEST_EXPR(d_unordered_map_init_not_owned_int32_key(&map, sizeof(int), 0, NULL) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(&map, &key, &value) == D_OK);
    int *got = d_unordered_map_get(&map, &key);
    D_TEST_NOT_NULL(got);
    D_TEST_EXPR(*got == 321);
    D_TEST_EXPR(d_unordered_map_remove(&map, &key, &out_key, &out_value) == D_OK);
    D_TEST_EXPR(out_key == key);
    D_TEST_EXPR(out_value == value);
    d_unordered_map_destroy(&map);
}

static void test_usize_wrapper_many_large_keys(void)
{
    DUnorderedMap map;

    D_TEST_EXPR(d_unordered_map_init_not_owned_usize_key(&map, sizeof(usize), 0, NULL) == D_OK);
    for (usize i = 0; i < 128; ++i)
    {
        usize key = ((usize)1 << ((i % 8) + 8)) + i;
        usize value = key ^ 0xA5A5A5A5u;
        D_TEST_EXPR(d_unordered_map_insert(&map, &key, &value) == D_OK);
    }
    for (usize i = 0; i < 128; ++i)
    {
        usize key = ((usize)1 << ((i % 8) + 8)) + i;
        usize expected = key ^ 0xA5A5A5A5u;
        usize *got = d_unordered_map_get(&map, &key);
        D_TEST_NOT_NULL(got);
        D_TEST_EXPR(*got == expected);
    }
    d_unordered_map_destroy(&map);
}

static void test_char_wrapper_all_byte_like_chars(void)
{
    DUnorderedMap map;

    D_TEST_EXPR(d_unordered_map_init_not_owned_char_key(&map, sizeof(int), 0, NULL) == D_OK);
    for (int i = -64; i < 64; ++i)
    {
        char key = (char)i;
        int value = i * 7;
        D_TEST_EXPR(d_unordered_map_insert(&map, &key, &value) == D_OK);
    }
    for (int i = -64; i < 64; ++i)
    {
        char key = (char)i;
        int *got = d_unordered_map_get(&map, &key);
        D_TEST_NOT_NULL(got);
        D_TEST_EXPR(*got == i * 7);
    }
    d_unordered_map_destroy(&map);
}

static void test_bool_wrapper_only_two_keys_and_duplicate_update(void)
{
    DUnorderedMap map;
    bool f = false;
    bool t = true;
    int one = 1;
    int two = 2;
    int three = 3;

    D_TEST_EXPR(d_unordered_map_init_not_owned_bool_key(&map, sizeof(int), 0, NULL) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(&map, &f, &one) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(&map, &t, &two) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(&map, &t, &three) == D_OK);
    assert_size(&map, 2);
    D_TEST_EXPR(*(int *)d_unordered_map_get(&map, &f) == 1);
    D_TEST_EXPR(*(int *)d_unordered_map_get(&map, &t) == 3);
    d_unordered_map_destroy(&map);
}

static void test_owned_str_key_value_map_delete(void)
{
    DUnorderedMap map;
    char *key = dup_lit("alpha");
    char *value = dup_lit("one");
    char *lookup = "alpha";
    char **got;

    reset_counters();
    g_live_owned_keys++;
    g_live_owned_values++;
    D_TEST_EXPR(d_unordered_map_init(&map, sizeof(char *), sizeof(char *), 0,
                                     local_hash_str, local_cmp_str,
                                     free_owned_str_key, free_owned_str_value) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(&map, &key, &value) == D_OK);
    got = d_unordered_map_get(&map, &lookup);
    D_TEST_NOT_NULL(got);
    D_TEST_STR_EQ(*got, "one");
    D_TEST_EXPR(d_unordered_map_delete(&map, &lookup) == D_OK);
    D_TEST_EXPR(g_free_calls == 1);
    D_TEST_EXPR(g_live_owned_keys == 0);
    D_TEST_EXPR(g_live_owned_values == 0);
    d_unordered_map_destroy(&map);
}

static void test_owned_str_key_value_map_remove_transfers_ownership(void)
{
    DUnorderedMap map;
    char *key = dup_lit("beta");
    char *value = dup_lit("two");
    char *lookup = "beta";
    char *out_key = NULL;
    char *out_value = NULL;

    reset_counters();
    g_live_owned_keys++;
    g_live_owned_values++;
    D_TEST_EXPR(d_unordered_map_init(&map, sizeof(char *), sizeof(char *), 0,
                                     local_hash_str, local_cmp_str,
                                     free_owned_str_key, free_owned_str_value) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(&map, &key, &value) == D_OK);
    D_TEST_EXPR(d_unordered_map_remove(&map, &lookup, &out_key, &out_value) == D_OK);
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

static void test_owned_str_key_value_duplicate_insert_destroys_old_pair(void)
{
    DUnorderedMap map;
    char *key1 = dup_lit("same");
    char *value1 = dup_lit("old");
    char *key2 = dup_lit("same");
    char *value2 = dup_lit("new");
    char *lookup = "same";
    char **got;

    reset_counters();
    g_live_owned_keys += 2;
    g_live_owned_values += 2;
    D_TEST_EXPR(d_unordered_map_init(&map, sizeof(char *), sizeof(char *), 0,
                                     local_hash_str, local_cmp_str,
                                     free_owned_str_key, free_owned_str_value) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(&map, &key1, &value1) == D_OK);
    D_TEST_EXPR(d_unordered_map_insert(&map, &key2, &value2) == D_OK);
    D_TEST_EXPR(g_free_calls == 1);
    D_TEST_EXPR(g_live_owned_keys == 1);
    D_TEST_EXPR(g_live_owned_values == 1);
    got = d_unordered_map_get(&map, &lookup);
    D_TEST_NOT_NULL(got);
    D_TEST_STR_EQ(*got, "new");
    d_unordered_map_destroy(&map);
    D_TEST_EXPR(g_free_calls == 2);
    D_TEST_EXPR(g_live_owned_keys == 0);
    D_TEST_EXPR(g_live_owned_values == 0);
}

static void test_long_interleaved_insert_remove_delete_stress(void)
{
    DUnorderedMap map;
    bool alive[300] = {0};

    reset_counters();
    init_collision_map(&map, sizeof(int), 1, count_key_destroy_int, count_value_destroy_int);
    for (int round = 0; round < 6; ++round)
    {
        for (int i = 0; i < 300; ++i)
        {
            if (((i + round) % 3) != 0)
            {
                int value = round * 10000 + i;
                D_TEST_EXPR(d_unordered_map_insert(&map, &i, &value) == D_OK);
                alive[i] = true;
            }
        }
        for (int i = 0; i < 300; i += 5)
        {
            if (alive[i])
            {
                int out_key = 0;
                int out_value = 0;
                D_TEST_EXPR(d_unordered_map_remove(&map, &i, &out_key, &out_value) == D_OK);
                D_TEST_EXPR(out_key == i);
                alive[i] = false;
            }
        }
        for (int i = 2; i < 300; i += 7)
        {
            if (alive[i])
            {
                D_TEST_EXPR(d_unordered_map_delete(&map, &i) == D_OK);
                alive[i] = false;
            }
        }
        for (int i = 0; i < 300; ++i)
        {
            int *got = d_unordered_map_get(&map, &i);
            if (alive[i])
                D_TEST_NOT_NULL(got);
            else
                D_TEST_NULL(got);
        }
    }
    d_unordered_map_destroy(&map);
}

static void test_two_groups_hash_all_entries_retrievable(void)
{
    DUnorderedMap map;

    D_TEST_EXPR(d_unordered_map_init(&map, sizeof(int), sizeof(int), 0,
                                     hash_int_two_groups, cmp_int_key, NULL, NULL) == D_OK);
    for (int i = 0; i < 50; ++i)
        insert_int(&map, i, i * 5);
    assert_size(&map, 50);
    for (int i = 0; i < 50; ++i)
        assert_get_int(&map, i, i * 5);
    d_unordered_map_destroy(&map);
}

static void test_insert_get_absent_key_is_null(void)
{
    DUnorderedMap map;

    init_int_map(&map, sizeof(int), 0, NULL, NULL);
    insert_int(&map, 1, 10);
    D_TEST_NULL(d_unordered_map_get(&map, &(int){99}));
    d_unordered_map_destroy(&map);
}

static void test_repeated_delete_same_key_returns_not_exist_second_time(void)
{
    DUnorderedMap map;

    init_int_map(&map, sizeof(int), 0, NULL, NULL);
    insert_int(&map, 42, 420);
    D_TEST_EXPR(d_unordered_map_delete(&map, &(int){42}) == D_OK);
    D_TEST_EXPR(d_unordered_map_delete(&map, &(int){42}) == D_ERR_NOT_EXIST);
    assert_size(&map, 0);
    d_unordered_map_destroy(&map);
}

static void test_insert_after_delete_same_key_works(void)
{
    DUnorderedMap map;

    init_int_map(&map, sizeof(int), 0, NULL, NULL);
    insert_int(&map, 7, 70);
    D_TEST_EXPR(d_unordered_map_delete(&map, &(int){7}) == D_OK);
    insert_int(&map, 7, 777);
    assert_size(&map, 1);
    assert_get_int(&map, 7, 777);
    d_unordered_map_destroy(&map);
}

static void test_capacity_grows_monotonically_under_load(void)
{
    DUnorderedMap map;
    usize prev_capacity = 0;
    usize curr_capacity = 0;

    init_int_map(&map, sizeof(int), 0, NULL, NULL);
    prev_capacity = d_unordered_map_get_capacity(&map);
    for (int i = 0; i < 500; ++i)
    {
        insert_int(&map, i, i);
        curr_capacity = d_unordered_map_get_capacity(&map);
        D_TEST_EXPR(curr_capacity >= prev_capacity);
        prev_capacity = curr_capacity;
    }
    d_unordered_map_destroy(&map);
}

int main(void)
{
    DTest tests[] = {
        D_TEST_GENERATE_TEST(test_init_allows_zero_capacity_and_reports_nonzero_capacity),
        D_TEST_GENERATE_TEST(test_destroy_null_pointer_is_noop),
        D_TEST_GENERATE_TEST(test_destroy_empty_map_is_safe),
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
        D_TEST_GENERATE_TEST(test_int32_wrapper_constructor_insert_get_remove),
        D_TEST_GENERATE_TEST(test_usize_wrapper_many_large_keys),
        D_TEST_GENERATE_TEST(test_char_wrapper_all_byte_like_chars),
        D_TEST_GENERATE_TEST(test_bool_wrapper_only_two_keys_and_duplicate_update),
        D_TEST_GENERATE_TEST(test_owned_str_key_value_map_delete),
        D_TEST_GENERATE_TEST(test_owned_str_key_value_map_remove_transfers_ownership),
        D_TEST_GENERATE_TEST(test_owned_str_key_value_duplicate_insert_destroys_old_pair),
        D_TEST_GENERATE_TEST(test_long_interleaved_insert_remove_delete_stress),
        D_TEST_GENERATE_TEST(test_two_groups_hash_all_entries_retrievable),
        D_TEST_GENERATE_TEST(test_insert_get_absent_key_is_null),
        D_TEST_GENERATE_TEST(test_repeated_delete_same_key_returns_not_exist_second_time),
        D_TEST_GENERATE_TEST(test_insert_after_delete_same_key_works),
        D_TEST_GENERATE_TEST(test_capacity_grows_monotonically_under_load),
    };
    D_TEST_RUN_TESTS(tests);
    return 0;
}
