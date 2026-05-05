#ifndef D_QUEUE_H
#define D_QUEUE_H
#include <stdbool.h>
#include "d_types.h"
#include "raw_ring_buffer.h"
typedef struct DQueue DQueue;

DResult d_queue_new(DQueue **d_queue, usize capacity, usize elem_size);
DResult d_queue_push(DQueue *d_queue, const void *elem);
DResult d_queue_pop(DQueue *d_queue, void *out_elem);
DResult d_queue_is_empty(const DQueue *, bool *is_empty);

DResult d_queue_get_size(const DQueue *d_queue, usize *size);
DResult d_queue_get_capacity(const DQueue *d_queue, usize *capacity);
#endif