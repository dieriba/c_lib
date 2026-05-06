#include <stdlib.h>
#include <string.h>
#include "container.h"
#include "raw_buffer.h"
#include "d_math.h"
#include "d_general_lib.h"

#define raw_buffer_nb_available_elem_slot_from_pos(raw_buffer, pos) \
    ((raw_buffer)->capacity - pos)
#define raw_buffer_nb_occupied_elem_slot(raw_buffer) \
    ((raw_buffer)->size)

#define raw_buffer_elt_size(raw_buffer, i) raw_buffer->elem_size *i
#define raw_buffer_elt_pos(raw_buffer, i) (char *)raw_buffer->data + (raw_buffer_elt_size(raw_buffer, i))

static bool raw_buffer_has_zero_sentinel(const RawBuffer *raw_buffer)
{
    return d_bits_8_check_bits_set(raw_buffer->opts, RAW_BUF_OPT_ZERO_SENTINEL);
}

static bool raw_buffer_compute_new_alloc_size(const RawBuffer *raw_buffer,
                                              usize capacity, usize *alloc_size)
{
    usize extra;
    usize total_elems;

    extra = raw_buffer_has_zero_sentinel(raw_buffer) ? 1 : 0;
    if (d_math_overflow_check_add_usize(capacity, extra, &total_elems))
        return false;
    if (d_math_overflow_check_mul_usize(total_elems, raw_buffer->elem_size, alloc_size))
        return false;
    return true;
}

static void raw_buffer_write_sentinel(RawBuffer *raw_buffer)
{
    if (raw_buffer->data == NULL || !raw_buffer_has_zero_sentinel(raw_buffer))
        return;
    memset(raw_buffer_elt_pos(raw_buffer, raw_buffer->size), 0, raw_buffer_elt_size(raw_buffer, 1));
}

static RawBuffer *raw_buffer_new_raw()
{
    return malloc(sizeof(RawBuffer));
}

static DResult increase_raw_buffer_capacity_if_needed(RawBuffer *raw_buffer, usize pos_start_cpy, usize nb_elem_to_copy)
{
    usize new_capacity;
    usize alloc_size;
    void *tmp;

    if (nb_elem_to_copy <= raw_buffer_nb_available_elem_slot_from_pos(raw_buffer, pos_start_cpy))
        return D_OK;
    if (d_math_overflow_check_add_usize(pos_start_cpy, nb_elem_to_copy, &new_capacity))
        return D_ERR_OVERFLOW;
    if (d_math_overflow_check_mul_usize(new_capacity, GROWTH_POLICY, &new_capacity))
        return D_ERR_OVERFLOW;
    if (!raw_buffer_compute_new_alloc_size(raw_buffer, new_capacity, &alloc_size))
        return D_ERR_OVERFLOW;
    tmp = realloc(raw_buffer->data, alloc_size);
    if (tmp == NULL)
        return D_ERR_ALLOC;
    raw_buffer->data = tmp;
    raw_buffer->capacity = new_capacity;
    raw_buffer_write_sentinel(raw_buffer);
    return D_OK;
}

DResult raw_buffer_init(RawBuffer *raw_buffer, usize elem_size, usize capacity, DBits8 opts)
{
    if (raw_buffer == NULL || elem_size == 0)
        return D_ERR_INVALID_ARG;
    raw_buffer->size = 0;
    raw_buffer->capacity = capacity == 0 ? DEFAULT_CAPACITY : capacity;
    raw_buffer->elem_size = elem_size;
    raw_buffer->opts = opts;
    raw_buffer->data = NULL;

    usize alloc_size;
    if (!raw_buffer_compute_new_alloc_size(raw_buffer, raw_buffer->capacity, &alloc_size) != D_OK)
        return D_ERR_OVERFLOW;
    if ((raw_buffer->data = malloc(alloc_size)) == NULL)
        return D_ERR_ALLOC;
    raw_buffer_write_sentinel(raw_buffer);
    return D_OK;
}

DResult raw_buffer_init_with_data(RawBuffer *raw_buffer, usize elem_size, const void *data, usize size, DBits8 opts)
{
    usize capacity = size < DEFAULT_CAPACITY ? DEFAULT_CAPACITY : size;
    DResult op_result = raw_buffer_init(raw_buffer, elem_size, capacity, opts);
    if (op_result != D_OK)
        return op_result;
    op_result = raw_buffer_append_data(raw_buffer, data, size);
    if (op_result != D_OK)
        return op_result;
    return D_OK;
}

