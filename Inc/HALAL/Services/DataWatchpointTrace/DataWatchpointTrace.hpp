
#pragma once
#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Models/PinModel/Pin.hpp"

volatile unsigned int *DWT_CYCCNT;
volatile unsigned int *DWT_CONTROL;
volatile unsigned int *SCB_DEMCR;
// habra que poner para hacerlo con traza y sin traza en un template
class DataWatchpointTrace {
   public:
    static void reset_cnt() {
        DWT_CYCCNT = (unsigned int *)0xE0001004;
        DWT_CONTROL = (unsigned int *)0xE0001000;
        SCB_DEMCR = (unsigned int *)0xE000EDFC;
        *SCB_DEMCR |= 0x01000000;  // enable DWT
        *DWT_CONTROL |= 1;         // enable the counter
        *DWT_CYCCNT = 0;           // reset counter
    }
    static unsigned int start_count() {
        *DWT_CONTROL |= 1;   // enables the counter
        return *DWT_CYCCNT;  // returns the current value of the counter
    }
    static unsigned int stop_count() {
        *DWT_CONTROL &= ~1;  // disables the counter
        return *DWT_CYCCNT;  // returns the current value of the counter
    }
    static unsigned int get_count() {
        return *DWT_CYCCNT;  // returns the current value of the counter
    }
};