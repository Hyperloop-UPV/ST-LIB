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

class DMA {
public:
    enum Stream : uint8_t {
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

    static void inscribe_stream();
    static void inscribe_stream(Stream dma_stream);
    static void start();

private:
    static vector<Stream> available_streams;
    static vector<Stream> inscribed_streams;
};