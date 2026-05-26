#ifndef D_QUEUE_H
#define D_QUEUE_H

/**
 * @defgroup d_queue Queue
 * @{
 * @brief Auto-growing FIFO queue backed by a ring buffer.
 *
 * ::DQueue provides first-in, first-out (FIFO) semantics over a heap-allocated
 * ring buffer. The buffer grows automatically (doubling, always a power of two)
 * when capacity is exceeded, so callers do not need to pre-size precisely.
 *
 * Elements are stored by value: every push copies `elem_size` bytes into the
 * internal buffer, and every pop copies `elem_size` bytes out.
 *
 * @par Typical lifecycle
 * @code{.c}
 *   DQueue q;
 *   d_queue_init(&q, 32, sizeof(int));
 *
 *   int a = 1, b = 2;
 *   d_queue_push(&q, &a);
 *   d_queue_push(&q, &b);
 *
 *   int front;
 *   d_queue_pop(&q, &front); // front == 1 (first pushed)
 *
 *   d_queue_destroy(&q);
 * @endcode
 */

#include <stdbool.h>
#include "d_error.h"
#include "d_types.h"
#include "raw_ring_buffer.h"

/**
 * @brief Auto-growing FIFO queue.
 *
 * Do not access internal fields directly; use the @ref d_queue API.
 */
typedef struct DQueue
{
    RawRingBuffer raw_ring_buffer; /**< Internal ring buffer (implementation detail). */
} DQueue;

/**
 * @brief Initialises a ::DQueue with an initial capacity hint.
 *
 * The capacity is rounded up to the next power of two internally. The queue
 * grows automatically when the capacity is exceeded, so @p capacity is only
 * a hint to reduce early reallocations.
 *
 * @param d_queue   Queue to initialise. Must not be NULL.
 * @param  capacity  Initial element capacity (hint). May be 0 for a default.
 * @param  elem_size Size in bytes of each element. Must be > 0.
 * @return ::D_OK, ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DQueue q;
 *   d_queue_init(&q, 128, sizeof(unsigned));
 *   d_queue_destroy(&q);
 * @endcode
 */
DResult d_queue_init(DQueue *d_queue, usize capacity, usize elem_size, DestroyElemFn free_fn, CopyElemFn copy_fn);

/**
 * @brief Enqueues a copy of the element pointed to by @p elem at the back of the queue.
 *
 * Grows the internal buffer automatically if needed.
 *
 * @param d_queue Queue to push onto. Must not be NULL.
 * @param     elem    Pointer to the element to copy. Must not be NULL.
 * @return ::D_OK, ::D_ERR_ALLOC if the growth reallocation fails,
 *         ::D_ERR_OVERFLOW if the capacity would exceed the @c usize limit.
 *
 * @code{.c}
 *   int val = 99;
 *   d_queue_push(&q, &val);
 * @endcode
 */
DResult d_queue_push(DQueue *d_queue, const void *elem);

DResult d_queue_copy(DQueue *dst, const DQueue *src);

/**
 * @brief Dequeues the front element and copies it into @p out_elem.
 *
 * @param d_queue  Queue to pop from. Must not be NULL.
 * @param    out_elem Buffer that receives the dequeued element.
 *                         Must not be NULL and must be at least @c elem_size bytes.
 * @return ::D_OK, ::D_ERR_EMPTY if the queue is empty.
 *
 * @code{.c}
 *   int front;
 *   if (d_queue_pop(&q, &front) == D_OK)
 *       printf("dequeued: %d\n", front);
 * @endcode
 */
DResult d_queue_pop(DQueue *d_queue, void *out_elem);

/**
 * @brief Returns true if the queue has no elements.
 *
 * @param  d_queue Queue to query. Must not be NULL.
 */
bool d_queue_is_empty(const DQueue *d_queue);

/**
 * @brief Returns the current number of elements in the queue.
 *
 * @param  d_queue Queue to query. Must not be NULL.
 */
usize d_queue_get_size(const DQueue *d_queue);

/**
 * @brief Returns the current allocated capacity in elements.
 *
 * @param  d_queue Queue to query. Must not be NULL.
 */
usize d_queue_get_capacity(const DQueue *d_queue);

/**
 * @brief Frees all memory held by the queue and zeroes the struct.
 *
 * Safe to call on a NULL pointer (no-op). After this call @p d_queue must
 * not be used without re-initialisation.
 *
 * @param d_queue Queue to destroy. May be NULL.
 */
void d_queue_destroy(DQueue *d_queue);

/** @} */ /* end of d_queue group */

#endif
