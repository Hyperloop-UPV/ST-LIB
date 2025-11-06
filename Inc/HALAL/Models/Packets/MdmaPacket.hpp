/*
 * MdmaPacket.hpp
 *
 * Created on: 06 nov. 2025
 *         Author: Boris
 */

#ifndef MDMA_PACKET_HPP
#define MDMA_PACKET_HPP

#include "HALAL/Models/Packets/Packet.hpp"

class Mdma; // Placeholder, remove later

/** 
 * @brief Packet that uses MDMA to transfer its data 
 * @note Non-atomic write operations may lead to corrupted / half-updated data if variables change during transfer
 * @note Don't use with containers of variable size unless sizes are fixed
 */
template<size_t BufferLength, class... Types>
class MdmaPacket : public StackPacket<BufferLength, Types...> {
public:
    static_assert(!has_container<Types...>::value, "MdmaPacket does not support containers");
    using Base = StackPacket<BufferLength, Types...>;

    MdmaPacket(uint16_t id, Types*... values) = delete;

    MdmaPacket(uint16_t id, Mdma& mdma, Types*... values) : Base(id, values...), mdma(mdma) {

        // Inscribe MDMA list and save id or handle or whatever of the list
        // (TODO)
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
    uint8_t* build(uint8_t* buffer, Mdma& mdma = nullptr) {
        // Trigger MDMA transfer with the provided MDMA instance or the one stored in the packet (default channel)
        // (TODO)

        // Fallback until MDMA is implemented
        return Base::build();
    }
    
private:
    Mdma& mdma;
    // MDMA descriptor management
    // (TODO)
};

#if __cpp_deduction_guides >= 201606
template<class... Types>
MdmaPacket(uint16_t, Mdma*, Types*... values)
    -> MdmaPacket<total_sizeof<Types...>::value, Types...>;

#endif

#endif // MDMA_PACKET_HPP