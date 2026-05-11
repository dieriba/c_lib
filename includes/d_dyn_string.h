#ifndef D_DYN_STRING_H
#define D_DYN_STRING_H

/**
 * @defgroup d_dyn_string Dynamic String
 * @{
 * @brief Heap-allocated, automatically-growing mutable string.
 *
 * ::DDynString is a null-terminated owning string that manages its own
 * memory. It grows automatically on append operations and always keeps a
 * zero terminator past the last character so that @ref d_dyn_string_get_string
 * always returns a valid C string.
 *
 * @par Typical lifecycle
 * @code{.c}
 *   DDynString s;
 *   d_dyn_string_init_from_c_string(&s, "hello");
 *   d_dyn_string_push_c_str(&s, ", world");
 *   printf("%s\n", d_dyn_string_get_string(&s)); // "hello, world"
 *   d_dyn_string_destroy(&s);
 * @endcode
 */

#include "raw_buffer.h"
#include "d_types.h"
#include "d_result.h"
/**
 * @brief Heap-allocated mutable string.
 *
 * Do not access internal fields directly; use the @ref d_dyn_string API.
 */
typedef struct DDynString
{
    RawBuffer str; /**< Internal raw byte buffer (implementation detail). */
} DDynString;

/* -----------------------------------------------------------------------
 * Initialisation
 * -------------------------------------------------------------------- */

/**
 * @brief Initialises an empty ::DDynString with a default internal capacity.
 *
 * @param dyn_string String to initialise. Must not be NULL.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if @p dyn_string is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DDynString s;
 *   d_dyn_string_init(&s);
 *   d_dyn_string_push_c_str(&s, "hello");
 *   d_dyn_string_destroy(&s);
 * @endcode
 */
DResult d_dyn_string_init(DDynString *dyn_string);

/**
 * @brief Initialises a ::DDynString from a null-terminated C string.
 *
 * The content of @p str is copied; the caller retains ownership of @p str.
 *
 * @param dyn_string String to initialise. Must not be NULL.
 * @param  str        Null-terminated source string. Must not be NULL.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if either argument is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DDynString s;
 *   d_dyn_string_init_from_c_string(&s, "hello, world");
 *   d_dyn_string_destroy(&s);
 * @endcode
 */
DResult d_dyn_string_init_from_c_string(DDynString *dyn_string, const char *str);

/**
 * @brief Initialises a ::DDynString as a deep copy of another ::DDynString.
 *
 * @param new_dyn_string Destination string to initialise. Must not be NULL.
 * @param  dstring        Source string to copy from. Must not be NULL.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if either argument is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 */
DResult d_dyn_string_init_from_dstring(DDynString *new_dyn_string, DDynString *dstring);

/**
 * @brief Initialises an empty ::DDynString with a pre-allocated capacity.
 *
 * Use this to avoid repeated reallocations when the final string length is
 * known ahead of time.
 *
 * @param dyn_string String to initialise. Must not be NULL.
 * @param  reserve    Number of characters to pre-allocate (excluding the null terminator).
 * @return ::D_OK, ::D_ERR_INVALID_ARG if @p dyn_string is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DDynString s;
 *   d_dyn_string_init_with_capacity(&s, 256);
 *   d_dyn_string_push_c_str(&s, "text that will grow");
 *   d_dyn_string_destroy(&s);
 * @endcode
 */
DResult d_dyn_string_init_with_capacity(DDynString *dyn_string, usize reserve);

/**
 * @brief Initialises a ::DDynString from a substring of a C string.
 *
 * Copies @p size characters starting at byte offset @p pos from @p str.
 * If `pos + size` exceeds the string length only the remaining characters
 * are copied.
 *
 * @param dyn_string String to initialise. Must not be NULL.
 * @param  str        Null-terminated source string. Must not be NULL.
 * @param  pos        Starting byte offset within @p str.
 * @param  size       Maximum number of characters to copy.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if either pointer is NULL or
 *         @p pos is past the end of @p str, ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DDynString s;
 *   d_dyn_string_init_with_sub_string(&s, "hello, world", 7, 5); // "world"
 *   d_dyn_string_destroy(&s);
 * @endcode
 */
DResult d_dyn_string_init_with_sub_string(DDynString *dyn_string, const char *str, usize pos, usize size);

/* -----------------------------------------------------------------------
 * Accessors
 * -------------------------------------------------------------------- */

/**
 * @brief Returns a read-only pointer to the internal null-terminated C string.
 *
 * @warning The pointer is invalidated by any mutation (push, resize, replace …).
 *          Do not store it across mutating calls.
 *
 * @param dstring The string to read. Must not be NULL.
 * @return Pointer to the null-terminated character data.
 *
 * @code{.c}
 *   printf("%s\n", d_dyn_string_get_string(&s));
 * @endcode
 */
const char *d_dyn_string_get_string(const DDynString *dstring);

/**
 * @brief Copies the character at index @p i into @p out_elem.
 *
 * @param  dstring  The string to read. Must not be NULL.
 * @param  i        Zero-based character index.
 * @param out_elem Pointer to a @c char that receives the value. Must not be NULL.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if any argument is NULL or @p i is out of range.
 */
DResult d_dyn_string_get_char_at(const DDynString *dstring, usize i, char *out_elem);

/**
 * @brief Tests two ::DDynString values for equality.
 *
 * @param d1 First string. Must not be NULL.
 * @param d2 Second string. Must not be NULL.
 * @return ::D_COMPARE_EQUAL if both strings have the same length and content,
 *         ::D_COMPARE_NOT_EQUAL otherwise, or ::D_COMPARE_ERROR if either
 *         argument is NULL.
 */
