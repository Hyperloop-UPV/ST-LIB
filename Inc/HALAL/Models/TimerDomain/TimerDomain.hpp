/*
 * TimerDomain.hpp
 *
 *  Created on: 3 dic. 2025
 *      Author: victor
 */

#pragma once

#ifndef TESTING_ENV
#include "stm32h7xx_hal.h"
#else
#include "MockedDrivers/stm32h7xx_hal_wrapper.h"
#endif

#ifdef HAL_TIM_MODULE_ENABLED

#include "HALAL/Models/GPIO.hpp"
#include "HALAL/Models/Pin.hpp"

#include <span>
#include <array>

#include "ErrorHandler/ErrorHandler.hpp"

// NOTE: only works for static arrays
#define ARRAY_LENGTH(a) (sizeof(a)/sizeof(*a))

#define TimerXList  \
    X(2, APB1LENR)  \
    X(3, APB1LENR)  \
    X(4, APB1LENR)  \
    X(5, APB1LENR)  \
    X(6, APB1LENR)  \
    X(7, APB1LENR)  \
    X(12, APB1LENR) \
    X(13, APB1LENR) \
    X(14, APB1LENR) \
                    \
    X(23, APB1HENR) \
    X(24, APB1HENR) \
                    \
    X(1, APB2ENR)   \
    X(8, APB2ENR)   \
    X(15, APB2ENR)  \
    X(16, APB2ENR)  \
    X(17, APB2ENR)

#define X(n, b) extern TIM_HandleTypeDef htim##n;
TimerXList
#undef X

#if !defined(glue)
#define glue_(a,b) a##b
#define glue(a,b) glue_(a,b)
#endif // !defined(glue)

/* Tim1 & Tim8 are advanced-control timers
 *  their ARR & prescaler are 16bit
 *  they have up to 6 independent channels for:
 *   - input capture (not channel 5 or 6)
 *   - output capture
 *   - PWM generation
 *   - One-pulse mode output
 */

/* Timers {TIM2, TIM5, TIM23, TIM24} are the only 32-bit counter resolution timers, the rest are 16-bit */
/* Timers 2, 3, 4, 5, 23, 24 are general-purpose timers
 * Timers 12, 13, 14 are also general-purpose timers (but separate in the ref manual)
 * Timers 15, 16, 17 are also general purpose timers (but separate in the ref manual)
 */

/* Tim6 & Tim7 are basic timers. Features:
 - 16-bit ARR upcounter
 - 16-bit PSC
 - Synchronization circuit to trigger the DAC
 - Interrupt/DMA generation on the update event
*/

/* advanced timer features:
 - 16-bit ARR up/down counter
 - 16-bit PSC
 - Up to 6 independent channels for:
   · Input capture (all channels but 5 and 6)
   · Output compare
   · PWM generation (Edge and Center aligned mode)
   · One-pulse mode output
 - Complementary outputs with programmable dead-time
 - Synchronization circuit to control the timer with 
  external signals and to interconnect several timers together.
 - Repetition counter to update the timer registers only after 
  a given number of cycles of the counter.
 - 2 break inputs to put the timer’s output signals in a safe user selectable configuration.
 - Interrupt/DMA generation on the following events:
   · Update: counter overflow/underflow, counter initialization (by software or internal/external trigger)
   · Trigger event (counter start, stop, initialization or count by internal/external trigger)
   · Input capture
   · Output compare
 - Supports incremental (quadrature) encoder and Hall-sensor circuitry for positioning purposes
 - Trigger input for external clock or cycle-by-cycle current management
*/
namespace ST_LIB {
extern void compile_error(const char *msg);

/* The number corresponds with the timer nº */
enum TimerRequest : uint8_t {
    AnyGeneralPurpose = 0,
    Any32bit = 0xFF,

    Advanced_1 = 1,
    Advanced_8 = 8,

    GeneralPurpose32bit_2 = 2,
    GeneralPurpose32bit_5 = 5,
    GeneralPurpose32bit_23 = 23,
    GeneralPurpose32bit_24 = 24,

    GeneralPurpose_3 = 3,
    GeneralPurpose_4 = 4,

    SlaveTimer_12 = 12,
    SlaveTimer_13 = 13,
    SlaveTimer_14 = 14,

    GeneralPurpose_15 = 15,
    GeneralPurpose_16 = 16,
    GeneralPurpose_17 = 17,

    Basic_6 = 6,
    Basic_7 = 7,
};

// Alternate functions for timers
enum class TimerAF {
    None,
    PWM,
    Encoder,
    InputCapture,
    BreakInput,
    BreakInputCompare,
};

enum class TimerChannel : uint8_t {
    CHANNEL_1 = 1,
    CHANNEL_2 = 2,
    CHANNEL_3 = 3,
    CHANNEL_4 = 4,
    CHANNEL_5 = 5,
    CHANNEL_6 = 6,

    CHANNEL_NEGATED_FLAG = 8,

