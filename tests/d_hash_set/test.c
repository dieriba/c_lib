#include "raw_map.h"
#include "d_hash_set.h"
#include "d_test.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct BinKey
{
    unsigned char bytes[16];
} BinKey;

typedef struct BigKey
{
    int id;
    double score;
    unsigned char payload[37];
} BigKey;

typedef struct PairKey
{
    int a;
    int b;
} PairKey;

static int g_free_calls;
static int g_key_sum;
static int g_live_strings;
static int g_seen_payload_sum;

static void reset_counters(void)
{
    g_free_calls = 0;
    g_key_sum = 0;
    g_live_strings = 0;
    g_seen_payload_sum = 0;
}

static usize hash_int_key(void *key)
{
    int k = *(int *)key;
    return (usize)((unsigned)k * 2654435761u);
}

static usize hash_int_collision(void *key)
{
    (void)key;
    return 0x2A;
}

static usize hash_int_two_groups(void *key)
{
    int k = *(int *)key;
    return (usize)((k & 1) << 7);
}

static bool cmp_int_key(void *a, void *b)
{
    return *(int *)a == *(int *)b;
}

static usize hash_pair_key(void *key)
{
    const PairKey *p = (const PairKey *)key;
    return (usize)((unsigned)p->a * 1315423911u) ^ (usize)((unsigned)p->b * 2654435761u);
}

