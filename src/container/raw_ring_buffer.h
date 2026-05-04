#ifndef RAW_RING_BUFFER_H
#define RAW_RING_BUFFER_H
#include "d_types.h"
#include "d_bits.h"

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
DResult raw_ring_buffer_init_with_data(RawRingBuffer *raw_ring_buffer, usize head, usize tail, usize elem_size, void *data, usize len);
RawRingBuffer *raw_ring_buffer_new_from(const RawRingBuffer *src);
void raw_ring_buffer_free(RawRingBuffer *raw_ring_buffer);

DResult raw_ring_buffer_push_front(RawRingBuffer *raw_ring_buffer, const void *elem);
DResult raw_ring_buffer_pop_front(RawRingBuffer *raw_ring_buffer, void *out_elem);

DResult raw_ring_buffer_push_back(RawRingBuffer *raw_ring_buffer, const void *elem);
DResult raw_ring_buffer_pop_back(RawRingBuffer *raw_ring_buffer, void *out_elem);

void raw_ring_buffer_clear(RawRingBuffer *raw_ring_buffer);

#endif