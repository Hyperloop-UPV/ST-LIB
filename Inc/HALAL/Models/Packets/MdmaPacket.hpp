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
    size_t size;
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
        buffer = reinterpret_cast<uint8_t*>(MPUManager::allocate_non_cached_memory(size));
        // buffer = new uint8_t[size];
        if (buffer == nullptr) {
            ErrorHandler("Failed to allocate MDMA buffer for packet");
        }

        MDMA::LinkedListNode* prev_node = nullptr;
        uint32_t offset = 0;
        uint32_t idx = 0;
        std::apply([&](auto&&... args) {
            (([&]() {
                using T = std::remove_pointer_t<decltype(args)>;
                MDMA::LinkedListNode* node = MDMA::link_node_pool.construct(args, buffer + offset);
                build_nodes[idx++] = node;
                offset += sizeof(T);
                if (prev_node != nullptr) {
                    prev_node->set_next(node->get_node());
                }
                prev_node = node;
            }()), ...);
        }, value_pointers);

        prev_node = nullptr;
        offset = 0;
        idx = 0;
        std::apply([&](auto&&... args) {
            (([&]() {
                MDMA::LinkedListNode* node = MDMA::link_node_pool.construct(buffer + offset, args);
                parse_nodes[idx++] = node;
                offset += sizeof(args);
                if (prev_node != nullptr) {
                    prev_node->set_next(node->get_node());
                }
                prev_node = node;
            }()), ...);
        }, value_pointers);

        build_transfer_node = MDMA::link_node_pool.construct(buffer, nullptr,offset); // Used when needed for dynamic destination 
        parse_transfer_node = MDMA::link_node_pool.construct(buffer, buffer,offset); // Used when needed for dynamic origin
    }

    /**
     * @brief Build the packet and transfer data into non-cached buffer using MDMA
     * @param destination_address Optional destination address for the built packet (should be non-cached, else you will need to manage cache coherency)
     * @return Pointer to the built packet data (internal buffer or destination address)
     */
    uint8_t* build(uint8_t* destination_address = nullptr) {
        set_build_destination(destination_address);
        Promise* promise = Promise::inscribe();
        MDMA::transfer_list(0, build_nodes[0], promise);
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
        set_build_destination(destination_address);
        MDMA::transfer_list(0, build_nodes[0], promise);
        return destination_address ? destination_address : buffer;
    }

    // Just for interface compliance
    uint8_t* build() override {
        uint8_t* destination_address = nullptr;
        return build(destination_address);
    }

    void parse(uint8_t* data = nullptr) override {
        Promise* promise = Promise::inscribe();
        auto source_node = set_parse_source(data);
        MDMA::transfer_list(0, source_node, promise);
        promise->wait();
    }

    void parse(Promise* promise, uint8_t* data = nullptr) {
        auto source_node = set_parse_source(data);
        MDMA::transfer_list(0, source_node, promise);
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

   private:
    MDMA::LinkedListNode* build_transfer_node; // Node used for destination address
    MDMA::LinkedListNode* parse_transfer_node; // Node used for source address
    MDMA::LinkedListNode* build_nodes[sizeof...(Types) + 1];
    MDMA::LinkedListNode* parse_nodes[sizeof...(Types) + 1];

    void set_build_destination(uint8_t* external_buffer) {
        if (external_buffer != nullptr) {
            build_transfer_node->set_destination(external_buffer);
            build_nodes[sizeof...(Types)]->set_next(build_transfer_node->get_node());
        } else {
            build_nodes[sizeof...(Types)]->set_next(nullptr);
        }
    }

    MDMA::LinkedListNode* set_parse_source(uint8_t* external_buffer) {
        if (external_buffer != nullptr) {
            parse_transfer_node->set_source(external_buffer);
            parse_transfer_node->set_next(parse_nodes[0]->get_node());
            return parse_transfer_node;
        } else {
            parse_nodes[0]->set_next(nullptr);
            return parse_nodes[0];
        }
    }
};

#endif // MDMA_PACKET_HPP