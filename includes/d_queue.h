#ifndef D_QUEUE_H
#define D_QUEUE_H
#include <stdbool.h>
#include "d_types.h"
#include "raw_ring_buffer.h"
typedef struct DQueue DQueue;

DQueue *d_queue_new(usize capacity, usize elem_size);
DResult d_queue_push(DQueue *d_queue, const void *elem);
DResult d_queue_pop(DQueue *d_queue, void *out_elem);
DResult d_queue_is_empty(DQueue *, bool *is_empty);


#define D_QUEUE_GETTER(FIELD, FIELD_TYPE)                                                    \
    static inline DResult d_queue_get_##FIELD(const DQueue *d_queue, FIELD_TYPE *FIELD) \
    {                                                                                           \
        return raw_ring_buffer_get_##FIELD((RawRingBuffer *)d_queue, FIELD);                      \
    }

D_QUEUE_GETTER(size, usize)
D_QUEUE_GETTER(capacity, usize)
#endif