    CHANNEL_1_NEGATED = 1 | CHANNEL_NEGATED_FLAG,
    CHANNEL_2_NEGATED = 2 | CHANNEL_NEGATED_FLAG,
    CHANNEL_3_NEGATED = 3 | CHANNEL_NEGATED_FLAG,
};

struct TimerPin {
    ST_LIB::TimerAF af;
    ST_LIB::GPIODomain::Pin pin;
    ST_LIB::TimerChannel channel;
};

#ifndef DEFAULT_PWM_FREQUENCY_MODE
#define DEFAULT_PWM_FREQUENCY_MODE ST_LIB::PWM_Frequency_Mode::PRECISION
#endif

enum class PWM_Frequency_Mode {
    PRECISION,
    SPEED,
};

constexpr std::array<uint8_t, 25> create_timer_idxmap() {
    std::array<uint8_t, 25> result{};
    
    // invalid timers that don't exist
    result[0] = -1;
    result[9] = -1; result[10] = -1; result[11] = -1;
    result[18] = -1; result[19] = -1; result[20] = -1; result[21] = -1; result[22] = -1;
    
    // general-purpose timers
    result[2] = 0; result[3] = 1; result[4] = 2;
    result[5] = 3; result[23] = 4; result[24] = 5;

    // more general-purpose timers
    result[12] = 6; result[13] = 7; result[14] = 8;

    // more general-purpose timers
    result[15] = 9; result[16] = 10; result[17] = 11;

    // basic timers
    result[6] = 12; result[7] = 13;

    // advanced control timers
    result[1] = 14; result[8] = 15;
    
    return result;
}

static constexpr std::array<uint8_t, 25> timer_idxmap = create_timer_idxmap();

struct TimerDomain {
    // There are 16 timers
    static constexpr std::size_t max_instances = 16;

    struct Entry {
        std::array<char, 8> name; /* max length = 7 */
        TimerRequest request;
        uint8_t pin_count;
        std::array<TimerPin, 7> pins; /* this won't be read in Timer constructor */
    };

    struct Config {
        uint8_t timer_idx;
    };

    static constexpr TIM_HandleTypeDef *hal_handles[16] = {
        // general purpose timers
        &htim2, &htim3, &htim4, &htim5, &htim23, &htim24,
        &htim12, &htim13, &htim14,
        &htim15, &htim16, &htim17,

        // basic timers
        &htim6, &htim7,

        // advanced control timers
        &htim1, &htim8
    };

#ifdef TESTING_ENV
    static TIM_TypeDef *cmsis_timers[16];
#else
    static constexpr TIM_TypeDef *cmsis_timers[16] = {
        // general purpose timers
        TIM2, TIM3, TIM4, TIM5, TIM23, TIM24,
        TIM12, TIM13, TIM14,
        TIM15, TIM16, TIM17,

        // basic timers
        TIM6, TIM7,

        // advanced control timers
        TIM1, TIM8
    };
#endif

    static constexpr IRQn_Type timer_irqn[] = {
        // general purpose timers 1
        TIM2_IRQn, TIM3_IRQn, TIM4_IRQn, TIM5_IRQn, TIM23_IRQn, TIM24_IRQn,
        // slave timers
        TIM8_BRK_TIM12_IRQn, TIM8_UP_TIM13_IRQn, TIM8_TRG_COM_TIM14_IRQn,
        // general purpose timers 2
        TIM15_IRQn, TIM16_IRQn, TIM17_IRQn,

        // basic timers
        TIM6_DAC_IRQn, /* TIM6 global and DAC1&2 underrun error interrupts */
        TIM7_IRQn,

        TIM1_UP_IRQn, TIM8_UP_TIM13_IRQn
    };

