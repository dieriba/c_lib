#include "d_queue.h"
#include "container.h"
#include <string.h>
#include "raw_ring_buffer.h"

ASSERT_FIRST_FIELD(DQueue, raw_ring_buffer);

DResult d_queue_init(DQueue *d_queue, usize capacity, usize elem_size, DestroyElemFn free_fn, CopyElemFn copy_fn)
{
    return raw_ring_buffer_default_init((RawRingBuffer *)d_queue, capacity, elem_size, free_fn, copy_fn);
}

DResult d_queue_copy(DQueue *dst, const DQueue *src)
{
    return raw_ring_buffer_copy((RawRingBuffer *)dst, (const RawRingBuffer *)src);
}

DResult d_queue_push(DQueue *d_queue, const void *elem)
{
    return raw_ring_buffer_push_back((RawRingBuffer *)d_queue, elem);
}

DResult d_queue_pop(DQueue *d_queue, void *out_elem)
{
    return raw_ring_buffer_pop_front((RawRingBuffer *)d_queue, out_elem);
}


bool d_queue_is_empty(const DQueue *d_queue)
{
    return raw_ring_buffer_is_empty((const RawRingBuffer *)d_queue);
}

usize d_queue_get_size(const DQueue *d_queue)
{
    return raw_ring_buffer_get_size((const RawRingBuffer *)d_queue);
}

usize d_queue_get_capacity(const DQueue *d_queue)
{
    return raw_ring_buffer_get_capacity((const RawRingBuffer *)d_queue);
}

void d_queue_destroy(DQueue *d_queue)
{
    if (d_queue == NULL)
        return;
    raw_ring_buffer_free((RawRingBuffer *)d_queue);
    memset(d_queue, 0, sizeof(DQueue));
}