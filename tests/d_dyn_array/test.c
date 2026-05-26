#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "d_test.h"
#include "d_dyn_array.h"
#include "d_types.h"

#define ARRAY_DEFAULT_OPTS ((BufferOpts)0)

static void expect_size(DDynArray *arr, usize expected)
{
    usize size = (usize)-1;

    D_TEST_EXPR(d_dyn_array_get_size(arr, &size) == D_OK);
    D_TEST_EXPR(size == expected);
}

static void expect_capacity_at_least(DDynArray *arr, usize min_capacity)
{
    usize capacity = 0;

    D_TEST_EXPR(d_dyn_array_get_capacity(arr, &capacity) == D_OK);
    D_TEST_EXPR(capacity >= min_capacity);
}

static void make_int_array(DDynArray *arr, usize reserved)
{
    D_TEST_EXPR(d_dyn_array_init(arr, sizeof(int), reserved, NULL, NULL, ARRAY_DEFAULT_OPTS) == D_OK);
}

static int get_int_at(DDynArray *arr, usize index)
{
    int value = 0x7fffffff;

    D_TEST_EXPR(d_dyn_array_get_elem_at(arr, index, &value) == D_OK);
    return value;
}

static void expect_int_at(DDynArray *arr, usize index, int expected)
{
    D_TEST_EXPR(get_int_at(arr, index) == expected);
}

static void push_int(DDynArray *arr, int value)
{
    D_TEST_EXPR(d_dyn_array_push_back(arr, &value) == D_OK);
}

static char *owned_string(const char *s)
{
    size_t len = strlen(s);
    char *copy = malloc(len + 1);

    D_TEST_NOT_NULL(copy);
    if (copy != NULL)
        memcpy(copy, s, len + 1);
    return copy;
}

static int g_destroy_count = 0;
static int g_destroy_sum = 0;
static int g_destroy_bad_slot_count = 0;
static char *g_expected_ptrs[16];
static int g_expected_ptr_count = 0;

static void reset_destroy_tracking(void)
{
    g_destroy_count = 0;
    g_destroy_sum = 0;
    g_destroy_bad_slot_count = 0;
    memset(g_expected_ptrs, 0, sizeof(g_expected_ptrs));
    g_expected_ptr_count = 0;
}

static void int_destroy_counter(void *elem_slot)
{
    int *value = elem_slot;

    g_destroy_count++;
    g_destroy_sum += *value;
}

/*
 * Checks the documented contract: for elem_size == sizeof(char *),
 * free_fn receives char **, i.e. the address of the element slot.
 */
static void tracked_string_ptr_destroy(void *elem_slot)
{
    char **slot = elem_slot;
    int found = 0;

    g_destroy_count++;
    for (int i = 0; i < g_expected_ptr_count; i++)
    {
        if (*slot == g_expected_ptrs[i])
        {
            found = 1;
            g_expected_ptrs[i] = NULL;
            break;
        }
    }
    if (!found)
    {
        g_destroy_bad_slot_count++;
        return;
    }
    free(*slot);
    *slot = NULL;
}

static void test_init_creates_empty_array(void)
{
    DDynArray arr;

    D_TEST_EXPR(d_dyn_array_init(&arr, sizeof(int), 0, NULL, NULL, ARRAY_DEFAULT_OPTS) == D_OK);
    expect_size(&arr, 0);
    d_dyn_array_destroy(&arr);
}

static void test_init_with_reserved_capacity_sets_capacity(void)
{
    DDynArray arr;

    D_TEST_EXPR(d_dyn_array_init(&arr, sizeof(int), 32, NULL, NULL, ARRAY_DEFAULT_OPTS) == D_OK);
    expect_size(&arr, 0);
    expect_capacity_at_least(&arr, 32);
    d_dyn_array_destroy(&arr);
}

static void test_init_ptr_arr_creates_empty_pointer_array(void)
{
    DDynArray arr;

    D_TEST_EXPR(d_dyn_array_init_ptr_arr(&arr, 4, NULL, NULL, ARRAY_DEFAULT_OPTS) == D_OK);
    expect_size(&arr, 0);
    expect_capacity_at_least(&arr, 4);
    d_dyn_array_destroy(&arr);
}

static void test_push_back_appends_single_int(void)
{
    DDynArray arr;
    int value = 42;

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_push_back(&arr, &value) == D_OK);
    expect_size(&arr, 1);
    expect_int_at(&arr, 0, 42);
    d_dyn_array_destroy(&arr);
}

