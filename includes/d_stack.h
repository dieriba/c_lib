#ifndef D_STACK_H
#define D_STACK_H
#include <stdbool.h>
#include "d_types.h"
#include "raw_ring_buffer.h"
typedef struct DStack DStack;

DStack *d_stack_new(usize capacity, usize elem_size);
DResult d_stack_push(DStack *d_stack, const void *elem);
DResult d_stack_pop(DStack *d_stack, void *out_elem);
DResult d_stack_is_empty(DStack *, bool *is_empty);

#define D_STACK_GETTER(FIELD, FIELD_TYPE)                                               \
    static inline DResult d_stack_get_##FIELD(const DStack *d_stack, FIELD_TYPE *FIELD) \
    {                                                                                   \
        return raw_ring_buffer_get_##FIELD((RawRingBuffer *)d_stack, FIELD);            \
    }

D_STACK_GETTER(size, usize)
D_STACK_GETTER(capacity, usize)
#endif