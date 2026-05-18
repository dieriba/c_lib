#ifndef D_TYPES_H
#define D_TYPES_H

/**
 * @defgroup d_types Types
 * @{
 * @brief Fundamental integer type aliases shared across the entire library.
 *
 * Provides portable fixed-width integer typedefs. Error and comparison
 * result codes are defined in @ref d_error.
 *
 * Include this header directly or via any module header to access the
 * integer type definitions.
 */

#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>

/** @brief Maximum value representable by @c usize (i.e. @c SIZE_MAX). */
#define MAX_SIZE_T_VALUE SIZE_MAX

/** @name Fixed-width signed integer aliases */
/** @{ */
typedef int8_t int8;   /**< Signed  8-bit integer.  */
typedef int16_t int16; /**< Signed 16-bit integer.  */
typedef int32_t int32; /**< Signed 32-bit integer.  */
typedef int64_t int64; /**< Signed 64-bit integer.  */
/** @} */

/** @name Fixed-width unsigned integer aliases */
/** @{ */
typedef uint8_t u8;   /**< Unsigned  8-bit integer.                              */
typedef uint16_t u16; /**< Unsigned 16-bit integer.                              */
typedef uint32_t u32; /**< Unsigned 32-bit integer.                              */
typedef uint64_t u64; /**< Unsigned 64-bit integer.                              */
typedef size_t usize; /**< Platform-native unsigned size type (alias for size_t). */
/** @} */

/** @brief Callback that returns a heap-allocated string representation of an element.
 *
 *  Used by debug-print functions. The caller is responsible for freeing the
 *  returned string. */
typedef char *(*FnElemRepr)(void *elem);

#endif