static bool cmp_pair_key(void *a, void *b)
{
    const PairKey *pa = (const PairKey *)a;
    const PairKey *pb = (const PairKey *)b;
    return pa->a == pb->a && pa->b == pb->b;
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

static usize hash_big_key(void *key)
{
    const BigKey *k = (const BigKey *)key;
    return (usize)((unsigned)k->id * 11400714819323198485ull);
}

static bool cmp_big_key(void *a, void *b)
{
    const BigKey *ka = (const BigKey *)a;
    const BigKey *kb = (const BigKey *)b;
    return ka->id == kb->id && ka->score == kb->score && memcmp(ka->payload, kb->payload, sizeof(ka->payload)) == 0;
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

static void free_count_int_key(void *elem)
{
    g_free_calls++;
    if (elem != NULL)
        g_key_sum += *(int *)elem;
}

static void free_count_big_key(void *elem)
{
    const BigKey *k = (const BigKey *)elem;
    g_free_calls++;
    if (k != NULL)
        g_seen_payload_sum += k->payload[0] + k->payload[36];
}

static void free_owned_string_key(void *elem)
{
    char *s;

    g_free_calls++;
    if (elem == NULL)
        return;
    s = *(char **)elem;
    if (s != NULL)
    {
        g_live_strings--;
        free(s);
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

static void init_int_set_custom(DHashSet *set, usize capacity, FnPtrFreeElem free_fn)
{
    D_TEST_EXPR(d_hash_set_init(set, sizeof(int), capacity, hash_int_key, cmp_int_key, free_fn, NULL) == D_OK);
}

static void init_collision_set(DHashSet *set, usize capacity, FnPtrFreeElem free_fn)
{
    D_TEST_EXPR(d_hash_set_init(set, sizeof(int), capacity, hash_int_collision, cmp_int_key, free_fn, NULL) == D_OK);
}

static void assert_size(DHashSet *set, usize expected)
{
    usize size = 999999;
    D_TEST_EXPR(d_hash_set_get_size(set, &size) == D_OK);
    D_TEST_EXPR(size == expected);
}

static void assert_capacity_at_least(DHashSet *set, usize expected_min)
{
    usize capacity = 0;
    D_TEST_EXPR(d_hash_set_get_capacity(set, &capacity) == D_OK);
    D_TEST_EXPR(capacity >= expected_min);
}

static void insert_int(DHashSet *set, int key)
{
    D_TEST_EXPR(d_hash_set_insert(set, &key) == D_OK);
}

static void assert_exists_int(DHashSet *set, int key)
{
    D_TEST_EXPR(d_hash_set_key_exists(set, &key) == true);
}

static void assert_missing_int(DHashSet *set, int key)
{
    D_TEST_EXPR(d_hash_set_key_exists(set, &key) == false);
}

static BinKey make_bin_key(int seed)
{
    BinKey k;
    for (usize i = 0; i < sizeof(k.bytes); ++i)
        k.bytes[i] = (unsigned char)(seed * 17 + (int)i * 31);
    return k;
}

static BigKey make_big_key(int id)
{
    BigKey k;
    k.id = id;
    k.score = (double)id * 1.25;
    memset(k.payload, (unsigned char)(id & 0xFF), sizeof(k.payload));
    return k;
}

static void test_init_rejects_null_set(void)
{
    D_TEST_EXPR(d_hash_set_init(NULL, sizeof(int), 0, hash_int_key, cmp_int_key, NULL, NULL) == D_ERR_INVALID_ARG);
}

static void test_init_rejects_zero_key_size(void)
{
    DHashSet set;
    D_TEST_EXPR(d_hash_set_init(&set, 0, 0, hash_int_key, cmp_int_key, NULL, NULL) == D_ERR_INVALID_ARG);
}

static void test_init_accepts_zero_capacity_and_aligns_capacity(void)
{
    DHashSet set;
    init_int_set_custom(&set, 0, NULL);
    assert_size(&set, 0);
    assert_capacity_at_least(&set, 16);
    d_hash_set_destroy(&set);
}

static void test_get_size_rejects_null_set(void)
{
    usize size = 123;
    D_TEST_EXPR(d_hash_set_get_size(NULL, &size) == D_ERR_INVALID_ARG);
}

static void test_get_size_rejects_null_out_pointer(void)
{
    DHashSet set;
    init_int_set_custom(&set, 0, NULL);
    D_TEST_EXPR(d_hash_set_get_size(&set, NULL) == D_ERR_INVALID_ARG);
    d_hash_set_destroy(&set);
}

static void test_get_capacity_rejects_null_set(void)
{
    usize capacity = 123;
    D_TEST_EXPR(d_hash_set_get_capacity(NULL, &capacity) == D_ERR_INVALID_ARG);
}

static void test_get_capacity_rejects_null_out_pointer(void)
{
    DHashSet set;
    init_int_set_custom(&set, 0, NULL);
    D_TEST_EXPR(d_hash_set_get_capacity(&set, NULL) == D_ERR_INVALID_ARG);
    d_hash_set_destroy(&set);
}

static void test_insert_rejects_null_set(void)
{
    int key = 1;
    D_TEST_EXPR(d_hash_set_insert(NULL, &key) == D_ERR_INVALID_ARG);
}

static void test_insert_rejects_null_key(void)
{
    DHashSet set;
    init_int_set_custom(&set, 0, NULL);
    D_TEST_EXPR(d_hash_set_insert(&set, NULL) == D_ERR_INVALID_ARG);
    assert_size(&set, 0);
    d_hash_set_destroy(&set);
}

static void test_key_exists_null_set_is_false(void)
{
    int key = 1;
    D_TEST_EXPR(d_hash_set_key_exists(NULL, &key) == false);
}

static void test_key_exists_null_key_is_false(void)
{
    DHashSet set;
    init_int_set_custom(&set, 0, NULL);
    D_TEST_EXPR(d_hash_set_key_exists(&set, NULL) == false);
    d_hash_set_destroy(&set);
}

static void test_delete_rejects_null_set(void)
{
    int key = 1;
    D_TEST_EXPR(d_hash_set_delete(NULL, &key) == D_ERR_INVALID_ARG);
}

static void test_delete_rejects_null_key(void)
{
    DHashSet set;
    init_int_set_custom(&set, 0, NULL);
    D_TEST_EXPR(d_hash_set_delete(&set, NULL) == D_ERR_INVALID_ARG);
    d_hash_set_destroy(&set);
}

static void test_remove_rejects_null_set(void)
{
    int key = 1;
    int out = 0;
    D_TEST_EXPR(d_hash_set_remove(NULL, &key, &out) == D_ERR_INVALID_ARG);
}

static void test_remove_rejects_null_key(void)
{
    DHashSet set;
    int out = 0;
    init_int_set_custom(&set, 0, NULL);
    D_TEST_EXPR(d_hash_set_remove(&set, NULL, &out) == D_ERR_INVALID_ARG);
    d_hash_set_destroy(&set);
}

static void test_remove_rejects_null_output_slot(void)
{
    DHashSet set;
    int key = 1;
    init_int_set_custom(&set, 0, NULL);
    insert_int(&set, key);
    D_TEST_EXPR(d_hash_set_remove(&set, &key, NULL) == D_ERR_INVALID_ARG);
    assert_exists_int(&set, key);
    assert_size(&set, 1);
    d_hash_set_destroy(&set);
}

static void test_destroy_null_pointer_is_noop(void)
{
    d_hash_set_destroy(NULL);
}

static void test_destroy_empty_set_is_safe(void)
{
    DHashSet set;
    D_TEST_EXPR(d_hash_set_init(&set, sizeof(int), 0, hash_int_key, cmp_int_key, NULL, NULL) == D_OK);
    d_hash_set_destroy(&set);
}

static void test_single_insert_exists_and_size_one(void)
{
    DHashSet set;
    init_int_set_custom(&set, 0, NULL);
    insert_int(&set, 42);
    assert_exists_int(&set, 42);
    assert_missing_int(&set, 43);
    assert_size(&set, 1);
    d_hash_set_destroy(&set);
}

static void test_duplicate_insert_keeps_size_one(void)
{
    DHashSet set;
    init_int_set_custom(&set, 0, NULL);
    insert_int(&set, 7);
    insert_int(&set, 7);
    insert_int(&set, 7);
    assert_exists_int(&set, 7);
    assert_size(&set, 1);
    d_hash_set_destroy(&set);
}

static void test_insert_copies_key_bytes_not_pointer(void)
{
    DHashSet set;
    int key = 11;
    init_int_set_custom(&set, 0, NULL);
    D_TEST_EXPR(d_hash_set_insert(&set, &key) == D_OK);
    key = 99;
    assert_exists_int(&set, 11);
    assert_missing_int(&set, 99);
    d_hash_set_destroy(&set);
}

static void test_many_sequential_ints_survive_rehash(void)
{
    DHashSet set;
    init_int_set_custom(&set, 1, NULL);
    for (int i = 0; i < 300; ++i)
        insert_int(&set, i);
    assert_size(&set, 300);
    for (int i = 0; i < 300; ++i)
        assert_exists_int(&set, i);
    for (int i = 300; i < 340; ++i)
        assert_missing_int(&set, i);
    assert_capacity_at_least(&set, 300);
    d_hash_set_destroy(&set);
}

static void test_collision_chain_all_keys_exist(void)
{
    DHashSet set;
    init_collision_set(&set, 0, NULL);
    for (int i = 0; i < 64; ++i)
        insert_int(&set, i);
    assert_size(&set, 64);
    for (int i = 0; i < 64; ++i)
        assert_exists_int(&set, i);
    d_hash_set_destroy(&set);
}

static void test_collision_delete_middle_keeps_later_keys_findable(void)
{
    DHashSet set;
    init_collision_set(&set, 0, NULL);
    for (int i = 0; i < 40; ++i)
        insert_int(&set, i);
    for (int i = 5; i < 30; i += 3)
        D_TEST_EXPR(d_hash_set_delete(&set, &i) == D_OK);
    for (int i = 0; i < 40; ++i)
    {
        if (i >= 5 && i < 30 && (i - 5) % 3 == 0)
            assert_missing_int(&set, i);
        else
            assert_exists_int(&set, i);
    }
    d_hash_set_destroy(&set);
}

static void test_collision_remove_middle_transfers_exact_key(void)
{
    DHashSet set;
    init_collision_set(&set, 0, NULL);
    for (int i = 0; i < 32; ++i)
        insert_int(&set, i);
    for (int i = 4; i < 20; i += 4)
    {
        int out = -1;
        D_TEST_EXPR(d_hash_set_remove(&set, &i, &out) == D_OK);
        D_TEST_EXPR(out == i);
        assert_missing_int(&set, i);
    }
    for (int i = 0; i < 32; ++i)
        if (!(i >= 4 && i < 20 && i % 4 == 0))
            assert_exists_int(&set, i);
    d_hash_set_destroy(&set);
}

static void test_remove_missing_returns_not_exist_and_does_not_touch_output(void)
{
    DHashSet set;
    int key = 123;
    int out = 777;
    init_int_set_custom(&set, 0, NULL);
    D_TEST_EXPR(d_hash_set_remove(&set, &key, &out) == D_ERR_NOT_EXIST);
    D_TEST_EXPR(out == 777);
    assert_size(&set, 0);
    d_hash_set_destroy(&set);
}

static void test_delete_missing_returns_not_exist(void)
{
    DHashSet set;
    int key = 123;
    init_int_set_custom(&set, 0, NULL);
    D_TEST_EXPR(d_hash_set_delete(&set, &key) == D_ERR_NOT_EXIST);
    assert_size(&set, 0);
    d_hash_set_destroy(&set);
}

static void test_delete_existing_decrements_size(void)
{
    DHashSet set;
    init_int_set_custom(&set, 0, NULL);
    insert_int(&set, 1);
    insert_int(&set, 2);
    insert_int(&set, 3);
    int key = 2;
    D_TEST_EXPR(d_hash_set_delete(&set, &key) == D_OK);
    assert_missing_int(&set, 2);
    assert_exists_int(&set, 1);
    assert_exists_int(&set, 3);
    assert_size(&set, 2);
    d_hash_set_destroy(&set);
}

static void test_reinsert_after_delete_reuses_deleted_slot(void)
{
    DHashSet set;
    init_collision_set(&set, 0, NULL);
    for (int i = 0; i < 32; ++i)
        insert_int(&set, i);
    for (int i = 0; i < 32; i += 2)
        D_TEST_EXPR(d_hash_set_delete(&set, &i) == D_OK);
    for (int i = 100; i < 116; ++i)
        insert_int(&set, i);
    for (int i = 1; i < 32; i += 2)
        assert_exists_int(&set, i);
    for (int i = 100; i < 116; ++i)
        assert_exists_int(&set, i);
    assert_size(&set, 32);
    d_hash_set_destroy(&set);
}

static void test_remove_then_reinsert_same_key(void)
{
    DHashSet set;
    int key = 55;
    int out = 0;
    init_int_set_custom(&set, 0, NULL);
    insert_int(&set, key);
    D_TEST_EXPR(d_hash_set_remove(&set, &key, &out) == D_OK);
    D_TEST_EXPR(out == 55);
    assert_missing_int(&set, key);
    insert_int(&set, key);
    assert_exists_int(&set, key);
    assert_size(&set, 1);
    d_hash_set_destroy(&set);
}

static void test_delete_then_delete_same_key_returns_not_exist_second_time(void)
{
    DHashSet set;
    int key = 8;
    init_int_set_custom(&set, 0, NULL);
    insert_int(&set, key);
    D_TEST_EXPR(d_hash_set_delete(&set, &key) == D_OK);
    D_TEST_EXPR(d_hash_set_delete(&set, &key) == D_ERR_NOT_EXIST);
    assert_size(&set, 0);
    d_hash_set_destroy(&set);
}

static void test_remove_does_not_call_destroyer(void)
{
    DHashSet set;
    int key = 10;
    int out = 0;
    reset_counters();
    init_int_set_custom(&set, 0, free_count_int_key);
    insert_int(&set, key);
    D_TEST_EXPR(d_hash_set_remove(&set, &key, &out) == D_OK);
    D_TEST_EXPR(out == 10);
    D_TEST_EXPR(g_free_calls == 0);
    d_hash_set_destroy(&set);
    D_TEST_EXPR(g_free_calls == 0);
}

static void test_delete_calls_destroyer_once_for_key(void)
{
    DHashSet set;
    int key = 14;
    reset_counters();
    init_int_set_custom(&set, 0, free_count_int_key);
    insert_int(&set, key);
    D_TEST_EXPR(d_hash_set_delete(&set, &key) == D_OK);
    D_TEST_EXPR(g_free_calls == 1);
    D_TEST_EXPR(g_key_sum == 14);
    d_hash_set_destroy(&set);
    D_TEST_EXPR(g_free_calls == 1);
}

static void test_destroy_calls_destroyer_for_all_live_keys_only(void)
{
    DHashSet set;
    reset_counters();
    init_int_set_custom(&set, 0, free_count_int_key);
    for (int i = 1; i <= 10; ++i)
        insert_int(&set, i);
    for (int i = 2; i <= 10; i += 2)
        D_TEST_EXPR(d_hash_set_delete(&set, &i) == D_OK);
    D_TEST_EXPR(g_free_calls == 5);
    d_hash_set_destroy(&set);
    D_TEST_EXPR(g_free_calls == 10);
    D_TEST_EXPR(g_key_sum == 55);
}

static void test_duplicate_insert_destroyer_called_for_replaced_key(void)
{
    DHashSet set;
    int key = 9;
    reset_counters();
    init_int_set_custom(&set, 0, free_count_int_key);
    insert_int(&set, key);
    insert_int(&set, key);
    assert_size(&set, 1);
    D_TEST_EXPR(g_free_calls == 1);
    d_hash_set_destroy(&set);
    D_TEST_EXPR(g_free_calls == 2);
}

static void test_destroy_after_remove_does_not_destroy_removed_key(void)
{
    DHashSet set;
    int out = 0;
    reset_counters();
    init_int_set_custom(&set, 0, free_count_int_key);
    for (int i = 1; i <= 5; ++i)
        insert_int(&set, i);
    int key = 3;
    D_TEST_EXPR(d_hash_set_remove(&set, &key, &out) == D_OK);
    D_TEST_EXPR(out == 3);
    d_hash_set_destroy(&set);
    D_TEST_EXPR(g_free_calls == 4);
    D_TEST_EXPR(g_key_sum == 1 + 2 + 4 + 5);
}

static void test_str_set_insert_and_lookup_with_distinct_pointer_same_text(void)
{
    DHashSet set;
    char *stored = dup_lit("alpha");
    char *lookup = dup_lit("alpha");
    D_TEST_EXPR(d_hash_set_init(&set, sizeof(char *), 0, local_hash_str, local_cmp_str, NULL, NULL) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &stored) == D_OK);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &lookup) == true);
    free(stored);
    free(lookup);
    d_hash_set_destroy(&set);
}

static void test_str_set_duplicate_text_keeps_size_one(void)
{
    DHashSet set;
    char *a = dup_lit("same");
    char *b = dup_lit("same");
    D_TEST_EXPR(d_hash_set_init(&set, sizeof(char *), 0, local_hash_str, local_cmp_str, NULL, NULL) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &a) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &b) == D_OK);
    assert_size(&set, 1);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &a) == true);
    free(a);
    free(b);
    d_hash_set_destroy(&set);
}

