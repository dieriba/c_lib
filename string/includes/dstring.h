#ifndef __D_STRING__H__
#define __D_STRING__H__

#include <dtypes.h>
#include <darray.h>
typedef struct _DString DString;


/**
 * @brief Represents a dynamic string structure.
 *
 * The _DString structure holds a dynamically allocated string. The string is
 * null-terminated, which means a null byte ('\0') is added at the end to
 * signify the end of the string. This ensures compatibility with C standard
 * library functions that operate on null-terminated strings.
 *
 * @struct _DString
 * @param string Pointer to the character array containing the string data.
 * @param len Length of the string, excluding the null terminator.
 */
struct _DString {
	char    *string;
	usize     	len;
};

typedef usize(*match)(char c);
typedef bool(*match_predicate)(char c);



/**
 * @brief Creates a new, empty dynamic string.
 *
 * Allocates and initializes a new _DString structure. The newly created
 * dynamic string is empty, with its length set to zero. The internal character
 * array is allocated but initially empty, and a null byte ('\0') is placed
 * at the beginning to ensure it is a valid null-terminated string.
 *
 * @return DString* A pointer to the newly created _DString structure. Returns
 *         NULL if memory allocation fails.
 */
DString* d_string_new(void);

/**
 * @brief Creates a new dynamic string from a given C string.
 *
 * Allocates and initializes a new _DString structure based on the provided
 * C string. The new dynamic string contains a copy of the input string,
 * including its null terminator. The length of the dynamic string will
 * reflect the length of the input string, excluding the null terminator.
 *
 * @param str A pointer to the null-terminated C string to copy. If NULL,
 *            the function will create an empty dynamic string.
 *
 * @return DString* A pointer to the newly created _DString structure containing
 *         the copied string. Returns NULL if memory allocation fails.
 */
DString* 	d_string_new_from_c_string(const char* str);

/**
 * @brief Creates a new dynamic string from a given  #DString.
 *
 * Allocates and initializes a new DString structure based on the provided
 * #DString. The new dynamic string contains a copy of the input #DString's string,
 * including its null terminator. The length of the dynamic string will
 * reflect the length of the copied string, excluding the null terminator.
 * 
 * @param str A pointer to the #DString to copy. If NULL,
 *            the function will create an empty dynamic string.
 *
 * @return DString* A pointer to the newly created _DString structure containing
 *         the copied #DString's string. Returns NULL if memory allocation fails.
 */
DString* 	d_string_new_from_dstring(DString* dstring);

/**
 * @brief Creates a new dynamic string with an initial reserved capacity.
 *
 * Allocates and initializes a new _DString structure with the specified initial
 * capacity. The reserved capacity determines the amount of memory allocated for
 * the internal character array, allowing for more efficient string manipulation
 * if you expect to add a significant amount of data. The initial string length
 * will be zero, and the internal character array will be null-terminated.
 *
 * @param reserve The initial capacity for the new _DString, specifying the
 *                number of characters to reserve space for. The reserved space
 *                is pre-allocated to optimize performance for future string
 *                modifications.
 *
 * @return DString* A pointer to the newly created _DString structure with the
 *         specified initial capacity. Returns NULL if memory allocation fails.
 */
DString* 	d_string_new_with_reserve(usize reserve);

/**
 * @brief Creates a new dynamic string from a substring of a given C string.
 *
 * Allocates and initializes a new _DString structure based on a substring of
 * the provided C string. The substring is extracted starting from the specified
 * position `from` and extending for `len` characters. The new dynamic string will
 * include a null terminator. If `from` is out of range or `len` exceeds the
 * remaining length from `from`, the substring length will be adjusted accordingly.
 *
 * @param str A pointer to the null-terminated C string from which to extract the
 *            substring. Must not be NULL.
 * @param from The starting index in the C string from which to begin the substring.
 * 			   If `from` is greater than the string length, the function will return NULL.
 * @param len The number of characters to include in the substring. If len extends
 *            beyond the end of the C string, the substring will include all characters
 *            up to the end of the string.
 *
 * @return DString* A pointer to the newly created _DString structure containing
 *         the substring. Returns NULL if memory allocation fails or if the input
 *         parameters are invalid.
 */
DString* 	d_string_new_with_substring(const char* str, usize from, usize len);

