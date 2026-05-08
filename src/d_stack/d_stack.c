#include "d_stack.h"
#include "raw_ring_buffer.h"
#include <stdlib.h>
#include <string.h>

DResult d_stack_init(DStack *d_stack, usize capacity, usize elem_size)
{
    if (d_stack == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_default_init((RawRingBuffer *)d_stack, capacity, elem_size);
}

DResult d_stack_push(DStack *d_stack, const void *elem)
{
    if (d_stack == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_push_back(&d_stack->raw_ring_buffer, elem);
}

DResult d_stack_pop(DStack *d_stack, void *out_elem)
{
    if (d_stack == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_pop_back(&d_stack->raw_ring_buffer, out_elem);
}

DResult d_stack_get_size(const DStack *d_stack, usize *size)
{
    if (d_stack == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_get_size(&d_stack->raw_ring_buffer, size);
}

DResult d_stack_get_capacity(const DStack *d_stack, usize *capacity)
{
    if (d_stack == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_get_capacity(&d_stack->raw_ring_buffer, capacity);
}

DResult d_stack_is_empty(const DStack *d_stack, bool *is_empty)
{
    if (d_stack == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_is_empty(&d_stack->raw_ring_buffer, is_empty);
}

void d_stack_destroy(DStack *d_stack)
{
    if (d_stack == NULL)
        return;
    raw_ring_buffer_free((RawRingBuffer *)d_stack);
    memset(d_stack, 0, sizeof(DStack));
}