static void test_push_back_copies_value_not_alias_source(void)
{
    DDynArray arr;
    int value = 10;

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_push_back(&arr, &value) == D_OK);
    value = 99;
    expect_int_at(&arr, 0, 10);
    d_dyn_array_destroy(&arr);
}

static void test_push_back_grows_capacity_and_preserves_values(void)
{
    DDynArray arr;

    make_int_array(&arr, 1);
    for (int i = 0; i < 128; i++)
        push_int(&arr, i * 3);
    expect_size(&arr, 128);
    expect_capacity_at_least(&arr, 128);
    for (int i = 0; i < 128; i++)
        expect_int_at(&arr, (usize)i, i * 3);
    d_dyn_array_destroy(&arr);
}

static void test_append_multiple_ints(void)
{
    DDynArray arr;
    int values[] = {1, 2, 3, 4, 5};

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_append(&arr, values, 5) == D_OK);
    expect_size(&arr, 5);
    for (int i = 0; i < 5; i++)
        expect_int_at(&arr, (usize)i, values[i]);
    d_dyn_array_destroy(&arr);
}

static void test_append_after_push_preserves_order(void)
{
    DDynArray arr;
    int first = 7;
    int rest[] = {8, 9, 10};

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_push_back(&arr, &first) == D_OK);
    D_TEST_EXPR(d_dyn_array_append(&arr, rest, 3) == D_OK);
    expect_size(&arr, 4);
    expect_int_at(&arr, 0, 7);
    expect_int_at(&arr, 1, 8);
    expect_int_at(&arr, 2, 9);
    expect_int_at(&arr, 3, 10);
    d_dyn_array_destroy(&arr);
}

static void test_append_zero_count_keeps_array_unchanged(void)
{
    DDynArray arr;
    int values[] = {11, 22};

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_append(&arr, values, 0) == D_OK);
    expect_size(&arr, 0);
    d_dyn_array_destroy(&arr);
}

static void test_get_elem_at_reads_first_middle_last(void)
{
    DDynArray arr;
    int values[] = {5, 10, 15, 20, 25};

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_append(&arr, values, 5) == D_OK);
    expect_int_at(&arr, 0, 5);
    expect_int_at(&arr, 2, 15);
    expect_int_at(&arr, 4, 25);
    d_dyn_array_destroy(&arr);
}

static void test_get_elem_at_rejects_index_equal_size(void)
{
    DDynArray arr;
    int value = 1;
    int out = 123;

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_push_back(&arr, &value) == D_OK);
    D_TEST_EXPR(d_dyn_array_get_elem_at(&arr, 1, &out) != D_OK);
    D_TEST_EXPR(out == 123);
    d_dyn_array_destroy(&arr);
}

static void test_get_elem_at_rejects_index_from_empty_array(void)
{
    DDynArray arr;
    int out = 123;

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_get_elem_at(&arr, 0, &out) != D_OK);
    D_TEST_EXPR(out == 123);
    d_dyn_array_destroy(&arr);
}

static void test_init_from_copies_size_capacity_and_values(void)
{
    DDynArray src;
    DDynArray copy;

    make_int_array(&src, 2);
    for (int i = 0; i < 10; i++)
        push_int(&src, i + 100);
    D_TEST_EXPR(d_dyn_array_init_from(&copy, &src) == D_OK);
    expect_size(&copy, 10);
    expect_capacity_at_least(&copy, 10);
    for (int i = 0; i < 10; i++)
        expect_int_at(&copy, (usize)i, i + 100);
    d_dyn_array_destroy(&copy);
    d_dyn_array_destroy(&src);
}

static void test_init_from_does_not_alias_source_storage(void)
{
    DDynArray src;
    DDynArray copy;
    int value = 1;
    int replacement = 99;

    make_int_array(&src, 0);
    D_TEST_EXPR(d_dyn_array_push_back(&src, &value) == D_OK);
    D_TEST_EXPR(d_dyn_array_init_from(&copy, &src) == D_OK);
    D_TEST_EXPR(d_dyn_array_remove_last_element(&src, NULL) == D_OK);
    D_TEST_EXPR(d_dyn_array_push_back(&src, &replacement) == D_OK);
    expect_int_at(&copy, 0, 1);
    expect_int_at(&src, 0, 99);
    d_dyn_array_destroy(&copy);
    d_dyn_array_destroy(&src);
}

