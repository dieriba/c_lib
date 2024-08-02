#ifndef __D_STRING__H__
#define __D_STRING__H__

#include <dtypes.h>
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
 * @brief Creates a new dynamic string by copying an existing dynamic string.
 *
 * Allocates and initializes a new `_DString` structure that is a duplicate of the provided `_DString`. The new string will contain the same
 * content as the original, including its null terminator. The new `_DString` is independent of the original, meaning modifications to one will
 * not affect the other. The behavior is undefined if `dstring` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure to be copied. The behavior is undefined if `dstring` is `NULL`.
 *
 * @return DString* A pointer to the newly created `_DString` structure containing a copy of the original string. Returns `NULL` if memory
 *         allocation.
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
 * position `pos` and extending for `len` characters. The new dynamic string will
 * include a null terminator. If `pos` is out of range (i.e., greater than or equal
 * to the length of the string), and `str` is `NULL`, the function will create an
 * empty `_DString`. If `pos + len` exceeds the string boundary, the substring will
 * include all characters from `pos` to the end of the string. If `str` is not `NULL`,
 * but `pos` is out of range, the function will return NULL.
 *
 * @param str A pointer to the null-terminated C string from which to extract the
 *            substring. If `str` is NULL, an empty `_DString` will be created if
 *            `pos` is out of range.
 * @param pos The starting index in the C string from which to begin the substring.
 *             If `pos` is greater than or equal to the length of the string and `str`
 *             is not NULL, the function will return NULL.
 * @param len The number of characters to include in the substring. If `pos + len`
 *            exceeds the end of the C string, the substring will include all characters
 *            from `pos` to the end of the string.
 *
 * @return DString* A pointer to the newly created _DString structure containing
 *         the substring. Returns NULL if memory allocation fails or if the input
 *         parameters are invalid (e.g., `pos` is out of range and `str` is not NULL).
 */
DString* 	d_string_new_with_substring(const char* str, usize pos, usize len);

/**
 * @brief Retrieves the current capacity of a dynamic string.
 *
 * Returns the amount of memory allocated for the internal character array of
 * the specified _DString. The capacity indicates how many characters the
 * dynamic string can hold before needing to allocate additional memory.
 *
 * @param dstring A pointer to the _DString structure whose capacity is to be
 *                retrieved. Must not be NULL. . The behavior is undefined if `dstring` is `NULL`.
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
 * internal character array will always be null-terminated. The behavior is undefined
 * if `dstring` is `NULL`.
 *
 * @param dstring A pointer to the _DString structure to be resized. Must not be NULL.
 * 				  The behavior is undefined if `dstring` is `NULL`.
 * @param len The new length for the _DString. If `len` is zero, the string will be
 *            resized to an empty string.
 *
 * @return DString* A pointer to the resized _DString structure. Returns `NULL` if
 *         memory allocation fails.
 */
DString* 	d_string_resize(DString* dstring, usize len);

/**
 * @brief Modifies the internal capacity of a dynamic string.
 *
 * Adjusts the capacity of the given _DString to the specified new capacity. If the new capacity
 * is greater than the current capacity, additional memory will be allocated. If the new capacity
 * is less than the current length, the internal string may be truncated to fit within the new
 * capacity, but the length will be preserved. If `dstring` is `NULL`, the behavior is undefined.
 *
 * @param dstring A pointer to the _DString structure whose capacity is to be modified. 
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param new_capacity The new capacity for the _DString. If `new_capacity` is zero, the
 *                     function will set the internal capacity to zero, which may result in
 *                     an empty or truncated string.
 *
 * @return DString* A pointer to the modified _DString structure. Returns `NULL` if
 *         memory allocation fails.
 */
DString* 	d_string_modify_capacity(DString* dstring, usize new_capacity);

/**
 * @brief Appends a single character to the end of a dynamic string.
 *
 * Adds the specified character `c` to the end of the given _DString. If necessary, additional
 * memory will be allocated to accommodate the new character. The internal character array will
 * always be null-terminated after appending. If `dstring` is `NULL`, the behavior is undefined.
 *
 * @param dstring A pointer to the _DString structure to which the character is to be appended. 
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param c The character to append to the end of the dynamic string.
 *
 * @return DString* A pointer to the modified _DString structure with the appended character. 
 *         Returns `NULL` if memory allocation fails or if the input parameters are invalid.
 */
DString* 	d_string_push_char(DString* dstring, char c);

/**
 * @brief Appends a substring of a specified length to the end of a dynamic string.
 *
 * Adds the substring from the given `str_to_append`, starting from the beginning up to `len` characters,
 * to the end of the specified _DString. If necessary, additional memory will be allocated to accommodate
 * the new substring. The internal character array will always be null-terminated after appending. If `dstring`
 * is `NULL`, the behavior is undefined. If `str_to_append` is `NULL`, the behavior is also undefined.
 *
 * @param dstring A pointer to the _DString structure to which the substring is to be appended. 
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param str_to_append A pointer to the null-terminated C string from which the substring will be appended.
 *                      If `str_to_append` is `NULL`, the behavior is undefined.
 * @param len The number of characters to append from `str_to_append`. If `len` exceeds the length of `str_to_append`,
 *            only the available characters up to the end of `str_to_append` will be appended.
 *
 * @return DString* A pointer to the modified _DString structure with the appended substring. 
 *         Returns `NULL` if memory allocation fails.
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
 * @brief Appends the contents of one dynamic string to another.
 *
 * Modifies the `_DString` structure `dstring1` by appending the contents of `dstring2` to it. This operation will extend `dstring1` to include all characters from `dstring2`, including its null terminator if `dstring2` has one.
 * 
 * @param dstring1 A pointer to the `_DString` structure to which `dstring2` will be appended. Must not be `NULL`.
 * @param dstring2 A pointer to the `_DString` structure whose contents will be appended to `dstring1`. Must not be `NULL`.
 *
 * @return DString* A pointer to the modified `dstring1` after appending the contents of `dstring2`. Returns `NULL` if memory allocation fails.
 * 
 * @note The behavior is undefined if `dstring1` or `dstring2` is `NULL`.
 */