/**
 * @brief Retrieves the current capacity of a dynamic string.
 *
 * Returns the amount of memory allocated for the internal character array of
 * the specified _DString. The capacity indicates how many characters the
 * dynamic string can hold before needing to allocate additional memory.
 *
 * @param dstring A pointer to the _DString structure whose capacity is to be
 *                retrieved. Must not be NULL.
 *
 * @return usize The current capacity of the _DString, representing the number
 *         of characters that can be stored in the internal character array.
 */
usize		d_string_get_capacity(DString* dstring);

/**
 * @brief Resizes a dynamic string to a specified length.
 *
 * Adjusts the size of the given _DString to the specified length. If the new
 * length is greater than the current length, additional memory will be allocated,
 * and the string will be extended with null bytes ('\0') if necessary. If the new
 * length is shorter than the current length, the string will be truncated. The
 * internal character array will always be null-terminated.
 *
 * @param dstring A pointer to the _DString structure to be resized. Must not be NULL.
 * @param len The new length for the _DString. If len is zero, the string will be
 *            resized to an empty string.
 *
 * @return DString* A pointer to the resized _DString structure. Returns NULL if
 *         memory allocation fails or if the input parameters are invalid.
 */
DString* 	d_string_resize(DString* dstring, usize len);

/**
 * @brief Increases the capacity of a dynamic string to a specified size.
 *
 * Expands the capacity of the given _DString to the specified new capacity. This
 * function ensures that the internal character array has enough space to accommodate
 * additional characters, which can improve performance for subsequent string
 * modifications. The current string content is preserved, and the string remains
 * null-terminated. If the new capacity is less than the current capacity, no change
 * will be made.
 *
 * @param dstring A pointer to the _DString structure whose capacity is to be increased.
 *                Must not be NULL.
 * @param new_capacity The new capacity to set for the _DString. Must be greater than
 *                     the current capacity.
 *
 * @return DString* A pointer to the _DString structure with the updated capacity.
 *         Returns NULL if memory allocation fails or if the new capacity is invalid.
 */
DString* 	d_string_modify_capacity(DString* dstring, usize new_capacity);

/**
 * @brief Appends a single character to the end of a dynamic string.
 *
 * Adds the specified character to the end of the given _DString. The function
 * ensures that there is enough capacity in the internal character array to
 * accommodate the new character. If necessary, the string's capacity will be
 * increased to fit the new character. The internal character array remains
 * null-terminated after the character is appended.
 *
 * @param dstring A pointer to the _DString structure to which the character is to be
 *                appended. Must not be NULL.
 * @param c The character to append to the end of the _DString.
 *
 * @return DString* A pointer to the updated _DString structure with the new character
 *         appended. Returns NULL if memory allocation fails.
 */
DString* 	d_string_push_char(DString* dstring, char c);

/**
 * @brief Appends a string to the end of a dynamic string.
 *
 * Concatenates the specified string to the end of the given _DString. The function
 * ensures that there is enough capacity in the internal character array to accommodate
 * the additional characters. If necessary, the string's capacity will be increased
 * to fit the new content. The internal character array remains null-terminated after
 * appending the specified number of characters from the input string.
 *
 * @param dstring A pointer to the _DString structure to which the string is to be
 *                appended. Must not be NULL.
 * @param str_to_append A pointer to a string to append to the _DString. If str_to_append is NULL, no action is taken.
 *                      The string may not be null-terminated.
 * @param len The number of characters to copy from `str_to_append` into the _DString.
 *
 * @return DString* A pointer to the updated _DString structure with the specified 
 *         number of characters appended. Returns NULL if memory allocation fails.
 */
DString* 	d_string_push_str_with_len(DString* dstring, const char *str_to_append, usize len);


/**
 * @brief Appends a C string to the end of a dynamic string.
 *
 * Concatenates the specified C string to the end of the given _DString. The function
 * ensures that there is enough capacity in the internal character array to accommodate
 * the additional characters. If necessary, the string's capacity will be increased
 * to fit the new content. The internal character array remains null-terminated after
 * appending the C string.
 *
 * @param dstring A pointer to the _DString structure to which the C string is to be
 *                appended. Must not be NULL.
 * @param str_to_append A pointer to the null-terminated C string to append to the
 *                      _DString. If str_to_append is NULL, no action is taken.
 *
 * @return DString* A pointer to the updated _DString structure with the C string
 *         appended. Returns NULL if memory allocation fails.
 */
