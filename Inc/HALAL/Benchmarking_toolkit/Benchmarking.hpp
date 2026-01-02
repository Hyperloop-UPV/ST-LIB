#pragma once

#include <array>
#include <inttypes.h>
#include <algorithm>
#include <string_view> 

#include "stm32h723xx.h"
#include "core_cm7.h"
#include "SEGGER_RTT.h"

enum class BenchmarkType : uint8_t{
    NONE,
    RUNTIME,
    GDB
};
enum BenchmarkLoggingTypes{
    _NONE,
    _SD = 1,
    _RTT = 2
};

struct TimeStamp{
    uint32_t id;
    uint32_t cycles;
};


static inline TimeStamp timestamp;
/**
 * This hash is used to send a lightweight identifier via RTT
 * on the host the source code is searched for BENCHMARK_MACROS
 * and hashed using the same hash algorithm, thus creating a 1:1
 * match between hash and string
 */
consteval uint32_t custom_hash(std::string_view str){
    uint32_t hash = 2166136261u; // FNV offset basis
    
    for (char c : str) {
        hash ^= (unsigned char)c;
        hash *= 16777619u; // FNV prime
    }
    
    return hash;
}
#if defined(RUNTIME_BENCHMARK) && defined(GDB_BENCHMARK)
    #error "Cannot use RUNTIME_BENCHMARK and GDB_BENCHMARK SIMULTANEOUSLY"
#endif

extern "C"{
#ifdef RUNTIME_BENCHMARK
    constexpr BenchmarkType benchmark_type = BenchmarkType::RUNTIME;
    static const char BENCHMARK_TYPE_RUNTIME[] __attribute__((used)) = "BENCHMARK_RUNTIME";
#elif defined(GDB_BENCHMARK)
    constexpr BenchmarkType benchmark_type = BenchmarkType::GDB;
    static const char BENCHMARK_TYPE_GDB[] __attribute__((used)) = "BENCHMARK_GDB";
#else 
    constexpr BenchmarkType benchmark_type = BenchmarkType::NONE;
    static const char BENCHMARK_TYPE_NONE[] __attribute__((used)) = "BENCHMARK_NONE";
#endif 

}

#if defined(RUNTIME_BENCHMARK)
#define RUNTIME_BENCHMARK_TAG  _RTT
#else
#define RUNTIME_BENCHMARK_TAG  _NONE
#endif

#if defined(SD)
#define SD_BENCHMARK_TAG  _SD
#else
#define SD_BENCHMARK_TAG  _NONE
#endif
inline constexpr int logging_type =  RUNTIME_BENCHMARK_TAG | SD_BENCHMARK_TAG;
template<BenchmarkType Type>
class Benchmarker{
public:
    Benchmarker(){
        DWT->CTRL |= 1;
        DWT->CYCCNT = 0;
        if constexpr(benchmark_type == BenchmarkType::RUNTIME){
            SEGGER_RTT_Init();
        }
    }
    template<uint32_t Id>
    __attribute__((always_inline)) inline static void __benchmark_start__(){
        if constexpr(benchmark_type == BenchmarkType::GDB){
            __asm__ volatile("BKPT \n");
        } else if constexpr(benchmark_type == BenchmarkType::RUNTIME){
            uint32_t cycles = DWT->CYCCNT;
            if constexpr( logging_type & _RTT){
                timestamp.id = Id;
                timestamp.cycles = cycles;
                SEGGER_RTT_Write(0,&timestamp,sizeof(TimeStamp));
            }
        }
    }
    template<uint32_t Id>
    __attribute__((always_inline)) inline static void __benchmark_end__(){
        if constexpr(benchmark_type == BenchmarkType::GDB){
            __asm__ volatile("BKPT \n");
        } else if constexpr(benchmark_type == BenchmarkType::RUNTIME){
            uint32_t cycles = DWT->CYCCNT;
            if constexpr( logging_type & _RTT){
                timestamp.id = Id;
                timestamp.cycles = cycles;
                SEGGER_RTT_Write(0,&timestamp,sizeof(TimeStamp));
            }
        }
    }
};

#define BENCHMARK_SETUP() \
    Benchmarker<benchmark_type> __b;
#define BENCHMARK_START(NAME) \
    Benchmarker<benchmark_type>::__benchmark_start__<custom_hash(NAME)>();
#define BENCHMARK_END(NAME)\
    Benchmarker<benchmark_type>::__benchmark_end__<custom_hash(NAME)>(); 
