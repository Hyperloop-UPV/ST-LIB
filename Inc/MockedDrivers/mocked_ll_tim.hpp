#pragma once
#include <stdint.h>
#include "MockedDrivers/stm32h723xx_wrapper.h"
#include "stm32h7xx.h"
#include "MockedDrivers/common.hpp"
#include "MockedDrivers/NVIC.hpp"
#include "MockedDrivers/Register.hpp"
#include <stdio.h>
enum class TimReg {
    Reg_CR1, Reg_CR2, Reg_SMCR, Reg_DIER, Reg_SR, Reg_EGR, 
    Reg_CCMR1, Reg_CCMR2, Reg_CCER, Reg_CNT, Reg_PSC, Reg_ARR, Reg_RCR,
    Reg_CCR1, Reg_CCR2, Reg_CCR3, Reg_CCR4, Reg_BDTR, Reg_DCR, 
    Reg_DMAR, Reg_CCMR3, Reg_CCR5, Reg_CCR6, Reg_AF1, Reg_AF2, Reg_TISEL
};
using enum TimReg;

template<TimReg Reg>
class TimerRegister : public RegisterBase<TimReg, Reg> {
public:
    using RegisterBase<TimReg, Reg>::RegisterBase;
    using RegisterBase<TimReg, Reg>::operator=;
};

static_assert(sizeof(TimerRegister<Reg_CR1>) == sizeof(uint32_t) );


struct TIM_TypeDef{
    TIM_TypeDef(void(* irq_handler)(void),IRQn_Type irq_n):
//        PSC(*this), callback{irq_handler}, irq_n{irq_n}
        callback{irq_handler}, irq_n{irq_n}
    {}

    template<TimReg Reg>
    struct PrescalerRegister : public RegisterBase<TimReg, Reg> {
        PrescalerRegister& operator=(uint32_t val) {
            this->set(val);
            // esto es lo más feo que he hecho en mucho tiempo pero no he conseguido otra cosa
            TIM_TypeDef *parent = (TIM_TypeDef*)((uint8_t*)&this->reg - offsetof(TIM_TypeDef, PSC));
            parent->active_PSC = val;
            return *this;
        }
    };

    void generate_update();
    TimerRegister<Reg_CR1> CR1;         /*!< TIM control register 1,                   Address offset: 0x00 */
    TimerRegister<Reg_CR2> CR2;         /*!< TIM control register 2,                   Address offset: 0x04 */
    TimerRegister<Reg_SMCR> SMCR;        /*!< TIM slave mode control register,          Address offset: 0x08 */
    TimerRegister<Reg_DIER> DIER;        /*!< TIM DMA/interrupt enable register,        Address offset: 0x0C */
    TimerRegister<Reg_SR> SR;          /*!< TIM status register,                      Address offset: 0x10 */
    TimerRegister<Reg_EGR> EGR;         /*!< TIM event generation register,            Address offset: 0x14 */
    TimerRegister<Reg_CCMR1> CCMR1;       /*!< TIM capture/compare mode register 1,      Address offset: 0x18 */
    TimerRegister<Reg_CCMR2> CCMR2;       /*!< TIM capture/compare mode register 2,      Address offset: 0x1C */
    TimerRegister<Reg_CCER> CCER;        /*!< TIM capture/compare enable register,      Address offset: 0x20 */
    TimerRegister<Reg_CNT> CNT;         /*!< TIM counter register,                     Address offset: 0x24 */
    PrescalerRegister<Reg_PSC> PSC;         /*!< TIM prescaler,                            Address offset: 0x28 */
    TimerRegister<Reg_ARR> ARR;         /*!< TIM auto-reload register,                 Address offset: 0x2C */
    TimerRegister<Reg_RCR> RCR;         /*!< TIM repetition counter register,          Address offset: 0x30 */
    TimerRegister<Reg_CCR1> CCR1;        /*!< TIM capture/compare register 1,           Address offset: 0x34 */
    TimerRegister<Reg_CCR2> CCR2;        /*!< TIM capture/compare register 2,           Address offset: 0x38 */
    TimerRegister<Reg_CCR3> CCR3;        /*!< TIM capture/compare register 3,           Address offset: 0x3C */
    TimerRegister<Reg_CCR4> CCR4;        /*!< TIM capture/compare register 4,           Address offset: 0x40 */
    TimerRegister<Reg_BDTR> BDTR;        /*!< TIM break and dead-time register,         Address offset: 0x44 */
    TimerRegister<Reg_DCR> DCR;         /*!< TIM DMA control register,                 Address offset: 0x48 */
    TimerRegister<Reg_DMAR> DMAR;        /*!< TIM DMA address for full transfer,        Address offset: 0x4C */
    uint32_t      RESERVED1;   /*!< Reserved, 0x50                                                 */
    TimerRegister<Reg_CCMR3> CCMR3;       /*!< TIM capture/compare mode register 3,      Address offset: 0x54 */
    TimerRegister<Reg_CCR5> CCR5;        /*!< TIM capture/compare register5,            Address offset: 0x58 */
    TimerRegister<Reg_CCR6> CCR6;        /*!< TIM capture/compare register6,            Address offset: 0x5C */
    TimerRegister<Reg_AF1> AF1;         /*!< TIM alternate function option register 1, Address offset: 0x60 */
    TimerRegister<Reg_AF2> AF2;         /*!< TIM alternate function option register 2, Address offset: 0x64 */
    TimerRegister<Reg_TISEL> TISEL;       /*!< TIM Input Selection register,             Address offset: 0x68 */
    // ========================================================================
    //  Internal Hardware State (Shadow Registers & Hidden Counters)
    // ========================================================================
    
