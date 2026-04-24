#ifndef __CONTAINER__H
#define __CONTAINER__H

#include "d_types.h"
#include "d_bits.h"

#define DEFAULT_CAPACITY 0x10

typedef struct Container
{
    void *data;
    usize len;
    usize capacity;
    usize elem_size;
} Container;

/**
 * @brief Ensures that `container` has enough capacity for `nb_elem_to_copy` more elements.
 *
 * If `zero_terminated` is true, the capacity check also reserves room for one extra
 * zero element after the logical end of the container.
 *
 * @param container Container to grow.
 * @param nb_elem_to_copy Number of additional elements that need to fit.
 * @param zero_terminated Whether one extra zero element slot must be available.
 *
 * @return OK if the current capacity is already sufficient.
 * @return OK if the internal allocation was successfully grown.
 * @return ERROR if `container == NULL`.
 * @return ERROR if the requested size would overflow.
 * @return ERROR if allocation fails.
 */
Result container_increase_capacity_if_needed(Container *container, usize nb_elem_to_copy, bool zero_terminated);

/**
 * @brief Creates a new container by shallow-copying the contents of `src`.
 *
 * The container structure and backing storage are newly allocated, but element bytes
 * are copied as-is. Owned resources inside elements are not deep-copied.
 *
 * If `zero_terminated` is true, the copied byte range includes one extra element slot
 * after `src->len`.
 *
 * @param src Source container to copy.
 * @param zero_terminated Whether to copy the extra zero-terminated slot.
 *
 * @return Newly allocated container on success.
 * @return NULL if `src == NULL`.
 * @return NULL if allocation of the container or backing storage fails.
 */
Container *container_new_from(Container *src, bool zero_terminated);

/**
 * @brief Inserts raw data into `dst` at element position `dst_pos`.
 *
 * Existing elements at and after `dst_pos` are moved forward. If `zero_terminated`
 * is true, one zeroed element is written after the new logical end.
 *
 * @param dst Destination container.
 * @param dst_pos Element index where insertion starts. Must be <= `dst->len`.
 * @param data Data to insert.
 * @param len Number of elements to insert.
 * @param zero_terminated Whether to keep one zeroed element after the logical end.
 *
 * @return OK on success.
 * @return ERROR if `dst == NULL` or `data == NULL`.
 * @return ERROR if `dst_pos > dst->len`.
 * @return ERROR if growing the destination storage fails.
 */
Result container_insert(Container *dst, usize dst_pos, const void *data, usize len, bool zero_terminated);

/**
 * @brief Appends all elements from `src` to the end of `dst`.
 *
 * Element bytes are copied as-is from `src->data` into `dst`.
 *
 * @param dst Destination container.
 * @param src Source container.
 * @param zero_terminated Whether to keep one zeroed element after the logical end.
 *
 * @return OK on success.
 * @return ERROR if `dst == NULL` or `src == NULL`.
 * @return ERROR if insertion/growth fails.
 */
Result container_append(Container *dst, Container *src, bool zero_terminated);

/**
 * @brief Prepends all elements from `src` to the beginning of `dst`.
 *
 * Element bytes are copied as-is from `src->data` into `dst`.
 *
 * @param dst Destination container.
 * @param src Source container.
 * @param zero_terminated Whether to keep one zeroed element after the logical end.
 *
 * @return OK on success.
 * @return ERROR if `dst == NULL` or `src == NULL`.
 * @return ERROR if insertion/growth fails.
 */
Result container_prepend(Container *dst, Container *src, bool zero_terminated);

#endif
