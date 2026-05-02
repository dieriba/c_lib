#ifndef D_HASH_SET
#define D_HASH_SET
#include "d_types.h"
#include "container.h"

typedef struct DHashSet DHashSet;

DHashSet *d_hash_set_new(usize key_size, usize capacity, FnPtrGenHash hash_fn, FnPtrCmpKey cmp_fn, FnPtrFreeElem free_fn);
DHashSet *d_hash_set_insert(DHashSet *map, void *key);
bool d_hash_set_key_exists(DHashSet *map, void *key);
bool d_hash_set_remove(DHashSet *map, void *key);
void d_hash_set_destroy(DHashSet **map);
#endif