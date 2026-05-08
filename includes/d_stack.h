#ifndef D_STACK_H
#define D_STACK_H
#include <stdbool.h>
#include "d_types.h"
#include "raw_ring_buffer.h"

typedef struct DStack
{
    RawRingBuffer raw_ring_buffer;
} DStack;

DResult d_stack_init(DStack *d_stack, usize capacity, usize elem_size);
DResult d_stack_push(DStack *d_stack, const void *elem);
DResult d_stack_pop(DStack *d_stack, void *out_elem);
DResult d_stack_get_size(const DStack *d_stack, usize *size);
DResult d_stack_get_capacity(const DStack *d_stack, usize *capacity);
DResult d_stack_is_empty(const DStack *, bool *is_empty);
void d_stack_destroy(DStack *d_stack);
#endif