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


extern "C" {
    inline DMA_HandleTypeDef *dma_irq_table[16] = {nullptr};
}


namespace ST_LIB {
    extern void compile_error(const char *msg);
    struct DMA_Domain {
        
        enum class Peripheral : uint8_t {none, adc1, adc2, adc3, 
                                        i2c1, i2c2, i2c3, i2c5,  
                                        spi1, spi2, spi3, spi4, spi5, spi6,
                                        fmac};
        
        enum class Stream : uint8_t {none, dma1_stream0, dma1_stream1, dma1_stream2, dma1_stream3, 
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
                case Stream::none: return nullptr;
            }
            return nullptr;
        }
        
        struct Entry {
            Peripheral instance;
            Stream stream;
            IRQn_Type irqn;
            uint8_t id;
        };

        template<Stream... Ss>
        struct DMA {
            using domain = DMA_Domain;

            std::array<Entry, sizeof...(Ss)> e{};

            
            consteval DMA(Peripheral instance) {
                static_assert(sizeof...(Ss) <= 3, "Máximo 3 streams");

                Stream streams[] = { Ss... };
                constexpr uint8_t n = sizeof...(Ss);

                for (uint8_t j = 0; j < n; j++) {
                    e[j].instance = instance;
                    e[j].stream   = streams[j];
                    if (streams[j] != Stream::none) {
                        e[j].irqn     = get_irqn(streams[j]);
                    } else {
                        e[j].irqn = (IRQn_Type)0; // Dummy value
                    }
                    e[j].id       = j;
                }
                
            }

