#ifndef __D_STRING__H__
#define __D_STRING__H__

#include <dtypes.h>
#include <darray.h>
typedef struct _DString DString;


/**
 * DArray:
 * @string: a pointer to the string. The string may be moved as
 *      char are added to the #DString.
 * @len: the number of char in the #DString.
 *
 * Contains the public fields of a DString.
 */
struct _DString {
	char    *string;
	usize     	len;
};

typedef usize(*match)(char c);


/**
 * d_string_new:
 *
 * Creates a new #DString with default settings.
 * Returns: the new #DString
 */
DString* d_string_new(void);

/**
 * d_string_new_with_string:
 * @str: Initial string to populate the new #DString with.
 *
 * Creates a new #DString initialized with the given string.
 * Returns: the new #DString
 */
DString* 	d_string_new_with_string(const char* str);

/**
 * d_string_new_with_reserve:
 * @reserve: Initial capacity for the new #DString.
 *
 * Creates a new #DString with the specified initial capacity.
 * Returns: the new #DString
 */
DString* 	d_string_new_with_reserve(usize reserve);

/**
 * d_string_get_capacity:
 * @DString: A #DString.
 *
 * Returns: the capacity of a given #DString
 */
usize		d_string_get_capacity(DString* dstring);

/**
 * d_string_resize:
 * @dstring: The #DString to resize.
 * @len: The new length to resize the string to.
 *
 * Resizes the #DString to the specified length.
 * If the new length is greater than the current capacity, reallocation occurs.
 * Returns: the resized #DString
 */
DString* 	d_string_resize(DString* dstring, usize len);

/**
 * d_string_increase_capacity:
 * @dstring: The #DString to increase the capacity of.
 * @len: The new capacity to increase the string to.
 *
 * Increases the capacity of the #DString to the specified length.
 * memory where point string may move
 * Returns: the #DString with increased capacity.
 */
DString* 	d_string_increase_capacity(DString* dstring, usize len);

/**
 * d_string_push_char:
 * @dstring: The #DString to modify.
 * @c: The character to append.
 *
 * Appends a single character to the end of the #DString.
 * reallocation may occurs.
 * Returns: the modified #DString
 */
DString* 	d_string_push_char(DString* dstring, char c);

/**
 * d_string_push_str:
 * @dstring: The #DString to modify.
 * @str_to_append: The string to append.
 *
 * Appends a string to the end of the #DString.
 * reallocation may occurs.
 * Returns: the modified #DString
 */
DString* 	d_string_push_str(DString* dstring, char *str_to_append);

/**
 * d_string_copy:
 * @dstring: The #DString to copy.
 *
 * Creates a new #DString that is a copy of the given string.
 * Returns: the new copy of the #DString
 */
DString*	d_string_copy(DString* dstring);


/**
 * d_string_compare:
 * @str1: The first #DString to compare.
 * @str2: The second #DString to compare.
 *
 * Compares two #DStrings.
 * Returns: An integer less than, equal to, or greater than zero if str1 is found, respectively, 
 * to be less than, to match, or be greater than str2.
 */
int32		d_string_compare(DString* str1, DString* str2);

/**
 * d_string_rfind_by_char:
 * @dstring: The #DString to search.
 * @c: The character to find.
 *
 * Finds the last occurrence of the character in the #DString.
 * Returns: The position of the character, or a MAX_SIZE_T_VALUE (which is defined in header as max value of size_t) value if not found.
 */
usize		d_string_rfind_by_char(DString* dstring, char c);

/**
 * d_string_rfind_by_str:
 * @dstring: The #DString to search.
 * @str: The string to find.
 *
 * Finds the last occurrence of the string in the #DString.
 * Returns: The position of the first char of the string, or a MAX_SIZE_T_VALUE (which is defined in header as max value of size_t) value if not found.
 */
usize		d_string_rfind_by_str(DString* dstring, const char *str);

