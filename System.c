#define BOOT_ATTR __attribute__((used, section(".boot_code")))

#include "stm32h7xx.h"
#include <math.h>

#if !defined(HSE_VALUE)
#define HSE_VALUE ((uint32_t)25000000) /*!< Value of the External oscillator in Hz */
#endif                                 /* HSE_VALUE */

#if !defined(CSI_VALUE)
#define CSI_VALUE ((uint32_t)4000000) /*!< Value of the Internal oscillator in Hz*/
#endif                                /* CSI_VALUE */

#if !defined(HSI_VALUE)
#define HSI_VALUE ((uint32_t)64000000) /*!< Value of the Internal oscillator in Hz*/
#endif                                 /* HSI_VALUE */

uint32_t SystemCoreClock = 64000000;
uint32_t SystemD2Clock = 64000000;
const uint8_t D1CorePrescTable[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};

BOOT_ATTR void SystemInit(void) {
    __IO uint32_t tmpreg;

    SCB->CPACR |= ((3UL << (10 * 2)) | (3UL << (11 * 2))); /* set CP10 and CP11 Full Access */
    /* Reset the RCC clock configuration to the default reset state ------------*/

    /* Increasing the CPU frequency */
    if (FLASH_LATENCY_DEFAULT > (READ_BIT((FLASH->ACR), FLASH_ACR_LATENCY))) {
        /* Program the new number of wait states to the LATENCY bits in the FLASH_ACR register */
        MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, (uint32_t)(FLASH_LATENCY_DEFAULT));
    }

    /* Set HSION bit */
    RCC->CR |= RCC_CR_HSION;

    /* Reset CFGR register */
    RCC->CFGR = 0x00000000;

    /* Reset HSEON, HSECSSON, CSION, HSI48ON, CSIKERON, PLL1ON, PLL2ON and PLL3ON bits */
    RCC->CR &= 0xEAF6ED7FU;

    /* Decreasing the number of wait states because of lower CPU frequency */
    if (FLASH_LATENCY_DEFAULT < (READ_BIT((FLASH->ACR), FLASH_ACR_LATENCY))) {
        /* Program the new number of wait states to the LATENCY bits in the FLASH_ACR register */
        MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, (uint32_t)(FLASH_LATENCY_DEFAULT));
    }

    /* Reset D1CFGR register */
    RCC->D1CFGR = 0x00000000;

    /* Reset D2CFGR register */
    RCC->D2CFGR = 0x00000000;

    /* Reset D3CFGR register */
    RCC->D3CFGR = 0x00000000;

    /* Reset PLLCKSELR register */
    RCC->PLLCKSELR = 0x02020200;

    /* Reset PLLCFGR register */
    RCC->PLLCFGR = 0x01FF0000;
    /* Reset PLL1DIVR register */
    RCC->PLL1DIVR = 0x01010280;
    /* Reset PLL1FRACR register */
    RCC->PLL1FRACR = 0x00000000;

    /* Reset PLL2DIVR register */
    RCC->PLL2DIVR = 0x01010280;

    /* Reset PLL2FRACR register */

    RCC->PLL2FRACR = 0x00000000;
    /* Reset PLL3DIVR register */
    RCC->PLL3DIVR = 0x01010280;

    /* Reset PLL3FRACR register */
    RCC->PLL3FRACR = 0x00000000;

    /* Reset HSEBYP bit */
    RCC->CR &= 0xFFFBFFFFU;

    /* Disable all interrupts */
    RCC->CIER = 0x00000000;

    RCC->AHB2ENR |= (RCC_AHB2ENR_D2SRAM1EN | RCC_AHB2ENR_D2SRAM2EN);

    tmpreg = RCC->AHB2ENR;
    (void)tmpreg;

    /*
     * Disable the FMC bank1 (enabled after reset).
     * This, prevents CPU speculation access on this bank which blocks the use of FMC during
     * 24us. During this time the others FMC master (such as LTDC) cannot use it!
     */
    FMC_Bank1_R->BTCR[0] = 0x000030D2;
}


