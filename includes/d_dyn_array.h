#ifndef D_DYN_ARRAY_H
#define D_DYN_ARRAY_H

/**
 * @defgroup d_dyn_array Dynamic Array
 * @{
 * @brief Type-erased, heap-allocated, automatically-growing array.
 *
 * ::DDynArray stores elements of a fixed, user-supplied size and grows
 * automatically when capacity is exceeded. It is the foundation for most
 * other collection types in the library.
 *
 * Elements are stored by value: every push, append, and get operation copies
 * `elem_size` bytes. Pointer elements (e.g. `char *`) must be stored as the
 * pointer value itself — use @ref d_dyn_array_init_ptr_arr and
 * @ref d_dyn_array_push_back_ptr for convenience.
 *
 * @par Destructor contract
 * When a non-NULL @c free_fn is supplied, it is invoked with a pointer to
 * the **element slot** (not the element value cast to a pointer):
 * - `elem_size == sizeof(Person)` → callback receives `Person *`
 * - `elem_size == sizeof(char *)`  → callback receives `char **`
 *
 * The destructor is called in four situations:
 *  1. @ref d_dyn_array_remove_elem_fast called with `out_elem == NULL`
 *  2. @ref d_dyn_array_remove_last_element called with `out_elem == NULL`
 *  3. @ref d_dyn_array_clear_array
 *  4. @ref d_dyn_array_destroy
 *
 * When `out_elem` is provided to a remove call the caller takes ownership
 * and the destructor is **not** called.
 *
 * @par Typical lifecycle
 * @code{.c}
 *   DDynArray arr;
 *   d_dyn_array_init(&arr, sizeof(int), 16, NULL, 0);
 *
 *   int x = 42;
 *   d_dyn_array_push_back(&arr, &x);
 *
 *   int out;
 *   d_dyn_array_get_elem_at(&arr, 0, &out); // out == 42
 *
 *   d_dyn_array_destroy(&arr);
 * @endcode
 */
#include "d_error.h"
#include "d_types.h"
#include "../src/container/raw_buffer.h"

/**
 * @brief Type-erased dynamic array.
 *
 * Do not access internal fields directly; use the @ref d_dyn_array API.
 */
typedef struct DDynArray
{
    RawBuffer array; /**< Internal raw buffer (implementation detail). */
} DDynArray;

#define d_dyn_array_default_init(d_dyn_array, elem_type, free_fn, copy_fn, opts) \
    d_dyn_array_init(d_dyn_array, sizeof(elem_type), 0x8, free_fn, copy_fn, opts)

#define d_dyn_array_default_ptr_arr_init(d_dyn_array, free_fn, copy_fn, opts) \
    d_dyn_array_init(d_dyn_array, sizeof(void *), 0x8, free_fn, copy_fn, opts)

#define d_dyn_array_get_elem_addr_at_safe(d_dyn_array, index) (void *)(((char *)((d_dyn_array)->array.data)) + d_dyn_array->array.elem_size * (index))
#define d_dyn_array_get_elem_deref_addr_at_safe(d_dyn_array, index) *((void **)(((char *)((d_dyn_array)->array.data)) + d_dyn_array->array.elem_size * (index)))

/* -----------------------------------------------------------------------
 * Initialisation
 * -------------------------------------------------------------------- */

/**
 * @brief Initialises a ::DDynArray for elements of size @p elem_size.
 *
 * @param  new_dyn_array  Array to initialise. Must not be NULL.
 * @param  elem_size      Size in bytes of each element. Must be > 0.
 * @param  reserved_elem  Number of elements to pre-allocate. May be 0.
 * @param  free_fn      Optional destructor called on removed elements. May be NULL.
 * @param  opts           Buffer option flags (pass @c 0 for defaults).
 * @return ::D_OK, ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DDynArray arr;
 *   d_dyn_array_init(&arr, sizeof(int), 8, NULL, 0);
 *   d_dyn_array_destroy(&arr);
 * @endcode
 */
DResult d_dyn_array_init(DDynArray *new_dyn_array, usize elem_size, usize reserved_elem, DestroyElemFn free_fn, CopyElemFn copy_fn, BufferOpts opts);

