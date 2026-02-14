/*
 * SpiPacket.hpp
 *
 *  Created on: 15 nov. 2023
 *      Author: ricardo
 */

#pragma once

#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Models/Packets/PacketValue.hpp"
#include "HALAL/Models/DataStructures/StackTuple.hpp"

/**
 * @brief Base structure of SPIPacket used to store them without need of knowing its template
 * definition
 */
class SPIBasePacket {
public:
    uint8_t* buffer; /**< pointer for the variables casted into binary data*/
    size_t size;     /**<size of the buffer*/

    /**
     * @brief function that receives binary data and uses it to update the variables
     */
    virtual void parse(uint8_t* data) = 0;

    /**
     * @brief function that transforms the variables into binary data and saves it on the buffer
     */
    virtual uint8_t* build() = 0;
};

/**
 * @brief template class used as a structure to link variables of any type to one or more
 * SPIPackets.
 */
template <size_t BufferLength, class... Types> class SPIPacket : public SPIBasePacket {
public:
    PacketValue<>* values[sizeof...(Types)];
    uint8_t buffer[BufferLength];
    stack_tuple<PacketValue<Types>...> packetvalue_warehouse;
    size_t& size = SPIBasePacket::size;
    SPIPacket(Types*... values) : packetvalue_warehouse{PacketValue<Types>(values)...} {
        int i = 0;
        packetvalue_warehouse.for_each([this, &i](auto& value) { this->values[i++] = &value; });
        size = BufferLength;
        SPIBasePacket::buffer = buffer;
    }

    void parse(uint8_t* data) override {
        for (PacketValue<>* value : values) {
            value->parse(data);
            data += value->get_size();
        }
    }

    uint8_t* build() override {
        uint8_t* data = buffer;
        for (PacketValue<>* value : values) {
            value->copy_to(data);
            data += value->get_size();
        }
        return buffer;
    }
};

template <> class SPIPacket<0> : public SPIBasePacket {
public:
    size_t& size = SPIBasePacket::size;
    SPIPacket() { size = 0; }

    void parse(uint8_t* data) override {}

    uint8_t* build() override { return nullptr; }
};
