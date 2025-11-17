#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "stm32h7xx_hal.h"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Utils/Promise.hpp"
#include <array>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <utility>

#ifdef MDMA
#undef MDMA
#endif

class MDMA{

    private:
    struct Instance{
        public:
        MDMA_HandleTypeDef handle;
        uint8_t id;
        uint8_t *data_buffer;
        uint8_t* destination_address;
        Promise* promise;
        bool using_promise{false};
        MDMA_LinkNodeTypeDef transfer_node{};
        uint32_t last_destination{0};
        uint32_t last_size{0};
        Instance() = default;
        Instance(MDMA_HandleTypeDef handle, uint8_t id, uint8_t* data_buffer, uint8_t* destination_address,MDMA_LinkNodeTypeDef transfer_node): handle(handle), id(id), data_buffer(data_buffer), destination_address(destination_address), transfer_node(transfer_node) {}


    };
    static void prepare_transfer(Instance& instance, MDMA_LinkNodeTypeDef* first_node);
    inline static std::unordered_map<uint8_t,std::vector<MDMA_LinkNodeTypeDef>> linked_lists{};
    inline static std::unordered_map<uint8_t, Instance> instances{};
    inline static std::unordered_map<uint8_t, uint32_t> packet_sizes{};
    static std::unordered_map<uint8_t, uint32_t> dst_size_to_flags;
    static std::unordered_map<uint8_t, uint32_t> src_size_to_flags;
    static std::unordered_map<uint8_t, MDMA_Channel_TypeDef*> instance_to_channel;
    static std::unordered_map<MDMA_Channel_TypeDef*, uint8_t> channel_to_instance;

    inline static uint8_t number_of_packets{0};

    static void TransferCompleteCallback(MDMA_HandleTypeDef *hmdma);
    static void TransferErrorCallback(MDMA_HandleTypeDef *hmdma);

    public:

    static void start();
    static void irq_handler();


    /**
	 * @brief A method to add MDMA channels in linked list mode.

	 * This method has to be invoked before the ST-LIB::start()
	 *
	 * @param data_buffer	the buffer where the MDMA will write the data, very important to be a non-cached buffer
     * @param destination_address  the address where the MDMA will read the data from, if nullptr it will make it so that the destination varies dinamically
	 *
	 * @return the id that represents the MDMA channel with its designated buffer inside this utility class, used in all its functions.
	 */

    static uint8_t inscribe(uint8_t* data_buffer, uint8_t* destination_address=nullptr);

    /**
	 * @brief A method to create a linked list from a tuple of pointers that come from packets, this linked list are global for all instances.

	 * @param MDMA_id	the id that represents the MDMA channel instance
     * @param values  the tuple of pointers that will be added to the linked list
     * 
	 * @return the id that represents the linked list associated to the packet.
	 */
    template<typename... pointers>
    static uint8_t add_packet(const uint8_t MDMA_id,const std::tuple<pointers...>& values);

    /**
     * @brief A method to merge multiple packets into a single linked list.
     * 
     * @param MDMA_id The ID of the MDMA channel instance.
     * @param base_packet_id The ID of the base packet to which other packets will be merged.
     * @param packets_id The IDs of the packets to be merged into the base packet.
     * 
     * @return The ID of the merged linked list.
     */
    template<typename... PacketIds>
    static uint8_t merge_packets(const uint8_t MDMA_id,const uint8_t base_packet_id, const PacketIds... packets_id);

    /**
     * @brief A method to start a transfer from source to destination using MDMA linked list
     *  
     * @param MDMA_id The ID of the MDMA channel instance.
     * @param packet_id The ID of the linked list packet to be transferred.
     * @param destination_address The destination address for the transfer. If nullptr, the default destination associated with the instance will be used.
     * @param promise An optional promise to be fulfilled upon transfer completion.
     */
    static void transfer_data(const uint8_t MDMA_id,uint8_t* source_address, const uint32_t data_length,uint8_t* destination_address=nullptr,Promise* promise=nullptr);

    /**
     * @brief A method to transfer a packet using MDMA linked list
     *  
     * @param MDMA_id The ID of the MDMA channel instance.
     * @param packet_id The ID of the linked list packet to be transferred.
     * @param destination_address The destination address for the transfer. If nullptr, the default destination associated with the instance will be used.
     * @param promise An optional promise to be fulfilled upon transfer completion.
     */
    static void transfer_packet(const uint8_t MDMA_id, const uint8_t packet_id, uint8_t* destination_address=nullptr, Promise* promise=nullptr);

};

