#ifndef GENERAL_LIB_H
#define GENERAL_LIB_H

/**
 * @defgroup d_general_lib General Utilities
 * @{
 * @brief Shared utility types, string helpers, and memory utilities.
 *
 * Provides the function-pointer typedefs used throughout the library
 * (hash, compare, destructor), a handful of string utilities (@ref d_substr,
 * @ref d_strdup, @ref d_itoa_ll, @ref d_itoa_usize), string splitting
 * wrappers, and the @ref memfill bulk initialiser.
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "d_types.h"

/**
 * @brief Default destructor for @c char * elements stored by pointer.
 *
 * Dereferences the slot pointer to obtain the @c char * and calls @c free on it.
 * Pass this as @c key_destructor_fn or @c value_destructor_fn when the container
 * holds heap-allocated C strings.
 *
 * @param elem Pointer to the @c char * slot inside the container.
 *
 * @code{.c}
 *   DHashSet set;
 *   d_hash_set_init_owned_str_key(&set, 16, NULL); // uses _free_str internally
 * @endcode
 */
static inline void _free_str(void *elem)
{
    free(*(char **)elem);
}

/**
 * @brief Extracts a heap-allocated substring from @p str.
 *
 * Copies up to @p size characters starting at byte offset @p pos. If
 * @c pos + @p size exceeds the string length, the copy is clamped to the end.
 * The result is always null-terminated. The caller is responsible for freeing
 * the returned pointer.
 *
 * @param str  Null-terminated source string. Must not be NULL.
 * @param pos  Zero-based start offset. If @p pos is strictly greater than
 *             @c strlen(str), returns NULL.
 * @param size Number of characters to copy. If @p pos equals @c strlen(str)
 *             or the effective copy length is 0, returns an empty allocated
 *             string @c "".
 * @return Newly allocated substring, @c "" (empty string) if the effective
 *         length is 0 and @p pos is in-range, or NULL if @p pos exceeds the
 *         string length or allocation fails.
 *
 * @code{.c}
 *   char *s = d_substr("hello world", 6, 5); // s == "world"
 *   free(s);
 *
 *   char *e = d_substr("hi", 2, 10); // pos == strlen → returns ""
 *   free(e);
 * @endcode
 */
char *d_substr(const char *str, usize pos, usize size);

/**
 * @brief Creates a heap-allocated duplicate of @p str.
 *
 * Implemented as @c d_substr(str, 0, strlen(str)). The caller is responsible
 * for freeing the returned pointer.
 *
 * @param str Null-terminated string to duplicate. Must not be NULL.
 * @return Newly allocated copy of @p str, or NULL on allocation failure.
 *
 * @code{.c}
 *   char *copy = d_strdup("hello");
 *   free(copy);
 * @endcode
 */
char *d_strdup(const char *str);

/**
 * @brief Converts a @c long @c long to a heap-allocated decimal string.
 *
 * The caller is responsible for freeing the returned pointer.
 *
 * @param nbr Value to convert.
 * @return Newly allocated null-terminated string, or NULL on allocation failure.
 *
 * @code{.c}
 *   char *s = d_itoa_ll(-9223372036854775807LL);
 *   puts(s);
 *   free(s);
 * @endcode
 */
char *d_itoa_ll(long long nbr);

/**
 * @brief Converts a @c long @c long to a decimal string using a caller-supplied buffer.
 *
 * No allocation is performed. The caller must supply a buffer large enough for
 * the result including the null terminator. For @c long @c long the worst case
 * is @c LLONG_MIN (@c -9223372036854775808), which requires **22 bytes**
 * (20 digits + sign + null terminator).
 *
 * @param nbr    Value to convert.
 * @param buffer Destination buffer. Must not be NULL and must be at least 22 bytes.
 * @return @p buffer.
 *
 * @code{.c}
 *   char buf[22];
 *   puts(d_itoa_ll_no_alloc(42LL, buf));
 * @endcode
 */
char *d_itoa_ll_no_alloc(long long nbr, char *buffer);

/**
 * @brief Converts a @c usize value to a heap-allocated decimal string.
 *
 * The caller is responsible for freeing the returned pointer.
 *
 * @param nb Value to convert.
 * @return Newly allocated null-terminated string, or NULL on allocation failure.
 *
 * @code{.c}
 *   char *s = d_itoa_usize(12345);
 *   free(s);
 * @endcode
 */
char *d_itoa_usize(usize nb);

/**
 * @brief Converts a @c usize value to a decimal string using a caller-supplied buffer.
 *
 * No allocation is performed. On 64-bit platforms @c usize can be up to
 * 18446744073709551615 (20 digits), so the buffer must be at least **21 bytes**
 * (20 digits + null terminator). 12 bytes suffice on 32-bit platforms.
 *
 * @param nb         Value to convert.
 * @param raw_buffer Destination buffer. Must not be NULL and must be at least
 *                   21 bytes on 64-bit platforms.
 * @return @p raw_buffer.
 *
 * @code{.c}
 *   char buf[21];
 *   puts(d_itoa_usize_no_alloc(SIZE_MAX, buf));
 * @endcode
 */
char *d_itoa_usize_no_alloc(usize nb, char *raw_buffer);

/**
 * @brief Fills a buffer with repeated copies of a fixed-size element.
 *
 * Uses an exponential doubling strategy: the first element is copied manually,
 * then successive @c memcpy calls double the filled region until all
 * @p nb_slot_to_fill slots are initialised. This is O(log n) @c memcpy calls.
 *
 * @param dst              Destination buffer. Must not be NULL.
 * @param filler_elem      Pointer to the element to replicate. Must not be NULL.
 * @param filler_elem_size Size in bytes of a single element. Must be > 0.
 * @param nb_slot_to_fill  Number of element-sized slots to fill.
 *                         If 0, @p dst is returned immediately without any writes.
 * @return @p dst on success, or NULL if @p dst is NULL, @p filler_elem is NULL,
 *         or @p filler_elem_size is 0.
 *
 * @code{.c}
 *   int arr[8];
 *   int zero = 0;
 *   memfill(arr, &zero, sizeof(int), 8); // arr is now {0, 0, 0, 0, 0, 0, 0, 0}
 * @endcode
 */
void *memfill(void *dst, void *filler_elem, usize filler_elem_size, usize nb_slot_to_fill);

/** @} */ /* end of d_general_lib group */

#define PSEUDO_FAST_STRCMP(s1, s2) ((s1[0] == s2[0]) && strcmp(s1, s2) == 0)

#endif
