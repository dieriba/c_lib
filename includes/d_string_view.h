#ifndef D_STRING_VIEW_H
#define D_STRING_VIEW_H

/**
 * @defgroup d_string_view String View
 * @{
 * @brief Non-owning, read-only window into a character buffer.
 *
 * ::DStringView is a lightweight `{data, size}` pair that refers to a
 * contiguous sequence of characters owned by someone else (a C string literal,
 * a ::DDynString, an input buffer, etc.). It never allocates and never frees.
 *
 * The data pointer is **not** guaranteed to be null-terminated unless it was
 * created from a null-terminated source. Always use `size` to bound reads.
 *
 * @note All search functions return @ref MAX_SIZE_T_VALUE (i.e. `SIZE_MAX`) to
 *       signal "not found". Compare the return value against that constant
 *       before using it as an index.
 *
 * @par Typical lifecycle
 * @code{.c}
 *   const char *src = "hello, world";
 *   DStringView view = d_string_view_from_c_string(src);
 *
 *   usize pos = d_string_view_find_first_matching_char_from_start(view, ',');
 *   if (pos != MAX_SIZE_T_VALUE) {
 *       DStringView before = d_string_view_subview(view, 0, pos); // "hello"
 *   }
 * @endcode
 */
#include "d_error.h"
#include "d_types.h"
#include "d_dyn_string.h"
#include "d_dyn_array.h"

/**
 * @brief Non-owning view into a character buffer.
 *
 * Both fields are considered immutable through the view — do not cast away
 * the @c const qualifier.
 */
typedef struct DStringView
{
    const char *data; /**< Pointer to the first character of the viewed range.     */
    usize size;       /**< Number of characters in the view (may be 0).            */
} DStringView;

/**
 * @brief Predicate callback used by find-by-predicate and trim-by-predicate functions.
 *
 * @param c A character from the viewed string.
 * @return @c true if the character satisfies the predicate, @c false otherwise.
 */
typedef bool (*match)(char c);

/**
 * @brief Constructs a ::DStringView from a string literal at compile time.
 *
 * Uses @c sizeof to determine the length — correct **only** for string literals
 * where the compiler knows the full size. Do NOT pass a @c char * variable;
 * use @ref d_string_view_from_c_string for runtime pointers.
 *
 * @code{.c}
 *   DStringView v = D_STRING_VIEW_FROM_LITERAL("hello"); // size == 5
 *   usize i = d_string_view_find_first_char_in_set_from_start(view,
 *                 D_STRING_VIEW_FROM_LITERAL(",;"));
 * @endcode
 */
#define D_STRING_VIEW_FROM_LITERAL(str) ((DStringView){.data = (str), .size = sizeof(str) - 1})

/* -----------------------------------------------------------------------
 * Construction
 * -------------------------------------------------------------------- */

/**
 * @brief Constructs a ::DStringView from a data pointer and an explicit length.
 *
 * The view does not need to be null-terminated.
 *
 * @param data Pointer to the first character. May be NULL only when @p size is 0.
 * @param size Number of characters in the view.
 * @return A ::DStringView covering `[data, data + size)`.
 *
 * @code{.c}
 *   char buf[] = {'h', 'i'};
 *   DStringView v = d_string_view_from_parts(buf, 2);
 * @endcode
 */
DStringView d_string_view_from_parts(const char *data, usize size);

/**
 * @brief Constructs a ::DStringView from a null-terminated C string.
 *
 * The size is set to `strlen(c_str)`. The null terminator is not included
 * in the view.
 *
 * @param c_str Null-terminated source string. If NULL, an empty view is returned.
 * @return A ::DStringView over the string content, or an empty view if @p c_str is NULL.
 *
 * @code{.c}
 *   DStringView v = d_string_view_from_c_string("hello");
 * @endcode
 */
DStringView d_string_view_from_c_string(const char *c_str);

/**
 * @brief Constructs a ::DStringView over the content of a ::DDynString.
 *
 * @warning The view is invalidated by any mutation of @p dstring.
 *
 * @param dstring Source dynamic string. Must not be NULL.
 * @return A ::DStringView over the dynamic string's character data.
 *
 * @code{.c}
 *   DStringView v = d_string_view_from_dyn_string(&my_dstring);
 * @endcode
 */
