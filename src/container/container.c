#include <stdlib.h>
#include <string.h>

#include "container.h"
#include "d_math.h"
#include "d_general_lib.h"

#define GROWTH_POLICY 2

#define container_nb_available_elem_slot(container) \
    ((container)->capacity - (container)->len)
#define container_nb_occupied_elem_slot(container) \
    ((container)->len)

#define container_elt_len(container, i) container->elem_size *i
#define container_elt_pos(container, i) (char *)container->data + (container_elt_len(container, i))

static bool container_has_zero_sentinel(const Container *container)
{
    return d_bits_8_check_bit_set(container->opts, CNT_OPT_ZERO_SENTINEL);
}

static bool container_compute_new_alloc_size(const Container *container,
                                             usize capacity, usize *alloc_size)
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
    if (container->data == NULL || !container_has_zero_sentinel(container))
        return;
    memset(container_elt_pos(container, container->len), 0, container_elt_len(container, 1));
}

static Container *container_new_raw()
{
    return malloc(sizeof(Container));
}

Result container_init(Container *container, usize elem_size, usize capacity, DBits8 opts)
{
    if (container == NULL || elem_size == 0)
        return ERROR;
    container->len = 0;
    container->capacity = capacity == 0 ? DEFAULT_CAPACITY : capacity;
    container->elem_size = elem_size;
    container->opts = opts;
    container->data = NULL;

    usize alloc_size;
    if (!container_compute_new_alloc_size(container, container->capacity, &alloc_size))
        return ERROR;
    if ((container->data = malloc(alloc_size)) == NULL)
        return ERROR;
    container_write_sentinel(container);
    return OK;
}

Result container_init_with_data(Container *container, usize elem_size, void *data, usize len, DBits8 opts)
{
    usize capacity = len < DEFAULT_CAPACITY ? DEFAULT_CAPACITY : len;
    if (container_init(container, elem_size, capacity, opts) == ERROR)
        return ERROR;
    if (container_append_data(container, data, len) == ERROR)
        return ERROR;
    return OK;
}

Container *container_new(usize elem_size, usize capacity, DBits8 opts)
{
    Container *container;

    container = container_new_raw();
    if (container == NULL)
        return NULL;
    if (container_init(container, elem_size, capacity, opts) == ERROR)
    {
        free(container);
        return NULL;
    }
    return container;
}

Container *container_new_from(const Container *src)
{
    Container *new_container;

    if (src == NULL)
        return NULL;

    new_container = container_new_raw();
    if (new_container == NULL)
        return NULL;
    if (container_init_with_data(new_container, src->elem_size, src->data, src->len, src->opts) == ERROR)
    {
        container_destroy(&new_container);
        return NULL;
    }
    return new_container;
}

void container_free(Container *container)
{
    if (container == NULL)
        return;
    free(container->data);
    container->data = NULL;
    container->len = 0;
    container->capacity = 0;
}

void container_destroy(Container **container)
{
    if (container == NULL || *container == NULL)
        return;
    container_free(*container);
    free(*container);
    *container = NULL;
}

Result container_increase_capacity_if_needed(Container *container, usize nb_elem_to_copy)
{
    usize new_capacity;
    usize alloc_size;
    void *tmp;

    if (container == NULL)
        return ERROR;
    if (nb_elem_to_copy <= container_nb_available_elem_slot(container))
        return OK;
    if (d_overflow_check_add_usize(container->capacity, nb_elem_to_copy, &new_capacity))
        return ERROR;
    if (d_overflow_check_mul_usize(new_capacity, GROWTH_POLICY, &new_capacity))
        return ERROR;
    if (!container_compute_new_alloc_size(container, new_capacity, &alloc_size))
        return ERROR;
    tmp = realloc(container->data, alloc_size);
    if (tmp == NULL)
        return ERROR;
    container->data = tmp;
    container->capacity = new_capacity;
    container_write_sentinel(container);
    return OK;
}

void *container_get_data(Container *container)
{
    if (container == NULL)
        return NULL;
    return container->data;
}

void *container_get_elem_at(Container *container, usize index)
{
    if (container == NULL || index >= container->len)
        return NULL;
    return container_elt_pos(container, index);
}

void *container_get_last_elem(Container *container)
{
    if (container == NULL || container->len == 0)
        return NULL;
    return container_elt_pos(container, container->len - 1);
}

