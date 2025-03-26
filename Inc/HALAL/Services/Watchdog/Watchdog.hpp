#pragma once
#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Models/PinModel/Pin.hpp"

extern IWDG_HandleTypeDef watchdog_handle;
extern bool reset_by_iwdg;

/**
 * @brief The watchdog class resets the board when it gets stuck inside a loop
 *
 * To ensure that the code its not stuck, the refresh function must be called at
 * least once each period Otherwise, the Watchdog will reset the board. The
 * Watchdog its not active until it is started.
 */
class Watchdog {
   public:
    static std::chrono::microseconds watchdog_time;
    
    static void start() {
        if ((chrono::duration_cast<chrono::microseconds>(watchdog_time)).count() > 32000000) {
            ErrorHandler("Watchdog refresh interval is too big");
        }
        if ((chrono::duration_cast<chrono::microseconds>(watchdog_time)).count() < 125) {
            ErrorHandler("Watchdog refresh interval is too short");
        }
        uint64_t milliseconds = chrono::duration_cast<chrono::milliseconds>(watchdog_time).count();
        uint32_t RL = double(milliseconds) * 8.0 - 1; // this is the formule for the Reload
        uint32_t prescaler = 0;
        while (RL > 4095) {
            milliseconds /= 2;
            prescaler += 1;
            RL = double(milliseconds) * 8.0 - 1;
        }

        watchdog_handle.Instance = IWDG1;
        watchdog_handle.Init.Prescaler = prescaler;
        watchdog_handle.Init.Reload = RL;
        watchdog_handle.Init.Window = RL;
        HAL_IWDG_Init(&watchdog_handle);
    }

    static void refresh() {
        HAL_IWDG_Refresh(&watchdog_handle);
    }
    static void check_reset_flag(){
        if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDG1RST)) {
            reset_by_iwdg = true;
            
        }
        __HAL_RCC_CLEAR_RESET_FLAGS();
    }
    template <typename TimeUnit>
    Watchdog(chrono::duration<int64_t, TimeUnit> period) {
        watchdog_time = std::chrono::duration_cast<std::chrono::microseconds>(period);
    }
};
