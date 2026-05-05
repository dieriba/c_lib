#ifndef D_UNORDERED_MAP
#define D_UNORDERED_MAP
#include "d_types.h"
#include "container.h"

typedef struct DUnorderedMap DUnorderedMap;

DResult d_unordered_map_new(DUnorderedMap **d_unordered_map, usize key_size, usize value_size, usize capacity, FnPtrGenHash hash_fn, FnPtrCmpKey cmp_fn, FnPtrFreeElem free_fn);
DResult d_unordered_map_insert(DUnorderedMap *map, void *key, void *value);
void *d_unordered_map_get(const DUnorderedMap *map, void *key);
DResult d_unordered_map_remove(DUnorderedMap *map, void *key, void *out_elem);
void d_unordered_map_destroy(DUnorderedMap **map);
DResult d_unordered_map_get_size(const DUnorderedMap *d_unordered_map, usize *size);
DResult d_unordered_map_get_capacity(const DUnorderedMap *d_unordered_map, usize *capacity);
#endif