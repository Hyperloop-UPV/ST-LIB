/*
 * DMA.hpp
 *
 * Created on: 10 dic. 2022
 * Author: aleja
*/
#pragma once
#include "C++Utilities/CppUtils.hpp"
#include "stm32h7xx_hal.h"
#include "main.h"
#include "HALAL/Models/MPUManager/MPUManager.hpp"
#include <cassert>
#include <array>

#define MAX_STREAMS 16



template<uint8_t N>
struct Config {
    uint32_t instance;
    uint8_t num_streams {N};
    std::array<DMA_InitTypeDef, N> handles;
    std::array<DMA_Stream_TypeDef*, N> streams;
    std::array<IRQn_Type, N> irqn;
};

class DMA {
    public:
        template<auto Instance, uint32_t... Streams>
        static constexpr auto inscribe_stream();
        
        static void start();

        template<auto Instance>
        constexpr bool is_spi();
        
        template<auto Instance>
        constexpr bool is_i2c();

        template<auto Instance>
        constexpr bool is_adc();

        template<auto Instance>
        constexpr bool is_fmac();
        
        template<auto Instance, uint8_t i>
        static constexpr uint32_t get_Request();
        
        template<auto Instance, uint8_t i>
        static constexpr uint32_t get_Direction();
        
        template<auto Instance, uint8_t i>
        static constexpr uint32_t get_PeriphInc();
        
        template<auto Instance, uint8_t i>
        static constexpr uint32_t get_MemInc();
        
        template<auto Instance, uint8_t i>
        static constexpr uint32_t get_PeriphDataAlignment();
        
        template<auto Instance, uint8_t i>
        static constexpr uint32_t get_MemDataAlignment();
        
        template<auto Instance, uint8_t i>
        static constexpr uint32_t get_Mode();
        
        template<auto Instance, uint8_t i>
        static constexpr uint32_t get_Priority();
        
        template<auto Instance, uint8_t i>
        static constexpr uint32_t get_FIFOMode();
        
        template<auto Instance, uint8_t i>
        static constexpr uint32_t get_FIFOThreshold();
        
        template<auto Instance, uint8_t i>
        static constexpr uint32_t get_MemBurst();
        
        template<auto Instance, uint8_t i>
        static constexpr uint32_t get_PeriphBurst();
        
        template<auto stream>
        static constexpr IRQn_Type get_irqn();
};


template<auto Instance, uint32_t... Streams>
constexpr auto DMA::inscribe_stream(){
    constexpr std::size_t N = sizeof...(Streams);
    static_assert(N <= MAX_STREAMS, "Too many streams inscribed");
    
    Config<N> handleConfig; 
    handleConfig.instance = Instance;
    constexpr uint32_t stream_vals[N] = {Streams...};
    handleConfig.streams = std::array<DMA_Stream_TypeDef*, N>{reinterpret_cast<DMA_Stream_TypeDef*>(Streams)...};

    if constexpr (is_adc<Instance>()) {
        static_assert(N == 1, "ADC DMA must have exactly one stream");
    }
    else if constexpr (is_fmac<Instance>()) {
        static_assert(N == 3, "FMAC DMA must have exactly three streams");
    }
    else {
        static_assert(N == 2, "Peripheral DMA must have exactly two streams (RX and TX)");
    }
    
    // TODO: verificar que los streams no esten siendo usados   
   
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        (([&] {
            handleConfig.handles[I].Request = get_Request<Instance, I>();
            handleConfig.handles[I].Direction = get_Direction<Instance, I>();
            handleConfig.handles[I].PeriphInc = get_PeriphInc<Instance, I>();
            handleConfig.handles[I].MemInc = get_MemInc<Instance, I>();
            handleConfig.handles[I].PeriphDataAlignment = get_PeriphDataAlignment<Instance, I>();
            handleConfig.handles[I].MemDataAlignment = get_MemDataAlignment<Instance, I>();
            handleConfig.handles[I].Mode = get_Mode<Instance, I>();
            handleConfig.handles[I].Priority = get_Priority<Instance, I>();
            handleConfig.handles[I].FIFOMode = get_FIFOMode<Instance, I>();
            handleConfig.handles[I].FIFOThreshold = get_FIFOThreshold<Instance, I>();
            handleConfig.handles[I].MemBurst = get_MemBurst<Instance, I>();
            handleConfig.handles[I].PeriphBurst = get_PeriphBurst<Instance, I>();

            handleConfig[I] = get_irqn<stream_vals[I]>();

        }()), ...);
    }(std::make_index_sequence<N>{});
    
    return handleConfig;
}

