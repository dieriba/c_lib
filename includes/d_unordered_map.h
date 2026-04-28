#include "d_types.h"
#ifndef D_UNORDERED_MAP
#define D_UNORDERED_MAP

typedef struct _DUnorderedMap DUnorderedMap;

typedef usize (*HashFn)(void *key);
typedef void (*FreeFn)(void *elem);

DUnorderedMap *d_unordered_map_new(usize elem_size, usize capacity, HashFn hash_fn, FreeFn free_fn);
DUnorderedMap *d_unordered_map_insert(DUnorderedMap *map, void *key, void *value);
void *d_unordered_map_get(DUnorderedMap *map, void *key);
void *d_unordered_map_remove(DUnorderedMap *map, void *key, void *out_elem);
void d_unordered_map_destroy(DUnorderedMap **map);
#endif