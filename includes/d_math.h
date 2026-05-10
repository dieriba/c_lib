#ifndef D_OVERFLOW_H
#define D_OVERFLOW_H

/**
 * @defgroup d_math Math Utilities
 * @{
 * @brief Overflow-safe arithmetic and power-of-two alignment helpers.
 *
 * Provides:
 *  - Per-type checked addition and multiplication via @ref D_MATH_OVERFLOW_CHECK_X
 *    (uses @c __builtin_add_overflow / @c __builtin_mul_overflow when available,
 *    falls back to portable checks otherwise).
 *  - Alignment macros: @ref d_math_align_round_up, @ref d_math_align_round_down.
 *  - Power-of-two utilities: @ref d_math_compute_pow2, @ref d_math_is_pow2,
 *    @ref d_math_mod_pow2.
 *
 * @warning All alignment and power-of-two macros expand their arguments more
 * than once — avoid expressions with side effects. Arguments are parenthesized,
 * so operator-precedence surprises are not a concern.
 *
 * @par Typical usage
 * @code{.c}
 *   usize a = SIZE_MAX, b = 1, result;
 *   if (d_math_overflow_check_add_usize(a, b, &result))
 *       puts("overflow!");
 *
 *   usize aligned = d_math_align_round_up(13, 8); // 16
 *   bool  is_p2   = d_math_is_pow2(16);           // true
 * @endcode
 */

#include "d_types.h"

/* -----------------------------------------------------------------------
 * Compiler built-in detection
 * -------------------------------------------------------------------- */

#if defined(__has_builtin)
#if __has_builtin(__builtin_add_overflow) && __has_builtin(__builtin_mul_overflow)
#define D_HAS_BUILTIN_OVERFLOW 1
#else
#define D_HAS_BUILTIN_OVERFLOW 0
#endif
#else
#define D_HAS_BUILTIN_OVERFLOW 0
#endif

/* -----------------------------------------------------------------------
 * D_MATH_OVERFLOW_CHECK_X — generates checked add/mul for one type
 * -------------------------------------------------------------------- */

/**
 * @brief Generates overflow-checked addition and multiplication for a type.
 *
 * Calling `D_MATH_OVERFLOW_CHECK_X(u32, u32, UINT32_MAX)` produces:
 *
 * ---
 *
 * **`bool d_math_overflow_check_add_u32(u32 a, u32 b, u32 *result)`**
 *
 * Adds @p a and @p b. Writes the sum into @p *result and returns @c false.
 * Returns @c true without writing @p *result if the addition would overflow
 * @p MAX. When @c __builtin_add_overflow is available @p result is passed
 * directly to the builtin — **@p result must not be NULL** in that case.
 * On compilers without the builtin @p result may be NULL (no-write on overflow).
 * For portability always pass a valid non-NULL pointer.
 *
 * ---
 *
 * **`bool d_math_overflow_check_mul_u32(u32 a, u32 b, u32 *result)`**
 *
 * Multiplies @p a and @p b. Writes the product into @p *result and returns
 * @c false. Returns @c true without writing @p *result if the multiplication
 * would overflow @p MAX. Same NULL-pointer caveat as the add variant above:
 * always pass a valid non-NULL @p result for portable behaviour.
 * Uses @c __builtin_mul_overflow when available.
 *
 * @param NAME Token appended to the generated function names (e.g. @c u32).
 * @param TYPE Underlying C type (e.g. @c u32).
 * @param MAX  Maximum representable value of @p TYPE (e.g. @c UINT32_MAX).
 */
