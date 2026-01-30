#include "HALAL/Models/DMA/DMA2.hpp"


extern "C" void DMA1_Stream0_IRQHandler(void) {
    HAL_DMA_IRQHandler(dma_irq_table[0]);
}

extern "C" void DMA1_Stream1_IRQHandler(void) { 
    HAL_DMA_IRQHandler(dma_irq_table[1]);
}

extern "C" void DMA1_Stream2_IRQHandler(void) { 
    HAL_DMA_IRQHandler(dma_irq_table[2]);
}

extern "C" void DMA1_Stream3_IRQHandler(void) { 
    HAL_DMA_IRQHandler(dma_irq_table[3]);
}

extern "C" void DMA1_Stream4_IRQHandler(void) { 
    HAL_DMA_IRQHandler(dma_irq_table[4]);
}

extern "C" void DMA1_Stream5_IRQHandler(void) { 
    HAL_DMA_IRQHandler(dma_irq_table[5]);
}

extern "C" void DMA1_Stream6_IRQHandler(void) { 
    HAL_DMA_IRQHandler(dma_irq_table[6]);
}

extern "C" void DMA1_Stream7_IRQHandler(void) { 
    HAL_DMA_IRQHandler(dma_irq_table[7]);
}

extern "C" void DMA2_Stream0_IRQHandler(void) { 
    HAL_DMA_IRQHandler(dma_irq_table[8]);
}

extern "C" void DMA2_Stream1_IRQHandler(void) { 
    HAL_DMA_IRQHandler(dma_irq_table[9]);
}

extern "C" void DMA2_Stream2_IRQHandler(void) { 
    HAL_DMA_IRQHandler(dma_irq_table[10]);
}

extern "C" void DMA2_Stream3_IRQHandler(void) { 
    HAL_DMA_IRQHandler(dma_irq_table[11]);
}

extern "C" void DMA2_Stream4_IRQHandler(void) { 
    HAL_DMA_IRQHandler(dma_irq_table[12]);
}

extern "C" void DMA2_Stream5_IRQHandler(void) { 
    HAL_DMA_IRQHandler(dma_irq_table[13]);
}

extern "C" void DMA2_Stream6_IRQHandler(void) { 
    HAL_DMA_IRQHandler(dma_irq_table[14]);
}

extern "C" void DMA2_Stream7_IRQHandler(void) { 
    HAL_DMA_IRQHandler(dma_irq_table[15]);
}
