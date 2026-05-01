#include "d_types.h"
#include <strings.h>

#if defined(__has_builtin)

#if __has_builtin(__builtin_ffs)
#define USE_BUILTIN_FFS 1
#else
#define USE_BUILTIN_FFS 0
#endif

#if __has_builtin(__builtin_ffsll)
#define USE_BUILTIN_FFSLL 1
#else
#define USE_BUILTIN_FFSLL 0
#endif

#endif

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

#define d_bits_clear_least_significant_bit_set(value) value &(value - 1)

static inline int d_bits_clear_most_significant_bit_set_int(int value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;

    return value & (value >> 1);
}

static inline long long int d_bits_clear_most_significant_bit_set_ll(long long int value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;

    return value & (value >> 1);
}

static inline int d_bits_get_index_least_significant_bit_set_int(int value)
{
#if USE_BUILTIN_FFS
    return __builtin_ffs(value);
#else
    static const unsigned char table[] =
        {
            0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};
    unsigned int a;
    unsigned int x = value & -value; // keep only the lowest set bit.

    a = x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ? 16 : 24);

    return table[x >> a] + a;
#endif
}

static inline int d_bits_get_index_least_significant_bit_set_ll(long long int value)
{
#if USE_BUILTIN_FFSLL
    return __builtin_ffsll(value);
#else
    unsigned usize x = value & -value;

    if (x <= 0xffffffff)
        return d_bits_get_index_least_significant_bit_set_int(value);
    else
        return 32 + d_bits_get_index_least_significant_bit_set_int(value >> 32);
#endif
}

static inline int d_bits_get_index_most_significant_bit_set_int(int value)
{
    value = d_bits_clear_most_significant_bit_set_int(value);
    return d_bits_get_index_least_significant_bit_set_int(value);
}

static inline int d_bits_get_index_most_significant_bit_set_ll(long long int value)
{
    value = d_bits_clear_most_significant_bit_set_ll(value);
    return d_bits_get_index_least_significant_bit_set_ll(value);
}