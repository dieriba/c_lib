#ifndef __D_ARRAY_H
#define __D_ARRAY_H

#include "d_types.h"
#include "buffer.h"

typedef struct _DynArray DynArray;

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

DynArray *d_dyn_array_new(usize elem_size, usize reserved_elem,
                          DestroyElemFunc free_func, BufferOpts opts);

DynArray *d_dyn_array_new_ptr_arr(usize reserved_elem,
                                  DestroyElemFunc free_func,
                                  BufferOpts opts);

DynArray *d_dyn_array_new_from(DynArray *dyn_array);

void d_dyn_array_destroy(DynArray **dyn_array);

usize d_dyn_array_get_capacity(DynArray *dyn_array);
void *d_dyn_array_get_elem_at(DynArray *dyn_array, usize index);

DynArray *d_dyn_array_append(DynArray *dyn_array, const void *data,
                             usize nb_elem_to_copy);

DynArray *d_dyn_push_back(DynArray *dyn_array, const void *data);
DynArray *d_dyn_push_back_ptr(DynArray *dyn_array, const void *data);

DynArray *d_dyn_array_remove_elem_fast(DynArray *dyn_array, usize index,
                                       void *out_elem);

DynArray *d_dyn_array_remove_last_element(DynArray *dyn_array,
                                          void *out_elem);

DynArray *d_dyn_array_clear_array(DynArray *dyn_array);

#endif
