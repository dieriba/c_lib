#ifndef D_HASH_SET
#define D_HASH_SET
#include "d_types.h"
#include "container.h"
#include "d_general_lib.h"

typedef struct DHashSet DHashSet;

#define D_HASH_SET_NEW_WITH_NOT_OWNED_KEY(KEY_NAME, KEY_TYPE)                                                                                 \
    static inline DResult d_hash_set_new_not_owned_##KEY_NAME##_key(DHashSet **d_hash_set, usize capacity, FnPtrFreeElem value_destructor_fn) \
    {                                                                                                                                         \
        return d_hash_set_new(d_hash_set, sizeof(KEY_TYPE), capacity, hash_##KEY_NAME##_key, compare_##KEY_NAME, NULL, value_destructor_fn);  \
    }

#define D_HASH_SET_NEW_WITH_OWNED_KEY(KEY_NAME, KEY_TYPE, KEY_DESTRUCTOR)                                                                              \
    static inline DResult d_hash_set_new_owned_##KEY_NAME##_key(DHashSet **d_hash_set, usize capacity, FnPtrFreeElem value_destructor_fn)              \
    {                                                                                                                                                  \
        return d_hash_set_new(d_hash_set, sizeof(KEY_TYPE), capacity, hash_##KEY_NAME##_key, compare_##KEY_NAME, KEY_DESTRUCTOR, value_destructor_fn); \
    }

DResult d_hash_set_new(DHashSet **d_hash_set, usize key_size, usize capacity, FnPtrGenHash hash_fn, FnPtrCmpKey cmp_fn, FnPtrFreeElem key_destructor_fn, FnPtrFreeElem value_destructor_fn);
D_HASH_SET_NEW_WITH_NOT_OWNED_KEY(int8, int8)
D_HASH_SET_NEW_WITH_NOT_OWNED_KEY(int16, int16)
D_HASH_SET_NEW_WITH_NOT_OWNED_KEY(int32, int32)
D_HASH_SET_NEW_WITH_NOT_OWNED_KEY(int64, int64)
D_HASH_SET_NEW_WITH_NOT_OWNED_KEY(u8, u8)
D_HASH_SET_NEW_WITH_NOT_OWNED_KEY(u16, u16)
D_HASH_SET_NEW_WITH_NOT_OWNED_KEY(u32, u32)
D_HASH_SET_NEW_WITH_NOT_OWNED_KEY(u64, u64)
D_HASH_SET_NEW_WITH_NOT_OWNED_KEY(usize, usize)
D_HASH_SET_NEW_WITH_NOT_OWNED_KEY(bool, bool)
D_HASH_SET_NEW_WITH_NOT_OWNED_KEY(char, char)
D_HASH_SET_NEW_WITH_NOT_OWNED_KEY(d_string_view, DStringView)

D_HASH_SET_NEW_WITH_OWNED_KEY(str, char *, _free_str)
D_HASH_SET_NEW_WITH_OWNED_KEY(d_dyn_string, DDynString *, (FnPtrFreeElem)d_dyn_string_destroy)

DResult d_hash_set_insert(DHashSet *map, void *key);
bool d_hash_set_key_exists(const DHashSet *map, void *key);
DResult d_hash_set_delete(DHashSet *map, void *key);
DResult d_hash_set_remove(DHashSet *map, void *key, void *slot_key);
DResult d_hash_set_get_size(const DHashSet *d_hash_set, usize *size);
DResult d_hash_set_get_capacity(const DHashSet *d_hash_set, usize *capacity);

void d_hash_set_destroy(DHashSet **map);

#endif