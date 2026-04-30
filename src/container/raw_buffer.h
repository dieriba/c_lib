#ifndef BUFFER_H
#define BUFFER_H

#include "d_bits.h"
#include "d_types.h"

#define DEFAULT_CAPACITY 0x10

typedef enum e_buffer_opts
{
    CNT_OPT_NONE = 0,
    CNT_OPT_ZERO_SENTINEL = 1 << 0,
} BufferOpts;

typedef struct _Buffer
{
    void *data;
    usize len;
    usize capacity;
    usize elem_size;
    DBits8 opts;
} RawBuffer;

Result buffer_init(RawBuffer *raw_buffer, usize elem_size, usize capacity, DBits8 opts);
Result buffer_init_with_data(RawBuffer *raw_buffer, usize elem_size, void *data, usize len, DBits8 opts);
RawBuffer *buffer_new(usize elem_size, usize capacity, DBits8 opts);
RawBuffer *buffer_new_from(const RawBuffer *src);
void buffer_free(RawBuffer *raw_buffer);
void buffer_destroy(RawBuffer **raw_buffer);

Result buffer_increase_capacity_if_needed(RawBuffer *raw_buffer, usize nb_elem_to_copy);

void *buffer_get_data(RawBuffer *raw_buffer);
void *buffer_get_elem_at(RawBuffer *raw_buffer, usize index);
void *buffer_get_last_elem(RawBuffer *raw_buffer);
void *buffer_get_first_elem(RawBuffer *raw_buffer);
usize buffer_get_len(const RawBuffer *raw_buffer);
usize buffer_get_capacity(const RawBuffer *raw_buffer);
usize buffer_get_elem_size(const RawBuffer *raw_buffer);
DBits8 buffer_get_opts(const RawBuffer *raw_buffer);

Result buffer_insert_data(RawBuffer *dst, usize dst_pos, const void *data, usize len);
Result buffer_resize(RawBuffer *raw_buffer, usize new_len, void *filler);
Result buffer_remove(RawBuffer *raw_buffer, usize pos, usize len_to_remove);
Result buffer_replace_data(RawBuffer *raw_buffer, usize pos, const void *data, usize len);
Result buffer_replace_data_trunc(RawBuffer *raw_buffer, usize pos, const void *data, usize len);
Result buffer_append_buffer(RawBuffer *dst, const RawBuffer *src);
Result buffer_append_data(RawBuffer *dst, const void *data, usize len);
Result buffer_prepend(RawBuffer *dst, const RawBuffer *src);
Result buffer_push(RawBuffer *raw_buffer, const void *elem);
Result buffer_pop(RawBuffer *raw_buffer, void *out_elem);
Result buffer_replace_buffer_trunc(RawBuffer *dst, const RawBuffer *src);
Result buffer_swap_remove(RawBuffer *raw_buffer, usize index, void *out_elem);
void buffer_clear(RawBuffer *raw_buffer);

#endif
