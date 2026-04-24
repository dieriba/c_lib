#include <stdlib.h>
#include <string.h>
#include "container.h"

#define GROWTH_POLICY 2
#define GROWTH_LIMIT (MAX_SIZE_T_VALUE / GROWTH_POLICY)

#define container_calcul_total_len(nb_elem, elem_size) nb_elem *elem_size
#define container_nb_available_elem_slot(container) (container)->capacity - (container)->len
#define container_nb_occupied_elem_slot(container) (container)->len
Result container_increase_capacity_if_needed(Container *container, usize nb_elem_to_copy, bool zero_terminated)
{
    if (container == NULL)
        return ERROR;
    usize total_len = container_calcul_total_len(container->len, container->elem_size);
    usize total_len_copy = container_calcul_total_len(nb_elem_to_copy, container->elem_size);
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

static Container *container_new()
{
    return malloc(sizeof(Container));
}

Container *container_new_from(Container *src, bool zero_terminated)
{
    if (src == NULL)
        return NULL;

    Container *new = container_new();

    if (new == NULL)
        return NULL;

    if ((new->data = malloc(src->elem_size * src->capacity)) == NULL)
    {
        free(new);
        return NULL;
    }
    new->len = src->len;
    new->capacity = src->capacity;
    new->elem_size = src->elem_size;
    usize size_to_cpy = ((zero_terminated == true) + src->len) * src->elem_size;
    memcpy(new->data, src->data, size_to_cpy);
    return new;
}

Result container_insert(Container *dst, usize dst_pos, const void *data, usize len, bool zero_terminated)
{
    if (dst == NULL || data == NULL)
        return ERROR;
    usize dst_len = container_nb_occupied_elem_slot(dst);
    if (dst_pos > dst_len)
        return ERROR;
    if (container_increase_capacity_if_needed(dst, len, zero_terminated) == ERROR)
        return ERROR;
    if (dst_pos != dst_len)
    {
        usize dst_elem_to_mv = dst_len - dst_pos;
        memmove(dst->data + dst_pos + len, dst->data + dst_pos, dst_elem_to_mv);
    }
    memcpy(dst->data + dst_pos, data, len);
    dst->len += len;
    if (zero_terminated)
        memset(dst->data + dst->len, 0, dst->elem_size);
    return OK;
}

Result container_append(Container *dst, Container *src, bool zero_terminated)
{
    if (dst == NULL || src == NULL)
        return ERROR;
    return container_insert(dst, dst->len, src->data, src->len, zero_terminated);
}

Result container_prepend(Container *dst, Container *src, bool zero_terminated)
{
    if (dst == NULL || src == NULL)
        return ERROR;
    return container_insert(dst, 0, src->data, src->len, zero_terminated);
}