DCompareResult d_dyn_string_compare(DDynString *d1, DDynString *d2);

/**
 * @brief Writes the number of characters (excluding the null terminator) into @p size.
 *
 * @param  dstring The string to query. Must not be NULL.
 * @param size    Receives the character count. Must not be NULL.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if either argument is NULL.
 */
DResult d_dyn_string_get_size(const DDynString *dstring, usize *size);

/**
 * @brief Writes the allocated capacity (in characters) into @p capacity.
 *
 * The capacity is always at least `size + 1` to hold the null terminator.
 *
 * @param  dstring  The string to query. Must not be NULL.
 * @param capacity Receives the capacity. Must not be NULL.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if either argument is NULL.
 */
DResult d_dyn_string_get_capacity(const DDynString *dstring, usize *capacity);

/* -----------------------------------------------------------------------
 * Mutation
 * -------------------------------------------------------------------- */

/**
 * @brief Resizes the string to exactly @p size characters.
 *
 * Characters beyond the old size are filled with @p c. If the new size is
 * smaller the string is truncated. The null terminator is always maintained.
 *
 * @param dstring The string to resize. Must not be NULL.
 * @param     size    New character count (excluding the null terminator).
 * @param     c       Fill character used when the string grows.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if @p dstring is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DDynString s;
 *   d_dyn_string_init_from_c_string(&s, "hi");
 *   d_dyn_string_resize(&s, 5, '-'); // "hi---"
 *   d_dyn_string_resize(&s, 1, 0);  // "h"
 *   d_dyn_string_destroy(&s);
 * @endcode
 */
DResult d_dyn_string_resize(DDynString *dstring, usize size, char c);

/**
 * @brief Appends a single character to the end of the string.
 *
 * @param dstring The string to append to. Must not be NULL.
 * @param     c       The character to append.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if @p dstring is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 */
DResult d_dyn_string_push_char(DDynString *dstring, char c);

/**
 * @brief Appends exactly @p size bytes from @p str_to_append.
 *
 * Unlike @ref d_dyn_string_push_c_str this variant uses an explicit length,
 * so it works with buffers that may contain embedded null bytes.
 *
 * @param dstring       The string to append to. Must not be NULL.
 * @param     str_to_append Pointer to the bytes to append. Must not be NULL.
 * @param     size          Number of bytes to copy.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if either pointer is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 */
DResult d_dyn_string_push_str_with_len(DDynString *dstring, const char *str_to_append, usize size);

/**
 * @brief Appends a null-terminated C string to the dynamic string.
 *
 * @param dstring       The string to append to. Must not be NULL.
 * @param     str_to_append Null-terminated string to append. Must not be NULL.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if either argument is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DDynString s;
 *   d_dyn_string_init_from_c_string(&s, "hello");
 *   d_dyn_string_push_c_str(&s, ", world"); // "hello, world"
 *   d_dyn_string_destroy(&s);
 * @endcode
 */
DResult d_dyn_string_push_c_str(DDynString *dstring, const char *str_to_append);

/**
 * @brief Appends all characters of @p dstring2 to @p dstring1.
 *
 * @p dstring2 is not modified.
 *
 * @param dstring1 Destination string. Must not be NULL.
 * @param     dstring2 Source string to append. Must not be NULL.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if either argument is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 */
DResult d_dyn_string_merge(DDynString *dstring1, DDynString *dstring2);

/**
 * @brief Replaces the entire content of @p dstring with a copy of @p str.
 *
 * The previous content is discarded.
 *
 * @param dstring The string to replace. Must not be NULL.
 * @param     str     Null-terminated replacement string. Must not be NULL.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if either argument is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 */
DResult d_dyn_string_replace_from_str(DDynString *dstring, const char *str);

/**
 * @brief Replaces the content of @p dstring with a copy of @p to_copy.
 *
 * @param dstring  The string to replace. Must not be NULL.
 * @param     to_copy  Source whose content is copied in. Must not be NULL.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if either argument is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 */
DResult d_dyn_string_replace_from_dstring(DDynString *dstring, const DDynString *to_copy);

/**
 * @brief Replaces the content of @p dstring in-place with the substring
 *        starting at @p pos of length @p size.
 *
 * No allocation occurs; the operation is performed inside the existing buffer.
 * If `pos + size` exceeds the current length all characters from @p pos to the
 * end are kept.
 *
 * @param dstring The string to modify. Must not be NULL.
 * @param     pos     Starting byte offset of the desired substring.
 * @param     size    Maximum character count of the desired substring.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if @p dstring is NULL or @p pos is
 *         at or past the current length.
 *
 * @code{.c}
 *   DDynString s;
 *   d_dyn_string_init_from_c_string(&s, "hello, world");
 *   d_dyn_string_sub_string_in_place(&s, 7, 5); // s == "world"
 *   d_dyn_string_destroy(&s);
 * @endcode
 */
DResult d_dyn_string_sub_string_in_place(DDynString *dstring, usize pos, usize size);

/* -----------------------------------------------------------------------
 * Destruction
 * -------------------------------------------------------------------- */

/**
 * @brief Releases all memory held by @p dstring and zeroes the struct.
 *
 * Safe to call on a NULL pointer (no-op). After this call @p dstring must
 * not be used without re-initialisation.
 *
 * @param dstring The string to destroy. May be NULL.
 */
void d_dyn_string_destroy(DDynString *dstring);

void d_dyn_string_dbg_print(DDynString *dstring);

/** @} */ /* end of d_dyn_string group */


#endif