static void test_str_set_remove_transfers_owned_pointer(void)
{
    DHashSet set;
    char *owned = dup_lit("owned-remove");
    char *query = dup_lit("owned-remove");
    char *out = NULL;
    reset_counters();
    g_live_strings = 1;
    D_TEST_EXPR(d_hash_set_init(&set, sizeof(char *), 0, local_hash_str, local_cmp_str, free_owned_string_key, NULL) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &owned) == D_OK);
    D_TEST_EXPR(d_hash_set_remove(&set, &query, &out) == D_OK);
    D_TEST_NOT_NULL(out);
    D_TEST_STR_EQ(out, "owned-remove");
    D_TEST_EXPR(g_free_calls == 0);
    free(out);
    g_live_strings--;
    free(query);
    d_hash_set_destroy(&set);
    D_TEST_EXPR(g_free_calls == 0);
    D_TEST_EXPR(g_live_strings == 0);
}

static void test_str_set_delete_destroys_owned_pointer(void)
{
    DHashSet set;
    char *owned = dup_lit("owned-delete");
    char *query = dup_lit("owned-delete");
    reset_counters();
    g_live_strings = 1;
    D_TEST_EXPR(d_hash_set_init(&set, sizeof(char *), 0, local_hash_str, local_cmp_str, free_owned_string_key, NULL) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &owned) == D_OK);
    D_TEST_EXPR(d_hash_set_delete(&set, &query) == D_OK);
    D_TEST_EXPR(g_free_calls == 1);
    D_TEST_EXPR(g_live_strings == 0);
    free(query);
    d_hash_set_destroy(&set);
    D_TEST_EXPR(g_free_calls == 1);
}

