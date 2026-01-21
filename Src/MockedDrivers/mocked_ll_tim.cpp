#include "MockedDrivers/mocked_ll_tim.hpp"

#include <iostream>

TIM_TypeDef __htim2{TIM2_IRQHandler, TIM2_IRQn};
TIM_TypeDef *TIM2_BASE = &__htim2;
TIM_TypeDef __htim3{TIM3_IRQHandler, TIM3_IRQn};
TIM_TypeDef *TIM3_BASE = &__htim3;
TIM_TypeDef __htim4{TIM4_IRQHandler, TIM4_IRQn};
TIM_TypeDef *TIM4_BASE = &__htim4;
TIM_TypeDef __htim5{TIM5_IRQHandler, TIM5_IRQn};
TIM_TypeDef *TIM5_BASE = &__htim5;
TIM_TypeDef __htim6{TIM6_DAC_IRQHandler, TIM6_DAC_IRQn};
TIM_TypeDef *TIM6_BASE = &__htim6;
TIM_TypeDef __htim7{TIM7_IRQHandler, TIM7_IRQn};
TIM_TypeDef *TIM7_BASE = &__htim7;
TIM_TypeDef __htim12{TIM8_BRK_TIM12_IRQHandler, TIM8_BRK_TIM12_IRQn};
TIM_TypeDef *TIM12_BASE = &__htim12;
TIM_TypeDef __htim13{TIM8_UP_TIM13_IRQHandler, TIM8_UP_TIM13_IRQn};
TIM_TypeDef *TIM13_BASE = &__htim13;
TIM_TypeDef __htim14{TIM8_TRG_COM_TIM14_IRQHandler, TIM8_TRG_COM_TIM14_IRQn};
TIM_TypeDef *TIM14_BASE = &__htim14;
TIM_TypeDef __htim23{TIM23_IRQHandler, TIM23_IRQn};
TIM_TypeDef *TIM23_BASE = &__htim23;
TIM_TypeDef __htim24{TIM24_IRQHandler, TIM24_IRQn};
TIM_TypeDef *TIM24_BASE = &__htim24;
TIM_TypeDef __htim1{TIM1_UP_IRQHandler, TIM1_UP_IRQn};
TIM_TypeDef *TIM1_BASE = &__htim1;
TIM_TypeDef __htim8{TIM8_UP_TIM13_IRQHandler, TIM8_UP_TIM13_IRQn};
TIM_TypeDef *TIM8_BASE = &__htim8;
TIM_TypeDef __htim15{TIM15_IRQHandler, TIM15_IRQn};
TIM_TypeDef *TIM15_BASE = &__htim15;
TIM_TypeDef __htim16{TIM16_IRQHandler, TIM16_IRQn};
TIM_TypeDef *TIM16_BASE = &__htim16;
TIM_TypeDef __htim17{TIM17_IRQHandler, TIM17_IRQn};
TIM_TypeDef *TIM17_BASE = &__htim17;

void TIM_TypeDef::generate_update() {
        active_PSC = PSC;
        active_ARR = ARR;
        active_RCR = RCR;
        internal_rcr_cnt = active_RCR;
        internal_psc_cnt = 0;
        *((uint32_t*)&CNT) = 0; // Usually UEV also resets CNT unless configured otherwise
        SR &= ~(1U << 0); // Clear UIF if needed, or set it depending on CR1
}
void simulate_ticks(TIM_TypeDef* tim){

    // Bit definitions for clarity
    const uint32_t CR1_UDIS = (1U << 1); // Update Disable
    const uint32_t CR1_ARPE = (1U << 7); // Auto-Reload Preload Enable
    const uint32_t SR_UIF   = (1U << 0); // Update Interrupt Flag

    // Determine the current Auto-Reload limit.
    // If ARPE is set, use the buffered (shadow) value.
    // If ARPE is clear, use the immediate register value.
    uint32_t current_limit = (tim->CR1 & CR1_ARPE) ? tim->active_ARR : tim->ARR.reg;

    // Check for Overflow
    if (tim->CNT > current_limit) {
        std::cout<<"timer overflow\n";
        tim->CNT = 0; // Rollover main counter

        // 4. Repetition Counter & Update Event Logic
        // The Update Event (UEV) is generated when the Repetition Counter underflows.
        if (tim->internal_rcr_cnt == 0) {
            
            // --- GENERATE UPDATE EVENT (UEV) ---

            // A. Update Shadow Registers from Preload Registers
            tim->active_PSC = tim->PSC;
            tim->active_ARR = tim->ARR;
            tim->active_RCR = tim->RCR;

            // B. Set Update Interrupt Flag (UIF)
            // Only if UDIS (Update Disable) is NOT set
            if (!(tim->CR1 & CR1_UDIS)) {
                tim->SR |= SR_UIF;
                if(NVIC_GetEnableIRQ(tim->irq_n)){
                    tim->callback();
                }
            }

            // C. Reload Repetition Counter with new value
            tim->internal_rcr_cnt = tim->active_RCR;

        } else {
            // No UEV yet, just decrement Repetition Counter
            tim->internal_rcr_cnt--;
        }
    }
}

void TIM_TypeDef::inc_cnt_and_check(uint32_t val) {
    if(val != 0 && this->check_CNT_increase_preconditions()){
        this->CNT += val;
        simulate_ticks(this);
    }
}