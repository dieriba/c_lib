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

DHashSet *d_hash_set_new(usize key_size, usize capacity, FnPtrGenHash hash_fn, FnPtrCmpKey cmp_fn, FnPtrFreeElem free_fn)
{
    DHashSet *d_hash_set = d_hash_set_new_raw();
    if (d_hash_set == NULL)
        return NULL;
    if (raw_map_init(&d_hash_set->raw_map, key_size, VALUE_SIZE, capacity, hash_fn, cmp_fn, free_fn) != D_OK)
        return NULL;
    return d_hash_set;
}

DHashSet *d_hash_set_insert(DHashSet *map, void *key)
{
    return raw_map_insert((RawMap *)map, key, key);
}

bool d_hash_set_key_exists(DHashSet *map, void *key)
{
    return raw_map_get((RawMap *)map, key) != NULL;
}

bool d_hash_set_remove(DHashSet *map, void *key)
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