DString* 	d_string_push_str_of_dstring(DString* dstring1, DString* dstring2);

/**
 * @brief Replaces the content of a dynamic string with a new string.
 *
 * Replaces the content of the `_DString` pointed to by `dstring` with the content of `str`.
 * The existing `_DString` content will be replaced by the new string, which is `str`. If necessary,
 * memory will be reallocated to fit the new string. The internal character array of `_DString` will
 * be null-terminated after replacement. If `dstring` is `NULL` or if the `_DString` pointed to by `dstring`
 * is `NULL`, or if `str` is `NULL`, the behavior is undefined.
 *
 * @param dstring A pointer to a pointer to the `_DString` structure to be replaced. 
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param str A pointer to the null-terminated C string that will replace the content of `_DString`.
 *            The behavior is undefined if `str` is `NULL`.
 *
 * @return DString* A pointer to the `_DString` structure with its content replaced by `str`. 
 *         Returns `NULL` if memory allocation fails.
 */
DString* 	d_string_replace_from_str(DString* dstring, const char* str);

/**
 * @brief Replaces the content of a dynamic string with the content of another dynamic string.
 *
 * Replaces the content of the `_DString` pointed to by `dstring` with the content of the `_DString` pointed
 * to by `copy`. The existing content of the `_DString` pointed to by `dstring` will be replaced by the content
 * of `copy`. If necessary, memory will be reallocated to accommodate the new content. The internal character
 * array of `_DString` will always be null-terminated after replacement. If `dstring` or `copy` is `NULL`, or
 * if the `_DString` pointed to by `dstring` is `NULL`, the behavior is undefined.
 *
 * @param dstring A pointer to a pointer to the `_DString` structure to be replaced. 
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param copy A pointer to the `_DString` whose content will be used to replace the content of the `_DString`
 *             pointed to by `dstring`. The behavior is undefined if `copy` is `NULL`.
 *
 * @return DString* A pointer to the `_DString` structure with its content replaced by the content of `copy`. 
 *         Returns `NULL` if memory allocation fails and sets *dstring to `NULL`.
 */
DString* 	d_string_replace_from_dstring(DString* dstring, const DString* copy);



/**
 * @brief Compares two dynamic strings.
 *
 * Compares the content of the two `_DString` structures `dstring1` and `dstring2` lexicographically, similar to the behavior of `strcmp`.
 * The function returns an integer that indicates the relationship between the two strings:
 * - A value less than 0 if `dstring1` is lexicographically less than `dstring2`.
 * - A value of 0 if `dstring1` is equal to `dstring2`.
 * - A value greater than 0 if `dstring1` is lexicographically greater than `dstring2`.
 *
 * The behavior is undefined if either `dstring1` or `dstring2` is `NULL`.
 *
 * @param dstring1 A pointer to the first `_DString` structure to be compared. The behavior is undefined if `dstring1` is `NULL`.
 * @param dstring2 A pointer to the second `_DString` structure to be compared. The behavior is undefined if `dstring2` is `NULL`.
 *
 * @return int32 An integer indicating the result of the comparison:
 *         - A negative value if `dstring1` is less than `dstring2`.
 *         - Zero if `dstring1` is equal to `dstring2`.
 *         - A positive value if `dstring1` is greater than `dstring2`.
 */
int32		d_string_compare(DString* dstring1, DString* dstring2);

/**
 * @brief Compares a dynamic string with a null-terminated C string.
 *
 * Compares the content of the `_DString` structure `dstring` with the null-terminated C string `c_str`. The comparison is done lexicographically,
 * similar to the behavior of `strcmp`. The function returns an integer that indicates the relationship between the two strings:
 * - A value less than 0 if `dstring` is lexicographically less than `c_str`.
 * - A value of 0 if `dstring` is equal to `c_str`.
 * - A value greater than 0 if `dstring` is lexicographically greater than `c_str`.
 *
 * The behavior is undefined if either `dstring` or `c_str` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure to be compared. The behavior is undefined if `dstring` is `NULL`.
 * @param c_str A pointer to the null-terminated C string to compare against. The behavior is undefined if `c_str` is `NULL`.
 *
 * @return int32 An integer indicating the result of the comparison:
 *         - A negative value if `dstring` is less than `c_str`.
 *         - Zero if `dstring` is equal to `c_str`.
 *         - A positive value if `dstring` is greater than `c_str`.
 */
int32		d_string_compare_againg_c_string(DString* dstring, char* c_str);


/**
 * @brief Finds the index of the first character in a dynamic string that is not equal to a specified character.
 *
 * Searches for the first occurrence of a character in the `_DString` that is not equal to the specified character `c`,
 * starting from the given position `pos`. If such a character is found, its index is returned. If all characters from `pos`
 * to the end of the string are equal to `c`, or if `pos` is out of range, the function will return `MAX_SIZE_T`, which is
 * defined as the maximum value of `size_t`. The internal behavior is undefined if `dstring` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search. 
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param c The character to compare against.
 * @param pos The starting index from which to begin the search. If `pos` is greater than or equal to the length of the
 *            dynamic string, the function will return `MAX_SIZE_T`.
 *
 * @return usize The index of the first character in `_DString` that is not equal to `c`, starting from `pos`. 
 *         Returns `MAX_SIZE_T` if no such character is found, if `pos` is out of range.
 */
