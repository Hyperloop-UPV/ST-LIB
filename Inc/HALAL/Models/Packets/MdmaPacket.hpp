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
#include "HALAL/Models/MPU.hpp"
#include "C++Utilities/CppImports.hpp"

#ifndef MDMA_PACKET_MAX_INSTANCES
#define MDMA_PACKET_MAX_INSTANCES 50
#endif

struct MdmaPacketDomain {
    struct Entry {
        size_t packet_mpu_index;
        size_t nodes_mpu_index;
    };

    struct Config {
        size_t packet_mpu_index;
        size_t nodes_mpu_index;
    };

    struct Instance {
        uint8_t* packet_buffer;
        MDMA::LinkedListNode* nodes;
    };

    static constexpr size_t max_instances = MDMA_PACKET_MAX_INSTANCES;

    template <std::size_t N>
    static consteval std::array<Config, N> build(std::span<const Entry> entries) {
        std::array<Config, N> cfgs{};
        for (std::size_t i = 0; i < N; i++) {
            cfgs[i].packet_mpu_index = entries[i].packet_mpu_index;
            cfgs[i].nodes_mpu_index = entries[i].nodes_mpu_index;
        }
        return cfgs;
    }

    template <std::size_t N>
    struct Init {
        static inline std::array<Instance, N> instances{};

        template <std::size_t MPU_N>
        static void init(const std::array<Config, N>& cfgs, std::array<MPUDomain::Instance, MPU_N>& mpu_instances) {
            for (std::size_t i = 0; i < N; i++) {
                instances[i].packet_buffer = static_cast<uint8_t*>(mpu_instances[cfgs[i].packet_mpu_index].ptr);
                instances[i].nodes = static_cast<MDMA::LinkedListNode*>(mpu_instances[cfgs[i].nodes_mpu_index].ptr);
            }
        }
    };

    /**
    * @brief A Packet class that uses MDMA for building and parsing packets.
    * @tparam Types The types of the values in the packet.
    * @note It uses non-cached memory for MDMA operations.
    */
    class MdmaPacketBase : public Packet {
    public:
        virtual uint8_t* build(bool* done, uint8_t* destination_address = nullptr) = 0;
        using Packet::build;
    };

    template<class... Types>
    class MdmaPacket : public MdmaPacketBase {
    public:
        uint16_t id;
        uint8_t* buffer;
        size_t size;
        std::tuple<uint16_t*, Types*...> value_pointers;

        MdmaPacket(const MdmaPacket&) = delete;
        MdmaPacket& operator=(const MdmaPacket&) = delete;

        struct Request {
            using domain = MdmaPacketDomain;

            static constexpr size_t packet_size_bytes = (sizeof(Types) + ...) + sizeof(uint16_t);
            static constexpr size_t nodes_size = (2 * (sizeof...(Types) + 1) + 2);

            using PacketMem = std::array<uint8_t, packet_size_bytes>;
            using NodesMem = std::array<MDMA::LinkedListNode, nodes_size>;

            MPUDomain::Buffer<PacketMem> packet_req;
            MPUDomain::Buffer<NodesMem> nodes_req;

            consteval Request() : 
                packet_req(MPUDomain::MemoryType::NonCached, MPUDomain::MemoryDomain::D1),
                nodes_req(MPUDomain::MemoryType::NonCached, MPUDomain::MemoryDomain::D1)
            {}

            template <class Ctx>
            consteval void inscribe(Ctx &ctx) const {
                size_t p_idx = packet_req.inscribe(ctx);
                size_t n_idx = nodes_req.inscribe(ctx);
                ctx.template add<MdmaPacketDomain>({p_idx, n_idx}, this);
            }
        };