DString* 	d_string_push_c_str(DString* dstring, const char *str_to_append);

/**
 * @brief Replaces the content of a dynamic string with a new C string.
 *
 * Replaces the current content of the given _DString with the specified C string.
 * If the new content requires more space than the current capacity of the _DString,
 * reallocation may occur to accommodate the new string. The function updates the
 * internal character array to match the new content, ensuring that the string is
 * null-terminated. If the provided C string is NULL, the _DString will be emptied.
 *
 * @param dstring A pointer to the _DString structure whose content is to be replaced.
 *                Must not be NULL.
 * @param str A pointer to the null-terminated C string to replace the existing content.
 *            If str is NULL, the _DString will be cleared.
 * 
 * @return DString* A pointer to the updated _DString structure with the new content.
 *         Returns NULL if memory allocation fails or if the input parameters are invalid.
 */
DString* 	d_string_replace_from_str(DString** dstring, const char* str);

/**
 * @brief Replaces the content of a dynamic string with another dynamic string.
 *
 * Replaces the current content of the given _DString with the content of another
 * _DString. If the new content requires more space than the current capacity, 
 * reallocation may occur to accommodate the new content. The function updates
 * the internal character array of the original _DString to match the content of
 * the provided _DString, including adjusting the length and ensuring the string
 * is null-terminated. If the provided _DString is NULL, the original _DString
 * will be emptied.
 *
 * @param dstring A pointer to the _DString structure to be replaced. Must not be NULL.
 * @param copy A pointer to the _DString whose content will replace the original.
 *             If copy is NULL, the original _DString will be cleared.
 *
 * @return DString* A pointer to the updated _DString structure with the new content.
 *         Returns NULL if memory allocation fails or if the input parameters are invalid.
 */
DString* 	d_string_replace_from_dstring(DString** dstring, const DString* copy);



/**
 * @brief Compares two dynamic strings for equality.
 *
 * Compares the content of the two given _DString structures lexicographically.
 * The comparison is case-sensitive and takes into account the null terminator.
 * Returns an integer less than, equal to, or greater than zero, depending on whether
 * the first string is lexicographically less than, equal to, or greater than the second string.
 *
 * @param str1 A pointer to the first _DString structure to be compared. Must not be NULL.
 * @param str2 A pointer to the second _DString structure to be compared. Must not be NULL.
 *
 * @return int32 An integer less than, equal to, or greater than zero if the first
 *         _DString is lexicographically less than, equal to, or greater than the second,
 *         respectively. Returns a negative value if either str1 or str2 is NULL.
 */
int32		d_string_compare(DString* str1, DString* str2);

/**
 * @brief Finds the first occurrence of a character in a dynamic string starting from a specified position.
 *
 * Searches for the first occurrence of the specified character in the given _DString,
 * starting from the index specified by `pos`. The function returns the index of the character
 * within the string. If the character is not found starting from `pos`, it returns `MAX_SIZE_T`,
 * which represents the maximum value of `size_t`, indicating that the character is absent.
 *
 * @param dstring A pointer to the _DString structure in which to search for the character.
 *                Must not be NULL.
 * @param c The character to search for in the _DString.
 * @param pos The index in the _DString from which to start the search 				
 *
 * @return usize The index of the first occurrence of the character in the _DString starting from `pos`.
 *         Returns `MAX_SIZE_T` if the character is not found or if `pos` is out of range or invalid.
 */
usize		d_string_find_char_from(DString* dstring, char c, usize pos);

/**
 * @brief Finds the first occurrence of a character in a dynamic string.
 *
 * Searches for the first occurrence of the specified character in the given _DString.
 * The function returns the index of the character within the string. If the character
 * is not found, it returns `MAX_SIZE_T`, which represents the maximum value of `size_t`,
 * indicating that the character is absent. The search is performed starting from the
 * beginning of the string.
 *
 * @param dstring A pointer to the _DString structure in which to search for the character.
 *                Must not be NULL.
 * @param c The character to search for in the _DString.
 *
 * @return usize The index of the first occurrence of the character in the _DString.
 *         Returns `MAX_SIZE_T` if the character is not found or if the input parameter is invalid.
 */
usize		d_string_find_char(DString* dstring, char c);