usize		d_string_find_first_not_matching_char_from_index(DString* dstring, char c, usize pos);

/**
 * @brief Finds the index of the first character in a dynamic string that is not equal to a specified character.
 *
 * Searches for the first occurrence of a character in the `_DString` that is not equal to the specified character `c`.
 * If such a character is found, its index is returned. If all characters in the dynamic string are equal to `c`,
 * the function will return `MAX_SIZE_T`, which is defined as the maximum value of `size_t`. The behavior is undefined
 * if `dstring` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search. 
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param c The character to compare against.
 *
 * @return usize The index of the first character in `_DString` that is not equal to `c`. 
 *         Returns `MAX_SIZE_T` if no such character is found.
 */
usize		d_string_find_first_not_matching_char_from_start(DString* dstring, char c);

/**
 * @brief Finds the index of the first character in a dynamic string that is not present in a specified set of characters.
 *
 * Searches for the first occurrence of a character in the `_DString` that is not in the set of characters specified by `str`,
 * starting from the given position `pos`. If such a character is found, its index is returned. If all characters from `pos`
 * to the end of the string are present in the set, or if `pos` is out of range, the function will return `MAX_SIZE_T`, which is
 * defined as the maximum value of `size_t`. The behavior is undefined if `dstring` or `str` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search.
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param str A pointer to a null-terminated C string containing the set of characters to check against.
 *            The behavior is undefined if `str` is `NULL`.
 * @param pos The starting index from which to begin the search. If `pos` is greater than or equal to the length of the
 *            dynamic string, the function will return `MAX_SIZE_T`.
 *
 * @return usize The index of the first character in `_DString` that is not in the set of characters specified by `str`,
 *         starting from `pos`. Returns `MAX_SIZE_T` if no such character is found, if `pos` is out of range.
 */
usize		d_string_find_first_char_not_of_str_from_index(DString* dstring, char* str, usize pos);


/**
 * @brief Finds the first character in a dynamic string that is not part of a given C string.
 *
 * Searches the `_DString` structure `dstring` for the first character that is not included in the null-terminated C string `str`.
 * The search starts from the beginning of `dstring`. If such a character is found, its index is returned. If no such character is found,
 * the function returns `MAX_SIZE_T`, which is the maximum value representable by `size_t`. The behavior is undefined if `dstring` or `str`
 * is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search. The behavior is undefined if `dstring` is `NULL`.
 * @param str A pointer to a null-terminated C string containing characters to be excluded from the search. The behavior is undefined if `str` is `NULL`.
 *
 * @return usize The index of the first character in `dstring` that is not in `str`. If all characters are in `str` or `dstring` is empty,
 *         returns `MAX_SIZE_T`.
 */
usize		d_string_find_first_char_not_of_str_from_start(DString* dstring, char* str);


/**
 * @brief Finds the index of the first character in a dynamic string that is not equal to a specified character, searching backwards.
 *
 * Searches for the first occurrence of a character in the `_DString` that is not equal to the specified character `c`,
 * starting from the given position `pos` and searching backwards towards the beginning of the string. If such a character
 * is found, its index is returned. If all characters from `pos` to the beginning of the string are equal to `c`, or if
 * `pos` is out of range, the function will return `MAX_SIZE_T`, which is defined as the maximum value of `size_t`. The
 * behavior is undefined if `dstring` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search.
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param c The character to compare against.
 * @param pos The starting index from which to begin the search, moving backwards. If `pos` is greater than or equal to
 *            the length of the dynamic string, the function will start the search from the end of the string.
 *
 * @return usize The index of the first character in `_DString` that is not equal to `c`, starting from `pos` and searching backwards.
 *         Returns `MAX_SIZE_T` if no such character is found.
 */
usize		d_string_find_last_of_matching_char_from_index(DString* dstring, char c, usize pos);

/**
 * @brief Finds the index of the first character in a dynamic string that is not equal to a specified character, searching backwards.
 *
 * Searches for the first occurrence of a character in the `_DString` that is not equal to the specified character `c`,
 * starting from the end of the string and moving backwards towards the beginning. If such a character is found, its index
 * is returned. If all characters in the string are equal to `c`, the function will return `MAX_SIZE_T`, which is defined as
 * the maximum value of `size_t`. The behavior is undefined if `dstring` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search.
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param c The character to compare against.
 *
 * @return usize The index of the first character in `_DString` that is not equal to `c`, starting from the end of the string
 *         and moving backwards. Returns `MAX_SIZE_T` if no such character is found.
 */
usize		d_string_find_last_of_matching_char_from_end(DString* dstring, char c);

/**
 * @brief Finds the index of the first character in a dynamic string that is not present in a specified set of characters, searching backwards.
 *
 * Searches for the first occurrence of a character in the `_DString` that is not present in the set of characters specified by `str`,
 * starting from the given position `pos` and searching backwards towards the beginning of the string. If such a character is found,
 * its index is returned. If all characters from `pos` to the beginning of the string are present in the set, or if `pos` is out of range,
 * the function will return `MAX_SIZE_T`, which is defined as the maximum value of `size_t`. The behavior is undefined if `dstring` or `str`
 * is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search.
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param str A pointer to a null-terminated C string containing the set of characters to check against.
 *            The behavior is undefined if `str` is `NULL`.
 * @param pos The starting index from which to begin the search, moving backwards. If `pos` is greater than or equal to the length of the
 *            dynamic string, the function will start the search from the end of the string.
 *
 * @return usize The index of the first character in `_DString` that is not in the set of characters specified by `str`,
 *         starting from `pos` and searching backwards. Returns `MAX_SIZE_T` if no such character is found, if `pos` is out of range,
 */
