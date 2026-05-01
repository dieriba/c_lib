#include <immintrin.h>
#include <string.h>
#include <assert.h>

#include "d_types.h"
#include "d_math.h"
#include "raw_map.h"
#include "d_bits.h"

#define RAW_MAP_DEFAULT_NB_GROUPS 2UL
#define RAW_MAP_GROUP_SIZE 16UL
#define RAW_MAP_MIN_CAPACITY (RAW_MAP_DEFAULT_NB_GROUPS * RAW_MAP_GROUP_SIZE)
#define SIMD_REQUIRED_ALIGNEMENT 0x10

#define raw_map_compute_h1(hash) (hash >> 7)
#define raw_map_compute_h2(hash) (hash & 0x7F)

#define raw_map_get_slot_size(raw_map) (raw_map->key_size + raw_map->value_size)

#define raw_map_get_slot_group_start_addr(raw_map, group_number) (char *)raw_map->map + (raw_map->nb_groups * RAW_MAP_GROUP_SIZE) + ((raw_map_get_slot_size(raw_map) * RAW_MAP_GROUP_SIZE) * group_number)
#define raw_map_get_slot_key_from_group_addr(group_addr, key_pos) (char *)group_addr + (key_pos * raw_map_get_slot_size(raw_map))

#define raw_map_get_slot_key(raw_map, key_pos) (char *)raw_map->map + (raw_map_get_slot_size(raw_map) * key_pos)
#define raw_map_get_slot_value(raw_map, key_pos) raw_map_get_slot_key(raw_map, key_pos) + raw_map->key_size

#define raw_map_get_control_byte_group_start_addr(raw_map, group_number) (char *)raw_map->map + (group_number * RAW_MAP_GROUP_SIZE)
#define raw_map_get_control_byte_addr(raw_map, control_byte_pos) (char *)raw_map->map + control_byte_pos

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

static inline void init_raw_map_metadata(void *addr, usize metadata_size)
{
    memset(addr, kEmpty, metadata_size);
}

static RawMap *init_raw_map(RawMap *raw_map, usize key_size, usize value_size, usize capacity, HashFn hash_fn, CmpFn cmp_fn, FreeFn free_fn)
{
    if (raw_map == NULL)
        return NULL;

    usize slot_size;
    if (d_overflow_check_add_usize(capacity, RAW_MAP_GROUP_SIZE, &capacity) || d_overflow_check_add_usize(key_size, value_size, &slot_size))
        return NULL;
    capacity = ALIGN_ROUND_DOWN(capacity, RAW_MAP_GROUP_SIZE);
    assert(capacity % RAW_MAP_GROUP_SIZE == 0);

    usize total_group_size;
    usize alloc_size;
    if (d_overflow_check_mul_usize(capacity, slot_size, &total_group_size) || d_overflow_check_add_usize(capacity, total_group_size, &alloc_size))
        return NULL;
    if (posix_memalign(&raw_map->map, SIMD_REQUIRED_ALIGNEMENT, alloc_size) != 0)
        return NULL;
    init_raw_map_metadata(raw_map->map, capacity);
    raw_map->nb_groups = capacity / RAW_MAP_GROUP_SIZE;
    raw_map->capacity = capacity;
    raw_map->value_size = value_size;
    raw_map->key_size = key_size;
    raw_map->len = 0;
    raw_map->cmp_fn = cmp_fn;
    raw_map->hash_fn = hash_fn == NULL ? default_hash_fn : hash_fn;
    raw_map->free_fn = free_fn;
    return raw_map;
}

static DBits32 get_mask_with_needle_pos_in_haystack(void *haystack, u8 needle)
{
    __m128 to_match = _mm_stream_load_si128(haystack);
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
    char *ctrls = raw_map_get_control_byte_group_start_addr(raw_map, group_number);
    while (true)
    {
        usize position = try_get_pos_deleted_in_group(ctrls);
        if (position != (usize)-1)
            return (group_number * RAW_MAP_GROUP_SIZE) + position;
        group_number = (group_number + 1) % raw_map->nb_groups;
        ctrls += RAW_MAP_GROUP_SIZE;
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
        if (empty_pos != (usize)-1)
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

static bool is_load_factor_reached(RawMap *raw_map)
{
    return raw_map;
}

static RawMap *rehash(RawMap *raw_map, void *key, void *value)
{
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
    usize position = find_from_hash(raw_map, key, hash_info);
    assert(position != SIZE_MAX);

    if (is_load_factor_reached(raw_map))
    {
        if (rehash(raw_map, key, value) == NULL)
            return NULL;
        return raw_map;
    }

    char *ctrl = raw_map_get_control_byte_addr(raw_map, position);
    if (d_bits_8_check_bits_set(*ctrl, MASK_CTRL_BYTE))
    {
        usize deleted_slot_position = try_find_deleted_slot_pos_from_group(raw_map, hash_info.group_number);
        if (deleted_slot_position < position)
            position = deleted_slot_position;
        ctrl = raw_map_get_control_byte_addr(raw_map, position);
    }
    *ctrl = hash_info.h2; // update control byte
    make_insert(raw_map, position, key, value);
    raw_map->len++;
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
    }
    if (out_elem)
        memcpy(out_elem, raw_map_get_slot_value(raw_map, position), raw_map->value_size);
    return true;
}