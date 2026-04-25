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
    void    *data;
    usize   len;
    usize   capacity;
    usize   elem_size;
    DBits8  opts;
} Container;

Result      container_init(Container *container, usize elem_size, usize capacity, DBits8 opts);
Container   *container_new(usize elem_size, usize capacity, DBits8 opts);
Container   *container_new_from(const Container *src);
void        container_destroy(Container *container);

Result      container_increase_capacity_if_needed(Container *container, usize nb_elem_to_copy);

void        *container_get_data(Container *container);
usize       container_get_len(const Container *container);
usize       container_get_capacity(const Container *container);
usize       container_get_elem_size(const Container *container);
DBits8      container_get_opts(const Container *container);

Result      container_insert(Container *dst, usize dst_pos, const void *data, usize len);
Result      container_append(Container *dst, const Container *src);
Result      container_prepend(Container *dst, const Container *src);
Result      container_push(Container *container, const void *elem);
Result      container_pop(Container *container, void *out_elem);
Result      container_swap_remove(Container *container, usize index, void *out_elem);
void        container_clear(Container *container);

#endif
