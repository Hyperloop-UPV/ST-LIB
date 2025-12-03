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
        
        enum class Stream : uint8_t {dma1_stream0, dma1_stream1, dma1_stream2, dma1_stream3, 
                                        dma1_stream4, dma1_stream5, dma1_stream6, dma1_stream7, 
                                        dma2_stream0, dma2_stream1, dma2_stream2, dma2_stream3, 
                                        dma2_stream4, dma2_stream5, dma2_stream6, dma2_stream7};
                                                    
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
            std::array<std::tuple<Stream, IRQn_Type>, 3> streams{};
            uint8_t count = 0;
        };

        struct DMA{
            using domain = DMA_Domain;

            Entry e;

            template<Stream... Ss>
            consteval DMA(Instance instance) : e(instance) {
                static_assert(sizeof...(Ss) <= 3, "Máximo 3 streams");
                size_t i = 0;
                ((e.streams[i++] = std::make_tuple(Ss, get_irqn(Ss))), ...);
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

        static inline IRQn_Type get_irqn(Stream stream) {
            if (stream == Stream::dma1_stream0) return DMA1_Stream0_IRQn;
            else if (stream == Stream::dma1_stream1) return DMA1_Stream1_IRQn;
            else if (stream == Stream::dma1_stream2) return DMA1_Stream2_IRQn;
            else if (stream == Stream::dma1_stream3) return DMA1_Stream3_IRQn;
            else if (stream == Stream::dma1_stream4) return DMA1_Stream4_IRQn;
            else if (stream == Stream::dma1_stream5) return DMA1_Stream5_IRQn;
            else if (stream == Stream::dma1_stream6) return DMA1_Stream6_IRQn;
            else if (stream == Stream::dma1_stream7) return DMA1_Stream7_IRQn;

            else if (stream == Stream::dma2_stream0) return DMA2_Stream0_IRQn;
            else if (stream == Stream::dma2_stream1) return DMA2_Stream1_IRQn;
            else if (stream == Stream::dma2_stream2) return DMA2_Stream2_IRQn;
            else if (stream == Stream::dma2_stream3) return DMA2_Stream3_IRQn;
            else if (stream == Stream::dma2_stream4) return DMA2_Stream4_IRQn;
            else if (stream == Stream::dma2_stream5) return DMA2_Stream5_IRQn;
            else if (stream == Stream::dma2_stream6) return DMA2_Stream6_IRQn;
            else if (stream == Stream::dma2_stream7) return DMA2_Stream7_IRQn;
            else ErrorHandler("Unknown DMA stream");
            return DMA1_Stream0_IRQn; // Nunca se alcanza
        }

        // Si quitas el auto peta todo
        static consteval inline bool is_one_of(Instance instance, auto... bases) {
            return ((instance == bases) || ...);
        }

        static consteval inline bool is_spi(Instance instance) {
            return is_one_of(instance, Instance::spi1, Instance::spi2,
                            Instance::spi3, Instance::spi4, Instance::spi5);
        }

        static consteval inline bool is_i2c(Instance instance) {
            return is_one_of(instance, Instance::i2c1, Instance::i2c2,
                            Instance::i2c3, Instance::i2c5);
        }

        static consteval inline bool is_adc(Instance instance) {
            return is_one_of(instance, Instance::adc1, Instance::adc2, Instance::adc3);
        }

        static consteval inline bool is_fmac(Instance instance) {
            return instance == Instance::fmac;
        }

        static consteval inline uint32_t get_Request(Instance instance, uint8_t i) {
            if (instance == Instance::adc1) return DMA_REQUEST_ADC1;
            if (instance == Instance::adc2) return DMA_REQUEST_ADC2;
            if (instance == Instance::adc3) return DMA_REQUEST_ADC3;

            if (instance == Instance::i2c1 && i == 0) return DMA_REQUEST_I2C1_RX;
            if (instance == Instance::i2c1 && i == 1) return DMA_REQUEST_I2C1_TX;
            if (instance == Instance::i2c2 && i == 0) return DMA_REQUEST_I2C2_RX;
            if (instance == Instance::i2c2 && i == 1) return DMA_REQUEST_I2C2_TX;
            if (instance == Instance::i2c3 && i == 0) return DMA_REQUEST_I2C3_RX;
            if (instance == Instance::i2c3 && i == 1) return DMA_REQUEST_I2C3_TX;
            if (instance == Instance::i2c5 && i == 0) return DMA_REQUEST_I2C5_RX; 
            if (instance == Instance::i2c5 && i == 1) return DMA_REQUEST_I2C5_TX;

            if (instance == Instance::spi1 && i == 0) return DMA_REQUEST_SPI1_RX;
            if (instance == Instance::spi1 && i == 1) return DMA_REQUEST_SPI1_TX;
            if (instance == Instance::spi2 && i == 0) return DMA_REQUEST_SPI2_RX;
            if (instance == Instance::spi2 && i == 1) return DMA_REQUEST_SPI2_TX;
            if (instance == Instance::spi3 && i == 0) return DMA_REQUEST_SPI3_RX;
            if (instance == Instance::spi3 && i == 1) return DMA_REQUEST_SPI3_TX;
            if (instance == Instance::spi4 && i == 0) return DMA_REQUEST_SPI4_RX;
            if (instance == Instance::spi4 && i == 1) return DMA_REQUEST_SPI4_TX;
            if (instance == Instance::spi5 && i == 0) return DMA_REQUEST_SPI5_RX;
            if (instance == Instance::spi5 && i == 1) return DMA_REQUEST_SPI5_TX; 
            
            if (instance == Instance::fmac && i == 0) return DMA_REQUEST_MEM2MEM;
            if (instance == Instance::fmac && i == 1) return DMA_REQUEST_FMAC_WRITE;
            if (instance == Instance::fmac && i == 2) return DMA_REQUEST_FMAC_READ;

            ErrorHandler("Invalid DMA request configuration");
            return 0;
        }


        static consteval inline uint32_t get_Direction(Instance instance, uint8_t i) {
            if (is_fmac(instance) && i == 0){
                    return DMA_MEMORY_TO_MEMORY;
                }
            else if  ((is_i2c(instance) && i == 1) ||
                (is_spi(instance) && i == 1) ||
                (is_fmac(instance) && i == 1)){
                    return DMA_MEMORY_TO_PERIPH;
                }

            return DMA_PERIPH_TO_MEMORY;
        }

        static consteval inline uint32_t get_PeriphInc(Instance instance, uint8_t i) {
            if  (is_fmac(instance) && i == 0){
                return DMA_PINC_ENABLE;
            }
            return DMA_PINC_DISABLE;
        }
        static consteval inline uint32_t get_MemInc(Instance instance, uint8_t i) {
            if  (is_fmac(instance) && i == 0){
                return DMA_MINC_DISABLE;
            }
            return DMA_MINC_ENABLE;
        }

        static consteval inline uint32_t get_PeriphDataAlignment(Instance instance, uint8_t i) {
            if  (is_i2c(instance)){
                return DMA_PDATAALIGN_WORD; // Revisar esto, I2C suele trabajar con bytes
            }
            else if  (is_spi(instance)){
                return DMA_PDATAALIGN_BYTE; 
            }

            return DMA_PDATAALIGN_HALFWORD;
        }

        static consteval inline uint32_t get_MemDataAlignment(Instance instance, uint8_t i) {
            if  (is_i2c(instance)){
                return DMA_MDATAALIGN_WORD;
            }
            else if  (is_spi(instance)){
                return DMA_MDATAALIGN_BYTE;
            }

            return DMA_MDATAALIGN_HALFWORD;
        }
        static consteval inline uint32_t get_Mode(Instance instance, uint8_t i) {
            if  (is_spi(instance) || is_fmac(instance)){
                return DMA_NORMAL;
            }
            
            return DMA_CIRCULAR;
        }    

        static consteval inline uint32_t get_Priority(Instance instance, uint8_t i) {
            if  (is_fmac(instance)){
                return DMA_PRIORITY_HIGH;
            }
            
            return DMA_PRIORITY_LOW;
        }
        static consteval inline uint32_t get_FIFOMode(Instance instance, uint8_t i) {
            if (is_fmac(instance)){
                return DMA_FIFOMODE_ENABLE;
            }
            return DMA_FIFOMODE_DISABLE;
        }

        static consteval inline uint32_t get_FIFOThreshold(Instance instance, uint8_t i) {
            if  (is_spi(instance)){
                return DMA_FIFO_THRESHOLD_FULL;
            }
            return DMA_FIFO_THRESHOLD_HALFFULL;
        }

        static consteval inline uint32_t get_MemBurst(Instance instance, uint8_t i) {
            return DMA_MBURST_SINGLE;
        }

        static consteval inline uint32_t get_PeriphBurst(Instance instance, uint8_t i) {
            return DMA_PBURST_SINGLE;
        }


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
                    DMA_HandleStruct.Init.Request             = get_Request(e.instance, j);
                    DMA_HandleStruct.Init.Direction           = get_Direction(e.instance, j);
                    DMA_HandleStruct.Init.PeriphInc           = get_PeriphInc(e.instance, j);
                    DMA_HandleStruct.Init.MemInc              = get_MemInc(e.instance, j);
                    DMA_HandleStruct.Init.PeriphDataAlignment = get_PeriphDataAlignment(e.instance, j);
                    DMA_HandleStruct.Init.MemDataAlignment    = get_MemDataAlignment(e.instance, j);
                    DMA_HandleStruct.Init.Mode                = get_Mode(e.instance, j);
                    DMA_HandleStruct.Init.Priority            = get_Priority(e.instance, j);
                    DMA_HandleStruct.Init.FIFOMode            = get_FIFOMode(e.instance, j);
                    DMA_HandleStruct.Init.FIFOThreshold       = get_FIFOThreshold(e.instance, j);
                    DMA_HandleStruct.Init.MemBurst            = get_MemBurst(e.instance, j);
                    DMA_HandleStruct.Init.PeriphBurst         = get_PeriphBurst(e.instance, j);

                    dma_handles[i] = DMA_HandleStruct;
                }

                cfgs[i].init_data = std::make_tuple(e.instance, dma_handles);
            }
        }
    };
}   