/**
 * @brief Finds the first occurrence of a substring within a dynamic string starting from a specified position.
 *
 * Searches for the first occurrence of the specified substring within the given _DString,
 * starting from the index specified by `pos`. If `pos` is greater than the length of the
 * _DString, the function returns `MAX_SIZE_T`, which represents the maximum value of `size_t`,
 * indicating that the substring cannot be found starting from such an out-of-range position.
 * If the substring is found, the function returns the starting index of its first occurrence.
 * The search is performed starting from the specified position in the _DString.
 *
 * @param dstring A pointer to the _DString structure in which to search for the substring.
 *                Must not be NULL.
 * @param str A pointer to the null-terminated C string (substring) to search for in the
 *            _DString. If str is NULL, the function may return `MAX_SIZE_T` as an invalid
 *            input indication.
 * @param pos The index in the _DString from which to start the search. If `pos` is greater
 *            than the length of the _DString, the function will return `MAX_SIZE_T`.
 *
 * @return usize The index of the first occurrence of the substring within the _DString
 *         starting from `pos`. Returns `MAX_SIZE_T` if the substring is not found or if
 *         `pos` is greater than the length of the _DString or if the input parameters are invalid.
 */
usize		d_string_find_str_from(DString* dstring, const char *str, usize pos);

/**
 * @brief Finds the first occurrence of a substring within a dynamic string.
 *
 * Searches for the first occurrence of the specified substring within the given _DString.
 * The function returns the starting index of the first occurrence of the substring. If the
 * substring is not found, it returns `MAX_SIZE_T`, which represents the maximum value of
 * `size_t`, indicating that the substring is absent. The search is performed starting from
 * the beginning of the _DString.
 *
 * @param dstring A pointer to the _DString structure in which to search for the substring.
 *                Must not be NULL.
 * @param str A pointer to the null-terminated C string (substring) to search for in the
 *            _DString. If str is NULL, the function may return `MAX_SIZE_T` as an invalid
 *            input indication.
 *
 * @return usize The index of the first occurrence of the substring within the _DString.
 *         Returns `MAX_SIZE_T` if the substring is not found or if the input parameters are invalid.
 */
usize		d_string_find_str(DString* dstring, const char *str);

/**
 * @brief Finds the index of the first character in a dynamic string that satisfies a predicate function, starting from a specified position.
 *
 * Searches for the first occurrence of a character in the given _DString that meets the condition
 * defined by the provided predicate function, starting from the index specified by `pos`. The predicate
 * function should return `true` if the character satisfies the condition and `false` otherwise. 
 * If `pos` is greater than the length of the _DString, the function returns `MAX_SIZE_T`, which represents
 * the maximum value of `size_t`, indicating that the search is out of range. If no character satisfying the
 * predicate is found starting from `pos`, it also returns `MAX_SIZE_T`.
 *
 * @param dstring A pointer to the _DString structure in which to search for the character.
 *                Must not be NULL.
 * @param fn A function pointer to the predicate function that determines whether a character
 *           satisfies the condition. The predicate function should have the following signature:
 *           `bool (*match_predicate)(char)`. If fn is NULL, the function may return `MAX_SIZE_T` as an invalid
 *           input indication.
 * @param pos The index in the _DString from which to start the search. If `pos` is greater than
 *            the length of the _DString, the function will return `MAX_SIZE_T`.
 *
 * @return usize The index of the first character in the _DString starting from `pos` that satisfies
 *         the predicate function. Returns `MAX_SIZE_T` if no character satisfies the predicate,
 *         if `pos` is greater than the length of the _DString, or if the input parameters are invalid.
 */
usize		d_string_find_by_predicate_from(DString* dstring, match_predicate fn, usize pos);

/**
 * @brief Finds the index of the first character in a dynamic string that satisfies a predicate function.
 *
 * Searches for the first occurrence of a character in the given _DString that meets the condition
 * defined by the provided predicate function. The search is performed starting from the beginning
 * of the _DString. The predicate function should return `true` if the character satisfies the condition
 * and `false` otherwise. The function returns the index of the first character that meets the condition.
 * If no character satisfies the predicate, it returns `MAX_SIZE_T`, which represents the maximum value
 * of `size_t`, indicating that no such character was found.
 *
 * @param dstring A pointer to the _DString structure in which to search for the character.
 *                Must not be NULL.
 * @param fn A function pointer to the predicate function that determines whether a character
 *           satisfies the condition. The predicate function should have the following signature:
 *           `bool (*match_predicate)(char)`. If fn is NULL, the function may return `MAX_SIZE_T` as an invalid
 *           input indication.
 *
 * @return usize The index of the first character in the _DString that satisfies the predicate function.
 *         Returns `MAX_SIZE_T` if no character satisfies the predicate or if the input parameters
 *         are invalid.
 */
