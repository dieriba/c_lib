#include "raw_map.h"
#include "d_unordered_map.h"
#include <stdlib.h>

struct DUnorderedMap
{
    RawMap raw_map;
};

static DUnorderedMap *d_unordered_map_new_raw()
{
    return malloc(sizeof(DUnorderedMap));
}

DUnorderedMap *d_unordered_map_new(usize key_size, usize value_size, usize capacity, FnPtrGenHash hash_fn, FnPtrCmpKey cmp_fn, FnPtrFreeElem free_fn)
{
    DUnorderedMap *d_unordered_map = d_unordered_map_new_raw();
    if (d_unordered_map == NULL)
        return NULL;
    if (raw_map_init(&d_unordered_map->raw_map, key_size, value_size, capacity, hash_fn, cmp_fn, free_fn) == NULL)
        return NULL;
    return d_unordered_map;
}

DUnorderedMap *d_unordered_map_insert(DUnorderedMap *map, void *key, void *value)
{
    return raw_map_insert((RawMap *)map, key, value);
}

void *d_unordered_map_get(DUnorderedMap *map, void *key)
{
    return raw_map_get((RawMap *)map, key);
}

bool d_unordered_map_remove(DUnorderedMap *map, void *key, void *out_elem)
{
    return raw_map_remove((RawMap *)map, key, out_elem);
}

void d_unordered_map_destroy(DUnorderedMap **map)
{
    if (map == NULL || *map == NULL)
        return;
    DUnorderedMap *d_map = *map;
    raw_map_free(&d_map->raw_map);
    free(d_map);
    *map = NULL;
}