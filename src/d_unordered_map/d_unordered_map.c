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

DResult d_unordered_map_new(DUnorderedMap **d_unordered_map, usize key_size, usize value_size, usize capacity, FnPtrGenHash hash_fn, FnPtrCmpKey cmp_fn, FnPtrFreeElem free_fn)
{
    if (d_unordered_map == NULL)
        return D_ERR_INVALID_ARG;
    else if ((*d_unordered_map = d_unordered_map_new_raw()))
        return D_ERR_ALLOC;
    return raw_map_init((RawMap *)(*d_unordered_map), key_size, value_size, capacity, hash_fn, cmp_fn, free_fn);
}

DResult d_unordered_map_get_size(const DUnorderedMap *d_unordered_map, usize *size)
{
    return raw_map_get_size((RawMap *)d_unordered_map, size);
}

DResult d_unordered_map_get_capacity(const DUnorderedMap *d_unordered_map, usize *capacity)
{
    return raw_map_get_capacity((RawMap *)d_unordered_map, capacity);
}

DResult d_unordered_map_insert(DUnorderedMap *map, void *key, void *value)
{
    return raw_map_insert((RawMap *)map, key, value);
}

void *d_unordered_map_get(const DUnorderedMap *map, void *key)
{
    return raw_map_get((RawMap *)map, key);
}

DResult d_unordered_map_remove(DUnorderedMap *map, void *key, void *out_elem)
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