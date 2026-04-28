#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "d_math.h"
#include "d_general_lib.h"

#define GROWTH_POLICY 2

#define buffer_nb_available_elem_slot(buffer) \
    ((buffer)->capacity - (buffer)->len)
#define buffer_nb_occupied_elem_slot(buffer) \
    ((buffer)->len)

#define buffer_elt_len(buffer, i) buffer->elem_size *i
#define buffer_elt_pos(buffer, i) (char *)buffer->data + (buffer_elt_len(buffer, i))

static bool buffer_has_zero_sentinel(const Buffer *buffer)
{
    return d_bits_8_check_bit_set(buffer->opts, CNT_OPT_ZERO_SENTINEL);
}

static bool buffer_compute_new_alloc_size(const Buffer *buffer,
                                          usize capacity, usize *alloc_size)
{
    usize extra;
    usize total_elems;

    extra = buffer_has_zero_sentinel(buffer) ? 1 : 0;
    if (d_overflow_check_add_usize(capacity, extra, &total_elems))
        return false;
    if (d_overflow_check_mul_usize(total_elems, buffer->elem_size, alloc_size))
        return false;
    return true;
}

static void buffer_write_sentinel(Buffer *buffer)
{
    if (buffer->data == NULL || !buffer_has_zero_sentinel(buffer))
        return;
    memset(buffer_elt_pos(buffer, buffer->len), 0, buffer_elt_len(buffer, 1));
}

static Buffer *buffer_new_raw()
{
    return malloc(sizeof(Buffer));
}

Result buffer_init(Buffer *buffer, usize elem_size, usize capacity, DBits8 opts)
{
    if (buffer == NULL || elem_size == 0)
        return ERROR;
    buffer->len = 0;
    buffer->capacity = capacity == 0 ? DEFAULT_CAPACITY : capacity;
    buffer->elem_size = elem_size;
    buffer->opts = opts;
    buffer->data = NULL;

    usize alloc_size;
    if (!buffer_compute_new_alloc_size(buffer, buffer->capacity, &alloc_size))
        return ERROR;
    if ((buffer->data = malloc(alloc_size)) == NULL)
        return ERROR;
    buffer_write_sentinel(buffer);
    return OK;
}

Result buffer_init_with_data(Buffer *buffer, usize elem_size, void *data, usize len, DBits8 opts)
{
    usize capacity = len < DEFAULT_CAPACITY ? DEFAULT_CAPACITY : len;
    if (buffer_init(buffer, elem_size, capacity, opts) == ERROR)
        return ERROR;
    if (buffer_append_data(buffer, data, len) == ERROR)
        return ERROR;
    return OK;
}

Buffer *buffer_new(usize elem_size, usize capacity, DBits8 opts)
{
    Buffer *buffer;

    buffer = buffer_new_raw();
    if (buffer == NULL)
        return NULL;
    if (buffer_init(buffer, elem_size, capacity, opts) == ERROR)
    {
        free(buffer);
        return NULL;
    }
    return buffer;
}

