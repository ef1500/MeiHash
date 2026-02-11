#ifndef MEIHASH_H
#define MEIHASH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" 
{
#endif
    void meihash128(const void *key, size_t len, uint64_t seed, void *out);

#ifdef __cplusplus
}
#endif
#endif // MEIHASH_H