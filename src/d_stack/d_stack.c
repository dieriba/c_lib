#include "d_stack.h"
#include "container.h"
#include <string.h>

ASSERT_FIRST_FIELD(DStack, raw_ring_buffer);

DResult d_stack_init(DStack *d_stack, usize capacity, usize elem_size, DestroyElemFn free_fn, CopyElemFn copy_fn)
{
    if (d_stack == NULL)
        return D_ERR_INVALID_ARG;
    return raw_ring_buffer_default_init((RawRingBuffer *)d_stack, capacity, elem_size, free_fn, copy_fn);
}

DResult d_stack_copy(DStack *dst, const DStack *src)
{
    return raw_ring_buffer_copy((RawRingBuffer *)dst, (const RawRingBuffer *)src);
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

void d_stack_destroy(DStack *d_stack)
{
    if (d_stack == NULL)
        return;
    raw_ring_buffer_free((RawRingBuffer *)d_stack);
    memset(d_stack, 0, sizeof(DStack));
}