static void test_str_set_destroy_destroys_all_owned_pointers(void)
{
    DHashSet set;
    reset_counters();
    D_TEST_EXPR(d_hash_set_init(&set, sizeof(char *), 0, local_hash_str, local_cmp_str, free_owned_string_key, NULL) == D_OK);
    for (int i = 0; i < 40; ++i)
    {
        char buf[32];
        char *s;
        snprintf(buf, sizeof(buf), "key-%02d", i);
        s = dup_lit(buf);
        g_live_strings++;
        D_TEST_EXPR(d_hash_set_insert(&set, &s) == D_OK);
    }
    assert_size(&set, 40);
    d_hash_set_destroy(&set);
    D_TEST_EXPR(g_free_calls == 40);
    D_TEST_EXPR(g_live_strings == 0);
}

static void test_str_set_duplicate_owned_key_destroys_replaced_pointer(void)
{
    DHashSet set;
    char *a = dup_lit("dup-owned");
    char *b = dup_lit("dup-owned");
    reset_counters();
    g_live_strings = 2;
    D_TEST_EXPR(d_hash_set_init(&set, sizeof(char *), 0, local_hash_str, local_cmp_str, free_owned_string_key, NULL) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &a) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &b) == D_OK);
    assert_size(&set, 1);
    D_TEST_EXPR(g_free_calls == 1);
    D_TEST_EXPR(g_live_strings == 1);
    d_hash_set_destroy(&set);
    D_TEST_EXPR(g_free_calls == 2);
    D_TEST_EXPR(g_live_strings == 0);
}

static void test_str_insert_delete_reinsert_same_content(void)
{
    DHashSet set;
    char *first = dup_lit("hello");
    char *second = dup_lit("hello");
    char *query = dup_lit("hello");
    reset_counters();
    g_live_strings = 2;
    D_TEST_EXPR(d_hash_set_init(&set, sizeof(char *), 0, local_hash_str, local_cmp_str, free_owned_string_key, NULL) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &first) == D_OK);
    D_TEST_EXPR(d_hash_set_delete(&set, &query) == D_OK);
    D_TEST_EXPR(g_free_calls == 1);
    D_TEST_EXPR(g_live_strings == 1);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &query) == false);
    D_TEST_EXPR(d_hash_set_insert(&set, &second) == D_OK);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &query) == true);
    assert_size(&set, 1);
    free(query);
    d_hash_set_destroy(&set);
    D_TEST_EXPR(g_free_calls == 2);
    D_TEST_EXPR(g_live_strings == 0);
}

static void test_int32_wrapper_basic_usage(void)
{
    DHashSet set;
    int32 key = 12345;
    D_TEST_EXPR(d_hash_set_new_not_owned_int32_key(&set, 0, NULL) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &key) == D_OK);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &key) == true);
    assert_size(&set, 1);
    d_hash_set_destroy(&set);
}

static void test_usize_wrapper_large_values(void)
{
    DHashSet set;
    usize a = (usize)0;
    usize b = (usize)SIZE_MAX;
    usize c = ((usize)1 << ((sizeof(usize) * 8) - 1));
    D_TEST_EXPR(d_hash_set_new_not_owned_usize_key(&set, 0, NULL) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &a) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &b) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &c) == D_OK);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &a) == true);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &b) == true);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &c) == true);
    assert_size(&set, 3);
    d_hash_set_destroy(&set);
}

static void test_bool_wrapper_keeps_only_two_values(void)
{
    DHashSet set;
    bool t = true;
    bool f = false;
    D_TEST_EXPR(d_hash_set_new_not_owned_bool_key(&set, 0, NULL) == D_OK);
    for (int i = 0; i < 10; ++i)
    {
        D_TEST_EXPR(d_hash_set_insert(&set, &t) == D_OK);
        D_TEST_EXPR(d_hash_set_insert(&set, &f) == D_OK);
    }
    D_TEST_EXPR(d_hash_set_key_exists(&set, &t) == true);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &f) == true);
    assert_size(&set, 2);
    d_hash_set_destroy(&set);
}

static void test_char_wrapper_all_byte_like_chars(void)
{
    DHashSet set;
    D_TEST_EXPR(d_hash_set_new_not_owned_char_key(&set, 0, NULL) == D_OK);
    for (int i = -64; i < 64; ++i)
    {
        char c = (char)i;
        D_TEST_EXPR(d_hash_set_insert(&set, &c) == D_OK);
    }
    for (int i = -64; i < 64; ++i)
    {
        char c = (char)i;
        D_TEST_EXPR(d_hash_set_key_exists(&set, &c) == true);
    }
    assert_size(&set, 128);
    d_hash_set_destroy(&set);
}

