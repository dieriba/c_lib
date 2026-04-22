#include "d_types.h"

#define D_GET_BIT(bits, bit) (bits & ((usize)1 << bit))
#define D_SET_BIT(bits, bit) (bits | ((usize)1 << bit))
#define D_TOGGLE_BIT(bits, bit) (bits ^ ((usize)1 << bit))
#define D_CLEAR_BIT(bits, bit) (bits & ~((usize)1 << bit))

typedef struct DBits8
{
    u8 bits;
} DBits8;

typedef struct DBits16
{
    u16 bits;
} DBits16;

typedef struct DBits32
{
    u32 bits;
} DBits32;

typedef struct DBits64
{
    u64 bits;
} DBits64;

#define D_BITS_X_API(NAME, TYPE, NB_BITS)                            \
    static inline bool NAME##_check_bit_set(TYPE bytes, usize bit) \
    {                                                              \
        return bit < (usize)NB_BITS && D_GET_BIT(bytes.bits, bit); \
    }                                                              \
                                                                   \
    static inline bool NAME##_set_bit(TYPE *bytes, usize bit)      \
    {                                                              \
        if (!bytes || bit >= (usize)NB_BITS)                       \
            return false;                                          \
        bytes->bits = D_SET_BIT(bytes->bits, bit);                 \
        return true;                                               \
    }                                                              \
                                                                   \
    static inline bool NAME##_toggle_bit(TYPE *bytes, usize bit)   \
    {                                                              \
        if (!bytes || bit >= (usize)NB_BITS)                       \
            return false;                                          \
        bytes->bits = D_TOGGLE_BIT(bytes->bits, bit);              \
        return true;                                               \
    }                                                              \
    static inline bool NAME##_clear_bit(TYPE *bytes, usize bit)    \
    {                                                              \
        if (!bytes || bit >= (usize)NB_BITS)                       \
            return false;                                          \
        bytes->bits = D_CLEAR_BIT(bytes->bits, bit);               \
        return true;                                               \
    }                                                              \
    static inline bool NAME##_reset_all_bits(TYPE *bytes)          \
    {                                                              \
        if (!bytes)                                                \
            return false;                                          \
        bytes->bits = 0;                                           \
        return true;                                               \
    }

D_BITS_X_API(d_bits_8, DBits8, 8)
D_BITS_X_API(d_bits_16, DBits16, 16)
D_BITS_X_API(d_bits_32, DBits32, 32)
D_BITS_X_API(d_bits_64, DBits64, 64)