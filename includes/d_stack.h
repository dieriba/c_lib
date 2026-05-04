#ifndef D_STACK_H
#define D_STACK_H
#include <stdbool.h>
#include "d_types.h"
#include "raw_ring_buffer.h"
typedef struct DStack DStack;

DResult d_stack_new(DStack** d_stack, usize capacity, usize elem_size);
DResult d_stack_push(DStack *d_stack, const void *elem);
DResult d_stack_pop(DStack *d_stack, void *out_elem);
DResult d_stack_get_size(DStack *d_stack, usize *size);
DResult d_stack_get_capacity(DStack *d_stack, usize *capacity);
DResult d_stack_is_empty(DStack *, bool *is_empty);
#endif