static void test_u64_wrapper_rehash_many_values(void)
{
    DHashSet set;
    D_TEST_EXPR(d_hash_set_new_not_owned_u64_key(&set, 1, NULL) == D_OK);
    for (u64 i = 0; i < 500; ++i)
    {
        u64 key = i * 0x9E3779B97F4A7C15ull;
        D_TEST_EXPR(d_hash_set_insert(&set, &key) == D_OK);
    }
    for (u64 i = 0; i < 500; ++i)
    {
        u64 key = i * 0x9E3779B97F4A7C15ull;
        D_TEST_EXPR(d_hash_set_key_exists(&set, &key) == true);
    }
    assert_size(&set, 500);
    d_hash_set_destroy(&set);
}

static void test_binary_keys_with_embedded_zero_bytes(void)
{
    DHashSet set;
    BinKey a = {{0, 1, 2, 0, 4, 5, 0, 7, 8, 9, 0, 11, 12, 0, 14, 15}};
    BinKey b = a;
    b.bytes[15] = 99;
    D_TEST_EXPR(d_hash_set_init(&set, sizeof(BinKey), 0, hash_bin_key, cmp_bin_key, NULL, NULL) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &a) == D_OK);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &a) == true);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &b) == false);
    assert_size(&set, 1);
    d_hash_set_destroy(&set);
}

static void test_many_binary_keys_survive_rehash(void)
{
    DHashSet set;
    D_TEST_EXPR(d_hash_set_init(&set, sizeof(BinKey), 1, hash_bin_key, cmp_bin_key, NULL, NULL) == D_OK);
    for (int i = 0; i < 250; ++i)
    {
        BinKey k = make_bin_key(i);
        D_TEST_EXPR(d_hash_set_insert(&set, &k) == D_OK);
    }
    for (int i = 0; i < 250; ++i)
    {
        BinKey k = make_bin_key(i);
        D_TEST_EXPR(d_hash_set_key_exists(&set, &k) == true);
    }
    assert_size(&set, 250);
    d_hash_set_destroy(&set);
}

static void test_big_struct_key_fieldwise_after_remove(void)
{
    DHashSet set;
    BigKey key = make_big_key(77);
    BigKey out;
    memset(&out, 0, sizeof(out));
    D_TEST_EXPR(d_hash_set_init(&set, sizeof(BigKey), 0, hash_big_key, cmp_big_key, NULL, NULL) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &key) == D_OK);
    memset(&key, 0xAB, sizeof(key));
    BigKey query = make_big_key(77);
    D_TEST_EXPR(d_hash_set_remove(&set, &query, &out) == D_OK);
    D_TEST_EXPR(out.id == 77);
    D_TEST_EXPR(out.score == 77.0 * 1.25);
    for (usize i = 0; i < sizeof(out.payload); ++i)
        D_TEST_EXPR(out.payload[i] == (unsigned char)77);
    d_hash_set_destroy(&set);
}

static void test_big_struct_destroyer_sees_all_live_keys(void)
{
    DHashSet set;
    reset_counters();
    D_TEST_EXPR(d_hash_set_init(&set, sizeof(BigKey), 0, hash_big_key, cmp_big_key, free_count_big_key, NULL) == D_OK);
    for (int i = 1; i <= 20; ++i)
    {
        BigKey k = make_big_key(i);
        D_TEST_EXPR(d_hash_set_insert(&set, &k) == D_OK);
    }
    d_hash_set_destroy(&set);
    D_TEST_EXPR(g_free_calls == 20);
    D_TEST_EXPR(g_seen_payload_sum == (2 * (20 * 21 / 2)));
}

static void test_pair_key_custom_comparator_distinguishes_fields(void)
{
    DHashSet set;
    PairKey a = {1, 2};
    PairKey b = {2, 1};
    PairKey c = {1, 3};
    D_TEST_EXPR(d_hash_set_init(&set, sizeof(PairKey), 0, hash_pair_key, cmp_pair_key, NULL, NULL) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &a) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &b) == D_OK);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &a) == true);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &b) == true);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &c) == false);
    assert_size(&set, 2);
    d_hash_set_destroy(&set);
}

static void test_two_group_hash_probe_and_delete_stress(void)
{
    DHashSet set;
    D_TEST_EXPR(d_hash_set_init(&set, sizeof(int), 0, hash_int_two_groups, cmp_int_key, NULL, NULL) == D_OK);
    for (int i = 0; i < 160; ++i)
        insert_int(&set, i);
    for (int i = 0; i < 160; i += 5)
        D_TEST_EXPR(d_hash_set_delete(&set, &i) == D_OK);
    for (int i = 200; i < 260; ++i)
        insert_int(&set, i);
    for (int i = 0; i < 160; ++i)
    {
        if (i % 5 == 0)
            assert_missing_int(&set, i);
        else
            assert_exists_int(&set, i);
    }
    for (int i = 200; i < 260; ++i)
        assert_exists_int(&set, i);
    d_hash_set_destroy(&set);
}

static void test_long_interleaved_insert_remove_delete_sequence(void)
{
    DHashSet set;
    init_collision_set(&set, 1, NULL);
    for (int round = 0; round < 6; ++round)
    {
        for (int i = 0; i < 80; ++i)
        {
            int key = round * 1000 + i;
            insert_int(&set, key);
        }
        for (int i = 0; i < 80; i += 3)
        {
            int key = round * 1000 + i;
            int out = -1;
            D_TEST_EXPR(d_hash_set_remove(&set, &key, &out) == D_OK);
            D_TEST_EXPR(out == key);
        }
        for (int i = 1; i < 80; i += 3)
        {
            int key = round * 1000 + i;
            D_TEST_EXPR(d_hash_set_delete(&set, &key) == D_OK);
        }
        for (int i = 2; i < 80; i += 3)
        {
            int key = round * 1000 + i;
            assert_exists_int(&set, key);
        }
    }
    for (int round = 0; round < 6; ++round)
        for (int i = 2; i < 80; i += 3)
        {
            int key = round * 1000 + i;
            assert_exists_int(&set, key);
        }
    d_hash_set_destroy(&set);
}

