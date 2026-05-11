#ifndef D_RESULT_H
#define D_RESULT_H

/**
 * @defgroup d_result Result Codes
 * @{
 * @brief Unified return and comparison result codes for the entire library.
 *
 * Provides ::DResult (returned by every fallible API call) and
 * ::DCompareResult (returned by comparison operations), as well as the
 * @ref d_result_print_result_as_str helper for diagnostic output.
 *
 * Include this header directly or via any module header to access these types.
 */

/**
 * @brief Unified return code for all library operations.
 *
 * Every function that can fail returns a @c DResult. Always compare the
 * return value to @c D_OK before reading any output parameter — the output
 * is undefined when the call fails.
 *
 * @code{.c}
 *   DDynArray arr;
 *   DResult r = d_dyn_array_init(&arr, sizeof(int), 8, NULL, 0);
 *   if (r != D_OK) {
 *       fprintf(stderr, "init failed: %s\n", d_result_print_result_as_str(r));
 *       return -1;
 *   }
 * @endcode
 */
typedef enum DResult
{
    D_OK = 0,          /**< Operation completed successfully.                                 */
    D_ERR_ALLOC,       /**< Memory allocation failed.                                         */
    D_ERR_INVALID_ARG, /**< One or more arguments are NULL or out-of-range.                   */
    D_ERR_NOT_EXIST,   /**< The target resource (key, slot, …) does not exist in the container.*/
    D_ERR_OVERFLOW,    /**< An arithmetic or buffer overflow would occur.                     */
    D_ERR_EMPTY        /**< The container is empty and the operation requires elements.       */
} DResult;

/**
 * @brief Result type returned by comparison operations.
 *
 * Returned by APIs such as @ref d_dyn_string_compare to distinguish
 * equality, inequality, and comparison failure.
 */
typedef enum
{
    D_COMPARE_ERROR = -2,   /**< Comparison failed (e.g. a NULL input was supplied). */
    D_COMPARE_EQUAL = 0,    /**< The two values are equal.                            */
    D_COMPARE_NOT_EQUAL = 1 /**< The two values differ.                               */
} DCompareResult;

/**
 * @brief Converts a @ref DResult code to a human-readable C string.
 *
 * Useful for diagnostic output. The returned pointer refers to a string
 * literal — do not modify or free it.
 *
 * @param  result The result code to convert.
 * @return A pointer to a static, null-terminated description string.
 *         Unknown codes return @c "UNKNOWN_RESULT"; never returns NULL.
 *
 * @code{.c}
 *   DResult r = some_operation();
 *   if (r != D_OK)
 *       printf("error: %s\n", d_result_print_result_as_str(r));
 * @endcode
 */
static inline char *d_result_print_result_as_str(DResult result)
{
    switch (result)
    {
    case D_OK:
        return "OK";
    case D_ERR_ALLOC:
        return "ERR_ALLOC";
    case D_ERR_INVALID_ARG:
        return "ERR_INVALID_ARG";
    case D_ERR_NOT_EXIST:
        return "ERR_NOT_EXIST";
    case D_ERR_OVERFLOW:
        return "ERR_OVERFLOW";
    case D_ERR_EMPTY:
        return "ERR_EMPTY";
    default:
        return "UNKNOWN_RESULT";
    }
}

/** @} */ /* end of d_result group */

#endif