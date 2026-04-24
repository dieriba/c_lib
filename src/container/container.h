#ifndef __CONTAINER__H
#define __CONTAINER__H
#include "d_types.h"
#include "d_bits.h"
#define DEFAULT_CAPACITY 0x10

typedef struct Container
{
    void *data;
    usize len;
    usize capacity;
    usize elem_size;

} Container;
Result container_increase_capacity_if_needed(Container *container, usize nb_elem_to_copy, bool zero_terminated);
#endif