#include "d_hash.h"
#include "wyhash.h"

u64 d_hash_raw_hash(void* data, size_t len, u64 seed)
{
    return wyhash(data, len, seed, _wyp);
}