usize		d_string_find_last_char_not_of_str_from_index(DString* dstring, char* str, usize pos);


/**
 * @brief Finds the index of the first character in a dynamic string that is not present in a specified set of characters, searching backwards.
 *
 * Searches for the first occurrence of a character in the `_DString` that is not present in the set of characters specified by `str`,
 * starting from the end of the string and moving backwards towards the beginning. If such a character is found, its index is returned.
 * If all characters in the string are present in the set, the function will return `MAX_SIZE_T`, which is defined as the maximum value of `size_t`.
 * The behavior is undefined if `dstring` or `str` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search.
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param str A pointer to a null-terminated C string containing the set of characters to check against.
 *            The behavior is undefined if `str` is `NULL`.
 *
 * @return usize The index of the first character in `_DString` that is not in the set of characters specified by `str`,
 *         starting from the end of the string and moving backwards. Returns `MAX_SIZE_T` if no such character is found.
 */
usize		d_string_find_last_char_not_of_str_from_end(DString* dstring, char* str);

/**
 * @brief Finds the first character in a dynamic string that does not match a specified predicate function, starting from a given position.
 *
 * Searches the `_DString` structure `dstring` for the first character that does not satisfy the condition defined by the predicate function `fn`.
 * The search begins at the specified position `pos` and continues to the end of the string. A character is considered not to match the predicate
 * if the predicate function returns `0` for that character. If such a character is found, its index is returned. If all characters from `pos` onward
 * satisfy the predicate (i.e., `fn` returns non-zero for all characters), or if `pos` is greater than or equal to the length of the string, the function
 * returns `MAX_SIZE_T`, which is the maximum value representable by `size_t`. The behavior is undefined if `dstring` or `fn` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search. The behavior is undefined if `dstring` is `NULL`.
 * @param fn A pointer to a predicate function that takes a character as input and returns a non-zero value if the character matches the condition,
 *           or `0` otherwise. The behavior is undefined if `fn` is `NULL`.
 * @param pos The starting index in `dstring` from which to begin the search. If `pos` is greater than or equal to the length of the string,
 *            the function returns `MAX_SIZE_T`.
 *
 * @return usize The index of the first character in `dstring` starting from `pos` that does not satisfy the predicate (i.e., where `fn` returns `0`).
 *         If all characters from `pos` onward satisfy the predicate, or if `pos` is out of range, returns `MAX_SIZE_T`.
 */
usize		d_string_find_first_not_of_predicate_from_index(DString* dstring, match fn, usize pos);

/**
 * @brief Finds the first character in a dynamic string that does not match a specified predicate function.
 *
 * Searches the `_DString` structure `dstring` for the first character that does not satisfy the condition defined by the predicate function `fn`.
 * The search starts from the beginning of `dstring`. A character is considered not to match the predicate if the predicate function returns `0` for that character.
 * If such a character is found, its index is returned. If all characters satisfy the predicate (i.e., `fn` returns non-zero for all characters),
 * the function returns `MAX_SIZE_T`, which is the maximum value representable by `size_t`. The behavior is undefined if `dstring` or `fn` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search. The behavior is undefined if `dstring` is `NULL`.
 * @param fn A pointer to a predicate function that takes a character as input and returns a non-zero value if the character matches the condition,
 *           or `0` otherwise. The behavior is undefined if `fn` is `NULL`.
 *
 * @return usize The index of the first character in `dstring` that does not satisfy the predicate (i.e., where `fn` returns `0`). If all characters satisfy the predicate,
 *         returns `MAX_SIZE_T`.
 */
usize		d_string_find_first_not_of_predicate_from_start(DString* dstring, match fn);

/**
 * @brief Finds the first character in a dynamic string, searching backwards from a given position, that does not match a specified predicate function.
 *
 * Searches the `_DString` structure `dstring` for the first character (in reverse order) that does not satisfy the condition defined by the predicate function `fn`.
 * The search begins at the specified position `pos` and proceeds backwards to the beginning of the string. If `pos` is greater than or equal to the length of the string,
 * the search will start from the end of the string. A character is considered not to match the predicate if the predicate function returns `0` for that character.
 * If such a character is found, its index is returned. If all characters from the start of the string to `pos` (or to the end if `pos` is out of bounds) satisfy
 * the predicate (i.e., `fn` returns non-zero for all characters), the function returns `MAX_SIZE_T`, which is the maximum value representable by `size_t`. 
 * The behavior is undefined if `dstring` or `fn` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search. The behavior is undefined if `dstring` is `NULL`.
 * @param fn A pointer to a predicate function that takes a character as input and returns a non-zero value if the character matches the condition,
 *           or `0` otherwise. The behavior is undefined if `fn` is `NULL`.
 * @param pos The starting index in `dstring` from which to begin the reverse search. If `pos` is greater than or equal to the length of the string,
 *            the search starts from the end of the string.
 *
 * @return usize The index of the first character in `dstring` starting from `pos` and searching backwards that does not satisfy the predicate (i.e., where `fn` returns `0`).
 *         If all characters from `pos` backwards (or from the end of the string if `pos` is out of range) satisfy the predicate, returns `MAX_SIZE_T`.
 */
usize		d_string_find_last_not_of_predicate_from_index(DString* dstring, match fn, usize pos);

