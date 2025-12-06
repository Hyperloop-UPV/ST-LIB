#pragma once
#include <cstdint>
#include "MockedDrivers/common.hpp"
#include "MockedDrivers/NVIC.hpp"
#include "MockedDrivers/tim_register_definitions.hpp"
#include "MockedDrivers/Register.hpp"
#include <iostream>
enum class TimRegs {
    CR1, CR2, SMCR, DIER, SR, EGR, CCMR1, CCMR2, CCER, CNT, PSC, ARR, RCR,
    CCR1, CCR2, CCR3, CCR4, BDTR, DCR, DMAR, CCMR3, CCR5, CCR6, AF1, AF2, TISEL
};



template<TimRegs Reg>
class TimerRegister : public RegisterBase<TimRegs, Reg> {
public:
    using RegisterBase<TimRegs, Reg>::RegisterBase;
    using RegisterBase<TimRegs, Reg>::operator=;
};

static_assert(sizeof(TimerRegister<TimRegs::CR1>) == sizeof(uint32_t) );


class TIM_TypeDef{
public:
    TIM_TypeDef(void(* irq_handler)(void),IRQn_Type irq_n):
    callback{irq_handler},irq_n{irq_n}
    {}
    void generate_update();
    TimerRegister<TimRegs::CR1> CR1;         /*!< TIM control register 1,                   Address offset: 0x00 */
    TimerRegister<TimRegs::CR2> CR2;         /*!< TIM control register 2,                   Address offset: 0x04 */
    TimerRegister<TimRegs::SMCR> SMCR;        /*!< TIM slave mode control register,          Address offset: 0x08 */
    TimerRegister<TimRegs::DIER> DIER;        /*!< TIM DMA/interrupt enable register,        Address offset: 0x0C */
    TimerRegister<TimRegs::SR> SR;          /*!< TIM status register,                      Address offset: 0x10 */
    TimerRegister<TimRegs::EGR> EGR;         /*!< TIM event generation register,            Address offset: 0x14 */
    TimerRegister<TimRegs::CCMR1> CCMR1;       /*!< TIM capture/compare mode register 1,      Address offset: 0x18 */
    TimerRegister<TimRegs::CCMR2> CCMR2;       /*!< TIM capture/compare mode register 2,      Address offset: 0x1C */
    TimerRegister<TimRegs::CCER> CCER;        /*!< TIM capture/compare enable register,      Address offset: 0x20 */
    TimerRegister<TimRegs::CNT> CNT;         /*!< TIM counter register,                     Address offset: 0x24 */
    TimerRegister<TimRegs::PSC> PSC;         /*!< TIM prescaler,                            Address offset: 0x28 */
    TimerRegister<TimRegs::ARR> ARR;         /*!< TIM auto-reload register,                 Address offset: 0x2C */
    TimerRegister<TimRegs::RCR> RCR;         /*!< TIM repetition counter register,          Address offset: 0x30 */
    TimerRegister<TimRegs::CCR1> CCR1;        /*!< TIM capture/compare register 1,           Address offset: 0x34 */
    TimerRegister<TimRegs::CCR2> CCR2;        /*!< TIM capture/compare register 2,           Address offset: 0x38 */
    TimerRegister<TimRegs::CCR3> CCR3;        /*!< TIM capture/compare register 3,           Address offset: 0x3C */
    TimerRegister<TimRegs::CCR4> CCR4;        /*!< TIM capture/compare register 4,           Address offset: 0x40 */
    TimerRegister<TimRegs::BDTR> BDTR;        /*!< TIM break and dead-time register,         Address offset: 0x44 */
    TimerRegister<TimRegs::DCR> DCR;         /*!< TIM DMA control register,                 Address offset: 0x48 */
    TimerRegister<TimRegs::DMAR> DMAR;        /*!< TIM DMA address for full transfer,        Address offset: 0x4C */
    uint32_t      RESERVED1;   /*!< Reserved, 0x50                                                 */
    TimerRegister<TimRegs::CCMR3> CCMR3;       /*!< TIM capture/compare mode register 3,      Address offset: 0x54 */
    TimerRegister<TimRegs::CCR5> CCR5;        /*!< TIM capture/compare register5,            Address offset: 0x58 */
    TimerRegister<TimRegs::CCR6> CCR6;        /*!< TIM capture/compare register6,            Address offset: 0x5C */
    TimerRegister<TimRegs::AF1> AF1;         /*!< TIM alternate function option register 1, Address offset: 0x60 */
    TimerRegister<TimRegs::AF2> AF2;         /*!< TIM alternate function option register 2, Address offset: 0x64 */
    TimerRegister<TimRegs::TISEL> TISEL;       /*!< TIM Input Selection register,             Address offset: 0x68 */
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
static_assert(sizeof(TimerRegister<TimRegs::CNT>) == sizeof(uint32_t));
void simulate_ticks(TIM_TypeDef* tim);

template<>
struct RegisterTraits<TimRegs,TimRegs::CNT> {
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
