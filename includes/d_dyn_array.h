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

/**
 * @brief Creates a new dynamic array.
 *
 * The array stores elements inline in a contiguous buffer.
 * Each element occupies `elem_size` bytes and is copied into storage.
 *
 * If `free_func` is non-NULL, it is called on each element before it is
 * removed (clear, destroy, remove).
 *
 * If `D_DYN_ARRAY_OPT_ZERO_TERMINATED` is set, one extra zeroed element
 * slot is kept after the logical end of the array.
 *
 * @param elem_size Size in bytes of one element. Must be > 0.
 * @param reserved_elem Initial capacity in number of elements. If 0,
 *        a default capacity is used.
 * @param free_func Optional destructor for elements. May be NULL.
 * @param clear If true, the allocated element storage is zero-initialized.
 * @param opts Bitmask of options.
 *
 * @return A newly allocated array on success.
 * @return NULL if `elem_size == 0`.
 * @return NULL if allocation of the array object or element storage fails.
 */
DynArray *d_dyn_array_new(usize elem_size, usize reserved_elem, DestroyElemFunc free_func, bool clear);

/**
 * @brief Creates a new dynamic array for storing pointers.
 *
 * This is equivalent to `d_dyn_array_new(sizeof(void *), ...)`.
 *
 * The array stores pointer values inline in its internal buffer.
 * If `free_func` is provided, it receives the address of each stored slot,
 * so for pointer arrays it will receive a pointer to pointer.
 *
 * @param reserved_elem Initial capacity in number of pointer slots. If 0,
 *        a default capacity is used.
 * @param free_func Optional destructor for elements. May be NULL.
 * @param clear If true, the allocated element storage is zero-initialized.
 * @param opts Bitmask of options.
 *
 * @return A newly allocated pointer array on success.
 * @return NULL if allocation fails.
 */
DynArray *d_dyn_array_new_ptr_arr(usize reserved_elem, DestroyElemFunc free_func, bool clear);

/**
 * @brief Creates a shallow copy of an existing array.
 *
 * Element bytes are copied as-is into a new array.
 * Owned resources are NOT duplicated.
 *
 * WARNING:
 *   If elements own resources, the original and the copy may end up sharing
 *   them. Destroying both arrays may then cause double-free or other
 *   ownership bugs.
 *
 * @param array Source array to copy.
 *
 * @return A new array containing a byte-for-byte copy of the elements.
 * @return NULL if allocation fails while creating the copy.
 *
 * @note This function assumes the internal append operation succeeds.
 *       In the current implementation, allocation failure during append is
 *       not checked after the new array is created.
 */
DynArray *d_dyn_array_shallow_copy(DynArray *array);

/**
 * @brief Returns the current capacity of the array.
 *
 * Capacity is the number of elements that can be stored without growing
 * the internal buffer.
 *
 * @param array Array instance.
 *
 * @return Current capacity in number of elements.
 */
usize d_dyn_array_get_capacity(DynArray *array);

/**
 * @brief Ensures that the array capacity is at least `new_capacity`.
 *
 * If `new_capacity` is less than or equal to the current capacity,
 * the array is left unchanged.
 *
 * If reallocation happens, all previously obtained element pointers may
 * become invalid.
 *
 * @param array Array instance.
 * @param new_capacity Desired minimum capacity in number of elements.
 *
 * @return `array` if capacity was already sufficient.
 * @return `array` if reallocation succeeds.
 * @return NULL if reallocation fails.
 *
 * @note In the current implementation, zero-terminated extra storage is not
 *       accounted for in this function when growing manually.
 */
DynArray *d_dyn_array_modify_capacity(DynArray *array, usize new_capacity);

/**
 * @brief Appends `len` elements from `data` to the end of the array.
 *
 * The bytes are copied with `memcpy`.
 *
 * @param array Target array.
 * @param data Pointer to the first element to copy.
 * @param len Number of elements to append.
 *
 * @return `array` on success.
 * @return NULL if `len == 0`.
 * @return NULL if `data == NULL` while `len != 0`.
 * @return NULL if growing the internal storage fails.
 */
DynArray *d_dyn_array_append(DynArray *array, const void *data, usize len);

/**
 * @brief Appends one inline element to the array.
 *
 * This is equivalent to `d_dyn_array_append(array, data, 1)`.
 * The bytes of the element are copied into the internal storage.
 *
 * @param dyn_array Target array.
 * @param data Pointer to the element to append.
 *
 * @return `dyn_array` on success.
 * @return NULL if `data == NULL`.
 * @return NULL if the append operation fails.
 */
DynArray *d_dyn_push_back(DynArray *dyn_array, const void *data);

/**
 * @brief Appends one pointer value to the array.
 *
 * The pointer value itself is stored in the array, not the object it points to.
 * Internally this function appends the address of the local pointer variable,
 * so that one pointer-sized value is copied into the array.
 *
 * @param dyn_array Target array.
 * @param data Pointer value to store.
 *
 * @return `dyn_array` on success.
 * @return NULL if the append operation fails.
 *
 * @note Passing NULL as `data` stores a NULL pointer only if the underlying
 *       append implementation accepts it. In the current implementation,
 *       this succeeds because the address of the local parameter is copied.
 */
DynArray *d_dyn_push_back_ptr(DynArray *dyn_array, const void *data);

/**
 * @brief Removes the last element of the array.
 *
 * If a destructor is set, it is called on the last element before removal.
 * The removed slot is then zeroed and the logical length is decremented.
 *
 * @param array Target array.
 *
 * @return `array` on success.
 * @return NULL if the array is empty.
 */
DynArray *d_dyn_array_remove_last_element(DynArray *array);

/**
 * @brief Removes the element at `index` without preserving order.
 *
 * If a destructor is set, it is called on the element being removed.
 * If the removed element is not already the last one, the last element is
 * copied into its slot. The old last slot is then zeroed and length is
 * decremented.
 *
 * @param array Target array.
 * @param index Index of the element to remove.
 *
 * @return `array` on success.
 * @return NULL if `index` is out of bounds.
 */
DynArray *d_dyn_array_remove_elem_fast(DynArray *array, usize index);

/**
 * @brief Clears all elements from the array.
 *
 * If a destructor is set, it is called on each live element.
 * Element storage is kept allocated and capacity is unchanged.
 *
 * @param array Target array.
 *
 * @return `array`.
 *
 * @note In the current implementation this function always returns `array`
 *       and does not report failure.
 */
DynArray *d_dyn_array_clear_array(DynArray *array);

/**
 * @brief Destroys the array and sets the caller's pointer to NULL.
 *
 * If a destructor is set, it is called on each live element before the
 * storage is freed.
 *
 * @param array Address of the array pointer to destroy.
 *
 * @return Nothing.
 *
 * @note If `array == NULL` or `*array == NULL`, the function does nothing.
 */
void d_dyn_array_destroy(DynArray **array);

/**
 * @brief Returns a pointer to the element at `index`.
 *
 * The returned pointer points directly inside the array storage.
 *
 * WARNING:
 *   Any operation that reallocates the storage or moves elements may
 *   invalidate this pointer.
 *
 * @param array Target array.
 * @param index Index of the requested element.
 *
 * @return Pointer to the element at `index`.
 * @return NULL if `index` is out of bounds.
 */
void *d_dyn_array_get_elem(DynArray *array, usize index);

#endif