usize		d_string_find_by_predicate(DString* dstring, match_predicate fn);

/**
 * @brief Finds the last occurrence of a character in a dynamic string starting from a specified position.
 *
 * Searches for the last occurrence of the specified character in the given _DString,
 * starting from the index specified by `pos` and moving backward. The function returns
 * the index of the last occurrence of the character before or at the specified position. 
 * If the character is not found, or if `pos` is greater than the length of the _DString,
 * the function returns `MAX_SIZE_T`, which represents the maximum value of `size_t`,
 * indicating that the character is absent or the search is out of range.
 *
 * @param dstring A pointer to the _DString structure in which to search for the character.
 *                Must not be NULL.
 * @param c The character to search for in the _DString.
 * @param pos The index in the _DString from which to start the search, moving backward.
 *            If `pos` is greater than the length of the _DString, it is treated as if
 *            the search starts from the end of the string.
 *
 * @return usize The index of the last occurrence of the character within the _DString
 *         starting from `pos` and moving backward. Returns `MAX_SIZE_T` if the character
 *         is not found or if `pos` is greater than the length of the _DString or if the
 *         input parameters are invalid.
 */
usize		d_string_rfind_char_from(DString* dstring, char c, usize pos);


/**
 * @brief Finds the last occurrence of a character in a dynamic string.
 *
 * Searches for the last occurrence of the specified character in the given _DString. 
 * The function returns the index of the last occurrence of the character. If the character 
 * is not found, it returns `MAX_SIZE_T`, which represents the maximum value of `size_t`, 
 * indicating that the character is absent in the string.
 *
 * @param dstring A pointer to the _DString structure in which to search for the character.
 *                Must not be NULL.
 * @param c The character to search for in the _DString.
 *
 * @return usize The index of the last occurrence of the character within the _DString.
 *         Returns `MAX_SIZE_T` if the character is not found or if the input parameters are invalid.
 */
usize		d_string_rfind_char(DString* dstring, char c);

/**
 * @brief Finds the last occurrence of a substring in a dynamic string starting from a specified position.
 *
 * Searches for the last occurrence of the specified substring within the given _DString,
 * starting from the index specified by `pos` and moving backward. The function returns
 * the starting index of the last occurrence of the substring before or at the specified position.
 * If the substring is not found, or if `pos` is greater than the length of the _DString,
 * the function returns `MAX_SIZE_T`, which represents the maximum value of `size_t`,
 * indicating that the substring is absent or the search is out of range.
 *
 * @param dstring A pointer to the _DString structure in which to search for the substring.
 *                Must not be NULL.
 * @param str A pointer to the null-terminated C string (substring) to search for in the
 *            _DString. If str is NULL, the function may return `MAX_SIZE_T` as an invalid
 *            input indication.
 * @param pos The index in the _DString from which to start the search, moving backward.
 *            If `pos` is greater than the length of the _DString, it is treated as if
 *            the search starts from the end of the string.
 *
 * @return usize The starting index of the last occurrence of the substring within the _DString
 *         starting from `pos` and moving backward. Returns `MAX_SIZE_T` if the substring is
 *         not found or if `pos` is greater than the length of the _DString or if the input
 *         parameters are invalid.
 */
usize		d_string_rfind_str_from(DString* dstring, const char *str, usize pos);

/**
 * @brief Finds the last occurrence of a substring in a dynamic string.
 *
 * Searches for the last occurrence of the specified substring within the given _DString. 
 * The function returns the starting index of the last occurrence of the substring. If the 
 * substring is not found, it returns `MAX_SIZE_T`, which represents the maximum value of 
 * `size_t`, indicating that the substring is absent in the string.
 *
 * @param dstring A pointer to the _DString structure in which to search for the substring.
 *                Must not be NULL.
 * @param str A pointer to the null-terminated C string (substring) to search for in the
 *            _DString. If str is NULL, the function may return `MAX_SIZE_T` as an invalid
 *            input indication.
 *
 * @return usize The starting index of the last occurrence of the substring within the _DString.
 *         Returns `MAX_SIZE_T` if the substring is not found or if the input parameters are invalid.
 */
