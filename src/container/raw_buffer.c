#include <stdlib.h>
#include <string.h>
#include "container.h"
#include "raw_buffer.h"
#include "d_math.h"
#include "d_general_lib.h"

#define buffer_nb_available_elem_slot(raw_buffer) \
    ((raw_buffer)->capacity - (raw_buffer)->size)
#define buffer_nb_occupied_elem_slot(raw_buffer) \
    ((raw_buffer)->size)

#define buffer_elt_size(raw_buffer, i) raw_buffer->elem_size *i
#define buffer_elt_pos(raw_buffer, i) (char *)raw_buffer->data + (buffer_elt_size(raw_buffer, i))

static bool buffer_has_zero_sentinel(const RawBuffer *raw_buffer)
{
    return d_bits_8_check_bit_set(raw_buffer->opts, RAW_BUF_OPT_ZERO_SENTINEL);
}

static bool buffer_compute_new_alloc_size(const RawBuffer *raw_buffer,
                                          usize capacity, usize *alloc_size)
{
    usize extra;
    usize total_elems;

    extra = buffer_has_zero_sentinel(raw_buffer) ? 1 : 0;
    if (d_mathcheck_add_usize(capacity, extra, &total_elems))
        return false;
    if (d_mathcheck_mul_usize(total_elems, raw_buffer->elem_size, alloc_size))
        return false;
    return true;
}

static void buffer_write_sentinel(RawBuffer *raw_buffer)
{
    if (raw_buffer->data == NULL || !buffer_has_zero_sentinel(raw_buffer))
        return;
    memset(buffer_elt_pos(raw_buffer, raw_buffer->size), 0, buffer_elt_size(raw_buffer, 1));
}

static RawBuffer *buffer_new_raw()
{
    return malloc(sizeof(RawBuffer));
}

static DResult increase_buffer_capacity_if_needed(RawBuffer *raw_buffer, usize nb_elem_to_copy)
{
    usize new_capacity;
    usize alloc_size;
    void *tmp;

    if (raw_buffer == NULL)
        return D_ERR_INVALID_ARG;
    if (nb_elem_to_copy <= buffer_nb_available_elem_slot(raw_buffer))
        return D_OK;
    if (d_mathcheck_add_usize(raw_buffer->capacity, nb_elem_to_copy, &new_capacity))
        return D_ERR_INVALID_ARG;
    if (d_mathcheck_mul_usize(new_capacity, GROWTH_POLICY, &new_capacity))
        return D_ERR_INVALID_ARG;
    if (!buffer_compute_new_alloc_size(raw_buffer, new_capacity, &alloc_size))
        return D_ERR_INVALID_ARG;
    tmp = realloc(raw_buffer->data, alloc_size);
    if (tmp == NULL)
        return D_ERR_ALLOC;
    raw_buffer->data = tmp;
    raw_buffer->capacity = new_capacity;
    buffer_write_sentinel(raw_buffer);
    return D_OK;
}

DResult buffer_init(RawBuffer *raw_buffer, usize elem_size, usize capacity, DBits8 opts)
{
    if (raw_buffer == NULL || elem_size == 0)
        return D_ERR_INVALID_ARG;
    raw_buffer->size = 0;
    raw_buffer->capacity = capacity == 0 ? DEFAULT_CAPACITY : capacity;
    raw_buffer->elem_size = elem_size;
    raw_buffer->opts = opts;
    raw_buffer->data = NULL;

    usize alloc_size;
    DResult op_result = buffer_compute_new_alloc_size(raw_buffer, raw_buffer->capacity, &alloc_size);
    if (op_result != D_OK)
        return op_result;
    if ((raw_buffer->data = malloc(alloc_size)) == NULL)
        return D_ERR_ALLOC;
    buffer_write_sentinel(raw_buffer);
    return D_OK;
}

DResult buffer_init_with_data(RawBuffer *raw_buffer, usize elem_size, void *data, usize len, DBits8 opts)
{
    usize capacity = len < DEFAULT_CAPACITY ? DEFAULT_CAPACITY : len;
    DResult op_result = buffer_init(raw_buffer, elem_size, capacity, opts);
    if (op_result != D_OK)
        return op_result;
    op_result = buffer_append_data(raw_buffer, data, len);
    if (op_result != D_OK)
        return op_result;
    return D_OK;
}

RawBuffer *buffer_new(usize elem_size, usize capacity, DBits8 opts)
{
    RawBuffer *raw_buffer;

    raw_buffer = buffer_new_raw();
    if (raw_buffer == NULL)
        return NULL;
    if (buffer_init(raw_buffer, elem_size, capacity, opts) != D_OK)
    {
        free(raw_buffer);
        return NULL;
    }
    return raw_buffer;
}

