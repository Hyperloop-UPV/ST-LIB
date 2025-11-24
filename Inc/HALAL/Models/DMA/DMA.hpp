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
    
class DMA {
    public:
        static constexpr bool is_one_of(auto Instance, auto... Bases);

        static constexpr bool is_spi(auto Instance);
        
        static constexpr bool is_i2c(auto Instance);

        static constexpr bool is_adc(auto Instance);

        static constexpr bool is_fmac(auto Instance);

        template<auto Instance, uint32_t... Streams>
        requires (
            (DMA::is_adc(Instance)  && sizeof...(Streams) == 1) ||
            (DMA::is_fmac(Instance) && sizeof...(Streams) == 3) ||
            (!DMA::is_adc(Instance) && !DMA::is_fmac(Instance) && sizeof...(Streams) == 2)
        )
        static void inscribe_stream(auto handle);

        static void start();        
    
    private:
        using PeriphVariant = std::variant<
            SPI_HandleTypeDef*,
            I2C_HandleTypeDef*,
            ADC_HandleTypeDef*,
            FMAC_HandleTypeDef*
        >;

        template<typename PeriphHandle>
        using DmaLinkEntry = std::tuple<
            PeriphHandle*, // (__HANDLE__) 
            DMA_HandleTypeDef*, // (__DMA_HANDLE__) 
            DMA_HandleTypeDef* PeriphHandle::*,   // (__PPP_DMA_FIELD__)
            IRQn_Type   
        >;

        template<typename PeriphHandle>
        static DmaLinkEntry<PeriphHandle> make_dma_entry(
            PeriphHandle* periph,
            DMA_HandleTypeDef* dma,
            DMA_HandleTypeDef* PeriphHandle::* member,
            IRQn_Type irqn
        );

        static uint32_t get_Request(auto Instance, uint8_t i);
        
        static uint32_t get_Direction(auto Instance, uint8_t i);
        
        static uint32_t get_PeriphInc(auto Instance, uint8_t i);
        
        static uint32_t get_MemInc(auto Instance, uint8_t i);
        
        static uint32_t get_PeriphDataAlignment(auto Instance, uint8_t i);
        
        static uint32_t get_MemDataAlignment(auto Instance, uint8_t i);
        
        static uint32_t get_Mode(auto Instance, uint8_t i);
        
        static uint32_t get_Priority(auto Instance, uint8_t i);
        
        static uint32_t get_FIFOMode(auto Instance, uint8_t i);
        
        static uint32_t get_FIFOThreshold(auto Instance, uint8_t i);
        
        static uint32_t get_MemBurst(auto Instance, uint8_t i);
        
        static uint32_t get_PeriphBurst(auto Instance, uint8_t i);
        
        static IRQn_Type get_irqn(auto stream);

        static bool is_stream_available(uintptr_t stream);

        static bool is_peripherial_available(uintptr_t peripherial);
        
        static std::array<DmaLinkEntry<PeriphVariant>, MAX_STREAMS> inscribed_streams;
        static uint8_t inscribed_index;

        static std::set<uintptr_t> used_peripherials;
        static std::set<uintptr_t> used_streams;
};

inline uint8_t DMA::inscribed_index{0};
inline std::set<uintptr_t> DMA::used_peripherials{};
inline std::set<uintptr_t> DMA::used_streams{};
inline std::array<DMA::DmaLinkEntry<DMA::PeriphVariant>, MAX_STREAMS> DMA::inscribed_streams{};