#define D_MATH_OVERFLOW_CHECK_X(NAME, TYPE, MAX)                                      \
    static inline bool d_math_overflow_check_add_##NAME(TYPE a, TYPE b, TYPE *result) \
    {                                                                                 \
        if (D_HAS_BUILTIN_OVERFLOW)                                                   \
            return __builtin_add_overflow(a, b, result);                              \
        else                                                                          \
        {                                                                             \
            if (a > (TYPE)(MAX - b))                                                  \
                return true;                                                          \
            if (result)                                                               \
                *result = a + b;                                                      \
            return false;                                                             \
        }                                                                             \
    }                                                                                 \
                                                                                      \
    static inline bool d_math_overflow_check_mul_##NAME(TYPE a, TYPE b, TYPE *result) \
    {                                                                                 \
        if (D_HAS_BUILTIN_OVERFLOW)                                                   \
            return __builtin_mul_overflow(a, b, result);                              \
        else                                                                          \
        {                                                                             \
            if (a != 0 && b > (MAX / a))                                              \
                return true;                                                          \
            if (result)                                                               \
                *result = a * b;                                                      \
            return false;                                                             \
        }                                                                             \
    }

/** @brief Instantiates overflow-checked add/mul for ::u8. */
D_MATH_OVERFLOW_CHECK_X(u8,    u8,    UINT8_MAX)
/** @brief Instantiates overflow-checked add/mul for ::u16. */
D_MATH_OVERFLOW_CHECK_X(u16,   u16,   UINT16_MAX)
/** @brief Instantiates overflow-checked add/mul for ::u32. */
D_MATH_OVERFLOW_CHECK_X(u32,   u32,   UINT32_MAX)
/** @brief Instantiates overflow-checked add/mul for ::u64. */
D_MATH_OVERFLOW_CHECK_X(u64,   u64,   UINT64_MAX)
/** @brief Instantiates overflow-checked add/mul for ::usize. */
D_MATH_OVERFLOW_CHECK_X(usize, usize, MAX_SIZE_T_VALUE)

/* -----------------------------------------------------------------------
 * Alignment
 * -------------------------------------------------------------------- */

/**
 * @brief Rounds @p value up to the next multiple of @p alignment.
 *
 * @p alignment must be a power of two. Expands arguments more than once —
 * do not pass expressions with side effects.
 *
 * @code{.c}
 *   d_math_align_round_up(13, 8);  // 16
 *   d_math_align_round_up(16, 8);  // 16 (already aligned)
 * @endcode
 */
#define d_math_align_round_up(value, alignment) \
    (((value) + ((alignment) - 1)) & ~((alignment) - 1))

/**
 * @brief Rounds @p value down to the nearest multiple of @p alignment.
 *
 * @p alignment must be a power of two. Expands arguments more than once —
 * do not pass expressions with side effects.
 *
 * @code{.c}
 *   d_math_align_round_down(13, 8); // 8
 *   d_math_align_round_down(8,  8); // 8 (already aligned)
 * @endcode
 */
#define d_math_align_round_down(value, alignment) \
    ((value) & ~((alignment) - 1))

/* -----------------------------------------------------------------------
 * Power-of-two utilities
 * -------------------------------------------------------------------- */

/**
 * @brief Evaluates to `1 << exponent` (i.e. 2^exponent).
 *
 * @code{.c}
 *   d_math_compute_pow2(4); // 16
 * @endcode
 */
#define d_math_compute_pow2(exponent) (1 << (exponent))

/**
 * @brief Evaluates to @c true if @p value is a non-zero power of two.
 *
 * Uses the identity `value && !(value & (value - 1))`.
 *
 * @code{.c}
 *   d_math_is_pow2(0);  // false
 *   d_math_is_pow2(8);  // true
 *   d_math_is_pow2(6);  // false
 * @endcode
 */
#define d_math_is_pow2(value) ((value) && !((value) & ((value) - 1)))

/**
 * @brief Computes `numerator % modulus` when @p modulus is a power of two.
 *
 * Replaces the modulo operation with a bitwise AND: `numerator & (modulus - 1)`.
 * Only correct when @p modulus is a power of two.
 *
 * @code{.c}
 *   d_math_mod_pow2(13, 8); // 5  (13 % 8)
 *   d_math_mod_pow2(8,  8); // 0
 * @endcode
 */
#define d_math_mod_pow2(numerator, modulus) ((numerator) & ((modulus) - 1))

/** @} */ /* end of d_math group */

#endif
