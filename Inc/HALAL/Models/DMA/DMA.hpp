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
    using PeriphVariant = std::variant<
            SPI_HandleTypeDef*,
            I2C_HandleTypeDef*,
            ADC_HandleTypeDef*,
            FMAC_HandleTypeDef*
        >;
        struct DmaLinkEntry{
            PeriphVariant periph; // (__HANDLE__) 
            DMA_HandleTypeDef* dma; // (__DMA_HANDLE__) 
            std::function<void(DMA_HandleTypeDef*)> linker;   // (__PPP_DMA_FIELD__)
            IRQn_Type irq; // (__IRQn_TYPE__)
        };

        template<typename PeriphHandle>
        static DmaLinkEntry make_dma_entry(
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
        
        static std::array<DmaLinkEntry, MAX_STREAMS> inscribed_streams;
        static uint8_t inscribed_index;

        static std::set<uintptr_t> used_peripherials;
        static std::set<uintptr_t> used_streams;

    public:
        static constexpr bool is_one_of(auto Instance, auto... Bases);

        static constexpr bool is_spi(auto Instance);
        
        static constexpr bool is_i2c(auto Instance);

        static constexpr bool is_adc(auto Instance);

        static constexpr bool is_fmac(auto Instance);

        template<auto Instance, auto... Streams>
        requires (
            (DMA::is_adc(Instance)  && sizeof...(Streams) == 1) ||
            (DMA::is_fmac(Instance) && sizeof...(Streams) == 3) ||
            (!DMA::is_adc(Instance) && !DMA::is_fmac(Instance) && sizeof...(Streams) == 2)
        )
        static void inscribe_stream(PeriphVariant handle);

        static void start();        
};

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

template<auto Instance, auto... Streams>
requires (
    (DMA::is_adc(Instance)  && sizeof...(Streams) == 1) ||
    (DMA::is_fmac(Instance) && sizeof...(Streams) == 3) ||
    (!DMA::is_adc(Instance) && !DMA::is_fmac(Instance) && sizeof...(Streams) == 2)
)
void DMA::inscribe_stream(PeriphVariant handle) {
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
            auto* spi_handle = std::get<SPI_HandleTypeDef*>(handle);
            auto member = (i == 0) ? &SPI_HandleTypeDef::hdmarx : &SPI_HandleTypeDef::hdmatx;
            inscribed_streams[inscribed_index] = make_dma_entry(spi_handle, dma, member, irq);
        }
        else if constexpr (is_i2c(Instance)) {
            auto* i2c_handle = std::get<I2C_HandleTypeDef*>(handle);
            auto member = (i == 0) ? &I2C_HandleTypeDef::hdmarx : &I2C_HandleTypeDef::hdmatx;
            inscribed_streams[inscribed_index] = make_dma_entry(i2c_handle, dma, member, irq);
        }
        else if constexpr (is_adc(Instance)) {
            auto* adc_handle = std::get<ADC_HandleTypeDef*>(handle);
            auto member = &ADC_HandleTypeDef::DMA_Handle;
            inscribed_streams[inscribed_index] = make_dma_entry(adc_handle, dma, member, irq);
        }
        else if constexpr (is_fmac(Instance)) {
            auto* fmac_handle = std::get<FMAC_HandleTypeDef*>(handle);
            DMA_HandleTypeDef* FMAC_HandleTypeDef::* member;
            if (i == 0) member = &FMAC_HandleTypeDef::hdmaPreload;
            else if (i == 1) member = &FMAC_HandleTypeDef::hdmaIn;
            else member = &FMAC_HandleTypeDef::hdmaOut;
            inscribed_streams[inscribed_index] = make_dma_entry(fmac_handle, dma, member, irq);
        }
        
        inscribed_index++;
    }
}

template<typename PeriphHandle>
DMA::DmaLinkEntry DMA::make_dma_entry(
    PeriphHandle* periph,
    DMA_HandleTypeDef* dma,
    DMA_HandleTypeDef* PeriphHandle::* member,
    IRQn_Type irqn
){
    return DmaLinkEntry{
        periph,
        dma,
        [periph, member, dma](DMA_HandleTypeDef* d) {
            periph->*member = d;
            d->Parent = periph;
        },
        irqn
    };
}

