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
} Buffer;

Result buffer_init(Buffer *buffer, usize elem_size, usize capacity, DBits8 opts);
Result buffer_init_with_data(Buffer *buffer, usize elem_size, void *data, usize len, DBits8 opts);
Buffer *buffer_new(usize elem_size, usize capacity, DBits8 opts);
Buffer *buffer_new_from(const Buffer *src);
void buffer_free(Buffer *buffer);
void buffer_destroy(Buffer **buffer);

Result buffer_increase_capacity_if_needed(Buffer *buffer, usize nb_elem_to_copy);

void *buffer_get_data(Buffer *buffer);
void *buffer_get_elem_at(Buffer *buffer, usize index);
void *buffer_get_last_elem(Buffer *buffer);
void *buffer_get_first_elem(Buffer *buffer);
usize buffer_get_len(const Buffer *buffer);
usize buffer_get_capacity(const Buffer *buffer);
usize buffer_get_elem_size(const Buffer *buffer);
DBits8 buffer_get_opts(const Buffer *buffer);

Result buffer_insert_data(Buffer *dst, usize dst_pos, const void *data, usize len);
Result buffer_resize(Buffer *buffer, usize new_len, void *filler);
Result buffer_remove(Buffer *buffer, usize pos, usize len_to_remove);
Result buffer_replace_data(Buffer *buffer, usize pos, const void *data, usize len);
Result buffer_replace_data_trunc(Buffer *buffer, usize pos, const void *data, usize len);
Result buffer_append_buffer(Buffer *dst, const Buffer *src);
Result buffer_append_data(Buffer *dst, const void *data, usize len);
Result buffer_prepend(Buffer *dst, const Buffer *src);
Result buffer_push(Buffer *buffer, const void *elem);
Result buffer_pop(Buffer *buffer, void *out_elem);
Result buffer_replace_buffer_trunc(Buffer *dst, const Buffer *src);
Result buffer_swap_remove(Buffer *buffer, usize index, void *out_elem);
void buffer_clear(Buffer *buffer);

#endif
