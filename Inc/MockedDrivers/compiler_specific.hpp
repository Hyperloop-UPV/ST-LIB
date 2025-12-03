#pragma once
#include <cstdint>
/*

This file contains implementatios or altername names for
ARM GCC compiler that don't work in x86_64

*/
#define __RBIT                            
#define __CLZ                             __builtin_clz
#define __COMPILER_BARRIER()                asm volatile("" ::: "memory")
#define __DSB()                         __asm__ volatile ("mfence" ::: "memory");
#define __ISB()                         __asm__ volatile ("lfence" ::: "memory");