RawBuffer *raw_buffer_new(usize elem_size, usize capacity, DBits8 opts)
{
    RawBuffer *raw_buffer;

    raw_buffer = raw_buffer_new_raw();
    if (raw_buffer == NULL)
        return NULL;
    if (raw_buffer_init(raw_buffer, elem_size, capacity, opts) != D_OK)
    {
        free(raw_buffer);
        return NULL;
    }
    return raw_buffer;
}

RawBuffer *raw_buffer_new_from(const RawBuffer *src)
{
    RawBuffer *new_buffer;

    if (src == NULL)
        return NULL;

    new_buffer = raw_buffer_new_raw();
    if (new_buffer == NULL)
        return NULL;

    if (raw_buffer_init_with_data(new_buffer, src->elem_size, src->data, src->size, src->opts) != D_OK)
    {
        raw_buffer_destroy(&new_buffer);
        return NULL;
    }
    return new_buffer;
}

void raw_buffer_free(RawBuffer *raw_buffer)
{
    if (raw_buffer == NULL)
        return;
    free(raw_buffer->data);
    raw_buffer->data = NULL;
    raw_buffer->size = 0;
    raw_buffer->capacity = 0;
}

void raw_buffer_destroy(RawBuffer **raw_buffer)
{
    if (raw_buffer == NULL || *raw_buffer == NULL)
        return;
    raw_buffer_free(*raw_buffer);
    free(*raw_buffer);
    *raw_buffer = NULL;
}

void *raw_buffer_get_data(RawBuffer *raw_buffer)
{
    if (raw_buffer == NULL)
        return NULL;
    return raw_buffer->data;
}

DResult raw_buffer_get_elem_at(RawBuffer *raw_buffer, usize index, void *out_elem)
{
    if (raw_buffer == NULL || index >= raw_buffer->size || out_elem == NULL)
        return D_ERR_INVALID_ARG;
    memcpy(out_elem, raw_buffer_elt_pos(raw_buffer, index), raw_buffer->elem_size);
    return D_OK;
}

DResult raw_buffer_get_last_elem(RawBuffer *raw_buffer, void *out_elem)
{
    if (raw_buffer == NULL)
        return D_ERR_INVALID_ARG;
    return raw_buffer_get_elem_at(raw_buffer, raw_buffer->size - 1, out_elem);
}

DResult raw_buffer_get_first_elem(RawBuffer *raw_buffer, void *out_elem)
{
    return raw_buffer_get_elem_at(raw_buffer, 0, out_elem);
}

DResult raw_buffer_insert_data_from_raw_data(RawBuffer *dst, usize dst_pos, const void *data, usize size)
{
    usize dst_size;
    usize byte_pos;
    usize byte_size;
    usize bytes_to_move;
    char *base;

    if (dst == NULL || data == NULL)
        return D_ERR_INVALID_ARG;
    else if (size == 0)
        return D_OK;
    dst_size = raw_buffer_nb_occupied_elem_slot(dst);
    if (dst_pos > dst_size)
        return D_ERR_INVALID_ARG;
    DResult op_result = increase_raw_buffer_capacity_if_needed(dst, dst_pos, size);
    if (op_result != D_OK)
        return op_result;
    byte_pos = dst_pos * dst->elem_size;
    byte_size = size * dst->elem_size;
    base = dst->data;
    if (dst_pos != dst_size)
    {
        bytes_to_move = (dst_size - dst_pos) * dst->elem_size;
        memmove(base + byte_pos + byte_size, base + byte_pos, bytes_to_move);
    }
    memmove(base + byte_pos, data, byte_size);
    dst->size += size;
    raw_buffer_write_sentinel(dst);
    return D_OK;
}

DResult raw_buffer_insert_data_from_raw_buffer(RawBuffer *dst, const RawBuffer *src, usize dst_pos)
{
    if (dst == NULL || src == NULL)
        return D_ERR_INVALID_ARG;
    return raw_buffer_insert_data_from_raw_data(dst, dst_pos, src->data, src->size);
}

DResult raw_buffer_resize(RawBuffer *raw_buffer, usize new_size, void *filler)
{
    if (raw_buffer == NULL)
        return D_ERR_INVALID_ARG;
    usize cnt_size = raw_buffer->size;
    if (new_size > cnt_size && filler == NULL)
        return D_ERR_INVALID_ARG;
    usize to_cpy = new_size > cnt_size ? new_size - cnt_size : 0;
    if (to_cpy != 0)
    {
        DResult op_result = increase_raw_buffer_capacity_if_needed(raw_buffer, cnt_size, to_cpy);
        if (op_result != D_OK)
            return op_result;
        memfill(raw_buffer_elt_pos(raw_buffer, cnt_size), filler, raw_buffer->elem_size, to_cpy);
    }
    raw_buffer->size = new_size;
    raw_buffer_write_sentinel(raw_buffer);
    return D_OK;
}

