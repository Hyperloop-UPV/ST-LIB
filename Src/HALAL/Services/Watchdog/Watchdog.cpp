#include "HALAL/Services/Watchdog/Watchdog.hpp"

IWDG_HandleTypeDef watchdog_handle;
std::chrono::microseconds Watchdog::watchdog_time = std::chrono::microseconds(1000000); //1 seconds by default