/**
 * @brief Initialises a ::DDynArray whose elements are raw pointers (@c void *).
 *
 * Shorthand for `d_dyn_array_init(arr, sizeof(void *), ...)`.
 * Use @ref d_dyn_array_push_back_ptr to append pointer values.
 *
 * @param  new_dyn_array  Array to initialise. Must not be NULL.
 * @param  reserved_elem  Number of pointer slots to pre-allocate. May be 0.
 * @param  free_fn      Optional destructor for removed pointer elements. May be NULL.
 * @param  opts           Buffer option flags (pass @c 0 for defaults).
 * @return ::D_OK, ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DDynArray ptrs;
 *   d_dyn_array_init_ptr_arr(&ptrs, 4, free, 0);
 *   char *s = strdup("hello");
 *   d_dyn_array_push_back_ptr(&ptrs, s);
 *   d_dyn_array_destroy(&ptrs); // calls free on "hello"
 * @endcode
 */
DResult d_dyn_array_init_ptr_arr(DDynArray *new_dyn_array, usize reserved_elem, DestroyElemFn free_fn, CopyElemFn copy_fn, BufferOpts opts);

/**
 * @brief Initialises @p new_dyn_array as an independent deep copy of @p dyn_array.
 *
 * The new array inherits the source's element size, capacity, and destructor.
 * Modifying one array does not affect the other.
 *
 * @param  new_dyn_array  Destination array to initialise. Must not be NULL.
 * @param  dyn_array      Source array to copy. Must not be NULL.
 * @return ::D_OK, ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   DDynArray copy;
 *   d_dyn_array_init_from(&copy, &original);
 *   // copy and original are now independent
 *   d_dyn_array_destroy(&copy);
 * @endcode
 */
DResult d_dyn_array_init_from(DDynArray *new_dyn_array, const DDynArray *dyn_array);

/* -----------------------------------------------------------------------
 * Accessors
 * -------------------------------------------------------------------- */

/**
 * @brief Copies the element at @p index into the buffer pointed to by @p out_elem.
 *
 * The caller must ensure @p out_elem points to a buffer of at least
 * `elem_size` bytes.
 *
 * @param  dyn_array The array to read. Must not be NULL.
 * @param  index     Zero-based element index.
 * @param  out_elem  Buffer that receives a copy of the element. Must not be NULL.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if @p index >= size.
 *
 * @code{.c}
 *   int val;
 *   d_dyn_array_get_elem_at(&arr, 2, &val);
 * @endcode
 */
DResult d_dyn_array_get_elem_at(const DDynArray *dyn_array, usize index, void *out_elem);

DResult d_dyn_array_get_elem_addr_at(const DDynArray *dyn_array, usize index, void **out_elem);

/**
 * @brief Returns the current element count.
 *
 * @param  dyn_array The array to query. Must not be NULL.
 */
usize d_dyn_array_get_size(const DDynArray *dyn_array);

/**
 * @brief Returns the allocated capacity in elements.
 *
 * @param  dyn_array The array to query. Must not be NULL.
 */
usize d_dyn_array_get_capacity(const DDynArray *dyn_array);

/**
 * @brief Returns true if the array has no elements.
 *
 * @param  dyn_array The array to query. Must not be NULL.
 */
bool d_dyn_array_is_empty(const DDynArray *dyn_array);

/* -----------------------------------------------------------------------
 * Insertion
 * -------------------------------------------------------------------- */

/**
 * @brief Appends @p nb_elem_to_copy elements from the contiguous array @p data.
 *
 * @param dyn_array       The array to append to. Must not be NULL.
 * @param     data            Pointer to the source elements. Must not be NULL.
 * @param     nb_elem_to_copy Number of elements to copy from @p data.
 * @return ::D_OK, ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   int vals[] = {1, 2, 3};
 *   d_dyn_array_append(&arr, vals, 3);
 * @endcode
 */
DResult d_dyn_array_append(DDynArray *dyn_array, const void *data, usize nb_elem_to_copy);