DResult raw_buffer_remove(RawBuffer *raw_buffer, usize pos, usize len_to_remove)
{
    if (raw_buffer == NULL || pos >= raw_buffer->size)
        return D_ERR_INVALID_ARG;

    usize cnt_size = raw_buffer->size;
    usize total_size;

    if (d_math_overflow_check_add_usize(pos, len_to_remove, &total_size))
        return D_ERR_INVALID_ARG;

    len_to_remove = total_size > cnt_size ? cnt_size - pos : len_to_remove;
    usize to_cpy = cnt_size - total_size;
    if (to_cpy != 0)
        memmove(raw_buffer_elt_pos(raw_buffer, pos), raw_buffer_elt_pos(raw_buffer, total_size), raw_buffer_elt_size(raw_buffer, to_cpy));
    raw_buffer->size -= len_to_remove;
    raw_buffer_write_sentinel(raw_buffer);
    return D_OK;
}

DResult raw_buffer_replace_data_at(RawBuffer *raw_buffer, usize pos, const void *data, usize size)
{
    if (raw_buffer == NULL || data == NULL || pos > raw_buffer->size)
        return D_ERR_INVALID_ARG;
    DResult op_result;
    if ((op_result = increase_raw_buffer_capacity_if_needed(raw_buffer, pos, size)) != D_OK)
        return op_result;
    memmove(raw_buffer_elt_pos(raw_buffer, pos), data, raw_buffer_elt_size(raw_buffer, size));
    raw_buffer->size = pos + size;
    raw_buffer_write_sentinel(raw_buffer);
    return D_OK;
}

DResult raw_buffer_replace(RawBuffer *dst, const RawBuffer *src, usize pos)
{
    if (dst == NULL || src == NULL)
        return D_ERR_INVALID_ARG;
    return raw_buffer_replace_data_at(dst, pos, src->data, src->size);
}

DResult raw_buffer_append_buffer(RawBuffer *dst, const RawBuffer *src)
{
    if (dst == NULL || src == NULL)
        return D_ERR_INVALID_ARG;
    return raw_buffer_replace_data_at(dst, dst->size, src->data, src->size);
}

DResult raw_buffer_append_data(RawBuffer *dst, const void *data, usize size)
{
    if (dst == NULL)
        return D_ERR_INVALID_ARG;
    return raw_buffer_replace_data_at(dst, dst->size, data, size);
}

DResult raw_buffer_prepend(RawBuffer *dst, const RawBuffer *src)
{
    return raw_buffer_insert_data_from_raw_buffer(dst, src, 0);
}

DResult raw_buffer_push(RawBuffer *raw_buffer, const void *elem)
{
    if (raw_buffer == NULL || elem == NULL)
        return D_ERR_INVALID_ARG;
    DResult op_result = increase_raw_buffer_capacity_if_needed(raw_buffer, raw_buffer->size, 1);
    if (op_result != D_OK)
        return op_result;
    memmove(raw_buffer_elt_pos(raw_buffer, raw_buffer->size), elem, raw_buffer_elt_size(raw_buffer, 1));
    raw_buffer->size++;
    raw_buffer_write_sentinel(raw_buffer);
    return D_OK;
}

DResult raw_buffer_pop(RawBuffer *raw_buffer, void *out_elem)
{
    if (raw_buffer == NULL)
        return D_ERR_INVALID_ARG;
    if (raw_buffer->size == 0)
        return D_OK;
    raw_buffer->size--;
    if (out_elem != NULL)
        memcpy(out_elem, raw_buffer_elt_pos(raw_buffer, raw_buffer->size), raw_buffer_elt_size(raw_buffer, 1));
    raw_buffer_write_sentinel(raw_buffer);
    return D_OK;
}

DResult raw_buffer_swap_remove(RawBuffer *raw_buffer, usize index, void *out_elem)
{
    if (raw_buffer == NULL || index >= raw_buffer->size)
        return D_ERR_INVALID_ARG;
    if (out_elem != NULL)
        memcpy(out_elem, raw_buffer_elt_pos(raw_buffer, index), raw_buffer_elt_size(raw_buffer, 1));
    raw_buffer->size--;
    if (index != raw_buffer->size)
        memcpy(raw_buffer_elt_pos(raw_buffer, index), raw_buffer_elt_pos(raw_buffer, raw_buffer->size), raw_buffer_elt_size(raw_buffer, 1));
    raw_buffer_write_sentinel(raw_buffer);
    return D_OK;
}

void raw_buffer_clear(RawBuffer *raw_buffer)
{
    if (raw_buffer == NULL)
        return;
    raw_buffer->size = 0;
    raw_buffer_write_sentinel(raw_buffer);
}