template<typename... pointers>
inline uint8_t MDMA::add_packet(const uint8_t MDMA_id, const std::tuple<pointers...>& values)
{
    Instance& instance = instances[MDMA_id];
    uint32_t offset{0};
    HAL_StatusTypeDef status;
    const uint8_t current_packet_id = number_of_packets++;

    std::vector<MDMA_LinkNodeTypeDef>& nodes = linked_lists[current_packet_id];
    nodes.clear();
    nodes.reserve(sizeof...(pointers) + 5U);

    MDMA_LinkNodeConfTypeDef nodeConfig;
    nodeConfig.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    nodeConfig.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
    nodeConfig.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
    nodeConfig.Init.BufferTransferLength = 1;
    nodeConfig.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
    nodeConfig.Init.SourceBlockAddressOffset = 0;
    nodeConfig.Init.DestBlockAddressOffset = 0;
    nodeConfig.BlockCount = 1;
    nodeConfig.Init.Priority = MDMA_PRIORITY_HIGH;
    nodeConfig.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    nodeConfig.Init.Request = MDMA_REQUEST_SW;

    std::apply([
        &nodes,
        &nodeConfig,
        &offset,
        &instance,
        &status
    ](auto... ptrs)
    {
        auto create_node = [&](auto ptr)
        {
            if (ptr == nullptr)
            {
                ErrorHandler("Nullptr given to MDMA");
            }

            using PointerType = std::decay_t<decltype(ptr)>;
            using UnderlyingType = std::remove_pointer_t<PointerType>;
            constexpr size_t type_size = sizeof(UnderlyingType);

            static_assert(type_size == 1 || type_size == 2 || type_size == 4,
                          "MDMA::add_packet: Passed a variable with type size > 4 ");

            MDMA_LinkNodeTypeDef node = {};
            nodeConfig.SrcAddress = reinterpret_cast<uint32_t>(ptr);
            nodeConfig.DstAddress = reinterpret_cast<uint32_t>(instance.data_buffer) + offset;
            nodeConfig.BlockDataLength = static_cast<uint32_t>(type_size);
            nodeConfig.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
            nodeConfig.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
            nodeConfig.Init.SourceInc = MDMA_SRC_INC_BYTE;
            nodeConfig.Init.DestinationInc = MDMA_DEST_INC_BYTE;

            status = HAL_MDMA_LinkedList_CreateNode(&node, &nodeConfig);
            if (status != HAL_OK)
            {
                ErrorHandler("Error creating linked list in MDMA");
            }

            offset += type_size;
            nodes.push_back(node);
        };

        (create_node(ptrs), ...);
    }, values);

    if (nodes.empty())
    {
        ErrorHandler("Error creating linked list in MDMA");
    }

    MDMA_LinkNodeTypeDef node = {};
    nodeConfig.Init.SourceInc = MDMA_SRC_INC_BYTE;
    nodeConfig.Init.DestinationInc = MDMA_DEST_INC_BYTE;
    nodeConfig.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
    nodeConfig.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
    nodeConfig.BlockDataLength = offset;
    nodeConfig.SrcAddress = reinterpret_cast<uint32_t>(instance.data_buffer);
    nodeConfig.DstAddress = reinterpret_cast<uint32_t>(instance.data_buffer) + offset;

    status = HAL_MDMA_LinkedList_CreateNode(&node, &nodeConfig);
    if (status != HAL_OK)
    {
        ErrorHandler("Error creating linked list in MDMA");
    }

    nodes.push_back(node);

    packet_sizes[current_packet_id] = offset;

    for (size_t i = 0; i + 1 < nodes.size(); ++i)
    {
        nodes[i].CLAR = reinterpret_cast<uint32_t>(&nodes[i + 1]);
    }
    nodes.back().CLAR = 0;

    return current_packet_id;
}

template<typename... PacketIds>
inline uint8_t MDMA::merge_packets(const uint8_t MDMA_id,const uint8_t base_packet_id, const PacketIds... packets_id)
{
    Instance& instance = instances[MDMA_id];    

    uint8_t new_packet_id = number_of_packets++;
    std::vector<MDMA_LinkNodeTypeDef>& merged_nodes = linked_lists[new_packet_id];

    uint32_t offset = 0;

    std::vector<MDMA_LinkNodeTypeDef> base_nodes = linked_lists[base_packet_id]; 
    if (!base_nodes.empty())
    {
        base_nodes.pop_back(); 
        for (const auto& node : base_nodes)
        {
            merged_nodes.push_back(node); 
            offset += node.CBNDTR;
        }
    }

    std::array<uint8_t, sizeof...(packets_id)> packet_ids{static_cast<uint8_t>(packets_id)...};
    for (size_t idx = 0; idx < packet_ids.size(); ++idx)
    {
        uint8_t pid = packet_ids[idx];
        std::vector<MDMA_LinkNodeTypeDef> nodes_to_merge = linked_lists[pid];
        
        if (nodes_to_merge.empty()) continue; 

        nodes_to_merge.pop_back(); 

        for (auto& node : nodes_to_merge)
        {
            
            offset += node.CBNDTR;
            node.CDAR = (uint32_t)instance.data_buffer + offset;

            merged_nodes.push_back(node);
        }
    }

    if (merged_nodes.empty())
    {
        ErrorHandler("Error merging linked list in MDMA");
    }

    MDMA_LinkNodeTypeDef aux_node = {};
    MDMA_LinkNodeConfTypeDef nodeConfig{};
    nodeConfig.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    nodeConfig.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
    nodeConfig.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
    nodeConfig.Init.BufferTransferLength = 1;
    nodeConfig.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
    nodeConfig.Init.SourceBlockAddressOffset = 0;
    nodeConfig.Init.DestBlockAddressOffset = 0;
    nodeConfig.BlockCount = 1;
    nodeConfig.Init.Priority = MDMA_PRIORITY_HIGH;
    nodeConfig.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    nodeConfig.Init.Request = MDMA_REQUEST_SW;
    nodeConfig.Init.SourceInc = MDMA_SRC_INC_BYTE;
    nodeConfig.Init.DestinationInc = MDMA_DEST_INC_BYTE;
    nodeConfig.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
    nodeConfig.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
    nodeConfig.BlockDataLength = 1;
    nodeConfig.SrcAddress = (uint32_t) instance.data_buffer;
    nodeConfig.DstAddress = (uint32_t) instance.data_buffer + offset; 
    
    HAL_MDMA_LinkedList_CreateNode(&aux_node, &nodeConfig);
    merged_nodes.push_back(aux_node); 

    for (size_t i = 0; i < merged_nodes.size() - 1; ++i)
    {
        merged_nodes[i].CLAR = reinterpret_cast<uint32_t>(&merged_nodes[i + 1]);
    }
    merged_nodes.back().CLAR = 0; 

    SCB_CleanDCache_by_Addr((uint32_t*)merged_nodes.data(), sizeof(MDMA_LinkNodeTypeDef) * merged_nodes.size());

    return new_packet_id; 
}