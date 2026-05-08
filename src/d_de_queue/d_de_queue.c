#include "d_de_queue.h"
#include "raw_ring_buffer.h"
#include <stdlib.h>
#include <string.h>

DResult d_de_queue_init(DDeQueue *d_de_queue, usize capacity, usize elem_size)
{
    return raw_ring_buffer_default_init((RawRingBuffer *)d_de_queue, capacity, elem_size);
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

DResult d_de_queue_get_size(const DDeQueue *d_de_queue, usize *size)
{
    return raw_ring_buffer_get_size((RawRingBuffer *)d_de_queue, size);
}

DResult d_de_queue_get_capacity(const DDeQueue *d_de_queue, usize *capacity)
{
    return raw_ring_buffer_get_capacity((RawRingBuffer *)d_de_queue, capacity);
}

DResult d_de_queue_is_empty(const DDeQueue *d_de_queue, bool *is_empty)
{
    return raw_ring_buffer_is_empty((RawRingBuffer *)d_de_queue, is_empty);
}

void d_de_queue_destroy(DDeQueue *d_de_queue)
{
    if (d_de_queue == NULL)
        return;
    raw_ring_buffer_free((RawRingBuffer *)d_de_queue);
    memset(d_de_queue, 0, sizeof(DDeQueue));
}