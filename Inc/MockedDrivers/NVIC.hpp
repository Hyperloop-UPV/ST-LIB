#pragma once

#include "stm32h723xx_wrapper.h"
#include "MockedDrivers/common.hpp"
#include "MockedDrivers/compiler_specific.hpp"

class NVIC_Type
{
    public:
    volatile uint32_t ISER[8U];               /*!< Offset: 0x000 (R/W)  Interrupt Set Enable Register */
             uint32_t RESERVED0[24U];
    volatile uint32_t ICER[8U];               /*!< Offset: 0x080 (R/W)  Interrupt Clear Enable Register */
             uint32_t RESERVED1[24U];
    volatile uint32_t ISPR[8U];               /*!< Offset: 0x100 (R/W)  Interrupt Set Pending Register */
             uint32_t RESERVED2[24U];
    volatile uint32_t ICPR[8U];               /*!< Offset: 0x180 (R/W)  Interrupt Clear Pending Register */
             uint32_t RESERVED3[24U];
    volatile uint32_t IABR[8U];               /*!< Offset: 0x200 (R/W)  Interrupt Active bit Register */
             uint32_t RESERVED4[56U];
    volatile uint8_t  IP[240U];               /*!< Offset: 0x300 (R/W)  Interrupt Priority Register (8Bit wide) */
             uint32_t RESERVED5[644U];
    volatile uint32_t STIR;                   /*!< Offset: 0xE00 ( /W)  Software Trigger Interrupt Register */
};

extern NVIC_Type* NVIC;

extern "C"{
void NVIC_EnableIRQ(IRQn_Type IRQn);

uint32_t NVIC_GetEnableIRQ(IRQn_Type IRQn);

void NVIC_DisableIRQ(IRQn_Type IRQn);

}