#include "HALAL/Models/DMA/DMA.hpp"

inline uint8_t DMA::inscribed_index{0};
inline std::set<uintptr_t> DMA::used_peripherials{};
inline std::set<uintptr_t> DMA::used_streams{};
inline std::array<DMA::DmaLinkEntry, MAX_STREAMS> DMA::inscribed_streams{};

void DMA::start() {
    for (uint8_t i = 0; i < inscribed_index; i++) {
        auto& [periph, dma, linker, irq] = inscribed_streams[i];
        
        // Inicializar DMA
        if (HAL_DMA_Init(dma) != HAL_OK) {
            ErrorHandler("DMA Init failed");
        }
        
        // Vincular DMA al periférico
        linker(dma);
        
        // Configurar interrupción
        HAL_NVIC_SetPriority(irq, 5, 0);
        HAL_NVIC_EnableIRQ(irq);
    }
}

bool DMA::is_stream_available(uintptr_t stream) {
    return used_streams.find(stream) == used_streams.end();
}

bool DMA::is_peripherial_available(uintptr_t peripheral) {
    return used_peripherials.find(peripheral) == used_peripherials.end();
}


IRQn_Type DMA::get_irqn(auto stream) {
    if  (stream == DMA1_Stream0) return DMA1_Stream0_IRQn;
    else if (stream == DMA1_Stream1) return DMA1_Stream1_IRQn;
    else if (stream == DMA1_Stream2) return DMA1_Stream2_IRQn;
    else if (stream == DMA1_Stream3) return DMA1_Stream3_IRQn;
    else if (stream == DMA1_Stream4) return DMA1_Stream4_IRQn;
    else if (stream == DMA1_Stream5) return DMA1_Stream5_IRQn;
    else if (stream == DMA1_Stream6) return DMA1_Stream6_IRQn;
    else if (stream == DMA1_Stream7) return DMA1_Stream7_IRQn;

    else if (stream == DMA2_Stream0) return DMA2_Stream0_IRQn;
    else if (stream == DMA2_Stream1) return DMA2_Stream1_IRQn;
    else if (stream == DMA2_Stream2) return DMA2_Stream2_IRQn;
    else if (stream == DMA2_Stream3) return DMA2_Stream3_IRQn;
    else if (stream == DMA2_Stream4) return DMA2_Stream4_IRQn;
    else if (stream == DMA2_Stream5) return DMA2_Stream5_IRQn;
    else if (stream == DMA2_Stream6) return DMA2_Stream6_IRQn;
    else if (stream == DMA2_Stream7) return DMA2_Stream7_IRQn;
    else ErrorHandler();
}

uint32_t DMA::get_Request(auto Instance, uint8_t i) {
    if (Instance == ADC1) return DMA_REQUEST_ADC1;
    if (Instance == ADC2) return DMA_REQUEST_ADC2;
    if (Instance == ADC3) return DMA_REQUEST_ADC3;

    if (Instance == I2C1 && i == 0) return DMA_REQUEST_I2C1_RX;
    if (Instance == I2C1 && i == 1) return DMA_REQUEST_I2C1_TX;
    if (Instance == I2C2 && i == 0) return DMA_REQUEST_I2C2_RX;
    if (Instance == I2C2 && i == 1) return DMA_REQUEST_I2C2_TX;
    if (Instance == I2C3 && i == 0) return DMA_REQUEST_I2C3_RX;
    if (Instance == I2C3 && i == 1) return DMA_REQUEST_I2C3_TX;
    if (Instance == I2C5 && i == 0) return DMA_REQUEST_I2C5_RX; 
    if (Instance == I2C5 && i == 1) return DMA_REQUEST_I2C5_TX;

    if (Instance == SPI1 && i == 0) return DMA_REQUEST_SPI1_RX;
    if (Instance == SPI1 && i == 1) return DMA_REQUEST_SPI1_TX;
    if (Instance == SPI2 && i == 0) return DMA_REQUEST_SPI2_RX;
    if (Instance == SPI2 && i == 1) return DMA_REQUEST_SPI2_TX;
    if (Instance == SPI3 && i == 0) return DMA_REQUEST_SPI3_RX;
    if (Instance == SPI3 && i == 1) return DMA_REQUEST_SPI3_TX;
    if (Instance == SPI4 && i == 0) return DMA_REQUEST_SPI4_RX;
    if (Instance == SPI4 && i == 1) return DMA_REQUEST_SPI4_TX;
    if (Instance == SPI5 && i == 0) return DMA_REQUEST_SPI5_RX;
    if (Instance == SPI5 && i == 1) return DMA_REQUEST_SPI5_TX; 
    
    if (Instance == FMAC && i == 0) return DMA_REQUEST_MEM2MEM;
    if (Instance == FMAC && i == 1) return DMA_REQUEST_FMAC_WRITE;
    if (Instance == FMAC && i == 2) return DMA_REQUEST_FMAC_READ;
    return 0;
}



