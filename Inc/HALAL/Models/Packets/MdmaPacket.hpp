/*
 * MdmaPacket.hpp
 *
 * Created on: 06 nov. 2025
 *         Author: Boris
 */

#ifndef MDMA_PACKET_HPP
#define MDMA_PACKET_HPP

#include "HALAL/Models/Packets/Packet.hpp"
#include "HALAL/Models/MDMA/MDMA.hpp"
#include "HALAL/Models/MPUManager/MPUManager.hpp"
#include "HALAL/Utils/Promise.hpp"


/**
 * @brief A Packet class that uses MDMA for building and parsing packets.
 * @tparam Types The types of the values in the packet.
 * @note This class requires MDMA and MPUManager to be properly configured.
 * @note It uses non-cached memory for MDMA operations.
 */
template<class... Types>
class MdmaPacket : public Packet {
   public:
    uint16_t id;
    uint8_t* buffer;
    size_t& size = Packet::size;
    std::tuple<uint16_t*, Types*...> value_pointers;

    /**
     * @brief Constructor for MdmaPacket
     * @param id The packet ID
     * @param values Pointers to the values to be included in the packet
     */
    MdmaPacket(uint16_t id, Types*... values) 
    : id(id), size((sizeof(Types) + ...) + sizeof(uint16_t)) , value_pointers(&this->id, values...) {
        packets[id] = this;
        // Allocate non-cached buffer for MDMA
        buffer = MPUManager::allocate_non_cached_memory(size);
    }

    /**
     * @brief Build the packet and transfer data into non-cached buffer using MDMA
     * @param destination_address Optional destination address for the built packet (should be non-cached, else you will need to manage cache coherency)
     * @return Pointer to the built packet data (internal buffer or destination address)
     */
    uint8_t* build(uint8_t* destination_address = nullptr) {
        Promise* promise = Promise::inscribe();
        // (TODO) Call MDMA to transfer data
        promise->wait();
        return destination_address ? destination_address : buffer;
    }

    /**
     * @brief Build the packet and transfer data into non-cached buffer using MDMA with a promise
     * @param promise Promise to be fulfilled upon transfer completion
     * @param destination_address Optional destination address for the built packet (should be non-cached, else you will need to manage cache coherency)
     * @return Pointer to the built packet data (internal buffer or destination address)
     */
    uint8_t* build(Promise* promise, uint8_t* destination_address = nullptr) {
        // (TODO) Call MDMA to transfer data
        return destination_address ? destination_address : buffer;
    }

    // Just for interface compliance
    uint8_t* build() override {
        return build(nullptr);
    }

    void parse(uint8_t* data = nullptr) override {
        Promise* promise = Promise::inscribe();
        // (TODO) Call MDMA to transfer data
        promise->wait();
    }

    void parse(Promise* promise, uint8_t* data = nullptr) {
        // (TODO) Call MDMA to transfer data
    }

    size_t get_size() override {
        return size;
    }

    uint16_t get_id() override {
        return id;
    }

    // Just for interface compliance, this is not efficient for MdmaPacket as it is (could be optimized by adding more functionality to MDMA)
    // Could be optimized by using a map of index to pointer or similar structure created at compile time.
    void set_pointer(size_t index, void* pointer) override {
        size_t current_idx = 0;

        std::apply([&](auto&&... args) {
            ((current_idx++ == index ? 
                (args = reinterpret_cast<std::remove_reference_t<decltype(args)>>(pointer)) 
                : nullptr), ...);
        }, value_pointers);
    }

};

#endif // MDMA_PACKET_HPP