usize		d_string_rfind_str(DString* dstring, const char *str);


/**
 * @brief Finds the index of the last character in a dynamic string that satisfies a predicate function, starting from a specified position.
 *
 * Searches for the last occurrence of a character in the given _DString that meets the condition
 * defined by the provided predicate function, starting from the index specified by `pos` and moving backward.
 * The predicate function should return `true` if the character satisfies the condition and `false` otherwise.
 * The function returns the index of the last character that meets the condition before or at the specified position.
 * If `pos` is greater than the length of the _DString, it is treated as if the search starts from the end of the string.
 * If no character satisfying the predicate is found, the function returns `MAX_SIZE_T`, which represents
 * the maximum value of `size_t`, indicating that no such character was found.
 *
 * @param dstring A pointer to the _DString structure in which to search for the character.
 *                Must not be NULL.
 * @param fn A function pointer to the predicate function that determines whether a character
 *           satisfies the condition. The predicate function should have the following signature:
 *           `bool (*match_predicate)(char)`. If fn is NULL, the function may return `MAX_SIZE_T` as an invalid
 *           input indication.
 * @param pos The index in the _DString from which to start the search, moving backward. If `pos` is greater
 *            than the length of the _DString, the function will treat it as starting from the end of the string.
 *
 * @return usize The index of the last character in the _DString starting from `pos` that satisfies the predicate function.
 *         Returns `MAX_SIZE_T` if no character satisfies the predicate, if `pos` is greater than the length of the
 *         _DString, or if the input parameters are invalid.
 */
usize		d_string_rfind_by_predicate_from(DString* dstring, match_predicate fn, usize pos);

/**
 * @brief Finds the index of the last character in a dynamic string that satisfies a predicate function.
 *
 * Searches for the last occurrence of a character in the given _DString that meets the condition
 * defined by the provided predicate function. The search is performed starting from the end of the
 * _DString and moving backward. The predicate function should return `true` if the character satisfies
 * the condition and `false` otherwise. The function returns the index of the last character that meets
 * the condition. If no character satisfying the predicate is found, it returns `MAX_SIZE_T`, which
 * represents the maximum value of `size_t`, indicating that no such character was found.
 *
 * @param dstring A pointer to the _DString structure in which to search for the character.
 *                Must not be NULL.
 * @param fn A function pointer to the predicate function that determines whether a character
 *           satisfies the condition. The predicate function should have the following signature:
 *           `bool (*match_predicate)(char)`. If fn is NULL, the function may return `MAX_SIZE_T` as an invalid
 *           input indication.
 *
 * @return usize The index of the last character in the _DString that satisfies the predicate function.
 *         Returns `MAX_SIZE_T` if no character satisfies the predicate or if the input parameters are invalid.
 */
usize		d_string_rfind_by_predicate(DString* dstring, match_predicate fn);


/**
 * @brief Creates a new dynamic string that is a substring of the given dynamic string.
 *
 * Creates a new _DString that is a substring of the specified _DString, starting from the index
 * `pos` and spanning `len` characters. The new _DString will contain the characters from `pos` to
 * `pos + len - 1` of the original _DString. If `pos` or `pos + len` is out of range 
 * no operation will be done on the string.
 *
 * @param dstring A pointer to the original _DString from which to create the substring.
 *                Must not be NULL.
 * @param pos The starting index of the substring within the original _DString.
 * @param len The length of the substring.
 *
 * @return DString* A new _DString containing the substring starting from `pos` with the specified length.
 *         Returns NULL if `pos` or `len` is out of range or if the input parameters are invalid.
 */
DString*	d_string_sub_string_new(DString* dstring, usize pos, usize len);

/**
 * @brief Truncates the dynamic string to become a substring starting from a specified position.
 *
 * Modifies the given _DString to retain only the substring starting from the index `pos` and spanning
 * `len` characters. The original _DString is resized to contain only the characters from `pos` to
 * `pos + len - 1`. If `pos` or `pos + len` is out of range no operation will be done on the string.
 *
 * @param dstring A pointer to the _DString that will be modified to become the substring.
 *                Must not be NULL.
 * @param pos The starting index of the substring within the original _DString.
 * @param len The length of the substring.
 *
 * @return DString* A pointer to the modified _DString, which now contains only the substring.
 *         Returns NULL if `pos` or `len` is out of range, or if the input parameters are invalid.
 */