template<auto Instance, uint32_t... Streams>
requires (
    (DMA::is_adc(Instance)  && sizeof...(Streams) == 1) ||
    (DMA::is_fmac(Instance) && sizeof...(Streams) == 3) ||
    (!DMA::is_adc(Instance) && !DMA::is_fmac(Instance) && sizeof...(Streams) == 2)
)
void DMA::inscribe_stream(auto handle) {
    const std::size_t N = sizeof...(Streams);
    if (inscribed_index + N > MAX_STREAMS){
        ErrorHandler("Too many streams inscribed");
    }
    
    uintptr_t periph_addr = reinterpret_cast<uintptr_t>(Instance);
    if (!is_peripherial_available(periph_addr)){
        ErrorHandler("Peripheral already in use");
    }
    
    std::array<DMA_Stream_TypeDef*, N> streams = {(DMA_Stream_TypeDef*)Streams... };

    for (uint8_t i = 0; i < N; i++){
        uintptr_t stream_addr = reinterpret_cast<uintptr_t>(streams[i]);
        if (!is_stream_available(stream_addr)){
            ErrorHandler("DMA stream already in use");
        }
    }

    for (uint8_t i = 0; i < N; i++){
        used_streams.insert(reinterpret_cast<uintptr_t>(streams[i]));
    }
    used_peripherials.insert(periph_addr);

    for (uint8_t i = 0; i < N; i++){
        DMA_HandleTypeDef* dma = new DMA_HandleTypeDef{};
        dma->Instance = streams[i];
        dma->Init.Request             = get_Request(Instance, i);
        dma->Init.Direction           = get_Direction(Instance, i);
        dma->Init.PeriphInc           = get_PeriphInc(Instance, i);
        dma->Init.MemInc              = get_MemInc(Instance, i);
        dma->Init.PeriphDataAlignment = get_PeriphDataAlignment(Instance, i);
        dma->Init.MemDataAlignment    = get_MemDataAlignment(Instance, i);
        dma->Init.Mode                = get_Mode(Instance, i);
        dma->Init.Priority            = get_Priority(Instance, i);
        dma->Init.FIFOMode            = get_FIFOMode(Instance, i);
        dma->Init.FIFOThreshold       = get_FIFOThreshold(Instance, i);
        dma->Init.MemBurst            = get_MemBurst(Instance, i);
        dma->Init.PeriphBurst         = get_PeriphBurst(Instance, i);
        IRQn_Type irq = get_irqn(streams[i]);

        if constexpr (is_spi(Instance)) {
            auto member = (i == 0) ? &SPI_HandleTypeDef::hdmarx : &SPI_HandleTypeDef::hdmatx;
            inscribed_streams[inscribed_index] = make_dma_entry(handle, dma, member, irq);
        }
        else if constexpr (is_i2c(Instance)) {
            auto member = (i == 0) ? &I2C_HandleTypeDef::hdmarx : &I2C_HandleTypeDef::hdmatx;
            inscribed_streams[inscribed_index] = make_dma_entry(handle, dma, member, irq);
        }
        else if constexpr (is_adc(Instance)) {
            auto member = &ADC_HandleTypeDef::DMA_Handle;
            inscribed_streams[inscribed_index] = make_dma_entry(handle, dma, member, irq);
        }
        else if constexpr (is_fmac(Instance)) {
            DMA_HandleTypeDef* FMAC_HandleTypeDef::* member;
            if (i == 0) member = &FMAC_HandleTypeDef::hdmaPreload;
            else if (i == 1) member = &FMAC_HandleTypeDef::hdmaIn;
            else member = &FMAC_HandleTypeDef::hdmaOut;
            inscribed_streams[inscribed_index] = make_dma_entry(handle, dma, member, irq);
        }
        
        inscribed_index++;
    }
}

template<typename PeriphHandle>
DMA::DmaLinkEntry<PeriphHandle> DMA::make_dma_entry(
    PeriphHandle* periph,
    DMA_HandleTypeDef* dma,
    DMA_HandleTypeDef* PeriphHandle::* member,
    IRQn_Type irqn
){
    return std::tuple{
        periph,
        dma,
        member,
        irqn
    };
}

bool DMA::is_stream_available(uintptr_t stream) {
    return used_streams.find(stream) == used_streams.end();
}

bool DMA::is_peripherial_available(uintptr_t peripheral) {
    return used_peripherials.find(peripheral) == used_peripherials.end();
}

constexpr bool DMA::is_one_of(auto Instance, auto... Bases) {
    return ((Instance == Bases) || ...);
}

constexpr bool DMA::is_spi(auto Instance) {
    return is_one_of(Instance, SPI1, SPI2, SPI3, SPI4, SPI5);
}

constexpr bool DMA::is_i2c(auto Instance) {
    return is_one_of(Instance, I2C1, I2C2, I2C3, I2C5);
}

constexpr bool DMA::is_adc(auto Instance) {
    return is_one_of(Instance, ADC1, ADC2, ADC3);
}

constexpr bool DMA::is_fmac(auto Instance) {
    return Instance == FMAC;
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
        return DMA_PDATAALIGN_WORD;
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