uint32_t DMA::get_Direction(auto Instance, uint8_t i) {
    if (is_fmac(Instance) && i == 0){
            return DMA_MEMORY_TO_MEMORY;
        }
    else if  ((is_i2c(Instance) && i == 1) ||
        (is_spi(Instance) && i == 1) ||
        (is_fmac(Instance) && i == 1)){
            return DMA_MEMORY_TO_PERIPH;
        }

    return DMA_PERIPH_TO_MEMORY;
}

uint32_t DMA::get_PeriphInc(auto Instance, uint8_t i) {
    if  (is_fmac(Instance) && i == 0){
        return DMA_PINC_ENABLE;
    }
    return DMA_PINC_DISABLE;
}
uint32_t DMA::get_MemInc(auto Instance, uint8_t i) {
    if  (is_fmac(Instance) && i == 0){
        return DMA_MINC_DISABLE;
    }
    return DMA_MINC_ENABLE;
}

uint32_t DMA::get_PeriphDataAlignment(auto Instance, uint8_t i) {
    if  (is_i2c(Instance)){
        return DMA_PDATAALIGN_WORD; // Revisar esto, I2C suele trabajar con bytes
    }
    else if  (is_spi(Instance)){
        return DMA_PDATAALIGN_BYTE; 
    }

    return DMA_PDATAALIGN_HALFWORD;
}

uint32_t DMA::get_MemDataAlignment(auto Instance, uint8_t i) {
    if  (is_i2c(Instance)){
        return DMA_MDATAALIGN_WORD;
    }
    else if  (is_spi(Instance)){
        return DMA_MDATAALIGN_BYTE;
    }

    return DMA_MDATAALIGN_HALFWORD;
}
uint32_t DMA::get_Mode(auto Instance, uint8_t i) {
    if  (is_spi(Instance) || is_fmac(Instance)){
        return DMA_NORMAL;
    }
    
    return DMA_CIRCULAR;
}    

 uint32_t DMA::get_Priority(auto Instance, uint8_t i) {
    if  (is_fmac(Instance)){
        return DMA_PRIORITY_HIGH;
    }
    
    return DMA_PRIORITY_LOW;
}
uint32_t DMA::get_FIFOMode(auto Instance, uint8_t i) {
    if (is_fmac(Instance)){
        return DMA_FIFOMODE_ENABLE;
    }
    return DMA_FIFOMODE_DISABLE;
}

uint32_t DMA::get_FIFOThreshold(auto Instance, uint8_t i) {
    if  (is_spi(Instance)){
        return DMA_FIFO_THRESHOLD_FULL;
    }
    return DMA_FIFO_THRESHOLD_HALFFULL;
}

uint32_t DMA::get_MemBurst(auto Instance, uint8_t i) {
    return DMA_MBURST_SINGLE;
}

uint32_t DMA::get_PeriphBurst(auto Instance, uint8_t i) {
    return DMA_PBURST_SINGLE;
}