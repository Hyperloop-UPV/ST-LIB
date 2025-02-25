#pragma once

#include "CppImports.hpp"


/**
 * @brief a Buffer class that acts as a RingBuffer using FIFO behaviour
 */
template <typename item_type, size_t N>
class RingBuffer{
public:
	RingBuffer(): msize(0), mcapacity(N), mhead_index(0), mlast_index(N){}


	/**
	 * @brief introduces a new item on the Ring Buffer unless it is full. Returns false when the Ring Buffer is full.
	 */
	bool push(item_type item){
		if(msize == mcapacity){
			return false;
		}

		msize++;
		mlast_index++;
		if(mlast_index >= mcapacity){
			mlast_index = 0;
		}

		buffer[mlast_index] = item;
		return true;
	}


	bool is_empty(){return msize == 0;}  /**< @brief returns true when RingBuffer is empty, false otherwise.*/
	bool is_full(){return msize == mcapacity;}  /**< @brief returns true when RingBuffer is full, false otherwise.*/
	uint32_t size(){return msize;}	/**< @brief returns the occupied size of the RingBuffer.*/




	/**
	 * @brief checks the next variable to read without removing it from the buffer. Do not use on empty buffer.
	 */
	item_type peek(){
		return buffer[mhead_index];
	}

	/**
	 * @brief checks the next variable to read and removes it from the buffer. Do not use on empty buffer.
	 */
	item_type pop(){
		uint32_t return_index = mhead_index;
		mhead_index++;
		msize--;
		if(mhead_index >= mcapacity){
			mhead_index = 0;
		}
		return buffer[return_index];
	}

	/**
	 * @brief pop later and push new. Don't care size of buffer, needs to be initialize before
	 */
	void push_pop(item_type item){
		mhead_index = (mhead_index+1)%mcapacity;
		mlast_index = (mlast_index+1)%mcapacity;
		buffer[mhead_index] = item;
	}

	item_type latest()const{
		return buffer[mlast_index];
	}

private:
	std::array<item_type, N> buffer;
	uint32_t msize;
	uint32_t mcapacity;
	uint32_t mhead_index;
	uint32_t mlast_index;
};
