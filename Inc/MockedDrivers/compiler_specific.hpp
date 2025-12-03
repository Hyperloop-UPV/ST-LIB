#pragma once
#include <cstdint>
/*

This file contains implementatios or altername names for
ARM GCC compiler that don't work in x86_64

*/
static inline uint32_t __RBIT(uint32_t val) {
    // 1. Hardware Byte Swap (Optimization: handles the large movements)
    // MSVC uses _byteswap_ulong, GCC/Clang uses __builtin_bswap32
#if defined(_MSC_VER)
    val = _byteswap_ulong(val);
#else
    val = __builtin_bswap32(val);
#endif

    // 2. Swap Nibbles (within bytes)
    // 0xF0 = 1111 0000 -> shifts to 0000 1111
    val = ((val & 0xF0F0F0F0) >> 4) | ((val & 0x0F0F0F0F) << 4);

    // 3. Swap Bit-Pairs (within nibbles)
    // 0xCC = 1100 1100 -> shifts to 0011 0011
    val = ((val & 0xCCCCCCCC) >> 2) | ((val & 0x33333333) << 2);

    // 4. Swap Single Bits (within pairs)
    // 0xAA = 1010 1010 -> shifts to 0101 0101
    val = ((val & 0xAAAAAAAA) >> 1) | ((val & 0x55555555) << 1);

    return val;
}                         
#define __CLZ                             __builtin_clz
#define __COMPILER_BARRIER()                asm volatile("" ::: "memory")
#define __DSB()                         __asm__ volatile ("mfence" ::: "memory");
#define __ISB()                         __asm__ volatile ("lfence" ::: "memory");