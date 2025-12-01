#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "stm32h7xx_hal.h"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Utils/Promise.hpp"
#include "HALAL/Models/MPUManager/MPUManager.hpp"
#include <array>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <utility>

#ifdef MDMA
#undef MDMA
#endif

#ifndef NODES_MAX
#define NODES_MAX 500
#endif

class MDMA{
    public:
    struct LinkedListNode;

    private:
    struct Instance{
    public:
        MDMA_HandleTypeDef handle;
        uint8_t id;
        uint8_t* data_buffer;
        uint8_t* destination_address;
        Promise* promise;
        bool using_promise;
        MDMA::LinkedListNode transfer_node;

        Instance()
            : handle{}
            , id(0U)
            , data_buffer(nullptr)
            , destination_address(nullptr)
            , promise(nullptr)
            , using_promise(false)
            , transfer_node{}
        {}

        Instance(MDMA_HandleTypeDef handle_,
                 uint8_t id_,
                 uint8_t* data_buffer_,
                 uint8_t* destination_address_,
                 MDMA_LinkNodeTypeDef transfer_node_)
            : handle(handle_)
            , id(id_)
            , data_buffer(data_buffer_)
            , destination_address(destination_address_)
            , promise(nullptr)
            , using_promise(false)
            , transfer_node(transfer_node_)
        {}


    };
    static void prepare_transfer(Instance& instance, MDMA::LinkedListNode& first_node);
    static Instance& get_instance(uint8_t id);
    inline static std::array<Instance,8> instances{};
    static std::unordered_map<uint8_t, MDMA_Channel_TypeDef*> instance_to_channel;
    static std::unordered_map<MDMA_Channel_TypeDef*, uint8_t> channel_to_instance;

    static void TransferCompleteCallback(MDMA_HandleTypeDef *hmdma);
    static void TransferErrorCallback(MDMA_HandleTypeDef *hmdma);

    public:

    
    /**
     * @brief A helper struct to create and manage MDMA linked list nodes.
     */
    struct LinkedListNode {
        /**
         * @brief Constructor to create an MDMA linked list node.
         * @tparam T The type of the data to be transferred.
         * @param source_ptr Pointer to the source data.
         * @param dest_ptr Pointer to the destination data.
         */
        template<typename T>
        LinkedListNode(T* source_ptr, void* dest_ptr) {
            MDMA_LinkNodeConfTypeDef nodeConfig;
            nodeConfig.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
            nodeConfig.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
            nodeConfig.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
            nodeConfig.Init.BufferTransferLength = 1;
            nodeConfig.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
            nodeConfig.Init.SourceBlockAddressOffset = 0;
            nodeConfig.Init.DestBlockAddressOffset = 0;
            nodeConfig.BlockCount = 1;
            nodeConfig.Init.Priority = MDMA_PRIORITY_VERY_HIGH;
            nodeConfig.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
            nodeConfig.Init.Request = MDMA_REQUEST_SW;

            this->node = {};
            nodeConfig.SrcAddress = reinterpret_cast<uint32_t>(source_ptr);
            nodeConfig.DstAddress = reinterpret_cast<uint32_t>(dest_ptr);
            nodeConfig.BlockDataLength = static_cast<uint32_t>(sizeof(T));
            nodeConfig.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
            nodeConfig.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
            nodeConfig.Init.SourceInc = MDMA_SRC_INC_BYTE;
            nodeConfig.Init.DestinationInc = MDMA_DEST_INC_BYTE;

            auto status = HAL_MDMA_LinkedList_CreateNode(&node, &nodeConfig);
            if (status != HAL_OK) {
                ErrorHandler("Error creating linked list in MDMA");
            }
        }

        /**
         * @brief Set the next node in the linked list.
         */
        void set_next(MDMA_LinkNodeTypeDef* next_node) { node.CLAR = reinterpret_cast<uint32_t>(next_node); }
        auto get_node() -> MDMA_LinkNodeTypeDef* { return &node; }
        auto get_size() -> uint32_t { return node.CBNDTR; }
        auto get_destination() -> uint32_t { return node.CDAR; }
        auto get_source() -> uint32_t { return node.CSAR; }
        auto get_next() -> MDMA_LinkNodeTypeDef* { return reinterpret_cast<MDMA_LinkNodeTypeDef*>(node.CLAR); }

    private:
        MDMA_LinkNodeTypeDef node;
    };

    // Pool for MDMA_LinkNodeTypeDef, uses external non-cached memory
    static Pool<LinkedListNode, NODES_MAX, true> link_node_pool;

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
     * @brief A method to start a transfer from source to destination using MDMA linked list
     *  
     * @param MDMA_id The ID of the MDMA channel instance.
     * @param packet_id The ID of the linked list packet to be transferred.
     * @param destination_address The destination address for the transfer. If nullptr, the default destination associated with the instance will be used.
     * @param promise An optional promise to be fulfilled upon transfer completion.
     */
    static void transfer_data(const uint8_t MDMA_id,uint8_t* source_address, const uint32_t data_length,uint8_t* destination_address=nullptr, Promise* promise=nullptr);

    /**
     * @brief A method to transfer using MDMA linked 
     *  
     * @param MDMA_id The ID of the MDMA channel instance.
     * @param first_node The linked list node representing the first node in the linked list.
     * @param promise An optional promise to be fulfilled upon transfer completion.
     */
    static void transfer_list(const uint8_t MDMA_id, MDMA::LinkedListNode& first_node, Promise* promise=nullptr);

};