BOOT_ATTR void SystemCoreClockUpdate(void) {
    uint32_t pllp, pllsource, pllm, pllfracen, hsivalue, tmp;
    uint32_t common_system_clock;
    float_t fracn1, pllvco;

    /* Get SYSCLK source -------------------------------------------------------*/

    switch (RCC->CFGR & RCC_CFGR_SWS) {
    case RCC_CFGR_SWS_HSI: /* HSI used as system clock source */
        common_system_clock = (uint32_t)(HSI_VALUE >> ((RCC->CR & RCC_CR_HSIDIV) >> 3));
        break;

    case RCC_CFGR_SWS_CSI: /* CSI used as system clock  source */
        common_system_clock = CSI_VALUE;
        break;

    case RCC_CFGR_SWS_HSE: /* HSE used as system clock  source */
        common_system_clock = HSE_VALUE;
        break;

    case RCC_CFGR_SWS_PLL1: /* PLL1 used as system clock  source */

        /* PLL_VCO = (HSE_VALUE or HSI_VALUE or CSI_VALUE/ PLLM) * PLLN
        SYSCLK = PLL_VCO / PLLR
        */
        pllsource = (RCC->PLLCKSELR & RCC_PLLCKSELR_PLLSRC);
        pllm = ((RCC->PLLCKSELR & RCC_PLLCKSELR_DIVM1) >> 4);
        pllfracen = ((RCC->PLLCFGR & RCC_PLLCFGR_PLL1FRACEN) >> RCC_PLLCFGR_PLL1FRACEN_Pos);
        fracn1 = (float_t)(uint32_t)(pllfracen * ((RCC->PLL1FRACR & RCC_PLL1FRACR_FRACN1) >> 3));

        if (pllm != 0U) {
            switch (pllsource) {
            case RCC_PLLCKSELR_PLLSRC_HSI: /* HSI used as PLL clock source */

                hsivalue = (HSI_VALUE >> ((RCC->CR & RCC_CR_HSIDIV) >> 3));
                pllvco = ((float_t)hsivalue / (float_t)pllm) *
                         ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) +
                          (fracn1 / (float_t)0x2000) + (float_t)1);

                break;

            case RCC_PLLCKSELR_PLLSRC_CSI: /* CSI used as PLL clock source */
                pllvco = ((float_t)CSI_VALUE / (float_t)pllm) *
                         ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) +
                          (fracn1 / (float_t)0x2000) + (float_t)1);
                break;

            case RCC_PLLCKSELR_PLLSRC_HSE: /* HSE used as PLL clock source */
                pllvco = ((float_t)HSE_VALUE / (float_t)pllm) *
                         ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) +
                          (fracn1 / (float_t)0x2000) + (float_t)1);
                break;

            default:
                hsivalue = (HSI_VALUE >> ((RCC->CR & RCC_CR_HSIDIV) >> 3));
                pllvco = ((float_t)hsivalue / (float_t)pllm) *
                         ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) +
                          (fracn1 / (float_t)0x2000) + (float_t)1);
                break;
            }
            pllp = (((RCC->PLL1DIVR & RCC_PLL1DIVR_P1) >> 9) + 1U);
            common_system_clock = (uint32_t)(float_t)(pllvco / (float_t)pllp);
        } else {
            common_system_clock = 0U;
        }
        break;

    default:
        common_system_clock = (uint32_t)(HSI_VALUE >> ((RCC->CR & RCC_CR_HSIDIV) >> 3));
        break;
    }

    tmp = D1CorePrescTable[(RCC->D1CFGR & RCC_D1CFGR_D1CPRE) >> RCC_D1CFGR_D1CPRE_Pos];

    /* common_system_clock frequency : CM7 CPU frequency  */
    common_system_clock >>= tmp;

    /* SystemD2Clock frequency : CM4 CPU, AXI and AHBs Clock frequency  */
    SystemD2Clock =
        (common_system_clock >>
         ((D1CorePrescTable[(RCC->D1CFGR & RCC_D1CFGR_HPRE) >> RCC_D1CFGR_HPRE_Pos]) & 0x1FU));

    SystemCoreClock = common_system_clock;
}

__attribute__((weak)) BOOT_ATTR void ConfigurationChecker() {
    // Nothing for now
}

__attribute__((weak)) void BoardInit(void) {
    // do nothing by default
}