DStringView d_string_view_from_dyn_string(const DDynString *dstring);

/* -----------------------------------------------------------------------
 * Basic queries
 * -------------------------------------------------------------------- */

/**
 * @brief Returns @c true if the view has zero characters.
 *
 * @param view The view to test.
 * @return @c true when `view.size == 0`.
 */
bool d_string_view_is_empty(DStringView view);

/**
 * @brief Returns the number of characters in the view.
 *
 * @param view The view to query.
 * @return `view.size`.
 */
usize d_string_view_len(DStringView view);

/**
 * @brief Returns the data pointer of the view.
 *
 * The returned pointer is never NULL — empty views point to a static @c ""
 * string. Do not rely on null-termination unless the view was constructed
 * from a null-terminated source.
 *
 * @param view The view to query.
 * @return `view.data` (always non-NULL).
 */
const char *d_string_view_data(DStringView view);

/**
 * @brief Copies the character at position @p index into @p out.
 *
 * @param  view  The view to read from.
 * @param  index Zero-based character index.
 * @param out   Pointer to a @c char that receives the character. Must not be NULL.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if @p out is NULL or @p index >= view.size.
 *
 * @code{.c}
 *   DStringView v = d_string_view_from_c_string("hello");
 *   char c;
 *   d_string_view_get_char_at(v, 1, &c); // c == 'e'
 * @endcode
 */
DResult d_string_view_get_char_at(DStringView view, usize index, char *out);

/* -----------------------------------------------------------------------
 * Sub-views and sub-strings
 * -------------------------------------------------------------------- */

/**
 * @brief Returns a sub-view starting at @p pos covering @p size characters.
 *
 * No allocation occurs. If @p pos is at or past the view length an empty view
 * is returned. If `pos + size` exceeds the view length only the remaining
 * characters are included.
 *
 * @param view The source view.
 * @param pos  Starting index within @p view.
 * @param size Maximum number of characters to include.
 * @return A new ::DStringView representing the sub-range, or an empty view
 *         if @p pos >= @p view.size.
 *
 * @code{.c}
 *   DStringView full = d_string_view_from_c_string("hello, world");
 *   DStringView sub  = d_string_view_subview(full, 7, 5); // "world"
 * @endcode
 */
DStringView d_string_view_subview(DStringView view, usize pos, usize size);

/**
 * @brief Allocates and returns a heap copy of the sub-range `[pos, pos+size)`.
 *
 * The returned string is null-terminated. The caller is responsible for
 * freeing it with @c free().
 *
 * @param view The source view.
 * @param pos  Starting index within @p view.
 * @param size Maximum number of characters to copy.
 * @return A newly allocated, null-terminated string, or NULL on allocation failure
 *         or if @p pos is out of range.
 */
char *d_string_view_substr(DStringView view, usize pos, usize size);

/* -----------------------------------------------------------------------
 * Comparison
 * -------------------------------------------------------------------- */

/**
 * @brief Returns @c true if two views have identical size and byte content.
 *
 * @param view1 First view.
 * @param view2 Second view.
 * @return @c true if sizes match and all bytes are identical, @c false otherwise.
 *
 * @code{.c}
 *   DStringView a = d_string_view_from_c_string("hello");
 *   DStringView b = d_string_view_from_c_string("hello");
 *   assert(d_string_view_compare(a, b));
 * @endcode
 */
bool d_string_view_compare(DStringView view1, DStringView view2);

/**
 * @brief Returns @c true if the view's content equals @p c_str.
 *
 * Equivalent to constructing a view from @p c_str and calling
 * @ref d_string_view_compare. If @p c_str is NULL it is treated as an
 * empty string.
 *
 * @param view  The view to compare.
 * @param c_str Null-terminated reference string.
 * @return @c true if equal, @c false otherwise.
 *
 * @code{.c}
 *   DStringView v = d_string_view_from_c_string("world");
 *   if (d_string_view_compare_against_c_string(v, "world"))
 *       puts("match");
 * @endcode
 */
