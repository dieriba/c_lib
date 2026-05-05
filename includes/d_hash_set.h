#ifndef D_HASH_SET
#define D_HASH_SET
#include "d_types.h"
#include "container.h"

typedef struct DHashSet DHashSet;

DResult d_hash_set_new(DHashSet **d_hash_set, usize key_size, usize capacity, FnPtrGenHash hash_fn, FnPtrCmpKey cmp_fn, FnPtrFreeElem free_fn);
DResult d_hash_set_insert(DHashSet *map, void *key);
bool d_hash_set_key_exists(const DHashSet *map, void *key);
DResult d_hash_set_remove(DHashSet *map, void *key);
DResult d_hash_set_get_size(const DHashSet *d_hash_set, usize *size);
DResult d_hash_set_get_capacity(const DHashSet *d_hash_set, usize *capacity);

void d_hash_set_destroy(DHashSet **map);

#endif