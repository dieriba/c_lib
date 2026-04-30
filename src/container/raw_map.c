#include <immintrin.h>
#include "d_types.h"
#include "d_math.h"
#include "raw_map.h"

#define compute_h1(hash) (hash >> 7)
#define compute_h2(hash) (hash & 0x7F)

#define DEFAULT_NB_GROUPS 2UL
#define GROUP_LEN 16UL
#define MIN_CAPACITY DEFAULT_NB_GROUPS *GROUP_LEN
#define raw_map_get_key(raw_map, key_idx) (char *)raw_map->map + raw_map->metadata_len + (raw_map->key_size + raw_map->value_size) * key_idx
#define raw_map_get_value_from_key_addr(raw_map, key_addr) (char *)key_addr + (raw_map->key_size)

static usize default_hash_fn(void *key)
{
}


static usize *compute_new_alloc_size(RawMap *raw_map)
{
    
}

static RawMap *raw_map_new_raw()
{
    return malloc(sizeof(RawMap));
}

static RawMap *raw_map_init(RawMap *raw_map, usize key_size, usize value_size, usize capacity, HashFn hash_fn, CmpFn cmp_fn, FreeFn free_fn)
{
    if (raw_map == NULL)
        return NULL;

    usize slot_size;
    if (d_overflow_check_add_usize(capacity, GROUP_LEN, &capacity) || d_overflow_check_add_usize(key_size, value_size, &slot_size))
        return NULL;

    capacity = ALIGN_ROUND_DOWN(capacity, GROUP_LEN);

    raw_map->metadata_len = capacity;

    usize total_group_size;
    if (d_overflow_check_mul_usize(capacity, slot_size, &total_group_size) || d_overflow_check_add_usize(capacity, total_group_size, &capacity) || (raw_map->map = malloc(capacity)) == NULL)
        return NULL;

    raw_map->capacity = capacity;
    raw_map->value_size = value_size;
    raw_map->key_size = key_size;
    raw_map->len = 0;
    raw_map->cmp_fn = cmp_fn;
    raw_map->hash_fn = hash_fn == NULL ? default_hash_fn : hash_fn;
    raw_map->free_fn = free_fn;
    return raw_map;
}

RawMap *raw_map_new(usize key_size, usize value_size, usize capacity, HashFn hash_fn, CmpFn cmp_fn, FreeFn free_fn)
{

    RawMap *raw_map = raw_map_new_raw();

    return raw_map_init(raw_map, key_size, value_size, capacity, hash_fn, cmp_fn, free_fn);
}

RawMap *raw_map_insert(RawMap *raw_map, void *key, void *value)
{
    if (raw_map == NULL || key == NULL)
        return NULL;
}

void *raw_map_get(RawMap *raw_map, void *key)
{
    if (raw_map == NULL || key == NULL)
        return NULL;
}

void *raw_map_remove(RawMap *raw_map, void *key, void *out_elem)
{
    if (raw_map == NULL || key == NULL)
        return NULL;
}