static void test_push_back_ptr_stores_pointer_value(void)
{
    DDynArray arr;
    int x = 123;
    int *out = NULL;

    D_TEST_EXPR(d_dyn_array_init_ptr_arr(&arr, 0, NULL, NULL, ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(d_dyn_array_push_back_ptr(&arr, &x) == D_OK);
    expect_size(&arr, 1);
    D_TEST_EXPR(d_dyn_array_get_elem_at(&arr, 0, &out) == D_OK);
    D_TEST_EXPR(out == &x);
    D_TEST_EXPR(*out == 123);
    d_dyn_array_destroy(&arr);
}

static void test_push_back_ptr_accepts_null_pointer_value(void)
{
    DDynArray arr;
    void *out = (void *)0x1;

    D_TEST_EXPR(d_dyn_array_init_ptr_arr(&arr, 0, NULL, NULL, ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(d_dyn_array_push_back_ptr(&arr, NULL) == D_OK);
    D_TEST_EXPR(d_dyn_array_get_elem_at(&arr, 0, &out) == D_OK);
    D_TEST_NULL(out);
    d_dyn_array_destroy(&arr);
}

static void test_remove_last_on_empty_array_returns_empty_err(void)
{
    DDynArray arr;
    int out = 123;

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_remove_last_element(&arr, &out) == D_ERR_EMPTY);
    D_TEST_EXPR(out == 123);
    expect_size(&arr, 0);
    d_dyn_array_destroy(&arr);
}

static void test_remove_last_returns_removed_value(void)
{
    DDynArray arr;
    int values[] = {1, 2, 3};
    int out = 0;

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_append(&arr, values, 3) == D_OK);
    D_TEST_EXPR(d_dyn_array_remove_last_element(&arr, &out) == D_OK);
    D_TEST_EXPR(out == 3);
    expect_size(&arr, 2);
    expect_int_at(&arr, 0, 1);
    expect_int_at(&arr, 1, 2);
    d_dyn_array_destroy(&arr);
}

static void test_remove_last_without_out_discards_value(void)
{
    DDynArray arr;
    int values[] = {1, 2, 3};

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_append(&arr, values, 3) == D_OK);
    D_TEST_EXPR(d_dyn_array_remove_last_element(&arr, NULL) == D_OK);
    expect_size(&arr, 2);
    expect_int_at(&arr, 0, 1);
    expect_int_at(&arr, 1, 2);
    d_dyn_array_destroy(&arr);
}

static void test_remove_elem_fast_from_middle_swaps_last_into_removed_slot(void)
{
    DDynArray arr;
    int values[] = {10, 20, 30, 40};
    int out = 0;

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_append(&arr, values, 4) == D_OK);
    D_TEST_EXPR(d_dyn_array_remove_elem_fast(&arr, 1, &out) == D_OK);
    D_TEST_EXPR(out == 20);
    expect_size(&arr, 3);
    expect_int_at(&arr, 0, 10);
    expect_int_at(&arr, 1, 40);
    expect_int_at(&arr, 2, 30);
    d_dyn_array_destroy(&arr);
}

static void test_remove_elem_fast_first_from_single_element_array(void)
{
    DDynArray arr;
    int value = 555;
    int out = 0;

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_push_back(&arr, &value) == D_OK);
    D_TEST_EXPR(d_dyn_array_remove_elem_fast(&arr, 0, &out) == D_OK);
    D_TEST_EXPR(out == 555);
    expect_size(&arr, 0);
    d_dyn_array_destroy(&arr);
}

static void test_remove_elem_fast_rejects_index_equal_size(void)
{
    DDynArray arr;
    int value = 1;
    int out = 123;

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_push_back(&arr, &value) == D_OK);
    D_TEST_EXPR(d_dyn_array_remove_elem_fast(&arr, 1, &out) != D_OK);
    D_TEST_EXPR(out == 123);
    expect_size(&arr, 1);
    expect_int_at(&arr, 0, 1);
    d_dyn_array_destroy(&arr);
}

static void test_remove_elem_fast_rejects_remove_from_empty_array(void)
{
    DDynArray arr;
    int out = 123;

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_remove_elem_fast(&arr, 0, &out) != D_OK);
    D_TEST_EXPR(out == 123);
    expect_size(&arr, 0);
    d_dyn_array_destroy(&arr);
}

