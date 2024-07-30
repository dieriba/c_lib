#ifndef __D_ARRAY_H
#define __D_ARRAY_H

#include <dtypes.h>

typedef struct _DArray			DArray;
typedef struct _DPointerArray	DPointerArray;

typedef void(*DestroyElemFunc)(void*);

/**
 * DArray:
 * @param data a pointer to the element data. The data may be moved as
 *     elements are added to the #GArray.
 * @param len  the number of elements in the #GArray not including the
 *     possible terminating zero element.
 *
 * Contains the public fields of a DArray.
 * The element data will not reference the data that was used to create the element,
 * and thus deallocate the Darray will not affect the data used in the first to create the array's element.
 */

struct _DArray {
	void*	data;
	usize   	len;
};

/**
 * DPointerArray:
 * @pdata: points to the array of pointers, which may be moved when the
 *     array grows
 * @param len  number of pointers in the array
 *
 * Contains the public fields of a pointer array.
 * Other variable may also point to the data which the pointer's array points to,
 * so be carefully when deallocating The Dpointer Array as double free error could occurs.
 */
struct _DPointerArray {
	void**	pdata;
	usize   	len;
};

/**
 * @brief Shrinks the capacity of the dynamic array (DArray) to fit its current length.
 * @param a a #DArray
 *
 * This macro reduces the array's capacity to match the number of elements it currently
 * holds, potentially freeing unused memory.
 *
 * This is a macro that internally calls `d_array_modify_capacity` for convenience.
 *
 * @return the #DArray with its capacity reduced to fit its length.
 */
#define d_array_shrink_to_fit(a) d_array_modify_capacity(a, a->len) 

/**
 * @brief Appends a single value to the end of a dynamic array.
 *
 * This macro provides a convenient way to append a single value `v` to the end of a `DArray` `a`. It
 * internally calls the `d_array_append_vals` function with the value to be added and a count of 1.
 * This simplifies the syntax for adding a single value to the dynamic array.
 *
 * @param a A pointer to the `DArray` to which the value will be appended.
 * @param v The value to be appended to the dynamic array.
 */
#define d_array_push_back(a, v)	d_array_append_vals((a), &(v), 1)

/**
 * @brief Retrieves a value from a dynamic array at a specified index.
 *
 * This macro accesses a value from the `DArray` `a` at the given index `i`. It casts the array's internal
 * data to the specified `data_type` and returns the value at the index. This provides a convenient way to
 * access elements in a typed dynamic array. The macro assumes that the `data_type` matches the type of elements
 * stored in the array.
 *
 * @param a A pointer to the `DArray` from which to retrieve the value.
 * @param data_type The type of the elements stored in the array.
 * @param i The index of the value to be retrieved.
 *
 * @return The value at the specified index `i` in the `DArray`.
 */
#define d_array_get_val_by_index(a,data_type,i)      (((data_type*) (void *) (a)->data) [(i)])


/**
 * @brief Creates a new dynamic array with specified element size and initial capacity.
 *
 * Allocates and initializes a new `DArray` structure with the specified size for each element and an initial capacity
 * for holding a certain number of elements. If `clear` is true, the allocated memory for elements is zeroed out. 
 * The `elem_size` parameter specifies the size of each element in bytes. If `reserved_elem` is greater than 0, it 
 * specifies the initial number of elements the array can hold before needing to resize. If `reserved_elem` is 0, the
 * default capacity is used instead. The function returns a pointer to the newly created `DArray`.
 *
 * @param clear A boolean value indicating whether to zero out the allocated memory for elements. If true,
 *              the memory is cleared; otherwise, it is left uninitialized.
 * @param elem_size The size of each element in the dynamic array, in bytes.
 * @param reserved_elem The initial number of elements that the array is reserved to hold. If set to 0, the
 *                      array will use a default initial capacity.
 *
 * @return DArray* A pointer to the newly created `DArray`. Returns NULL if the allocation fails.
 */
DArray  *d_array_new				(bool	clear,		usize elem_size, usize reserved_elem);