    static inline void rcc_enable_timer(TIM_TypeDef *tim) {
#define X(n, b) \
    else if(tim == TIM##n) { SET_BIT(RCC->b, RCC_##b##_TIM##n##EN); }

        if(false) {}
        TimerXList
        else {
            ErrorHandler("Invalid timer given to rcc_enable_timer");
        }
#undef X
    }

    static constexpr std::array<char, 8> EMPTY_TIMER_NAME = {0,0,0,0, 0,0,0,0};

    struct Timer {
        using domain = TimerDomain;
        //GPIODomain::GPIO gpios[4];
        Entry e;
        GPIODomain::GPIO gpio0;
        GPIODomain::GPIO gpio1;
        GPIODomain::GPIO gpio2;
        GPIODomain::GPIO gpio3;
        GPIODomain::GPIO gpio4;
        GPIODomain::GPIO gpio5;
        GPIODomain::GPIO gpio6;

        static consteval GPIODomain::AlternateFunction get_gpio_af(ST_LIB::TimerRequest req, ST_LIB::TimerPin pin);

        static consteval GPIODomain::OperationMode get_operation_mode(ST_LIB::TimerAF af)
        {
            switch(af) {
                case TimerAF::None: return GPIODomain::OperationMode::INPUT;

                case TimerAF::PWM: return GPIODomain::OperationMode::ALT_PP;
                case TimerAF::Encoder: return GPIODomain::OperationMode::ALT_PP;

                // TODO: check what this really needs to be for each
                case TimerAF::InputCapture: return GPIODomain::OperationMode::OUTPUT_OPENDRAIN;

                case TimerAF::BreakInput: return GPIODomain::OperationMode::OUTPUT_OPENDRAIN;
                case TimerAF::BreakInputCompare: return GPIODomain::OperationMode::OUTPUT_OPENDRAIN;
            }
        }

        static consteval GPIODomain::Pull get_pull(ST_LIB::TimerAF af)
        {
            switch(af) {
                case TimerAF::None: return GPIODomain::Pull::None;

                case TimerAF::PWM: return GPIODomain::Pull::None;
                case TimerAF::Encoder: return GPIODomain::Pull::Up; /* might be wrong */

                // TODO: check what this really needs to be for each
                case TimerAF::InputCapture: return GPIODomain::Pull::Up;
                
                case TimerAF::BreakInput: return GPIODomain::Pull::None;
                case TimerAF::BreakInputCompare: return GPIODomain::Pull::None;
            }
        }

        static consteval GPIODomain::Speed get_speed(ST_LIB::TimerAF af)
        {
            switch(af) {
                case TimerAF::None: return GPIODomain::Speed::Low;

                case TimerAF::PWM: return GPIODomain::Speed::Low;
                case TimerAF::Encoder: return GPIODomain::Speed::Low;

                // TODO: check what this really needs to be for each
                case TimerAF::InputCapture: return GPIODomain::Speed::Low;
                
                case TimerAF::BreakInput: return GPIODomain::Speed::Low;
                case TimerAF::BreakInputCompare: return GPIODomain::Speed::Low;
            }
        }

        static constexpr ST_LIB::TimerPin empty_pin = {
            .af = TimerAF::None,
            .pin = ST_LIB::PA0,
            .channel = TimerChannel::CHANNEL_1,
        };

#define GetPinFromIdx(pinargs, idx) \
    sizeof...(pinargs) > idx ? (ST_LIB::TimerPin[]){pinargs...}[idx] : empty_pin
#define GetGPIOFromIdx(pinargs, request, idx) \
    sizeof...(pinargs) > idx ? (ST_LIB::TimerPin[]){pinargs...}[idx].pin : ST_LIB::PA0,                      \
    get_operation_mode(sizeof...(pinargs) > idx ? (ST_LIB::TimerPin[]){pinargs...}[idx].af : TimerAF::None), \
    get_pull(sizeof...(pinargs) > idx ? (ST_LIB::TimerPin[]){pinargs...}[idx].af : TimerAF::None),           \
    get_speed(sizeof...(pinargs) > idx ? (ST_LIB::TimerPin[]){pinargs...}[idx].af : TimerAF::None),          \
    (sizeof...(pinargs) > idx ? get_gpio_af(request, (ST_LIB::TimerPin[]){pinargs...}[idx]) : GPIODomain::AlternateFunction::NO_AF)

        template <typename... T>
        consteval Timer(TimerRequest request = TimerRequest::AnyGeneralPurpose, 
            std::array<char, 8> name = EMPTY_TIMER_NAME, T... pinargs) :
            e(name, request, sizeof...(pinargs), 
              std::array<TimerPin, 7>(
                {GetPinFromIdx(pinargs, 0), GetPinFromIdx(pinargs, 1), GetPinFromIdx(pinargs, 2), 
                 GetPinFromIdx(pinargs, 3), GetPinFromIdx(pinargs, 4), GetPinFromIdx(pinargs, 5),
                 GetPinFromIdx(pinargs, 6)})),
            gpio0(GetGPIOFromIdx(pinargs, request, 0)),
            gpio1(GetGPIOFromIdx(pinargs, request, 1)),
            gpio2(GetGPIOFromIdx(pinargs, request, 2)),
            gpio3(GetGPIOFromIdx(pinargs, request, 3)),
            gpio4(GetGPIOFromIdx(pinargs, request, 4)),
            gpio5(GetGPIOFromIdx(pinargs, request, 5)),
            gpio6(GetGPIOFromIdx(pinargs, request, 6))
        {
            static_assert((std::is_same_v<T, TimerPin> && ...), 
                  "All template arguments must be of type TimerPin");
            if(sizeof...(pinargs) > 7) {
                ST_LIB::compile_error("Max 7 pins per timer");
            }
        }

        // anything uninitialized will be 0
        template <typename... T>
        consteval Timer(Entry ent, T... pinargs) :
            e(ent.name, ent.request, sizeof...(pinargs), 
              std::array<TimerPin, 7>(
                {GetPinFromIdx(pinargs, 0), GetPinFromIdx(pinargs, 1), GetPinFromIdx(pinargs, 2), 
                 GetPinFromIdx(pinargs, 3), GetPinFromIdx(pinargs, 4), GetPinFromIdx(pinargs, 5),
                 GetPinFromIdx(pinargs, 6)})),
            gpio0(GetGPIOFromIdx(pinargs, ent.request, 0)),
            gpio1(GetGPIOFromIdx(pinargs, ent.request, 1)),
            gpio2(GetGPIOFromIdx(pinargs, ent.request, 2)),
            gpio3(GetGPIOFromIdx(pinargs, ent.request, 3)),
            gpio4(GetGPIOFromIdx(pinargs, ent.request, 4)),
            gpio5(GetGPIOFromIdx(pinargs, ent.request, 5)),
            gpio6(GetGPIOFromIdx(pinargs, ent.request, 6))
        {
            static_assert((std::is_same_v<T, TimerPin> && ...), 
                  "All template arguments must be of type TimerPin");
            if(sizeof...(pinargs) > 7) {
                ST_LIB::compile_error("Max 7 pins per timer");
            }
        }
        
        template<class Ctx>
        consteval void inscribe(Ctx &ctx) const {
            if(e.pin_count > 0) { gpio0.inscribe(ctx); }
            if(e.pin_count > 1) { gpio1.inscribe(ctx); }
            if(e.pin_count > 2) { gpio2.inscribe(ctx); }
            if(e.pin_count > 3) { gpio3.inscribe(ctx); }
            if(e.pin_count > 4) { gpio4.inscribe(ctx); }
            if(e.pin_count > 5) { gpio5.inscribe(ctx); }
            if(e.pin_count > 6) { gpio6.inscribe(ctx); }

            TimerDomain::Entry local_entry = {
                .name = e.name,
                .request = e.request,
                .pin_count = e.pin_count,
                .pins = e.pins,
            };
            ctx.template add<TimerDomain>(local_entry, this);
        }
    };

    template<std::size_t N>
    static consteval std::array<Config, N> build(std::span<const Entry> requests) {
        std::array<Config, N> cfgs{};
        uint16_t cfg_idx = 0;
        bool used_timers[25] = {0};

        if(requests.size() > max_instances) {
            ST_LIB::compile_error("too many Timer requests, there are only 16 timers");
        }

        int remaining_requests[max_instances] = {};
        int count_remaining_requests = (int)requests.size();
        for(int i = 0; i < (int)requests.size(); i++) remaining_requests[i] = i;

        for(int i = 0; i < (int)requests.size(); i++) {
            if(requests[i].request != TimerRequest::AnyGeneralPurpose &&
               (requests[i].request < 1 || requests[i].request > 24 ||
               (requests[i].request > 17 && requests[i].request < 23)))
            {
                ST_LIB::compile_error("Invalid TimerRequest value for timer");
            }

            uint32_t used_channels = 0;
            for(uint16_t pi = 0; pi < requests[i].pin_count; pi++) {
                uint32_t channel_bit = (1 << static_cast<uint32_t>(requests[i].pins[pi].channel));
                if(used_channels & channel_bit) {
                    ST_LIB::compile_error("Only one pin per channel for each timer");
                }
                used_channels |= channel_bit;
            }
        }

        // First find any that have requested a specific timer
        for(std::size_t i = 0; i < N; i++) {
            uint8_t reqint = static_cast<uint8_t>(requests[i].request);
            if((requests[i].request != TimerRequest::AnyGeneralPurpose) &&
               (requests[i].request != TimerRequest::Any32bit))
            {
                if(used_timers[reqint]) {
                    ST_LIB::compile_error("Error: Timer already used");
                }
                used_timers[reqint] = true;

                Config cfg = {
                    .timer_idx = timer_idxmap[reqint],
                };
                cfgs[cfg_idx++] = cfg;

                // unordered remove (remaining requests is not used here so these are ordered)
                count_remaining_requests--;
                remaining_requests[i] = remaining_requests[count_remaining_requests];
            }
        }

        // 32 bit timers, very important for scheduler
        uint8_t bits32_timers[] = {2, 5, 23, 24};
        uint8_t remaining_32bit_timers[4] = {0};
        uint8_t count_remaining_32bit_timers = 0;
        uint8_t count_32bit_requests = 0;

        for(int i = 0; i < (int)ARRAY_LENGTH(bits32_timers); i++) {
            if(!used_timers[bits32_timers[i]])
                remaining_32bit_timers[count_remaining_32bit_timers++] = bits32_timers[i];
        }

        for(int i = 0; i < count_remaining_requests; ) {
            const Entry &e = requests[remaining_requests[i]];
            if(e.request == TimerRequest::Any32bit) {
                if(count_remaining_32bit_timers <= count_32bit_requests) {
                    ST_LIB::compile_error("No remaining 32 bit timers, there are only 4. Timers {2, 5, 23, 24}");
                }
    
                uint8_t reqint = remaining_32bit_timers[count_32bit_requests];
                Config cfg = {
                    .timer_idx = timer_idxmap[reqint],
                };
                cfgs[cfg_idx++] = cfg;

                // unordered remove
                count_remaining_requests--;
                remaining_requests[i] = remaining_requests[count_remaining_requests];
            } else {
                i++;
            }
        }

        // can use any CountingMode (32 bit timers can also but they are higher priority)
        uint8_t up_down_updown_timers[] = {3, 4};

        // 16 bit timers
        /* NOTE: timers {TIM12, TIM13, TIM14} are also 16 bit but 
         *  they are used as slave timers to tim8
         */
        uint8_t bits16_timers[] = {15, 16, 17};
        uint8_t remaining_timers[15] = {0};
        uint8_t count_remaining_timers = 0;

        for(int i = 0; i < (int)ARRAY_LENGTH(bits16_timers); i++) {
            if(!used_timers[bits16_timers[i]])
                remaining_timers[count_remaining_timers++] = bits16_timers[i];
        }

        for(int i = 0; i < (int)ARRAY_LENGTH(up_down_updown_timers); i++) {
            if(!used_timers[up_down_updown_timers[i]])
                remaining_timers[count_remaining_timers++] = up_down_updown_timers[i];
        }

        for(int i = 0; i < (int)ARRAY_LENGTH(bits32_timers); i++) {
            if(!used_timers[bits32_timers[i]])
                remaining_timers[count_remaining_timers++] = bits32_timers[i];
        }

        if(count_remaining_requests > count_remaining_timers) {
            ST_LIB::compile_error("This should not happen");
        }

        for(int i = 0; i < count_remaining_requests; i++) {
            const Entry &e = requests[remaining_requests[i]];
            if(e.request != TimerRequest::AnyGeneralPurpose) {
                ST_LIB::compile_error("This only processes TimerRequest::AnyGeneralPurpose");
            }
            uint8_t reqint = remaining_timers[i];
            Config cfg = {
                .timer_idx = timer_idxmap[reqint],
            };
            cfgs[cfg_idx++] = cfg;
        }

        return cfgs;
    }

    // Runtime object
    struct Instance {
        TIM_TypeDef *tim;
        TIM_HandleTypeDef *hal_tim;
        uint8_t timer_idx;
    };

    static void (*callbacks[TimerDomain::max_instances])(void*);
    static void *callback_data[TimerDomain::max_instances];

    template <std::size_t N> struct Init {
        static inline std::array<Instance, N> instances{};

        static void init(std::span<const Config, N> cfgs) {
            for(std::size_t i = 0; i < N; i++) {
                const Config &e = cfgs[i];

                TIM_HandleTypeDef *handle = hal_handles[e.timer_idx];
                TIM_TypeDef *tim = cmsis_timers[e.timer_idx];
                handle->Instance = tim;
                handle->Init.Period = 0;
                handle->Init.Prescaler = 0;
                handle->Init.CounterMode = TIM_COUNTERMODE_UP;
                handle->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
                handle->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
                handle->Init.RepetitionCounter = 0;

                handle->ChannelState[0] = HAL_TIM_CHANNEL_STATE_READY;
                handle->ChannelState[1] = HAL_TIM_CHANNEL_STATE_READY;
                handle->ChannelState[2] = HAL_TIM_CHANNEL_STATE_READY;
                handle->ChannelState[3] = HAL_TIM_CHANNEL_STATE_READY;
                handle->ChannelState[4] = HAL_TIM_CHANNEL_STATE_READY;
                handle->ChannelState[5] = HAL_TIM_CHANNEL_STATE_READY;
                
                handle->ChannelNState[0] = HAL_TIM_CHANNEL_STATE_READY;
                handle->ChannelNState[1] = HAL_TIM_CHANNEL_STATE_READY;
                handle->ChannelNState[2] = HAL_TIM_CHANNEL_STATE_READY;
                handle->ChannelNState[3] = HAL_TIM_CHANNEL_STATE_READY;
                handle->DMABurstState = HAL_DMA_BURST_STATE_READY;
                handle->Lock = HAL_UNLOCKED;
                handle->State = HAL_TIM_STATE_READY;

                rcc_enable_timer(tim);

                Instance *inst = &instances[i];
                inst->tim = tim;
                inst->hal_tim = handle;
                inst->timer_idx = e.timer_idx;
            }
        }
    };
};

consteval GPIODomain::AlternateFunction 
TimerDomain::Timer::get_gpio_af(ST_LIB::TimerRequest req, ST_LIB::TimerPin pin)
{
    enum TimerAF_Use {
        Channel_1 = 1,
        Channel_2 = 2,
        Channel_3 = 3,
        Channel_4 = 4,

        NegatedChannelFlag = 8,

        Channel_1_Negated = 1 | NegatedChannelFlag,
        Channel_2_Negated = 2 | NegatedChannelFlag,
        Channel_3_Negated = 3 | NegatedChannelFlag,
        Channel_4_Negated = 4 | NegatedChannelFlag,

        ExternalTriggerFilter, /* ETR */
        BreakInput_1,
        BreakInput_2,
        BreakInputCompare_1,
        BreakInputCompare_2,
    };

    struct TimerPossiblePin {
        ST_LIB::GPIODomain::Pin pin;
        ST_LIB::GPIODomain::AlternateFunction af;
        TimerAF_Use use;
    };

    // 4 capture-compare channels
    // complementary output
#define Tim1PinsMacro \
    {ST_LIB::PE6, ST_LIB::GPIODomain::AlternateFunction::AF1, BreakInput_2},          \
    {ST_LIB::PE6, ST_LIB::GPIODomain::AlternateFunction::AF12, BreakInputCompare_2},  \
                                                                                      \
    {ST_LIB::PA6, ST_LIB::GPIODomain::AlternateFunction::AF1, BreakInput_1},          \
    {ST_LIB::PA6, ST_LIB::GPIODomain::AlternateFunction::AF11, BreakInputCompare_1},  \
                                                                                      \
    {ST_LIB::PA7, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_1_Negated},     \
    {ST_LIB::PB0, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_2_Negated},     \
    {ST_LIB::PB1, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_3_Negated},     \
    {ST_LIB::PE7, ST_LIB::GPIODomain::AlternateFunction::AF1, ExternalTriggerFilter}, \
    {ST_LIB::PE8, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_1_Negated},     \
    {ST_LIB::PE9, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_1},             \
    {ST_LIB::PE10, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_2_Negated},    \
    {ST_LIB::PE11, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_2},            \
    {ST_LIB::PE12, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_3_Negated},    \
    {ST_LIB::PE13, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_3},            \
    {ST_LIB::PE14, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_4},            \
                                                                                      \
    {ST_LIB::PE15, ST_LIB::GPIODomain::AlternateFunction::AF1, BreakInput_1},         \
    {ST_LIB::PE15, ST_LIB::GPIODomain::AlternateFunction::AF13, BreakInputCompare_1}, \
                                                                                      \
    {ST_LIB::PB12, ST_LIB::GPIODomain::AlternateFunction::AF1, BreakInput_1},         \
    {ST_LIB::PB12, ST_LIB::GPIODomain::AlternateFunction::AF13, BreakInputCompare_1}, \
                                                                                      \
    {ST_LIB::PB13, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_1_Negated},    \
    {ST_LIB::PB14, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_2_Negated},    \
    {ST_LIB::PB15, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_3_Negated},    \
                                                                                      \
    {ST_LIB::PG4, ST_LIB::GPIODomain::AlternateFunction::AF1, BreakInput_2},          \
    {ST_LIB::PG4, ST_LIB::GPIODomain::AlternateFunction::AF11, BreakInputCompare_2},  \
                                                                                      \
    {ST_LIB::PG5, ST_LIB::GPIODomain::AlternateFunction::AF1, ExternalTriggerFilter}, \
                                                                                      \
    {ST_LIB::PA8, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_1},             \
    {ST_LIB::PA9, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_2},             \
    {ST_LIB::PA10, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_3},            \
    {ST_LIB::PA11, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_4},            \
                                                                                      \
    {ST_LIB::PA12, ST_LIB::GPIODomain::AlternateFunction::AF1, ExternalTriggerFilter},\
    {ST_LIB::PA12, ST_LIB::GPIODomain::AlternateFunction::AF12, BreakInput_2},

    // 4 capture-compare channels
#define Tim2PinsMacro \
    {ST_LIB::PA0, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_1},              \
    {ST_LIB::PA0, ST_LIB::GPIODomain::AlternateFunction::AF1, ExternalTriggerFilter},  \
    {ST_LIB::PA1, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_2},              \
    {ST_LIB::PA2, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_3},              \
    {ST_LIB::PA3, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_4},              \
                                                                                       \
    {ST_LIB::PA5, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_1},              \
    {ST_LIB::PA5, ST_LIB::GPIODomain::AlternateFunction::AF1, ExternalTriggerFilter},  \
                                                                                       \
    {ST_LIB::PA15, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_1},             \
    {ST_LIB::PA15, ST_LIB::GPIODomain::AlternateFunction::AF1, ExternalTriggerFilter}, \
    {ST_LIB::PB3, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_2},              \
    {ST_LIB::PB10, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_3},             \
    {ST_LIB::PB11, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_4},

    // 4 capture-compare channels
#define Tim3PinsMacro \
    {ST_LIB::PA6, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_1}, \
    {ST_LIB::PA7, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_2}, \
    {ST_LIB::PB0, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_3}, \
    {ST_LIB::PB1, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_4}, \
                                                                          \
    {ST_LIB::PB4, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_1}, \
    {ST_LIB::PB5, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_2}, \
                                                                          \
    {ST_LIB::PC6, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_1}, \
    {ST_LIB::PC7, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_2}, \
    {ST_LIB::PC8, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_3}, \
    {ST_LIB::PC9, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_4}, \
                                                                          \
    {ST_LIB::PD2, ST_LIB::GPIODomain::AlternateFunction::AF2, ExternalTriggerFilter},

    // 4 capture-compare channels
#define Tim4PinsMacro \
    {ST_LIB::PB6, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_1},  \
    {ST_LIB::PB7, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_2},  \
    {ST_LIB::PB8, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_3},  \
    {ST_LIB::PB9, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_4},  \
                                                                           \
    {ST_LIB::PD12, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_1}, \
    {ST_LIB::PD13, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_2}, \
    {ST_LIB::PD14, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_3}, \
    {ST_LIB::PD15, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_4}, \
                                                                           \
    {ST_LIB::PE0, ST_LIB::GPIODomain::AlternateFunction::AF2, ExternalTriggerFilter},

    // 4 capture-compare channels
#define Tim5PinsMacro \
    {ST_LIB::PA0, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_1}, \
    {ST_LIB::PA1, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_2}, \
    {ST_LIB::PA2, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_3}, \
    {ST_LIB::PA3, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_4}, \
    {ST_LIB::PA4, ST_LIB::GPIODomain::AlternateFunction::AF2, ExternalTriggerFilter},

    // anything invalid, this doesn't get checked for basic timers because they have no pins
#define Tim6PinsMacro {ST_LIB::PA0, ST_LIB::GPIODomain::AlternateFunction::NO_AF, Channel_1}
    // anything invalid, this doesn't get checked for basic timers because they have no pins
#define Tim7PinsMacro {ST_LIB::PA0, ST_LIB::GPIODomain::AlternateFunction::NO_AF, Channel_1}

    // 4 capture-compare channels
    // complementary output
#define Tim8PinsMacro \
    {ST_LIB::PA0, ST_LIB::GPIODomain::AlternateFunction::AF3, ExternalTriggerFilter}, \
    {ST_LIB::PA5, ST_LIB::GPIODomain::AlternateFunction::AF3, Channel_1_Negated},     \
                                                                                      \
    {ST_LIB::PA6, ST_LIB::GPIODomain::AlternateFunction::AF3, BreakInput_1},          \
    {ST_LIB::PA6, ST_LIB::GPIODomain::AlternateFunction::AF10, BreakInputCompare_1},  \
                                                                                      \
    {ST_LIB::PA7, ST_LIB::GPIODomain::AlternateFunction::AF3, Channel_1_Negated},     \
                                                                                      \
    {ST_LIB::PA8, ST_LIB::GPIODomain::AlternateFunction::AF3, BreakInput_2},          \
    {ST_LIB::PA8, ST_LIB::GPIODomain::AlternateFunction::AF12, BreakInputCompare_2},  \
                                                                                      \
    {ST_LIB::PB0, ST_LIB::GPIODomain::AlternateFunction::AF3, Channel_2_Negated},     \
    {ST_LIB::PB1, ST_LIB::GPIODomain::AlternateFunction::AF3, Channel_3_Negated},     \
    {ST_LIB::PB14, ST_LIB::GPIODomain::AlternateFunction::AF3, Channel_2_Negated},    \
    {ST_LIB::PB15, ST_LIB::GPIODomain::AlternateFunction::AF3, Channel_3_Negated},    \
                                                                                      \
    {ST_LIB::PC6, ST_LIB::GPIODomain::AlternateFunction::AF3, Channel_1},             \
    {ST_LIB::PC7, ST_LIB::GPIODomain::AlternateFunction::AF3, Channel_2},             \
    {ST_LIB::PC8, ST_LIB::GPIODomain::AlternateFunction::AF3, Channel_3},             \
    {ST_LIB::PC9, ST_LIB::GPIODomain::AlternateFunction::AF3, Channel_4},             \
                                                                                      \
    {ST_LIB::PG2, ST_LIB::GPIODomain::AlternateFunction::AF3, BreakInput_1},          \
    {ST_LIB::PG2, ST_LIB::GPIODomain::AlternateFunction::AF11, BreakInputCompare_1},  \
                                                                                      \
    {ST_LIB::PG3, ST_LIB::GPIODomain::AlternateFunction::AF3, BreakInput_2},          \
    {ST_LIB::PG3, ST_LIB::GPIODomain::AlternateFunction::AF11, BreakInputCompare_2},  \
                                                                                      \
    {ST_LIB::PG8, ST_LIB::GPIODomain::AlternateFunction::AF3, ExternalTriggerFilter},

    // 2 capture-compare channels
#define Tim12PinsMacro \
    {ST_LIB::PB14, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_1}, \
    {ST_LIB::PB15, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_2},

    // 1 capture-compare channel
#define Tim13PinsMacro \
    {ST_LIB::PA6, ST_LIB::GPIODomain::AlternateFunction::AF9, Channel_1}, \
    {ST_LIB::PF8, ST_LIB::GPIODomain::AlternateFunction::AF9, Channel_1},

    // 1 capture-compare channel
#define Tim14PinsMacro \
    {ST_LIB::PA7, ST_LIB::GPIODomain::AlternateFunction::AF9, Channel_1}, \
    {ST_LIB::PF9, ST_LIB::GPIODomain::AlternateFunction::AF9, Channel_1},

    // 2 capture-compare channels
#define Tim15PinsMacro \
    {ST_LIB::PA0, ST_LIB::GPIODomain::AlternateFunction::AF4, BreakInput_1},      \
                                                                                  \
    {ST_LIB::PA1, ST_LIB::GPIODomain::AlternateFunction::AF4, Channel_1_Negated}, \
    {ST_LIB::PA2, ST_LIB::GPIODomain::AlternateFunction::AF4, Channel_1},         \
    {ST_LIB::PA3, ST_LIB::GPIODomain::AlternateFunction::AF4, Channel_2},         \
                                                                                  \
    {ST_LIB::PC12, ST_LIB::GPIODomain::AlternateFunction::AF2, Channel_1},        \
    {ST_LIB::PD2, ST_LIB::GPIODomain::AlternateFunction::AF4, Channel_2},         \
                                                                                  \
    {ST_LIB::PE3, ST_LIB::GPIODomain::AlternateFunction::AF4, BreakInput_1},      \
    {ST_LIB::PE4, ST_LIB::GPIODomain::AlternateFunction::AF4, BreakInput_1},      \
                                                                                  \
    {ST_LIB::PE4, ST_LIB::GPIODomain::AlternateFunction::AF4, Channel_1},         \
    {ST_LIB::PE4, ST_LIB::GPIODomain::AlternateFunction::AF4, Channel_2},

    // 1 capture-compare channel
#define Tim16PinsMacro \
    {ST_LIB::PF6, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_1},         \
    {ST_LIB::PF8, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_1_Negated}, \
    {ST_LIB::PF10, ST_LIB::GPIODomain::AlternateFunction::AF1, BreakInput_1},

    // 1 capture-compare channel
#define Tim17PinsMacro \
    {ST_LIB::PB5, ST_LIB::GPIODomain::AlternateFunction::AF1, BreakInput_1},      \
    {ST_LIB::PB7, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_1_Negated}, \
    {ST_LIB::PB9, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_1},         \
    {ST_LIB::PF7, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_1},         \
    {ST_LIB::PF9, ST_LIB::GPIODomain::AlternateFunction::AF1, Channel_1_Negated}, \
    {ST_LIB::PG6, ST_LIB::GPIODomain::AlternateFunction::AF1, BreakInput_1},

    // 4 capture-compare channels
#define Tim23PinsMacro \
    {ST_LIB::PB2, ST_LIB::GPIODomain::AlternateFunction::AF13, ExternalTriggerFilter}, \
    {ST_LIB::PF0, ST_LIB::GPIODomain::AlternateFunction::AF13, Channel_1},             \
    {ST_LIB::PF1, ST_LIB::GPIODomain::AlternateFunction::AF13, Channel_2},             \
    {ST_LIB::PF2, ST_LIB::GPIODomain::AlternateFunction::AF13, Channel_3},             \
    {ST_LIB::PF3, ST_LIB::GPIODomain::AlternateFunction::AF13, Channel_4},             \
                                                                                       \
    {ST_LIB::PF6, ST_LIB::GPIODomain::AlternateFunction::AF13, Channel_1},             \
    {ST_LIB::PF7, ST_LIB::GPIODomain::AlternateFunction::AF13, Channel_2},             \
    {ST_LIB::PF8, ST_LIB::GPIODomain::AlternateFunction::AF13, Channel_3},             \
    {ST_LIB::PF9, ST_LIB::GPIODomain::AlternateFunction::AF13, Channel_4},             \
                                                                                       \
    {ST_LIB::PG3, ST_LIB::GPIODomain::AlternateFunction::AF13, ExternalTriggerFilter}, \
    {ST_LIB::PG12, ST_LIB::GPIODomain::AlternateFunction::AF13, Channel_1},            \
    {ST_LIB::PG13, ST_LIB::GPIODomain::AlternateFunction::AF13, Channel_2},            \
    {ST_LIB::PG14, ST_LIB::GPIODomain::AlternateFunction::AF13, Channel_3},

    // 4 capture-compare channels
#define Tim24PinsMacro \
    {ST_LIB::PB3, ST_LIB::GPIODomain::AlternateFunction::AF14, ExternalTriggerFilter}, \
    {ST_LIB::PF11, ST_LIB::GPIODomain::AlternateFunction::AF14, Channel_1},            \
    {ST_LIB::PF12, ST_LIB::GPIODomain::AlternateFunction::AF14, Channel_2},            \
    {ST_LIB::PF13, ST_LIB::GPIODomain::AlternateFunction::AF14, Channel_3},            \
    {ST_LIB::PF14, ST_LIB::GPIODomain::AlternateFunction::AF14, Channel_4},            \
    {ST_LIB::PG2, ST_LIB::GPIODomain::AlternateFunction::AF14, ExternalTriggerFilter},

#define X(timx, ignore) \
    constexpr TimerPossiblePin glue(tim, glue(timx, pins))[31] = { \
        glue(Tim, glue(timx, PinsMacro))                           \
    };                                                             \
    constexpr std::size_t glue(tim, glue(timx, pin_count)) = ARRAY_LENGTH(glue(tim, glue(timx, pins)));

    TimerXList

#undef X

    struct TimerPossPins {
        TimerPossiblePin pins[31];
        std::size_t pin_count;
    };
    constexpr TimerPossPins empty_pins = {
        .pins = {},
        .pin_count = 0
    };
    constexpr TimerPossPins tim_pins[25] = {
        empty_pins,                         /* 0 */
        {{Tim1PinsMacro}, tim1pin_count},   /* TIM1 */
        {{Tim2PinsMacro}, tim2pin_count},   /* TIM2 */
        {{Tim3PinsMacro}, tim3pin_count},   /* TIM3 */
        {{Tim4PinsMacro}, tim4pin_count},   /* TIM4 */
        {{Tim5PinsMacro}, tim5pin_count},   /* TIM5 */
        {{Tim6PinsMacro}, tim6pin_count},   /* TIM6 - won't get checked since they have no associated pins */
        {{Tim7PinsMacro}, tim7pin_count},   /* TIM7 - won't get checked since they have no associated pins */
        {{Tim8PinsMacro}, tim8pin_count},   /* TIM8 */
        empty_pins,                         /* 9 */
        empty_pins,                         /* 10 */
        empty_pins,                         /* 11 */
        {{Tim12PinsMacro}, tim12pin_count}, /* TIM12 */
        {{Tim13PinsMacro}, tim13pin_count}, /* TIM13 */
        {{Tim14PinsMacro}, tim14pin_count}, /* TIM14 */
        {{Tim15PinsMacro}, tim15pin_count}, /* TIM15 */
        {{Tim16PinsMacro}, tim16pin_count}, /* TIM16 */
        {{Tim17PinsMacro}, tim17pin_count}, /* TIM17 */
        empty_pins,                         /* 18 */
        empty_pins,                         /* 19 */
        empty_pins,                         /* 20 */
        empty_pins,                         /* 21 */
        empty_pins,                         /* 22 */
        {{Tim23PinsMacro}, tim23pin_count}, /* TIM23 */
        {{Tim24PinsMacro}, tim24pin_count}, /* TIM24 */
    };

    if(req == TimerRequest::AnyGeneralPurpose || req == TimerRequest::Any32bit)
    {
        ST_LIB::compile_error("Any* timers can't use pins");
        return GPIODomain::AlternateFunction::NO_AF;
    }
    if(req == TimerRequest::Basic_6 || req == TimerRequest::Basic_7)
    {
        ST_LIB::compile_error("Basic timers can't use pins");
        return GPIODomain::AlternateFunction::NO_AF;
    }

    if(pin.channel == TimerChannel::CHANNEL_NEGATED_FLAG) {
        ST_LIB::compile_error("You're not supporsed to use TimerChannel::CHANNEL_NEGATED_FLAG as a channel nº");
        return GPIODomain::AlternateFunction::NO_AF;
    }

    bool found = false;
    for(std::size_t j = 0; j < tim_pins[(int)req].pin_count; j++) {
        if(pin.af == ST_LIB::TimerAF::None) {
            ST_LIB::compile_error("Error: Timers with pins must have associated TimerAF (alternate functions)");
        } else if(((pin.af == ST_LIB::TimerAF::InputCapture || pin.af == ST_LIB::TimerAF::PWM || pin.af == ST_LIB::TimerAF::Encoder) && 
            (static_cast<uint8_t>(pin.channel) == static_cast<uint8_t>(tim_pins[(int)req].pins[j].use))) ||

            ((pin.af == ST_LIB::TimerAF::BreakInput) &&
            (tim_pins[(int)req].pins[j].use == BreakInput_1 || tim_pins[(int)req].pins[j].use == BreakInput_2)) ||

            ((pin.af == ST_LIB::TimerAF::BreakInputCompare) &&
            (tim_pins[(int)req].pins[j].use == BreakInputCompare_1 || tim_pins[(int)req].pins[j].use == BreakInputCompare_2)))
        {
            found = true;
            return tim_pins[(int)req].pins[j].af;
        }
    }
    if(!found) {
        ST_LIB::compile_error("Error: Couldn't find any pins with the requested alternate function");
    }

    return GPIODomain::AlternateFunction::NO_AF;
}

} // namespace ST_LIB

#endif // HAL_TIM_MODULE_ENABLED

/* Old init code from TimerPeripheral.cpp, some might be recycled

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if(HAL_TIMEx_MasterConfigSynchronization(handle, &sMasterConfig) != HAL_OK) {
        ErrorHandler("Unable to configure master synch on %s", e.name);
    }
*/