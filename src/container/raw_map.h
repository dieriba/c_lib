#ifndef RAW_MAP_H
#define RAW_MAP_H
#include <stdlib.h>
#include "d_string_view.h"
#include "d_dyn_string.h"
#include "d_types.h"
#include "d_result.h"
#include "container.h"
#include "d_general_lib.h"
#include "wyhash.h"
typedef struct RawMap
{
    void *map;
    usize key_size;
    usize value_size;
    usize capacity;
    usize size;
    usize nb_groups;
    usize max_load_factor;
    FnPtrGenHash hash_fn;
    FnPtrFreeElem key_destructor_fn;
    FnPtrFreeElem value_destructor_fn;
    FnPtrCmpKey cmp_fn;
} RawMap;

DResult raw_map_init(RawMap *raw_map, usize key_size, usize value_size, usize capacity, FnPtrGenHash hash_fn, FnPtrCmpKey cmp_fn, FnPtrFreeElem key_destructor_fn, FnPtrFreeElem value_destructor_fn);
DResult raw_map_insert(RawMap *raw_map, void *new_key, void *new_value);

void *raw_map_get(RawMap *raw_map, void *key);
DResult raw_map_remove(RawMap *raw_map, void *key, void *slot_key, void *slot_value);
DResult raw_map_delete(RawMap *raw_map, void *key);
void raw_map_free(RawMap *raw_map);

#define RAW_MAP_GETTER(FIELD, FIELD_TYPE)                                               \
    static inline DResult raw_map_get_##FIELD(const RawMap *raw_map, FIELD_TYPE *FIELD) \
    {                                                                                   \
        if (raw_map == NULL || FIELD == NULL)                                           \
            return D_ERR_INVALID_ARG;                                                   \
        *FIELD = raw_map->FIELD;                                                        \
        return D_OK;                                                                    \
    }

RAW_MAP_GETTER(size, usize)
RAW_MAP_GETTER(capacity, usize)

static inline bool compare_str(void *s1, void *s2)
{
    return s1 && s2 && strcmp(*(char **)s1, *(char **)s2) == 0;
}

static inline bool compare_d_string_view(void *view1, void *view2)
{
    return view1 && view2 && d_string_view_compare(*(DStringView *)view1, *(DStringView *)view2);
}

static inline bool compare_d_dyn_string(void *d_dyn_str_1, void *d_dyn_str_2)
{
    return d_dyn_str_1 && d_dyn_str_2 && d_dyn_string_compare(*(DDynString **)d_dyn_str_1, *(DDynString **)d_dyn_str_2) == D_COMPARE_EQUAL;
}

static inline bool compare_ptr(void *a, void *b)
{
    return a == b;
}

#define GENERATE_COMPARE_X_TYPE(NAME, TYPE)             \
    static inline bool compare_##NAME(void *a, void *b) \
    {                                                   \
        return a && b && *(TYPE *)a == *(TYPE *)b;      \
    }

GENERATE_COMPARE_X_TYPE(int8, int8)
GENERATE_COMPARE_X_TYPE(int16, int16)
GENERATE_COMPARE_X_TYPE(int32, int32)
GENERATE_COMPARE_X_TYPE(int64, int64)

GENERATE_COMPARE_X_TYPE(u8, u8)
GENERATE_COMPARE_X_TYPE(u16, u16)
GENERATE_COMPARE_X_TYPE(u32, u32)
GENERATE_COMPARE_X_TYPE(u64, u64)

GENERATE_COMPARE_X_TYPE(usize, usize)
GENERATE_COMPARE_X_TYPE(bool, bool)
GENERATE_COMPARE_X_TYPE(char, char)

static inline u64 hash_str_key(void *data)
{
    char *s = *((char **)data);
    return wyhash(s, strlen(s), 0, _wyp);
}

static inline u64 hash_d_string_view_key(void *data)
{
    DStringView *s = (DStringView *)data;
    return wyhash(s->data, s->size, 0, _wyp);
}

static inline u64 hash_d_dyn_string_key(void *data)
{
    DDynString *s = *((DDynString **)data);
    return wyhash(d_dyn_string_get_string(s), s->str.size, 0, _wyp);
}

#define GENERATE_HASH_X_TYPE(KEY_TYPE_NAME, KEY_TYPE)        \
    static inline u64 hash_##KEY_TYPE_NAME##_key(void *data) \
    {                                                        \
        return wyhash(data, sizeof(KEY_TYPE), 0, _wyp);      \
    }

GENERATE_HASH_X_TYPE(int8, int8)
GENERATE_HASH_X_TYPE(int16, int16)
GENERATE_HASH_X_TYPE(int32, int32)
GENERATE_HASH_X_TYPE(int64, int64)

GENERATE_HASH_X_TYPE(u8, u8)
GENERATE_HASH_X_TYPE(u16, u16)
GENERATE_HASH_X_TYPE(u32, u32)
GENERATE_HASH_X_TYPE(u64, u64)

GENERATE_HASH_X_TYPE(usize, usize)

GENERATE_HASH_X_TYPE(bool, bool)

GENERATE_HASH_X_TYPE(char, char)

#endif