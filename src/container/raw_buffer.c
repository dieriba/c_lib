#include <stdlib.h>
#include <string.h>
#include "container.h"
#include "raw_buffer.h"
#include "d_math.h"
#include "d_general_lib.h"

#define buffer_nb_available_elem_slot(raw_buffer) \
    ((raw_buffer)->capacity - (raw_buffer)->len)
#define buffer_nb_occupied_elem_slot(raw_buffer) \
    ((raw_buffer)->len)

#define buffer_elt_len(raw_buffer, i) raw_buffer->elem_size *i
#define buffer_elt_pos(raw_buffer, i) (char *)raw_buffer->data + (buffer_elt_len(raw_buffer, i))



static bool buffer_has_zero_sentinel(const RawBuffer *raw_buffer)
{
    return d_bits_8_check_bit_set(raw_buffer->opts, CNT_OPT_ZERO_SENTINEL);
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
    memset(buffer_elt_pos(raw_buffer, raw_buffer->len), 0, buffer_elt_len(raw_buffer, 1));
}

static RawBuffer *buffer_new_raw()
{
    return malloc(sizeof(RawBuffer));
}

Result buffer_init(RawBuffer *raw_buffer, usize elem_size, usize capacity, DBits8 opts)
{
    if (raw_buffer == NULL || elem_size == 0)
        return ERROR;
    raw_buffer->len = 0;
    raw_buffer->capacity = capacity == 0 ? DEFAULT_CAPACITY : capacity;
    raw_buffer->elem_size = elem_size;
    raw_buffer->opts = opts;
    raw_buffer->data = NULL;

    usize alloc_size;
    if (!buffer_compute_new_alloc_size(raw_buffer, raw_buffer->capacity, &alloc_size))
        return ERROR;
    if ((raw_buffer->data = malloc(alloc_size)) == NULL)
        return ERROR;
    buffer_write_sentinel(raw_buffer);
    return OK;
}

Result buffer_init_with_data(RawBuffer *raw_buffer, usize elem_size, void *data, usize len, DBits8 opts)
{
    usize capacity = len < DEFAULT_CAPACITY ? DEFAULT_CAPACITY : len;
    if (buffer_init(raw_buffer, elem_size, capacity, opts) == ERROR)
        return ERROR;
    if (buffer_append_data(raw_buffer, data, len) == ERROR)
        return ERROR;
    return OK;
}

RawBuffer *buffer_new(usize elem_size, usize capacity, DBits8 opts)
{
    RawBuffer *raw_buffer;

    raw_buffer = buffer_new_raw();
    if (raw_buffer == NULL)
        return NULL;
    if (buffer_init(raw_buffer, elem_size, capacity, opts) == ERROR)
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
    if (buffer_init_with_data(new_buffer, src->elem_size, src->data, src->len, src->opts) == ERROR)
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
    raw_buffer->len = 0;
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

Result buffer_increase_capacity_if_needed(RawBuffer *raw_buffer, usize nb_elem_to_copy)
{
    usize new_capacity;
    usize alloc_size;
    void *tmp;

    if (raw_buffer == NULL)
        return ERROR;
    if (nb_elem_to_copy <= buffer_nb_available_elem_slot(raw_buffer))
        return OK;
    if (d_mathcheck_add_usize(raw_buffer->capacity, nb_elem_to_copy, &new_capacity))
        return ERROR;
    if (d_mathcheck_mul_usize(new_capacity, GROWTH_POLICY, &new_capacity))
        return ERROR;
    if (!buffer_compute_new_alloc_size(raw_buffer, new_capacity, &alloc_size))
        return ERROR;
    tmp = realloc(raw_buffer->data, alloc_size);
    if (tmp == NULL)
        return ERROR;
    raw_buffer->data = tmp;
    raw_buffer->capacity = new_capacity;
    buffer_write_sentinel(raw_buffer);
    return OK;
}

void *buffer_get_data(RawBuffer *raw_buffer)
{
    if (raw_buffer == NULL)
        return NULL;
    return raw_buffer->data;
}

void *buffer_get_elem_at(RawBuffer *raw_buffer, usize index)
{
    if (raw_buffer == NULL || index >= raw_buffer->len)
        return NULL;
    return buffer_elt_pos(raw_buffer, index);
}

void *buffer_get_last_elem(RawBuffer *raw_buffer)
{
    if (raw_buffer == NULL || raw_buffer->len == 0)
        return NULL;
    return buffer_elt_pos(raw_buffer, raw_buffer->len - 1);
}

void *buffer_get_first_elem(RawBuffer *raw_buffer)
{
    if (raw_buffer == NULL || raw_buffer->len == 0)
        return NULL;
    return buffer_elt_pos(raw_buffer, 0);
}

usize buffer_get_len(const RawBuffer *raw_buffer)
{
    if (raw_buffer == NULL)
        return 0;
    return raw_buffer->len;
}

usize buffer_get_capacity(const RawBuffer *raw_buffer)
{
    if (raw_buffer == NULL)
        return 0;
    return raw_buffer->capacity;
}

usize buffer_get_elem_size(const RawBuffer *raw_buffer)
{
    if (raw_buffer == NULL)
        return 0;
    return raw_buffer->elem_size;
}

DBits8 buffer_get_opts(const RawBuffer *raw_buffer)
{
    if (raw_buffer == NULL)
        return CNT_OPT_NONE;
    return raw_buffer->opts;
}

Result buffer_insert_data(RawBuffer *dst, usize dst_pos, const void *data, usize len)
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

    dst_len = buffer_nb_occupied_elem_slot(dst);
    if (dst_pos > dst_len)
        return ERROR;
    if (buffer_increase_capacity_if_needed(dst, len) == ERROR)
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
    buffer_write_sentinel(dst);
    return OK;
}

