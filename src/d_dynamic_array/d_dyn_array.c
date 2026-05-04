#include "d_dyn_array.h"
#include "d_bits.h"
#include "raw_buffer.h"

#include <string.h>
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

DResult d_dyn_array_new(DynArray **new_dyn_array, usize elem_size, usize reserved_elem, DestroyElemFunc free_func, BufferOpts opts)
{
	if (new_dyn_array == NULL || elem_size == 0)
		return D_ERR_INVALID_ARG;

	DynArray *arr = d_dyn_array_new_raw();
	if ((*new_dyn_array = arr) == NULL)
		return D_ERR_ALLOC;
	DResult op_result = buffer_init((DynArray *)arr, elem_size, reserved_elem, opts);
	if (op_result != D_OK)
	{
		free(new_dyn_array);
		return op_result;
	}
	arr->free_func = free_func;
	return D_OK;
}

DResult d_dyn_array_new_ptr_arr(DynArray **new_dyn_array, usize reserved_elem, DestroyElemFunc free_func, BufferOpts opts)
{
	return d_dyn_array_new(new_dyn_array, sizeof(void *), reserved_elem, free_func, opts);
}

DResult d_dyn_array_new_from(DynArray **new_dyn_array, DynArray *dyn_array)
{
	if (new_dyn_array == NULL || dyn_array == NULL)
		return D_ERR_INVALID_ARG;
	RawBuffer *raw_buffer = &dyn_array->array;
	DResult op_result = d_dyn_array_new(new_dyn_array, raw_buffer->elem_size, raw_buffer->capacity, dyn_array->free_func, raw_buffer->opts);
	if (op_result != D_OK)
		return op_result;
	if ((op_result = d_dyn_array_append(new_dyn_array, raw_buffer->data, raw_buffer->size)) != D_OK)
	{
		d_dyn_array_destroy(new_dyn_array);
		return op_result;
	}
	return D_OK;
}

DResult d_dyn_array_append(DynArray *dyn_array, const void *data, usize nb_elem_to_copy)
{
	return buffer_append_data((RawBuffer *)dyn_array, data, nb_elem_to_copy);
}

DResult d_dyn_array_push_back(DynArray *dyn_array, const void *data)
{
	return buffer_push((RawBuffer *)dyn_array, data);
}

DResult d_dyn_array_push_back_ptr(DynArray *dyn_array, const void *data)
{
	return d_dyn_array_push_back(dyn_array, &data);
}

void *d_dyn_array_get_elem_at(DynArray *dyn_array, usize index)
{
	return buffer_get_elem_at((RawBuffer *)dyn_array, index);
}

DResult d_dyn_array_get_size(DynArray* dyn_array, usize* size)
{
	return buffer_get_size((RawBuffer*)dyn_array, size);
}

DResult d_dyn_array_get_capacity(DynArray* dyn_array, usize *capacity)
{
	return buffer_get_capacity((RawBuffer*)dyn_array, capacity);
}

DResult d_dyn_array_remove_elem_fast(DynArray *dyn_array, usize index, void *out_elem)
{
	if (dyn_array == NULL)
		return NULL;
	void *elem = buffer_get_elem_at(&dyn_array->array, index);
	if (elem == NULL)
		return dyn_array;
	DestroyElemFunc free_func = dyn_array->free_func;
	if (free_func && !out_elem)
		free_func(elem);
	return buffer_swap_remove(&dyn_array->array, index, out_elem);
}

DResult d_dyn_array_remove_last_element(DynArray *dyn_array, void *out_elem)
{
	if (dyn_array == NULL)
		return NULL;
	usize size = dyn_array->array.size;
	if (size == 0)
		return dyn_array;
	return d_dyn_array_remove_elem_fast(dyn_array, size - 1, out_elem);
}

static void destroy_elements(DynArray *dyn_array)
{
	if (dyn_array == NULL)
		return;
	DestroyElemFunc free_func = dyn_array->free_func;
	if (!free_func)
		return;
	RawBuffer *raw_buffer = &dyn_array->array;
	usize elem_size = raw_buffer->elem_size;
	for (size_t i = 0; i < dyn_array->array.size; i++)
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
	dyn_array->array.size = 0;
	return dyn_array;
}
