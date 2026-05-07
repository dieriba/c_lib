#include "raw_map.h"
#include "d_unordered_map.h"
#include <stdlib.h>

#define D_UNORDERED_MAP_DEFINE_TYPED_CTOR(KEY_TYPE, KEY_NAME)                                                                                    \
    DResult d_unordered_map_new_##KEY_NAME(DUnorderedMap **d_unordered_map, usize value_size, usize capacity, FnPtrFreeElem free_fn)             \
    {                                                                                                                                            \
        return d_unordered_map_new(d_unordered_map, sizeof(KEY_TYPE), value_size, capacity, hash_##KEY_NAME##_key, compare_##KEY_NAME, free_fn); \
    }

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
    else if ((*d_unordered_map = d_unordered_map_new_raw()) == NULL)
        return D_ERR_ALLOC;
    return raw_map_init((RawMap *)(*d_unordered_map), key_size, value_size, capacity, hash_fn, cmp_fn, free_fn);
}

DResult d_unordered_map_new_str(DUnorderedMap **d_unordered_map, usize value_size, usize capacity, FnPtrFreeElem free_fn)
{
    return d_unordered_map_new(d_unordered_map, sizeof(char *), value_size, capacity, hash_string_key, compare_str, free_fn);
}

D_UNORDERED_MAP_DEFINE_TYPED_CTOR(int8, int8)
D_UNORDERED_MAP_DEFINE_TYPED_CTOR(int16, int16)
D_UNORDERED_MAP_DEFINE_TYPED_CTOR(int32, int32)
D_UNORDERED_MAP_DEFINE_TYPED_CTOR(int64, int64)
D_UNORDERED_MAP_DEFINE_TYPED_CTOR(u8, u8)
D_UNORDERED_MAP_DEFINE_TYPED_CTOR(u16, u16)
D_UNORDERED_MAP_DEFINE_TYPED_CTOR(u32, u32)
D_UNORDERED_MAP_DEFINE_TYPED_CTOR(u64, u64)
D_UNORDERED_MAP_DEFINE_TYPED_CTOR(usize, usize)
D_UNORDERED_MAP_DEFINE_TYPED_CTOR(bool, bool)
D_UNORDERED_MAP_DEFINE_TYPED_CTOR(char, char)

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