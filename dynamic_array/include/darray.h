#ifndef __D_ARRAY_H
#define __D_ARRAY_H

#include "dtypes.h"

typedef struct _DArray			DArray;
typedef struct _DBytesArray		DBytesArray;

struct _DArray {
	void        *data;
	u_int64_t   len;
};

struct _DBytesArray {
	void        *data;
	u_int64_t   len;
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
DArray  *d_array_new				(bool	clear,		u_int8_t elem_size, u_int64_t reserved_elem);

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
DArray  *d_array_append_vals		(DArray *array, 	const void *data,		u_int64_t len);

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
DArray  *d_array_remove_index_fast  (DArray	*array, 	u_int64_t	index);

DArray  *d_array_clear_array		(DArray* arr);

void    d_array_destroy_d_array		(DArray** arr);
#endif