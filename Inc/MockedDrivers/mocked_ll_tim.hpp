#pragma once
#include <cstdint>
#include "MockedDrivers/common.hpp"
#include "MockedDrivers/NVIC.hpp"
#include "MockedDrivers/tim_register_definitions.hpp"
#include "MockedDrivers/Register.hpp"
#include <iostream>
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


class TIM_TypeDef{
public:
    TIM_TypeDef(void(* irq_handler)(void),IRQn_Type irq_n):
    callback{irq_handler},irq_n{irq_n}
    {}
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
    TimerRegister<Reg_PSC> PSC;         /*!< TIM prescaler,                            Address offset: 0x28 */
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
            std::cout<<"TIMER IS NOT ENABLED!!\n";
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
        if (!main_counter_tick) {
            return false;
        }
        return true;
    }
};
static_assert(sizeof(TimerRegister<Reg_CNT>) == sizeof(uint32_t));
void simulate_ticks(TIM_TypeDef* tim);

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


#define DECLARE_TIMER(TIM_IDX) \
    extern TIM_TypeDef*  TIM_IDX##_BASE; \
    extern "C"{ \
    void  TIM_IDX##_IRQHandler(void); \
    } 
#define INSTANTIATE_TIMER(TIM_IDX) \
    TIM_TypeDef __htim##TIM_IDX{TIM_IDX##_IRQHandler,TIM_IDX##_IRQn}; \
    TIM_TypeDef*  TIM_IDX##_BASE = &__htim##TIM_IDX;

DECLARE_TIMER(TIM1)
DECLARE_TIMER(TIM2)
