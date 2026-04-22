#include <stdlib.h>
#include "container.h"

#define GROWTH_POLICY 2
#define GROWTH_LIMIT (MAX_SIZE_T_VALUE / GROWTH_POLICY)

#define calcul_total_len(nb_elem, elem_size) nb_elem *elem_size

void *container_increase_capacity_if_needed(void *data, usize *capacity, usize nb_elem, usize elem_size, usize nb_elem_to_copy, bool null_terminated)
{
    usize total_len = calcul_total_len(nb_elem, elem_size);
    usize total_len_copy = calcul_total_len(nb_elem_to_copy, elem_size);
    usize extra_null_elem = null_terminated ? 1 : 0;
    if (MAX_SIZE_T_VALUE - total_len < total_len_copy)
        return NULL;

    usize nb_elem_needed = nb_elem + nb_elem_to_copy;
    if ((nb_elem_needed + extra_null_elem) <= *capacity)
        return data;

    usize total_needed_len = total_len + total_len_copy;
    if (total_needed_len > GROWTH_LIMIT)
        return NULL;

    usize new_capacity = total_needed_len * GROWTH_POLICY;

    void *tmp = realloc(data, new_capacity + extra_null_elem);
    if (tmp == NULL)
        return NULL;
    *capacity = nb_elem_needed;
    return tmp;
}