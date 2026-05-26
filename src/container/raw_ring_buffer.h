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
    DestroyElemFn free_fn;
    CopyElemFn copy_fn;
} RawRingBuffer;

DResult raw_ring_buffer_init(RawRingBuffer *raw_ring_buffer, usize head, usize tail, usize capacity, usize elem_size, DestroyElemFn free_fn, CopyElemFn copy_fn);
DResult raw_ring_buffer_default_init(RawRingBuffer *raw_ring_buffer, usize capacity, usize elem_size, DestroyElemFn free_fn, CopyElemFn copy_fn);
DResult raw_ring_buffer_copy(RawRingBuffer *dst, const RawRingBuffer *src);
DResult raw_ring_buffer_push_front(RawRingBuffer *raw_ring_buffer, const void *elem);
DResult raw_ring_buffer_pop_front(RawRingBuffer *raw_ring_buffer, void *out_elem);

DResult raw_ring_buffer_push_back(RawRingBuffer *raw_ring_buffer, const void *elem);
DResult raw_ring_buffer_pop_back(RawRingBuffer *raw_ring_buffer, void *out_elem);

bool raw_ring_buffer_is_empty(const RawRingBuffer *raw_ring_buffer);

void raw_ring_buffer_clear(RawRingBuffer *raw_ring_buffer);
void raw_ring_buffer_free(RawRingBuffer *raw_ring_buffer);

#define RAW_RING_BUFFER_GETTER(FIELD, FIELD_TYPE)                                                    \
    static inline FIELD_TYPE raw_ring_buffer_get_##FIELD(const RawRingBuffer *raw_ring_buffer)       \
    {                                                                                                \
        return raw_ring_buffer->FIELD;                                                               \
    }

RAW_RING_BUFFER_GETTER(size, usize)
RAW_RING_BUFFER_GETTER(capacity, usize)

#endif