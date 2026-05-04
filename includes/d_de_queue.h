#ifndef D_DE_QUEUE_H
#define D_DE_QUEUE_H
#include <stdbool.h>
#include "d_types.h"

typedef struct DDeQueue DDeQueue;

DResult d_de_queue_new(DDeQueue **d_de_queue, usize capacity, usize elem_size);

DResult d_de_queue_push_front(DDeQueue *d_de_queue, const void *elem);
DResult d_de_queue_pop_front(DDeQueue *d_de_queue, void *out_elem);

DResult d_de_queue_push_back(DDeQueue *d_de_queue, const void *elem);
DResult d_de_queue_pop_back(DDeQueue *d_de_queue, void *out_elem);
DResult d_de_queue_get_size(DDeQueue* d_de_queue, usize *size);
DResult d_de_queue_get_capacity(DDeQueue* d_de_queue, usize *capacity);
DResult d_de_queue_is_empty(DDeQueue *d_de_queue, bool *is_empty);

#endif