/*
 * DMA.hpp
 *
 *  Created on: 10 dic. 2022
 *      Author: aleja
 */

#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "stm32h7xx_hal.h"
#include "HALAL/Models/MPUManager/MPUManager.hpp"
#include <cassert>
#define MAX_NUM_STREAMS 15



class DMA {
    public:
    enum class Stream : uint8_t {
        DMA1Stream0 = 11,
        DMA1Stream1 = 12,
        DMA1Stream2 = 13,
        DMA1Stream3 = 14,
        DMA1Stream4 = 15,
        DMA1Stream5 = 16,
        DMA1Stream6 = 17,
        DMA2Stream0 = 56,
        DMA2Stream1 = 57,
        DMA2Stream2 = 58,
        DMA2Stream3 = 59,
        DMA2Stream4 = 60,
        DMA2Stream5 = 68,
        DMA2Stream6 = 69,
        DMA2Stream7 = 70,
    };
    struct DMAStream {
        Stream id;
        bool used = false;

        constexpr DMAStream(Stream id) : id(id) {}

        consteval void inscribe() {
            if (used) {
                throw "DMA stream duplicado (compile-time)";
            }
            used = true;
        }

        constexpr bool is_free() const { return !used; }
    };
public:
    static inline  std::array<DMAStream, MAX_NUM_STREAMS> streams = {{
        {Stream::DMA1Stream0}, {Stream::DMA1Stream1}, {Stream::DMA1Stream2},
        {Stream::DMA1Stream3}, {Stream::DMA1Stream4}, {Stream::DMA1Stream5},
        {Stream::DMA1Stream6}, {Stream::DMA2Stream0}, {Stream::DMA2Stream1},
        {Stream::DMA2Stream2}, {Stream::DMA2Stream3}, {Stream::DMA2Stream4},
        {Stream::DMA2Stream5}, {Stream::DMA2Stream6}, {Stream::DMA2Stream7},
    }};

    // --- Inscribir un stream concreto ---
    static consteval void inscribe_stream(Stream s) {
        for (auto& st : streams) {
            if (st.id == s) {
                st.inscribe();
                return;
            }
        }
        throw "Stream DMA no encontrado";
    }

    // --- Buscar el primer stream libre ---
    static consteval Stream inscribe_first_available() {
        for (auto& st : streams) {
            if (st.is_free()) {
                st.inscribe();
                return st.id;
            }
        }
        throw "No hay streams DMA libres";
    }

    static void start() {
        __HAL_RCC_DMA1_CLK_ENABLE();
        __HAL_RCC_DMA2_CLK_ENABLE();

        for (auto& st : streams) {
            if (!st.is_free()) {
                HAL_NVIC_SetPriority(static_cast<IRQn_Type>(st.id), 0, 0);
                HAL_NVIC_EnableIRQ(static_cast<IRQn_Type>(st.id));
            }
        }
    }
};
