#include "raw_map.h"
#include "d_hash_set.h"

#define VALUE_SIZE 0UL

struct DHashSet
{
    RawMap raw_map;
};

static DHashSet *d_hash_set_new_raw()
{
    return malloc(sizeof(DHashSet));
}

DResult d_hash_set_new(DHashSet **d_hash_set, usize key_size, usize capacity, FnPtrGenHash hash_fn, FnPtrCmpKey cmp_fn, FnPtrFreeElem free_fn)
{
    if (d_hash_set == NULL)
        return D_ERR_INVALID_ARG;
    else if ((*d_hash_set = d_hash_set_new_raw()) == NULL)
        return D_ERR_ALLOC;
    return raw_map_init((RawMap *)*d_hash_set, key_size, VALUE_SIZE, capacity, hash_fn, cmp_fn, free_fn);
}

DResult d_hash_set_get_size(const DHashSet *d_hash_set, usize *size)
{
    return raw_map_get_size((RawMap *)d_hash_set, size);
}

DResult d_hash_set_get_capacity(const DHashSet *d_hash_set, usize *capacity)
{
    return raw_map_get_capacity((RawMap *)d_hash_set, capacity);
}

DResult d_hash_set_insert(DHashSet *map, void *key)
{
    return raw_map_insert((RawMap *)map, key, key);
}

bool d_hash_set_key_exists(const DHashSet *map, void *key)
{
    return raw_map_get((RawMap *)map, key) != NULL;
}

DResult d_hash_set_remove(DHashSet *map, void *key)
{
    return raw_map_remove((RawMap *)map, key, NULL);
}

void d_hash_set_destroy(DHashSet **map)
{
    if (map == NULL || *map == NULL)
        return;
    DHashSet *d_map = *map;
    raw_map_free(&d_map->raw_map);
    free(d_map);
    *map = NULL;
}