static void test_remove_all_from_collision_set_until_empty(void)
{
    DHashSet set;
    init_collision_set(&set, 0, NULL);
    for (int i = 0; i < 96; ++i)
        insert_int(&set, i);
    for (int i = 95; i >= 0; --i)
    {
        int out = -1;
        D_TEST_EXPR(d_hash_set_remove(&set, &i, &out) == D_OK);
        D_TEST_EXPR(out == i);
    }
    assert_size(&set, 0);
    for (int i = 0; i < 96; ++i)
        assert_missing_int(&set, i);
    d_hash_set_destroy(&set);
}

static void test_insert_after_remove_all_from_collision_set(void)
{
    DHashSet set;
    init_collision_set(&set, 0, NULL);
    for (int i = 0; i < 64; ++i)
        insert_int(&set, i);
    for (int i = 0; i < 64; ++i)
    {
        int out = -1;
        D_TEST_EXPR(d_hash_set_remove(&set, &i, &out) == D_OK);
    }
    for (int i = 1000; i < 1064; ++i)
        insert_int(&set, i);
    assert_size(&set, 64);
    for (int i = 1000; i < 1064; ++i)
        assert_exists_int(&set, i);
    d_hash_set_destroy(&set);
}

static void test_failed_remove_does_not_change_size_or_destroy(void)
{
    DHashSet set;
    int key = 1;
    int missing = 2;
    int out = -1;
    reset_counters();
    init_int_set_custom(&set, 0, free_count_int_key);
    insert_int(&set, key);
    D_TEST_EXPR(d_hash_set_remove(&set, &missing, &out) == D_ERR_NOT_EXIST);
    D_TEST_EXPR(out == -1);
    D_TEST_EXPR(g_free_calls == 0);
    assert_size(&set, 1);
    assert_exists_int(&set, key);
    d_hash_set_destroy(&set);
    D_TEST_EXPR(g_free_calls == 1);
}

static void test_failed_delete_does_not_change_size_or_destroy(void)
{
    DHashSet set;
    int key = 1;
    int missing = 2;
    reset_counters();
    init_int_set_custom(&set, 0, free_count_int_key);
    insert_int(&set, key);
    D_TEST_EXPR(d_hash_set_delete(&set, &missing) == D_ERR_NOT_EXIST);
    D_TEST_EXPR(g_free_calls == 0);
    assert_size(&set, 1);
    assert_exists_int(&set, key);
    d_hash_set_destroy(&set);
    D_TEST_EXPR(g_free_calls == 1);
}

static void test_capacity_never_shrinks_after_deletes(void)
{
    DHashSet set;
    usize cap_before = 0;
    usize cap_after = 0;
    init_int_set_custom(&set, 1, NULL);
    for (int i = 0; i < 300; ++i)
        insert_int(&set, i);
    D_TEST_EXPR(d_hash_set_get_capacity(&set, &cap_before) == D_OK);
    for (int i = 0; i < 300; ++i)
        D_TEST_EXPR(d_hash_set_delete(&set, &i) == D_OK);
    D_TEST_EXPR(d_hash_set_get_capacity(&set, &cap_after) == D_OK);
    D_TEST_EXPR(cap_after == cap_before);
    assert_size(&set, 0);
    d_hash_set_destroy(&set);
}

static void test_capacity_grows_monotonically_under_load(void)
{
    DHashSet set;
    usize cap_at_125 = 0;
    usize cap_at_250 = 0;
    usize cap_at_500 = 0;
    init_int_set_custom(&set, 0, NULL);
    for (int i = 0; i < 125; ++i)
        insert_int(&set, i);
    D_TEST_EXPR(d_hash_set_get_capacity(&set, &cap_at_125) == D_OK);
    for (int i = 125; i < 250; ++i)
        insert_int(&set, i);
    D_TEST_EXPR(d_hash_set_get_capacity(&set, &cap_at_250) == D_OK);
    for (int i = 250; i < 500; ++i)
        insert_int(&set, i);
    D_TEST_EXPR(d_hash_set_get_capacity(&set, &cap_at_500) == D_OK);
    D_TEST_EXPR(cap_at_250 >= cap_at_125);
    D_TEST_EXPR(cap_at_500 >= cap_at_250);
    assert_size(&set, 500);
    for (int i = 0; i < 500; ++i)
        assert_exists_int(&set, i);
    d_hash_set_destroy(&set);
}

static void test_remove_after_rehash_returns_correct_key_not_query_address(void)
{
    DHashSet set;
    int query = 123;
    int out = -1;
    init_int_set_custom(&set, 1, NULL);
    for (int i = 0; i < 200; ++i)
        insert_int(&set, i);
    D_TEST_EXPR(d_hash_set_remove(&set, &query, &out) == D_OK);
    D_TEST_EXPR(out == 123);
    query = 9999;
    D_TEST_EXPR(out == 123);
    d_hash_set_destroy(&set);
}

static void test_rehash_after_many_deleted_slots_preserves_remaining_keys(void)
{
    DHashSet set;
    init_collision_set(&set, 1, NULL);
    for (int i = 0; i < 140; ++i)
        insert_int(&set, i);
    for (int i = 0; i < 140; i += 2)
        D_TEST_EXPR(d_hash_set_delete(&set, &i) == D_OK);
    for (int i = 1000; i < 1250; ++i)
        insert_int(&set, i);
    for (int i = 1; i < 140; i += 2)
        assert_exists_int(&set, i);
    for (int i = 1000; i < 1250; ++i)
        assert_exists_int(&set, i);
    d_hash_set_destroy(&set);
}

static void test_delete_all_then_destroy_calls_destroyer_exactly_once_per_key(void)
{
    DHashSet set;
    reset_counters();
    init_int_set_custom(&set, 0, free_count_int_key);
    for (int i = 1; i <= 50; ++i)
        insert_int(&set, i);
    for (int i = 1; i <= 50; ++i)
        D_TEST_EXPR(d_hash_set_delete(&set, &i) == D_OK);
    D_TEST_EXPR(g_free_calls == 50);
    d_hash_set_destroy(&set);
    D_TEST_EXPR(g_free_calls == 50);
    D_TEST_EXPR(g_key_sum == 1275);
}

static void test_remove_all_then_destroy_calls_no_destroyer(void)
{
    DHashSet set;
    int outputs[50];
    reset_counters();
    init_int_set_custom(&set, 0, free_count_int_key);
    for (int i = 1; i <= 50; ++i)
        insert_int(&set, i);
    for (int i = 1; i <= 50; ++i)
        D_TEST_EXPR(d_hash_set_remove(&set, &i, &outputs[i - 1]) == D_OK);
    D_TEST_EXPR(g_free_calls == 0);
    d_hash_set_destroy(&set);
    D_TEST_EXPR(g_free_calls == 0);
    for (int i = 1; i <= 50; ++i)
        D_TEST_EXPR(outputs[i - 1] == i);
}

