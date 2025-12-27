/*
 * TimerDomain.hpp
 *
 *  Created on: 3 dic. 2025
 *      Author: victor
 */

#pragma once

#include "stm32h7xx_hal_tim.h"

// NOTE: only works for static arrays
#define ARRAY_LENGTH(a) (sizeof(a)/sizeof(*a))

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim12;
extern TIM_HandleTypeDef htim13;
extern TIM_HandleTypeDef htim14;
extern TIM_HandleTypeDef htim15;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;
extern TIM_HandleTypeDef htim23;
extern TIM_HandleTypeDef htim24;

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
class TimerDomain {
    static constexpr std::array<int, 25> create_idxmap() {
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

    static constexpr std::array<int, 25> idxmap = create_idxmap();

    static const TIM_HandleTypeDef *hal_handles[16] = {
        // general purpose timers
        &htim2, &htim3, &htim4, &htim5, &htim23, &htim24,
        &htim12, &htim13, &htim14,
        &htim15, &htim16, &htim17,

        // basic timers
        &htim6, &htim7,

        // advanced control timers
        &htim1, &htim8
    };

    static const TIM_TypeDef *cmsis_timers[16] = {
        // general purpose timers
        TIM2, TIM3, TIM4, TIM5, TIM23, TIM24,
        TIM12, TIM13, TIM14,
        TIM15, TIM16, TIM17,

        // basic timers
        TIM6, TIM7,

        // advanced control timers
        TIM1, TIM8
    };

    static inline void rcc_enable_timer(TIM_TypeDef *tim) {
        switch(tim) {
            case TIM2: SET_BIT(RCC->APB1LENR, RCC_APB1LENR_TIM2EN); break;
            case TIM3: SET_BIT(RCC->APB1LENR, RCC_APB1LENR_TIM3EN); break;
            case TIM4: SET_BIT(RCC->APB1LENR, RCC_APB1LENR_TIM4EN); break;
            case TIM5: SET_BIT(RCC->APB1LENR, RCC_APB1LENR_TIM5EN); break;
            case TIM6: SET_BIT(RCC->APB1LENR, RCC_APB1LENR_TIM6EN); break;
            case TIM7: SET_BIT(RCC->APB1LENR, RCC_APB1LENR_TIM7EN); break;
            case TIM12: SET_BIT(RCC->APB1LENR, RCC_APB1LENR_TIM12EN); break;
            case TIM13: SET_BIT(RCC->APB1LENR, RCC_APB1LENR_TIM13EN); break;
            case TIM14: SET_BIT(RCC->APB1LENR, RCC_APB1LENR_TIM14EN); break;

            case TIM24: SET_BIT(RCC->APB1HENR, RCC_APB1HENR_TIM24EN); break;
            case TIM23: SET_BIT(RCC->APB1HENR, RCC_APB1HENR_TIM23EN); break;

            case TIM17: SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM17EN); break;
            case TIM16: SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM16EN); break;
            case TIM15: SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM15EN); break;
            case TIM8: SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM8EN); break;
            case TIM1: SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN); break;
        }
    }

    // Do any compile time checks needed for the timers...
    static consteval bool check_timer(Config *cfg, const Entry req, int reqidx) {
        if(req.period == 0) {
            ErrorInRequestN("Error: In request reqidx: period must be greater than 0 (>0)", reqidx);
            return false;
        }

        uint8_t reqint = static_cast<uint8_t>(req.request);
        if(!(reqint == 2 || reqint == 5 || reqint == 23 || reqint == 24)) {
            if(req.period > 0xFFFF) {
                ErrorInRequestN("Error: In request reqidx: Timers other than {TIM2, TIM5, TIM23, TIM24} have a maximum period of 0xFFFF (they are uint16_t)", reqidx);
                return false;
            }
        }

        if(!(reqint == 1 || reqint == 8 ||
            reqint == 2 || reqint == 5 || reqint == 23 || reqint == 24 || 
            reqint == 3 || reqint == 4))
        {
            if(req.counting_mode != CountingMode::UP) {
                ErrorInRequestN("Error: In request reqidx: Timers other than {Advanced{TIM1, TIM8}, TIM2, TIM3, TIM4, TIM5, TIM23, TIM24} only support upcounting", reqidx);
                return false;
            }
        }

        if(req.request == Basic1 || req.request == Basic2) {
            // basic timers
            cfg->kind = TimerDomain::Kind::Basic;
        } else if(req.request == Advanced1 || req.request == Advanced2) {
            // advanced timers
            cfg->kind = TimerDomain::Kind::Advanced;
        } else {
            if(cfg->timer_idx >= 0 && cfg->timer_idx <= 5) {
                // general purpose timers 1
            } else if(cfg->timer_idx >= 6 && cfg->timer_idx <= 8) {
                // general purpose timers 2
            } else if(cfg->timer_idx >= 9 && cfg->timer_idx <= 11) {
                // general purpose timers 3
            } else {
                ErrorInRequestN("Unknown timer idx in reqidx", reqidx);
            }

            cfg->kind = TimerDomain::Kind::GeneralPurpose;
        }
    }

    /* to show the error with an index */
    static consteval ErrorInRequestN(char *str, int n)
    {
        switch(n) {
            case 0: compile_error(str);
            case 1: compile_error(str);
            case 2: compile_error(str);
            case 3: compile_error(str);
            case 4: compile_error(str);
            case 5: compile_error(str);
            case 6: compile_error(str);
            case 7: compile_error(str);
            case 8: compile_error(str);
            case 9: compile_error(str);
            case 10: compile_error(str);
            case 11: compile_error(str);
            case 12: compile_error(str);
            case 13: compile_error(str);
            case 14: compile_error(str);
            case 15: compile_error(str);
        }
    }

    Config DoTimer(const Entry request, uint8_t reqint, int reqidx, const char *name_too_long_msg) {
        Config cfg;
        if(request.name.length() == 0) {
            // "Timer" + tostring(reqint)
            cfg.name[0] = 'T';
            cfg.name[1] = 'i';
            cfg.name[2] = 'm';
            cfg.name[3] = 'e';
            cfg.name[4] = 'r';
            cfg.name[5] = (reqint/10) + '0';
            cfg.name[6] = (reqint%10) + '0';
            cfg.name[7] = '\0';
        } else {
            if(request.name.length() >= sizeof(cfg.name)) {
                ErrorInRequestN(name_too_long_msg, reqidx);
            }
            for(int si = 0; si < request.name.length(); si++) {
                cfg.name[si] = request.name[si];
            }
            cfg.name[request.name.length()] = '\0';
        }
        cfg.timer_idx = TimerDomain.idxmap[reqint];
        cfg.prescaler = request.prescaler;
        cfg.period = request.period;
        cfg.deadtime = request.deadtime;
        cfg.polarity = request.polarity;
        cfg.negated_polarity = request.negated_polarity;

        check_timer(&cfg, request, i);
        return cfg;
    }

    enum Kind : uint8_t {
        Basic,
        GeneralPurpose,
        Advanced,
    };

