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
void TIM_TypeDef::simulate_ticking() {
    // Bit definitions for clarity
    const uint32_t CR1_CEN  = (1U << 0); // Counter Enable
    const uint32_t CR1_UDIS = (1U << 1); // Update Disable
    const uint32_t CR1_ARPE = (1U << 7); // Auto-Reload Preload Enable
    const uint32_t SR_UIF   = (1U << 0); // Update Interrupt Flag

    // 1. Check if Counter is Enabled
    if (!(CR1 & CR1_CEN)) {
        std::cout<<"TIMER IS NOT ENABLED!!\n";
        return; 
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
        return;
    }

    // 3. Main Counter Logic
    CNT +=1;

    // Determine the current Auto-Reload limit.
    // If ARPE is set, use the buffered (shadow) value.
    // If ARPE is clear, use the immediate register value.
    uint32_t current_limit = (CR1 & CR1_ARPE) ? active_ARR : ARR;

    // Check for Overflow
    if (CNT > current_limit) {
        std::cout<<"timer overflow\n";
        CNT = 0; // Rollover main counter

        // 4. Repetition Counter & Update Event Logic
        // The Update Event (UEV) is generated when the Repetition Counter underflows.
        if (internal_rcr_cnt == 0) {
            
            // --- GENERATE UPDATE EVENT (UEV) ---

            // A. Update Shadow Registers from Preload Registers
            active_PSC = PSC;
            active_ARR = ARR;
            active_RCR = RCR;

            // B. Set Update Interrupt Flag (UIF)
            // Only if UDIS (Update Disable) is NOT set
            if (!(CR1 & CR1_UDIS)) {
                SR |= SR_UIF;
                callback();
            }

            // C. Reload Repetition Counter with new value
            internal_rcr_cnt = active_RCR;

        } else {
            // No UEV yet, just decrement Repetition Counter
            internal_rcr_cnt--;
        }
    }
}