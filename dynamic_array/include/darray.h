#ifndef __D_ARRAY_H
#define __D_ARRAY_H

#include <dtypes.h>

typedef struct _DArray			DArray;
typedef struct _DPointerArray	DPointerArray;

/**
 * DArray:
 * @data: a pointer to the element data. The data may be moved as
 *     elements are added to the #GArray.
 * @len: the number of elements in the #GArray not including the
 *     possible terminating zero element.
 *
 * Contains the public fields of a DArray.
 * The element data will not reference the data that was used to create the element,
 * and thus deallocate the Darray will not affect the data used in the first to create the array's element.
 */
struct _DArray {
	void*	data;
	u64   	len;
};

/**
 * DPointerArray:
 * @pdata: points to the array of pointers, which may be moved when the
 *     array grows
 * @len: number of pointers in the array
 *
 * Contains the public fields of a pointer array.
 * Other variable may also point to the data which the pointer's array points to,
 * so be carefully when deallocating The Dpointer Array as double free error could occurs.
 */
struct _DPointerArray {
	void**	pdata;
	u64   	len;
};

/**
 * d_array_push_back:
 * @a: a #DArray
 * @v: the value to append to the #DArray
 *
 * Adds the value on to the end of the array. The array will grow in
 * size automatically if necessary.
 *
 *
 * Returns: the #DArray
 */
#define d_array_push_back(a,v)	  d_array_append_vals (a, v, 1)

/**
 * d_array_get_val_by_index:
 * @a: a #DArray
 * @data_type: data_type of array's element will be used to cast out the void*
 * @v: the value to append to the #DArray
 *
 * Retrieve the value of an element by its given
 * 
 * Returns: the #DArray
 */
#define d_array_get_val_by_index(a,data_type,i)      (((data_type*) (void *) (a)->data) [(i)])
/**
 * d_array_new:
 * @clear: %TRUE if #DArray elements should be automatically cleared
 *     to 0 when they are allocated
 * @element_size: the size of each element in bytes
 *
 * Creates a new #DArray.
 *
 * Returns: the new #DArray
 */
DArray  *d_array_new				(bool	clear,		u64 elem_size, u64 reserved_elem);

/**
 * d_array_append_vals:
 * @array: a #DArray
 * @data: (not nullable): a pointer to the elements to append to the end of the array
 * @len: the number of elements to append
 *
 * Adds @len elements onto the end of the array.
 *
 * Returns: the #DArray
 */
/**
 * d_array_push_back:
 * @a: a #DArray
 * @v: the value to append to the #DArray
 *
 * Adds the value on to the end of the array. The array will grow in
 * size automatically if necessary.
 *
 *
 * Returns: the #DArray
 */
DArray  *d_array_append_vals		(DArray *array, 	const void *data,		u64 len);

/**
 * d_array_remove_index_fast:
 * @array: a @DArray
 * @index: the index of the element to remove
 *
 * Removes the element at the given index from a #DArray. The last
 * element in the array is used to fill in the space, so this function
 * does not preserve the order of the #DArray.
 *
 * Returns: the #DArray
 */
DArray  *d_array_remove_index_fast  (DArray	*array, 	u64	index);

/**
 * d_array_clear_array:
 * @array: a @DArray
 *
 * Set array @len: back to 0, meaning that it preserves the previous data
 * however any new push_back or append function will be appended from the start
 * and thus overwritting the value inside the array
 * Returns: the #DArray
 */
DArray  *d_array_clear_array		(DArray* array);

/**
 * d_array_clear_array:
 * @array: a pointer to a @DArray
 *
 * Clears out a DArray struct meaning after the call the DArray will be deallocated,
 * and thus any variable that may still hold a reference to it will generate an invalid memory access
 * error such use after free
 */
void    d_array_destroy_d_array		(DArray** array);

/*-------------------------------------------------DPointerArray-------------------------------------------------*/

/**
 * d_pointer_array_new:
 * @null_terminaed: 1 if one extra element should be added in the array as a null pointer 0 if not
 * @reserved_elem: if > 0 preallocated the value space else use default capacity
 *
 * Creates a new #DArray.
 *
 * Returns: the new #DArray
 */
DPointerArray  *d_pointer_array_new				(u64 reserved_elem, bool null_terminated);

/**
 * d_pointer_array_append_vals:
 * @array: a #DPointerArray
 * @data: (not nullable): a pointer to the elements to append to the end of the array
 * @len: the number of elements to append
 *
 * Adds @len elements onto the end of the array.
 *
 * Returns: the #DPointerArray
 */
DPointerArray  *d_pointer_array_append_vals		(DPointerArray *array, 	const void *data,	u64 len);

/**
 * d_pointer_array_push_back:
 * @a: a #DArray
 * @v: the value to append to the #DPointerArray
 *
 * Adds the value on to the end of the array. The array will grow in
 * size automatically if necessary.
 *
 *
 * Returns: the #DPointerArray
 */
DPointerArray  *d_pointer_array_push_back		(DPointerArray *array, 	const void *data);

/**
 * d_pointer_array_remove_index_fast:
 * @array: a @DPointerArray
 * @index: the index of the element to remove
 *
 * Removes the element at the given index from a #DPointerArray and free it, if free_func field is set. The last
 * element in the array is used to fill in the space, so this function
 * does not preserve the order of the #DPointerArray.
 *
 * Returns: the #DPointerArray
 */
DPointerArray  *d_pointer_array_remove_index_fast  (DPointerArray	*array, 	u64	index);

/**
 * d_pointer_array_clear_array:
 * @array: a @DPointerArray
 *
 * free all array's element and setting back to default state as it was created with d_pointer_array_new function call.
 * if free function is set it will be applied onto each element of the array
 * Returns: the #DPointerArray
 */
DPointerArray  *d_pointer_array_clear_array		(DPointerArray* array);

/**
 * d_pointer_array_clear_array:
 * @array: a pointer to a @DPointerArray
 *
 * Clears out a DPointerArray struct meaning after the call the DPointerArray will be deallocated,
 * and thus any variable that may still hold a reference to it will generate an invalid memory access
 * error such use after free
 */
void    d_pointer_array_destroy		(DPointerArray** array);

#endif