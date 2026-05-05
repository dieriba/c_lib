#include "d_stack.h"
#include "raw_ring_buffer.h"
#include <stdlib.h>

struct DStack
{
    RawRingBuffer raw_ring_buffer;
};

static DStack *new_d_stack()
{
    return malloc(sizeof(DStack));
}

DResult d_stack_new(DStack** d_stack, usize capacity, usize elem_size)
{
    if (d_stack == NULL)
        return D_ERR_INVALID_ARG;
    else if ((*d_stack = new_d_stack()) == NULL)
        return D_ERR_ALLOC;
    return raw_ring_buffer_default_init((RawRingBuffer *)*d_stack, capacity, elem_size);
}

DResult d_stack_push(DStack *d_stack, const void *elem)
{
    return raw_ring_buffer_push_back((RawRingBuffer *)d_stack, elem);
}

DResult d_stack_pop(DStack *d_stack, void *out_elem)
{
    return raw_ring_buffer_pop_back((RawRingBuffer *)d_stack, out_elem);
}

DResult d_stack_get_size(const DStack *d_stack, usize *size)
{
    return raw_ring_buffer_get_size((RawRingBuffer *)d_stack, size);
}

DResult d_stack_get_capacity(const DStack *d_stack, usize *capacity)
{
    return raw_ring_buffer_get_capacity((RawRingBuffer *)d_stack, capacity);
}

DResult d_stack_is_empty(const DStack *d_stack, bool *is_empty)
{
    return raw_ring_buffer_is_empty((RawRingBuffer *)d_stack, is_empty);
}