            template <class Ctx>
            consteval array<size_t, sizeof...(Ss)> inscribe(Ctx &ctx) const {
                array<size_t, sizeof...(Ss)> indices{};
                for (size_t i = 0; i < sizeof...(Ss); i++) {
                    indices[i] = ctx.template add<DMA_Domain>(e[i], this);
                }
                return indices;
            }
        };

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
            else if (stream == Stream::none) return (IRQn_Type)0;
            else compile_error("No tiene que llegar aqui nunca, creo");
        }

        static constexpr inline bool is_one_of(Peripheral instance, auto... bases) {
            return ((instance == bases) || ...);
        }

        static constexpr inline bool is_spi(Peripheral instance) {
            return is_one_of(instance, Peripheral::spi1, Peripheral::spi2,
                            Peripheral::spi3, Peripheral::spi4, Peripheral::spi5, Peripheral::spi6);
        }

        static constexpr inline bool is_i2c(Peripheral instance) {
            return is_one_of(instance, Peripheral::i2c1, Peripheral::i2c2,
                            Peripheral::i2c3, Peripheral::i2c5);
        }

        static constexpr inline bool is_adc(Peripheral instance) {
            return is_one_of(instance, Peripheral::adc1, Peripheral::adc2, Peripheral::adc3);
        }

        static constexpr inline bool is_fmac(Peripheral instance) {
            return instance == Peripheral::fmac;
        }

        static constexpr inline bool is_none(Peripheral instance){
            return instance == Peripheral::none;
        }

        static consteval inline uint32_t get_Request(Peripheral instance, uint8_t i) {
            if (instance == Peripheral::none) return DMA_REQUEST_MEM2MEM;

            if (instance == Peripheral::adc1) return DMA_REQUEST_ADC1;
            if (instance == Peripheral::adc2) return DMA_REQUEST_ADC2;
            if (instance == Peripheral::adc3) return DMA_REQUEST_ADC3;

            if (instance == Peripheral::i2c1 && i == 0) return DMA_REQUEST_I2C1_RX;
            if (instance == Peripheral::i2c1 && i == 1) return DMA_REQUEST_I2C1_TX;
            if (instance == Peripheral::i2c2 && i == 0) return DMA_REQUEST_I2C2_RX;
            if (instance == Peripheral::i2c2 && i == 1) return DMA_REQUEST_I2C2_TX;
            if (instance == Peripheral::i2c3 && i == 0) return DMA_REQUEST_I2C3_RX;
            if (instance == Peripheral::i2c3 && i == 1) return DMA_REQUEST_I2C3_TX;
            if (instance == Peripheral::i2c5 && i == 0) return DMA_REQUEST_I2C5_RX; 
            if (instance == Peripheral::i2c5 && i == 1) return DMA_REQUEST_I2C5_TX;

            if (instance == Peripheral::spi1 && i == 0) return DMA_REQUEST_SPI1_RX;
            if (instance == Peripheral::spi1 && i == 1) return DMA_REQUEST_SPI1_TX;
            if (instance == Peripheral::spi2 && i == 0) return DMA_REQUEST_SPI2_RX;
            if (instance == Peripheral::spi2 && i == 1) return DMA_REQUEST_SPI2_TX;
            if (instance == Peripheral::spi3 && i == 0) return DMA_REQUEST_SPI3_RX;
            if (instance == Peripheral::spi3 && i == 1) return DMA_REQUEST_SPI3_TX;
            if (instance == Peripheral::spi4 && i == 0) return DMA_REQUEST_SPI4_RX;
            if (instance == Peripheral::spi4 && i == 1) return DMA_REQUEST_SPI4_TX;
            if (instance == Peripheral::spi5 && i == 0) return DMA_REQUEST_SPI5_RX;
            if (instance == Peripheral::spi5 && i == 1) return DMA_REQUEST_SPI5_TX; 
            
            if (instance == Peripheral::fmac && i == 0) return DMA_REQUEST_MEM2MEM;
            if (instance == Peripheral::fmac && i == 1) return DMA_REQUEST_FMAC_WRITE;
            if (instance == Peripheral::fmac && i == 2) return DMA_REQUEST_FMAC_READ;

            compile_error("Invalid DMA request configuration");
            return 0;
        }

        static consteval inline uint32_t get_Direction(Peripheral instance, uint8_t i) {
            if ((is_fmac(instance) && i == 0) || instance == Peripheral::none) {
                    return DMA_MEMORY_TO_MEMORY;
                }
            else if  ((is_i2c(instance) && i == 1) ||
                (is_spi(instance) && i == 1) ||
                (is_fmac(instance) && i == 1)){
                    return DMA_MEMORY_TO_PERIPH;
                }
            return DMA_PERIPH_TO_MEMORY;
        }

        static consteval inline uint32_t get_PeriphInc(Peripheral instance, uint8_t i) {
            if  ((is_fmac(instance) && i == 0) || is_none(instance)){
                return DMA_PINC_ENABLE;
            }
            return DMA_PINC_DISABLE;
        }

        static consteval inline uint32_t get_MemInc(Peripheral instance, uint8_t i) {
            if  (is_fmac(instance) && i == 0){
                return DMA_MINC_DISABLE;
            }
            return DMA_MINC_ENABLE;
        }

        static consteval inline uint32_t get_PeriphDataAlignment(Peripheral instance, uint8_t i) {
            if  (is_spi(instance) || is_i2c(instance)){
                return DMA_PDATAALIGN_BYTE; 
            }
            else if (is_none(instance)){
                return DMA_PDATAALIGN_WORD;
            }
            return DMA_PDATAALIGN_HALFWORD;
        }

        static consteval inline uint32_t get_MemDataAlignment(Peripheral instance, uint8_t i) {
            if  (is_i2c(instance)){
                return DMA_MDATAALIGN_WORD;
            }
            else if  (is_spi(instance)){
                return DMA_MDATAALIGN_BYTE;
            }

            return DMA_MDATAALIGN_HALFWORD;
        }

        static consteval inline uint32_t get_Mode(Peripheral instance, uint8_t i) {
            if  (is_spi(instance) || is_fmac(instance) || is_none(instance)){
                return DMA_NORMAL;
            }
            
            return DMA_CIRCULAR;
        }    

        static consteval inline uint32_t get_Priority(Peripheral instance, uint8_t i) {
            if  (is_fmac(instance)){
                return DMA_PRIORITY_HIGH;
            }
            
            return DMA_PRIORITY_LOW;
        }
        
        static consteval inline uint32_t get_FIFOMode(Peripheral instance, uint8_t i) {
            if (is_fmac(instance)){
                return DMA_FIFOMODE_ENABLE;
            }
            return DMA_FIFOMODE_DISABLE;
        }

        static consteval inline uint32_t get_FIFOThreshold(Peripheral instance, uint8_t i) {
            if  (is_spi(instance)){
                return DMA_FIFO_THRESHOLD_FULL;
            }
            return DMA_FIFO_THRESHOLD_HALFFULL;
        }

        static consteval inline uint32_t get_MemBurst(Peripheral instance, uint8_t i) {
            return DMA_MBURST_SINGLE;
        }

        static consteval inline uint32_t get_PeriphBurst(Peripheral instance, uint8_t i) {
            return DMA_PBURST_SINGLE;
        }

        struct Config {
            std::tuple<Peripheral, 
                    DMA_InitTypeDef, 
                    Stream, 
                    IRQn_Type,
                    uint8_t> 
            init_data{};
        };

        template <size_t N>
        static consteval std::array<Config, N> build(span<const Entry> instances) {
            std::array<Config, N> cfgs{};
            std::array<Entry, N> ents;
            for(size_t i=0; i<N; ++i) ents[i] = instances[i];

            std::array<bool, MAX_STREAMS> used_streams{}; // Defaults to false

            // First pass: process user-specified streams
            for (std::size_t i = 0; i < N; ++i) {
                const auto &e = ents[i];
                if (e.stream != Stream::none) {
                    uint8_t stream_idx = static_cast<uint8_t>(e.stream) - 1;
                    if (used_streams[stream_idx]) {
                        compile_error("DMA stream already in use");
                    }
                    used_streams[stream_idx] = true;
                }
            }

            // Second pass: assign streams for entries with Stream::none
            for (std::size_t i = 0; i < N; ++i) {
                auto &e = ents[i];
                if (e.stream == Stream::none) {
                    bool assigned = false;
                    for (uint8_t j = 0; j < MAX_STREAMS; ++j) {
                        if (!used_streams[j]) {
                            e.stream = static_cast<Stream>(j + 1);
                            e.irqn = get_irqn(e.stream);
                            used_streams[j] = true;
                            assigned = true;
                            break;
                        }
                    }
                    if (!assigned) {
                        compile_error("Not enough DMA streams available");
                    }
                }
            }


            for (std::size_t i = 0; i < N; ++i){
                const auto &e = ents[i];

                for (std::size_t j = 0; j < i; ++j){
                    const auto &prev = ents[j];
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


        
        struct Instance {
            DMA_HandleTypeDef dma;

            void start(uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength){
                HAL_DMA_Start_IT(&dma, SrcAddress, DstAddress, DataLength);
            }
        };

        
        template <std::size_t N> struct Init {
            static inline std::array<Instance, N> instances{};

            static void init(std::span<const Config, N> cfgs) {
                if (N == 0) return;
                __HAL_RCC_DMA1_CLK_ENABLE();
	            __HAL_RCC_DMA2_CLK_ENABLE();
                for (std::size_t i = 0; i < N; ++i) {
                    const auto &e = cfgs[i];
                    auto [instance, dma_init, stream, irqn, id] = e.init_data;           
                    
                    instances[i].dma = {};
                    instances[i].dma.Instance = stream_to_DMA_StreamTypeDef(stream);
                    instances[i].dma.Init = dma_init;

                    if (HAL_DMA_Init(&instances[i].dma) != HAL_OK) {
                        ErrorHandler("DMA Init failed");
                    }
                    else{
                        HAL_NVIC_SetPriority(irqn, 0, 0);
                        HAL_NVIC_EnableIRQ(irqn);
                        dma_irq_table[static_cast<uint8_t>(stream) - 1] = &instances[i].dma;
                    }
                }
            }
        };
    };
}