bool d_string_view_compare_against_c_string(DStringView view, const char *c_str);

/* -----------------------------------------------------------------------
 * Prefix / suffix tests
 * -------------------------------------------------------------------- */

/** @brief Returns @c true if @p view starts with character @p c. */
bool d_string_view_starts_with_char(DStringView view, char c);
/** @brief Returns @c true if @p view ends with character @p c. */
bool d_string_view_ends_with_char(DStringView view, char c);
/** @brief Returns @c true if @p view starts with the characters of @p prefix. */
bool d_string_view_starts_with_view(DStringView view, DStringView prefix);
/** @brief Returns @c true if @p view ends with the characters of @p suffix. */
bool d_string_view_ends_with_view(DStringView view, DStringView suffix);
/** @brief Returns @c true if @p view starts with the null-terminated string @p prefix. */
bool d_string_view_starts_with_c_string(DStringView view, const char *prefix);
/** @brief Returns @c true if @p view ends with the null-terminated string @p suffix. */
bool d_string_view_ends_with_c_string(DStringView view, const char *suffix);

/* -----------------------------------------------------------------------
 * Single-character search
 * -------------------------------------------------------------------- */

/**
 * @brief Finds the first occurrence of @p c at or after index @p pos.
 *
 * @param view The view to search.
 * @param c    Character to find.
 * @param pos  Starting index for the search.
 * @return Zero-based index of the first match, or @ref MAX_SIZE_T_VALUE if not found.
 *
 * @code{.c}
 *   DStringView v = d_string_view_from_c_string("hello");
 *   usize i = d_string_view_find_first_matching_char_from_index(v, 'l', 0); // 2
 * @endcode
 */
usize d_string_view_find_first_matching_char_from_index(DStringView view, char c, usize pos);

/**
 * @brief Finds the first occurrence of @p c starting from the beginning.
 *
 * Equivalent to `d_string_view_find_first_matching_char_from_index(view, c, 0)`.
 *
 * @return Zero-based index, or @ref MAX_SIZE_T_VALUE if not found.
 */
usize d_string_view_find_first_matching_char_from_start(DStringView view, char c);

/**
 * @brief Finds the first character that is **not** @p c at or after @p pos.
 *
 * @return Zero-based index, or @ref MAX_SIZE_T_VALUE if every character from @p pos onward equals @p c.
 */
usize d_string_view_find_first_not_matching_char_from_index(DStringView view, char c, usize pos);

/**
 * @brief Finds the first character that is **not** @p c starting from the beginning.
 *
 * @return Zero-based index, or @ref MAX_SIZE_T_VALUE if all characters equal @p c.
 */
usize d_string_view_find_first_not_matching_char_from_start(DStringView view, char c);

/**
 * @brief Finds the last occurrence of @p c at or before index @p pos.
 *
 * @return Zero-based index, or @ref MAX_SIZE_T_VALUE if not found.
 */
usize d_string_view_find_last_matching_char_from_index(DStringView view, char c, usize pos);

/**
 * @brief Finds the last occurrence of @p c searching from the end.
 *
 * @return Zero-based index, or @ref MAX_SIZE_T_VALUE if not found.
 */
usize d_string_view_find_last_matching_char_from_end(DStringView view, char c);

/**
 * @brief Finds the last character that is **not** @p c at or before @p pos.
 *
 * @return Zero-based index, or @ref MAX_SIZE_T_VALUE if not found.
 */
usize d_string_view_find_last_not_matching_char_from_index(DStringView view, char c, usize pos);

/**
 * @brief Finds the last character that is **not** @p c searching from the end.
 *
 * @return Zero-based index, or @ref MAX_SIZE_T_VALUE if not found.
 */
usize d_string_view_find_last_not_matching_char_from_end(DStringView view, char c);

/* -----------------------------------------------------------------------
 * Sub-view / sub-string search
 * -------------------------------------------------------------------- */

