#ifndef RAW_RING_BUFFER_H
#define RAW_RING_BUFFER_H
#include "d_error.h"
#include "d_types.h"
#include "d_bits.h"
#include <stdlib.h>

typedef struct RawRingBuffer
{
    void *data;
    usize size;
    usize capacity;
    usize elem_size;
    usize head;
    usize tail;
} RawRingBuffer;

DResult raw_ring_buffer_init(RawRingBuffer *raw_ring_buffer, usize head, usize tail, usize capacity, usize elem_size);
DResult raw_ring_buffer_default_init(RawRingBuffer *raw_ring_buffer, usize capacity, usize elem_size);
DResult raw_ring_buffer_init_with_data(RawRingBuffer *raw_ring_buffer, usize head, usize tail, usize elem_size, void *data, usize size);
DResult raw_ring_buffer_new_from(RawRingBuffer **new_raw_ring_buffer, const RawRingBuffer *src);

DResult raw_ring_buffer_push_front(RawRingBuffer *raw_ring_buffer, const void *elem);
DResult raw_ring_buffer_pop_front(RawRingBuffer *raw_ring_buffer, void *out_elem);

DResult raw_ring_buffer_push_back(RawRingBuffer *raw_ring_buffer, const void *elem);
DResult raw_ring_buffer_pop_back(RawRingBuffer *raw_ring_buffer, void *out_elem);

DResult raw_ring_buffer_is_empty(const RawRingBuffer *raw_ring_buffer, bool *is_empty);

void raw_ring_buffer_clear(RawRingBuffer *raw_ring_buffer);
void raw_ring_buffer_free(RawRingBuffer *raw_ring_buffer);

#define RAW_RING_BUFFER_GETTER(FIELD, FIELD_TYPE)                                                              \
    static inline DResult raw_ring_buffer_get_##FIELD(const RawRingBuffer *raw_ring_buffer, FIELD_TYPE *FIELD) \
    {                                                                                                          \
        if (raw_ring_buffer == NULL || FIELD == NULL)                                                          \
            return D_ERR_INVALID_ARG;                                                                          \
        *FIELD = raw_ring_buffer->FIELD;                                                                       \
        return D_OK;                                                                                           \
    }

RAW_RING_BUFFER_GETTER(size, usize)
RAW_RING_BUFFER_GETTER(capacity, usize)

#endif