DString*	d_string_sub_string_in_place(DString* dstring, usize pos, usize len);

/**
 * @brief Removes leading occurrences of a specified character from the dynamic string in place.
 *
 * Modifies the given _DString by removing characters from the start of the string as long as they
 * match the specified character `c`. The function adjusts the internal representation of the _DString
 * to exclude these leading characters and updates its length accordingly. If no leading characters match `c`,
 * the _DString remains unchanged. The operation is performed directly on the original _DString.
 *
 * @param dstring A pointer to the _DString structure that will be modified. Must not be NULL.
 * @param c The character to be trimmed from the beginning of the _DString.
 *
 * @return DString* A pointer to the modified _DString, with leading occurrences of the specified character removed.
 *         Returns NULL if the input parameters are invalid.
 */
DString*	d_string_trim_left_by_char_in_place(DString* dstring, char c);

/**
 * @brief Removes leading characters from the dynamic string that satisfy a predicate function.
 *
 * Modifies the given _DString in place by removing characters from the start of the string as long as they
 * satisfy the condition defined by the provided predicate function `fn`. The function updates the length of
 * the _DString to exclude these leading characters. If no leading characters satisfy the predicate, the
 * _DString remains unchanged. The operation is performed directly on the original _DString.
 *
 * @param dstring A pointer to the _DString structure that will be modified. Must not be NULL.
 * @param fn A function pointer to the predicate function that determines whether a character should be removed.
 *           The predicate function should have the following signature: `bool (*match)(char)`. The function
 *           will be called for each leading character until a character does not satisfy the predicate.
 *           If `fn` is NULL, the function may return NULL or handle the invalid input according to the implementation.
 *
 * @return DString* A pointer to the modified _DString, with leading characters removed based on the predicate.
 *         Returns NULL if the input parameters are invalid.
 */
DString*	d_string_trim_left_by_predicate_in_place(DString* dstring, match fn);

/**
 * @brief Creates a new dynamic string with leading occurrences of a specified character removed.
 *
 * Creates a new _DString by removing characters from the start of the given _DString as long as they match
 * the specified character `c`. The new _DString will contain the characters from the original _DString, but
 * without the leading occurrences of `c`. If no leading characters match `c`, the new _DString will be
 * identical to the original one. This function does not modify the original _DString; instead, it returns
 * a new _DString reflecting the trimmed result.
 *
 * @param dstring A pointer to the original _DString from which to create the new trimmed string.
 *                Must not be NULL.
 * @param c The character to be removed from the beginning of the original _DString.
 *
 * @return DString* A pointer to the new _DString with leading occurrences of the specified character removed.
 *         Returns NULL if the input parameters are invalid.
 */
DString*	d_string_trim_left_by_char_new(DString* dstring, char c);

/**
 * @brief Creates a new dynamic string with leading characters removed based on a predicate function.
 *
 * Creates a new _DString by removing characters from the start of the given _DString as long as they satisfy
 * the condition defined by the provided predicate function `fn`. The new _DString will contain the characters
 * from the original _DString, but without the leading characters that match the predicate. If no leading characters
 * satisfy the predicate, the new _DString will be identical to the original one. This function does not modify the
 * original _DString; instead, it returns a new _DString reflecting the trimmed result.
 *
 * @param dstring A pointer to the original _DString from which to create the new trimmed string.
 *                Must not be NULL.
 * @param fn A function pointer to the predicate function that determines whether a character should be removed.
 *           The predicate function should have the following signature: `bool (*match)(char)`. The function
 *           will be called for each leading character until a character does not satisfy the predicate.
 *           If `fn` is NULL, the function may return NULL or handle the invalid input according to the implementation.
 *
 * @return DString* A pointer to the new _DString with leading characters removed based on the predicate function.
 *         Returns NULL if the input parameters are invalid.
 */
DString*	d_string_trim_left_by_predicate_new(DString* dstring, match fn);