/**
 * @brief Creates a copy of an existing dynamic array.
 *
 * Allocates and initializes a new `DArray` that is a copy of the given `DArray`. The new `DArray` will have the
 * same element size and capacity as the original array, and will contain copies of all elements from the original.
 * This function performs a deep copy of the array's contents, meaning that the elements themselves are duplicated,
 * not just the references to them. The original `DArray` remains unchanged.
 *
 * @param array A pointer to the `DArray` to be copied. Must not be NULL.
 *
 * @return DArray* A pointer to the newly created `DArray` that is a copy of the original. Returns NULL if
 *         the allocation fails or if the input `array` is NULL.
 */
DArray  *d_array_copy				(DArray* array);

/**
 * @brief Retrieves the current capacity of the dynamic array (DArray).
 *
 * The d_array_get_capacity function returns the current capacity of the dynamic array.
 * The capacity represents the maximum number of elements that the array can hold without
 * needing to be resized.
 *
 * @param arrayA pointer to the DArray whose capacity is to be retrieved.
 * @return The current capacity of the DArray as an unsigned integer (usize).
 */
usize	d_array_get_capacity		(DArray* array);

/**
 * @brief Modifies the capacity of a dynamic array (DArray) to the specified new_capacity.
 * @param array a pointer to the #DArray that needs its capacity modified.
 * @param new_capacity the new desired capacity for the #DArray. This can be greater
 *                or smaller than the current capacity.
 *
 * This function adjusts the array's storage capacity, which can be used to either
 * increase or decrease the size of the array.
 *
 * @return a pointer to the updated #DArray with the modified capacity. If the
 *          reallocation fails, it typically returns NULL.
 */
DArray	*d_array_modify_capacity	(DArray* array, usize new_capacity);

/**
 * @brief Appends a block of values to the end of a dynamic array.
 *
 * Adds a specified number of elements from the provided data block to the end of the given `DArray`. 
 * The `data` pointer points to the block of values to be appended, and `len` specifies the number of elements to add.
 * If necessary, the function will resize the array to accommodate the new elements. This operation may involve copying
 * the elements from the provided data to the internal storage of the `DArray`.
 *
 * @param array A pointer to the `DArray` to which the values will be appended. Must not be NULL.
 * @param data A pointer to the block of values to be appended. Must not be NULL.
 * @param len The number of elements to append from the data block. Should be greater than 0.
 *
 * @return DArray* A pointer to the updated `DArray` with the new values appended. Returns NULL if the operation fails.
 */
DArray  *d_array_append_vals		(DArray *array, 	const void *data,		usize len);

/**
 * @brief Removes the last element from the dynamic array (DArray).
 * @array: a pointer to the `#DArray` from which an element will be removed.
 *
 * Removes the last element from the dynamic array (DArray). If the array is empty,
 * this function does nothing.
 *
 * @return a pointer to the updated `#DArray`.
 */
DArray  *d_array_pop_back		(DArray *array);

/**
 * @brief Removes an element from a dynamic array at a specified index quickly.
 *
 * Removes the element located at the specified index `index` from the `DArray` `array`. This operation is performed
 * quickly by swapping the element to be removed with the last element of the array and then reducing the array size.
 * This approach is efficient in terms of performance but may disrupt the order of the remaining elements. This function
 * does not shrink the allocated memory for the array; it only updates the logical size of the array.
 *
 * @param array A pointer to the `DArray` from which the element will be removed. Must not be NULL.
 * @param index The index of the element to be removed. Must be within the bounds of the array (i.e., 0 <= index < array size).
 *
 * @return DArray* A pointer to the updated `DArray` with the element removed. Returns NULL if the operation fails
 *         or if the `array` is NULL or `index` is out of bounds.
 */
DArray  *d_array_remove_index_fast  (DArray	*array, 	usize	index);

/**
 * @brief Clears the contents of a dynamic array by resetting its length.
 *
 * Resets the logical size (length) of the given `DArray` `array` to zero, effectively clearing all elements from
 * the array. The function does not deallocate or resize the allocated memory; instead, it leaves the data buffer
 * intact. Future operations that append data will start from the beginning of the buffer, overwriting the cleared data.
 * This method is efficient for reusing the array without reallocating memory.
 *
 * @param array A pointer to the `DArray` to be cleared. Must not be NULL.
 *
 * @return DArray* A pointer to the updated `DArray` with its length reset to zero. Returns NULL if the operation fails
 *         or if the `array` is NULL.
 */
