#include "d_dyn_array.h"
#include "d_bits.h"
#include <stdlib.h>
#include <string.h>
#include "container.h"

struct _DynArray
{
	Container array;
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
	DBits8 opts;
};

#define d_dyn_array_elt_len(arr, i) ((arr)->array.elem_size * (i))
#define d_dyn_array_elt_pos(arr, i) ((char *)(arr)->array.data + d_dyn_array_elt_len((arr), (i)))

DynArray *d_dyn_array_new(usize elem_size, usize reserved_elem, DestroyElemFunc free_func, bool clear, DynArrayOpts opts)
{
	if (elem_size == 0)
		return NULL;
	DynArray *dyn_array = malloc(sizeof(DynArray));
	if (dyn_array == NULL)
		return (NULL);
	dyn_array->array.capacity = reserved_elem == 0 ? DEFAULT_CAPACITY : reserved_elem;
	dyn_array->array.elem_size = elem_size;
	bool zero_terminated = D_GET_BIT(opts, D_DYN_ARRAY_OPT_ZERO_TERMINATED) != 0;
	dyn_array->array.data = malloc((elem_size * dyn_array->array.capacity) + (zero_terminated == true * elem_size));
	d_bits_8_assign(&dyn_array->opts, opts);
	if (dyn_array->array.data == NULL)
	{
		free(dyn_array);
		return NULL;
	}
	dyn_array->array.len = 0;
	dyn_array->free_func = free_func;

	if (clear)
		memset(d_dyn_array_elt_pos(dyn_array, 0), 0, d_dyn_array_elt_len(dyn_array, dyn_array->array.capacity));
	else if (zero_terminated)
		memset(d_dyn_array_elt_pos(dyn_array, 0), 0, elem_size);
	return dyn_array;
}

DynArray *d_dyn_array_append(DynArray *dyn_array, const void *data, usize nb_elem_to_copy)
{
	if (nb_elem_to_copy == 0 || (nb_elem_to_copy != 0 && data == NULL))
		return NULL;
	if (container_increase_capacity_if_needed(&dyn_array->array, nb_elem_to_copy, d_bits_8_check_bit_set(dyn_array->opts, D_DYN_ARRAY_OPT_ZERO_TERMINATED)) == ERROR)
		return NULL;
	memcpy(d_dyn_array_elt_pos(dyn_array, dyn_array->array.len), data, d_dyn_array_elt_len(dyn_array, nb_elem_to_copy));
	dyn_array->array.len += nb_elem_to_copy;
	return dyn_array;
}

DynArray *d_dyn_array_shallow_copy(DynArray *dyn_array)
{
	DynArray *new_array = d_dyn_array_new(dyn_array->array.elem_size, dyn_array->array.capacity, dyn_array->free_func, false, dyn_array->opts.bits);
	d_dyn_array_append_vals(new_array, dyn_array->array.data, dyn_array->array.len);
	return new_array;
}

usize d_dyn_array_get_capacity(DynArray *dyn_array)
{
	return dyn_array->array.capacity;
}

void *d_dyn_array_get_elem(DynArray *dyn_array, usize index)
{
	if (index >= dyn_array->array.len)
	{
		return NULL;
	}
#ifdef BOUNDARY_CHECK
	/* ABORT PROGRAM */
#endif

	return d_dyn_array_elt_pos(dyn_array, index);
}

DynArray *d_dyn_array_modify_capacity(DynArray *dyn_array, usize new_capacity)
{
	if (new_capacity <= dyn_array->array.capacity)
		return dyn_array;
	void *tmp = realloc(dyn_array->array.data, new_capacity * dyn_array->array.elem_size);
	if (tmp == NULL)
		return NULL;
	dyn_array->array.data = tmp;
	dyn_array->array.capacity = new_capacity;
	return dyn_array;
}

DynArray *d_dyn_array_remove_elem_fast(DynArray *dyn_array, usize index)
{
	if (index >= dyn_array->array.len)
		return NULL;

	DestroyElemFunc free_func = dyn_array->free_func;
	void *elem = d_dyn_array_elt_pos(dyn_array, index);
	if (free_func)
		free_func(elem);
	usize index_last_elem = dyn_array->array.len - 1;
	if (index != index_last_elem)
		memcpy(d_dyn_array_elt_pos(dyn_array, index), d_dyn_array_elt_pos(dyn_array, index_last_elem), d_dyn_array_elt_len(dyn_array, 1));
	memset(d_dyn_array_elt_pos(dyn_array, index_last_elem), 0, d_dyn_array_elt_len(dyn_array, 1));
	dyn_array->array.len--;
	return dyn_array;
}

DynArray *d_dyn_array_remove_last_element(DynArray *dyn_array)
{
	if (dyn_array->array.len == 0)
		return NULL;

	DestroyElemFunc free_func = dyn_array->free_func;
	usize index = dyn_array->array.len - 1;
	void *elem = d_dyn_array_elt_pos(dyn_array, index);
	if (free_func)
		free_func(elem);
	memset(d_dyn_array_elt_pos(dyn_array, index), 0, d_dyn_array_elt_len(dyn_array, 1));
	dyn_array->array.len--;
	return dyn_array;
}

static void destroy_elements(DynArray *dyn_array)
{
	DestroyElemFunc free_func = dyn_array->free_func;
	if (!free_func)
		return;
	void *data = dyn_array->array.data;
	for (size_t i = 0; i < dyn_array->array.len; i++)
	{
		void *elem = d_dyn_array_elt_pos(dyn_array, i);
		free_func(elem);
		memset(elem, 0, dyn_array->array.elem_size);
	}
}

void d_dyn_array_destroy(DynArray **dyn_array)
{
	if (dyn_array == NULL || *dyn_array == NULL)
		return;
	DynArray *d_array = *dyn_array;
	destroy_elements(d_array);
	free(d_array->array.data);
	free(d_array);
	*dyn_array = NULL;
}

DynArray *d_dyn_array_clear_array(DynArray *dyn_array)
{
	destroy_elements(dyn_array);
	dyn_array->array.len = 0;
	return dyn_array;
}