/**
 * @brief Finds the first character in a dynamic string, searching backwards, that does not match a specified predicate function.
 *
 * Searches the `_DString` structure `dstring` for the first character (in reverse order) that does not satisfy the condition defined by the predicate function `fn`.
 * The search begins from the end of the string and proceeds backwards to the beginning. A character is considered not to match the predicate if the predicate function returns `0` for that character.
 * If such a character is found, its index is returned. If all characters in the string satisfy the predicate (i.e., `fn` returns non-zero for all characters), the function returns `MAX_SIZE_T`,
 * which is the maximum value representable by `size_t`. The behavior is undefined if `dstring` or `fn` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search. The behavior is undefined if `dstring` is `NULL`.
 * @param fn A pointer to a predicate function that takes a character as input and returns a non-zero value if the character matches the condition,
 *           or `0` otherwise. The behavior is undefined if `fn` is `NULL`.
 *
 * @return usize The index of the first character in `dstring` searched backwards from the end that does not satisfy the predicate (i.e., where `fn` returns `0`).
 *         If all characters in `dstring` satisfy the predicate, returns `MAX_SIZE_T`.
 */
usize		d_string_find_last_not_of_predicate_from_end(DString* dstring, match fn);

/**
 * @brief Finds the index of the first occurrence of a specified character in a dynamic string, starting from a given position.
 *
 * Searches for the first occurrence of the character `c` in the `_DString`, starting from the specified position `pos` and moving
 * forward towards the end of the string. If the character is found, its index is returned. If the character is not found or if `pos`
 * is out of range, the function will return `MAX_SIZE_T`, which is defined as the maximum value of `size_t`. The behavior is undefined
 * if `dstring` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search.
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param c The character to find.
 * @param pos The starting index from which to begin the search. If `pos` is greater than or equal to the length of the dynamic string,
 *            the function will return `MAX_SIZE_T`.
 *
 * @return usize The index of the first occurrence of `c` in `_DString`, starting from `pos` and moving forward. 
 *         Returns `MAX_SIZE_T` if the character is not found, if `pos` is out of range.
 */
usize		d_string_find_first_matching_char_from_index(DString* dstring, char c, usize pos);

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
 *         Returns `MAX_SIZE_T` if the character is not found.
 */
usize		d_string_find_first_matching_char_from_start(DString* dstring, char c);

/**
 * @brief Finds the index of the first occurrence of a specified substring in a dynamic string, starting from a given position.
 *
 * Searches for the first occurrence of the substring specified by `str` in the `_DString`, starting from the specified position `pos`
 * and moving forward towards the end of the string. If the substring is found, its starting index is returned. If the substring is not
 * found or if `pos` is out of range, the function will return `MAX_SIZE_T`, which is defined as the maximum value of `size_t`. 
 * The behavior is undefined if `dstring` or `str` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search.
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param str A pointer to a null-terminated C string representing the substring to find.
 *            The behavior is undefined if `str` is `NULL`.
 * @param pos The starting index from which to begin the search. If `pos` is greater than or equal to the length of the dynamic string,
 *            the function will return `MAX_SIZE_T`.
 *
 * @return usize The index of the first occurrence of the substring `str` in `_DString`, starting from `pos` and moving forward.
 *         Returns `MAX_SIZE_T` if the substring is not found, if `pos` is out of range.
 */
usize		d_string_find_matching_str_from_index(DString* dstring, const char *str, usize pos);

/**
 * @brief Finds the index of the first occurrence of a specified substring in a dynamic string, starting from the beginning.
 *
 * Searches for the first occurrence of the substring specified by `str` in the `_DString`, starting from the beginning of the string
 * and moving forward towards the end. If the substring is found, its starting index is returned. If the substring is not found,
 * the function will return `MAX_SIZE_T`, which is defined as the maximum value of `size_t`. The behavior is undefined if `dstring` or `str`
 * is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search.
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param str A pointer to a null-terminated C string representing the substring to find.
 *            The behavior is undefined if `str` is `NULL`.
 *
 * @return usize The index of the first occurrence of the substring `str` in `_DString`, starting from the beginning and moving forward.
 *         Returns `MAX_SIZE_T` if the substring is not found.
 */
usize		d_string_find_matching_str_from_start(DString* dstring, const char *str);

/**
 * @brief Finds the index of the first character in a dynamic string that matches a given predicate function, starting from a specified position.
 *
 * Searches for the first occurrence of a character in the `_DString` that satisfies the condition defined by the predicate function `fn`,
 * starting from the specified position `pos` and moving forward towards the end of the string. The predicate function `fn` is called
 * for each character in the string, and if it returns a non-zero value, the character is considered a match. If a matching character is found,
 * its index is returned. If no matching character is found or if `pos` is out of range, the function will return `MAX_SIZE_T`, which is
 * defined as the maximum value of `size_t`. The behavior is undefined if `dstring` or `fn` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search.
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param fn A pointer to a function that takes a character and returns a non-zero value if the character matches the predicate, 
 *            or zero if it does not match. The behavior is undefined if `fn` is `NULL`.
 * @param pos The starting index from which to begin the search. If `pos` is greater than or equal to the length of the dynamic string,
 *            the function will return `MAX_SIZE_T`.
 *
 * @return usize The index of the first character in `_DString` that matches the predicate function `fn`, starting from `pos` and moving forward.
 *         Returns `MAX_SIZE_T` if no such character is found, if `pos` is out of range.
 */
usize		d_string_find_first_matching_predicate_from_index(DString* dstring, match fn, usize pos);

/**
 * @brief Finds the index of the first character in a dynamic string that matches a given predicate function, starting from the beginning.
 *
 * Searches for the first occurrence of a character in the `_DString` that satisfies the condition defined by the predicate function `fn`,
 * starting from the beginning of the string and moving forward. The predicate function `fn` is called for each character in the string,
 * and if it returns a non-zero value, the character is considered a match. If a matching character is found, its index is returned.
 * If no matching character is found, the function will return `MAX_SIZE_T`, which is defined as the maximum value of `size_t`. 
 * The behavior is undefined if `dstring` or `fn` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search.
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param fn A pointer to a function that takes a character and returns a non-zero value if the character matches the predicate, 
 *            or zero if it does not match. The behavior is undefined if `fn` is `NULL`.
 *
 * @return usize The index of the first character in `_DString` that matches the predicate function `fn`, starting from the beginning and moving forward.
 *         Returns `MAX_SIZE_T` if no such character is found.
 */
