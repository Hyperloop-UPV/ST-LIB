/*
 * Time.cpp
 *
 *  Created on: 30 oct. 2022
 *      Author: Dani
 */

#include "HALAL/Services/Time/Time.hpp"

#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Models/TimerPeripheral/TimerPeripheral.hpp"

TIM_HandleTypeDef* Time::low_precision_timer = &htim7;

uint8_t Time::high_precision_ids = 0;
uint8_t Time::low_precision_ids = 0;
uint8_t Time::mid_precision_ids = 0;

bool Time::mid_precision_registered = false;

stack<TIM_HandleTypeDef*> Time::available_high_precision_timers;

unordered_map<uint8_t, Time::Alarm> Time::high_precision_alarms_by_id;
unordered_map<uint8_t, Time::Alarm> Time::low_precision_alarms_by_id;
unordered_map<uint8_t, Time::Alarm> Time::mid_precision_alarms_by_id;
unordered_map<TIM_HandleTypeDef*, Time::Alarm> Time::high_precision_alarms_by_timer;

unordered_map<TIM_HandleTypeDef*, TIM_TypeDef*> Time::timer32_by_timer32handler_map =
    {{&htim2, TIM2}, {&htim5, TIM5}, {&htim23, TIM23}, {&htim24, TIM24}};
unordered_map<TIM_HandleTypeDef*, IRQn_Type> Time::timer32interrupt_by_timer32handler_map =
    {{&htim2, TIM2_IRQn}, {&htim5, TIM5_IRQn}, {&htim23, TIM23_IRQn}, {&htim24, TIM24_IRQn}};

stack<uint8_t> Time::low_precision_erasable_ids;
stack<uint8_t> Time::mid_precision_erasable_ids;

uint64_t Time::global_tick = 0;
uint64_t Time::low_precision_tick = 0;
uint64_t Time::mid_precision_tick = 0;

void Time::init_timer(
    TIM_TypeDef* tim,
    TIM_HandleTypeDef* htim,
    uint32_t prescaler,
    uint32_t period,
    IRQn_Type interrupt_channel
) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim->Instance = tim;
    htim->Init.Prescaler = prescaler;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = period;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(htim);

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(htim, &sClockSourceConfig);

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

    HAL_TIMEx_MasterConfigSynchronization(htim, &sMasterConfig);

    HAL_NVIC_SetPriority(interrupt_channel, 0, 0);
    HAL_NVIC_EnableIRQ(interrupt_channel);
}

void Time::start() {
    __HAL_RCC_TIM7_CLK_ENABLE();
    __HAL_RCC_TIM24_CLK_ENABLE();
    __HAL_RCC_TIM23_CLK_ENABLE();
    Time::init_timer(TIM2, &htim2, 0, HIGH_PRECISION_MAX_ARR, TIM2_IRQn);
    Time::init_timer(TIM5, &htim5, 0, HIGH_PRECISION_MAX_ARR, TIM5_IRQn);
    Time::init_timer(TIM24, &htim24, 0, HIGH_PRECISION_MAX_ARR, TIM24_IRQn);
    Time::init_timer(TIM7, &htim7, 275, 1000, TIM7_IRQn);
    HAL_TIM_Base_Start_IT(low_precision_timer);

    if (global_timer != nullptr) {
        if (timer32_by_timer32handler_map.find(global_timer) ==
            timer32_by_timer32handler_map.end()) {
            ErrorHandler("Global Timer pointer is not valid");
            global_timer = nullptr;
        } else {
            hal_enable_timer(global_timer);
            Time::init_timer(
                timer32_by_timer32handler_map[global_timer],
                global_timer,
                0,
                HIGH_PRECISION_MAX_ARR,
                timer32interrupt_by_timer32handler_map[global_timer]
            );
            HAL_TIM_Base_Start_IT(global_timer);
        }
    }
    for (auto timer : high_precision_timers) {
        if (timer32_by_timer32handler_map.find(timer) == timer32_by_timer32handler_map.end()) {
            ErrorHandler("Global Timer pointer is not valid");
            high_precision_timers.erase(timer);
        } else {
            hal_enable_timer(timer);
            Time::init_timer(
                timer32_by_timer32handler_map[timer],
                timer,
                0,
                HIGH_PRECISION_MAX_ARR,
                timer32interrupt_by_timer32handler_map[timer]
            );
            Time::available_high_precision_timers.push(timer);
        }
    }
}

