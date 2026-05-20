#ifndef RAW_BUFFER_H
#define RAW_BUFFER_H

#include "d_bits.h"
#include "d_error.h"
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
    DestroyElemFn free_fn;
    CopyElemFn copy_fn;
    DBits8 opts;
} RawBuffer;

DResult raw_buffer_init(RawBuffer *raw_buffer, usize elem_size, usize capacity, DestroyElemFn free_fn, CopyElemFn copy_fn, DBits8 opts);
DResult raw_buffer_init_with_data(RawBuffer *raw_buffer, usize elem_size, const void *data, usize size, DestroyElemFn free_fn, CopyElemFn copy_fn, DBits8 opts);
DResult raw_buffer_init_from_raw_buffer(RawBuffer *new_raw_buffer, const RawBuffer *src);
void raw_buffer_free(RawBuffer *raw_buffer);
DResult raw_buffer_clear(RawBuffer *raw_buffer);
DResult raw_buffer_copy(RawBuffer *dst, const RawBuffer *src);

void *raw_buffer_get_data(const RawBuffer *raw_buffer);
DResult raw_buffer_get_elem_at(RawBuffer *raw_buffer, usize index, void *out_elem);
DResult raw_buffer_get_elem_addr(RawBuffer *raw_buffer, usize index, void **out_elem);
DResult raw_buffer_get_last_elem(RawBuffer *raw_buffer, void *out_elem);
DResult raw_buffer_get_first_elem(RawBuffer *raw_buffer, void *out_elem);
DResult raw_buffer_insert_data_from_raw_data(RawBuffer *dst, usize dst_pos, const void *data, usize size);
DResult raw_buffer_resize(RawBuffer *raw_buffer, usize new_size, void *filler);
DResult raw_buffer_remove(RawBuffer *raw_buffer, usize pos, usize len_to_remove);
DResult raw_buffer_replace_with(RawBuffer *dst, const RawBuffer *src, usize pos);
DResult raw_buffer_replace_data_at(RawBuffer *raw_buffer, usize pos, const void *data, usize size);
DResult raw_buffer_append_buffer(RawBuffer *dst, const RawBuffer *src);
DResult raw_buffer_append_data(RawBuffer *dst, const void *data, usize size);
DResult raw_buffer_prepend(RawBuffer *dst, const RawBuffer *src);
DResult raw_buffer_push(RawBuffer *raw_buffer, const void *elem);
DResult raw_buffer_pop(RawBuffer *raw_buffer, void *out_elem);
DResult raw_buffer_swap_remove(RawBuffer *raw_buffer, usize index, void *out_elem);

#define RAW_BUFFER_GETTER(FIELD, FIELD_TYPE)                                                     \
    static inline DResult raw_buffer_get_##FIELD(const RawBuffer *raw_buffer, FIELD_TYPE *FIELD) \
    {                                                                                            \
        if (raw_buffer == NULL || FIELD == NULL)                                                 \
            return D_ERR_INVALID_ARG;                                                            \
        *FIELD = raw_buffer->FIELD;                                                              \
        return D_OK;                                                                             \
    }

RAW_BUFFER_GETTER(size, usize)
RAW_BUFFER_GETTER(capacity, usize)
RAW_BUFFER_GETTER(elem_size, usize)
RAW_BUFFER_GETTER(opts, DBits8)

#endif
