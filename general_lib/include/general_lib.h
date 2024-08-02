#ifndef __GENERAL_LIB__H
#define __GENERAL_LIB__H

#include <darray.h>
#include <stdlib.h>
#include <dtypes.h>
#include <dstring.h>
#include <string.h>

typedef bool(*DCompareFnc)(char);

//DPointerArray* d_split_string_by_predicate_fn(const char *string, DCompareFnc predicate);
//DPointerArray* d_split_string_by_str(const char* string, const char*);
DPointerArray* d_split_string_by_char(const char* string, char c);
DPointerArray* d_split_string_by_char_of_str(const char* string, const char* delims);
//DPointerArray* d_split_string_by_pattern(const char*string, const char* pattern);

/**
 * @brief Extracts a substring from a given string.
 *
 * Allocates a new string and extracts a substring from the specified position `pos` of the input string `str`, extending for `len` characters.
 * If `str` is `NULL`, `NULL` will be returned. If `pos` is equal to or exceeds the length of `str`, or if `len` is `0`, the function will return NULL.
 * If `pos + len` exceeds the end of the string, the substring will include all characters from `pos` to the end of the string.
 * The resulting substring is null-terminated. The caller is responsible for freeing the allocated memory.
 *
 * @param str A pointer to the null-terminated input string from which to extract the substring. If `str` is `NULL`, an empty string is returned.
 * @param pos The starting index in the input string from which to begin the substring extraction. If `pos` is equal to or exceeds the length of `str`, or if `len` is `0`, the function will return an empty string.
 * @param len The number of characters to include in the substring. If `len` is `0`, the function will return an empty string. If `pos + len` exceeds the end of the string, the substring will include all characters from `pos` to the end of the string.
 *
 * @return char* A pointer to the newly allocated string containing the substring. Returns `NULL` if `pos` is equal to or exceeds the length of `str`, `len` is `0`, `str` is NULL, or if memory allocation fails.
 */
char    *d_substr(const char* str, usize pos, usize len);

/**
 * @brief Creates a duplicate of a standard null-terminated C string.
 *
 * Allocates memory and creates a duplicate of the provided null-terminated C string. The duplicated string is
 * a separate memory allocation and must be freed by the caller. If `str` is `NULL`, the function returns an empty string.
 *
 * @param str A pointer to the null-terminated C string to be duplicated. If `str` is `NULL`, the behavior is undefined.
 *
 * @return char* A pointer to the newly allocated null-terminated C string that contains the same content as the input string.
 *               Returns `NULL` if memory allocation fails.
 */
char    *d_strdup(const char* str);

/**
 * @brief Converts a 32-bit integer to a string.
 *
 * Allocates a new string and converts the given 32-bit signed integer to its string representation. The resulting string is null-terminated.
 * The caller is responsible for freeing the allocated memory.
 *
 * @param value The 32-bit signed integer to convert.
 *
 * @return char* A pointer to the newly allocated string containing the string representation of the integer. Returns `NULL` if memory allocation fails.
 */
char    *d_itoa_i32(int32 nb);

/**
 * @brief Converts a 32-bit integer to a string using a user-provided buffer.
 *
 * Converts the given 32-bit signed integer to its string representation. The caller must provide a buffer large enough to hold the result, 
 * including the null terminator. The function writes the result to this buffer.
 *
 * @param value The 32-bit signed integer to convert.
 * @param buffer A pointer to a buffer where the string representation of the integer will be written. The buffer must be large enough to
 *               hold the result including the null terminator.
 *
 * @return char* A pointer to the provided buffer containing the string representation of the integer.
 * 
 * @note Ensure the buffer is sufficiently large to hold the result. For a 32-bit integer, the buffer should be at least 12 bytes.
 */
char* d_itoa_i32_no_alloc(int32 nb, const char* buffer);

/**
 * @brief Converts a `usize` value to a string.
 *
 * Allocates a new string and converts the given `usize` value to its string representation. The resulting string is null-terminated.
 * The caller is responsible for freeing the allocated memory.
 *
 * @param value The `usize` value to convert.
 *
 * @return char* A pointer to the newly allocated string containing the string representation of the `usize` value. Returns `NULL` if memory allocation fails.
 */
char    *d_itoa_usize(usize nb);

/**
 * @brief Converts a `size_t` value to a string using a user-provided buffer.
 *
 * Converts the given `size_t` value to its string representation. The caller must provide a buffer large enough to hold the result,
 * including the null terminator. The function writes the result to this buffer.
 *
 * @param value The `size_t` value to convert.
 * @param buffer A pointer to a buffer where the string representation of the `size_t` value will be written. The buffer must be large enough to
 *               hold the result including the null terminator. A minimum size of 21 bytes is recommended for 64-bit platforms, and 12 bytes for 32-bit platforms.
 *
 * @return char* A pointer to the provided buffer containing the string representation of the `size_t` value.
 * 
 * @note For 64-bit platforms, ensure the buffer is at least 21 bytes to accommodate the maximum number of digits in a `size_t` value plus the null terminator.
 *       For 32-bit platforms, a minimum of 12 bytes is sufficient.
 */
char    *d_itoa_usize_no_alloc(usize nb, const char* buffer);
#endif