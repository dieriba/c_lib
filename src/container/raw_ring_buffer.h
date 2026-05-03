#ifndef RAW_RING_BUFFER_H
#define RAW_RING_BUFFER_H

#include "d_types.h"
#include "d_bits.h"

typedef enum e_buffer_opts
{
    RAW_RING_BUF_OPT_NONE = 0,
    RAW_RING_BUF_OPT_RESIZE_FULL = 1 << 0,
} BufferOpts;

typedef struct RawRingBuffer
{
    void *data;
    usize len;
    usize capacity;
    usize elem_size;
    usize front;
    usize back;
    DBits8 opts;
} RawRingBuffer;

#endif