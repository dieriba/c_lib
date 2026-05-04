#ifndef D_DE_QUEUE_H
#define D_DE_QUEUE_H
#include <stdbool.h>
#include "d_types.h"

typedef struct DDeQueue DDeQueue;

DDeQueue *d_de_queue_new(usize capacity, usize elem_size);

DResult d_de_queue_push_front(DDeQueue *d_de_queue, const void *elem);
DResult d_de_queue_pop_front(DDeQueue *d_de_queue, void *out_elem);

DResult d_de_queue_push_back(DDeQueue *d_de_queue, const void *elem);
DResult d_de_queue_pop_back(DDeQueue *d_de_queue, void *out_elem);

DResult d_de_queue_is_empty(DDeQueue *d_de_queue, bool *is_empty);

#define D_DE_QUEUE_GETTER(FIELD, FIELD_TYPE)                                                    \
    static inline DResult d_de_queue_get_##FIELD(const DDeQueue *d_de_queue, FIELD_TYPE *FIELD) \
    {                                                                                           \
        return d_de_queue_get_##FIELD((RawRingBuffer *)d_de_queue, FIELD);                      \
    }

D_DE_QUEUE_GETTER(size, usize)
D_DE_QUEUE_GETTER(capacity, usize)

#endif