Buffer *buffer_new_from(const Buffer *src)
{
    Buffer *new_buffer;

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

void buffer_free(Buffer *buffer)
{
    if (buffer == NULL)
        return;
    free(buffer->data);
    buffer->data = NULL;
    buffer->len = 0;
    buffer->capacity = 0;
}

void buffer_destroy(Buffer **buffer)
{
    if (buffer == NULL || *buffer == NULL)
        return;
    buffer_free(*buffer);
    free(*buffer);
    *buffer = NULL;
}

Result buffer_increase_capacity_if_needed(Buffer *buffer, usize nb_elem_to_copy)
{
    usize new_capacity;
    usize alloc_size;
    void *tmp;

    if (buffer == NULL)
        return ERROR;
    if (nb_elem_to_copy <= buffer_nb_available_elem_slot(buffer))
        return OK;
    if (d_overflow_check_add_usize(buffer->capacity, nb_elem_to_copy, &new_capacity))
        return ERROR;
    if (d_overflow_check_mul_usize(new_capacity, GROWTH_POLICY, &new_capacity))
        return ERROR;
    if (!buffer_compute_new_alloc_size(buffer, new_capacity, &alloc_size))
        return ERROR;
    tmp = realloc(buffer->data, alloc_size);
    if (tmp == NULL)
        return ERROR;
    buffer->data = tmp;
    buffer->capacity = new_capacity;
    buffer_write_sentinel(buffer);
    return OK;
}

void *buffer_get_data(Buffer *buffer)
{
    if (buffer == NULL)
        return NULL;
    return buffer->data;
}

void *buffer_get_elem_at(Buffer *buffer, usize index)
{
    if (buffer == NULL || index >= buffer->len)
        return NULL;
    return buffer_elt_pos(buffer, index);
}

void *buffer_get_last_elem(Buffer *buffer)
{
    if (buffer == NULL || buffer->len == 0)
        return NULL;
    return buffer_elt_pos(buffer, buffer->len - 1);
}

void *buffer_get_first_elem(Buffer *buffer)
{
    if (buffer == NULL || buffer->len == 0)
        return NULL;
    return buffer_elt_pos(buffer, 0);
}

usize buffer_get_len(const Buffer *buffer)
{
    if (buffer == NULL)
        return 0;
    return buffer->len;
}

usize buffer_get_capacity(const Buffer *buffer)
{
    if (buffer == NULL)
        return 0;
    return buffer->capacity;
}

usize buffer_get_elem_size(const Buffer *buffer)
{
    if (buffer == NULL)
        return 0;
    return buffer->elem_size;
}

DBits8 buffer_get_opts(const Buffer *buffer)
{
    if (buffer == NULL)
        return CNT_OPT_NONE;
    return buffer->opts;
}

Result buffer_insert_data(Buffer *dst, usize dst_pos, const void *data, usize len)
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

Result buffer_resize(Buffer *buffer, usize new_len, void *filler)
{
    if (buffer == NULL)
        return ERROR;
    usize cnt_len = buffer->len;
    if (new_len == cnt_len)
        return OK;
    usize to_cpy = new_len > cnt_len ? new_len - cnt_len : 0;
    if (to_cpy != 0)
    {
        if (buffer_increase_capacity_if_needed(buffer, to_cpy) == ERROR)
            return ERROR;
        memfill(buffer_elt_pos(buffer, cnt_len), filler, buffer->elem_size, to_cpy);
    }
    buffer->len = new_len;
    buffer_write_sentinel(buffer);
    return OK;
}

Result buffer_remove(Buffer *buffer, usize pos, usize len_to_remove)
{
    if (buffer == NULL || pos >= buffer->len)
        return ERROR;

    usize cnt_len = buffer->len;
    usize total_len;

    if (d_overflow_check_add_usize(pos, len_to_remove, &total_len))
        return ERROR;

    len_to_remove = total_len > cnt_len ? cnt_len - pos : len_to_remove;
    usize to_cpy = cnt_len - total_len;
    if (to_cpy != 0)
        memmove(buffer_elt_pos(buffer, pos), buffer_elt_pos(buffer, total_len), buffer_elt_len(buffer, to_cpy));
    buffer->len -= len_to_remove;
    buffer_write_sentinel(buffer);
    return OK;
}

Result buffer_replace_data(Buffer *buffer, usize pos, const void *data, usize len)
{
    if (buffer == NULL || data == NULL || pos >= buffer->len)
        return ERROR;
    if (len == 0)
        return OK;

    usize total_len;
    if (d_overflow_check_add_usize(pos, len, &total_len))
        return ERROR;
    usize extra_elem_to_allocate = total_len > buffer->capacity ? total_len - buffer->capacity : 0;
    if (extra_elem_to_allocate != 0 && buffer_increase_capacity_if_needed(buffer, extra_elem_to_allocate) == ERROR)
        return ERROR;
    memmove(buffer_elt_pos(buffer, pos), data, buffer_elt_len(buffer, len));
    if (extra_elem_to_allocate != 0)
    {
        buffer->len = total_len;
        buffer_write_sentinel(buffer);
    }
    return OK;
}

Result buffer_replace_data_trunc(Buffer *buffer, usize pos, const void *data, usize len)
{
    if (buffer == NULL)
        return ERROR;
    if (pos >= buffer->len)
        return ERROR;
    if (len != 0)
    {
        if (buffer_replace_data(buffer, pos, data, len) == ERROR)
            return ERROR;
    }

    buffer->len = pos + len;
    buffer_write_sentinel(buffer);
    return OK;
}

Result buffer_replace_buffer_trunc(Buffer *dst, const Buffer *src)
{
    if (dst == NULL || src == NULL || buffer_replace_data_trunc(dst, 0, src->data, src->len) == ERROR)
        return ERROR;
    return OK;
}

Result buffer_append_buffer(Buffer *dst, const Buffer *src)
{
    if (dst == NULL || src == NULL)
        return ERROR;
    return buffer_insert_data(dst, dst->len, src->data, src->len);
}

Result buffer_append_data(Buffer *dst, const void *data, usize len)
{
    if (dst == NULL)
        return ERROR;
    return buffer_insert_data(dst, dst->len, data, len);
}

Result buffer_prepend(Buffer *dst, const Buffer *src)
{
    if (dst == NULL || src == NULL)
        return ERROR;
    return buffer_insert_data(dst, 0, src->data, src->len);
}

Result buffer_push(Buffer *buffer, const void *elem)
{
    if (buffer == NULL || elem == NULL)
        return ERROR;
    if (buffer_increase_capacity_if_needed(buffer, 1) == ERROR)
        return ERROR;
    memmove(buffer_elt_pos(buffer, buffer->len), elem, buffer_elt_len(buffer, 1));
    buffer->len++;
    buffer_write_sentinel(buffer);
    return OK;
}

Result buffer_pop(Buffer *buffer, void *out_elem)
{
    if (buffer == NULL)
        return ERROR;
    if (buffer->len == 0)
        return OK;
    buffer->len--;
    if (out_elem != NULL)
        memcpy(out_elem, buffer_elt_pos(buffer, buffer->len), buffer_elt_len(buffer, 1));
    buffer_write_sentinel(buffer);
    return OK;
}

Result buffer_swap_remove(Buffer *buffer, usize index, void *out_elem)
{
    if (buffer == NULL || index >= buffer->len)
        return ERROR;
    if (out_elem != NULL)
        memcpy(out_elem, buffer_elt_pos(buffer, index), buffer_elt_len(buffer, 1));
    buffer->len--;
    if (index != buffer->len)
        memcpy(buffer_elt_pos(buffer, index), buffer_elt_pos(buffer, buffer->len), buffer_elt_len(buffer, 1));
    buffer_write_sentinel(buffer);
    return OK;
}

void buffer_clear(Buffer *buffer)
{
    if (buffer == NULL)
        return;
    buffer->len = 0;
    buffer_write_sentinel(buffer);
}
