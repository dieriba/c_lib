#ifndef D_UNORDERED_MAP
#define D_UNORDERED_MAP
#include "d_types.h"
#include "container.h"
#include "d_general_lib.h"

typedef struct DUnorderedMap
{
    RawMap raw_map;
} DUnorderedMap;

DResult d_unordered_map_init(DUnorderedMap *d_unordered_map, usize key_size, usize value_size, usize capacity, FnPtrGenHash hash_fn, FnPtrCmpKey cmp_fn, FnPtrFreeElem key_destructor_fn, FnPtrFreeElem value_destructor_fn);

#define D_UNORDERED_MAP_INIT_WITH_NOT_OWNED_KEY(KEY_NAME, KEY_TYPE)                                                                                                            \
    static inline DResult d_unordered_map_init_not_owned_##KEY_NAME##_key(DUnorderedMap *d_unordered_map, usize value_size, usize capacity, FnPtrFreeElem value_destructor_fn) \
    {                                                                                                                                                                          \
        return d_unordered_map_init(d_unordered_map, sizeof(KEY_TYPE), value_size, capacity, hash_##KEY_NAME##_key, compare_##KEY_NAME, NULL, value_destructor_fn);            \
    }

#define D_UNORDERED_MAP_INIT_WITH_OWNED_KEY(KEY_NAME, KEY_TYPE, KEY_DESTRUCTOR)                                                                                               \
    static inline DResult d_unordered_map_init_owned_##KEY_NAME##_key(DUnorderedMap *d_unordered_map, usize value_size, usize capacity, FnPtrFreeElem value_destructor_fn)    \
    {                                                                                                                                                                         \
        return d_unordered_map_init(d_unordered_map, sizeof(KEY_TYPE), value_size, capacity, hash_##KEY_NAME##_key, compare_##KEY_NAME, KEY_DESTRUCTOR, value_destructor_fn); \
    }

D_UNORDERED_MAP_INIT_WITH_NOT_OWNED_KEY(int8, int8)
D_UNORDERED_MAP_INIT_WITH_NOT_OWNED_KEY(int16, int16)
D_UNORDERED_MAP_INIT_WITH_NOT_OWNED_KEY(int32, int32)
D_UNORDERED_MAP_INIT_WITH_NOT_OWNED_KEY(int64, int64)
D_UNORDERED_MAP_INIT_WITH_NOT_OWNED_KEY(u8, u8)
D_UNORDERED_MAP_INIT_WITH_NOT_OWNED_KEY(u16, u16)
D_UNORDERED_MAP_INIT_WITH_NOT_OWNED_KEY(u32, u32)
D_UNORDERED_MAP_INIT_WITH_NOT_OWNED_KEY(u64, u64)
D_UNORDERED_MAP_INIT_WITH_NOT_OWNED_KEY(usize, usize)
D_UNORDERED_MAP_INIT_WITH_NOT_OWNED_KEY(bool, bool)
D_UNORDERED_MAP_INIT_WITH_NOT_OWNED_KEY(char, char)
D_UNORDERED_MAP_INIT_WITH_NOT_OWNED_KEY(str, char *)
D_UNORDERED_MAP_INIT_WITH_NOT_OWNED_KEY(d_dyn_string, DDynString *)

D_UNORDERED_MAP_INIT_WITH_NOT_OWNED_KEY(d_string_view, DStringView)

D_UNORDERED_MAP_INIT_WITH_OWNED_KEY(str, char *, _free_str)
D_UNORDERED_MAP_INIT_WITH_OWNED_KEY(d_dyn_string, DDynString *, (FnPtrFreeElem)d_dyn_string_destroy)

DResult d_unordered_map_insert(DUnorderedMap *map, void *key, void *value);
void *d_unordered_map_get(const DUnorderedMap *map, void *key);
DResult d_unordered_map_remove(DUnorderedMap *map, void *key, void *slot_key, void *slot_value);
DResult d_unordered_map_delete(DUnorderedMap *map, void *key);
void d_unordered_map_destroy(DUnorderedMap *map);
DResult d_unordered_map_get_size(const DUnorderedMap *d_unordered_map, usize *size);
DResult d_unordered_map_get_capacity(const DUnorderedMap *d_unordered_map, usize *capacity);
#endif