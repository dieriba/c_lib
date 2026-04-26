#ifndef CONTAINER_H
#define CONTAINER_H

#include "d_bits.h"
#include "d_types.h"

#define DEFAULT_CAPACITY 0x10

typedef enum e_container_opts
{
    CNT_OPT_NONE = 0,
    CNT_OPT_ZERO_SENTINEL = 1 << 0,
} ContainerOpts;

typedef struct _Container
{
    void *data;
    usize len;
    usize capacity;
    usize elem_size;
    DBits8 opts;
} Container;

Result container_init(Container *container, usize elem_size, usize capacity, DBits8 opts);
Result container_init_with_data(Container *container, usize elem_size, void *data, usize len, DBits8 opts);
Container *container_new(usize elem_size, usize capacity, DBits8 opts);
Container *container_new_from(const Container *src);
void container_free(Container *container);
void container_destroy(Container **container);

Result container_increase_capacity_if_needed(Container *container, usize nb_elem_to_copy);

void *container_get_data(Container *container);
void *container_get_elem_at(Container *container, usize index);
void *container_get_last_elem(Container *container);
void *container_get_first_elem(Container *container);
usize container_get_len(const Container *container);
usize container_get_capacity(const Container *container);
usize container_get_elem_size(const Container *container);
DBits8 container_get_opts(const Container *container);

Result container_insert_data(Container *dst, usize dst_pos, const void *data, usize len);
Result container_resize(Container *container, usize new_len, void *filler);
Result container_remove(Container *container, usize pos, usize len_to_remove);
Result container_replace_data(Container *container, usize pos, const void *data, usize len);
Result container_replace_data_trunc(Container *container, usize pos, const void *data, usize len);
Result container_append_container(Container *dst, const Container *src);
Result container_append_data(Container *dst, const void *data, usize len);
Result container_prepend(Container *dst, const Container *src);
Result container_push(Container *container, const void *elem);
Result container_pop(Container *container, void *out_elem);
Result container_replace_container_trunc(Container *dst, const Container *src);
Result container_swap_remove(Container *container, usize index, void *out_elem);
void container_clear(Container *container);

#endif
