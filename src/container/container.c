#include <stdlib.h>
#include "container.h"

#define GROWTH_POLICY 2
#define GROWTH_LIMIT (MAX_SIZE_T_VALUE / GROWTH_POLICY)

#define calcul_total_len(nb_elem, elem_size) nb_elem *elem_size

Result container_increase_capacity_if_needed(Container *container, usize nb_elem_to_copy, bool zero_terminated)
{
    usize total_len = calcul_total_len(container->len, container->elem_size);
    usize total_len_copy = calcul_total_len(nb_elem_to_copy, container->elem_size);
    usize extra_zero_elem = zero_terminated ? 1 : 0;
    if (MAX_SIZE_T_VALUE - total_len < total_len_copy)
        return ERROR;

    usize nb_elem_needed = container->len + nb_elem_to_copy;
    if ((nb_elem_needed + extra_zero_elem) <= container->capacity)
        return OK;

    usize total_needed_len = total_len + total_len_copy;
    if (total_needed_len > GROWTH_LIMIT)
        return ERROR;

    usize new_capacity = total_needed_len * GROWTH_POLICY;

    void *tmp = realloc(container->data, new_capacity + extra_zero_elem);
    if (tmp == NULL)
        return ERROR;
    container->data = tmp;
    container->capacity = nb_elem_needed;
    return OK;
}