static void test_insert_min_and_max_int_values(void)
{
    DHashSet set;
    int vals[] = {0, -1, 1, INT32_MIN, INT32_MAX};
    init_int_set_custom(&set, 0, NULL);
    for (usize i = 0; i < sizeof(vals) / sizeof(vals[0]); ++i)
        insert_int(&set, vals[i]);
    for (usize i = 0; i < sizeof(vals) / sizeof(vals[0]); ++i)
        assert_exists_int(&set, vals[i]);
    assert_size(&set, sizeof(vals) / sizeof(vals[0]));
    d_hash_set_destroy(&set);
}

static void test_string_empty_and_long_keys(void)
{
    DHashSet set;
    char long_buf[512];
    memset(long_buf, 'x', sizeof(long_buf) - 1);
    long_buf[sizeof(long_buf) - 1] = '\0';
    char *empty = dup_lit("");
    char *long_s = dup_lit(long_buf);
    char *query_empty = dup_lit("");
    char *query_long = dup_lit(long_buf);
    D_TEST_EXPR(d_hash_set_init(&set, sizeof(char *), 0, local_hash_str, local_cmp_str, NULL, NULL) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &empty) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &long_s) == D_OK);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &query_empty) == true);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &query_long) == true);
    assert_size(&set, 2);
    free(empty);
    free(long_s);
    free(query_empty);
    free(query_long);
    d_hash_set_destroy(&set);
}

static void test_string_many_keys_delete_some_lookup_rest(void)
{
    DHashSet set;
    char *keys[120];
    D_TEST_EXPR(d_hash_set_init(&set, sizeof(char *), 1, local_hash_str, local_cmp_str, NULL, NULL) == D_OK);
    for (int i = 0; i < 120; ++i)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "str-key-%03d", i);
        keys[i] = dup_lit(buf);
        D_TEST_EXPR(d_hash_set_insert(&set, &keys[i]) == D_OK);
    }
    for (int i = 0; i < 120; i += 4)
        D_TEST_EXPR(d_hash_set_delete(&set, &keys[i]) == D_OK);
    for (int i = 0; i < 120; ++i)
    {
        if (i % 4 == 0)
            D_TEST_EXPR(d_hash_set_key_exists(&set, &keys[i]) == false);
        else
            D_TEST_EXPR(d_hash_set_key_exists(&set, &keys[i]) == true);
    }
    for (int i = 0; i < 120; ++i)
        free(keys[i]);
    d_hash_set_destroy(&set);
}

static void test_wrapper_remove_returns_key_for_u32(void)
{
    DHashSet set;
    u32 key = 0xDEADBEEFu;
    u32 out = 0;
    D_TEST_EXPR(d_hash_set_new_not_owned_u32_key(&set, 0, NULL) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &key) == D_OK);
    D_TEST_EXPR(d_hash_set_remove(&set, &key, &out) == D_OK);
    D_TEST_EXPR(out == key);
    assert_size(&set, 0);
    d_hash_set_destroy(&set);
}

static void test_wrapper_delete_for_int64_min_max(void)
{
    DHashSet set;
    int64 a = (int64)INT64_MIN;
    int64 b = (int64)INT64_MAX;
    D_TEST_EXPR(d_hash_set_new_not_owned_int64_key(&set, 0, NULL) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &a) == D_OK);
    D_TEST_EXPR(d_hash_set_insert(&set, &b) == D_OK);
    D_TEST_EXPR(d_hash_set_delete(&set, &a) == D_OK);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &a) == false);
    D_TEST_EXPR(d_hash_set_key_exists(&set, &b) == true);
    assert_size(&set, 1);
    d_hash_set_destroy(&set);
}

static void test_stress_alternating_duplicate_insert_delete_same_key(void)
{
    DHashSet set;
    int key = 42;
    reset_counters();
    init_int_set_custom(&set, 0, free_count_int_key);
    for (int i = 0; i < 100; ++i)
    {
        D_TEST_EXPR(d_hash_set_insert(&set, &key) == D_OK);
        D_TEST_EXPR(d_hash_set_insert(&set, &key) == D_OK);
        assert_size(&set, 1);
        D_TEST_EXPR(d_hash_set_delete(&set, &key) == D_OK);
        assert_size(&set, 0);
    }
    D_TEST_EXPR(g_free_calls == 200);
    d_hash_set_destroy(&set);
    D_TEST_EXPR(g_free_calls == 200);
}

static void test_remove_output_slot_can_be_same_address_as_query(void)
{
    DHashSet set;
    int key = 313;
    init_int_set_custom(&set, 0, NULL);
    insert_int(&set, key);
    D_TEST_EXPR(d_hash_set_remove(&set, &key, &key) == D_OK);
    D_TEST_EXPR(key == 313);
    assert_size(&set, 0);
    d_hash_set_destroy(&set);
}

static void test_delete_during_collision_probe_does_not_delete_neighbor(void)
{
    DHashSet set;
    int keys[] = {11, 22, 33, 44, 55};
    init_collision_set(&set, 0, NULL);
    for (usize i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i)
        insert_int(&set, keys[i]);
    D_TEST_EXPR(d_hash_set_delete(&set, &keys[2]) == D_OK);
    assert_missing_int(&set, 33);
    assert_exists_int(&set, 11);
    assert_exists_int(&set, 22);
    assert_exists_int(&set, 44);
    assert_exists_int(&set, 55);
    d_hash_set_destroy(&set);
}

