#include <stdlib.h>
#include <string.h>

#include "raw_ring_buffer.h"
#include "container.h"
#include "d_bits.h"
#include "d_math.h"

#define raw_ring_buffer_get_head(raw_ring_buffer) ((char *)raw_ring_buffer->data + (raw_ring_buffer->head * raw_ring_buffer->elem_size))
#define raw_ring_buffer_get_tail(raw_ring_buffer) ((char *)raw_ring_buffer->data + (raw_ring_buffer->tail * raw_ring_buffer->elem_size))
#define raw_ring_buffer_get_elem_at(raw_ring_buffer, index) ((char *)raw_ring_buffer->data + index * raw_ring_buffer->elem_size)
#define raw_ring_buffer_elt_size(raw_ring_buffer, nb_elem) (raw_ring_buffer->elem_size * nb_elem)

typedef void (*PushFn)(RawRingBuffer *, const void *elem);
typedef void (*PopFn)(RawRingBuffer *, void *out_elem);

static RawRingBuffer *raw_ring_buffer_new_raw()
{
    return malloc(sizeof(RawRingBuffer));
}

static void raw_ring_buffer_destroy(RawRingBuffer **raw_ring_buffer)
{
    if (raw_ring_buffer == NULL || *raw_ring_buffer == NULL)
        return;
    RawRingBuffer *buf = *raw_ring_buffer;
    raw_ring_buffer_free(buf);
    free(buf);
    *raw_ring_buffer = NULL;
}

DResult raw_ring_buffer_init(RawRingBuffer *raw_ring_buffer, usize head, usize tail, usize capacity, usize elem_size)
{
    if (raw_ring_buffer == NULL || elem_size == 0)
        return D_ERR_INVALID_ARG;
    if (capacity == 0)
        capacity = DEFAULT_CAPACITY;
    else
    {
        usize new_capacity = d_math_compute_pow2(d_bits_get_index_least_significant_bit_set_ll(capacity));
        if (new_capacity < capacity && container_next_pow2_checked(&capacity) != D_OK)
            return D_ERR_OVERFLOW;
    }
    raw_ring_buffer->size = 0;
    raw_ring_buffer->capacity = capacity;
    raw_ring_buffer->elem_size = elem_size;
    raw_ring_buffer->data = NULL;
    raw_ring_buffer->head = head;
    raw_ring_buffer->tail = tail;
    usize alloc_size;
    if (d_math_overflow_check_mul_usize(capacity, raw_ring_buffer->elem_size, &alloc_size))
        return D_ERR_INVALID_ARG;
    if ((raw_ring_buffer->data = malloc(alloc_size)) == NULL)
        return D_ERR_ALLOC;
    return D_OK;
}

inline DResult raw_ring_buffer_default_init(RawRingBuffer *raw_ring_buffer, usize capacity, usize elem_size)
{
    return raw_ring_buffer_init(raw_ring_buffer, 0, 0, capacity, elem_size);
}

DResult raw_ring_buffer_init_with_data(RawRingBuffer *raw_ring_buffer, usize head, usize tail, usize elem_size, void *data, usize size)
{
    DResult op_result = raw_ring_buffer_init(raw_ring_buffer, head, tail, size, elem_size);
    if (op_result != D_OK)
        return op_result;
    raw_ring_buffer->size = size;
    memcpy(raw_ring_buffer->data, data, raw_ring_buffer_elt_size(raw_ring_buffer, size));
    return D_OK;
}

DResult raw_ring_buffer_new_from(RawRingBuffer **new_raw_ring_buffer, const RawRingBuffer *src)
{
    if (new_raw_ring_buffer == NULL || src == NULL)
        return D_ERR_INVALID_ARG;

    if ((*new_raw_ring_buffer = raw_ring_buffer_new_raw()) == NULL)
        return D_ERR_ALLOC;
    DResult op_result = raw_ring_buffer_init_with_data(*new_raw_ring_buffer, src->head, src->tail, src->elem_size, src->data, src->capacity);
    if (op_result != D_OK)
    {
        raw_ring_buffer_destroy(new_raw_ring_buffer);
        return op_result;
    }
    return D_OK;
}

