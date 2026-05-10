#ifndef D_HASH_H
#define D_HASH_H

/**
 * @defgroup d_hash Hash
 * @{
 * @brief Low-level wyhash wrapper used internally by the hash map and set.
 */

#include "d_types.h"

/**
 * @brief Computes a 64-bit wyhash of @p len bytes starting at @p data.
 *
 * This is the raw hashing primitive used by all hash containers in the library.
 * Callers who need a stable, general-purpose hash may use it directly, but it
 * is not guaranteed to be consistent across library versions.
 *
 * @param data Pointer to the bytes to hash. Must not be NULL.
 * @param len  Number of bytes to read from @p data.
 * @param seed 64-bit seed value. Pass @c 0 for the default (unseeded) hash.
 * @return A 64-bit hash value. Distribution is uniform for wyhash inputs.
 *
 * @code{.c}
 *   const char *key = "hello";
 *   u64 h = d_hash_raw_hash((void *)key, strlen(key), 0);
 * @endcode
 */
u64 d_hash_raw_hash(void *data, size_t len, u64 seed);

/** @} */ /* end of d_hash group */

#endif