int main(void)
{
    DTest tests[] = {
        D_TEST_GENERATE_TEST(test_init_rejects_null_set),
        D_TEST_GENERATE_TEST(test_init_rejects_zero_key_size),
        D_TEST_GENERATE_TEST(test_init_accepts_zero_capacity_and_aligns_capacity),
        D_TEST_GENERATE_TEST(test_get_size_rejects_null_set),
        D_TEST_GENERATE_TEST(test_get_size_rejects_null_out_pointer),
        D_TEST_GENERATE_TEST(test_get_capacity_rejects_null_set),
        D_TEST_GENERATE_TEST(test_get_capacity_rejects_null_out_pointer),
        D_TEST_GENERATE_TEST(test_insert_rejects_null_set),
        D_TEST_GENERATE_TEST(test_insert_rejects_null_key),
        D_TEST_GENERATE_TEST(test_key_exists_null_set_is_false),
        D_TEST_GENERATE_TEST(test_key_exists_null_key_is_false),
        D_TEST_GENERATE_TEST(test_delete_rejects_null_set),
        D_TEST_GENERATE_TEST(test_delete_rejects_null_key),
        D_TEST_GENERATE_TEST(test_remove_rejects_null_set),
        D_TEST_GENERATE_TEST(test_remove_rejects_null_key),
        D_TEST_GENERATE_TEST(test_remove_rejects_null_output_slot),
        D_TEST_GENERATE_TEST(test_destroy_null_pointer_is_noop),
        D_TEST_GENERATE_TEST(test_destroy_empty_set_is_safe),
        D_TEST_GENERATE_TEST(test_single_insert_exists_and_size_one),
        D_TEST_GENERATE_TEST(test_duplicate_insert_keeps_size_one),
        D_TEST_GENERATE_TEST(test_insert_copies_key_bytes_not_pointer),
        D_TEST_GENERATE_TEST(test_many_sequential_ints_survive_rehash),
        D_TEST_GENERATE_TEST(test_collision_chain_all_keys_exist),
        D_TEST_GENERATE_TEST(test_collision_delete_middle_keeps_later_keys_findable),
        D_TEST_GENERATE_TEST(test_collision_remove_middle_transfers_exact_key),
        D_TEST_GENERATE_TEST(test_remove_missing_returns_not_exist_and_does_not_touch_output),
        D_TEST_GENERATE_TEST(test_delete_missing_returns_not_exist),
        D_TEST_GENERATE_TEST(test_delete_existing_decrements_size),
        D_TEST_GENERATE_TEST(test_reinsert_after_delete_reuses_deleted_slot),
        D_TEST_GENERATE_TEST(test_remove_then_reinsert_same_key),
        D_TEST_GENERATE_TEST(test_delete_then_delete_same_key_returns_not_exist_second_time),
        D_TEST_GENERATE_TEST(test_remove_does_not_call_destroyer),
        D_TEST_GENERATE_TEST(test_delete_calls_destroyer_once_for_key),
        D_TEST_GENERATE_TEST(test_destroy_calls_destroyer_for_all_live_keys_only),
        D_TEST_GENERATE_TEST(test_duplicate_insert_destroyer_called_for_replaced_key),
        D_TEST_GENERATE_TEST(test_destroy_after_remove_does_not_destroy_removed_key),
        D_TEST_GENERATE_TEST(test_str_set_insert_and_lookup_with_distinct_pointer_same_text),
        D_TEST_GENERATE_TEST(test_str_set_duplicate_text_keeps_size_one),
        D_TEST_GENERATE_TEST(test_str_set_remove_transfers_owned_pointer),
        D_TEST_GENERATE_TEST(test_str_set_delete_destroys_owned_pointer),
        D_TEST_GENERATE_TEST(test_str_set_destroy_destroys_all_owned_pointers),
        D_TEST_GENERATE_TEST(test_str_set_duplicate_owned_key_destroys_replaced_pointer),
        D_TEST_GENERATE_TEST(test_str_insert_delete_reinsert_same_content),
        D_TEST_GENERATE_TEST(test_int32_wrapper_basic_usage),
        D_TEST_GENERATE_TEST(test_usize_wrapper_large_values),
        D_TEST_GENERATE_TEST(test_bool_wrapper_keeps_only_two_values),
        D_TEST_GENERATE_TEST(test_char_wrapper_all_byte_like_chars),
        D_TEST_GENERATE_TEST(test_u64_wrapper_rehash_many_values),
        D_TEST_GENERATE_TEST(test_binary_keys_with_embedded_zero_bytes),
        D_TEST_GENERATE_TEST(test_many_binary_keys_survive_rehash),
        D_TEST_GENERATE_TEST(test_big_struct_key_fieldwise_after_remove),
        D_TEST_GENERATE_TEST(test_big_struct_destroyer_sees_all_live_keys),
        D_TEST_GENERATE_TEST(test_pair_key_custom_comparator_distinguishes_fields),
        D_TEST_GENERATE_TEST(test_two_group_hash_probe_and_delete_stress),
        D_TEST_GENERATE_TEST(test_long_interleaved_insert_remove_delete_sequence),
        D_TEST_GENERATE_TEST(test_remove_all_from_collision_set_until_empty),
        D_TEST_GENERATE_TEST(test_insert_after_remove_all_from_collision_set),
        D_TEST_GENERATE_TEST(test_failed_remove_does_not_change_size_or_destroy),
        D_TEST_GENERATE_TEST(test_failed_delete_does_not_change_size_or_destroy),
        D_TEST_GENERATE_TEST(test_capacity_never_shrinks_after_deletes),
        D_TEST_GENERATE_TEST(test_capacity_grows_monotonically_under_load),
        D_TEST_GENERATE_TEST(test_remove_after_rehash_returns_correct_key_not_query_address),
        D_TEST_GENERATE_TEST(test_rehash_after_many_deleted_slots_preserves_remaining_keys),
        D_TEST_GENERATE_TEST(test_delete_all_then_destroy_calls_destroyer_exactly_once_per_key),
        D_TEST_GENERATE_TEST(test_remove_all_then_destroy_calls_no_destroyer),
        D_TEST_GENERATE_TEST(test_insert_min_and_max_int_values),
        D_TEST_GENERATE_TEST(test_string_empty_and_long_keys),
        D_TEST_GENERATE_TEST(test_string_many_keys_delete_some_lookup_rest),
        D_TEST_GENERATE_TEST(test_wrapper_remove_returns_key_for_u32),
        D_TEST_GENERATE_TEST(test_wrapper_delete_for_int64_min_max),
        D_TEST_GENERATE_TEST(test_stress_alternating_duplicate_insert_delete_same_key),
        D_TEST_GENERATE_TEST(test_remove_output_slot_can_be_same_address_as_query),
        D_TEST_GENERATE_TEST(test_delete_during_collision_probe_does_not_delete_neighbor),
    };

    D_TEST_RUN_TESTS(tests);
}
