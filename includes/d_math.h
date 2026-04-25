#ifndef D_OVERFLOW_H
#define D_OVERFLOW_H

#include "d_types.h"

#if defined(__has_builtin)
# if __has_builtin(__builtin_add_overflow) && __has_builtin(__builtin_mul_overflow)
#  define D_HAS_BUILTIN_OVERFLOW 1
# else
#  define D_HAS_BUILTIN_OVERFLOW 0
# endif
#else
# define D_HAS_BUILTIN_OVERFLOW 0
#endif


#define D_OVERFLOW_CHECK_X(NAME, TYPE, MAX) \
    static inline bool d_overflow_check_add_##NAME(TYPE a, TYPE b, TYPE* result) \
    { \
        if (D_HAS_BUILTIN_OVERFLOW) \
            return __builtin_add_overflow(a, b, result); \
        else \
        { \
            if (a > (TYPE)(MAX - b)) \
                return true; \
            if (result) \
                *result = a + b; \
            return false; \
        }\
    } \
    \
    static inline bool d_overflow_check_mul_##NAME(TYPE a, TYPE b, TYPE* result) \
    { \
        if (D_HAS_BUILTIN_OVERFLOW) \
            return __builtin_mul_overflow(a, b, result); \
        else \
        { \
            if (a != 0 && b > (MAX / a)) \
                return true; \
            if (result) \
                *result = a * b; \
            return false;\
        }\
    } \
 

D_OVERFLOW_CHECK_X(u8, u8, UINT8_MAX)
D_OVERFLOW_CHECK_X(u16, u16, UINT16_MAX)
D_OVERFLOW_CHECK_X(u32, u32, UINT32_MAX)
D_OVERFLOW_CHECK_X(u64, u64, UINT64_MAX)
D_OVERFLOW_CHECK_X(usize, usize, MAX_SIZE_T_VALUE)
#endif