Result buffer_resize(RawBuffer *raw_buffer, usize new_len, void *filler)
{
    if (raw_buffer == NULL)
        return ERROR;
    usize cnt_len = raw_buffer->len;
    if (new_len == cnt_len)
        return OK;
    usize to_cpy = new_len > cnt_len ? new_len - cnt_len : 0;
    if (to_cpy != 0)
    {
        if (buffer_increase_capacity_if_needed(raw_buffer, to_cpy) == ERROR)
            return ERROR;
        memfill(buffer_elt_pos(raw_buffer, cnt_len), filler, raw_buffer->elem_size, to_cpy);
    }
    raw_buffer->len = new_len;
    buffer_write_sentinel(raw_buffer);
    return OK;
}

Result buffer_remove(RawBuffer *raw_buffer, usize pos, usize len_to_remove)
{
    if (raw_buffer == NULL || pos >= raw_buffer->len)
        return ERROR;

    usize cnt_len = raw_buffer->len;
    usize total_len;

    if (d_mathcheck_add_usize(pos, len_to_remove, &total_len))
        return ERROR;

    len_to_remove = total_len > cnt_len ? cnt_len - pos : len_to_remove;
    usize to_cpy = cnt_len - total_len;
    if (to_cpy != 0)
        memmove(buffer_elt_pos(raw_buffer, pos), buffer_elt_pos(raw_buffer, total_len), buffer_elt_len(raw_buffer, to_cpy));
    raw_buffer->len -= len_to_remove;
    buffer_write_sentinel(raw_buffer);
    return OK;
}

Result buffer_replace_data(RawBuffer *raw_buffer, usize pos, const void *data, usize len)
{
    if (raw_buffer == NULL || data == NULL || pos >= raw_buffer->len)
        return ERROR;
    if (len == 0)
        return OK;

    usize total_len;
    if (d_mathcheck_add_usize(pos, len, &total_len))
        return ERROR;
    usize extra_elem_to_allocate = total_len > raw_buffer->capacity ? total_len - raw_buffer->capacity : 0;
    if (extra_elem_to_allocate != 0 && buffer_increase_capacity_if_needed(raw_buffer, extra_elem_to_allocate) == ERROR)
        return ERROR;
    memmove(buffer_elt_pos(raw_buffer, pos), data, buffer_elt_len(raw_buffer, len));
    if (extra_elem_to_allocate != 0)
    {
        raw_buffer->len = total_len;
        buffer_write_sentinel(raw_buffer);
    }
    return OK;
}

Result buffer_replace_data_trunc(RawBuffer *raw_buffer, usize pos, const void *data, usize len)
{
    if (raw_buffer == NULL)
        return ERROR;
    if (pos >= raw_buffer->len)
        return ERROR;
    if (len != 0)
    {
        if (buffer_replace_data(raw_buffer, pos, data, len) == ERROR)
            return ERROR;
    }

    raw_buffer->len = pos + len;
    buffer_write_sentinel(raw_buffer);
    return OK;
}

Result buffer_replace_buffer_trunc(RawBuffer *dst, const RawBuffer *src)
{
    if (dst == NULL || src == NULL || buffer_replace_data_trunc(dst, 0, src->data, src->len) == ERROR)
        return ERROR;
    return OK;
}

Result buffer_append_buffer(RawBuffer *dst, const RawBuffer *src)
{
    if (dst == NULL || src == NULL)
        return ERROR;
    return buffer_insert_data(dst, dst->len, src->data, src->len);
}

Result buffer_append_data(RawBuffer *dst, const void *data, usize len)
{
    if (dst == NULL)
        return ERROR;
    return buffer_insert_data(dst, dst->len, data, len);
}

Result buffer_prepend(RawBuffer *dst, const RawBuffer *src)
{
    if (dst == NULL || src == NULL)
        return ERROR;
    return buffer_insert_data(dst, 0, src->data, src->len);
}

Result buffer_push(RawBuffer *raw_buffer, const void *elem)
{
    if (raw_buffer == NULL || elem == NULL)
        return ERROR;
    if (buffer_increase_capacity_if_needed(raw_buffer, 1) == ERROR)
        return ERROR;
    memmove(buffer_elt_pos(raw_buffer, raw_buffer->len), elem, buffer_elt_len(raw_buffer, 1));
    raw_buffer->len++;
    buffer_write_sentinel(raw_buffer);
    return OK;
}

Result buffer_pop(RawBuffer *raw_buffer, void *out_elem)
{
    if (raw_buffer == NULL)
        return ERROR;
    if (raw_buffer->len == 0)
        return OK;
    raw_buffer->len--;
    if (out_elem != NULL)
        memcpy(out_elem, buffer_elt_pos(raw_buffer, raw_buffer->len), buffer_elt_len(raw_buffer, 1));
    buffer_write_sentinel(raw_buffer);
    return OK;
}

Result buffer_swap_remove(RawBuffer *raw_buffer, usize index, void *out_elem)
{
    if (raw_buffer == NULL || index >= raw_buffer->len)
        return ERROR;
    if (out_elem != NULL)
        memcpy(out_elem, buffer_elt_pos(raw_buffer, index), buffer_elt_len(raw_buffer, 1));
    raw_buffer->len--;
    if (index != raw_buffer->len)
        memcpy(buffer_elt_pos(raw_buffer, index), buffer_elt_pos(raw_buffer, raw_buffer->len), buffer_elt_len(raw_buffer, 1));
    buffer_write_sentinel(raw_buffer);
    return OK;
}

void buffer_clear(RawBuffer *raw_buffer)
{
    if (raw_buffer == NULL)
        return;
    raw_buffer->len = 0;
    buffer_write_sentinel(raw_buffer);
}