public:
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

    enum PWM_MODE : uint8_t {
        NORMAL = 0,
        PHASED = 1,
        CENTER_ALIGNED = 2,
    };

    struct PWMData {
        uint32_t channel;
        PWM_MODE mode;
    };

    /* The number corresponds with the timer nº */
    enum TimerRequest : uint8_t {
        Advanced1 = 1,
        Advanced2 = 8,

        AnyGeneralPurpose = 0xFF,
        GeneralPurpose1 = 2,
        GeneralPurpose2 = 3,
        GeneralPurpose3 = 4,
        GeneralPurpose4 = 5,
        GeneralPurpose5 = 23,
        GeneralPurpose6 = 24,

        GeneralPurpose7 = 12,
        GeneralPurpose8 = 13,
        GeneralPurpose9 = 14,

        GeneralPurpose10 = 15,
        GeneralPurpose11 = 16,
        GeneralPurpose12 = 17,

        Basic1 = 6,
        Basic2 = 7,
    };

    struct Entry {
        string name;
        TimerRequest request;
        ST_LIB::GPIODomain::Pin *pin;
        TimerDomain::CountingMode counting_mode;
        uint16_t prescaler;
        uint32_t period;
        uint32_t deadtime;
        uint32_t polarity;
        uint32_t negated_polarity;
    };

    struct Device {
        using domain = TimerDomain;
        Entry e;

        consteval Device(string name = "", TimerRequest request = TimerRequest::AnyGeneralPurpose, 
            ST_LIB::GPIODomain::Pin *pin = 0, TimerDomain::CountingMode counting_mode = CountingMode::UP,
            uint16_t prescaler = 5, uint32_t period = 55000, uint32_t deadtime = 0, 
            uint32_t polarity = TIM_OCPOLARITY_HIGH, uint32_t negated_polarity = TIM_OCPOLARITY_HIGH) :
            e(name, request, pin, counting_mode, prescaler, period, deadtime, polarity, negated_polarity) {}
        
        template<class Ctx>
        consteval void inscribe(Ctx &ctx) const {
            ctx.template add<TimerDomain>(e);
        }
    };

    // There are 16 timers
    static constexpr std::size_t max_instances = 16;

    struct Config {
        char name[8]; /* "Timerxx\0" */
        TimerDomain::Kind kind;
        uint16_t timer_idx;
        ST_LIB::GPIODomain::Pin *asociated_pin;
        TimerDomain::CountingMode counting_mode;
        uint32_t prescaler;
        uint32_t period;
        uint32_t deadtime;
        uint32_t polarity;
        uint32_t negated_polarity;
    };

    template<std::size_t N>
    static consteval std::array<Config, N> build(span<const Entry> requests) {
        array<Config, N> cfgs{};
        uint16_t cfg_idx = 0;
        bool usedTimer[25] = {0};

        if(requests.size() > max_instances) {
            throw "too many Timer requests, there are only 16 timers";
        }

        int remaining_requests[max_instances] = {};
        std::size_t count_remaining_requests = requsts.size();
        for(int i = 0; i < requests.size(); i++) remaining_requests[i] = i;

        for(int i = 0; i < requests.size(); i++) {
            if(requests[i].request != TimerRequest::AnyGeneralPurpose &&
               (requests[i].request < 1 || requests[i].request > 24 ||
               (requests[i].request > 17 && requests[i].request < 23)))
            {
                ErrorInRequestN("Invalid TimerRequest value for timer i", i);
            }
        }

        // First find any that have requested a specific timer
        for(std::size_t i = 0; i < N; i++) {
            uint8_t reqint = static_cast<uint8_t>(requests[i].request);
            if(reqint != static_cast<uint8_t>(TimerRequest::AnyGeneralPurpose)) {
                if(usedTimer[reqint]) {
                    ErrorInRequestN("Error: Timer already used. Error in request i", i);
                }
                usedTimer[reqint] = true;

                Config cfg = DoTimer(requests[i], reqint, i, "Error: In request reqidx: Timer name too large, max = 7 (sizeof cfg.name - 1)");
                cfgs[cfg_idx++] = cfg;

                // unordered remove (remaining requests is not used here so these are ordered)
                count_remaining_requests--;
                remaining_requests[i] = remaining_requests[count_remaining_requests];
            }
        }

        // 32 bit timers, very important for scheduler
        uint8_t bits32_timers[] = {2, 5, 23, 24};
        // can use any CountingMode (32 bit timers can also but they are higher priority)
        uint8_t up_down_updown_timers[] = {3, 4};
        // 16 bit timers
        uint8_t bits16_timers[] = {12, 13, 14, 15, 16, 17};
        uint8_t remaining_timers[15] = {0};
        uint8_t count_remaining_timers = 0;

        for(int i = 0; i < ARRAY_LENGTH(bits16_timers); i++) {
            if(!used_timers[bits16_timers[i]])
                remaining_timers[count_remaining_timers++] = bits16_timers[i];
        }

        for(int i = 0; i < ARRAY_LENGTH(up_down_updown_timers); i++) {
            if(!used_timers[up_down_updown_timers[i]])
                remaining_timers[count_remaining_timers++] = up_down_updown_timers[i];
        }

        for(int i = 0; i < ARRAY_LENGTH(bits32_timers); i++) {
            if(!used_timers[bits32_timers[i]])
                remaining_timers[count_remaining_timers++] = bits32_timers[i];
        }

        if(count_remaining_requests > count_remaining_timers) {
            compile_error("This should not happen");
        }

        for(int i = 0; i < count_remaining_requests; i++) {
            const Entry &e = requests[remaining_requests[i]];
            if(e.request == AnyGeneralPurpose) {
                uint8_t reqint = remaining_timers[i];
                // NOTE: I don't want to do an ordered remove so this has the real index
                Config cfg = DoTimer(requests[i], reqint, i, "Error: In one of AnyGeneralPurpose timers: Timer name too large, max = 7 (sizeof cfg.name - 1)");
                cfgs[cfg_idx++] = cfg;
            }
        }

        return cfgs;
    }

    // Runtime object
    struct Instance {
        template<auto&> friend struct TimerWrapper;

        inline void counter_enable() {
            SET_BIT(tim->CR1, TIM_CR1_CEN);
        }
        inline void counter_disable() {
            CLEAR_BIT(tim->CR1, TIM_CR1_CEN);
        }

        inline void clear_update_interrupt_flag() {
            CLEAR_BIT(tim->SR, TIM_SR_UIF);
        }

        /* Enabled by default */
        inline void enable_update_interrupt() {
            CLEAR_BIT(tim->CR1, TIM_CR1_UDIS);
        }
        inline void disable_update_interrupt() {
            SET_BIT(tim->CR1, TIM_CR1_UDIS);
        }

        /* interrupt gets called only once, counter needs to be reenabled */
        inline void set_one_pulse_mode() {
            SET_BIT(tim->CR1, TIM_CR1_OPM);
        }
        inline void multi_interrupt() {
            CLEAR_BIT(tim->CR1, TIM_CR1_OPM);
        }

        inline TIM_HandleTypeDef *get_hal_handle() {
            return hal_tim;
        }
        inline TIM_TypeDef *get_cmsis_handle() {
            return tim;
        }

        template<uint16_t psc = 0>
        inline void configure(void (*callback)()) {
            if constexpr (psc != 0) {
                tim->PSC = psc;
            }
            this.callback = callback;
            this.counter_enable();
        }
        
        // leftover from old TimerPeripheral, maybe this was useful?
        inline uint16_t get_prescaler() {
            return tim->PSC;
        }
        inline uint32_t get_period() {
            return tim->ARR;
        }
    };

    template<Device &dev>
    struct TimerWrapper {
        TIM_TypeDef *tim;
        TIM_HandleTypeDef *hal_tim;
        char name[8];
        const TimerDomain::Kind kind;
        uint16_t timer_idx;
        void (*callback)(TimerDomain::Instance);

        if constexpr (dev.e.request == TimerRequest::Advanced1 || dev.e.request == TimerRequest::Advanced2) {
            // advanced specific functions
        }

        if constexpr (dev.e.request != TimerRequest::Basic1 && dev.e.request != TimerRequest::Basic2) {
            // general purpose and advanced functions
        }

        /* WARNING: The counter _must_ be disabled to switch from edge-aligned to center-aligned mode */
        template<TimerDomain::CounterMode mode>
        inline void set_mode(void) {
            constexpr uint8_t reqint = static_cast<uint8_t>(dev.e.request);
            if constexpr (!(reqint == 1 || reqint == 8 ||
                reqint == 2 || reqint == 5 || reqint == 23 || reqint == 24 || 
                reqint == 3 || reqint == 4))
            {
                compile_error("Error: In request reqidx: Timers other than {Advanced{TIM1, TIM8}, TIM2, TIM3, TIM4, TIM5, TIM23, TIM24} only support upcounting");
                return;
            }

            if constexpr (mode == CounterMode::UP) {
                MODIFY_REG(tim->CR1, TIM_CR1_CMS, 0);
                CLEAR_BIT(tim->CR1, TIM_CR1_DIR); // upcounter
            } else if constexpr (mode == CounterMode::DOWN) {
                MODIFY_REG(tim->CR1, TIM_CR1_CMS, 0);
                SET_BIT(tim->CR1, TIM_CR1_DIR); // downcounter
            } else if constexpr (mode == CounterMode::CENTER_ALIGNED_INTERRUPT_DOWN) {
                MODIFY_REG(tim->CR1, TIM_CR1_CMS, TIM_CR1_CMS_0);
            } else if constexpr (mode == CounterMode::CENTER_ALIGNED_INTERRUPT_UP) {
                MODIFY_REG(tim->CR1, TIM_CR1_CMS, TIM_CR1_CMS_1);
            } else if constexpr (mode == CounterMode::CENTER_ALIGNED_INTERRUPT_BOTH) {
                MODIFY_REG(tim->CR1, TIM_CR1_CMS, (TIM_CR1_CMS_0 | TIM_CR1_CMS_1));
            }
        }
    };

    template <std::size_t N> struct Init {
        static inline std::array<Instance, N> instances{};

        static void init(std::span<const Config, N> cfgs) {
            static_assert(N > 0);
            for(std::size_t i = 0; i < N; i++) {
                const Config &e = cfgs[i];

                TIM_HandleTypeDef *handle = &hal_handles[e.timer_idx];
                handle->Instance = cmsis_timers[e.timer_idx];
                handle->Init.Prescaler = e.prescaler;
                handle->Init.CounterMode = TIM_COUNTERMODE_UP;
                handle->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
                handle->Init.Period = e.period;
                handle->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
                handle->Init.RepetitionCounter = 0;

                TIM_TypeDef *tim = &cmsis_timers[e.timer_idx];
                tim->PSC = e.prescaler;
                tim->ARR = e.period;

                if constexpr (e.counting_mode == CounterMode::UP) {
                    CLEAR_BIT(tim->CR1, TIM_CR1_DIR); // upcounter
                } else if constexpr (e.counting_mode == CounterMode::DOWN) {
                    SET_BIT(tim->CR1, TIM_CR1_DIR); // downcounter
                } else if constexpr (e.counting_mode == CounterMode::CENTER_ALIGNED_INTERRUPT_DOWN) {
                    MODIFY_REG(tim->CR1, TIM_CR1_CMS, TIM_CR1_CMS_0);
                } else if constexpr (e.counting_mode == CounterMode::CENTER_ALIGNED_INTERRUPT_UP) {
                    MODIFY_REG(tim->CR1, TIM_CR1_CMS, TIM_CR1_CMS_1);
                } else if constexpr (e.counting_mode == CounterMode::CENTER_ALIGNED_INTERRUPT_BOTH) {
                    MODIFY_REG(tim->CR1, TIM_CR1_CMS, (TIM_CR1_CMS_0 | TIM_CR1_CMS_1));
                }

                rcc_enable_timer(tim);

                // InputCapture stuff should be dome somewhere else..
                // PWM stuff should be done somewhere else..

                instances[i] = Instance{
                    .tim = &cmsis_timers[e.timer_idx],
                    .hal_tim = handle,
                    .name = {e.name[0], e.name[1], e.name[2], e.name[3], e.name[4], e.name[5], e.name[6], e.name[7]},
                    .kind = e.kind,
                    .timer_idx = e.timer_idx,
                };
            }
        }
    }
};


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