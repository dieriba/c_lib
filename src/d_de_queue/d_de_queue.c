#include "d_de_queue.h"
#include "container.h"
#include <string.h>

ASSERT_FIRST_FIELD(DDeQueue, raw_ring_buffer);

DResult d_de_queue_init(DDeQueue *d_de_queue, usize capacity, usize elem_size, DestroyElemFn free_fn, CopyElemFn copy_fn)
{
    return raw_ring_buffer_default_init((RawRingBuffer *)d_de_queue, capacity, elem_size, free_fn, copy_fn);
}

DResult d_de_queue_copy(DDeQueue *dst, const DDeQueue *src)
{
    return raw_ring_buffer_copy((RawRingBuffer *)dst, (const RawRingBuffer *)src);
}

DResult d_de_queue_push_front(DDeQueue *d_de_queue, const void *elem)
{
    return raw_ring_buffer_push_front((RawRingBuffer *)d_de_queue, elem);
}

DResult d_de_queue_pop_front(DDeQueue *d_de_queue, void *out_elem)
{
    return raw_ring_buffer_pop_front((RawRingBuffer *)d_de_queue, out_elem);
}

DResult d_de_queue_push_back(DDeQueue *d_de_queue, const void *elem)
{
    return raw_ring_buffer_push_back((RawRingBuffer *)d_de_queue, elem);
}

DResult d_de_queue_pop_back(DDeQueue *d_de_queue, void *out_elem)
{
    return raw_ring_buffer_pop_back((RawRingBuffer *)d_de_queue, out_elem);
}


bool d_de_queue_is_empty(const DDeQueue *d_de_queue)
{
    return raw_ring_buffer_is_empty((const RawRingBuffer *)d_de_queue);
}

usize d_de_queue_get_size(const DDeQueue *d_de_queue)
{
    return raw_ring_buffer_get_size((const RawRingBuffer *)d_de_queue);
}

usize d_de_queue_get_capacity(const DDeQueue *d_de_queue)
{
    return raw_ring_buffer_get_capacity((const RawRingBuffer *)d_de_queue);
}

void d_de_queue_destroy(DDeQueue *d_de_queue)
{
    raw_ring_buffer_free((RawRingBuffer *)d_de_queue);
}