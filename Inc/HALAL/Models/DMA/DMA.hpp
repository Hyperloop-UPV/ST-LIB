#pragma once
#include "C++Utilities/CppUtils.hpp"
#include "stm32h7xx_hal.h"
#include "main.h"
#include "HALAL/Models/MPUManager/MPUManager.hpp"
#include <cassert>
#include <array>
#include <variant>
#include <functional>
#include <set>

using std::array;
using std::size_t;
using std::span;
using std::tuple;

#define MAX_STREAMS 16
    

namespace ST_LIB {
    struct DMA_Domain {
        using xTypeDef = std::variant<
            ADC_TypeDef*,
            I2C_TypeDef*,        
            SPI_TypeDef*,
            FMAC_TypeDef*
        >;
        using xHandleDef = std::variant<
            ADC_HandleTypeDef*,
            I2C_HandleTypeDef*,        
            SPI_HandleTypeDef*,
            FMAC_HandleTypeDef*
        >;

        enum class Instance : uint8_t {adc1, adc2, adc3, 
                                        i2c1, i2c2, i2c3, i2c5,  
                                        spi1, spi2, spi3, spi4, spi5,
                                        fmac};
        
        static inline xTypeDef instance_to_xTypeDef(Instance i) {
            switch (i) {
                case Instance::adc1: return ADC1;
                case Instance::adc2: return ADC2;
                case Instance::adc3: return ADC3;

                case Instance::i2c1: return I2C1;
                case Instance::i2c2: return I2C2;
                case Instance::i2c3: return I2C3;
                case Instance::i2c5: return I2C5;

                case Instance::spi1: return SPI1;
                case Instance::spi2: return SPI2;
                case Instance::spi3: return SPI3;
                case Instance::spi4: return SPI4;
                case Instance::spi5: return SPI5;

                case Instance::fmac: return FMAC;
            }
        }

        enum class Stream : uint8_t {dma1_stream0, dma1_stream1, dma1_stream2, dma1_stream3, 
                                        dma1_stream4, dma1_stream5, dma1_stream6, dma1_stream7, 
                                        dma2_stream0, dma2_stream1, dma2_stream2, dma2_stream3, 
                                        dma2_stream4, dma2_stream5, dma2_stream6, dma2_stream7};
                
        static inline DMA_Stream_TypeDef* stream_to_DMA_StreamTypeDef(Stream s) {
            switch (s) {
                case Stream::dma1_stream0: return DMA1_Stream0;
                case Stream::dma1_stream1: return DMA1_Stream1;
                case Stream::dma1_stream2: return DMA1_Stream2;
                case Stream::dma1_stream3: return DMA1_Stream3;
                case Stream::dma1_stream4: return DMA1_Stream4;
                case Stream::dma1_stream5: return DMA1_Stream5;
                case Stream::dma1_stream6: return DMA1_Stream6;
                case Stream::dma1_stream7: return DMA1_Stream7;

                case Stream::dma2_stream0: return DMA2_Stream0;
                case Stream::dma2_stream1: return DMA2_Stream1;
                case Stream::dma2_stream2: return DMA2_Stream2;
                case Stream::dma2_stream3: return DMA2_Stream3;
                case Stream::dma2_stream4: return DMA2_Stream4;
                case Stream::dma2_stream5: return DMA2_Stream5;
                case Stream::dma2_stream6: return DMA2_Stream6;
                case Stream::dma2_stream7: return DMA2_Stream7;
            }
        }
        
        struct Entry {
            Instance instance;
            std::array<Stream, 3> streams{};
            uint8_t count = 0;
            IRQn_Type irqn;
        };

        struct DMA{
            using domain = DMA_Domain;

            Entry e;

            template<Stream... Ss>
            consteval DMA(Instance instance) : e(instance) {
                static_assert(sizeof...(Ss) <= 3, "Máximo 3 streams");
                size_t i = 0;
                ((e.streams[i++] = Ss), ...);
                e.count = i;
            }

            template <class Ctx> consteval void inscribe(Ctx &ctx) const {
                ctx.template add<DMA_Domain>(e);
            }
        };

        static constexpr std::size_t max_instances {MAX_STREAMS};
        static_assert(max_instances > 0, "The number of instances must be greater than 0");
        
        struct Config {
            std::tuple<Instance, std::array<DMA_HandleTypeDef, 3>> init_data{};
        };

        template <size_t N>
        static consteval std::array<Config, N> build(span<const Entry> instances){
            std::array<Config, N> cfgs{};

            for (std::size_t i = 0; i < N; ++i){
                const auto &e = instances[i];

                for (std::size_t j = 0; j < i; ++j){
                    const auto &prev = instances[j];
                    if (prev.instance == e.instance && prev.streams == e.streams){
                        struct peripherial_already_inscribed {};
                        throw peripherial_already_inscribed{};
                    }
                }
                
                std::array<DMA_HandleTypeDef, 3> dma_handles;
                for (std::size_t j = 0; j < e.count; j++){
                    DMA_HandleTypeDef DMA_HandleStruct;
                    DMA_HandleStruct.Instance = e.streams[j];
                    DMA_HandleStrcut.Init.Request             = get_Request(e.instance, j);
                    DMA_HandleStrcut.Init.Direction           = get_Direction(e.instance, j);
                    DMA_HandleStrcut.Init.PeriphInc           = get_PeriphInc(e.instance, j);
                    DMA_HandleStrcut.Init.MemInc              = get_MemInc(e.instance, j);
                    DMA_HandleStrcut.Init.PeriphDataAlignment = get_PeriphDataAlignment(e.instance, j);
                    DMA_HandleStrcut.Init.MemDataAlignment    = get_MemDataAlignment(e.instance, j);
                    DMA_HandleStrcut.Init.Mode                = get_Mode(e.instance, j);
                    DMA_HandleStrcut.Init.Priority            = get_Priority(e.instance, j);
                    DMA_HandleStrcut.Init.FIFOMode            = get_FIFOMode(e.instance, j);
                    DMA_HandleStrcut.Init.FIFOThreshold       = get_FIFOThreshold(e.instance, j);
                    DMA_HandleStrcut.Init.MemBurst            = get_MemBurst(e.instance, j);
                    DMA_HandleStrcut.Init.PeriphBurst         = get_PeriphBurst(e.instance, j);

                    dma_handles[i] = DMA_HandleStruct;
                }

                cfgs[i].init_data = std::make_tuple(e.instance, dma_handles)
            }
        }
    };
}   