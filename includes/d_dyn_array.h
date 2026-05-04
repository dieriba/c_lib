#ifndef D_ARRAY_H
#define D_ARRAY_H

#include "d_types.h"
#include "raw_buffer.h"

typedef struct DDynArray DDynArray;

/*
 * DestroyElemFunc:
 *
 * Called to destroy a single element stored in the array.
 * The function receives a pointer to the element slot inside the array,
 * not the element value interpreted as a pointer.
 *
 * Examples:
 *   - if elem_size == sizeof(Person), the callback receives a Person *
 *   - if elem_size == sizeof(char *), the callback receives a char **
 */
typedef void (*DestroyElemFunc)(void *elem);

DResult d_dyn_array_new(DDynArray **new_dyn_array, usize elem_size, usize reserved_elem, DestroyElemFunc free_func, BufferOpts opts);
DResult d_dyn_array_new_ptr_arr(DDynArray **new_dyn_array, usize reserved_elem, DestroyElemFunc free_func, BufferOpts opts);
DResult d_dyn_array_new_from(DDynArray **new_dyn_array, DDynArray *dyn_array);

void d_dyn_array_destroy(DDynArray **dyn_array);

void *d_dyn_array_get_elem_at(DDynArray *dyn_array, usize index);

DResult d_dyn_array_append(DDynArray *dyn_array, const void *data, usize nb_elem_to_copy);

DResult d_dyn_array_push_back(DDynArray *dyn_array, const void *data);
DResult d_dyn_array_push_back_ptr(DDynArray *dyn_array, const void *data);
DResult d_dyn_array_remove_elem_fast(DDynArray *dyn_array, usize index, void *out_elem);
DResult d_dyn_array_remove_last_element(DDynArray *dyn_array, void *out_elem);
DResult d_dyn_array_get_size(DDynArray *dyn_array, usize *size);
DResult d_dyn_array_get_capacity(DDynArray *dyn_array, usize *capacity);
DDynArray *d_dyn_array_clear_array(DDynArray *dyn_array);

#endif