/**
 * @brief Copies the element pointed to by @p data and appends it to the array.
 *
 * @param dyn_array The array to append to. Must not be NULL.
 * @param     data      Pointer to the element to copy. Must not be NULL.
 * @return ::D_OK, ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   int x = 42;
 *   d_dyn_array_push_back(&arr, &x);
 * @endcode
 */
DResult d_dyn_array_push_back(DDynArray *dyn_array, const void *data);

/**
 * @brief Appends a pointer value to a pointer array.
 *
 * Stores the pointer @p data itself (not the data it points to). Intended for
 * arrays initialised with @ref d_dyn_array_init_ptr_arr. Passing NULL as
 * @p data stores a null pointer — this is valid.
 *
 * @param dyn_array The pointer array to append to. Must not be NULL.
 * @param     data      Pointer value to store (may be NULL).
 * @return ::D_OK, ::D_ERR_ALLOC on allocation failure.
 *
 * @code{.c}
 *   char *s = strdup("hello");
 *   d_dyn_array_push_back_ptr(&ptrs, s);
 * @endcode
 */
DResult d_dyn_array_push_back_ptr(DDynArray *dyn_array, const void *data);

/* -----------------------------------------------------------------------
 * Removal
 * -------------------------------------------------------------------- */

/**
 * @brief Removes the element at @p index in O(1) by swapping it with the last element.
 *
 * @note This operation does **not** preserve element order.
 *
 * If @p out_elem is non-NULL the removed element is copied there and the
 * destructor is **not** called. If @p out_elem is NULL the destructor is
 * called (when one was supplied at init time).
 *
 * @param dyn_array The array to remove from. Must not be NULL.
 * @param     index     Zero-based index of the element to remove.
 * @param     out_elem  Optional buffer that receives the removed element.
 *                          Must be at least @c elem_size bytes if non-NULL.
 * @return ::D_OK, ::D_ERR_INVALID_ARG if @p index >= size.
 *
 * @code{.c}
 *   int removed;
 *   d_dyn_array_remove_elem_fast(&arr, 1, &removed); // caller owns the element
 *   d_dyn_array_remove_elem_fast(&arr, 0, NULL);     // destructor called
 * @endcode
 */
DResult d_dyn_array_remove_elem_fast(DDynArray *dyn_array, usize index, void *out_elem);

/**
 * @brief Removes the last element from the array.
 *
 * If @p out_elem is non-NULL the removed element is copied there and the
 * destructor is **not** called. If @p out_elem is NULL the destructor is
 * called (when one was supplied at init time).
 *
 * @param dyn_array The array to pop from. Must not be NULL.
 * @param     out_elem  Optional buffer that receives the popped element.
 *                      Must be at least @c elem_size bytes if non-NULL.
 * @return ::D_OK, ::D_ERR_EMPTY if the array contains no elements.
 */
DResult d_dyn_array_remove_last_element(DDynArray *dyn_array, void *out_elem);

/* -----------------------------------------------------------------------
 * Clear / Destroy
 * -------------------------------------------------------------------- */

/**
 * @brief Removes all elements but keeps the allocated memory.
 *
 * Calls the destructor (if any) for every element then resets the size to 0.
 * The internal buffer is not freed so subsequent pushes will not reallocate
 * until capacity is exceeded.
 *
 * @param dyn_array The array to clear. Must not be NULL.
 *
 * @code{.c}
 *   d_dyn_array_clear_array(&arr);
 *   // arr.size == 0 but capacity is unchanged
 * @endcode
 */
DResult d_dyn_array_clear_array(DDynArray *dyn_array);

/**
 * @brief Calls the destructor for all remaining elements, frees the buffer,
 *        and zeroes the struct.
 *
 * Safe to call on a NULL pointer (no-op). After this call @p dyn_array must
 * not be used without re-initialisation.
 *
 * @param dyn_array The array to destroy. May be NULL.
 */
void d_dyn_array_destroy(DDynArray *dyn_array);

void d_dyn_array_dbg_print(DDynArray *dyn_array, FnElemRepr elem_repr);

/** @} */ /* end of d_dyn_array group */

#endif
