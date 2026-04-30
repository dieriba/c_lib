#ifndef KEY_H
#define KEY_H

#include <string.h>
#include "d_types.h"


static inline bool compare_str(void* s1, void* s2)
{
    return s1 && s2 && strcmp(*(char**)s1, *(char**)s2) == 0;
}

static inline bool compare_ptr(void* a, void* b)
{
    return a == b;
}

#define GENERATE_COMPARE_X_TYPE(NAME, TYPE) \
    static inline bool compare_##NAME(TYPE* a, TYPE* b) \
    { \
        return a && b && *a == *b; \
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
#endif