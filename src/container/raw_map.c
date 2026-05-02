#include <immintrin.h>
#include <string.h>
#include <assert.h>

#include "container.h"
#include "d_types.h"
#include "d_math.h"
#include "raw_map.h"
#include "d_bits.h"

#define RAW_MAP_DEFAULT_NB_GROUPS 2UL
#define RAW_MAP_GROUP_SIZE 16UL

#define SIMD_REQUIRED_ALIGNEMENT 0x10

#define raw_map_compute_h1(hash) (hash >> 7)
#define raw_map_compute_h2(hash) (hash & 0x7F)

#define raw_map_compute_slot_size(raw_map) (raw_map->key_size + raw_map->value_size)

#define raw_map_get_slot_group_start_addr(raw_map, group_number) (char *)raw_map->map + (raw_map->nb_groups * RAW_MAP_GROUP_SIZE) + ((raw_map_compute_slot_size(raw_map) * RAW_MAP_GROUP_SIZE) * group_number)
#define raw_map_get_slot_key_from_group_addr(group_addr, key_pos) (char *)group_addr + (key_pos * raw_map_compute_slot_size(raw_map))

#define map_get_slot_key_from_map_addr(map_addr, nb_groups, slot_size, key_pos) (char *)map_addr + (nb_groups * RAW_MAP_GROUP_SIZE) + (slot_size * key_pos)
#define map_get_slot_value_from_map_addr(map_addr, nb_groups, slot_size, key_size, key_pos) map_get_slot_key_from_map_addr(map_addr, nb_groups, slot_size, key_pos) + key_size

#define raw_map_get_slot_key(raw_map, key_pos) map_get_slot_key_from_map_addr(raw_map->map, raw_map->nb_groups, raw_map_compute_slot_size(raw_map), key_pos)
#define raw_map_get_slot_value(raw_map, key_pos) map_get_slot_value_from_map_addr(raw_map->map, raw_map->nb_groups, raw_map_compute_slot_size(raw_map), raw_map->key_size, key_pos)

#define map_get_control_byte_group_start_addr(map_addr, group_number) (char *)map_addr + (group_number * RAW_MAP_GROUP_SIZE)

#define raw_map_get_control_byte_group_start_addr(raw_map, group_number) (char *)raw_map->map + (group_number * RAW_MAP_GROUP_SIZE)
#define raw_map_get_control_byte_addr(raw_map, control_byte_pos) (char *)raw_map->map + control_byte_pos

#define MAX_LOAD_FACTOR 0.7
#define MASK_CTRL_BYTE 0x80

typedef enum RawMapCtrl
{
    kEmpty = -128, // 0b10000000
    kDeleted = -2, // 0b11111110
} RawMapCtrl;

typedef struct HashInfo
{
    usize group_number;
    u8 h2;
} HashInfo;

static usize default_hash_fn(void *key)
{
}

static RawMap *new_raw_map()
{
    return malloc(sizeof(RawMap));
}

static RawMap *init_raw_map_map(RawMap *raw_map, usize capacity, usize alloc_size)
{
    if (posix_memalign(&raw_map->map, SIMD_REQUIRED_ALIGNEMENT, alloc_size) != 0)
        return NULL;
    memset(raw_map->map, kEmpty, capacity);
    raw_map->nb_groups = capacity / RAW_MAP_GROUP_SIZE;
    raw_map->capacity = capacity;
    raw_map->len = 0;
    raw_map->max_load_factor = MAX_LOAD_FACTOR * capacity;
    return raw_map;
}

static RawMap *compute_alloc_size_and_capacity(RawMap *raw_map, usize slot_size, usize *capacity, usize *alloc_size)
{
    usize new_capacity = *capacity;
    new_capacity = new_capacity == 0 ? DEFAULT_CAPACITY : new_capacity;
    if (d_mathcheck_add_usize(new_capacity, RAW_MAP_GROUP_SIZE, &new_capacity))
        return NULL;
    new_capacity = d_math_align_round_down(new_capacity, RAW_MAP_GROUP_SIZE);
    assert(new_capacity % RAW_MAP_GROUP_SIZE == 0);
    *capacity = new_capacity;

    usize total_group_size;
    if (d_mathcheck_mul_usize(new_capacity, slot_size, &total_group_size) || d_mathcheck_add_usize(new_capacity, total_group_size, alloc_size))
        return NULL;

    return raw_map;
}

static RawMap *init_raw_map(RawMap *raw_map, usize key_size, usize value_size, usize capacity, HashFn hash_fn, CmpFn cmp_fn, FreeFn free_fn)
{
    if (raw_map == NULL)
        return NULL;

    usize slot_size;
    if (d_mathcheck_add_usize(key_size, value_size, &slot_size))
        return NULL;
    usize alloc_size;
    if (compute_alloc_size_and_capacity(raw_map, slot_size, &capacity, &alloc_size) == NULL)
        return NULL;
    if (init_raw_map_map(raw_map, capacity, alloc_size) == NULL)
        return NULL;

    raw_map->value_size = value_size;
    raw_map->key_size = key_size;
    raw_map->cmp_fn = cmp_fn;
    raw_map->hash_fn = hash_fn == NULL ? default_hash_fn : hash_fn;
    raw_map->free_fn = free_fn;
    return raw_map;
}

