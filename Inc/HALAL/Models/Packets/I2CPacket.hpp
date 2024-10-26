/*
 * SPI.hpp
 *
 *  Created on: 5 nov. 2022
 *      Author: Pablo
 */

#pragma once

#include <cstdint>
#include <malloc.h>

/**
 * @brief I2C data packet class. Contains a pointer to the data stored
 *            in bytes and the number of bytes.
 *
 */
class I2CPacket
{
    uint32_t size; /**< Size in bytes of the data. */
    uint16_t id; /**< Slave id */
    uint8_t* data; /**< Data pointer. */

private:
    I2CPacket(){};

public:

    /**
     * @brief Construct a new I2C Packet object. The constructor will create an empty packet
     *        of the specified size. This constructor is intended for receive packets.
     * 
     * @param size Size in bytes of the packet data.
     */
    I2CPacket(uint32_t size) {
		this->size = size;
		this->id = 0;
		this->data = (uint8_t*)malloc(this->size);

    }

    /**
     * @brief Constructs a new I2CPacket object. The constructor will handle automatically
     *           the data type and size.
     *
     * @tparam Type Generic data type.
     * @param data Data to be stored in the packet.
     */
    template<typename Type>
    I2CPacket(uint16_t id, Type data) {
        this->size = sizeof(Type);
        this->id = id;
        this->data = (uint8_t*)malloc(this->size);

        memcpy(this->data, &data, this->size);
    }

    /**
     * @brief Constructs a new I2CPacket object by passing an array
     *           of any type and length.
     *
     * @tparam Type Generic data type.
     * @param data Pointer to the data to be store.
     * @param size Size of the array.
     */
    template<typename Type>
    I2CPacket(uint8_t id, Type* data, uint32_t size){
        this->size = size * sizeof(Type);
        this->id = id;
        this->data = (uint8_t*)malloc(this->size);
        memcpy(this->data, data, this->size);
    }

    /**
     * @brief Deletes a I2CPacket instance.
     *
     */
    ~I2CPacket() {
        if (data)
            free(data);
    }

    /**
     * @brief Gets the data size in bytes.
     *
     * @return uint32_t Returns the data size in bytes.
     */
    uint32_t get_size() {
        return size;
    }

    /**
	 * @brief Gets the id.
	 *
	 * @return uint32_t Returns the data size in bytes.
	 */
    uint16_t get_id(){
    	return id;
    }

    /**
     * @brief Get the pointer to the data.
     *
     * @return uint8_t* Returns a pointer to the packet data.
     */
    uint8_t* get_data() {
        return data;
    }

    /**
	 * @brief Set new data to the packet.
	 *
¡	 */
    void set_data(uint8_t* data){
    	this->data = data;
    }

};