/**
 * @brief Finds the first occurrence of @p to_find within @p view at or after @p pos.
 *
 * @param view    The view to search.
 * @param to_find Sub-view to locate.
 * @param pos     Starting index for the search.
 * @return Index of the first matching position, or @ref MAX_SIZE_T_VALUE if not found.
 *
 * @code{.c}
 *   DStringView v = d_string_view_from_c_string("abcabc");
 *   DStringView needle = d_string_view_from_c_string("bc");
 *   usize i = d_string_view_find_first_matching_view_from_index(v, needle, 0); // 1
 * @endcode
 */
usize d_string_view_find_first_matching_view_from_index(DStringView view, DStringView to_find, usize pos);

/** @brief Finds the first occurrence of @p to_find starting from the beginning.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_first_matching_view_from_start(DStringView view, DStringView to_find);

/** @brief Finds the last occurrence of @p to_find at or before @p pos.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_last_matching_view_from_index(DStringView view, DStringView to_find, usize pos);

/** @brief Finds the last occurrence of @p to_find searching from the end.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_last_matching_view_from_end(DStringView view, DStringView to_find);

/* -----------------------------------------------------------------------
 * Character-set search
 * -------------------------------------------------------------------- */

/**
 * @brief Finds the first character in @p set at or after @p pos.
 *
 * @param view The view to search.
 * @param set  View over the set of characters to look for.
 *             Use @ref D_STRING_VIEW_FROM_LITERAL for string-literal sets.
 * @param pos  Starting index.
 * @return Index of the first matching character, or @ref MAX_SIZE_T_VALUE if not found.
 *
 * @code{.c}
 *   DStringView v = d_string_view_from_c_string("hello, world");
 *   usize i = d_string_view_find_first_char_in_set_from_start(v,
 *                 D_STRING_VIEW_FROM_LITERAL(" ,")); // 5 (the comma)
 * @endcode
 */
usize d_string_view_find_first_char_in_set_from_index(DStringView view, DStringView set, usize pos);

/** @brief Finds the first character in @p set from the beginning.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_first_char_in_set_from_start(DStringView view, DStringView set);

/** @brief Finds the first character **not** in @p set at or after @p pos.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_first_char_not_in_set_from_index(DStringView view, DStringView set, usize pos);

/** @brief Finds the first character **not** in @p set from the beginning.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_first_char_not_in_set_from_start(DStringView view, DStringView set);

/** @brief Finds the last character in @p set at or before @p pos.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_last_char_in_set_from_index(DStringView view, DStringView set, usize pos);

/** @brief Finds the last character in @p set searching from the end.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_last_char_in_set_from_end(DStringView view, DStringView set);

/** @brief Finds the last character **not** in @p set at or before @p pos.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_last_char_not_in_set_from_index(DStringView view, DStringView set, usize pos);

/** @brief Finds the last character **not** in @p set from the end.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_last_char_not_in_set_from_end(DStringView view, DStringView set);

/* -----------------------------------------------------------------------
 * Predicate search
 * -------------------------------------------------------------------- */

/**
 * @brief Finds the first character satisfying @p fn at or after @p pos.
 *
 * @param view The view to search.
 * @param fn   Predicate function. Must not be NULL.
 * @param pos  Starting index.
 * @return Index of the first character where `fn(c)` is @c true,
 *         or @ref MAX_SIZE_T_VALUE if not found.
 *
 * @code{.c}
 *   #include <ctype.h>
 *   DStringView v = d_string_view_from_c_string("  hello");
 *   usize i = d_string_view_find_first_matching_predicate_from_start(v, isalpha); // 2
 * @endcode
 */
usize d_string_view_find_first_matching_predicate_from_index(DStringView view, match fn, usize pos);

/** @brief Finds the first character satisfying @p fn from the beginning.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_first_matching_predicate_from_start(DStringView view, match fn);

/** @brief Finds the first character **not** satisfying @p fn at or after @p pos.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_first_not_matching_predicate_from_index(DStringView view, match fn, usize pos);

/** @brief Finds the first character **not** satisfying @p fn from the beginning.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_first_not_matching_predicate_from_start(DStringView view, match fn);

/** @brief Finds the last character satisfying @p fn at or before @p pos.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_last_matching_predicate_from_index(DStringView view, match fn, usize pos);

/** @brief Finds the last character satisfying @p fn from the end.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_last_matching_predicate_from_end(DStringView view, match fn);

/** @brief Finds the last character **not** satisfying @p fn at or before @p pos.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_last_not_matching_predicate_from_index(DStringView view, match fn, usize pos);

/** @brief Finds the last character **not** satisfying @p fn from the end.
 *  @return Index, or @ref MAX_SIZE_T_VALUE if not found. */
