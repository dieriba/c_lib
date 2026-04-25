#include <stdlib.h>
#include <string.h>
#include "container.h"
#include "d_math.h"
#define GROWTH_POLICY 2
#define GROWTH_LIMIT (MAX_SIZE_T_VALUE / GROWTH_POLICY)

#define container_nb_available_elem_slot(container) (container)->capacity - (container)->len
#define container_nb_occupied_elem_slot(container) (container)->len
#define container_total_len(container) (container->len * container->elem_size)

Container *container_new(usize elem_size, usize capacity, DBits8 opts)
{
    Container *new_container = malloc(sizeof(Container));

    if (new_container == NULL)
        return NULL;
    new_container->elem_size = elem_size;
    new_container->capacity = capacity;
    new_container->opts = opts;
    return new_container;
}

static bool container_has_zero_sentinel(const Container *container)
{
    return d_bits_8_check_bit_set(container->opts, CNT_OPT_ZERO_SENTINEL);
}

static bool container_compute_new_alloc_size(const Container *container, usize capacity, usize *alloc_size)
{
    usize extra;
    usize total_elems;

    extra = container_has_zero_sentinel(container) ? 1 : 0;
    if (d_overflow_check_add_usize(capacity, extra, &total_elems))
        return false;
    if (d_overflow_check_mul_usize(total_elems, container->elem_size, alloc_size))
        return false;
    return true;
}

static void container_write_sentinel(Container *container)
{
    if (!container->data || !container_has_zero_sentinel(container))
        return;
    unsigned char *end = (unsigned char *)container->data + container->len * container->elem_size;
    memset(end, 0, container->elem_size);
}

Result container_init(Container *container, usize elem_size, usize capacity, DBits8 opts)
{
    if (container == NULL || elem_size == 0)
        return ERROR;

    container->len = 0;
    container->capacity = capacity == 0 ? DEFAULT_CAPACITY : capacity;
    container->elem_size = elem_size;
    container->opts = opts;
    usize alloc_size;
    if (!container_compute_new_alloc_size(container, container->capacity, &alloc_size))
        return ERROR;
    container->data = malloc(alloc_size);
    if (container->data == NULL)
        return ERROR;

    container_write_sentinel(container);
    return OK;
}

Result container_increase_capacity_if_needed(Container *container, usize nb_elem_to_copy)
{
    if (container == NULL)
        return ERROR;

    if (nb_elem_to_copy <= container_nb_available_elem_slot(container))
        return OK;

    usize new_capacity;
    if (d_overflow_check_add_usize(container->capacity, nb_elem_to_copy, &new_capacity))
        return ERROR;
    if (d_overflow_check_mul_usize(new_capacity, GROWTH_POLICY, &new_capacity))
        return ERROR;
    usize alloc_size;
    if (!container_compute_new_alloc_size(container, new_capacity, &alloc_size))
        return ERROR;
    void *tmp = realloc(container->data, alloc_size);
    if (tmp == NULL)
        return ERROR;
    container->data = tmp;
    container->capacity = new_capacity;
    return OK;
}

Container *container_new_from(const Container *src)
{
    Container *new = container_new(src->elem_size, src->capacity, src->opts);

    if (new == NULL)
        return NULL;
    usize alloc_size;
    if (!container_compute_new_alloc_size(src, src->capacity, &alloc_size))
    {
        free(new);
        return NULL;
    }
    if ((new->data = malloc(alloc_size)) == NULL)
    {
        free(new);
        return NULL;
    }
    new->len = src->len;
    usize size_to_cpy = src->len * src->elem_size;
    memcpy(new->data, src->data, size_to_cpy);
    container_write_sentinel(new);
    return new;
}

Result container_insert(Container *dst, usize dst_pos, const void *data, usize len)
{
    if (dst == NULL || data == NULL || len == 0)
        return ERROR;
    usize dst_len = container_nb_occupied_elem_slot(dst);
    if (dst_pos > dst_len)
        return ERROR;
    if (container_increase_capacity_if_needed(dst, len) == ERROR)
        return ERROR;
    unsigned char *base = dst->data;
    usize byte_pos = dst_pos * dst->elem_size;
    usize byte_len = len * dst->elem_size;
    if (dst_pos != dst_len)
    {
        usize bytes_to_move = (dst_len - dst_pos) * dst->elem_size;
        memmove(base + byte_pos + byte_len, base + byte_pos, bytes_to_move);
    }
    memcpy(base + byte_pos, data, byte_len);
    dst->len += len;
    container_write_sentinel(dst);
    return OK;
}

Result container_append(Container *dst, Container *src)
{
    if (dst == NULL || src == NULL)
        return ERROR;
    return container_insert(dst, dst->len, src->data, src->len);
}

Result container_prepend(Container *dst, Container *src)
{
    if (dst == NULL || src == NULL)
        return ERROR;
    return container_insert(dst, 0, src->data, src->len);
}

void *container_get_data(Container *container)
{
    if (container == NULL)
        return NULL;
    return container->data;
}

usize container_get_len(const Container *container)
{
    if (container == NULL)
        return 0;
    return container->len;
}

usize container_get_capacity(const Container *container)
{
    if (container == NULL)
        return 0;
    return container->capacity;
}

usize container_get_elem_size(const Container *container)
{
    if (container == NULL)
        return 0;
    return container->elem_size;
}

DBits8 container_get_opts(const Container *container)
{
    if (container == NULL)
        return CNT_OPT_NONE;
    return container->opts;
}