/**
 * d_string_rfind_by_str_from:
 * @dstring: The #DString to search.
 * @str: The string to find.
 * @pos: The position to start the search from.
 *
 * Finds the last occurrence of the string in the #DString starting from a specified position.
 * Returns: The position of the first char of the string, or a MAX_SIZE_T_VALUE (which is defined in header as max value of size_t) value if not found.
 */
usize		d_string_rfind_by_str_from(DString* dstring, const char *str, usize pos);

/**
 * d_string_rfind_by_predicate:
 * @dstring: The #DString to search.
 * @fn: The predicate function to use for matching.
 *
 * Finds the last occurrence of a match based on the predicate function in the #DString.
 * Returns: The position of the match, or a MAX_SIZE_T_VALUE (which is defined in header as max value of size_t) value if not found.
 */
usize		d_string_rfind_by_predicate(DString* dstring, match fn);

/**
 * d_string_rfind_by_char_from:
 * @dstring: The #DString to search.
 * @c: The character to find.
 * @pos: The position to start the search from.
 *
 * Finds the last occurrence of the character in the #DString starting from a specified position.
 * Returns: The position of the character, or a MAX_SIZE_T_VALUE (which is defined in header as max value of size_t) value if not found.
 */
usize		d_string_rfind_by_char_from(DString* dstring, char c, usize pos);

/**
 * d_string_rfind_by_predicate_from:
 * @dstring: The #DString to search.
 * @fn: The predicate function to use for matching.
 * @pos: The position to start the search from.
 *
 * Finds the last occurrence of a match based on the predicate function in the #DString starting from a specified position.
 * Returns: The position of the match, or a MAX_SIZE_T_VALUE (which is defined in header as max value of size_t) value if not found.
 */
usize		d_string_rfind_by_predicate_from(DString* dstring, match fn, usize pos);


/**
 * d_string_find_by_char:
 * @dstring: The #DString to search.
 * @c: The character to find.
 *
 * Finds the first occurrence of the character in the #DString.
 * Returns: The position of the character, or a MAX_SIZE_T_VALUE (which is defined in header as max value of size_t) value if not found.
 */
usize		d_string_find_by_char(DString* dstring, char c);

/**
 * d_string_find_by_str:
 * @dstring: The #DString to search.
 * @str: The string to find.
 *
 * Finds the first occurrence of the string in the #DString.
 * Returns: The position of the first char of the string, or a MAX_SIZE_T_VALUE (which is defined in header as max value of size_t) value if not found.
 */
usize		d_string_find_by_str(DString* dstring, const char *str);

/**
 * d_string_find_by_str_from:
 * @dstring: The #DString to search.
 * @str: The string to find.
 * @pos: The position to start the search from.
 *
 * Finds the first occurrence of the string in the #DString starting from a specified position.
 * Returns: The position of the first char of the string, or a MAX_SIZE_T_VALUE (which is defined in header as max value of size_t) value if not found.
 */
usize		d_string_find_by_str_from(DString* dstring, const char *str, usize pos);

/**
 * d_string_find_by_predicate:
 * @dstring: The #DString to search.
 * @fn: The predicate function to use for matching.
 *
 * Finds the first occurrence of a match based on the predicate function in the #DString.
 * Returns: The position of the match, or a MAX_SIZE_T_VALUE (which is defined in header as max value of size_t) value if not found.
 */
usize		d_string_find_by_predicate(DString* dstring, match fn);

/**
 * d_string_find_by_char_from:
 * @dstring: The #DString to search.
 * @c: The character to find.
 * @pos: The position to start the search from.
 *
 * Finds the first occurrence of the character in the #DString starting from a specified position.
 * Returns: The position of the character, or MAX_SIZE_T_VALUE (which is defined in header as max value of size_t) value if not found.
 */
usize		d_string_find_by_char_from(DString* dstring, char c, usize pos);

/**
 * d_string_find_by_predicate_from:
 * @dstring: The #DString to search.
 * @fn: The predicate function to use for matching.
 * @pos: The position to start the search from.
 *
 * Finds the first occurrence of a match based on the predicate function in the #DString starting from a specified position.
 * Returns: The position of the match, or MAX_SIZE_T_VALUE (which is defined in header as max value of size_t) value if not found.
 */
