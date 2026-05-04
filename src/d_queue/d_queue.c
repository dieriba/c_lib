#include "d_queue.h"
#include "raw_ring_buffer.h"

struct DQueue
{
    RawRingBuffer raw_ring_buffer;
};

static DQueue *new_d_queue()
{
    return malloc(sizeof(DQueue));
}

DResult d_queue_new(DQueue **d_queue, usize capacity, usize elem_size)
{
    if (d_queue == NULL)
        return D_ERR_INVALID_ARG;
    else if ((*d_queue = new_d_queue()) == NULL)
        return D_ERR_ALLOC;
    return raw_ring_buffer_default_init((RawRingBuffer *)*d_queue, capacity, elem_size);
}

DResult d_queue_push(DQueue *d_queue, const void *elem)
{
    return raw_ring_buffer_push_back((RawRingBuffer *)d_queue, elem);
}

DResult d_queue_pop(DQueue *d_queue, void *out_elem)
{
    return raw_ring_buffer_pop_front((DQueue *)d_queue, out_elem);
}

DResult d_queue_get_size(DQueue *d_queue, usize *size)
{
    return raw_ring_buffer_get_size((DQueue *)d_queue, size);
}

DResult d_queue_get_capacity(DQueue *d_queue, usize *capacity)
{
    return raw_ring_buffer_get_capacity((DQueue *)d_queue, capacity);
}

DResult d_queue_is_empty(DQueue *d_queue, bool *is_empty)
{
    return raw_ring_buffer_is_empty((RawRingBuffer *)d_queue, is_empty);
}