usize d_string_view_find_last_not_matching_predicate_from_end(DStringView view, match fn);

/* -----------------------------------------------------------------------
 * Trim (returns a narrower sub-view, no allocation)
 * -------------------------------------------------------------------- */

/**
 * @brief Returns a view with leading occurrences of @p c removed.
 *
 * @code{.c}
 *   DStringView v = d_string_view_from_c_string("   hello");
 *   DStringView t = d_string_view_trim_left_by_char(v, ' '); // "hello"
 * @endcode
 */
DStringView d_string_view_trim_left_by_char(DStringView view, char c);

/**
 * @brief Returns a view with trailing occurrences of @p c removed.
 *
 * @code{.c}
 *   DStringView v = d_string_view_from_c_string("hello   ");
 *   DStringView t = d_string_view_trim_right_by_char(v, ' '); // "hello"
 * @endcode
 */
DStringView d_string_view_trim_right_by_char(DStringView view, char c);

/**
 * @brief Returns a view with leading characters satisfying @p fn removed.
 *
 * @code{.c}
 *   #include <ctype.h>
 *   DStringView v = d_string_view_from_c_string("  42abc");
 *   DStringView t = d_string_view_trim_left_by_predicate(v, isspace); // "42abc"
 * @endcode
 */
DStringView d_string_view_trim_left_by_predicate(DStringView view, match fn);

/**
 * @brief Returns a view with trailing characters satisfying @p fn removed.
 *
 * @code{.c}
 *   #include <ctype.h>
 *   DStringView v = d_string_view_from_c_string("hello123");
 *   DStringView t = d_string_view_trim_right_by_predicate(v, isdigit); // "hello"
 * @endcode
 */
DStringView d_string_view_trim_right_by_predicate(DStringView view, match fn);

/* -----------------------------------------------------------------------
 * Split
 * -------------------------------------------------------------------- */

/**
 * @brief Splits @p view on every occurrence of @p c and stores the resulting
 *        substrings in @p new_dyn_array as heap-allocated @c char * strings.
 *
 * Each element in the output array is a @c char * (stored in a pointer array).
 * The strings are individually heap-allocated; the array owns them and frees
 * them when @ref d_dyn_array_destroy is called. Consecutive delimiters are
 * skipped — no empty tokens are emitted.
 *
 * To iterate over the tokens:
 * @code{.c}
 *   char *token;
 *   d_dyn_array_get_elem_at(&parts, i, &token);
 * @endcode
 *
 * @param new_dyn_array Uninitialized ::DDynArray that will hold @c char * elements.
 * @param  view          The view to split.
 * @param  opts          Buffer option flags for the internal array (pass @c 0).
 * @param  c             Delimiter character.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if @p new_dyn_array is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DDynArray parts;
 *   DStringView csv = d_string_view_from_c_string("a,b,c");
 *   d_string_view_split_by_char_owned(&parts, csv, 0, ',');
 *   // parts contains three heap-allocated char* tokens: "a", "b", "c"
 *   d_dyn_array_destroy(&parts); // frees each token
 * @endcode
 */
DResult d_string_view_split_by_char_owned(DDynArray *new_dyn_array, DStringView view, BufferOpts opts, char c);

