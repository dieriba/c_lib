#ifndef D_ARRAY_H
#define D_ARRAY_H

#include "d_types.h"
#include "raw_buffer.h"

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

DynArray *d_dyn_array_new_ptr_arr(usize reserved_elem, DestroyElemFunc free_func, BufferOpts opts);

DynArray *d_dyn_array_new_from(DynArray *dyn_array);

void d_dyn_array_destroy(DynArray **dyn_array);

void *d_dyn_array_get_elem_at(DynArray *dyn_array, usize index);

DResult d_dyn_array_append(DynArray *dyn_array, const void *data, usize nb_elem_to_copy);

DResult d_dyn_array_push_back(DynArray *dyn_array, const void *data);
DResult d_dyn_array_push_back_ptr(DynArray *dyn_array, const void *data);
DResult d_dyn_array_remove_elem_fast(DynArray *dyn_array, usize index, void *out_elem);
DResult d_dyn_array_remove_last_element(DynArray *dyn_array, void *out_elem);

DynArray *d_dyn_array_clear_array(DynArray *dyn_array);

#define D_DYN_ARRAY_GETTER(FIELD, FIELD_TYPE)                                                     \
    static inline DResult d_dyn_array_get_##FIELD(const DynArray *d_dyn_array, FIELD_TYPE *FIELD) \
    {                                                                                             \
        return buffer_get_##FIELD((RawBuffer *)d_dyn_array, FIELD);                               \
    }

D_DYN_ARRAY_GETTER(size, usize)
D_DYN_ARRAY_GETTER(capacity, usize)

#endif
