#ifndef D_STACK_H
#define D_STACK_H

/**
 * @defgroup d_stack Stack
 * @{
 * @brief Auto-growing LIFO stack backed by a ring buffer.
 *
 * ::DStack provides last-in, first-out (LIFO) semantics over a heap-allocated
 * ring buffer. The buffer grows automatically (doubling, always a power of two)
 * when capacity is exceeded, so callers do not need to pre-size precisely.
 *
 * Elements are stored by value: every push copies @c elem_size bytes in, and
 * every pop copies @c elem_size bytes out.
 *
 * @par Typical lifecycle
 * @code{.c}
 *   DStack stack;
 *   d_stack_init(&stack, 8, sizeof(int));
 *
 *   int x = 10, y = 20;
 *   d_stack_push(&stack, &x);
 *   d_stack_push(&stack, &y);
 *
 *   int top;
 *   d_stack_pop(&stack, &top); // top == 20 (last pushed)
 *
 *   d_stack_destroy(&stack);
 * @endcode
 */

#include <stdbool.h>
#include "d_error.h"
#include "d_types.h"
#include "raw_ring_buffer.h"

/**
 * @brief Auto-growing LIFO stack.
 *
 * Do not access internal fields directly; use the @ref d_stack API.
 */
typedef struct DStack
{
    RawRingBuffer raw_ring_buffer; /**< Internal ring buffer (implementation detail). */
} DStack;

/**
 * @brief Initialises a ::DStack with an initial capacity hint.
 *
 * The capacity is rounded up to the next power of two internally. The stack
 * grows automatically when the capacity is exceeded, so @p capacity is only
 * a hint to reduce early reallocations.
 *
 * @param d_stack   Stack to initialise. Must not be NULL.
 * @param capacity  Initial element capacity (hint). May be 0 for a default.
 * @param elem_size Size in bytes of each element. Must be > 0.
 * @return ::D_OK, ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DStack stack;
 *   d_stack_init(&stack, 16, sizeof(double));
 *   d_stack_destroy(&stack);
 * @endcode
 */
DResult d_stack_init(DStack *d_stack, usize capacity, usize elem_size, DestroyElemFn free_fn, CopyElemFn copy_fn);

/**
 * @brief Pushes a copy of @p elem onto the top of the stack.
 *
 * Grows the internal buffer automatically if needed.
 *
 * @param d_stack Stack to push onto. Must not be NULL.
 * @param elem    Pointer to the element to copy. Must not be NULL.
 * @return ::D_OK, ::D_ERR_ALLOC if the growth reallocation fails,
 *         ::D_ERR_OVERFLOW if the capacity would exceed the @c usize limit.
 *
 * @code{.c}
 *   int val = 42;
 *   d_stack_push(&stack, &val);
 * @endcode
 */
DResult d_stack_push(DStack *d_stack, const void *elem);

/**
 * @brief Removes the top element and copies it into @p out_elem.
 *
 * @param d_stack  Stack to pop from. Must not be NULL.
 * @param out_elem Buffer that receives the popped element. Must not be NULL
 *                 and must be at least @c elem_size bytes.
 * @return ::D_OK, ::D_ERR_EMPTY if the stack is empty.
 *
 * @code{.c}
 *   int top;
 *   if (d_stack_pop(&stack, &top) == D_OK)
 *       printf("popped: %d\n", top);
 * @endcode
 */
DResult d_stack_pop(DStack *d_stack, void *out_elem);


DResult d_stack_copy(DStack *dst, const DStack *src);

/**
 * @brief Returns the current number of elements in the stack.
 *
 * @param d_stack Stack to query. Must not be NULL.
 */
usize d_stack_get_size(const DStack *d_stack);

/**
 * @brief Returns the current allocated capacity in elements.
 *
 * @param d_stack Stack to query. Must not be NULL.
 */
usize d_stack_get_capacity(const DStack *d_stack);

/**
 * @brief Returns true if the stack has no elements.
 *
 * @param d_stack Stack to query. Must not be NULL.
 */
bool d_stack_is_empty(const DStack *d_stack);

/**
 * @brief Frees all memory held by the stack and zeroes the struct.
 *
 * Safe to call on a NULL pointer (no-op). After this call @p d_stack must
 * not be used without re-initialisation.
 *
 * @param d_stack Stack to destroy. May be NULL.
 */
void d_stack_destroy(DStack *d_stack);

/** @} */ /* end of d_stack group */

#endif