/**
 * @brief Removes trailing occurrences of a specified character from the dynamic string in place.
 *
 * Modifies the given _DString by removing characters from the end of the string as long as they match
 * the specified character `c`. The function adjusts the length of the _DString to exclude these trailing
 * characters. If no trailing characters match `c`, the _DString remains unchanged. The operation is performed
 * directly on the original _DString.
 *
 * @param dstring A pointer to the _DString structure that will be modified. Must not be NULL.
 * @param c The character to be trimmed from the end of the _DString.
 *
 * @return DString* A pointer to the modified _DString, with trailing occurrences of the specified character removed.
 *         Returns NULL if the input parameters are invalid.
 */
DString*	d_string_trim_right_by_char_in_place(DString* dstring, char c);

/**
 * d_string_trim_right_predicate_in_place:
 * @param dstring The #DString to trim.
 * @param fn The predicate function to use for trimming.
 *
 * Trims characters from the right of the #DString based on the predicate function without reallocating memory.
 * The operation modifies the original #DString.
 * @return The trimmed #DString
 */
DString*	d_string_trim_right_by_prediacte_in_place(DString* dstring, match fn);

/**
 * @brief Creates a new dynamic string with trailing occurrences of a specified character removed.
 *
 * Creates a new _DString by removing characters from the end of the given _DString as long as they match
 * the specified character `c`. The new _DString will contain the characters from the original _DString, but
 * without the trailing occurrences of `c`. If no trailing characters match `c`, the new _DString will be
 * identical to the original one. This function does not modify the original _DString; instead, it returns
 * a new _DString reflecting the trimmed result.
 *
 * @param dstring A pointer to the original _DString from which to create the new trimmed string.
 *                Must not be NULL.
 * @param c The character to be removed from the end of the original _DString.
 *
 * @return DString* A pointer to the new _DString with trailing occurrences of the specified character removed.
 *         Returns NULL if the input parameters are invalid.
 */
DString*	d_string_trim_right_by_char_new(DString* dstring, char c);

/**
 * @brief Creates a new dynamic string with trailing characters removed based on a predicate function.
 *
 * Creates a new _DString by removing characters from the end of the given _DString as long as they satisfy
 * the condition defined by the provided predicate function `fn`. The new _DString will contain the characters
 * from the original _DString, but without the trailing characters that match the predicate. If no trailing characters
 * satisfy the predicate, the new _DString will be identical to the original one. This function does not modify the
 * original _DString; instead, it returns a new _DString reflecting the trimmed result.
 *
 * @param dstring A pointer to the original _DString from which to create the new trimmed string.
 *                Must not be NULL.
 * @param fn A function pointer to the predicate function that determines whether a character should be removed.
 *           The predicate function should have the following signature: `bool (*match)(char)`. The function
 *           will be called for each trailing character until a character does not satisfy the predicate.
 *           If `fn` is NULL, the function may return NULL or handle the invalid input according to the implementation.
 *
 * @return DString* A pointer to the new _DString with trailing characters removed based on the predicate function.
 *         Returns NULL if the input parameters are invalid.
 */
DString*	d_string_trim_right_by_prediacte_new(DString* dstring, match fn);

/**
 * @brief Converts a dynamic string to a dynamic array of characters.
 *
 * Converts the given _DString to a new `DArray` where each element in the array corresponds to a character
 * from the _DString. The conversion includes all characters up to and including the null terminator. The
 * resulting `DArray` will represent the same sequence of characters as the original _DString. This function
 * does not modify the original _DString; instead, it creates and returns a new `DArray`.
 *
 * @param dstring A pointer to the _DString that will be converted to a `DArray`. Must not be NULL.
 *
 * @return DArray* A pointer to a new `DArray` containing the characters from the _DString.
 *         Returns NULL if the input _DString is NULL or if the conversion fails.
 */
DArray* 	d_string_as_d_array(DString* dstring);

/**
 * @brief Frees the memory allocated for a dynamic string and its internal resources.
 *
 * Deallocates the memory used by the given _DString and sets the pointer to NULL. This function
 * should be called when the _DString is no longer needed to avoid memory leaks. The function handles
 * both the destruction of the _DString structure itself and the memory used for its internal string data.
 *
 * @param dstring A pointer to a pointer to the _DString to be destroyed. The function will set the
 *                original pointer to NULL after deallocation. Must not be NULL.
 */
void		d_string_destroy(DString** dstring);
#endif