usize		d_string_find_first_matching_predicate_from_start(DString* dstring, match fn);

/**
 * @brief Finds the last occurrence of a specified character in a dynamic string, searching backwards from a given position.
 *
 * Searches the `_DString` structure `dstring` for the last occurrence of the character `c`, starting from the specified position `pos` and proceeding backwards to the beginning of the string.
 * If `pos` is greater than or equal to the length of the string, the search will start from the end of the string. If the character is found, its index is returned. 
 * If the character is not found, or if `pos` is out of range, the function returns `MAX_SIZE_T`, which is the maximum value representable by `size_t`. 
 * The behavior is undefined if `dstring` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search. The behavior is undefined if `dstring` is `NULL`.
 * @param c The character to search for in the dynamic string.
 * @param pos The starting index in `dstring` from which to begin the reverse search. If `pos` is greater than or equal to the length of the string,
 *            the search starts from the end of the string.
 *
 * @return usize The index of the last occurrence of the character `c` in `dstring`, starting from `pos` and searching backwards. If the character is not found,
 *         or if `pos` is out of range, returns `MAX_SIZE_T`.
 */
usize		d_string_find_last_matching_char_from_index(DString* dstring, char c, usize pos);


/**
 * @brief Finds the index of the last occurrence of a specified character in a dynamic string, starting from the end and moving backwards.
 *
 * Searches for the last occurrence of the character `c` in the `_DString`, starting from the end of the string and moving backwards
 * towards the beginning. If the character is found, its index is returned. If the character is not found, the function will return 
 * `MAX_SIZE_T`, which is defined as the maximum value of `size_t`. The behavior is undefined if `dstring` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search.
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param c The character to find.
 *
 * @return usize The index of the last occurrence of `c` in `_DString`, starting from the end and moving backwards.
 *         Returns `MAX_SIZE_T` if the character is not found.
 */
usize		d_string_find_last_matching_char_from_end(DString* dstring, char c);

/**
 * @brief Finds the last occurrence of a specified substring in a dynamic string, searching backwards from a given position.
 *
 * Searches the `_DString` structure `dstring` for the last occurrence of the substring `str`, starting from the specified position `pos` and proceeding backwards to the beginning of the string.
 * If `pos` is greater than or equal to the length of the string, the search will start from the end of the string. If the substring is found, the index of the first character of the last occurrence
 * is returned. If the substring is not found, or if `pos` is out of range, the function returns `MAX_SIZE_T`, which is the maximum value representable by `size_t`. 
 * The behavior is undefined if `dstring` or `str` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search. The behavior is undefined if `dstring` is `NULL`.
 * @param str A pointer to the null-terminated substring to search for in the dynamic string. The behavior is undefined if `str` is `NULL`.
 * @param pos The starting index in `dstring` from which to begin the reverse search. If `pos` is greater than or equal to the length of the string,
 *            the search starts from the end of the string.
 *
 * @return usize The index of the first character of the last occurrence of the substring `str` in `dstring`, starting from `pos` and searching backwards. 
 *         If the substring is not found, or if `pos` is out of range, returns `MAX_SIZE_T`.
 */
usize		d_string_find_last_matching_str_from_index(DString* dstring, const char *str, usize pos);

/**
 * @brief Finds the index of the last occurrence of a specified substring in a dynamic string, starting from the end and moving backwards.
 *
 * Searches for the last occurrence of the substring specified by `str` in the `_DString`, starting from the end of the string and moving backwards
 * towards the beginning. If the substring is found, its starting index is returned. If the substring is not found, the function will return
 * `MAX_SIZE_T`, which is defined as the maximum value of `size_t`. The behavior is undefined if `dstring` or `str` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search.
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param str A pointer to a null-terminated C string representing the substring to find.
 *            The behavior is undefined if `str` is `NULL`.
 *
 * @return usize The index of the last occurrence of the substring `str` in `_DString`, starting from the end and moving backwards.
 *         Returns `MAX_SIZE_T` if the substring is not found.
 */
usize		d_string_find_last_matching_str_from_end(DString* dstring, const char *str);


/**
 * @brief Finds the last character in a dynamic string, searching backwards from a given position, that matches a specified predicate function.
 *
 * Searches the `_DString` structure `dstring` for the last character (in reverse order) that satisfies the condition defined by the predicate function `fn`.
 * The search begins at the specified position `pos` and proceeds backwards to the beginning of the string. A character is considered to match the predicate if the predicate function returns a non-zero value for that character.
 * If such a character is found, its index is returned. If no character that matches the predicate is found, or if `pos` is out of range, the function returns `MAX_SIZE_T`, 
 * which is the maximum value representable by `size_t`. The behavior is undefined if `dstring` or `fn` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search. The behavior is undefined if `dstring` is `NULL`.
 * @param fn A pointer to a predicate function that takes a character as input and returns a non-zero value if the character matches the condition,
 *           or `0` otherwise. The behavior is undefined if `fn` is `NULL`.
 * @param pos The starting index in `dstring` from which to begin the reverse search. If `pos` is greater than or equal to the length of the string,
 *            the search starts from the end of the string.
 *
 * @return usize The index of the last character in `dstring` starting from `pos` and searching backwards that satisfies the predicate (i.e., where `fn` returns a non-zero value).
 *         If no character matches the predicate, or if `pos` is out of range, returns `MAX_SIZE_T`.
 */
usize		d_string_find_last_matching_predicate_from_index(DString* dstring, match fn, usize pos);

