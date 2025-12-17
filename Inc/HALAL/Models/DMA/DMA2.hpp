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
    extern void compile_error(const char *msg);
    struct DMA_Domain {
        
        enum class Instance : uint8_t {none, adc1, adc2, adc3, 
                                        i2c1, i2c2, i2c3, i2c5,  
                                        spi1, spi2, spi3, spi4, spi5,
                                        fmac};
        
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
            Stream stream;
            IRQn_Type irqn;
            uint8_t id;
        };

        template<Stream... Ss>
        struct DMA {
            using domain = DMA_Domain;

            std::array<Entry, 3> e{};

            
            consteval DMA(Instance instance) {
                static_assert(sizeof...(Ss) <= 3, "Máximo 3 streams");

                Stream streams[] = { Ss... };
                constexpr uint8_t n = sizeof...(Ss);

                for (uint8_t j = 0; j < n; j++) {
                    e[j].instance = instance;
                    e[j].stream   = streams[j];
                    e[j].irqn     = get_irqn(streams[j]);
                    e[j].id       = j;
                }
                
            }

            template <class Ctx>
            consteval void inscribe(Ctx &ctx) const {
                for (const auto& entry : e) {
                    if (entry.stream != Stream{}) 
                        ctx.template add<DMA_Domain>(entry);
                }
            }
        };

        // NO se para que quiero esto
        static constexpr std::size_t max_instances {MAX_STREAMS};
        static_assert(max_instances > 0, "The number of instances must be greater than 0");

        static inline constexpr IRQn_Type get_irqn(Stream stream) {
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
        static constexpr inline bool is_one_of(Instance instance, auto... bases) {
            return ((instance == bases) || ...);
        }

        static constexpr inline bool is_spi(Instance instance) {
            return is_one_of(instance, Instance::spi1, Instance::spi2,
                            Instance::spi3, Instance::spi4, Instance::spi5);
        }

        static constexpr inline bool is_i2c(Instance instance) {
            return is_one_of(instance, Instance::i2c1, Instance::i2c2,
                            Instance::i2c3, Instance::i2c5);
        }

        static constexpr inline bool is_adc(Instance instance) {
            return is_one_of(instance, Instance::adc1, Instance::adc2, Instance::adc3);
        }

        static constexpr inline bool is_fmac(Instance instance) {
            return instance == Instance::fmac;
        }

        static consteval inline uint32_t get_Request(Instance instance, uint8_t i) {
            if (instance == Instance::none) return DMA_REQUEST_MEM2MEM;

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
            if ((is_fmac(instance) && i == 0) || instance == Instance::none) {
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
            if  (is_spi(instance) || is_fmac(instance) || instance == Instance::none){
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

        struct Config {
            std::tuple<Instance, 
                    DMA_InitTypeDef, 
                    Stream, 
                    IRQn_Type,
                    uint8_t> 
            init_data{};
        };

        template <size_t N>
        static consteval std::array<Config, N> build(span<const Entry> instances){
            std::array<Config, N> cfgs{};

            for (std::size_t i = 0; i < N; ++i){
                const auto &e = instances[i];

                // No entiendo como funciona esto, pero esta copiado tal cual
                // Yo creo que a mi no me sirve porque mis instancias si que estan repetidas
                for (std::size_t j = 0; j < i; ++j){
                    const auto &prev = instances[j];
                    if (prev.stream == e.stream){
                        compile_error("DMA stream already in use");
                    }
                }
        
                DMA_InitTypeDef DMA_InitStruct;
                DMA_InitStruct.Request             = get_Request(e.instance, e.id);
                DMA_InitStruct.Direction           = get_Direction(e.instance, e.id);
                DMA_InitStruct.PeriphInc           = get_PeriphInc(e.instance, e.id);
                DMA_InitStruct.MemInc              = get_MemInc(e.instance, e.id);
                DMA_InitStruct.PeriphDataAlignment = get_PeriphDataAlignment(e.instance, e.id);
                DMA_InitStruct.MemDataAlignment    = get_MemDataAlignment(e.instance, e.id);
                DMA_InitStruct.Mode                = get_Mode(e.instance, e.id);
                DMA_InitStruct.Priority            = get_Priority(e.instance, e.id);
                DMA_InitStruct.FIFOMode            = get_FIFOMode(e.instance, e.id);
                DMA_InitStruct.FIFOThreshold       = get_FIFOThreshold(e.instance, e.id);
                DMA_InitStruct.MemBurst            = get_MemBurst(e.instance, e.id);
                DMA_InitStruct.PeriphBurst         = get_PeriphBurst(e.instance, e.id);


                cfgs[i].init_data = std::make_tuple(e.instance,
                                                    DMA_InitStruct,
                                                    e.stream,
                                                    e.irqn,
                                                    e.id);
            }
            return cfgs;
        }

        struct Instances_ {
            DMA_HandleTypeDef dma;

            void start(uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength){
                HAL_DMA_Start_IT(&dma, SrcAddress, DstAddress, DataLength);
            }
        };

        
        template <std::size_t N> struct Init {
            static inline std::array<Instances_, N> instances{};

            static void init(std::span<const Config, N> cfgs) {
                //static_assert(N > 0);
                __HAL_RCC_DMA1_CLK_ENABLE();
	            __HAL_RCC_DMA2_CLK_ENABLE();
                for (std::size_t i = 0; i < N; ++i) {
                    const auto &e = cfgs[i];
                    auto [instance, dma_init, stream, irqn, id] = e.init_data;           
                    
                    instances[i].dma.Instance = stream_to_DMA_StreamTypeDef(stream);
                    instances[i].dma.Init = dma_init;

                    if (HAL_DMA_Init(&instances[i].dma) != HAL_OK) {
                        ErrorHandler("DMA Init failed");
                    }
                    else{
                        HAL_NVIC_SetPriority(irqn, 0, 0);
                        HAL_NVIC_EnableIRQ(irqn);
                    }
                }
            }
        };
    };
}
