/*
 * MdmaPacket.hpp
 *
 * Created on: 06 nov. 2025
 *         Author: Boris
 */

#ifndef MDMA_PACKET_HPP
#define MDMA_PACKET_HPP

#include "HALAL/Models/Packets/Packet.hpp"


/** 
 * @brief Packet that uses MDMA to transfer its data 
 * @note Non-atomic write operations may lead to corrupted / half-updated data if variables change during transfer
 */
template<size_t BufferLength, class... Types>
class MdmaPacket : public StackPacket<BufferLength, Types...> {
public:
    using Base = StackPacket<BufferLength, Types...>;
    
    MdmaPacket(uint16_t id, Types*... values)
        : Base(id, values...) {
        if constexpr (BufferLength == 0) {
            // Cache variable sizes to detect changes later
            // (TODO)
        }

        // Create descriptor linked list in MDMA
        // (TODO)
    }

    /**
     * Change buffer pointer to build data into
     */
    void set_buffer(uint8_t* new_buffer) {
        this->buffer = new_buffer;
    }
    
    /**
     * @brief Build the packet data into internal buffer
     */
    uint8_t* build() override {
        return this->build(this->buffer);  // Calls build(uint8_t*)
    }

    /**
     * @brief Build the packet data into provided buffer using MDMA
     */
    uint8_t* build(uint8_t* buffer) {
        if constexpr (BufferLength == 0) {
            // Check if any variable size has changed and update descriptors
            // (TODO)
        }

        // Trigger MDMA transfer
        // (TODO)

        // Fallback until MDMA is implemented
        return Base::build();
    }
    
private:
    // MDMA descriptor management
    // (TODO)
};

#if __cpp_deduction_guides >= 201606
template<class... Types>
MdmaPacket(uint16_t, Types*... values)
    -> MdmaPacket<(!has_container<Types...>::value)*total_sizeof<Types...>::value, Types...>;
#endif

#endif // MDMA_PACKET_HPP