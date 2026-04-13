#include "d_types.h"

#ifndef __CONTAINER__H
#define __CONTAINER__H
#define DEFAULT_CAPACITY 0x10
Result container_increase_capacity_if_needed(intptr_t **data, usize *capacity, usize nb_elem, usize elem_size, usize nb_elem_to_copy, bool null_terminated);
#endif