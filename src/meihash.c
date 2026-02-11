#include "meihash.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if defined(__SIZEOF_INT128__)
    typedef __uint128_t uint128_t;
#else
    #error "128-bit integer type is not supported on this platform."
#endif

static inline uint64_t rotl64(uint64_t x, unsigned int r) 
{
    return (x << r) | (x >> (64 - r));
}

static inline uint64_t read64(const void *ptr) 
{
    uint64_t val;
    memcpy(&val, ptr, sizeof(uint64_t));
    return val;
}

static inline uint32_t read32(const void *ptr) 
{
    uint32_t val;
    memcpy(&val, ptr, sizeof(uint32_t));
    return val;
}

static inline uint128_t meimix(uint64_t Mi, uint64_t Yi, uint64_t P0, uint64_t P1, uint64_t S0, uint64_t S1)
{
    // Mei Hash Mixer
    // Dual-MUM Folding pipeline
    
    // P Path (Seed Data)
    uint64_t Mp = Mi ^ P0;
    uint64_t Yp = Yi ^ P1;
    uint128_t Mk = (uint128_t)Mp * Yp;

    //S Path ("Secret" data)
    uint64_t Ms = Mi ^ S0;
    uint64_t Ys = Yi ^ S1;
    uint128_t Mz = (uint128_t)Ms * Ys;

    return Mk ^ Mz;
}

static const uint64_t P0_DEF = 0x59757a7541696861ULL; // "YuzuAiha" in ASCII
static const uint64_t P1_DEF = 0x4d65694169686172ULL; // "MeiAihar" in ASCII
static const uint64_t S0_DEF = 0x4d495a5553415741ULL; // "MIZUSAWA" in ASCII
static const uint64_t S1_DEF = 0x494c4f5645594f55ULL; // "ILOVEYOU" in ASCII

void meihash128(const void *input, size_t len, uint64_t seed, void *output)
{
    const uint8_t *ptr = (const uint8_t *)input;

    // Initialize state
    uint64_t p0 = P0_DEF ^ seed;
    uint64_t p1 = P1_DEF ^ rotl64(seed, 32);
    uint64_t s0 = S0_DEF ^ seed;
    uint64_t s1 = S1_DEF;

    uint64_t final_m = 0;
    uint64_t final_y = 0;

    // Accumulators
    uint64_t accumulator1 = P1_DEF;
    uint64_t accumulator2 = P0_DEF;

    uint128_t lane1 = 0;
    uint128_t lane2 = 0;
    uint128_t lane3 = 0;
    uint128_t lane4 = 0;

    while (len >= 64)
    {
        uint64_t mix1 = read64(ptr);
        uint64_t mix2 = read64(ptr + 8);
        lane1 ^= meimix(mix1, mix2, p0, p1, s0, s1);

        uint64_t mix3 = read64(ptr + 16);
        uint64_t mix4 = read64(ptr + 24);
        lane2 ^= meimix(mix3, mix4, p0, p1, s0, s1);

        uint64_t mix5 = read64(ptr + 32);
        uint64_t mix6 = read64(ptr + 40);
        lane3 ^= meimix(mix5, mix6, p0, p1, s0, s1);

        uint64_t mix7 = read64(ptr + 48);
        uint64_t mix8 = read64(ptr + 56);
        lane4 ^= meimix(mix7, mix8, p0, p1, s0, s1);

        accumulator1 ^= (uint64_t)lane1; accumulator2 ^= (uint64_t)(lane1 >> 64);

        accumulator1 ^= rotl64(accumulator1, 1); accumulator2 ^= rotl64(accumulator2, 1);
        accumulator1 ^= (uint64_t)lane2; accumulator2 ^= (uint64_t)(lane2 >> 64);

        accumulator1 ^= rotl64(accumulator1, 1); accumulator2 ^= rotl64(accumulator2, 1);
        accumulator1 ^= (uint64_t)lane3; accumulator2 ^= (uint64_t)(lane3 >> 64);

        accumulator1 ^= rotl64(accumulator1, 1); accumulator2 ^= rotl64(accumulator2, 1);
        accumulator1 ^= (uint64_t)lane4; accumulator2 ^= (uint64_t)(lane4 >> 64);

        uint64_t temp = accumulator1;
        accumulator1 = rotl64(accumulator1, 1) + accumulator2;
        accumulator2 = rotl64(temp, 1) + accumulator1;

        ptr += 64;
        len -= 64;
    }

    while (len >= 16)
    {
        uint64_t mix1 = read64(ptr);
        uint64_t mix2 = read64(ptr + 8);
        uint128_t mixed = meimix(mix1, mix2, p0, p1, s0, s1);

        accumulator1 ^= (uint64_t)mixed;
        accumulator2 ^= (uint64_t)(mixed >> 64);

        uint64_t temp = accumulator1;
        accumulator1 = rotl64(accumulator1, 1) + accumulator2;
        accumulator2 = rotl64(temp, 1) + accumulator1;

        ptr += 16;
        len -= 16;
    }

    if (len > 0)
    {
        switch (len)
        {
            case 15: final_y |= ((uint64_t)ptr[14]) << 48;
            case 14: final_y |= ((uint64_t)ptr[13]) << 40;
            case 13: final_y |= ((uint64_t)ptr[12]) << 32;
            case 12: final_y |= ((uint64_t)ptr[11]) << 24;
            case 11: final_y |= ((uint64_t)ptr[10]) << 16;
            case 10: final_y |= ((uint64_t)ptr[9]) << 8;
            case 9:  final_y |= ((uint64_t)ptr[8]);
            case 8:  final_m |= read64(ptr);
                     break;
            case 7:  final_m |= ((uint64_t)ptr[6]) << 48;
            case 6:  final_m |= ((uint64_t)ptr[5]) << 40;
            case 5:  final_m |= ((uint64_t)ptr[4]) << 32;
            case 4:  final_m |= ((uint64_t)ptr[3]) << 24;
            case 3:  final_m |= ((uint64_t)ptr[2]) << 16;
            case 2:  final_m |= ((uint64_t)ptr[1]) << 8;
            case 1:  final_m |= (uint64_t)ptr[0]; break;
        }

        uint128_t meimixed = meimix(final_m, final_y, p0, p1, s0, s1);
        accumulator1 ^= (uint64_t)meimixed;
        accumulator2 ^= (uint64_t)(meimixed >> 64);

    }

    accumulator1 ^= len;

    // Avalanche
    uint128_t finalmix = (uint128_t)accumulator1 * s0;
    finalmix ^= (uint128_t)accumulator2 * p0;

    accumulator1 = (uint64_t)finalmix ^ (uint64_t)(finalmix >> 64);
    accumulator2 = (uint64_t)(finalmix >> 64) + accumulator1;

    accumulator1 ^= accumulator1 >> 33; accumulator1 *= p0;
    accumulator1 ^= accumulator1 >> 33; accumulator1 *= s1;
    accumulator1 ^= accumulator1 >> 33;

    accumulator2 ^= accumulator2 >> 33; accumulator2 *= p1;
    accumulator2 ^= accumulator2 >> 33; accumulator2 *= s0;
    accumulator2 ^= accumulator2 >> 33;

    // Finalize output
    ((uint64_t *)output)[0] = accumulator1;
    ((uint64_t *)output)[1] = accumulator2;
}