/**
 * @brief Finds the index of the last character in a dynamic string that matches a given predicate function, starting from the end and moving backwards.
 *
 * Searches for the last occurrence of a character in the `_DString` that satisfies the condition defined by the predicate function `fn`,
 * starting from the end of the string and moving backwards towards the beginning. The predicate function `fn` is called for each character
 * in the string, and if it returns a non-zero value, the character is considered a match. If a matching character is found, its index is returned.
 * If no matching character is found, the function will return `MAX_SIZE_T`, which is defined as the maximum value of `size_t`. The behavior is
 * undefined if `dstring` or `fn` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure in which to search.
 *                The behavior is undefined if `dstring` is `NULL`.
 * @param fn A pointer to a function that takes a character and returns a non-zero value if the character matches the predicate,
 *            or zero if it does not match. The behavior is undefined if `fn` is `NULL`.
 *
 * @return usize The index of the last character in `_DString` that matches the predicate function `fn`, starting from the end and moving backwards.
 *         Returns `MAX_SIZE_T` if no such character is found.
 */
usize		d_string_find_last_matching_predicate_from_end(DString* dstring, match fn);


/**
 * @brief Modifies a dynamic string to represent a substring starting from a specified position.
 *
 * Adjusts the `_DString` to contain a substring starting from the specified position `pos` and extending for `len` characters. 
 * This operation changes the original `_DString` to represent the new substring. If `pos` is greater than or equal to the length of 
 * the string, the function will return `NULL`, indicating an invalid operation. If `pos + len` exceeds the end of the string, the 
 * substring will include all characters from `pos` to the end of the string. The function does not allocate new memory; it only adjusts
 * the existing `_DString`. The behavior is undefined if `dstring` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure to be modified. The behavior is undefined if `dstring` is `NULL`.
 * @param pos The starting index in the `_DString` from which to begin the substring.
 *            If `pos` is greater than or equal to the length of the string, the function will return `NULL`.
 * @param len The number of characters to include in the substring. If `pos + len` exceeds the end of the string, the substring will
 *            include all characters from `pos` to the end of the string.
 *
 * @return DString* A pointer to the modified `_DString` structure containing the substring, or `NULL` if `pos` is greater than
 *         or equal to the length of the string, or if memory allocation fails.
 */
DString*	d_string_sub_string_in_place(DString* dstring, usize pos, usize len);

/**
 * @brief Trims characters from the beginning of a dynamic string based on a specified character.
 *
 * Modifies the `_DString` to remove all occurrences of the specified character `c` from the beginning of the string. This operation
 * adjusts the original `_DString` to exclude the specified character from its start until a different character is encountered. If the 
 * resulting string is empty after trimming, the `_DString` will be modified to represent an empty string. The function does not allocate
 * new memory; it only adjusts the existing `_DString`. The behavior is undefined if `dstring` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure to be modified. The behavior is undefined if `dstring` is `NULL`.
 * @param c The character to be removed from the beginning of the string.
 *
 * @return DString* A pointer to the modified `_DString` structure with leading occurrences of the character `c` removed.
 */
DString*	d_string_trim_left_by_char_in_place(DString* dstring, char c);

/**
 * @brief Trims characters from the beginning of a dynamic string based on a specified predicate function.
 *
 * Modifies the `_DString` to remove all characters from the beginning of the string that satisfy the condition defined by the predicate
 * function `fn`. The predicate function is called for each character in the string from the beginning, and if it returns a non-zero value,
 * the character is removed. The process continues until a character that does not satisfy the predicate is encountered. If the resulting
 * string is empty after trimming, the `_DString` will be modified to represent an empty string. The function does not allocate new memory;
 * it only adjusts the existing `_DString`. The behavior is undefined if `dstring` or `fn` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure to be modified. The behavior is undefined if `dstring` is `NULL`.
 * @param fn A pointer to a function that takes a character and returns a non-zero value if the character should be removed,
 *            or zero if it should be kept. The behavior is undefined if `fn` is `NULL`.
 *
 * @return DString* A pointer to the modified `_DString` structure with leading characters removed based on the predicate function.
 */
DString*	d_string_trim_left_by_predicate_in_place(DString* dstring, match fn);

/**
 * @brief Creates a new dynamic string by trimming characters from the beginning of an existing dynamic string based on a specified character.
 *
 * Allocates and initializes a new `_DString` that represents a substring of the original `_DString` with all occurrences of the specified
 * character `c` removed from the beginning. The new string starts from the first character that is not equal to `c`. If the resulting string
 * is empty after trimming, the new `_DString` will represent an empty string. If the original `_DString` is empty or if all characters
 * are equal to `c`, the resulting string will be empty. The behavior is undefined if `dstring` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure from which to create the new trimmed string. The behavior is undefined if `dstring` is `NULL`.
 * @param c The character to be removed from the beginning of the string.
 *
 * @return DString* A pointer to the newly created `_DString` structure with leading occurrences of the character `c` removed.
 *         Returns `NULL` if memory allocation fails.
 */
DString*	d_string_trim_left_by_char_new(DString* dstring, char c);

/**
 * @brief Creates a new dynamic string by trimming characters from the beginning of an existing dynamic string based on a specified predicate function.
 *
 * Allocates and initializes a new `_DString` that represents a substring of the original `_DString` with all characters removed from the
 * beginning that satisfy the condition defined by the predicate function `fn`. The predicate function is called for each character in the
 * original string from the beginning, and if it returns a non-zero value, the character is removed. The new string starts from the first
 * character that does not satisfy the predicate. If the resulting string is empty after trimming, the new `_DString` will represent an
 * empty string. The behavior is undefined if `dstring` or `fn` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure from which to create the new trimmed string. The behavior is undefined if `dstring` is `NULL`.
 * @param fn A pointer to a function that takes a character and returns a non-zero value if the character should be removed, or zero if it should be kept.
 *            The behavior is undefined if `fn` is `NULL`.
 *
 * @return DString* A pointer to the newly created `_DString` structure with leading characters removed based on the predicate function.
 *         Returns `NULL` if memory allocation fails.
 */
