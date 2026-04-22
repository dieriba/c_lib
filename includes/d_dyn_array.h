#ifndef __D_ARRAY_H
#define __D_ARRAY_H

#include "d_types.h"

typedef struct _DynArray DynArray;

/*
 * DestroyElemFunc:
 *
 * Called to destroy a single element stored in the array.
 *
 * IMPORTANT:
 *   The function receives a pointer to the element slot inside the array,
 *   NOT the element value interpreted as a pointer.
 *
 * Examples:
 *   - if elem_size == sizeof(Person), the callback receives a Person *
 *   - if elem_size == sizeof(char *), the callback receives a char **
 *
 * It is the caller's responsibility to cast and destroy correctly.
 */
typedef void (*DestroyElemFunc)(void *elem);

/*
 * DynArrayOpts:
 *
 * Bitmask of options that can be combined with '|'.
 *
 * Example:
 *   opts = D_DYN_ARRAY_OPT_CLEAR | D_DYN_ARRAY_OPT_ZERO_TERMINATED;
 */
typedef enum DynArrayOpts
{
    D_DYN_ARRAY_OPT_ZERO_TERMINATED = 1 << 1
} DynArrayOpts;

/**
 * @brief Creates a new dynamic array.
 *
 * The array stores elements inline in a contiguous buffer.
 * Each element occupies `elem_size` bytes and is copied into storage.
 *
 * If `free_func` is non-NULL, it is called on each element before it is
 * removed (clear, destroy, remove).
 *
 * @param elem_size Size of one element (must be > 0)
 * @param reserved_elem Initial capacity (0 = default)
 * @param free_func Optional destructor (may be NULL)
 * @param clear If true, allocated memory is zero-initialized
 * @param opts Bitmask of options:
 *        - D_DYN_ARRAY_OPT_ZERO_TERMINATED:
 *            Maintain one extra zeroed element at index [len]
 *
 * @return New array or NULL on failure
 */
DynArray *d_dyn_array_new(
    usize elem_size,
    usize reserved_elem,
    DestroyElemFunc free_func,
    bool clear,
    DynArrayOpts opts);

/**
 * @brief Performs a shallow copy of the array.
 *
 * Copies element bytes as-is into a new array.
 *
 * WARNING:
 *   If elements own resources, the original and the copy will share them.
 *   Destroying both arrays may lead to double free.
 *
 * @param array Source array
 * @return New array or NULL on failure
 */
DynArray *d_dyn_array_shallow_copy(DynArray *array);

/**
 * @brief Returns current capacity.
 */
usize d_dyn_array_get_capacity(DynArray *array);

/**
 * @brief Ensures capacity is at least new_capacity.
 *
 * May reallocate and invalidate element pointers.
 */
DynArray *d_dyn_array_modify_capacity(DynArray *array, usize new_capacity);

/**
 * @brief Appends `len` elements from `data`.
 *
 * Elements are copied using memcpy.
 *
 * NOTE:
 *   Elements must be safely movable via memcpy.
 *
 * @return Updated array or NULL on failure
 */
DynArray *d_dyn_array_append(
    DynArray *array,
    const void *data,
    usize len);

/**
 * @brief Removes last element.
 *
 * Calls destructor if provided.
 */
DynArray *d_dyn_array_remove_last_element(DynArray *array);

/**
 * @brief Removes element at index (unordered).
 *
 * Behavior:
 *   - destroy element at index
 *   - move last element into its place
 *   - decrement length
 *
 * Order is NOT preserved.
 *
 * @return Updated array or NULL if index invalid
 */
DynArray *d_dyn_array_remove_elem_fast(DynArray *array, usize index);

/**
 * @brief Clears all elements.
 *
 * Calls destructor on all elements if provided.
 * Capacity is unchanged.
 */
DynArray *d_dyn_array_clear_array(DynArray *array);

/**
 * @brief Destroys the array.
 *
 * Calls destructor on all elements if provided,
 * frees storage, and sets *array to NULL.
 */
void d_dyn_array_destroy(DynArray **array);

/**
 * @brief Returns pointer to element at index.
 *
 * WARNING:
 *   Pointer is invalidated after any modification that may reallocate
 *   or move elements (append, remove, clear, etc).
 */
void *d_dyn_array_get_elem(DynArray *array, usize index);

#endif