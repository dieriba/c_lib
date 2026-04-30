#ifndef D_UNORDERED_MAP
#define D_UNORDERED_MAP
#include "d_types.h"
#include "raw_map.h"
typedef struct _DUnorderedMap DUnorderedMap;

DUnorderedMap *d_unordered_map_new(usize elem_size, usize capacity, HashFn hash_fn, CmpFn key_eq_fn, FreeFn free_fn);
DUnorderedMap *d_unordered_map_insert(DUnorderedMap *map, void *key, void *value);
void *d_unordered_map_get(DUnorderedMap *map, void *key);
void *d_unordered_map_remove(DUnorderedMap *map, void *key, void *out_elem);
void d_unordered_map_destroy(DUnorderedMap **map);
#endif