static void test_remove_elem_fast_with_out_does_not_call_destructor(void)
{
    DDynArray arr;
    int values[] = {1, 2, 3};
    int out = 0;

    reset_destroy_tracking();
    D_TEST_EXPR(d_dyn_array_init(&arr, sizeof(int), 0, int_destroy_counter, NULL, ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(d_dyn_array_append(&arr, values, 3) == D_OK);
    D_TEST_EXPR(d_dyn_array_remove_elem_fast(&arr, 1, &out) == D_OK);
    D_TEST_EXPR(out == 2);
    D_TEST_EXPR(g_destroy_count == 0);
    d_dyn_array_destroy(&arr);
}

static void test_remove_elem_fast_without_out_calls_destructor_once(void)
{
    DDynArray arr;
    int values[] = {4, 5, 6};

    reset_destroy_tracking();
    D_TEST_EXPR(d_dyn_array_init(&arr, sizeof(int), 0, int_destroy_counter, NULL, ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(d_dyn_array_append(&arr, values, 3) == D_OK);
    D_TEST_EXPR(d_dyn_array_remove_elem_fast(&arr, 1, NULL) == D_OK);
    D_TEST_EXPR(g_destroy_count == 1);
    D_TEST_EXPR(g_destroy_sum == 5);
    expect_size(&arr, 2);
    d_dyn_array_destroy(&arr);
}

static void test_clear_empty_array_is_ok(void)
{
    DDynArray arr;

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_clear_array(&arr) == D_OK);
    expect_size(&arr, 0);
    d_dyn_array_destroy(&arr);
}

static void test_clear_removes_all_elements_but_keeps_capacity(void)
{
    DDynArray arr;
    int values[] = {1, 2, 3, 4};
    usize before_capacity = 0;
    usize after_capacity = 0;

    make_int_array(&arr, 16);
    D_TEST_EXPR(d_dyn_array_append(&arr, values, 4) == D_OK);
    D_TEST_EXPR(d_dyn_array_get_capacity(&arr, &before_capacity) == D_OK);
    D_TEST_EXPR(d_dyn_array_clear_array(&arr) == D_OK);
    expect_size(&arr, 0);
    D_TEST_EXPR(d_dyn_array_get_capacity(&arr, &after_capacity) == D_OK);
    D_TEST_EXPR(after_capacity == before_capacity);
    d_dyn_array_destroy(&arr);
}

static void test_clear_calls_destructor_for_each_element(void)
{
    DDynArray arr;
    int values[] = {1, 2, 3, 4};

    reset_destroy_tracking();
    D_TEST_EXPR(d_dyn_array_init(&arr, sizeof(int), 0, int_destroy_counter, NULL, ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(d_dyn_array_append(&arr, values, 4) == D_OK);
    D_TEST_EXPR(d_dyn_array_clear_array(&arr) == D_OK);
    D_TEST_EXPR(g_destroy_count == 4);
    D_TEST_EXPR(g_destroy_sum == 10);
    expect_size(&arr, 0);
    d_dyn_array_destroy(&arr);
}

static void test_destroy_null_pointer_is_safe(void)
{
    d_dyn_array_destroy(NULL);
    D_TEST_EXPR(true);
}

static void test_destroy_empty_array_is_safe(void)
{
    DDynArray arr;

    make_int_array(&arr, 0);
    d_dyn_array_destroy(&arr);
    D_TEST_EXPR(true);
}

static void test_destroy_calls_destructor_for_each_remaining_element(void)
{
    DDynArray arr;
    int values[] = {9, 8, 7};

    reset_destroy_tracking();
    D_TEST_EXPR(d_dyn_array_init(&arr, sizeof(int), 0, int_destroy_counter, NULL, ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(d_dyn_array_append(&arr, values, 3) == D_OK);
    d_dyn_array_destroy(&arr);
    D_TEST_EXPR(g_destroy_count == 3);
    D_TEST_EXPR(g_destroy_sum == 24);
}

static void test_destructor_not_called_for_removed_owned_element_with_out(void)
{
    DDynArray arr;
    char *a = owned_string("a");
    char *b = owned_string("b");
    char *out = NULL;

    reset_destroy_tracking();
    g_expected_ptrs[0] = a;
    g_expected_ptrs[1] = b;
    g_expected_ptr_count = 2;

    D_TEST_EXPR(d_dyn_array_init_ptr_arr(&arr, 0, tracked_string_ptr_destroy, NULL, ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(d_dyn_array_push_back_ptr(&arr, a) == D_OK);
    D_TEST_EXPR(d_dyn_array_push_back_ptr(&arr, b) == D_OK);
    D_TEST_EXPR(d_dyn_array_remove_elem_fast(&arr, 0, &out) == D_OK);
    D_TEST_EXPR(out == a);
    D_TEST_EXPR(g_destroy_count == 0);

    free(out);
    g_expected_ptrs[0] = NULL;
    d_dyn_array_destroy(&arr);
    D_TEST_EXPR(g_destroy_count == 1);
    D_TEST_EXPR(g_destroy_bad_slot_count == 0);
}

static void test_pointer_array_destructor_receives_element_slot_on_destroy(void)
{
    DDynArray arr;
    char *a = owned_string("alpha");
    char *b = owned_string("beta");

    reset_destroy_tracking();
    g_expected_ptrs[0] = a;
    g_expected_ptrs[1] = b;
    g_expected_ptr_count = 2;

    D_TEST_EXPR(d_dyn_array_init_ptr_arr(&arr, 0, tracked_string_ptr_destroy, NULL, ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(d_dyn_array_push_back_ptr(&arr, a) == D_OK);
    D_TEST_EXPR(d_dyn_array_push_back_ptr(&arr, b) == D_OK);
    d_dyn_array_destroy(&arr);
    D_TEST_EXPR(g_destroy_count == 2);
    D_TEST_EXPR(g_destroy_bad_slot_count == 0);
}

static void test_pointer_array_destructor_receives_element_slot_on_clear(void)
{
    DDynArray arr;
    char *a = owned_string("clear-a");
    char *b = owned_string("clear-b");

    reset_destroy_tracking();
    g_expected_ptrs[0] = a;
    g_expected_ptrs[1] = b;
    g_expected_ptr_count = 2;

    D_TEST_EXPR(d_dyn_array_init_ptr_arr(&arr, 0, tracked_string_ptr_destroy, NULL, ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(d_dyn_array_push_back_ptr(&arr, a) == D_OK);
    D_TEST_EXPR(d_dyn_array_push_back_ptr(&arr, b) == D_OK);
    D_TEST_EXPR(d_dyn_array_clear_array(&arr) == D_OK);
    D_TEST_EXPR(g_destroy_count == 2);
    D_TEST_EXPR(g_destroy_bad_slot_count == 0);
    expect_size(&arr, 0);
    d_dyn_array_destroy(&arr);
}

static void test_pointer_array_destructor_receives_element_slot_on_remove_without_out(void)
{
    DDynArray arr;
    char *a = owned_string("remove-a");
    char *b = owned_string("remove-b");

    reset_destroy_tracking();
    g_expected_ptrs[0] = a;
    g_expected_ptrs[1] = b;
    g_expected_ptr_count = 2;

    D_TEST_EXPR(d_dyn_array_init_ptr_arr(&arr, 0, tracked_string_ptr_destroy, NULL, ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(d_dyn_array_push_back_ptr(&arr, a) == D_OK);
    D_TEST_EXPR(d_dyn_array_push_back_ptr(&arr, b) == D_OK);
    D_TEST_EXPR(d_dyn_array_remove_elem_fast(&arr, 0, NULL) == D_OK);
    D_TEST_EXPR(g_destroy_count == 1);
    D_TEST_EXPR(g_destroy_bad_slot_count == 0);
    expect_size(&arr, 1);
    d_dyn_array_destroy(&arr);
    D_TEST_EXPR(g_destroy_count == 2);
    D_TEST_EXPR(g_destroy_bad_slot_count == 0);
}

static void test_large_struct_elements_are_copied_exactly(void)
{
    typedef struct BigElem
    {
        uint64_t a;
        uint32_t b;
        unsigned char bytes[37];
    } BigElem;

    DDynArray arr;
    BigElem elems[3];
    BigElem out;

    memset(elems, 0, sizeof(elems));
    for (usize i = 0; i < 3; i++)
    {
        elems[i].a = 0x1111111111111111ULL + i;
        elems[i].b = 0x22222222U + (uint32_t)i;
        for (usize j = 0; j < sizeof(elems[i].bytes); j++)
            elems[i].bytes[j] = (unsigned char)(i * 10 + j);
    }

    D_TEST_EXPR(d_dyn_array_init(&arr, sizeof(BigElem), 0, NULL, NULL, ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(d_dyn_array_append(&arr, elems, 3) == D_OK);
    memset(&out, 0, sizeof(out));
    D_TEST_EXPR(d_dyn_array_get_elem_at(&arr, 2, &out) == D_OK);
    D_TEST_MEM_EQ(&out, &elems[2], sizeof(BigElem));
    d_dyn_array_destroy(&arr);
}

static void test_many_mutations_preserve_valid_state(void)
{
    DDynArray arr;
    int out = 0;

    make_int_array(&arr, 0);
    for (int i = 0; i < 100; i++)
        push_int(&arr, i);
    for (int i = 0; i < 50; i++)
        D_TEST_EXPR(d_dyn_array_remove_last_element(&arr, &out) == D_OK);
    for (int i = 100; i < 175; i++)
        push_int(&arr, i);

    expect_size(&arr, 125);
    expect_capacity_at_least(&arr, 125);
    for (int i = 0; i < 50; i++)
        expect_int_at(&arr, (usize)i, i);
    for (int i = 0; i < 75; i++)
        expect_int_at(&arr, (usize)(50 + i), 100 + i);
    d_dyn_array_destroy(&arr);
}

static void test_push_to_exact_power_of_two_then_one_more(void)
{
    DDynArray arr;

    make_int_array(&arr, 8);
    for (int i = 0; i < 8; i++)
        push_int(&arr, i);
    expect_size(&arr, 8);
    push_int(&arr, 8); /* triggers growth */
    expect_size(&arr, 9);
    expect_capacity_at_least(&arr, 9);
    for (int i = 0; i <= 8; i++)
        expect_int_at(&arr, (usize)i, i);
    d_dyn_array_destroy(&arr);
}

static void test_append_then_clear_then_push_again(void)
{
    DDynArray arr;
    int first[] = {1, 2, 3, 4, 5};
    int second[] = {10, 20, 30};

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_append(&arr, first, 5) == D_OK);
    D_TEST_EXPR(d_dyn_array_clear_array(&arr) == D_OK);
    expect_size(&arr, 0);
    D_TEST_EXPR(d_dyn_array_append(&arr, second, 3) == D_OK);
    expect_size(&arr, 3);
    expect_int_at(&arr, 0, 10);
    expect_int_at(&arr, 1, 20);
    expect_int_at(&arr, 2, 30);
    d_dyn_array_destroy(&arr);
}

static void test_remove_elem_fast_last_element_is_swap_with_self(void)
{
    DDynArray arr;
    int values[] = {1, 2, 3};
    int out = 0;

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_append(&arr, values, 3) == D_OK);
    D_TEST_EXPR(d_dyn_array_remove_elem_fast(&arr, 2, &out) == D_OK);
    D_TEST_EXPR(out == 3);
    expect_size(&arr, 2);
    expect_int_at(&arr, 0, 1);
    expect_int_at(&arr, 1, 2);
    d_dyn_array_destroy(&arr);
}

static void test_get_elem_at_does_not_alias_internal_storage(void)
{
    DDynArray arr;
    int values[] = {100, 200, 300};
    int out = 0;

    make_int_array(&arr, 0);
    D_TEST_EXPR(d_dyn_array_append(&arr, values, 3) == D_OK);
    D_TEST_EXPR(d_dyn_array_get_elem_at(&arr, 0, &out) == D_OK);
    out = 999;
    expect_int_at(&arr, 0, 100); /* internal value must be unaffected */
    d_dyn_array_destroy(&arr);
}

static void test_init_from_is_independent_after_source_clear(void)
{
    DDynArray src;
    DDynArray copy;
    int values[] = {7, 8, 9};

    make_int_array(&src, 0);
    D_TEST_EXPR(d_dyn_array_append(&src, values, 3) == D_OK);
    D_TEST_EXPR(d_dyn_array_init_from(&copy, &src) == D_OK);
    D_TEST_EXPR(d_dyn_array_clear_array(&src) == D_OK);
    expect_size(&src, 0);
    expect_size(&copy, 3);
    expect_int_at(&copy, 0, 7);
    expect_int_at(&copy, 1, 8);
    expect_int_at(&copy, 2, 9);
    d_dyn_array_destroy(&copy);
    d_dyn_array_destroy(&src);
}

static void string_ptr_free(void *elem_slot)
{
    free(*(char **)elem_slot);
}

static void *string_ptr_copy(void *elem_slot)
{
    char **slot = elem_slot;
    char **new_slot = malloc(sizeof(char *));

    if (new_slot == NULL)
        return NULL;
    *new_slot = strdup(*slot);
    if (*new_slot == NULL)
    {
        free(new_slot);
        return NULL;
    }
    return new_slot;
}

static void test_init_from_with_copy_fn_deep_copies_pointer_elements(void)
{
    DDynArray src;
    DDynArray copy;
    char *out_a = NULL;
    char *out_b = NULL;
    char *copy_a = NULL;
    char *copy_b = NULL;

    D_TEST_EXPR(d_dyn_array_init_ptr_arr(&src, 0, string_ptr_free, string_ptr_copy, ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(d_dyn_array_push_back_ptr(&src, owned_string("hello")) == D_OK);
    D_TEST_EXPR(d_dyn_array_push_back_ptr(&src, owned_string("world")) == D_OK);
    D_TEST_EXPR(d_dyn_array_init_from(&copy, &src) == D_OK);

    D_TEST_EXPR(d_dyn_array_get_elem_at(&src, 0, &out_a) == D_OK);
    D_TEST_EXPR(d_dyn_array_get_elem_at(&src, 1, &out_b) == D_OK);
    D_TEST_EXPR(d_dyn_array_get_elem_at(&copy, 0, &copy_a) == D_OK);
    D_TEST_EXPR(d_dyn_array_get_elem_at(&copy, 1, &copy_b) == D_OK);

    D_TEST_EXPR(strcmp(out_a, copy_a) == 0);
    D_TEST_EXPR(strcmp(out_b, copy_b) == 0);
    D_TEST_EXPR(out_a != copy_a);
    D_TEST_EXPR(out_b != copy_b);

    expect_size(&copy, 2);
    d_dyn_array_destroy(&copy);
    d_dyn_array_destroy(&src);
}

static void test_init_from_without_copy_fn_pointer_array_is_shallow(void)
{
    DDynArray src;
    DDynArray copy;
    char *s = owned_string("shallow");
    char *out_src = NULL;
    char *out_copy = NULL;

    D_TEST_EXPR(d_dyn_array_init_ptr_arr(&src, 0, NULL, NULL, ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(d_dyn_array_push_back_ptr(&src, s) == D_OK);
    D_TEST_EXPR(d_dyn_array_init_from(&copy, &src) == D_OK);
    D_TEST_EXPR(d_dyn_array_get_elem_at(&src, 0, &out_src) == D_OK);
    D_TEST_EXPR(d_dyn_array_get_elem_at(&copy, 0, &out_copy) == D_OK);
    D_TEST_EXPR(out_src == out_copy);
    free(s);
    d_dyn_array_destroy(&copy);
    d_dyn_array_destroy(&src);
}

static void test_init_from_propagates_copy_fn_to_the_copy(void)
{
    DDynArray src;
    DDynArray copy1;
    DDynArray copy2;
    char *src_s = NULL;
    char *copy1_s = NULL;
    char *copy2_s = NULL;

    D_TEST_EXPR(d_dyn_array_init_ptr_arr(&src, 0, string_ptr_free, string_ptr_copy, ARRAY_DEFAULT_OPTS) == D_OK);
    D_TEST_EXPR(d_dyn_array_push_back_ptr(&src, owned_string("propagate")) == D_OK);
    D_TEST_EXPR(d_dyn_array_init_from(&copy1, &src) == D_OK);
    D_TEST_EXPR(d_dyn_array_init_from(&copy2, &copy1) == D_OK);

    D_TEST_EXPR(d_dyn_array_get_elem_at(&src, 0, &src_s) == D_OK);
    D_TEST_EXPR(d_dyn_array_get_elem_at(&copy1, 0, &copy1_s) == D_OK);
    D_TEST_EXPR(d_dyn_array_get_elem_at(&copy2, 0, &copy2_s) == D_OK);

    D_TEST_EXPR(strcmp(src_s, copy1_s) == 0);
    D_TEST_EXPR(strcmp(src_s, copy2_s) == 0);
    D_TEST_EXPR(src_s != copy1_s);
    D_TEST_EXPR(copy1_s != copy2_s);
    D_TEST_EXPR(src_s != copy2_s);

    d_dyn_array_destroy(&copy2);
    d_dyn_array_destroy(&copy1);
    d_dyn_array_destroy(&src);
}

int main(void)
{
    DTest tests[] = {
        D_TEST_GENERATE_TEST(test_init_creates_empty_array),
        D_TEST_GENERATE_TEST(test_init_with_reserved_capacity_sets_capacity),
        D_TEST_GENERATE_TEST(test_init_ptr_arr_creates_empty_pointer_array),
        D_TEST_GENERATE_TEST(test_push_back_appends_single_int),
        D_TEST_GENERATE_TEST(test_push_back_copies_value_not_alias_source),
        D_TEST_GENERATE_TEST(test_push_back_grows_capacity_and_preserves_values),
        D_TEST_GENERATE_TEST(test_append_multiple_ints),
        D_TEST_GENERATE_TEST(test_append_after_push_preserves_order),
        D_TEST_GENERATE_TEST(test_append_zero_count_keeps_array_unchanged),
        D_TEST_GENERATE_TEST(test_get_elem_at_reads_first_middle_last),
        D_TEST_GENERATE_TEST(test_get_elem_at_rejects_index_equal_size),
        D_TEST_GENERATE_TEST(test_get_elem_at_rejects_index_from_empty_array),
        D_TEST_GENERATE_TEST(test_init_from_copies_size_capacity_and_values),
        D_TEST_GENERATE_TEST(test_init_from_does_not_alias_source_storage),
        D_TEST_GENERATE_TEST(test_push_back_ptr_stores_pointer_value),
        D_TEST_GENERATE_TEST(test_push_back_ptr_accepts_null_pointer_value),
        D_TEST_GENERATE_TEST(test_remove_last_on_empty_array_returns_empty_err),
        D_TEST_GENERATE_TEST(test_remove_last_returns_removed_value),
        D_TEST_GENERATE_TEST(test_remove_last_without_out_discards_value),
        D_TEST_GENERATE_TEST(test_remove_elem_fast_from_middle_swaps_last_into_removed_slot),
        D_TEST_GENERATE_TEST(test_remove_elem_fast_first_from_single_element_array),
        D_TEST_GENERATE_TEST(test_remove_elem_fast_rejects_index_equal_size),
        D_TEST_GENERATE_TEST(test_remove_elem_fast_rejects_remove_from_empty_array),
        D_TEST_GENERATE_TEST(test_remove_elem_fast_with_out_does_not_call_destructor),
        D_TEST_GENERATE_TEST(test_remove_elem_fast_without_out_calls_destructor_once),
        D_TEST_GENERATE_TEST(test_clear_empty_array_is_ok),
        D_TEST_GENERATE_TEST(test_clear_removes_all_elements_but_keeps_capacity),
        D_TEST_GENERATE_TEST(test_clear_calls_destructor_for_each_element),
        D_TEST_GENERATE_TEST(test_destroy_null_pointer_is_safe),
        D_TEST_GENERATE_TEST(test_destroy_empty_array_is_safe),
        D_TEST_GENERATE_TEST(test_destroy_calls_destructor_for_each_remaining_element),
        D_TEST_GENERATE_TEST(test_destructor_not_called_for_removed_owned_element_with_out),
        D_TEST_GENERATE_TEST(test_pointer_array_destructor_receives_element_slot_on_destroy),
        D_TEST_GENERATE_TEST(test_pointer_array_destructor_receives_element_slot_on_clear),
        D_TEST_GENERATE_TEST(test_pointer_array_destructor_receives_element_slot_on_remove_without_out),
        D_TEST_GENERATE_TEST(test_large_struct_elements_are_copied_exactly),
        D_TEST_GENERATE_TEST(test_many_mutations_preserve_valid_state),
        D_TEST_GENERATE_TEST(test_push_to_exact_power_of_two_then_one_more),
        D_TEST_GENERATE_TEST(test_append_then_clear_then_push_again),
        D_TEST_GENERATE_TEST(test_remove_elem_fast_last_element_is_swap_with_self),
        D_TEST_GENERATE_TEST(test_get_elem_at_does_not_alias_internal_storage),
        D_TEST_GENERATE_TEST(test_init_from_is_independent_after_source_clear),
        D_TEST_GENERATE_TEST(test_init_from_with_copy_fn_deep_copies_pointer_elements),
        D_TEST_GENERATE_TEST(test_init_from_without_copy_fn_pointer_array_is_shallow),
        D_TEST_GENERATE_TEST(test_init_from_propagates_copy_fn_to_the_copy),
    };

    D_TEST_RUN_TESTS(tests);
    return 0;
}
