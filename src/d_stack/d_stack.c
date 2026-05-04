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

DStack *d_stack_new(usize capacity, usize elem_size)
{
    DStack *d_stack = new_d_stack();
    if (d_stack == NULL)
        return NULL;
    if (raw_ring_buffer_default_init((RawRingBuffer *)d_stack, capacity, elem_size) != D_OK)
        return NULL;
    return d_stack;
}

DResult d_stack_push(DStack *d_stack, const void *elem)
{
    return raw_ring_buffer_push_back((RawRingBuffer *)d_stack, elem);
}

DResult d_stack_pop(DStack *d_stack, void *out_elem)
{
    return raw_ring_buffer_pop_back((DStack *)d_stack, out_elem);
}

DResult d_stack_is_empty(DStack *d_stack, bool *is_empty)
{
    return raw_ring_buffer_is_empty((RawRingBuffer *)d_stack, is_empty);
}