void *container_get_first_elem(Container *container)
{
    if (container == NULL || container->len == 0)
        return NULL;
    return container_elt_pos(container, 0);
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

Result container_insert_data(Container *dst, usize dst_pos, const void *data, usize len)
{
    usize dst_len;
    usize byte_pos;
    usize byte_len;
    usize bytes_to_move;
    char *base;

    if (dst == NULL || data == NULL)
        return ERROR;
    else if (len == 0)
        return OK;

    dst_len = container_nb_occupied_elem_slot(dst);
    if (dst_pos > dst_len)
        return ERROR;
    if (container_increase_capacity_if_needed(dst, len) == ERROR)
        return ERROR;
    byte_pos = dst_pos * dst->elem_size;
    byte_len = len * dst->elem_size;
    base = dst->data;
    if (dst_pos != dst_len)
    {
        bytes_to_move = (dst_len - dst_pos) * dst->elem_size;
        memmove(base + byte_pos + byte_len, base + byte_pos, bytes_to_move);
    }
    memmove(base + byte_pos, data, byte_len);
    dst->len += len;
    container_write_sentinel(dst);
    return OK;
}

Result container_resize(Container *container, usize new_len, void *filler)
{
    if (container == NULL)
        return ERROR;
    usize cnt_len = container->len;
    if (new_len == cnt_len)
        return OK;
    usize to_cpy = new_len > cnt_len ? new_len - cnt_len : 0;
    if (to_cpy != 0)
    {
        if (container_increase_capacity_if_needed(container, to_cpy) == ERROR)
            return ERROR;
        memfill(container_elt_pos(container, cnt_len), filler, container->elem_size, to_cpy);
    }
    container->len = new_len;
    container_write_sentinel(container);
    return OK;
}

Result container_remove(Container *container, usize pos, usize len_to_remove)
{
    if (container == NULL || pos >= container->len)
        return ERROR;

    usize cnt_len = container->len;
    usize total_len;

    if (d_overflow_check_add_usize(pos, len_to_remove, &total_len))
        return ERROR;

    len_to_remove = total_len > cnt_len ? cnt_len - pos : len_to_remove;
    usize to_cpy = cnt_len - total_len;
    if (to_cpy != 0)
        memmove(container_elt_pos(container, pos), container_elt_pos(container, total_len), container_elt_len(container, to_cpy));
    container->len -= len_to_remove;
    container_write_sentinel(container);
    return OK;
}

Result container_replace_data(Container *container, usize pos, const void *data, usize len)
{
    if (container == NULL || data == NULL || pos >= container->len)
        return ERROR;
    if (len == 0)
        return OK;

    usize total_len;
    if (d_overflow_check_add_usize(pos, len, &total_len))
        return ERROR;
    usize extra_elem_to_allocate = total_len > container->capacity ? total_len - container->capacity : 0;
    if (extra_elem_to_allocate != 0 && container_increase_capacity_if_needed(container, extra_elem_to_allocate) == ERROR)
        return ERROR;
    memmove(container_elt_pos(container, pos), data, container_elt_len(container, len));
    if (extra_elem_to_allocate != 0)
    {
        container->len = total_len;
        container_write_sentinel(container);
    }
    return OK;
}

Result container_replace_data_trunc(Container *container, usize pos, const void *data, usize len)
{
    if (container == NULL)
        return ERROR;
    if (pos >= container->len)
        return ERROR;
    if (len != 0)
    {
        if (container_replace_data(container, pos, data, len) == ERROR)
            return ERROR;
    }

    container->len = pos + len;
    container_write_sentinel(container);
    return OK;
}

Result container_replace_container_trunc(Container *dst, const Container *src)
{
    if (dst == NULL || src == NULL || container_replace_data_trunc(dst, 0, src->data, src->len) == ERROR)
        return ERROR;
    return OK;
}

Result container_append_container(Container *dst, const Container *src)
{
    if (dst == NULL || src == NULL)
        return ERROR;
    return container_insert_data(dst, dst->len, src->data, src->len);
}

Result container_append_data(Container *dst, const void *data, usize len)
{
    if (dst == NULL)
        return ERROR;
    return container_insert_data(dst, dst->len, data, len);
}

Result container_prepend(Container *dst, const Container *src)
{
    if (dst == NULL || src == NULL)
        return ERROR;
    return container_insert_data(dst, 0, src->data, src->len);
}

Result container_push(Container *container, const void *elem)
{
    if (container == NULL || elem == NULL)
        return ERROR;
    if (container_increase_capacity_if_needed(container, 1) == ERROR)
        return ERROR;
    memmove(container_elt_pos(container, container->len), elem, container_elt_len(container, 1));
    container->len++;
    container_write_sentinel(container);
    return OK;
}

Result container_pop(Container *container, void *out_elem)
{
    if (container == NULL)
        return ERROR;
    if (container->len == 0)
        return OK;
    container->len--;
    if (out_elem != NULL)
        memcpy(out_elem, container_elt_pos(container, container->len), container_elt_len(container, 1));
    container_write_sentinel(container);
    return OK;
}

Result container_swap_remove(Container *container, usize index, void *out_elem)
{
    if (container == NULL || index >= container->len)
        return ERROR;
    if (out_elem != NULL)
        memcpy(out_elem, container_elt_pos(container, index), container_elt_len(container, 1));
    container->len--;
    if (index != container->len)
        memcpy(container_elt_pos(container, index), container_elt_pos(container, container->len), container_elt_len(container, 1));
    container_write_sentinel(container);
    return OK;
}

void container_clear(Container *container)
{
    if (container == NULL)
        return;
    container->len = 0;
    container_write_sentinel(container);
}
