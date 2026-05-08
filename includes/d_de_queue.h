#ifndef D_DE_QUEUE_H
#define D_DE_QUEUE_H
#include <stdbool.h>
#include "d_types.h"
#include "raw_ring_buffer.h"

typedef struct DDeQueue
{
    RawRingBuffer raw_ring_buffer;
} DDeQueue;

DResult d_de_queue_init(DDeQueue *d_de_queue, usize capacity, usize elem_size);

DResult d_de_queue_push_front(DDeQueue *d_de_queue, const void *elem);
DResult d_de_queue_pop_front(DDeQueue *d_de_queue, void *out_elem);

DResult d_de_queue_push_back(DDeQueue *d_de_queue, const void *elem);
DResult d_de_queue_pop_back(DDeQueue *d_de_queue, void *out_elem);
DResult d_de_queue_get_size(const DDeQueue *d_de_queue, usize *size);
DResult d_de_queue_get_capacity(const DDeQueue *d_de_queue, usize *capacity);
DResult d_de_queue_is_empty(const DDeQueue *d_de_queue, bool *is_empty);
void d_de_queue_destroy(DDeQueue *d_de_queue);
#endif