DArray  *d_array_clear_array		(DArray* array);

/**
 * @brief Frees the memory allocated for a dynamic array and its internal resources.
 *
 * Deallocates the memory used by the given `DArray` and its internal data buffer. This function will also set
 * the pointer to the `DArray` to NULL to prevent dangling pointers. It is essential to call this function
 * when the `DArray` is no longer needed to avoid memory leaks. The function handles both the destruction of
 * the `DArray` structure and the memory used for its internal data.
 *
 * @param array A pointer to a pointer to the `DArray` to be destroyed. The function will set the original pointer
 *              to NULL after deallocation. Must not be NULL.
 */
void    d_array_destroy				(DArray** array);

/*-------------------------------------------------DPointerArray-------------------------------------------------*/

/**
 * @brief Creates a new dynamic pointer array with specified initial capacity.
 *
 * Allocates and initializes a new `DPointerArray` with an initial capacity for holding a certain number of pointers.
 * The `reserved_elem` parameter specifies the initial number of elements the array can hold before needing to resize.
 * If `null_terminated` is true, the array is terminated with a NULL pointer. This can be useful for functions that
 * expect a NULL-terminated list of pointers. If `reserved_elem` is 0, the array will use a default initial capacity.
 *
 * @param reserved_elem The initial number of pointers that the array is reserved to hold. If set to 0, the array will
 *                      use a default initial capacity.
 * @param null_terminated A boolean flag indicating whether the array should be NULL-terminated. If true, a NULL pointer
 *                        will be added as the last element in the array.
 *
 * @param free_func  The free function that will be called on each element when destroying the d_array, the free_func
 * 					 must be declared as such `void fn_name(void*)`.
 * 
 * @return DPointerArray* A pointer to the newly created `DPointerArray`. Returns NULL if the allocation fails.
 */
DPointerArray*	d_pointer_array_new					(usize reserved_elem, bool null_terminated, DestroyElemFunc free_func);

/**
 * @brief Retrieves the current capacity of a dynamic pointer array.
 *
 * Returns the number of pointers that the given `DPointerArray` `array` is currently able to hold. The capacity indicates
 * the size of the internal buffer allocated for storing pointers, but does not necessarily reflect the number of pointers
 * currently in use (i.e., the logical size of the array). This function provides information about the allocated space
 * in the array, which can be useful for determining if resizing is needed before performing operations that add new pointers.
 *
 * @param array A pointer to the `DPointerArray` whose capacity is to be retrieved. Must not be NULL.
 *
 * @return usize The current capacity of the `DPointerArray`, representing the number of pointers it can hold.
 *               Returns 0 if the `array` is NULL.
 */
usize 			d_pointer_array_get_capacity				(DPointerArray* array);

/**
 * @brief mofify the capacity of a dynamic pointer array.
 *
 * Resizes the given `DPointerArray` `array` to accommodate at least `new_capacity` pointers. This function allocates
 * additional memory if necessary to meet the specified capacity. If `new_capacity` is greater than the current capacity,
 * the array will be resized accordingly. The function does not alter the logical size of the array; it only affects
 * the amount of memory reserved for future pointer additions.
 *
 * @param array A pointer to the `DPointerArray` whose capacity is to be increased. Must not be NULL.
 * @param new_capacity The new capacity for the array, specifying the minimum number of pointers the array should be able
 *                     to hold. Should be greater than the current capacity.
 *
 * @return DPointerArray* A pointer to the updated `DPointerArray` with increased capacity. Returns NULL if the operation
 *         fails, if the `array` is NULL, or if `new_capacity` is less than or equal to the current capacity.
 */
DPointerArray*	d_pointer_array_modify_capacity	(DPointerArray* array, usize new_capacity);

