
#pragma once
#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Models/PinModel/Pin.hpp"
#include "core_cm7.h"

#if !defined DWT_LSR_Present_Msk
#define DWT_LSR_Present_Msk ITM_LSR_Present_Msk
#endif
#if !defined DWT_LSR_Access_Msk
#define DWT_LSR_Access_Msk ITM_LSR_Access_Msk
#endif
#define DWT_LAR_KEY 0xC5ACCE55
#define DEMCR_TRCENA 0x01000000
#define DWT_CTRL_CYCCNTENA 0x00000001
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
    static void start() {
        unlock_dwt();
        reset_cnt();
    }
    static unsigned int start_count() {
        DWT->CTRL |= DWT_CTRL_CYCCNTENA;  // enables the counter
        DWT->CYCCNT = 0;
        return DWT->CYCCNT;
    }
    static unsigned int stop_count() {
        DWT->CTRL &= ~DWT_CTRL_CYCCNTENA;  // disable the counter
        return DWT->CYCCNT;
    }
    static unsigned int get_count() {
        return DWT->CYCCNT;  // returns the current value of the counter
    }

   private:
    static void reset_cnt() {
        CoreDebug->DEMCR |= DEMCR_TRCENA;
        DWT->CYCCNT = 0;  // reset the counter
        DWT->CTRL = 0;
    }

    static void unlock_dwt() {  // unlock the dwt
        uint32_t lsr = DWT->LSR;
        if ((lsr & DWT_LSR_Present_Msk) != 0) {
            if ((lsr & DWT_LSR_Access_Msk) != 0) {
                DWT->LAR = DWT_LAR_KEY;
            }
        }
    }
};