    // Internal counter for the prescaler (counts 0 to active_PSC)
    uint32_t internal_psc_cnt = 0;

    // Internal counter for repetition (counts down from active_RCR to 0)
    uint32_t internal_rcr_cnt = 0;

    // "Shadow" registers. These hold the values currently being used by the hardware logic.
    // They are updated from the public registers (Preload registers) only on an Update Event (UEV).
    uint32_t active_PSC = 0;
    uint32_t active_ARR = 0;
    uint32_t active_RCR = 0;

    void(*callback)();
    IRQn_Type irq_n;
    bool check_CNT_increase_preconditions(){
            // Bit definitions for clarity
        const uint32_t CR1_CEN  = (1U << 0); // Counter Enable

        // 1. Check if Counter is Enabled
        if (!(CR1 & CR1_CEN)) {
            printf("TIMER IS NOT ENABLED!!\n");
            return false; 
        }
        // 2. Prescaler Logic
        // The internal prescaler counts from 0 up to active_PSC.
        // We check if we reached the limit *before* incrementing to ensure accurate 
        // behavior for PSC=0 or PSC=1.
        bool main_counter_tick = false;

        if (internal_psc_cnt >= active_PSC) {
            internal_psc_cnt = 0; // Rollover
            main_counter_tick = true;
        } else {
            internal_psc_cnt++;   // Increment
        }

        // If prescaler didn't overflow, the main counter doesn't move.
        return main_counter_tick;
    }

    void inc_cnt_and_check(uint32_t val);
};
static_assert(sizeof(TimerRegister<Reg_CNT>) == sizeof(uint32_t));
void simulate_ticks(TIM_TypeDef* tim);

/*
template<>
struct RegisterTraits<TimReg,TimReg::Reg_CNT> {
    static void write(uint32_t& target, uint32_t val) {
        TIM_TypeDef* timer = (TIM_TypeDef*)(((uint8_t*)&target)-offsetof(TIM_TypeDef, CNT));
        target = val;
        if(val != 0 && timer->check_CNT_increase_preconditions()){
            simulate_ticks(timer);
        }
    }
};
*/

#undef TIM2_BASE
#undef TIM3_BASE
#undef TIM4_BASE
#undef TIM5_BASE
#undef TIM6_BASE
#undef TIM7_BASE
#undef TIM12_BASE
#undef TIM13_BASE
#undef TIM14_BASE
#undef TIM23_BASE
#undef TIM24_BASE
#undef TIM1_BASE
#undef TIM8_BASE
#undef TIM15_BASE
#undef TIM16_BASE
#undef TIM17_BASE

extern TIM_TypeDef* TIM2_BASE;
extern "C"{ void TIM2_IRQHandler(void); }
extern TIM_TypeDef* TIM3_BASE;
extern "C"{ void TIM3_IRQHandler(void); }
extern TIM_TypeDef* TIM4_BASE;
extern "C"{ void TIM4_IRQHandler(void); }
extern TIM_TypeDef* TIM5_BASE;
extern "C"{ void TIM5_IRQHandler(void); }
extern TIM_TypeDef* TIM6_BASE;
extern "C"{ void TIM6_DAC_IRQHandler(void); }
extern TIM_TypeDef* TIM7_BASE;
extern "C"{ void TIM7_IRQHandler(void); }

extern TIM_TypeDef* TIM12_BASE;
extern "C"{ void TIM8_BRK_TIM12_IRQHandler(void); }
extern TIM_TypeDef* TIM13_BASE;
extern TIM_TypeDef* TIM14_BASE;
extern "C"{ void TIM8_TRG_COM_TIM14_IRQHandler(void); }

extern TIM_TypeDef* TIM23_BASE;
extern "C"{ void TIM23_IRQHandler(void); }
extern TIM_TypeDef* TIM24_BASE;
extern "C"{ void TIM24_IRQHandler(void); }
extern TIM_TypeDef* TIM1_BASE;
extern "C"{ void TIM1_UP_IRQHandler(void); }
extern TIM_TypeDef* TIM8_BASE;
extern "C"{ void TIM8_UP_TIM13_IRQHandler(void); }
extern TIM_TypeDef* TIM15_BASE;
extern "C"{ void TIM15_IRQHandler(void); }
extern TIM_TypeDef* TIM16_BASE;
extern "C"{ void TIM16_IRQHandler(void); }
extern TIM_TypeDef* TIM17_BASE;
extern "C"{ void TIM17_IRQHandler(void); }
