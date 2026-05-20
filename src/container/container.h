#ifndef CONTAINER_H
#define CONTAINER_H
#include <stdbool.h>
#include "d_error.h"
#include "d_types.h"
#define GROWTH_POLICY 2UL
#define DEFAULT_CAPACITY 16UL

#define ASSERT_FIRST_FIELD(t, field) _Static_assert(offsetof(t, field) == 0, #t ": " #field " must be the first field")

DResult container_next_pow2_checked(usize *pow2);

#endif