/**
 * @brief Splits @p view on any character present in @p set and stores the
 *        resulting substrings as heap-allocated @c char * strings.
 *
 * Owned variant: each token is a freshly-allocated @c char *; the array takes
 * ownership and frees every token when @ref d_dyn_array_destroy is called.
 * Consecutive delimiters are skipped — no empty tokens are emitted.
 *
 * @param new_dyn_array Uninitialized ::DDynArray that will hold @c char * elements.
 * @param  view          The view to split.
 * @param  opts          Buffer option flags (pass @c 0).
 * @param  set           View over the set of delimiter characters.
 *                       Use @ref D_STRING_VIEW_FROM_LITERAL for string-literal sets.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if @p new_dyn_array is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DDynArray tokens;
 *   DStringView v = d_string_view_from_c_string("one two,three");
 *   d_string_view_split_by_char_of_str_owned(&tokens, v, 0, D_STRING_VIEW_FROM_LITERAL(" ,"));
 *   // tokens: heap-allocated "one", "two", "three"
 *   d_dyn_array_destroy(&tokens); // frees each token
 * @endcode
 */
DResult d_string_view_split_by_char_of_str_owned(DDynArray *new_dyn_array, DStringView view, BufferOpts opts, DStringView set);

/**
 * @brief Splits @p view on every occurrence of @p c and stores the resulting
 *        sub-views (non-owning) in @p new_dyn_array.
 *
 * Each element in the output array is a ::DStringView that points directly
 * into @p view's underlying buffer — no allocation per token. The views become
 * invalid if the source buffer is freed or mutated. The array itself must still
 * be destroyed by the caller.
 *
 * @param new_dyn_array Uninitialized ::DDynArray that will hold ::DStringView elements.
 * @param  view          The view to split.
 * @param  opts          Buffer option flags (pass @c 0).
 * @param  c             Delimiter character.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if @p new_dyn_array is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   const char *src = "a,b,c";
 *   DDynArray parts;
 *   d_string_view_split_by_char_not_owned(&parts, d_string_view_from_c_string(src), 0, ',');
 *   DStringView tok;
 *   d_dyn_array_get_elem_at(&parts, 0, &tok); // tok.data == src + 0, tok.size == 1
 *   d_dyn_array_destroy(&parts);
 * @endcode
 */
DResult d_string_view_split_by_char_not_owned(DDynArray *new_dyn_array, DStringView view, BufferOpts opts, char c);

/**
 * @brief Splits @p view on any character in @p set and stores the resulting
 *        sub-views (non-owning) in @p new_dyn_array.
 *
 * Each element in the output array is a ::DStringView that points directly
 * into @p view's underlying buffer — no allocation per token. The views become
 * invalid if the source buffer is freed or mutated. The array itself must still
 * be destroyed by the caller. Consecutive delimiters are skipped.
 *
 * @param new_dyn_array Uninitialized ::DDynArray that will hold ::DStringView elements.
 * @param  view          The view to split.
 * @param  opts          Buffer option flags (pass @c 0).
 * @param  set           View over the set of delimiter characters.
 *                       Use @ref D_STRING_VIEW_FROM_LITERAL for string-literal sets.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if @p new_dyn_array is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   const char *src = "one two,three";
 *   DDynArray parts;
 *   d_string_view_split_by_char_of_str_not_owned(&parts, d_string_view_from_c_string(src),
 *                                                 0, D_STRING_VIEW_FROM_LITERAL(" ,"));
 *   // parts[0].data == src, parts[1].data == src+4, parts[2].data == src+8
 *   d_dyn_array_destroy(&parts);
 * @endcode
 */
DResult d_string_view_split_by_char_of_str_not_owned(DDynArray *new_dyn_array, DStringView view, BufferOpts opts, DStringView set);

/* -----------------------------------------------------------------------
 * Conversion to owning string
 * -------------------------------------------------------------------- */

/**
 * @brief Initialises a ::DDynString from the characters of @p view.
 *
 * The new dynamic string owns a heap copy of the view's content. The view
 * is not modified.
 *
 * @param new_dyn_string Dynamic string to initialise. Must not be NULL.
 * @param  view           Source view to copy from.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if @p new_dyn_string is NULL,
 *         ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DStringView v = d_string_view_from_c_string("hello");
 *   DDynString s;
 *   d_dyn_string_init_from_string_view(&s, v);
 *   d_dyn_string_destroy(&s);
 * @endcode
 */
DResult d_dyn_string_init_from_string_view(DDynString *new_dyn_string, DStringView view);

void d_string_view_dbg_print(DStringView *d_string_view);

/** @} */ /* end of d_string_view group */

#endif
