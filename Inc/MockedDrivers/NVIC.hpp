#pragma once

#include "stm32h723xx_wrapper.h"
#include "MockedDrivers/common.hpp"
#include "MockedDrivers/compiler_specific.hpp"

#include "MockedDrivers/Register.hpp"

enum class NVICReg {
    Reg_ISER,
    Reg_ICER,
    Reg_ISPR,
    Reg_ICPR,
    Reg_IABR,
    Reg_IP,
};

template <NVICReg Reg> class NVICRegister : public RegisterBase<NVICReg, Reg> {
public:
    using RegisterBase<NVICReg, Reg>::RegisterBase;
    using RegisterBase<NVICReg, Reg>::operator=;
};

class NVIC_Type {
    struct ICER_Register : public RegisterBase<NVICReg, NVICReg::Reg_ICER> {
        ICER_Register& operator=(uint32_t val) {
            volatile uint32_t* ISER_offset =
                (volatile uint32_t*)((volatile uint8_t*)&this->reg - offsetof(NVIC_Type, ICER));
            *ISER_offset = *ISER_offset & ~val;
            return *this;
        }
    };

public:
    volatile uint32_t ISER[8U]; /*!< Offset: 0x000 (R/W)  Interrupt Set Enable Register */
    uint32_t RESERVED0[24U];
    ICER_Register ICER[8U]; /*!< Offset: 0x080 (R/W)  Interrupt Clear Enable Register */
    uint32_t RESERVED1[24U];
    volatile uint32_t ISPR[8U]; /*!< Offset: 0x100 (R/W)  Interrupt Set Pending Register */
    uint32_t RESERVED2[24U];
    volatile uint32_t ICPR[8U]; /*!< Offset: 0x180 (R/W)  Interrupt Clear Pending Register */
    uint32_t RESERVED3[24U];
    volatile uint32_t IABR[8U]; /*!< Offset: 0x200 (R/W)  Interrupt Active bit Register */
    uint32_t RESERVED4[56U];
    volatile uint8_t IP[240U]; /*!< Offset: 0x300 (R/W)  Interrupt Priority Register (8Bit wide) */
    uint32_t RESERVED5[644U];
    volatile uint32_t STIR; /*!< Offset: 0xE00 ( /W)  Software Trigger Interrupt Register */
};

extern NVIC_Type* NVIC;

extern "C" {
void NVIC_EnableIRQ(IRQn_Type IRQn);

uint32_t NVIC_GetEnableIRQ(IRQn_Type IRQn);

void NVIC_DisableIRQ(IRQn_Type IRQn);
}
