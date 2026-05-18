#ifndef D_ERROR_H
#define D_ERROR_H

#include "d_types.h"
#include <stddef.h>
#include <stdio.h>

typedef enum DResult
{
    D_OK = 0,          /**< Operation completed successfully.                                  */
    D_ERR_ALLOC,       /**< Memory allocation failed.                                          */
    D_ERR_INVALID_ARG, /**< One or more arguments are NULL or out-of-range.                    */
    D_ERR_NOT_EXIST,   /**< The target resource (key, slot, …) does not exist in the container.*/
    D_ERR_OVERFLOW,    /**< An arithmetic or buffer overflow would occur.                      */
    D_ERR_EMPTY        /**< The container is empty and the operation requires elements.        */
} DResult;

/**
 * @brief Result type returned by comparison operations.
 */
typedef enum
{
    D_COMPARE_ERROR = -2,   /**< Comparison failed (e.g. a NULL input was supplied). */
    D_COMPARE_EQUAL = 0,    /**< The two values are equal.                            */
    D_COMPARE_NOT_EQUAL = 1 /**< The two values are not equal.                        */
} DCompareResult;

/**
 * @brief Converts a ::DResult code to a human-readable C string.
 *
 * Returns a pointer to a string literal — do not modify or free it.
 * Unknown codes return @c "UNKNOWN_RESULT"; never returns NULL.
 */
static inline const char *d_error_print_result_as_str(DResult result)
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

typedef struct DError
{
    DResult code;    /**< Underlying result code.                               */
    const char *msg; /**< Optional custom message. NULL falls back to the code. */
} DError;

/** @brief Returns a success ::DError. */
static inline DError d_error_ok(void) { return (DError){D_OK, NULL}; }

/** @brief Constructs a ::DError with @p code and a custom @p msg. */
static inline DError d_error_new(DResult code, const char *msg) { return (DError){code, msg}; }

/** @brief Returns true when @p err represents success. */
static inline bool d_error_is_ok(DError err) { return err.code == D_OK; }

/**
 * @brief Returns the message for @p err.
 *
 * Returns @c err.msg when set, otherwise falls back to
 * @ref d_error_print_result_as_str. Never returns NULL.
 */
const char *d_error_message(DError err);

/** @brief Prints the message of @p err to stderr followed by a newline. */
static inline void d_error_print(DError err)
{
    if (err.msg)
        fprintf(stderr, "%s\n", d_error_message(err));
    else
        printf("No Error\n");
}

/** @} */

#endif