        MdmaPacket(Instance& instance, uint16_t id, Types*... values) 
            : id(id), size((sizeof(Types) + ...) + sizeof(uint16_t)) , value_pointers(&this->id, values...) {
            
            packets[this->id] = this;
            this->buffer = instance.packet_buffer;
            MDMA::LinkedListNode* nodes = instance.nodes;

            MDMA::LinkedListNode* prev_node = nullptr;
            uint32_t offset = 0;
            uint32_t idx = 0;
            size_t node_idx = 0;

            std::apply([&](auto&&... args) { (([&]() {
                    using PointerType = std::decay_t<decltype(args)>;
                    using UnderlyingType = std::remove_pointer_t<PointerType>;

                    constexpr size_t type_size = sizeof(UnderlyingType);
                    MDMA::LinkedListNode* node = new (&nodes[node_idx++]) MDMA::LinkedListNode(args, buffer + offset,type_size); // Placement new
                    build_nodes[idx++] = node;
                    offset += type_size;

                    if (prev_node != nullptr) {
                        prev_node->set_next(node->get_node());
                    }
                    prev_node = node;
            }()), ...); }, value_pointers);

            prev_node = nullptr;
            offset = 0;
            idx = 0;

            std::apply([&](auto&&... args) { (([&]() {
                using PointerType = std::decay_t<decltype(args)>;
                using UnderlyingType = std::remove_pointer_t<PointerType>;

                constexpr size_t type_size = sizeof(UnderlyingType);
                MDMA::LinkedListNode* node = new (&nodes[node_idx++]) MDMA::LinkedListNode(buffer + offset, args, type_size); // Placement new
                parse_nodes[idx++] = node;
                offset += type_size;

                if (prev_node != nullptr) {
                    prev_node->set_next(node->get_node());
                }
                prev_node = node;
            }()), ...); }, value_pointers);

            build_transfer_node = new (&nodes[node_idx++]) MDMA::LinkedListNode(buffer, nullptr, size); 
            parse_transfer_node = new (&nodes[node_idx++]) MDMA::LinkedListNode(buffer, buffer, size); 
        }

        /**
        * @brief Build the packet and transfer data into non-cached buffer using MDMA
        * @param destination_address Optional destination address for the built packet (should be non-cached, else you will need to manage cache coherency). It isn't optional here because there's a specific overload without parameters for compliance with Packet interface.
        * @return Pointer to the built packet data (internal buffer or destination address)
        */
        uint8_t* build(uint8_t* destination_address) {
            set_build_destination(destination_address);
            bool done = false;
            MDMA::transfer_list(build_nodes[0], &done);
            while (!done) {
                MDMA::update();
            }
            return destination_address ? destination_address : buffer;
        }

        /**
        * @brief Build the packet and transfer data into non-cached buffer using MDMA with a promise
        * @param done Promise to be fulfilled upon transfer completion
        * @param destination_address Optional destination address for the built packet (should be non-cached, else you will need to manage cache coherency)
        * @return Pointer to the built packet data (internal buffer or destination address)
        */
        uint8_t* build(bool* done, uint8_t* destination_address = nullptr) {
            set_build_destination(destination_address);
            MDMA::transfer_list(build_nodes[0], done);
            return destination_address ? destination_address : buffer;
        }

        // Just for interface compliance
        uint8_t* build() override {
            uint8_t* destination_address = nullptr;
            return build(destination_address);
        }

        /**
         * @brief Parse the packet data from non-cached buffer using MDMA
         * @param data Optional source data address to parse from (should be non-cached, else you will need to manage cache coherency). It isn't optional here becasue there's a specific overload without parameters for compliance with Packet interface.
         * @param done Optional pointer to a boolean that will be set to true when parsing is done.
         */
        void parse(uint8_t* data) override {
            bool done = false;
            auto source_node = set_parse_source(data);
            MDMA::transfer_list(source_node, &done);
            while (!done) {
                MDMA::update();
            }
        }

        /**
         * @brief Parse the packet data from non-cached buffer using MDMA with a promise
         * @param done Pointer to a boolean that will be set to true when parsing is done.
         * @param data Optional source data address to parse from (should be non-cached, else you will need to manage cache coherency).
         */
        void parse(bool* done, uint8_t* data = nullptr) {
            auto source_node = set_parse_source(data);
            MDMA::transfer_list(source_node, done);
        }

        size_t get_size() override {
            return size;
        }

        uint16_t get_id() override {
            return id;
        }

        // Just for interface compliance, this is not efficient for MdmaPacket as it is.
        // Could be optimized by using a map of index to pointer or similar structure created at compile time, but doesn't seem worthy now.
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
};

#endif // MDMA_PACKET_HPP