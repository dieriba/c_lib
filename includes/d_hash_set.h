#ifndef D_HASH_SET
#define D_HASH_SET
#include "d_types.h"
#include "container.h"
#include "d_general_lib.h"

#define D_HASH_SET_TYPED_CTORS(KEY_TYPE, KEY_NAME) \
    DResult d_hash_set_new_##KEY_NAME##_key(DHashSet **d_hash_set, usize capacity, FnPtrFreeElem free_fn);

typedef struct DHashSet DHashSet;

DResult d_hash_set_new(DHashSet **d_hash_set, usize key_size, usize capacity, FnPtrGenHash hash_fn, FnPtrCmpKey cmp_fn, FnPtrFreeElem free_fn);
DResult d_hash_set_new_str(DHashSet **d_hash_set, usize capacity, FnPtrFreeElem free_fn);
D_HASH_SET_TYPED_CTORS(int8, int8)
D_HASH_SET_TYPED_CTORS(int16, int16)
D_HASH_SET_TYPED_CTORS(int32, int32)
D_HASH_SET_TYPED_CTORS(int64, int64)
D_HASH_SET_TYPED_CTORS(u8, u8)
D_HASH_SET_TYPED_CTORS(u16, u16)
D_HASH_SET_TYPED_CTORS(u32, u32)
D_HASH_SET_TYPED_CTORS(u64, u64)
D_HASH_SET_TYPED_CTORS(usize, usize)
D_HASH_SET_TYPED_CTORS(bool, bool)
D_HASH_SET_TYPED_CTORS(char, char)

DResult d_hash_set_insert(DHashSet *map, void *key);
bool d_hash_set_key_exists(const DHashSet *map, void *key);
DResult d_hash_set_remove(DHashSet *map, void *key);
DResult d_hash_set_get_size(const DHashSet *d_hash_set, usize *size);
DResult d_hash_set_get_capacity(const DHashSet *d_hash_set, usize *capacity);

void d_hash_set_destroy(DHashSet **map);

#endif