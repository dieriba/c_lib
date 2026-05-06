#include "d_de_queue.h"
#include "raw_ring_buffer.h"
#include <stdlib.h>
struct DDeQueue
{
    RawRingBuffer raw_ring_buffer;
};

static DDeQueue *new_d_de_queue()
{
    return malloc(sizeof(DDeQueue));
}

DResult d_de_queue_new(DDeQueue **d_de_queue, usize capacity, usize elem_size)
{
    if (d_de_queue == NULL)
        return D_ERR_INVALID_ARG;
    else if ((*d_de_queue = new_d_de_queue()) == NULL)
        return D_ERR_ALLOC;
    DResult op_result = raw_ring_buffer_default_init(&(*d_de_queue)->raw_ring_buffer, capacity, elem_size);
    if (op_result != D_OK)
    {
        free(*d_de_queue);
        *d_de_queue = NULL;
    }
    return op_result;
}

DResult d_de_queue_push_front(DDeQueue *d_de_queue, const void *elem)
{
    if (d_de_queue == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_push_front(&d_de_queue->raw_ring_buffer, elem);
}

DResult d_de_queue_pop_front(DDeQueue *d_de_queue, void *out_elem)
{
    if (d_de_queue == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_pop_front(&d_de_queue->raw_ring_buffer, out_elem);
}

DResult d_de_queue_push_back(DDeQueue *d_de_queue, const void *elem)
{
    if (d_de_queue == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_push_back(&d_de_queue->raw_ring_buffer, elem);
}

DResult d_de_queue_pop_back(DDeQueue *d_de_queue, void *out_elem)
{
    if (d_de_queue == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_pop_back(&d_de_queue->raw_ring_buffer, out_elem);
}

DResult d_de_queue_get_size(const DDeQueue *d_de_queue, usize *size)
{
    if (d_de_queue == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_get_size(&d_de_queue->raw_ring_buffer, size);
}

DResult d_de_queue_get_capacity(const DDeQueue *d_de_queue, usize *capacity)
{
    if (d_de_queue == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_get_capacity(&d_de_queue->raw_ring_buffer, capacity);
}

DResult d_de_queue_is_empty(const DDeQueue *d_de_queue, bool *is_empty)
{
    if (d_de_queue == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_is_empty(&d_de_queue->raw_ring_buffer, is_empty);
}

void d_de_queue_destroy(DDeQueue **d_de_queue)
{
    if (d_de_queue == NULL || *d_de_queue == NULL)
        return;
    DDeQueue *de_queue = *d_de_queue;
    raw_ring_buffer_free(&de_queue->raw_ring_buffer);
    free(de_queue);
    *d_de_queue = NULL;
}