bool Time::is_valid_timer(TIM_HandleTypeDef* tim) {
    if (tim == global_timer || tim == low_precision_timer || tim == mid_precision_timer ||
        high_precision_timers.contains(tim))
        return true;
    return false;
}

void Time::hal_enable_timer(TIM_HandleTypeDef* tim) {
    if (tim == &htim2) {
        __HAL_RCC_TIM2_CLK_ENABLE();
    } else if (tim == &htim5) {
        __HAL_RCC_TIM5_CLK_ENABLE();
    } else if (tim == &htim23) {
        __HAL_RCC_TIM23_CLK_ENABLE();
    } else if (tim == &htim24) {
        __HAL_RCC_TIM24_CLK_ENABLE();
    }
}

// PUBLIC SERVICE METHODS

uint64_t Time::get_global_tick() {
    if (global_timer == nullptr) {
        ErrorHandler("tried to use global tick without global timer configured");
        return 0;
    }
    uint64_t current_tick = Time::global_tick + global_timer->Instance->CNT;
    uint32_t apb1_tim_freq = HAL_RCC_GetPCLK1Freq() * 2;
    double to_nanoseconds = 1.0 / apb1_tim_freq * 1000000000.0;
    return current_tick * to_nanoseconds;
}

void Time::start_timer(TIM_HandleTypeDef* handle, uint32_t prescaler, uint32_t arr) {
    handle->Instance->ARR = arr;
    handle->Instance->PSC = prescaler;
    HAL_TIM_Base_Start_IT(handle);
}

void Time::stop_timer(TIM_HandleTypeDef* handle) { HAL_TIM_Base_Stop_IT(handle); }

uint8_t Time::register_high_precision_alarm(uint32_t period_in_us, function<void()> func) {
    if (available_high_precision_timers.size() == 0) {
        ErrorHandler("There are no available high precision timers left");
        return 0;
    }

    TIM_HandleTypeDef* tim = Time::available_high_precision_timers.top();
    Time::available_high_precision_timers.pop();

    Time::Alarm alarm = {.period = period_in_us, .tim = tim, .alarm = [&]() {}, .is_on = true};

    for (auto timer : high_precision_timers) {
        NVIC_DisableIRQ(timer32interrupt_by_timer32handler_map[timer]);
    }
    Time::high_precision_alarms_by_id[high_precision_ids] = alarm;
    Time::high_precision_alarms_by_timer[tim] = alarm;

    Time::ConfigTimer(tim, period_in_us);

    alarm.alarm = func;
    Time::high_precision_alarms_by_id[high_precision_ids] = alarm;
    Time::high_precision_alarms_by_timer[tim] = alarm;
    for (auto timer : high_precision_timers) {
        NVIC_EnableIRQ(timer32interrupt_by_timer32handler_map[timer]);
    }

    return high_precision_ids++;
}

bool Time::unregister_high_precision_alarm(uint8_t id) {
    if (not Time::high_precision_alarms_by_id.contains(id)) {
        return false;
    }

    for (auto timer : high_precision_timers) {
        NVIC_DisableIRQ(timer32interrupt_by_timer32handler_map[timer]);
    }
    Time::Alarm& alarm = high_precision_alarms_by_id[id];
    Time::stop_timer(alarm.tim);
    Time::available_high_precision_timers.push(alarm.tim);

    Time::high_precision_alarms_by_timer.erase(alarm.tim);
    Time::high_precision_alarms_by_id.erase(id);
    for (auto timer : high_precision_timers) {
        NVIC_EnableIRQ(timer32interrupt_by_timer32handler_map[timer]);
    }

    return true;
}