static DBits32 get_mask_with_needle_pos_in_haystack(void *haystack, u8 needle)
{
    __m128 to_match = _mm_load_si128(haystack);
    __m128 filter = _mm_set1_epi8(needle);
    __m128 filtered = _mm_cmpeq_epi8(to_match, filter);
    return _mm_movemask_epi8(filtered);
}

static usize try_get_pos_empty_flag_from_ctrl_group(void *ctrl_group_addr)
{
    DBits32 mask = get_mask_with_needle_pos_in_haystack(ctrl_group_addr, kEmpty);
    return d_bits_get_index_least_significant_bit_set_int(mask);
}

static usize try_get_pos_deleted_in_group(void *addr_group)
{
    DBits32 mask = get_mask_with_needle_pos_in_haystack(addr_group, kDeleted);
    return d_bits_get_index_least_significant_bit_set_int(mask);
}

static usize try_find_key_pos_in_group(RawMap *raw_map, void *key, usize group_number, u8 h2)
{
    void *ctrls = raw_map_get_control_byte_group_start_addr(raw_map, group_number);
    DBits32 candidate_keys = get_mask_with_needle_pos_in_haystack(ctrls, h2);
    void *slot_group_addr = raw_map_get_slot_group_start_addr(raw_map, group_number);
    while (candidate_keys != 0)
    {
        int16 candidate_pos = d_bits_get_index_least_significant_bit_set_int((int)candidate_keys);
        void *candidate_key = raw_map_get_slot_key_from_group_addr(slot_group_addr, candidate_pos);
        if (raw_map->cmp_fn(key, candidate_key))
            return (group_number * RAW_MAP_GROUP_SIZE) + candidate_pos;
        candidate_keys = d_bits_clear_least_significant_bit_set(candidate_keys);
    }
    return SIZE_MAX;
}

static usize try_find_deleted_slot_pos_from_group(RawMap *raw_map, usize group_number)
{
    usize nb_groups_read = 0;
    while (nb_groups_read < raw_map->nb_groups)
    {
        char *ctrls = raw_map_get_control_byte_group_start_addr(raw_map, group_number);
        usize position = try_get_pos_deleted_in_group(ctrls);
        if (position != SIZE_MAX)
            return (group_number * RAW_MAP_GROUP_SIZE) + position;
        group_number = (group_number + 1) % raw_map->nb_groups;
        nb_groups_read++;
    }

    return SIZE_MAX;
}

static usize find_from_hash(RawMap *raw_map, void *key, HashInfo hash_info)
{
    usize group_number = hash_info.group_number;
    while (true)
    {
        usize key_pos = try_find_key_pos_in_group(raw_map, key, group_number, hash_info.h2);
        if (key_pos != SIZE_MAX)
            return key_pos;
        void *ctrl_group_addr = raw_map_get_control_byte_group_start_addr(raw_map, group_number);
        usize empty_pos = try_get_pos_empty_flag_from_ctrl_group(ctrl_group_addr);
        if (empty_pos != SIZE_MAX)
            return empty_pos;
        group_number = (group_number + 1) % raw_map->nb_groups;
    }
    return SIZE_MAX;
}

static bool compute_hash(RawMap *raw_map, void *key, HashInfo *hash_info)
{
    if (raw_map == NULL || key == NULL)
        return false;
    usize hash = raw_map->hash_fn(key);
    *hash_info = (HashInfo){
        .group_number = raw_map_compute_h1(hash) % raw_map->nb_groups,
        .h2 = raw_map_compute_h2(hash),
    };
    return true;
}

static void make_insert(RawMap *raw_map, usize position, void *key, void *value)
{
    void *slot_key = raw_map_get_slot_key(raw_map, position);
    void *slot_value = (char *)slot_key + raw_map->key_size;
    memcpy(slot_key, key, raw_map->key_size);
    memcpy(slot_value, value, raw_map->value_size);
}

