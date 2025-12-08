
#pragma once
#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Models/PinModel/Pin.hpp"
#include "core_cm7.h"
#include "HALAL/Benchmarking_toolkit/Benchmarking.hpp"
#if !defined DWT_LSR_Present_Msk
#define DWT_LSR_Present_Msk ITM_LSR_Present_Msk
#endif
#if !defined DWT_LSR_Access_Msk
#define DWT_LSR_Access_Msk ITM_LSR_Access_Msk
#endif
#define DWT_LAR_KEY 0xC5ACCE55
#define DEMCR_TRCENA 0x01000000
#define DWT_CTRL_CYCCNTENA 0x00000001

extern void increment_overflow();

/*
    This class is designed to study the efficiency of an algorithm 
    by counting the number of clock cycles required for execution. 
    It provides a more predictable and reliable method for measuring 
    performance compared to using time-based measurements.
*/
/*
 To use this class you should use:
    - start_count() -> and store in a variable the CYCCNT (this should be really close to zero)
    - stop_count() -> and store in a variable the actual CYCCNT.
    The difference will be the number of clock cycles done in the algorithm
*/
class DataWatchpointTrace {
   public:
    static uint32_t num_overflows;
    static void enable() {
        DWT->CTRL |= DWT_CTRL_CYCCNTENA | DWT_CTRL_EXCTRCENA_Msk | DWT_CTRL_EXCEVTENA_Msk;  // enables the counter
        DWT->FUNCTION0 |= (1 << 7) | 0b1000; // enable comparison
        DWT->COMP0 = 0xFFFFFFFF; // detect overflow
    }
    /**
     * needs to be done via polling because there is no way to trigger an interrupt
     */
    static void check_overflow(){
        uint32_t cached_function_0 = DWT->FUNCTION0;
        if( cached_function_0 &= DWT_FUNCTION_MATCHED_Msk){
            increment_overflow();
        }
    }
    static void disable(){
        DWT->CTRL &= ~DWT_CTRL_CYCCNTENA;  // disable the counter
    }

    static unsigned int get_count() {
        return DWT->CYCCNT;  // returns the current value of the counter
    }

   private:
    static void reset_cnt() {
        DWT->CYCCNT = 0;  // reset the counter

    }
};