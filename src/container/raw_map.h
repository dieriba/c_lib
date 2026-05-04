#ifndef RAW_MAP_H
#define RAW_MAP_H
#include "d_types.h"
#include "container.h"
#include "d_general_lib.h"
#include <stdlib.h>
typedef struct RawMap
{
    void *map;
    usize key_size;
    usize value_size;
    usize capacity;
    usize size;
    usize nb_groups;
    usize max_load_factor;
    FnPtrGenHash hash_fn;
    FnPtrFreeElem free_fn;
    FnPtrCmpKey cmp_fn;
} RawMap;

DResult raw_map_init(RawMap *raw_map, usize key_size, usize value_size, usize capacity, FnPtrGenHash hash_fn, FnPtrCmpKey cmp_fn, FnPtrFreeElem free_fn);
DResult raw_map_insert(RawMap *raw_map, void *new_key, void *new_value);

void *raw_map_get(RawMap *raw_map, void *key);
bool raw_map_remove(RawMap *raw_map, void *key, void *out_elem);
void raw_map_free(RawMap *raw_map);

#define RAW_MAP_GETTER(FIELD, FIELD_TYPE)                                               \
    static inline DResult raw_map_get_##FIELD(const RawMap *raw_map, FIELD_TYPE *FIELD) \
    {                                                                                   \
        if (raw_map == NULL || FIELD == NULL)                                           \
            return D_ERR_INVALID_ARG;                                                   \
        *FIELD = raw_map->FIELD;                                                        \
        return D_OK;                                                                    \
    }

RAW_MAP_GETTER(size, usize)
RAW_MAP_GETTER(capacity, usize)
#endif