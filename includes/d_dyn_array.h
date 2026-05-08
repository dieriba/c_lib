#ifndef D_ARRAY_H
#define D_ARRAY_H

#include "d_types.h"
#include "raw_buffer.h"

typedef void (*DestroyElemFunc)(void *elem);

typedef struct DDynArray
{
    RawBuffer array;
    /*
     * free_func (optional):
     *
     * If non-NULL, this function is called to destroy elements before they are
     * removed from the array (e.g., during clear, destroy, or remove operations).
     *
     * IMPORTANT:
     *   The function is called with a pointer to the element *inside the array*,
     *   not the element value interpreted as a pointer.
     *
     *   In other words, the argument is the address of the element slot.
     *
     * Example:
     *
     *   If elem_size == sizeof(Person):
     *       free_func receives a `Person *`
     *
     *   If elem_size == sizeof(char *):
     *       free_func receives a `char **`
     *
     * It is the responsibility of the user to cast the pointer to the correct
     * type and destroy the element accordingly.
     */
    DestroyElemFunc free_func;
} DDynArray;

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

DResult d_dyn_array_init(DDynArray *new_dyn_array, usize elem_size, usize reserved_elem, DestroyElemFunc free_func, BufferOpts opts);
DResult d_dyn_array_init_ptr_arr(DDynArray *new_dyn_array, usize reserved_elem, DestroyElemFunc free_func, BufferOpts opts);
DResult d_dyn_array_init_from(DDynArray *new_dyn_array, DDynArray *dyn_array);

DResult d_dyn_array_get_elem_at(const DDynArray *dyn_array, usize index, void *out_elem);

DResult d_dyn_array_append(DDynArray *dyn_array, const void *data, usize nb_elem_to_copy);

DResult d_dyn_array_push_back(DDynArray *dyn_array, const void *data);
DResult d_dyn_array_push_back_ptr(DDynArray *dyn_array, const void *data);
DResult d_dyn_array_remove_elem_fast(DDynArray *dyn_array, usize index, void *out_elem);
DResult d_dyn_array_remove_last_element(DDynArray *dyn_array, void *out_elem);
DResult d_dyn_array_get_size(const DDynArray *dyn_array, usize *size);
DResult d_dyn_array_get_capacity(const DDynArray *dyn_array, usize *capacity);
DDynArray *d_dyn_array_clear_array(DDynArray *dyn_array);
void d_dyn_array_destroy(DDynArray *dyn_array);

#endif
