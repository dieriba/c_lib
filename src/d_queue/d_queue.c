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
    DResult op_result = raw_ring_buffer_default_init(&(*d_queue)->raw_ring_buffer, capacity, elem_size);
    if (op_result != D_OK)
    {
        free(*d_queue);
        *d_queue = NULL;
    }
    return op_result;
}

DResult d_queue_push(DQueue *d_queue, const void *elem)
{
    if (d_queue == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_push_back(&d_queue->raw_ring_buffer, elem);
}

DResult d_queue_pop(DQueue *d_queue, void *out_elem)
{
    if (d_queue == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_pop_front(&d_queue->raw_ring_buffer, out_elem);
}

DResult d_queue_get_size(const DQueue *d_queue, usize *size)
{
    if (d_queue == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_get_size(&d_queue->raw_ring_buffer, size);
}

DResult d_queue_get_capacity(const DQueue *d_queue, usize *capacity)
{
    if (d_queue == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_get_capacity(&d_queue->raw_ring_buffer, capacity);
}

DResult d_queue_is_empty(const DQueue *d_queue, bool *is_empty)
{
    if (d_queue == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_is_empty(&d_queue->raw_ring_buffer, is_empty);
}

void d_queue_destroy(DQueue **d_queue)
{
    if (d_queue == NULL || *d_queue == NULL)
        return;
    DQueue *queue = *d_queue;
    raw_ring_buffer_free(&queue->raw_ring_buffer);
    free(queue);
    *d_queue = NULL;
}