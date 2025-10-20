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

void static  inline constexpr  inscribe_stream() {
	for (int i = 0; i < MAX_NUM_STREAMS; i++) {
		if (available_streams[i] == true) {
			available_streams[i] = false;
			return;
		}
	}
	assert(false); // No DMA_Stream available
}
void static inline consteval inscribe_stream(Stream dma_stream) {
		for(int i = 0; i < MAX_NUM_STREAMS; i++){
			if(streams[i] == dma_stream && available_streams[i] == true){
				available_streams[i] = false;
				return;
			}else{
				break;
			}
		}
		assert(false) //The DMA_STREAM that you want is not available;
}
	static void start();

private:
	static   std::array<bool,MAX_NUM_STREAMS> available_streams{true};
	static  constexpr std::array<Stream, 15> streams = {
    Stream::DMA1Stream0, Stream::DMA1Stream1, Stream::DMA1Stream2, Stream::DMA1Stream3,
    Stream::DMA1Stream4, Stream::DMA1Stream5, Stream::DMA1Stream6,
    Stream::DMA2Stream0, Stream::DMA2Stream1, Stream::DMA2Stream2, Stream::DMA2Stream3,
    Stream::DMA2Stream4, Stream::DMA2Stream5, Stream::DMA2Stream6, Stream::DMA2Stream7
	};
};