usize		d_string_find_by_predicate_from(DString* dstring, match fn, usize pos);

/**
 * d_string_sub_string:
 * @dstring: The #DString to create a substring from.
 * @pos: The starting position of the substring.
 * @len: The length of the substring.
 *
 * Creates a new #DString that is a substring of the given string.
 * Returns: The new substring #DString
 */
DString*	d_string_sub_string(DString* dstring, usize pos, usize len);

/**
 * d_string_trim_left_char_in_place:
 * @dstring: The #DString to trim.
 * @c: The character to trim.
 *
 * Trims characters from the left of the #DString without reallocating memory.
 * The operation modifies the original #DString.
 * Returns: The trimmed #DString
 */
DString*	d_string_trim_left_by_char_in_place(DString* dstring, char c);

/**
 * d_string_trim_right_char_in_place:
 * @dstring: The #DString to trim.
 * @c: The character to trim.
 *
 * Trims characters from the right of the #DString without reallocating memory.
 * The operation modifies the original #DString.
 * Returns: The trimmed #DString
 */
DString*	d_string_trim_right_by_char_in_place(DString* dstring, char c);

/**
 * d_string_trim_left_predicate_in_place:
 * @dstring: The #DString to trim.
 * @fn: The predicate function to use for trimming.
 *
 * Trims characters from the left of the #DString based on the predicate function without reallocating memory.
 * The operation modifies the original #DString.
 * Returns: The trimmed #DString
 */
DString*	d_string_trim_left_by_predicate_in_place(DString* dstring, match fn);


/**
 * d_string_trim_right_predicate_in_place:
 * @dstring: The #DString to trim.
 * @fn: The predicate function to use for trimming.
 *
 * Trims characters from the right of the #DString based on the predicate function without reallocating memory.
 * The operation modifies the original #DString.
 * Returns: The trimmed #DString
 */
DString*	d_string_trim_right_by_prediacte_in_place(DString* dstring, match fn);

/**
 * d_string_trim_left_char_new:
 * @dstring: The #DString to trim.
 * @c: The character to trim.
 *
 * Trims characters from the left of the #DString and reallocates memory if necessary.
 * The operation creates a new #DString with the trimmed result.
 * Returns: The new trimmed #DString
 */
DString*	d_string_trim_left_by_char_new(DString* dstring, char c);

/**
 * d_string_trim_right_char_new:
 * @dstring: The #DString to trim.
 * @c: The character to trim.
 *
 * Trims characters from the right of the #DString and reallocates memory if necessary.
 * The operation creates a new #DString with the trimmed result.
 * Returns: The new trimmed #DString
 */
DString*	d_string_trim_right_by_char_new(DString* dstring, char c);

/**
 * d_string_trim_left_predicate_new:
 * @dstring: The #DString to trim.
 * @fn: The predicate function to use for trimming.
 *
 * Trims characters from the left of the #DString based on the predicate function and reallocates memory if necessary.
 * The operation creates a new #DString with the trimmed result.
 * Returns: The new trimmed #DString
 */
DString*	d_string_trim_left_by_predicate_new(DString* dstring, match fn);

/**
 * d_string_trim_right_predicate_new:
 * @dstring: The #DString to trim.
 * @fn: The predicate function to use for trimming.
 *
 * Trims characters from the right of the #DString based on the predicate function and reallocates memory if necessary.
 * The operation creates a new #DString with the trimmed result.
 * Returns: The new trimmed #DString
 */
DString*	d_string_trim_right_by_prediacte_new(DString* dstring, match fn);

/**
 * d_string_as_d_array:
 * @dstring: The #DString to convert.
 *
 * Converts the #DString to a dynamic array representation.
 * Returns: The new #DArray representation of the string.
 */
DArray* 	d_string_as_d_array(DString* dstring);
#endif