RawBuffer *buffer_new_from(const RawBuffer *src)
{
    RawBuffer *new_buffer;

    if (src == NULL)
        return NULL;

    new_buffer = buffer_new_raw();
    if (new_buffer == NULL)
        return NULL;

    if (buffer_init_with_data(new_buffer, src->elem_size, src->data, src->size, src->opts) != D_OK)
    {
        buffer_destroy(&new_buffer);
        return NULL;
    }
    return new_buffer;
}

void buffer_free(RawBuffer *raw_buffer)
{
    if (raw_buffer == NULL)
        return;
    free(raw_buffer->data);
    raw_buffer->data = NULL;
    raw_buffer->size = 0;
    raw_buffer->capacity = 0;
}

void buffer_destroy(RawBuffer **raw_buffer)
{
    if (raw_buffer == NULL || *raw_buffer == NULL)
        return;
    buffer_free(*raw_buffer);
    free(*raw_buffer);
    *raw_buffer = NULL;
}

void *buffer_get_data(RawBuffer *raw_buffer)
{
    if (raw_buffer == NULL)
        return NULL;
    return raw_buffer->data;
}

void *buffer_get_elem_at(RawBuffer *raw_buffer, usize index)
{
    if (raw_buffer == NULL || index >= raw_buffer->size)
        return NULL;
    return buffer_elt_pos(raw_buffer, index);
}

void *buffer_get_last_elem(RawBuffer *raw_buffer)
{
    if (raw_buffer == NULL)
        return NULL;
    return buffer_get_elem_at(raw_buffer, raw_buffer->size - 1);
}

void *buffer_get_first_elem(RawBuffer *raw_buffer)
{
    return buffer_get_elem_at(raw_buffer, 0);
}

DResult buffer_insert_data(RawBuffer *dst, usize dst_pos, const void *data, usize len)
{
    usize dst_size;
    usize byte_pos;
    usize byte_size;
    usize bytes_to_move;
    char *base;

    if (dst == NULL || data == NULL || len == 0)
        return D_ERR_INVALID_ARG;

    dst_size = buffer_nb_occupied_elem_slot(dst);
    if (dst_pos > dst_size)
        return D_ERR_INVALID_ARG;
    DResult op_result = increase_buffer_capacity_if_needed(dst, len);
    if (op_result != D_OK)
        return op_result;
    byte_pos = dst_pos * dst->elem_size;
    byte_size = len * dst->elem_size;
    base = dst->data;
    if (dst_pos != dst_size)
    {
        bytes_to_move = (dst_size - dst_pos) * dst->elem_size;
        memmove(base + byte_pos + byte_size, base + byte_pos, bytes_to_move);
    }
    memmove(base + byte_pos, data, byte_size);
    dst->size += len;
    buffer_write_sentinel(dst);
    return D_OK;
}

DResult buffer_resize(RawBuffer *raw_buffer, usize new_size, void *filler)
{
    if (raw_buffer == NULL)
        return D_ERR_ALLOC;
    usize cnt_size = raw_buffer->size;
    if (new_size == cnt_size)
        return D_ERR_INVALID_ARG;
    usize to_cpy = new_size > cnt_size ? new_size - cnt_size : 0;
    if (to_cpy != 0)
    {
        DResult op_result = increase_buffer_capacity_if_needed(raw_buffer, to_cpy);
        if (op_result != D_OK)
            return op_result;
        memfill(buffer_elt_pos(raw_buffer, cnt_size), filler, raw_buffer->elem_size, to_cpy);
    }
    raw_buffer->size = new_size;
    buffer_write_sentinel(raw_buffer);
    return D_OK;
}

DResult buffer_remove(RawBuffer *raw_buffer, usize pos, usize len_to_remove)
{
    if (raw_buffer == NULL || pos >= raw_buffer->size)
        return D_ERR_INVALID_ARG;

    usize cnt_size = raw_buffer->size;
    usize total_size;

    if (d_mathcheck_add_usize(pos, len_to_remove, &total_size))
        return D_ERR_INVALID_ARG;

    len_to_remove = total_size > cnt_size ? cnt_size - pos : len_to_remove;
    usize to_cpy = cnt_size - total_size;
    if (to_cpy != 0)
        memmove(buffer_elt_pos(raw_buffer, pos), buffer_elt_pos(raw_buffer, total_size), buffer_elt_size(raw_buffer, to_cpy));
    raw_buffer->size -= len_to_remove;
    buffer_write_sentinel(raw_buffer);
    return D_OK;
}

