// #include "HALAL/Models/DMA/DMA.hpp"

// void DMA::start() 
// {
//     __HAL_RCC_DMA1_CLK_ENABLE();
//     __HAL_RCC_DMA2_CLK_ENABLE();

//     for (auto const &inst : instancias) {
//         std::visit([](auto const &cfg) {
//             for (size_t i = 0; i < cfg.handles.size(); ++i) {
//                 static DMA_HandleTypeDef dma_handle{};
//                 dma_handle.Instance = cfg.streams[i];
//                 dma_handle.Init = cfg.handles[i];

//                 if (HAL_DMA_Init(&dma_handle) != HAL_OK) {
//                     Error_Handler();
//                 }

//                 // Enlazar DMA con periférico
//                 __HAL_LINKDMA(cfg.instance, global_handle, dma_handle);

//                 HAL_NVIC_SetPriority(cfg.irqn[i], 0, 0);
//                 HAL_NVIC_EnableIRQ(cfg.irqn[i]);
//             }
//         }, inst);
//     }
// }
