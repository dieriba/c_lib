#include <immintrin.h>
#include "d_types.h"
#include "raw_map.h"

#define compute_h1(hash) (hash >> 7)
#define compute_h2(hash) (hash & 0x7F)

static usize default_hash_fn(void *key)
{
}

static RawMap *raw_map_new_raw()
{
    return malloc(sizeof(RawMap));
}

static RawMap *raw_map_init(RawMap *map, usize key_size, usize value_size, usize capacity, HashFn hash_fn, CmpFn cmp_fn, FreeFn free_fn)
{
    if (map == NULL)
        return NULL;

    map->value_size = value_size;
    map->key_size = key_size;
    map->cmp_fn = cmp_fn;
    map->hash_fn = hash_fn == NULL ? default_hash_fn : hash_fn;
    map->free_fn = free_fn;
    return map;
}

RawMap *raw_map_new(usize key_size, usize value_size, usize capacity, HashFn hash_fn, CmpFn cmp_fn, FreeFn free_fn)
{

    RawMap *raw_map = raw_map_new_raw();

    return raw_map_init(raw_map, key_size, value_size, capacity, hash_fn, cmp_fn, free_fn);
}

static usize get_hash_from_key()
{
    
}

RawMap *raw_map_insert(RawMap *map, void *key, void *value)
{
    if (map == NULL)
        return NULL;
}

void *raw_map_get(RawMap *map, void *key)
{
}

void *raw_map_remove(RawMap *map, void *key, void *out_elem)
{
}