template<auto Instance>
constexpr bool DMA::is_spi() {
    return Instance == SPI1_BASE || Instance == SPI2_BASE || Instance == SPI3_BASE ||
    Instance == SPI4_BASE || Instance == SPI5_BASE;
}

template<auto Instance>
constexpr bool DMA::is_i2c() {
    return Instance == I2C1_BASE || Instance == I2C2_BASE || Instance == I2C3_BASE || Instance == I2C5_BASE;
}

template<auto Instance>
constexpr bool DMA::is_adc() {
    return Instance == ADC1_BASE || Instance == ADC2_BASE || Instance == ADC3_BASE;
}

template<auto Instance>
constexpr bool DMA::is_fmac() {
    return Instance == FMAC_BASE;
} 

template<auto stream>
constexpr IRQn_Type DMA::get_irqn() {
    if (stream == DMA1_Stream0_BASE) return DMA1_Stream0_IRQn;
    if (stream == DMA1_Stream1_BASE) return DMA1_Stream1_IRQn;
    if (stream == DMA1_Stream2_BASE) return DMA1_Stream2_IRQn;
    if (stream == DMA1_Stream3_BASE) return DMA1_Stream3_IRQn;
    if (stream == DMA1_Stream4_BASE) return DMA1_Stream4_IRQn;
    if (stream == DMA1_Stream5_BASE) return DMA1_Stream5_IRQn;
    if (stream == DMA1_Stream6_BASE) return DMA1_Stream6_IRQn;
    if (stream == DMA1_Stream7_BASE) return DMA1_Stream7_IRQn;

    if (stream == DMA2_Stream0_BASE) return DMA2_Stream0_IRQn;
    if (stream == DMA2_Stream1_BASE) return DMA2_Stream1_IRQn;
    if (stream == DMA2_Stream2_BASE) return DMA2_Stream2_IRQn;
    if (stream == DMA2_Stream3_BASE) return DMA2_Stream3_IRQn;
    if (stream == DMA2_Stream4_BASE) return DMA2_Stream4_IRQn;
    if (stream == DMA2_Stream5_BASE) return DMA2_Stream5_IRQn;
    if (stream == DMA2_Stream6_BASE) return DMA2_Stream6_IRQn;
    return DMA2_Stream7_IRQn;
}


template<auto Instance, uint8_t i>
constexpr uint32_t DMA::get_Request() {
    if constexpr (Instance == ADC1_BASE) return DMA_REQUEST_ADC1;
    if constexpr (Instance == ADC2_BASE) return DMA_REQUEST_ADC2;
    if constexpr (Instance == ADC3_BASE) return DMA_REQUEST_ADC3;

    if constexpr (Instance == I2C1_BASE && i == 0) return DMA_REQUEST_I2C1_RX;
    if constexpr (Instance == I2C1_BASE && i == 1) return DMA_REQUEST_I2C1_TX;
    if constexpr (Instance == I2C2_BASE && i == 0) return DMA_REQUEST_I2C2_RX;
    if constexpr (Instance == I2C2_BASE && i == 1) return DMA_REQUEST_I2C2_TX;
    if constexpr (Instance == I2C3_BASE && i == 0) return DMA_REQUEST_I2C3_RX;
    if constexpr (Instance == I2C3_BASE && i == 1) return DMA_REQUEST_I2C3_TX;
    if constexpr (Instance == I2C5_BASE && i == 0) return DMA_REQUEST_I2C5_RX; 
    if constexpr (Instance == I2C5_BASE && i == 1) return DMA_REQUEST_I2C5_TX;

    if constexpr (Instance == SPI1_BASE && i == 0) return DMA_REQUEST_SPI1_RX;
    if constexpr (Instance == SPI1_BASE && i == 1) return DMA_REQUEST_SPI1_TX;
    if constexpr (Instance == SPI2_BASE && i == 0) return DMA_REQUEST_SPI2_RX;
    if constexpr (Instance == SPI2_BASE && i == 1) return DMA_REQUEST_SPI2_TX;
    if constexpr (Instance == SPI3_BASE && i == 0) return DMA_REQUEST_SPI3_RX;
    if constexpr (Instance == SPI3_BASE && i == 1) return DMA_REQUEST_SPI3_TX;
    if constexpr (Instance == SPI4_BASE && i == 0) return DMA_REQUEST_SPI4_RX;
    if constexpr (Instance == SPI4_BASE && i == 1) return DMA_REQUEST_SPI4_TX;
    if constexpr (Instance == SPI5_BASE && i == 0) return DMA_REQUEST_SPI5_RX;
    if constexpr (Instance == SPI5_BASE && i == 1) return DMA_REQUEST_SPI5_TX; 
    
    if constexpr (Instance == FMAC_BASE && i == 0) return DMA_REQUEST_MEM2MEM;
    if constexpr (Instance == FMAC_BASE && i == 1) return DMA_REQUEST_FMAC_WRITE;
    if constexpr (Instance == FMAC_BASE && i == 2) return DMA_REQUEST_FMAC_READ;
    return 0;
}

