// #include "HALAL/Models/DMA/DMA.hpp"

// void DMA::start() {
//     __HAL_RCC_DMA1_CLK_ENABLE();
//     __HAL_RCC_DMA2_CLK_ENABLE();
//     for (uint8_t i = 0; i < inscribed_index; i++) {
//         auto& [periph, dma, linker, irq] = inscribed_streams[i];
        
//         if (HAL_DMA_Init(dma) != HAL_OK) {
//             ErrorHandler("DMA Init failed");
//         }
        
//         linker(dma);
        
//         HAL_NVIC_SetPriority(irq, 0, 0);
//         HAL_NVIC_EnableIRQ(irq);
//     }
// }