DResult buffer_replace_data(RawBuffer *raw_buffer, usize pos, const void *data, usize len)
{
    if (raw_buffer == NULL || data == NULL || len == 0 || pos >= raw_buffer->size)
        return D_ERR_INVALID_ARG;
    usize total_size;
    if (d_mathcheck_add_usize(pos, len, &total_size))
        return D_ERR_INVALID_ARG;
    usize extra_elem_to_allocate = total_size > raw_buffer->capacity ? total_size - raw_buffer->capacity : 0;
    DResult op_result;
    if (extra_elem_to_allocate != 0 && (op_result = increase_buffer_capacity_if_needed(raw_buffer, extra_elem_to_allocate)) != D_OK)
        return op_result;
    memmove(buffer_elt_pos(raw_buffer, pos), data, buffer_elt_size(raw_buffer, len));
    if (extra_elem_to_allocate != 0)
    {
        raw_buffer->size = total_size;
        buffer_write_sentinel(raw_buffer);
    }
    return D_OK;
}

DResult buffer_replace_data_trunc(RawBuffer *raw_buffer, usize pos, const void *data, usize len)
{
    if (raw_buffer == NULL || pos >= raw_buffer->size)
        return D_ERR_INVALID_ARG;
    if (len != 0)
    {
        DResult op_result = buffer_replace_data(raw_buffer, pos, data, len);
        if (op_result != D_OK)
            return op_result;
    }

    raw_buffer->size = pos + len;
    buffer_write_sentinel(raw_buffer);
    return D_OK;
}

DResult buffer_replace_buffer_trunc(RawBuffer *dst, const RawBuffer *src)
{
    if (dst == NULL || src == NULL)
        return D_ERR_INVALID_ARG;

    return buffer_replace_data_trunc(dst, 0, src->data, src->size);
}

DResult buffer_append_buffer(RawBuffer *dst, const RawBuffer *src)
{
    if (dst == NULL || src == NULL)
        return D_ERR_INVALID_ARG;
    return buffer_insert_data(dst, dst->size, src->data, src->size);
}

DResult buffer_append_data(RawBuffer *dst, const void *data, usize len)
{
    if (dst == NULL)
        return D_ERR_INVALID_ARG;
    return buffer_insert_data(dst, dst->size, data, len);
}

DResult buffer_prepend(RawBuffer *dst, const RawBuffer *src)
{
    if (src == NULL)
        return D_ERR_INVALID_ARG;
    return buffer_insert_data(dst, 0, src->data, src->size);
}

DResult buffer_push(RawBuffer *raw_buffer, const void *elem)
{
    if (raw_buffer == NULL || elem == NULL)
        return D_ERR_INVALID_ARG;
    DResult op_result = increase_buffer_capacity_if_needed(raw_buffer, 1);
    if (op_result != D_OK)
        return op_result;
    memmove(buffer_elt_pos(raw_buffer, raw_buffer->size), elem, buffer_elt_size(raw_buffer, 1));
    raw_buffer->size++;
    buffer_write_sentinel(raw_buffer);
    return D_OK;
}

DResult buffer_pop(RawBuffer *raw_buffer, void *out_elem)
{
    if (raw_buffer == NULL)
        return D_ERR_INVALID_ARG;
    if (raw_buffer->size == 0)
        return D_OK;
    raw_buffer->size--;
    if (out_elem != NULL)
        memcpy(out_elem, buffer_elt_pos(raw_buffer, raw_buffer->size), buffer_elt_size(raw_buffer, 1));
    buffer_write_sentinel(raw_buffer);
    return D_OK;
}

DResult buffer_swap_remove(RawBuffer *raw_buffer, usize index, void *out_elem)
{
    if (raw_buffer == NULL || index >= raw_buffer->size)
        return D_ERR_INVALID_ARG;
    if (out_elem != NULL)
        memcpy(out_elem, buffer_elt_pos(raw_buffer, index), buffer_elt_size(raw_buffer, 1));
    raw_buffer->size--;
    if (index != raw_buffer->size)
        memcpy(buffer_elt_pos(raw_buffer, index), buffer_elt_pos(raw_buffer, raw_buffer->size), buffer_elt_size(raw_buffer, 1));
    buffer_write_sentinel(raw_buffer);
    return D_OK;
}

void buffer_clear(RawBuffer *raw_buffer)
{
    if (raw_buffer == NULL)
        return;
    raw_buffer->size = 0;
    buffer_write_sentinel(raw_buffer);
}