/**
 * @brief Appends a block of pointers to the end of a dynamic pointer array.
 *
 * Adds a specified number of pointers from the provided data block to the end of the given `DPointerArray`. 
 * The `data` pointer points to the block of pointers to be appended, and `len` specifies the number of pointers to add.
 * If necessary, the function will resize the array to accommodate the new pointers. This operation may involve copying
 * the pointers from the provided data to the internal storage of the `DPointerArray`.
 *
 * @param array A pointer to the `DPointerArray` to which the pointers will be appended. Must not be NULL.
 * @param data A pointer to the block of pointers to be appended. Must not be NULL.
 * @param len The number of pointers to append from the data block. Should be greater than 0.
 *
 * @return DPointerArray* A pointer to the updated `DPointerArray` with the new pointers appended. Returns NULL if
 *         the operation fails or if the `array` or `data` is NULL.
 */
DPointerArray  *d_pointer_array_append_vals		(DPointerArray *array, 	const void **data,	usize len);

/**
 * @brief Appends a single pointer to the end of a dynamic pointer array.
 *
 * Adds a single pointer from the provided data to the end of the given `DPointerArray`. The function will handle
 * resizing the array if necessary to accommodate the new pointer. This operation involves copying the pointer from
 * the provided data to the internal storage of the `DPointerArray`.
 *
 * @param array A pointer to the `DPointerArray` to which the pointer will be appended. Must not be NULL.
 * @param data A pointer to the data to be appended. This should be a single pointer to be added to the array. Must not be NULL.
 *
 * @return DPointerArray* A pointer to the updated `DPointerArray` with the new pointer appended. Returns NULL if
 *         the operation fails or if the `array` or `data` is NULL.
 */
DPointerArray  *d_pointer_array_push_back		(DPointerArray *array, 	const void *data);

/**
 * @brief Removes a pointer from a dynamic pointer array at a specified index quickly.
 *
 * Removes the pointer located at the specified index `index` from the `DPointerArray` `array`. This operation
 * is performed quickly by swapping the pointer to be removed with the last pointer of the array and then reducing
 * the logical size of the array. This approach is efficient but may disrupt the order of the remaining pointers.
 * The function does not shrink the allocated memory for the array; it only updates the logical size of the array.
 *
 * @param array A pointer to the `DPointerArray` from which the pointer will be removed. Must not be NULL.
 * @param index The index of the pointer to be removed. Must be within the bounds of the array (i.e., 0 <= index < array size).
 *
 * @return DPointerArray* A pointer to the updated `DPointerArray` with the pointer removed. Returns NULL if the
 *         operation fails or if the `array` is NULL or `index` is out of bounds.
 */
DPointerArray  *d_pointer_array_remove_index_fast  (DPointerArray	*array, 	usize	index);

/**
 * @brief Clears all pointers from a dynamic pointer array while retaining allocated space.
 *
 * Removes all pointers from the given `DPointerArray` `array` by resetting its logical size to zero. The function clears
 * the internal data, but does not deallocate or resize the allocated memory; the space reserved for the pointers remains
 * intact. This allows the array to be reused for new pointers without needing to reallocate memory. The function prepares
 * the array for new data by starting fresh from the beginning of the buffer.
 *
 * @param array A pointer to the `DPointerArray` to be cleared. Must not be NULL.
 *
 * @return DPointerArray* A pointer to the updated `DPointerArray` with all pointers removed. Returns NULL if the
 *         operation fails or if the `array` is NULL.
 */
DPointerArray  *d_pointer_array_clear_array		(DPointerArray* array);

/**
 * @brief Frees the memory allocated for a dynamic pointer array and its internal resources.
 *
 * Deallocates the memory used by the given `DPointerArray` and its internal data buffer. The function also sets
 * the pointer to the `DPointerArray` to NULL to avoid dangling pointers. This operation ensures that no memory leaks
 * occur by properly cleaning up the allocated resources. The function handles both the destruction of the `DPointerArray`
 * structure and the memory used for its internal data.
 *
 * @param array A pointer to a pointer to the `DPointerArray` to be destroyed. The function will set the original pointer
 *              to NULL after deallocation. Must not be NULL.
 */
void    d_pointer_array_destroy		(DPointerArray** array);

#endif