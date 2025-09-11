#pragma once
#include <cstdint>
#include <iostream>
#include <bitset>

#include "HALAL/Benchmarking_toolkit/DataWatchpointTrace/DataWatchpointTrace.hpp"
#include "SEGGER_RTT.h"


enum EVENTS : uint32_t{
    SIMPLE_MARK = 1,
    ISR = 2,
    ETH_TX = 4,
    ETH_RX = 8,
    SPI = 16,
    CONTROL_ALGORITHM = 32
};


template<EVENTS... Ev>
constexpr uint32_t configure_benchmarks(){
    return (Ev | ...);
}
inline constexpr uint32_t benchmark_configuration = configure_benchmarks<SIMPLE_MARK>();

struct Performace_Packet{
    uint32_t initial_timestamp;
    uint32_t final_timestamp;
    uint32_t event_type;
    uint32_t event_id;
};

template<uint32_t Configuration,EVENTS E = SIMPLE_MARK,uint32_t ID>
constexpr void _benchmark_begin(Performace_Packet* p){

    if constexpr( Configuration & E){
        p->event_id = (uint32_t)ID;
        p->event_type = static_cast<uint32_t>(E);
        // we cannot set DWT->CYCCNT to 0 every time want to perform a benchmark
        // because other benchmarks might be taking place
        // so now we need to handle overflow
        p->initial_timestamp = DWT->CYCCNT;
        asm volatile("" ::: "memory");//acts as a memory barrier at compile time, so instructions are not reordered

    }
    else{
        //THIS EVENT IS NOT ENABLED
    }
}

template<uint32_t Configuration,EVENTS E = SIMPLE_MARK,uint32_t ID>
constexpr void _benchmark_end(Performace_Packet* p){
    if constexpr(Configuration & E){
        if(DWT->CYCCNT < p->initial_timestamp){
            p->final_timestamp = DWT->CYCCNT + (0xFFFFFFFF) - p->initial_timestamp;
        }else{
            p->final_timestamp = DWT->CYCCNT;
        }
    }
    else{
        //THIS EVENT IS NOT ENABLED
    }
}


/**
 * 
 */
#define BENCHMARK_BEGIN(TYPE,ID)      \
{                                              \
    constexpr bool begin_##ID##_found = true;       \
    Performace_Packet p; \
    _benchmark_begin<benchmark_configuration,TYPE,(uint32_t)ID>(&p);  \
        

/**
 * 
 */
#define BENCHMARK_END(TYPE,ID) \
    static_assert(begin_##ID##_found == true,"NO MATCHING BENCHMARK BEGIN FOUND IN CURRENT SCOPE"); \
    _benchmark_end<benchmark_configuration,TYPE,(uint32_t)ID>(&p); \
    SEGGER_RTT_Write(0,&p,sizeof(Performace_Packet)); \
} // closing the scope opened by BENCHMARK_BEGIN