static DResult increase_buffer_capacity_if_needed(RawRingBuffer *raw_ring_buffer)
{
    if (raw_ring_buffer == NULL)
        return D_ERR_INVALID_ARG;
    if (raw_ring_buffer->size + 1 < raw_ring_buffer->capacity)
        return D_OK;
    usize new_capacity = raw_ring_buffer->capacity;
    if (container_next_pow2_checked(&new_capacity) != D_OK)
        return D_ERR_OVERFLOW;
    usize alloc_size;
    if (d_math_overflow_check_mul_usize(new_capacity, raw_ring_buffer->elem_size, &alloc_size))
        return D_ERR_OVERFLOW;

    void *tmp = realloc(raw_ring_buffer->data, alloc_size);
    if (tmp == NULL)
        return D_ERR_ALLOC;
    raw_ring_buffer->data = tmp;
    usize nb_elem_from_head = raw_ring_buffer->capacity - raw_ring_buffer->head;
    if (raw_ring_buffer->head > raw_ring_buffer->tail)
    {
        usize new_head_pos = new_capacity - nb_elem_from_head;
        memcpy(
            raw_ring_buffer_get_elem_at(raw_ring_buffer, new_head_pos),
            raw_ring_buffer_get_head(raw_ring_buffer),
            raw_ring_buffer_elt_size(raw_ring_buffer, nb_elem_from_head));
        raw_ring_buffer->head = new_head_pos;
    }
    raw_ring_buffer->capacity = new_capacity;
    return D_OK;
}

static void push_front(RawRingBuffer *raw_ring_buffer, const void *elem)
{
    raw_ring_buffer->head = d_math_mod_pow2(raw_ring_buffer->head - 1, raw_ring_buffer->capacity);
    memcpy(raw_ring_buffer_get_head(raw_ring_buffer), elem, raw_ring_buffer->elem_size);
}

static void push_back(RawRingBuffer *raw_ring_buffer, const void *elem)
{
    memcpy(raw_ring_buffer_get_tail(raw_ring_buffer), elem, raw_ring_buffer->elem_size);
    raw_ring_buffer->tail = d_math_mod_pow2(raw_ring_buffer->tail + 1, raw_ring_buffer->capacity);
}

static DResult push(RawRingBuffer *raw_ring_buffer, const void *elem, PushFn push_fn)
{
    if (raw_ring_buffer == NULL || elem == NULL)
        return D_ERR_INVALID_ARG;
    DResult op_result = increase_buffer_capacity_if_needed(raw_ring_buffer);
    if (op_result != D_OK)
        return op_result;
    push_fn(raw_ring_buffer, elem);
    raw_ring_buffer->size++;
    return D_OK;
}

DResult raw_ring_buffer_push_front(RawRingBuffer *raw_ring_buffer, const void *elem)
{
    return push(raw_ring_buffer, elem, push_front);
}

DResult raw_ring_buffer_push_back(RawRingBuffer *raw_ring_buffer, const void *elem)
{
    return push(raw_ring_buffer, elem, push_back);
}

static void pop_front(RawRingBuffer *raw_ring_buffer, void *out_elem)
{
    memcpy(out_elem, raw_ring_buffer_get_head(raw_ring_buffer), raw_ring_buffer->elem_size);
    raw_ring_buffer->head = d_math_mod_pow2(raw_ring_buffer->head + 1, raw_ring_buffer->capacity);
}

static void pop_back(RawRingBuffer *raw_ring_buffer, void *out_elem)
{
    raw_ring_buffer->tail = d_math_mod_pow2(raw_ring_buffer->tail - 1, raw_ring_buffer->capacity);
    memcpy(out_elem, raw_ring_buffer_get_tail(raw_ring_buffer), raw_ring_buffer->elem_size);
}

static DResult pop(RawRingBuffer *raw_ring_buffer, void *out_elem, PopFn pop_fn)
{
    if (raw_ring_buffer == NULL || raw_ring_buffer->size == 0 || out_elem == NULL)
        return D_ERR_INVALID_ARG;
    pop_fn(raw_ring_buffer, out_elem);
    raw_ring_buffer->size--;
    return D_OK;
}

DResult raw_ring_buffer_pop_front(RawRingBuffer *raw_ring_buffer, void *out_elem)
{
    return pop(raw_ring_buffer, out_elem, pop_front);
}

DResult raw_ring_buffer_pop_back(RawRingBuffer *raw_ring_buffer, void *out_elem)
{
    return pop(raw_ring_buffer, out_elem, pop_back);
}

DResult raw_ring_buffer_is_empty(const RawRingBuffer *raw_ring_buffer, bool *is_empty)
{
    if (raw_ring_buffer == NULL || is_empty == NULL)
        return D_ERR_INVALID_ARG;
    *is_empty = raw_ring_buffer->size == 0;
    return D_OK;
}

void raw_ring_buffer_clear(RawRingBuffer *raw_ring_buffer)
{
    raw_ring_buffer->size = 0;
    raw_ring_buffer->head = 0;
    raw_ring_buffer->tail = 0;
}

void raw_ring_buffer_free(RawRingBuffer *raw_ring_buffer)
{
    if (raw_ring_buffer == NULL)
        return;
    free(raw_ring_buffer->data);
    memset(raw_ring_buffer, 0, sizeof(RawRingBuffer));
}