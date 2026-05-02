#ifndef D_UNORDERED_MAP
#define D_UNORDERED_MAP
#include "d_types.h"
#include "container.h"

typedef struct DUnorderedMap DUnorderedMap;

DUnorderedMap *d_unordered_map_new(usize key_size, usize value_size, usize capacity, FnPtrGenHash hash_fn, FnPtrCmpKey cmp_fn, FnPtrFreeElem free_fn);
DUnorderedMap *d_unordered_map_insert(DUnorderedMap *map, void *key, void *value);
void *d_unordered_map_get(DUnorderedMap *map, void *key);
bool d_unordered_map_remove(DUnorderedMap *map, void *key, void *out_elem);
void d_unordered_map_destroy(DUnorderedMap **map);
#endif