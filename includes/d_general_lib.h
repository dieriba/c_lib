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
#include "d_dyn_array.h"
#include "d_dyn_string.h"
#include "d_string_view.h"

/**
 * @brief Predicate over a single @c char, used by string-scanning APIs.
 *
 * @return @c true if the character matches the predicate's condition.
 */
typedef bool (*DCompareFnc)(char);

/**
 * @brief Hash function for a map or set key.
 *
 * @param key Pointer to the key bytes. Never NULL when called by the library.
 * @return An unsigned hash value.
 */
typedef usize (*FnPtrGenHash)(void *key);

/**
 * @brief Equality predicate for two map or set keys.
 *
 * @return @c true if the two keys are equal, @c false otherwise.
 */
typedef bool (*FnPtrCmpKey)(void *, void *);

/**
 * @brief Destructor called when a container element is removed or overwritten.
 *
 * The argument is a **pointer to the stored slot**, not the element value
 * itself. For a container holding @c char *, the slot contains a @c char *,
 * so the callback receives a @c char ** and must dereference it before freeing:
 *
 * @code{.c}
 *   void my_free_str(void *elem) { free(*(char **)elem); }
 * @endcode
 */
typedef void (*FnPtrFreeElem)(void *elem);

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
 * @brief Splits a string view on any character found in @p set.
 *
 * Wraps @c d_string_view_split_by_char_of_str. Initialises @p new_dyn_array
 * with element type ::DStringView; each resulting view points into @p view's
 * underlying buffer and becomes invalid if that buffer is freed or mutated.
 *
 * @param new_dyn_array Receives the array of ::DStringView tokens. Must not be NULL.
 * @param view          The string to split.
 * @param set           View over the set of delimiter characters.
 *                      Use @ref D_STRING_VIEW_FROM_LITERAL for string-literal sets.
 * @param opts          Buffer options for the backing array (pass @c RAW_BUF_OPT_NONE
 *                      for default behaviour).
 * @return ::D_OK, ::D_ERR_INVALID_ARG if @p new_dyn_array is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DDynArray parts;
 *   d_split_string_by_char_of_str(&parts, d_string_view_from_c_string("a,b;c"),
 *                                  D_STRING_VIEW_FROM_LITERAL(",;"), RAW_BUF_OPT_NONE);
 *   // parts contains DStringView tokens: "a", "b", "c"
 *   d_dyn_array_destroy(&parts);
 * @endcode
 */
DResult d_split_string_by_char_of_str(DDynArray *new_dyn_array, DStringView view, DStringView set, BufferOpts opts);

/**
 * @brief Splits a string view on a single delimiter character.
 *
 * Wraps @c d_string_view_split_by_char. Initialises @p new_dyn_array with
 * element type ::DStringView; each resulting view points into @p view's
 * underlying buffer and becomes invalid if that buffer is freed or mutated.
 *
 * @param new_dyn_array Receives the array of ::DStringView tokens. Must not be NULL.
 * @param view          The string to split.
 * @param c             Delimiter character.
 * @param opts          Buffer options for the backing array (pass @c RAW_BUF_OPT_NONE
 *                      for default behaviour).
 * @return ::D_OK, ::D_ERR_INVALID_ARG if @p new_dyn_array is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DDynArray parts;
 *   d_split_string_by_char(&parts, d_string_view_from_c_string("one:two:three"),
 *                           ':', RAW_BUF_OPT_NONE);
 *   // parts contains DStringView tokens: "one", "two", "three"
 *   d_dyn_array_destroy(&parts);
 * @endcode
 */
DResult d_split_string_by_char(DDynArray *new_dyn_array, DStringView view, char c, BufferOpts opts);

/**
 * @brief Extracts a heap-allocated substring from @p str.
 *
 * Copies up to @p size characters starting at byte offset @p pos. If
 * @c pos + @p size exceeds the string length, the copy is clamped to the end.
 * The result is always null-terminated. The caller is responsible for freeing
 * the returned pointer.
 *
 * @param str  Null-terminated source string. If NULL, returns NULL.
 * @param pos  Zero-based start offset. If @p pos is strictly greater than
 *             @c strlen(str), returns NULL.
 * @param size Number of characters to copy. If @p pos equals @c strlen(str)
 *             or the effective copy length is 0, returns an empty allocated
 *             string @c "".
 * @return Newly allocated substring, @c "" (empty string) if the effective
 *         length is 0 and @p pos is in-range, or NULL if @p str is NULL,
 *         @p pos exceeds the string length, or allocation fails.
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
 * @param str Null-terminated string to duplicate. If NULL, returns NULL.
 * @return Newly allocated copy of @p str, or NULL if @p str is NULL or
 *         allocation fails.
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

#endif
