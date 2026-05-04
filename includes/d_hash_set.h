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

#define D_HASH_SET_GETTER(FIELD, FIELD_TYPE)                                                    \
    static inline DResult d_hash_set_get_##FIELD(const DHashSet *d_hash_set, FIELD_TYPE *FIELD) \
    {                                                                                           \
        return raw_map_get_##FIELD((RawMap *)d_hash_set, FIELD);                                \
    }

D_HASH_SET_GETTER(size, usize)
D_HASH_SET_GETTER(capacity, usize)

#endif