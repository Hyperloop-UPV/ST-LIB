/*
 * TimerDomain.hpp
 *
 *  Created on: 3 dic. 2025
 *      Author: victor
 */

#pragma once

#include "stm32h7xx_hal.h"
//#include "stm32h7xx_hal_tim.h"

#ifdef HAL_TIM_MODULE_ENABLED

#include "HALAL/Models/GPIO.hpp"

#include <span>
#include <array>
#include <initializer_list>

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
};

struct TimerPin {
    TimerAF af;
    ST_LIB::GPIODomain::Pin pin;
    uint8_t channel;
};

constexpr std::array<int, 25> create_timer_idxmap() {
    std::array<int, 25> result{};
    
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

static constexpr std::array<int, 25> timer_idxmap = create_timer_idxmap();

struct TimerDomain {
    // There are 16 timers
    static constexpr std::size_t max_instances = 16;

    enum CountingMode : uint8_t {
        UP = 0,
        DOWN = 1,
        /* center-aligned = counter counts up and down alternatively */
        /* Output compare interrupt flags of channels configured in output (CCxS=00 in TIMx_CCMRx register) are
           set only when the counter is counting down */
        CENTER_ALIGNED_INTERRUPT_DOWN = 2,
        /* Output compare interrupt flags of channels configured in output (CCxS=00 in TIMx_CCMRx register) are
           set only when the counter is counting up */
        CENTER_ALIGNED_INTERRUPT_UP = 3,
        /* both up and down */
        CENTER_ALIGNED_INTERRUPT_BOTH = 4,
    };

    enum Kind : uint8_t {
        Basic,
        GeneralPurpose,
        Advanced,
    };
    
    struct Entry {
        std::array<char, 8> name; /* max length = 7 */
        TimerRequest request;
        TimerDomain::CountingMode counting_mode;
        uint32_t deadtime;
        uint32_t polarity;
        uint32_t negated_polarity;
        uint8_t pin_count;
        std::array<TimerPin, 4> pins;
    };

    struct Config {
        char name[8]; /* "Timerxx\0" */
        TimerDomain::Kind kind;
        uint16_t timer_idx;
        TimerDomain::CountingMode counting_mode;
        uint32_t deadtime;
        uint32_t polarity;
        uint32_t negated_polarity;
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

    static constexpr Config DoTimer(const Entry request, int reqint, int reqidx) {
        Config cfg;
        if(request.name[0] == '\0') {
            /* "Timer" + tostring(reqint) */
            cfg.name[0] = 'T';
            cfg.name[1] = 'i';
            cfg.name[2] = 'm';
            cfg.name[3] = 'e';
            cfg.name[4] = 'r';
            cfg.name[5] = (reqint/10) + '0';
            cfg.name[6] = (reqint%10) + '0';
            cfg.name[7] = '\0';
        } else {
            for(int si = 0; si < 8; si++) {
                cfg.name[si] = request.name[si];
            }
        }
        cfg.timer_idx = timer_idxmap[reqint];
        cfg.deadtime = request.deadtime;
        cfg.polarity = request.polarity;
        cfg.negated_polarity = request.negated_polarity;

        // Do any compile time checks needed for the timers...
        if(!(reqint == 1 || reqint == 8 ||
            reqint == 2 || reqint == 5 || reqint == 23 || reqint == 24 || 
            reqint == 3 || reqint == 4))
        {
            if(request.counting_mode != CountingMode::UP) {
                ST_LIB::compile_error("Error: Timers other than {Advanced{TIM1, TIM8}, TIM2, TIM3, TIM4, TIM5, TIM23, TIM24} only support upcounting");
            }
        }

        if(request.request == Basic_6 || request.request == Basic_7) {
            // basic timers
            cfg.kind = TimerDomain::Kind::Basic;
        } else if(request.request == Advanced_1 || request.request == Advanced_8) {
            // advanced timers
            cfg.kind = TimerDomain::Kind::Advanced;
        } else {
            if(cfg.timer_idx >= 0 && cfg.timer_idx <= 5) {
                // general purpose timers 1
            } else if(cfg.timer_idx >= 6 && cfg.timer_idx <= 8) {
                // general purpose timers 2
            } else if(cfg.timer_idx >= 9 && cfg.timer_idx <= 11) {
                // general purpose timers 3
            } else {
                ST_LIB::compile_error("Unknown timer idx");
            }

            cfg.kind = TimerDomain::Kind::GeneralPurpose;
        }

        return cfg;
    }

    static constexpr std::array<char, 8> EMPTY_TIMER_NAME = {0,0,0,0, 0,0,0,0};

    struct Timer {
        using domain = TimerDomain;
        Entry e;

        consteval Timer(TimerRequest request = TimerRequest::AnyGeneralPurpose, 
            TimerDomain::CountingMode counting_mode = CountingMode::UP, 
            std::array<char, 8> name = EMPTY_TIMER_NAME, uint32_t deadtime = 0, 
            uint32_t polarity = TIM_OCPOLARITY_HIGH, uint32_t negated_polarity = TIM_OCPOLARITY_HIGH,
            std::initializer_list<TimerPin> pinargs = {})
        {
            e.name = name;
            e.request = request;
            e.counting_mode = counting_mode;
            e.deadtime = deadtime;
            e.polarity = polarity;
            e.negated_polarity = negated_polarity;
            
            e.pin_count = pinargs.size();
            if(pinargs.size() > 4) {
                ST_LIB::compile_error("Max 4 pins per timer");
            }
            int i = 0;
            for(TimerPin p : pinargs) {
                e.pins[i] = p;
                i++;
            }
        }

        // anything not initialized will be 0
        consteval Timer(Entry e, std::initializer_list<TimerPin> pinargs = {}) {
            this->e.name = e.name;
            this->e.request = e.request;
            this->e.counting_mode = e.counting_mode;
            this->e.deadtime = e.deadtime;
            this->e.polarity = e.polarity;
            this->e.negated_polarity = e.negated_polarity;
            if(pinargs.size() == 0) {
                this->e.pin_count = e.pin_count;
                this->e.pins = e.pins;
            } else {
                e.pin_count = pinargs.size();
                if(pinargs.size() > 4) {
                    ST_LIB::compile_error("Max 4 pins per timer");
                }
                int i = 0;
                for(TimerPin p : pinargs) {
                    e.pins[i] = p;
                    i++;
                }
            }
        }
        
        template<class Ctx>
        consteval void inscribe(Ctx &ctx) const {
            ctx.template add<TimerDomain>(e, this);
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

        check_pins(requests);

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

                Config cfg = DoTimer(requests[i], reqint, i);
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
                Config cfg = DoTimer(requests[i], reqint, i);
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
            Config cfg = DoTimer(requests[i], reqint, i);
            cfgs[cfg_idx++] = cfg;
        }

        return cfgs;
    }

    // Runtime object
    struct Instance {
        char name[8];
        TIM_TypeDef *tim;
        TIM_HandleTypeDef *hal_tim;
        TimerDomain::Kind kind;
        uint16_t timer_idx;
    };

    static void (*callbacks[TimerDomain::max_instances])(void*);
    static void *callback_data[TimerDomain::max_instances];

    template <std::size_t N> struct Init {
        static inline std::array<Instance, N> instances{};

        static void init(std::span<const Config, N> cfgs) {
            for(std::size_t i = 0; i < N; i++) {
                const Config &e = cfgs[i];

                TIM_HandleTypeDef *handle = (TIM_HandleTypeDef*)&hal_handles[e.timer_idx];
                TIM_TypeDef *tim = cmsis_timers[e.timer_idx];
                handle->Instance = tim;
                handle->Init.Period = 0;
                handle->Init.Prescaler = 0;
                handle->Init.CounterMode = TIM_COUNTERMODE_UP;
                handle->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
                handle->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
                handle->Init.RepetitionCounter = 0;

                /* if(e.counting_mode == CountingMode::UP) {
                    CLEAR_BIT(tim->CR1, TIM_CR1_DIR); // upcounter
                } else */
                if(e.counting_mode == CountingMode::DOWN) {
                    SET_BIT(tim->CR1, TIM_CR1_DIR); // downcounter
                } else if(e.counting_mode == CountingMode::CENTER_ALIGNED_INTERRUPT_DOWN) {
                    MODIFY_REG(tim->CR1, TIM_CR1_CMS, TIM_CR1_CMS_0);
                } else if(e.counting_mode == CountingMode::CENTER_ALIGNED_INTERRUPT_UP) {
                    MODIFY_REG(tim->CR1, TIM_CR1_CMS, TIM_CR1_CMS_1);
                } else if(e.counting_mode == CountingMode::CENTER_ALIGNED_INTERRUPT_BOTH) {
                    MODIFY_REG(tim->CR1, TIM_CR1_CMS, (TIM_CR1_CMS_0 | TIM_CR1_CMS_1));
                }

                rcc_enable_timer(tim);

                // InputCapture stuff should be dome somewhere else..
                // PWM stuff should be done somewhere else..

                Instance *inst = &instances[i];
                inst->tim = tim;
                inst->hal_tim = handle;
                inst->name[0] = e.name[0];
                inst->name[1] = e.name[1];
                inst->name[2] = e.name[2];
                inst->name[3] = e.name[3];
                inst->name[4] = e.name[4];
                inst->name[5] = e.name[5];
                inst->name[6] = e.name[6];
                inst->name[7] = e.name[7];
                inst->kind = e.kind;
                inst->timer_idx = e.timer_idx;
                __NOP();
            }
        }
    };

    static consteval void check_pins(std::span<const Entry> requests)
    {
        /* good luck n_n */
        for(std::size_t i = 0; i < requests.size(); i++) {
            const Entry &e = requests[i];

            switch(e.request) {
                case TimerRequest::AnyGeneralPurpose:
                case TimerRequest::Any32bit: {
                    if(e.pin_count > 0) {
                        ST_LIB::compile_error("Any* timers can't use pins");
                    }
                } break;

                case TimerRequest::Advanced_1: {
                    // 4 capture-compare channels
                    // complementary output
                    
                    /* TIM1 pins
                     * PE6:
                     * - TIM1_BKIN2: AF1
                     * - TIM1_BKIN2_COMP12: AF12
                     * PA6:
                     * - TIM1_BKIN: AF1
                     * - TIM1_BKIN_COMP12: AF11
                     * PA7:
                     * - TIM1_CH1N: AF1
                     * PB0:
                     * - TIM1_CH2N: AF1
                     * PB1:
                     * - TIM1_CH3N: AF1
                     * PE7:
                     * - TIM1_ETR: AF1
                     * PE8:
                     * - TIM1_CH1N: AF1
                     * PE9:
                     * - TIM1_CH1: AF1
                     * PE10:
                     * - TIM1_CH2N: AF1
                     * PE11:
                     * - TIM1_CH2: AF1
                     * PE12:
                     * - TIM1_CH3N: AF1
                     * PE13:
                     * - TIM1_CH3: AF1
                     * PE14:
                     * - TIM1_CH4: AF1
                     * PE15:
                     * - TIM1_BKIN: AF1
                     * - TIM1_BKIN_COMP12: AF13
                     * PB12:
                     * - TIM1_BKIN: AF1
                     * - TIM1_BKIN_COMP12: AF13
                     * PB13:
                     * - TIM1_CH1N: AF1
                     * PB14:
                     * - TIM1_CH2N: AF1
                     * PB15:
                     * - TIM1_CH3N: AF1
                     * PG4:
                     * - TIM1_BKIN2: AF1
                     * - TIM1_BKIN2_COMP12: AF11
                     * PG5:
                     * - TIM1_ETR: AF1
                     * PA8:
                     * - TIM1_CH1: AF1
                     * PA9:
                     * - TIM1_CH2: AF1
                     * PA10:
                     * - TIM1_CH3: AF1
                     * PA11:
                     * - TIM1_CH4: AF1
                     * PA12:
                     * - TIM1_ETR: AF1
                     * - TIM1_BKIN2: AF12
                     * 
                     */
                } break;

                case TimerRequest::GeneralPurpose32bit_2: {
                    // 4 capture-compare channels
                    
                    /* TIM2 pins
                     * PA0:
                     * - TIM2_CH1: AF1
                     * - TIM2_ETR: AF1
                     * PA1:
                     * - TIM2_CH2: AF1
                     * PA2:
                     * - TIM2_CH3: AF1
                     * PA3:
                     * - TIM2_CH4: AF1
                     * PA5:
                     * - TIM2_CH1: AF1
                     * - TIM2_ETR: AF1
                     * PA15:
                     * - TIM2_CH1: AF1
                     * - TIM2_ETR: AF1
                     * PB3:
                     * - TIM2_CH2: AF1
                     * PB10:
                     * - TIM2_CH3: AF1
                     * PB11:
                     * - TIM2_CH4: AF1
                     */
                } break;

                case TimerRequest::GeneralPurpose_3: {
                    // 4 capture-compare channels

                    /* TIM3 pins:
                     * PA6:
                     * - TIM3_CH1: AF2
                     * PA7:
                     * - TIM3_CH2: AF2
                     * PB0:
                     * - TIM3_CH3: AF2
                     * PB1:
                     * - TIM3_CH4: AF2
                     * PB4:
                     * - TIM3_CH1: AF2
                     * PB5:
                     * - TIM3_CH2: AF2
                     * PC6:
                     * - TIM3_CH1: AF2
                     * PC7:
                     * - TIM3_CH2: AF2
                     * PC8:
                     * - TIM3_CH3: AF2
                     * PC9:
                     * - TIM3_CH4: AF2
                     * PD2:
                     * - TIM3_ETR: AF2
                     */
                } break;

                case TimerRequest::GeneralPurpose_4: {
                    // 4 capture-compare channels

                    /* TIM4 pins:
                     * PB6:
                     * - TIM4_CH1: AF2
                     * PB7:
                     * - TIM4_CH2: AF2
                     * PB8:
                     * - TIM4_CH3: AF2
                     * PB9:
                     * - TIM4_CH4: AF2
                     * PD12:
                     * - TIM4_CH1: AF2
                     * PD13:
                     * - TIM4_CH2: AF2
                     * PD14:
                     * - TIM4_CH3: AF2
                     * PD15:
                     * - TIM4_CH4: AF2
                     * PE0:
                     * - TIM4_ETR: AF2
                     */
                } break;

                case TimerRequest::GeneralPurpose32bit_5: {
                    // 4 capture-compare channels

                    /* TIM5 pins:
                     * PA0:
                     * - TIM5_CH1: AF2
                     * PA1:
                     * - TIM5_CH2: AF2
                     * PA2:
                     * - TIM5_CH3: AF2
                     * PA3:
                     * - TIM5_CH4: AF2
                     * PA4:
                     * - TIM5_ETR: AF2
                     */
                } break;

                case TimerRequest::Basic_6:
                case TimerRequest::Basic_7: {
                    if(e.pin_count > 0) {
                        ST_LIB::compile_error("Basic timers can't use pins");
                    }
                } break;

                case TimerRequest::Advanced_8: {
                    // 4 capture-compare channels
                    // complementary output

                    /* TIM8 pins:
                     * PA0:
                     * - TIM8_ETR: AF3
                     * PA5:
                     * - TIM8_CH1N: AF3
                     * PA6:
                     * - TIM8_BKIN: AF3
                     * - TIM8_BKIN_COMP12: AF10
                     * PA7:
                     * - TIM8_CH1N: AF3
                     * PA8:
                     * - TIM8_BKIN2: AF3
                     * - TIM8_BKIN2_COMP12: AD12
                     * PB0:
                     * - TIM8_CH2N: AF3
                     * PB1:
                     * - TIM8_CH3N: AF3
                     * PB14:
                     * - TIM8_CH2N: AF3
                     * PB15:
                     * - TIM8_CH3N: AF3
                     * PC6:
                     * - TIM8_CH1: AF3
                     * PC7:
                     * - TIM8_CH2: AF3
                     * PC8:
                     * - TIM8_CH3: AF3
                     * PC9:
                     * - TIM8_CH4: AF3
                     * PG2:
                     * - TIM8_BKIN: AF3
                     * - TIM8_BKIN_COMP12: AF11
                     * PG3:
                     * - TIM8_BKIN2: AF3
                     * - TIM8_BKIN2_COMP12: AF11
                     * PG8:
                     * - TIM8_ETR: AF3
                     */
                } break;

                case TimerRequest::SlaveTimer_12: {
                    // 2 capture-compare channels

                    /* TIM12 pins:
                     * PB14:
                     * - TIM12_CH1: AF2
                     * PB15:
                     * - TIM12_CH2: AF2
                     */
                } break;

                case TimerRequest::SlaveTimer_13: {
                    // 1 capture-compare channel

                    /* TIM13 pins:
                     * PA6:
                     * - TIM13_CH1: AF9
                     * PF8:
                     * - TIM13_CH1: AF9
                     */
                } break;

                case TimerRequest::SlaveTimer_14: {
                    // 1 capture-compare channel

                    /* TIM14 pins:
                     * PA7:
                     * - TIM14_CH1: AF9
                     * PF9:
                     * - TIM14_CH1: AF9
                     */
                } break;

                case TimerRequest::GeneralPurpose_15: {
                    // 2 capture-compare channels

                    /* TIM15 pins:
                     * PA0:
                     * - TIM15_BKIN: AF4
                     * PA1:
                     * - TIM15_CH1N: AF4
                     * PA2:
                     * - TIM15_CH1: AF4
                     * PA3:
                     * - TIM15_CH2: AF4
                     * PC12:
                     * - TIM15_CH1: AF2
                     * PD2:
                     * - TIM15_BKIN: AF4
                     * PE3:
                     * - TIM15_BKIN: AF4
                     * PE4:
                     * - TIM15_CH1N: AF4
                     * PE5:
                     * - TIM15_CH1: AF4
                     * PE6:
                     * - TIM15_CH2: AF4
                     */
                } break;

                case TimerRequest::GeneralPurpose_16: {
                    // 1 capture-compare channel

                    /* TIM16 pins:
                     * PF6:
                     * - TIM16_CH1: AF1
                     * PF8:
                     * - TIM16_CH1N: AF1
                     * PF10:
                     * - TIM16_BKIN: AF1
                     */
                } break;

                case TimerRequest::GeneralPurpose_17: {
                    // 1 capture-compare channel

                    /* TIM17 pins:
                     * PB5:
                     * - TIM17_BKIN: AF1
                     * PB7:
                     * - TIM17_CH1N: AF1
                     * PB9:
                     * - TIM17_CH1: AF1
                     * PF7:
                     * - TIM17_CH1: AF1
                     * PF9:
                     * - TIM17_CH1N: AF1
                     * PG6:
                     * - TIM17_BKIN: AF1
                     */
                } break;

                case TimerRequest::GeneralPurpose32bit_23: {
                    // 4 capture-compare channels

                } break;

                case TimerRequest::GeneralPurpose32bit_24: {
                    // 4 capture-compare channels

                } break;
            }
        }
    }
};
} // namespace ST_LIB

#endif // HAL_TIM_MODULE_ENABLED

/* Old init code from TimerPeripheral.cpp, some might be recycled

                TIM_MasterConfigTypeDef sMasterConfig = {0};
                TIM_IC_InitTypeDef sConfigIC = {0};
                TIM_OC_InitTypeDef sConfigOC = {0};
                TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

                TIM_HandleTypeDef *handle = &hal_handles[e.timer_idx];
                handle->Instance = cmsis_timers[e.timer_idx];
                handle->Init.Prescaler = e.prescaler;
                handle->Init.CounterMode = TIM_COUNTERMODE_UP;
                handle->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
                // PWM stuff should be done somewhere else..
                handle->Init.Period = e.period;
                handle->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
                handle->Init.RepetitionCounter = 0;
                
                if(e.type == TIM_TYPE::BASE) {
                    if(HAL_TIM_Base_Init(handle) != HAL_OK) {
                        // NOTE: In TimerPeripheral.cpp this is %d for a string ???
                        ErrorHandler("Unable to init base timer on %s", e.name);
                    }
                }

                // InputCapture stuff should be dome somewhere else..
                // PWM stuff should be done somewhere else..
                
                sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
                sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
                sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
                if(HAL_TIMEx_MasterConfigSynchronization(handle, &sMasterConfig) != HAL_OK) {
                    ErrorHandler("Unable to configure master synch on %s", e.name);
                }

                // InputCapture stuff should be dome somewhere else..
                // PWM stuff should be done somewhere else..

                sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
                sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
                sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
                sBreakDeadTimeConfig.DeadTime = e.deadtime;
                sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
                sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
                sBreakDeadTimeConfig.BreakFilter = 0;
                sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
                sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
                sBreakDeadTimeConfig.Break2Filter = 0;
                sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
                if(HAL_TIMEx_ConfigBreakDeadTime(handle, &sBreakDeadTimeConfig) != HAL_OK) {
                    ErrorHandler("Unable to configure break dead time on %s", e.name);
                }
*/