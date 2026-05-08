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

DResult d_hash_set_new(DHashSet **d_hash_set, usize key_size, usize capacity, FnPtrGenHash hash_fn, FnPtrCmpKey cmp_fn, FnPtrFreeElem key_destructor_fn, FnPtrFreeElem value_destructor_fn)
{
    if (d_hash_set == NULL || key_size == 0)
        return D_ERR_INVALID_ARG;
    else if ((*d_hash_set = d_hash_set_new_raw()) == NULL)
        return D_ERR_ALLOC;
    DResult op_result = raw_map_init((RawMap *)*d_hash_set, key_size, VALUE_SIZE, capacity, hash_fn, cmp_fn, key_destructor_fn, value_destructor_fn);
    if (op_result != D_OK)
    {
        free(*d_hash_set);
        *d_hash_set = NULL;
    }
    return op_result;
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

void d_hash_set_destroy(DHashSet **map)
{
    if (map == NULL || *map == NULL)
        return;
    DHashSet *d_map = *map;
    raw_map_free(&d_map->raw_map);
    free(d_map);
    *map = NULL;
}