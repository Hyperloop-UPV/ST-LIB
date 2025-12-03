#include "MockedDrivers/mocked_ll_tim.hpp"

#include <iostream>

INSTANTIATE_TIMER(TIM2)

void TIM_TypeDef::generate_update() {
        active_PSC = PSC;
        active_ARR = ARR;
        active_RCR = RCR;
        internal_rcr_cnt = active_RCR;
        internal_psc_cnt = 0;
        CNT = 0; // Usually UEV also resets CNT unless configured otherwise
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