uint8_t Time::register_mid_precision_alarm(uint32_t period_in_us, function<void()> func) {
    if (mid_precision_timer == nullptr) {
        ErrorHandler("Cannot register mid precision alarm if no timer is used for mid "
                     "precision");
        return 0;
    }
    if (mid_precision_alarms_by_id.size() == std::numeric_limits<uint8_t>::max()) {
        ErrorHandler("Cannot register mid precision alarm as all id's are already "
                     "occupied");
        return 0;
    }
    if (std::find_if(
            TimerPeripheral::timers.begin(),
            TimerPeripheral::timers.end(),
            [&](TimerPeripheral& tim) -> bool {
                return tim.handle == mid_precision_timer && (tim.is_occupied());
            }
        ) != TimerPeripheral::timers.end()) {
        ErrorHandler("a timer cannot be used as mid precision timer and PWM or Input "
                     "Capture Simultaneously");
        return 0; // TODO: put this check in high precision timers and global
                  // timer too
    }

    Time::Alarm alarm = {
        .period = period_in_us / Time::mid_precision_step_in_us,
        .tim = Time::mid_precision_timer,
        .alarm = [&]() {},
        .offset = Time::mid_precision_tick,
        .is_on = true
    };

    while (mid_precision_alarms_by_id.contains(mid_precision_ids))
        mid_precision_ids++;

    NVIC_DisableIRQ(timer32interrupt_by_timer32handler_map[mid_precision_timer]);
    Time::mid_precision_alarms_by_id[mid_precision_ids] = alarm;

    if (not Time::mid_precision_registered) {
        hal_enable_timer(mid_precision_timer);
        Time::init_timer(
            timer32_by_timer32handler_map[mid_precision_timer],
            Time::mid_precision_timer,
            275,
            Time::mid_precision_step_in_us,
            timer32interrupt_by_timer32handler_map[mid_precision_timer]
        );
        Time::ConfigTimer(Time::mid_precision_timer, Time::mid_precision_step_in_us);
        NVIC_DisableIRQ(timer32interrupt_by_timer32handler_map[mid_precision_timer]);
        Time::mid_precision_registered = true;
    }

    alarm.alarm = func;
    Time::mid_precision_alarms_by_id[mid_precision_ids] = alarm;
    NVIC_EnableIRQ(timer32interrupt_by_timer32handler_map[mid_precision_timer]);

    return mid_precision_ids;
}

bool Time::unregister_mid_precision_alarm(uint8_t id) {
    if (not Time::mid_precision_alarms_by_id.contains(id)) {
        return false;
    }

    NVIC_DisableIRQ(timer32interrupt_by_timer32handler_map[mid_precision_timer]);
    Time::Alarm& alarm = Time::mid_precision_alarms_by_id[id];
    alarm.is_on = false;
    Time::mid_precision_erasable_ids.push(id);
    NVIC_EnableIRQ(timer32interrupt_by_timer32handler_map[mid_precision_timer]);

    return true;
}

uint8_t Time::register_low_precision_alarm(uint32_t period_in_ms, function<void()> func) {
    if (low_precision_alarms_by_id.size() == std::numeric_limits<uint8_t>::max()) {
        ErrorHandler("Cannot register low precision alarm as all id's are already "
                     "occupied");
        return 0;
    }
    Time::Alarm alarm = {
        .period = period_in_ms,
        .tim = low_precision_timer,
        .alarm = func,
        .offset = low_precision_tick,
        .is_on = true
    };

    while (low_precision_alarms_by_id.contains(low_precision_ids))
        low_precision_ids++;

    NVIC_DisableIRQ(TIM7_IRQn);
    Time::low_precision_alarms_by_id[low_precision_ids] = alarm;
    NVIC_EnableIRQ(TIM7_IRQn);
    return low_precision_ids++;
}

uint8_t Time::register_low_precision_alarm(uint32_t period_in_ms, void (*func)()) {
    if (low_precision_alarms_by_id.size() == std::numeric_limits<uint8_t>::max()) {
        ErrorHandler("Cannot register low precision alarm as all id's are already "
                     "occupied");
        return 0;
    }
    Time::Alarm alarm = {
        .period = period_in_ms,
        .tim = low_precision_timer,
        .alarm = func,
        .offset = low_precision_tick,
        .is_on = true
    };

    while (low_precision_alarms_by_id.contains(low_precision_ids))
        low_precision_ids++;

    NVIC_DisableIRQ(TIM7_IRQn);
    Time::low_precision_alarms_by_id[low_precision_ids] = alarm;
    NVIC_EnableIRQ(TIM7_IRQn);
    return low_precision_ids++;
}

