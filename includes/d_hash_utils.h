#ifndef D_HASH_UTILS_H
#define D_HASH_UTILS_H

#include "d_types.h"
#include "d_string_view.h"
#include "d_dyn_string.h"
#include "d_hash.h"
#include <string.h>
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
    return d_hash_raw_hash(s, strlen(s), 0);
}

static inline u64 hash_d_string_view_key(void *data)
{
    DStringView *s = (DStringView *)data;
    return d_hash_raw_hash(s->data, s->size, 0);
}

static inline u64 hash_d_dyn_string_key(void *data)
{
    DDynString *s = *((DDynString **)data);
    return d_hash_raw_hash(d_dyn_string_get_string(s), s->str.size, 0);
}

#define GENERATE_HASH_X_TYPE(KEY_TYPE_NAME, KEY_TYPE)        \
    static inline u64 hash_##KEY_TYPE_NAME##_key(void *data) \
    {                                                        \
        return d_hash_raw_hash(data, sizeof(KEY_TYPE), 0);      \
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