DString*	d_string_trim_left_by_predicate_new(DString* dstring, match fn);


/**
 * @brief Trims characters from the end of a dynamic string based on a specified character.
 *
 * Modifies the `_DString` to remove all occurrences of the specified character `c` from the end of the string. This operation adjusts
 * the original `_DString` to exclude the specified character from the end until a different character is encountered. If the resulting
 * string is empty after trimming, the `_DString` will be modified to represent an empty string. The function does not allocate new memory;
 * it only adjusts the existing `_DString`. The behavior is undefined if `dstring` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure to be modified. The behavior is undefined if `dstring` is `NULL`.
 * @param c The character to be removed from the end of the string.
 *
 * @return DString* A pointer to the modified `_DString` structure with trailing occurrences of the character `c` removed.
 */
DString*	d_string_trim_right_by_char_in_place(DString* dstring, char c);

/**
 * @brief Trims characters from the end of a dynamic string based on a specified predicate function.
 *
 * Modifies the `_DString` to remove all characters from the end of the string that satisfy the condition defined by the predicate function `fn`.
 * The predicate function is called for each character from the end of the string towards the beginning. If it returns a non-zero value, the character
 * is removed. The process continues until a character that does not satisfy the predicate is encountered. If the resulting string is empty after trimming,
 * the `_DString` will be modified to represent an empty string. The function does not allocate new memory; it only adjusts the existing `_DString`.
 * The behavior is undefined if `dstring` or `fn` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure to be modified. The behavior is undefined if `dstring` is `NULL`.
 * @param fn A pointer to a function that takes a character and returns a non-zero value if the character should be removed, or zero if it should be kept.
 *            The behavior is undefined if `fn` is `NULL`.
 *
 * @return DString* A pointer to the modified `_DString` structure with trailing characters removed based on the predicate function.
 */
DString*	d_string_trim_right_by_predicate_in_place(DString* dstring, match fn);

/**
 * @brief Creates a new dynamic string by trimming characters from the end of an existing dynamic string based on a specified character.
 *
 * Allocates and initializes a new `_DString` that represents a substring of the original `_DString` with all occurrences of the specified
 * character `c` removed from the end. The new string starts from the first character that is not equal to `c` when traversed from the end.
 * If the resulting string is empty after trimming, the new `_DString` will represent an empty string. If the original `_DString` is empty
 * or if all characters are equal to `c`, the resulting string will be empty. The behavior is undefined if `dstring` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure from which to create the new trimmed string. The behavior is undefined if `dstring` is `NULL`.
 * @param c The character to be removed from the end of the string.
 *
 * @return DString* A pointer to the newly created `_DString` structure with trailing occurrences of the character `c` removed.
 *         Returns `NULL` if memory allocation fails.
 */
DString*	d_string_trim_right_by_char_new(DString* dstring, char c);

/**
 * @brief Creates a new dynamic string by trimming characters from the end of an existing dynamic string based on a specified predicate function.
 *
 * Allocates and initializes a new `_DString` that represents a substring of the original `_DString` with all characters removed from the
 * end that satisfy the condition defined by the predicate function `fn`. The predicate function is called for each character from the end of
 * the string towards the beginning, and if it returns a non-zero value, the character is removed. The new string starts from the first character
 * that does not satisfy the predicate. If the resulting string is empty after trimming, the new `_DString` will represent an empty string.
 * The behavior is undefined if `dstring` or `fn` is `NULL`.
 *
 * @param dstring A pointer to the `_DString` structure from which to create the new trimmed string. The behavior is undefined if `dstring` is `NULL`.
 * @param fn A pointer to a function that takes a character and returns a non-zero value if the character should be removed, or zero if it should be kept.
 *            The behavior is undefined if `fn` is `NULL`.
 *
 * @return DString* A pointer to the newly created `_DString` structure with trailing characters removed based on the predicate function.
 *         Returns `NULL` if memory allocation fails.
 */
DString*	d_string_trim_right_by_predicate_new(DString* dstring, match fn);


/**
 * @brief Converts a dynamic string into a dynamic array of characters.
 *
 * Allocates and initializes a new `DArray` structure where each element of the array corresponds to a character from the given `_DString` structure. 
 * The conversion includes all characters from the `DString`, including the null terminator if present.
 * 
 * @param dstring A pointer to the `_DString` structure to be converted. If `dstring` is `NULL`, the behavior is undefined.
 *
 * @return DArray* A pointer to the newly created `DArray` structure containing the characters from `dstring`. Returns `NULL` if memory allocation fails
 *         or if `dstring` is `NULL`.
 */
//DArray*		d_string_convert_as_d_array(DString* dstring);


/**
 * @brief Deallocates memory used by a dynamic string and sets its pointer to NULL.
 *
 * Frees the memory associated with the `_DString` structure pointed to by `dstring` and sets the pointer to `NULL`. This function ensures
 * that memory is properly released and that the pointer no longer references the deallocated memory. The behavior is undefined if `dstring`
 * is `NULL` or if the pointer `dstring` itself is `NULL`.
 *
 * @param dstring A pointer to a pointer to the `_DString` structure to be destroyed. If `*dstring` is `NULL`, the function has no effect.
 *                After the function call, `*dstring` will be set to `NULL`.
 */
void		d_string_destroy(DString** dstring);


#endif