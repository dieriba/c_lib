#include "d_dyn_array.h"
#include "d_bits.h"
#include <stdlib.h>
#include <string.h>
#include "raw_buffer.h"

struct _DynArray
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
};

#define d_dyn_array_elt_len(arr, i) ((arr)->array.elem_size * (i))
#define d_dyn_array_elt_pos(arr, i) ((char *)(arr)->array.data + d_dyn_array_elt_len((arr), (i)))

static DynArray *d_dyn_array_new_raw()
{
	return malloc(sizeof(DynArray));
}

DynArray *d_dyn_array_new(usize elem_size, usize reserved_elem, DestroyElemFunc free_func, BufferOpts opts)
{
	if (elem_size == 0)
		return NULL;
	DynArray *dyn_array = d_dyn_array_new_raw();
	if (dyn_array == NULL)
		return (NULL);
	if (buffer_init(&dyn_array->array, elem_size, reserved_elem, opts) == ERROR)
	{
		free(dyn_array);
		return NULL;
	}
	dyn_array->free_func = free_func;
	return dyn_array;
}

DynArray *d_dyn_array_new_ptr_arr(usize reserved_elem, DestroyElemFunc free_func, BufferOpts opts)
{
	return d_dyn_array_new(sizeof(void *), reserved_elem, free_func, opts);
}

DynArray *d_dyn_array_append(DynArray *dyn_array, const void *data, usize nb_elem_to_copy)
{
	if (dyn_array == NULL || data == NULL)
		return NULL;
	if (buffer_append_data(&dyn_array->array, data, nb_elem_to_copy) == ERROR)
		return NULL;
	return dyn_array;
}

DynArray *d_dyn_push_back(DynArray *dyn_array, const void *data)
{
	if (dyn_array == NULL || data == NULL)
		return NULL;
	if (buffer_push(&dyn_array->array, data) == ERROR)
		return NULL;
	return dyn_array;
}

DynArray *d_dyn_push_back_ptr(DynArray *dyn_array, const void *data)
{
	return d_dyn_push_back(dyn_array, &data);
}

DynArray *d_dyn_array_new_from(DynArray *dyn_array)
{
	if (dyn_array == NULL)
		return NULL;
	RawBuffer *raw_buffer = &dyn_array->array;
	usize elem_size = buffer_get_elem_size(raw_buffer);
	usize capacity = buffer_get_capacity(raw_buffer);
	usize opts = buffer_get_opts(raw_buffer);

	DynArray *new_array = d_dyn_array_new(elem_size, capacity, dyn_array->free_func, opts);
	if (new_array == NULL)
		return NULL;
	if (d_dyn_array_append(new_array, buffer_get_data(dyn_array), buffer_get_len(dyn_array)) == NULL)
	{
		d_dyn_array_destroy(&new_array);
		return NULL;
	}
	return new_array;
}

usize d_dyn_array_get_capacity(DynArray *dyn_array)
{
	if (dyn_array == NULL)
		return 0;
	return buffer_get_capacity(&dyn_array->array);
}

void *d_dyn_array_get_elem_at(DynArray *dyn_array, usize index)
{
	if (dyn_array == NULL)
		return NULL;
	return buffer_get_elem_at(&dyn_array->array, index);
}

DynArray *d_dyn_array_remove_elem_fast(DynArray *dyn_array, usize index, void *out_elem)
{
	if (dyn_array == NULL)
		return NULL;
	void *elem = buffer_get_elem_at(&dyn_array->array, index);
	if (elem == NULL)
		return dyn_array;
	DestroyElemFunc free_func = dyn_array->free_func;
	if (free_func && !out_elem)
		free_func(elem);
	buffer_swap_remove(&dyn_array->array, index, out_elem);
	return dyn_array;
}

DynArray *d_dyn_array_remove_last_element(DynArray *dyn_array, void *out_elem)
{
	if (dyn_array == NULL)
		return NULL;
	usize len = buffer_get_len(&dyn_array->array);
	if (len == 0)
		return dyn_array;
	d_dyn_array_remove_elem_fast(dyn_array, len - 1, out_elem);
	return dyn_array;
}

static void destroy_elements(DynArray *dyn_array)
{
	if (dyn_array == NULL)
		return;
	DestroyElemFunc free_func = dyn_array->free_func;
	if (!free_func)
		return;
	RawBuffer *raw_buffer = &dyn_array->array;
	usize elem_size = buffer_get_elem_size(raw_buffer);
	for (size_t i = 0; i < dyn_array->array.len; i++)
	{
		void *elem = buffer_get_elem_at(raw_buffer, i);
		free_func(elem);
		memset(elem, 0, elem_size);
	}
}

void d_dyn_array_destroy(DynArray **dyn_array)
{
	if (dyn_array == NULL || *dyn_array == NULL)
		return;
	DynArray *d_array = *dyn_array;
	destroy_elements(d_array);
	buffer_free(&d_array->array);
	free(d_array);
	*dyn_array = NULL;
}

DynArray *d_dyn_array_clear_array(DynArray *dyn_array)
{
	if (dyn_array == NULL)
		return NULL;
	destroy_elements(dyn_array);
	dyn_array->array.len = 0;
	return dyn_array;
}
