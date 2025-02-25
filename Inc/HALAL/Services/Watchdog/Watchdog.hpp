#pragma once
#include "HALAL/Models/PinModel/Pin.hpp"
#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"


extern IWDG_HandleTypeDef watchdog_handle;


/**
 * @brief The watchdog class resets the board when it gets stuck inside a loop
 *
 * To ensure that the code its not stuck, the refresh function must be called at least once each period
 * Otherwise, the Watchdog will reset the board. The Watchdog its not active until it is started.
 */
class Watchdog{
public:

    template<typename TimeUnit>
    static void start(chrono::duration<int64_t, TimeUnit> period){
    	if((chrono::duration_cast<chrono::microseconds>(period)).count() > 32000000){
    		ErrorHandler("Watchdog refresh interval is too big");
    	}
    	if((chrono::duration_cast<chrono::microseconds>(period)).count() < 125){
    		ErrorHandler("Watchdog refresh interval is too short");
    	}
        uint64_t microseconds = chrono::duration_cast<chrono::microseconds>(period).count();
        uint32_t RL = (uint64_t)((double) microseconds / 4 / 31.25) - 1;
        uint32_t prescaler = 0;

        while(RL > 4095){
        	microseconds /= 2;
        	prescaler += 1;
        }

        watchdog_handle.Instance = IWDG1;
        watchdog_handle.Init.Prescaler = prescaler;
        watchdog_handle.Init.Reload = RL;
        watchdog_handle.Init.Window = RL;
        HAL_IWDG_Init(&watchdog_handle);
    }

    static inline void refresh(){
    	__HAL_IWDG_RELOAD_COUNTER(&watchdog_handle);
    }

};
