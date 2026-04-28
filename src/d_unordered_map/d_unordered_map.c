#include "d_unordered_map.h"
#include <stdlib.h>

typedef struct SwissTable
{
    u8 *metadata;
    void **groups;
} SwissTable;

struct _DUnorderedMap
{
    SwissTable table;
    HashFn hash_fn;
    FreeFn free_fn;
    usize elem_size;
};

#define compute_h1(hash) (hash >> 7)
#define compute_h2(hash) (hash & 0x7F)

static usize default_hash_fn(void *key)
{
}

static DUnorderedMap *d_unordered_map_new_raw()
{
    return malloc(sizeof(DUnorderedMap));
}

static DUnorderedMap *init_map(DUnorderedMap *map, usize elem_size, usize capacity, HashFn hash_fn, FreeFn free_fn)
{
    if (map == NULL)
        return NULL;

    map->elem_size = elem_size;
    map->hash_fn = hash_fn == NULL ? default_hash_fn : hash_fn;
    map->free_fn = free_fn;

    return map;
}

DUnorderedMap *d_unordered_map_new(usize elem_size, usize capacity, HashFn hash_fn, FreeFn free_fn)
{
    DUnorderedMap *map = d_unordered_map_new_raw();

    if (init_map(map, elem_size, capacity, hash_fn, free_fn) == NULL)
        return NULL;

    return map;
}

static usize get_hash_from_key()
{
}

DUnorderedMap *d_unordered_map_insert(DUnorderedMap *map, void *key, void *value)
{
    if (map == NULL)
        return NULL;
}

void *d_unordered_map_get(DUnorderedMap *map, void *key)
{
}

void *d_unordered_map_remove(DUnorderedMap *map, void *key, void *out_elem)
{
}