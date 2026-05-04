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

DDeQueue *d_de_queue_new(usize capacity, usize elem_size)
{
    DDeQueue *d_de_queue = new_d_de_queue();
    if (d_de_queue == NULL)
        return NULL;
    if (raw_ring_buffer_default_init((RawRingBuffer *)d_de_queue, capacity, elem_size) != D_OK)
        return NULL;
    return d_de_queue;
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
    return raw_ring_buffer_pop_back((DDeQueue *)d_de_queue, out_elem);
}

DResult d_de_queue_is_empty(DDeQueue *d_de_queue, bool *is_empty)
{
    return raw_ring_buffer_is_empty((RawRingBuffer *)d_de_queue, is_empty);
}
