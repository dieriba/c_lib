#ifndef RAW_MAP_H
#define RAW_MAP_H
#include "d_types.h"

typedef usize (*HashFn)(void *key);
typedef bool (*CmpFn)(void*, void*);
typedef void (*FreeFn)(void *elem);

typedef struct RawMap {
    void* map;
    usize key_size;
    usize value_size;
    usize capacity;
    usize len;
    usize nb_groups;
    HashFn hash_fn;
    FreeFn free_fn;
    CmpFn cmp_fn;
} RawMap;

#endif