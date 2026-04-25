#include "d_types.h"

#define D_TOGGLE_BIT(bits, bit) (bits ^ ((usize)1 << bit))
#define D_CLEAR_BIT(bits, bit) (bits & ~((usize)1 << bit))

typedef u_int8_t DBits8;
typedef uint16_t DBits16;
typedef uint32_t DBits32;
typedef uint64_t DBits64;

#define D_BITS_X_API(NAME, TYPE, NB_BITS)                                             \
    static inline bool NAME##_check_bits_set(TYPE bits_set, const TYPE bits_to_check) \
    {                                                                                 \
        return (bits_set & bits_to_check) == bits_to_check;                           \
    }                                                                                 \
                                                                                      \
    static inline bool NAME##_set_bits(TYPE *bits_set, const TYPE bits_to_set)        \
    {                                                                                 \
        if (!bits_set)                                                                \
            return false;                                                             \
        *bits_set = *bits_set | bits_to_set;                                          \
        return true;                                                                  \
    }                                                                                 \
    static inline bool NAME##_unset_bits(TYPE *bits_set, const TYPE bits_to_unset)    \
    {                                                                                 \
        if (!bits_set)                                                                \
            return false;                                                             \
        *bits_set = *bits_set & ~bits_to_unset;                                       \
        return true;                                                                  \
    }                                                                                 \
    static inline bool NAME##_assign(TYPE *bits_set, TYPE bits)                       \
    {                                                                                 \
        if (!bits_set)                                                                \
            return false;                                                             \
        *bits_set = bits;                                                             \
        return true;                                                                  \
    }                                                                                 \
    static inline bool NAME##_toggle_bit(TYPE *bits_set, usize bit)                   \
    {                                                                                 \
        if (!bits_set || bit >= (usize)NB_BITS)                                       \
            return false;                                                             \
        *bits_set = D_TOGGLE_BIT(*bits_set, bit);                                     \
        return true;                                                                  \
    }                                                                                 \
    static inline bool NAME##_clear_bit(TYPE *bits_set, usize bit)                    \
    {                                                                                 \
        if (!bits_set || bit >= (usize)NB_BITS)                                       \
            return false;                                                             \
        *bits_set = D_CLEAR_BIT(*bits_set, bit);                                      \
        return true;                                                                  \
    }                                                                                 \
    static inline bool NAME##_reset_all_bits(TYPE *bits_set)                          \
    {                                                                                 \
        if (!bits_set)                                                                \
            return false;                                                             \
        *bits_set = 0;                                                                \
        return true;                                                                  \
    }

D_BITS_X_API(d_bits_8, u8, 8)
D_BITS_X_API(d_bits_16, u16, 16)
D_BITS_X_API(d_bits_32, u32, 32)
D_BITS_X_API(d_bits_64, u64, 64)