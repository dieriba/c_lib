#ifndef BUFFER_H
#define BUFFER_H

#include "d_bits.h"
#include "d_types.h"
#include <stdlib.h>

typedef enum e_buffer_opts
{
    RAW_BUF_OPT_NONE = 0,
    RAW_BUF_OPT_ZERO_SENTINEL = 1 << 0,
} BufferOpts;

typedef struct _Buffer
{
    void *data;
    usize size;
    usize capacity;
    usize elem_size;
    DBits8 opts;
} RawBuffer;

DResult buffer_init(RawBuffer *raw_buffer, usize elem_size, usize capacity, DBits8 opts);
DResult buffer_init_with_data(RawBuffer *raw_buffer, usize elem_size, void *data, usize len, DBits8 opts);
RawBuffer *buffer_new(usize elem_size, usize capacity, DBits8 opts);
RawBuffer *buffer_new_from(const RawBuffer *src);
void buffer_free(RawBuffer *raw_buffer);
void buffer_destroy(RawBuffer **raw_buffer);

void *buffer_get_data(RawBuffer *raw_buffer);
void *buffer_get_elem_at(RawBuffer *raw_buffer, usize index);
void *buffer_get_last_elem(RawBuffer *raw_buffer);
void *buffer_get_first_elem(RawBuffer *raw_buffer);

DResult buffer_insert_data(RawBuffer *dst, usize dst_pos, const void *data, usize len);
DResult buffer_resize(RawBuffer *raw_buffer, usize new_size, void *filler);
DResult buffer_remove(RawBuffer *raw_buffer, usize pos, usize len_to_remove);
DResult buffer_replace_data(RawBuffer *raw_buffer, usize pos, const void *data, usize len);
DResult buffer_replace_data_trunc(RawBuffer *raw_buffer, usize pos, const void *data, usize len);
DResult buffer_append_buffer(RawBuffer *dst, const RawBuffer *src);
DResult buffer_append_data(RawBuffer *dst, const void *data, usize len);
DResult buffer_prepend(RawBuffer *dst, const RawBuffer *src);
DResult buffer_push(RawBuffer *raw_buffer, const void *elem);
DResult buffer_pop(RawBuffer *raw_buffer, void *out_elem);
DResult buffer_replace_buffer_trunc(RawBuffer *dst, const RawBuffer *src);
DResult buffer_swap_remove(RawBuffer *raw_buffer, usize index, void *out_elem);
void buffer_clear(RawBuffer *raw_buffer);

#define RAW_BUFFER_GETTER(FIELD, FIELD_TYPE)                                   \
    static inline DResult buffer_get_##FIELD(const RawBuffer *raw_buffer, FIELD_TYPE *FIELD) \
    {                                                                          \
        if (raw_buffer == NULL || FIELD == NULL)                               \
            return D_ERR_INVALID_ARG;                                          \
        *FIELD = raw_buffer->FIELD;                                            \
        return D_OK;                                                           \
    }

RAW_BUFFER_GETTER(size, usize)
RAW_BUFFER_GETTER(capacity, usize)
RAW_BUFFER_GETTER(elem_size, usize)
RAW_BUFFER_GETTER(opts, DBits8)

#endif
