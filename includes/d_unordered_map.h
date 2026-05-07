#ifndef D_UNORDERED_MAP
#define D_UNORDERED_MAP
#include "d_types.h"
#include "container.h"
#include "d_general_lib.h"

#define D_UNORDERED_MAP_TYPED_CTORS(KEY_TYPE, KEY_NAME) \
    DResult d_unordered_map_new_##KEY_NAME##_key(DUnorderedMap **d_unordered_map, usize value_size, usize capacity, FnPtrFreeElem free_fn);

typedef struct DUnorderedMap DUnorderedMap;

DResult d_unordered_map_new(DUnorderedMap **d_unordered_map, usize key_size, usize value_size, usize capacity, FnPtrGenHash hash_fn, FnPtrCmpKey cmp_fn, FnPtrFreeElem free_fn);
DResult d_unordered_map_new_str(DUnorderedMap **d_unordered_map, usize value_size, usize capacity, FnPtrFreeElem free_fn);
D_UNORDERED_MAP_TYPED_CTORS(int8, int8)
D_UNORDERED_MAP_TYPED_CTORS(int16, int16)
D_UNORDERED_MAP_TYPED_CTORS(int32, int32)
D_UNORDERED_MAP_TYPED_CTORS(int64, int64)
D_UNORDERED_MAP_TYPED_CTORS(u8, u8)
D_UNORDERED_MAP_TYPED_CTORS(u16, u16)
D_UNORDERED_MAP_TYPED_CTORS(u32, u32)
D_UNORDERED_MAP_TYPED_CTORS(u64, u64)
D_UNORDERED_MAP_TYPED_CTORS(usize, usize)
D_UNORDERED_MAP_TYPED_CTORS(bool, bool)
D_UNORDERED_MAP_TYPED_CTORS(char, char)

DResult d_unordered_map_insert(DUnorderedMap *map, void *key, void *value);
void *d_unordered_map_get(const DUnorderedMap *map, void *key);
DResult d_unordered_map_remove(DUnorderedMap *map, void *key, void *out_elem);
void d_unordered_map_destroy(DUnorderedMap **map);
DResult d_unordered_map_get_size(const DUnorderedMap *d_unordered_map, usize *size);
DResult d_unordered_map_get_capacity(const DUnorderedMap *d_unordered_map, usize *capacity);
#endif