bool Time::unregister_low_precision_alarm(uint8_t id) {
    if (not Time::low_precision_alarms_by_id.contains(id)) {
        return false;
    }

    NVIC_DisableIRQ(TIM7_IRQn);
    Time::Alarm& alarm = Time::low_precision_alarms_by_id[id];
    alarm.is_on = false;
    Time::low_precision_erasable_ids.push(id);
    NVIC_EnableIRQ(TIM7_IRQn);

    return true;
}

uint8_t Time::set_timeout(int milliseconds, function<void()> callback) {
    if (low_precision_alarms_by_id.size() == std::numeric_limits<uint8_t>::max()) {
        ErrorHandler("Cannot register timeout as all low precision alarms id's are "
                     "already occupied");
        return 255;
    }
    while (low_precision_alarms_by_id.contains(low_precision_ids))
        low_precision_ids++;
    uint8_t id = low_precision_ids;
    uint64_t tick_on_register = low_precision_tick;
    Time::register_low_precision_alarm(milliseconds, [&, id, callback, tick_on_register]() {
        if (tick_on_register == low_precision_tick) {
            return;
        }
        callback();
        Time::unregister_low_precision_alarm(id);
    });
    return id;
}

void Time::cancel_timeout(uint8_t id) {
    // we can take advantage of the fact that if we try to
    // unregister an alarm that is not registered, it will return false
    // instead of going HardFault or ErrorHandler
    Time::unregister_low_precision_alarm(id);
}

void Time::global_timer_callback() { Time::global_tick += Time::HIGH_PRECISION_MAX_ARR; }

void Time::high_precision_timer_callback(TIM_HandleTypeDef* tim) {
    Time::high_precision_alarms_by_timer[tim].alarm();
}

void Time::mid_precision_timer_callback() {
    for (pair<const uint8_t, Time::Alarm>& pair : Time::mid_precision_alarms_by_id) {
        Time::Alarm& alarm = pair.second;
        if (alarm.is_on == false || alarm.offset == Time::mid_precision_tick)
            continue;
        if ((Time::mid_precision_tick - alarm.offset) % alarm.period == 0) {
            alarm.alarm();
        }
    }
    Time::mid_precision_tick++;
    while (not Time::mid_precision_erasable_ids.empty()) {
        Time::mid_precision_alarms_by_id.erase(Time::mid_precision_erasable_ids.top());
        Time::mid_precision_erasable_ids.pop();
    }
}

void Time::low_precision_timer_callback() {
    for (auto& pair : Time::low_precision_alarms_by_id) {
        Time::Alarm& alarm = pair.second;
        if (alarm.is_on == false || alarm.offset == Time::low_precision_tick)
            continue;
        if ((Time::low_precision_tick - alarm.offset) % alarm.period == 0) {
            alarm.alarm();
        }
    }
    low_precision_tick++;
    while (not Time::low_precision_erasable_ids.empty()) {
        Time::low_precision_alarms_by_id.erase(Time::low_precision_erasable_ids.top());
        Time::low_precision_erasable_ids.pop();
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* tim) {
    if (tim == Time::global_timer) {
        Time::global_timer_callback();
    } else if (tim == Time::mid_precision_timer) {
        Time::mid_precision_timer_callback();
    } else if (tim == Time::low_precision_timer) {
        Time::low_precision_timer_callback();
    }

    else if (Time::high_precision_timers.contains(tim)) {
        Time::high_precision_timer_callback(tim);
    }
}

void Time::ConfigTimer(TIM_HandleTypeDef* tim, uint32_t period_in_us) {
    __HAL_TIM_DISABLE_IT(tim, TIM_IT_UPDATE);
    tim->Instance->CR1 = ~(TIM_CR1_DIR | TIM_CR1_CMS) & tim->Instance->CR1;
    tim->Instance->CR1 = TIM_COUNTERMODE_UP | tim->Instance->CR1;
    tim->Instance->CR1 = ~TIM_CR1_CKD & tim->Instance->CR1;
    tim->Instance->CR1 = TIM_CLOCKDIVISION_DIV1 | tim->Instance->CR1;
    tim->Instance->ARR = period_in_us;
    tim->Instance->PSC = 275;
    tim->Instance->EGR = TIM_EGR_UG;
    tim->Instance->SMCR = 0;
    tim->Instance->BDTR = 0;
    tim->Instance->CR1 = TIM_CR1_CEN;
    tim->Instance->CNT = 1;
    tim->Instance->DIER = TIM_IT_UPDATE;
}