static void copy_slot_from_old_map_to_new_map(RawMap *raw_map, void *old_map, usize old_group_number, usize total_occupied_slot, void *new_key, void *new_value)
{
    usize copied_slot = 0;
    usize group_number = 0;
    __m128i mask = _mm_set1_epi8(MASK_CTRL_BYTE);
    while (copied_slot != total_occupied_slot)
    {
        void *ctrls = map_get_control_byte_group_start_addr(old_map, group_number);
        __m128i haystack = _mm_load_si128(ctrls);
        __m128i haystack_with_only_ctrl_b = _mm_and_si128(haystack, mask);
        DBits32 mask_occupied_slot = get_mask_with_needle_pos_in_haystack(&haystack_with_only_ctrl_b, 0);
        while (mask_occupied_slot != 0)
        {
            usize key_pos_in_mask = d_bits_get_index_least_significant_bit_set_int(mask_occupied_slot);
            usize key_pos = (RAW_MAP_GROUP_SIZE * group_number) + key_pos_in_mask;
            raw_map_insert(raw_map,
                           map_get_slot_key_from_map_addr(old_map, old_group_number, raw_map_compute_slot_size(raw_map), key_pos),
                           map_get_slot_value_from_map_addr(old_map, old_group_number, raw_map_compute_slot_size(raw_map), raw_map->key_size, key_pos));
            mask_occupied_slot = d_bits_clear_least_significant_bit_set(mask_occupied_slot);
            copied_slot++;
        }
        group_number++;
    }
    raw_map_insert(raw_map, new_key, new_value);
}

static RawMap *rehash(RawMap *raw_map, void *key, void *value)
{
    usize new_capacity;

    if (d_math_overflow_check_mul_usize(raw_map->capacity, GROWTH_POLICY, &new_capacity))
        return NULL;
    usize alloc_size;
    if (compute_alloc_size_and_capacity(raw_map, raw_map_compute_slot_size(raw_map), &new_capacity, &alloc_size) == false)
        return NULL;

    void *old_map = raw_map->map;
    usize total_occupied_slot = raw_map->len;
    usize old_group_number = raw_map->nb_groups;
    if (init_raw_map_map(raw_map, new_capacity, alloc_size) == NULL)
        return NULL;
    copy_slot_from_old_map_to_new_map(raw_map, old_map, old_group_number, total_occupied_slot, key, value);
    free(old_map);
    return raw_map;
}

RawMap *raw_map_new(usize key_size, usize value_size, usize capacity, HashFn hash_fn, CmpFn cmp_fn, FreeFn free_fn)
{
    RawMap *raw_map = new_raw_map();

    return init_raw_map(raw_map, key_size, value_size, capacity, hash_fn, cmp_fn, free_fn);
}

void *raw_map_get(RawMap *raw_map, void *key)
{
    HashInfo hash_info;
    if (compute_hash(raw_map, key, &hash_info) == false)
        return NULL;
    int position = find_from_hash(raw_map, key, hash_info);
    assert(position != SIZE_MAX);
    char *ctrl = raw_map_get_control_byte_addr(raw_map, position);
    return d_bits_8_check_bits_set(*ctrl, MASK_CTRL_BYTE) ? NULL : raw_map_get_slot_value(raw_map, position);
}

RawMap *raw_map_insert(RawMap *raw_map, void *key, void *value)
{
    HashInfo hash_info;
    if (value == NULL || compute_hash(raw_map, key, &hash_info) == false)
        return NULL;
    usize insert_position = find_from_hash(raw_map, key, hash_info);
    assert(insert_position != SIZE_MAX);

    if (raw_map->len >= raw_map->max_load_factor)
    {
        if (rehash(raw_map, key, value) == NULL)
            return NULL;
        return raw_map;
    }

    char *ctrl = raw_map_get_control_byte_addr(raw_map, insert_position);
    if (d_bits_8_check_bits_set(*ctrl, MASK_CTRL_BYTE))
    {
        usize deleted_slot_position = try_find_deleted_slot_pos_from_group(raw_map, hash_info.group_number);
        if (deleted_slot_position != SIZE_MAX)
        {
            usize start_group_position = hash_info.group_number * RAW_MAP_GROUP_SIZE;
            usize rel_distance_insert_pos_from_start_grp_pos = insert_position - start_group_position;
            usize rel_distance_del_pos_from_start_grp_pos = deleted_slot_position - start_group_position;
            if (rel_distance_del_pos_from_start_grp_pos < rel_distance_insert_pos_from_start_grp_pos)
                insert_position = deleted_slot_position;
        }
        ctrl = raw_map_get_control_byte_addr(raw_map, insert_position);
        raw_map->len++;
    }
    *ctrl = hash_info.h2; // update control byte
    make_insert(raw_map, insert_position, key, value);
    return raw_map;
}

bool raw_map_remove(RawMap *raw_map, void *key, void *out_elem)
{
    HashInfo hash_info;
    if (compute_hash(raw_map, key, &hash_info) == false)
        return NULL;
    int position = find_from_hash(raw_map, key, hash_info);
    assert(position != SIZE_MAX);

    char *ctrl = raw_map_get_control_byte_addr(raw_map, position);
    if (*ctrl != kEmpty)
    {
        *ctrl = kDeleted;
        raw_map->len--;
        if (out_elem)
            memcpy(out_elem, raw_map_get_slot_value(raw_map, position), raw_map->value_size);
        return true;
    }
    return false;
}