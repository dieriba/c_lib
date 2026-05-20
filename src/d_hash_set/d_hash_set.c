#include <stdio.h>
#include <stdlib.h>
#include "raw_map.h"
#include "d_hash_set.h"

#define VALUE_SIZE 0UL

ASSERT_FIRST_FIELD(DHashSet, raw_map);

DResult d_hash_set_init(DHashSet *d_hash_set, usize key_size, usize capacity, FnPtrGenHash hash_fn, FnPtrCmpKey cmp_fn, DestroyElemFn key_destructor_fn)
{
    if (d_hash_set == NULL || key_size == 0)
        return D_ERR_INVALID_ARG;
    return raw_map_init((RawMap *)d_hash_set, key_size, VALUE_SIZE, capacity, hash_fn, cmp_fn, key_destructor_fn, NULL);
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

DResult d_hash_set_delete(DHashSet *map, void *key)
{
    return raw_map_delete((RawMap *)map, key);
}

DResult d_hash_set_remove(DHashSet *map, void *key, void *slot_key)
{
    if (slot_key == NULL)
        return D_ERR_INVALID_ARG;
    return raw_map_remove((RawMap *)map, key, slot_key, NULL);
}

void d_hash_set_destroy(DHashSet *map)
{
    if (map == NULL)
        return;
    raw_map_free((RawMap *)map);
    memset(map, 0, sizeof(DHashSet));
}