template<auto Instance, uint8_t i>
constexpr uint32_t DMA::get_Direction() {
    if constexpr (is_fmac<Instance>() && i == 0){
            return DMA_MEMORY_TO_MEMORY;
        }
    else if constexpr ((is_i2c<Instance>() && i == 1) ||
        (is_spi<Instance>() && i == 1) ||
        (is_fmac<Instance>() && i == 1)){
            return DMA_MEMORY_TO_PERIPH;
        }

    return DMA_PERIPH_TO_MEMORY;
        
}

template<auto Instance, uint8_t i>
constexpr uint32_t DMA::get_PeriphInc() {
    if constexpr (is_fmac<Instance>() && i == 0){
        return DMA_PINC_ENABLE;
    }
    return DMA_PINC_DISABLE;
}

template<auto Instance, uint8_t i>
constexpr uint32_t DMA::get_MemInc() {
    if constexpr (is_fmac<Instance>() && i == 0){
        return DMA_MINC_DISABLE;
    }
    return DMA_MINC_ENABLE;
}

template<auto Instance, uint8_t i>
constexpr uint32_t DMA::get_PeriphDataAlignment() {
    if constexpr (is_spi<Instance>()){
        return DMA_PDATAALIGN_WORD;
    }
    else if constexpr (is_spi<Instance>()){
        return DMA_PDATAALIGN_BYTE;
    }

    return DMA_PDATAALIGN_HALFWORD;
}

template<auto Instance, uint8_t i>
constexpr uint32_t DMA::get_MemDataAlignment() {
    if constexpr (is_spi<Instance>()){
        return DMA_MDATAALIGN_WORD;
    }
    else if constexpr (is_spi<Instance>()){
        return DMA_MDATAALIGN_BYTE;
    }

    return DMA_MDATAALIGN_HALFWORD;
}

template<auto Instance, uint8_t i>
constexpr uint32_t DMA::get_Mode() {
    if constexpr (is_spi<Instance>() || is_fmac<Instance>()){
        return DMA_NORMAL;
    }
    
    return DMA_CIRCULAR;
    
}

template<auto Instance, uint8_t i>
constexpr uint32_t DMA::get_Priority() {
    if constexpr (is_fmac<Instance>()){
        return DMA_PRIORITY_HIGH;
    }
    
    return DMA_PRIORITY_LOW;
}

template<auto Instance, uint8_t i>
constexpr uint32_t DMA::get_FIFOMode() {
    if constexpr (is_fmac<Instance>()){
            return DMA_FIFOMODE_ENABLE;
    }
    
    return DMA_FIFOMODE_DISABLE;
}

template<auto Instance, uint8_t i>
constexpr uint32_t DMA::get_FIFOThreshold() {
    if constexpr (is_spi<Instance>()){
        return DMA_FIFO_THRESHOLD_FULL;
    }
    return DMA_FIFO_THRESHOLD_HALFFULL;
}

template<auto Instance, uint8_t i>
constexpr uint32_t DMA::get_MemBurst() {
    return DMA_MBURST_SINGLE;
}

template<auto Instance, uint8_t i>
constexpr uint32_t DMA::get_PeriphBurst() {
    return DMA_PBURST_SINGLE;
}
