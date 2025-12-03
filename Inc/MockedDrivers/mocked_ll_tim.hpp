#pragma once
#include <cstdint>
#include "MockedDrivers/common.hpp"
#include "MockedDrivers/tim_register_definitions.hpp"
class TIM_TypeDef{
public:
    TIM_TypeDef(void(* irq_handler)(void)):
    callback{irq_handler}
    {}
    void simulate_ticking();
    void generate_update();
    volatile uint32_t CR1;         /*!< TIM control register 1,                   Address offset: 0x00 */
    volatile uint32_t CR2;         /*!< TIM control register 2,                   Address offset: 0x04 */
    volatile uint32_t SMCR;        /*!< TIM slave mode control register,          Address offset: 0x08 */
    volatile uint32_t DIER;        /*!< TIM DMA/interrupt enable register,        Address offset: 0x0C */
    volatile uint32_t SR;          /*!< TIM status register,                      Address offset: 0x10 */
    volatile uint32_t EGR;         /*!< TIM event generation register,            Address offset: 0x14 */
    volatile uint32_t CCMR1;       /*!< TIM capture/compare mode register 1,      Address offset: 0x18 */
    volatile uint32_t CCMR2;       /*!< TIM capture/compare mode register 2,      Address offset: 0x1C */
    volatile uint32_t CCER;        /*!< TIM capture/compare enable register,      Address offset: 0x20 */
    volatile uint32_t CNT;         /*!< TIM counter register,                     Address offset: 0x24 */
    volatile uint32_t PSC;         /*!< TIM prescaler,                            Address offset: 0x28 */
    volatile uint32_t ARR;         /*!< TIM auto-reload register,                 Address offset: 0x2C */
    volatile uint32_t RCR;         /*!< TIM repetition counter register,          Address offset: 0x30 */
    volatile uint32_t CCR1;        /*!< TIM capture/compare register 1,           Address offset: 0x34 */
    volatile uint32_t CCR2;        /*!< TIM capture/compare register 2,           Address offset: 0x38 */
    volatile uint32_t CCR3;        /*!< TIM capture/compare register 3,           Address offset: 0x3C */
    volatile uint32_t CCR4;        /*!< TIM capture/compare register 4,           Address offset: 0x40 */
    volatile uint32_t BDTR;        /*!< TIM break and dead-time register,         Address offset: 0x44 */
    volatile uint32_t DCR;         /*!< TIM DMA control register,                 Address offset: 0x48 */
    volatile uint32_t DMAR;        /*!< TIM DMA address for full transfer,        Address offset: 0x4C */
    uint32_t      RESERVED1;   /*!< Reserved, 0x50                                                 */
    volatile uint32_t CCMR3;       /*!< TIM capture/compare mode register 3,      Address offset: 0x54 */
    volatile uint32_t CCR5;        /*!< TIM capture/compare register5,            Address offset: 0x58 */
    volatile uint32_t CCR6;        /*!< TIM capture/compare register6,            Address offset: 0x5C */
    volatile uint32_t AF1;         /*!< TIM alternate function option register 1, Address offset: 0x60 */
    volatile uint32_t AF2;         /*!< TIM alternate function option register 2, Address offset: 0x64 */
    volatile uint32_t TISEL;       /*!< TIM Input Selection register,             Address offset: 0x68 */
private:
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
};

#define DECLARE_TIMER(TIM_IDX) \
    extern TIM_TypeDef*  TIM_IDX##_BASE; \
    extern "C"{ \
    void  TIM_IDX##_IRQHandler(void); \
    } 
#define INSTANTIATE_TIMER(TIM_IDX) \
    TIM_TypeDef __htim##TIM_IDX{TIM_IDX##_IRQHandler}; \
    TIM_TypeDef*  TIM_IDX##_BASE = &__htim##TIM_IDX;

DECLARE_TIMER(TIM1)
DECLARE_TIMER(TIM2)
