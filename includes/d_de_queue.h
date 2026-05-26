#ifndef D_DE_QUEUE_H
#define D_DE_QUEUE_H

/**
 * @defgroup d_de_queue Deque (Double-Ended Queue)
 * @{
 * @brief Auto-growing double-ended queue backed by a ring buffer.
 *
 * ::DDeQueue supports O(1) push and pop from **both** ends of the queue.
 * The buffer grows automatically (doubling, always a power of two) when
 * capacity is exceeded.
 *
 * Elements are stored by value: every push copies `elem_size` bytes into
 * the internal ring buffer, and every pop copies `elem_size` bytes out.
 *
 * @par Typical lifecycle
 * @code{.c}
 *   DDeQueue dq;
 *   d_de_queue_init(&dq, 16, sizeof(int));
 *
 *   int a = 1, b = 2, c = 3;
 *   d_de_queue_push_back(&dq, &a);  // [1]
 *   d_de_queue_push_back(&dq, &b);  // [1, 2]
 *   d_de_queue_push_front(&dq, &c); // [3, 1, 2]
 *
 *   int val;
 *   d_de_queue_pop_front(&dq, &val); // val == 3
 *   d_de_queue_pop_back(&dq, &val);  // val == 2
 *
 *   d_de_queue_destroy(&dq);
 * @endcode
 */

#include <stdbool.h>
#include "d_error.h"
#include "raw_ring_buffer.h"

/**
 * @brief Auto-growing double-ended queue.
 *
 * Do not access internal fields directly; use the @ref d_de_queue API.
 */
typedef struct DDeQueue
{
    RawRingBuffer raw_ring_buffer; /**< Internal ring buffer (implementation detail). */
} DDeQueue;

/**
 * @brief Initialises a ::DDeQueue with an initial capacity hint.
 *
 * The capacity is rounded up to the next power of two internally. The deque
 * grows automatically when the capacity is exceeded, so @p capacity is only
 * a hint to reduce early reallocations.
 *
 * @param d_de_queue Deque to initialise. Must not be NULL.
 * @param  capacity   Initial element capacity (hint). May be 0 for a default.
 * @param  elem_size  Size in bytes of each element. Must be > 0.
 * @return ::D_OK, ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DDeQueue dq;
 *   d_de_queue_init(&dq, 64, sizeof(float));
 *   d_de_queue_destroy(&dq);
 * @endcode
 */
DResult d_de_queue_init(DDeQueue *d_de_queue, usize capacity, usize elem_size, DestroyElemFn free_fn, CopyElemFn copy_fn);

DResult d_de_queue_copy(DDeQueue *dst, const DDeQueue *src);


/**
 * @brief Inserts a copy of @p elem at the **front** of the deque.
 *
 * Grows the internal buffer automatically if needed.
 *
 * @param d_de_queue Deque to push onto. Must not be NULL.
 * @param     elem       Pointer to the element to copy. Must not be NULL.
 * @return ::D_OK, ::D_ERR_ALLOC if the growth reallocation fails,
 *         ::D_ERR_OVERFLOW if the capacity would exceed the @c usize limit.
 *
 * @code{.c}
 *   int x = 5;
 *   d_de_queue_push_front(&dq, &x); // x becomes the new front element
 * @endcode
 */
DResult d_de_queue_push_front(DDeQueue *d_de_queue, const void *elem);

/**
 * @brief Removes the **front** element and copies it into @p out_elem.
 *
 * @param d_de_queue Deque to pop from. Must not be NULL.
 * @param    out_elem   Buffer that receives the removed element.
 *                           Must not be NULL and must be at least @c elem_size bytes.
 * @return ::D_OK, ::D_ERR_EMPTY if the deque is empty.
 *
 * @code{.c}
 *   int front;
 *   d_de_queue_pop_front(&dq, &front);
 * @endcode
 */
DResult d_de_queue_pop_front(DDeQueue *d_de_queue, void *out_elem);

/**
 * @brief Inserts a copy of @p elem at the **back** of the deque.
 *
 * Grows the internal buffer automatically if needed.
 *
 * @param d_de_queue Deque to push onto. Must not be NULL.
 * @param     elem       Pointer to the element to copy. Must not be NULL.
 * @return ::D_OK, ::D_ERR_ALLOC if the growth reallocation fails,
 *         ::D_ERR_OVERFLOW if the capacity would exceed the @c usize limit.
 *
 * @code{.c}
 *   int x = 7;
 *   d_de_queue_push_back(&dq, &x); // x becomes the new back element
 * @endcode
 */
DResult d_de_queue_push_back(DDeQueue *d_de_queue, const void *elem);

/**
 * @brief Removes the **back** element and copies it into @p out_elem.
 *
 * @param d_de_queue Deque to pop from. Must not be NULL.
 * @param    out_elem   Buffer that receives the removed element.
 *                           Must not be NULL and must be at least @c elem_size bytes.
 * @return ::D_OK, ::D_ERR_EMPTY if the deque is empty.
 *
 * @code{.c}
 *   int back;
 *   d_de_queue_pop_back(&dq, &back);
 * @endcode
 */
DResult d_de_queue_pop_back(DDeQueue *d_de_queue, void *out_elem);

/**
 * @brief Returns the current number of elements in the deque.
 *
 * @param  d_de_queue Deque to query. Must not be NULL.
 */
usize d_de_queue_get_size(const DDeQueue *d_de_queue);

/**
 * @brief Returns the current allocated capacity in elements.
 *
 * @param  d_de_queue Deque to query. Must not be NULL.
 */
usize d_de_queue_get_capacity(const DDeQueue *d_de_queue);

/**
 * @brief Returns true if the deque has no elements.
 *
 * @param  d_de_queue Deque to query. Must not be NULL.
 */
bool d_de_queue_is_empty(const DDeQueue *d_de_queue);

/**
 * @brief Frees all memory held by the deque and zeroes the struct.
 *
 * Safe to call on a NULL pointer (no-op). After this call @p d_de_queue must
 * not be used without re-initialisation.
 *
 * @param d_de_queue Deque to destroy. May be NULL.
 */
void d_de_queue_destroy(DDeQueue *d_de_queue);

/** @} */ /* end of d_de_queue group */

#endif
