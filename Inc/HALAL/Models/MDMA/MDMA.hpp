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
#define NODES_MAX 100
#endif

#ifndef TRANSFER_QUEUE_MAX_SIZE
#define TRANSFER_QUEUE_MAX_SIZE 50
#endif
class MDMA{
    public:
    /**
     * @brief A helper struct to create and manage MDMA linked list nodes.
     */
    struct LinkedListNode {
    template<typename T>
    LinkedListNode(T* source_ptr, void* dest_ptr) {
        init_node(source_ptr, dest_ptr, sizeof(T));
    }

    template<typename T>
    LinkedListNode(T* source_ptr, void* dest_ptr, size_t size) {
        init_node(source_ptr, dest_ptr, size);
    }

    void set_next(MDMA_LinkNodeTypeDef* next_node) { node.CLAR = reinterpret_cast<uint32_t>(next_node); }
    void set_destination(void* destination) { node.CDAR = reinterpret_cast<uint32_t>(destination); }
    void set_source(void* source) { node.CSAR = reinterpret_cast<uint32_t>(source); }
    auto get_node() -> MDMA_LinkNodeTypeDef* { return &node; }
    auto get_size() -> uint32_t { return node.CBNDTR; }
    auto get_destination() -> uint32_t { return node.CDAR; }
    auto get_source() -> uint32_t { return node.CSAR; }
    auto get_next() -> MDMA_LinkNodeTypeDef* { return reinterpret_cast<MDMA_LinkNodeTypeDef*>(node.CLAR); }

private:
    MDMA_LinkNodeTypeDef node;

    void init_node(void* src, void* dst, size_t size) {
        MDMA_LinkNodeConfTypeDef nodeConfig{};
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
        nodeConfig.SrcAddress = reinterpret_cast<uint32_t>(src);
        nodeConfig.DstAddress = reinterpret_cast<uint32_t>(dst);
        nodeConfig.BlockDataLength = static_cast<uint32_t>(size);
        nodeConfig.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
        nodeConfig.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
        nodeConfig.Init.SourceInc = MDMA_SRC_INC_BYTE;
        nodeConfig.Init.DestinationInc = MDMA_DEST_INC_BYTE;

        if (HAL_MDMA_LinkedList_CreateNode(&node, &nodeConfig) != HAL_OK) {
            ErrorHandler("Error creating linked list in MDMA");
        }
    }
};

    private:
    struct Instance{
    public:
        MDMA_HandleTypeDef handle;
        uint8_t id;
        bool* done;
        MDMA_LinkNodeTypeDef transfer_node;

        Instance()
            : handle{}
            , id(0U)
            , done(nullptr)
            , transfer_node{}
        {}

        Instance(MDMA_HandleTypeDef handle_,
                 uint8_t id_,
                 bool* done_,
                 MDMA_LinkNodeTypeDef transfer_node_)
            : handle(handle_)
            , id(id_)
            , done(done_)
            , transfer_node(transfer_node_)
        {}


    };
    static void prepare_transfer(Instance& instance, MDMA::LinkedListNode* first_node);
    static void prepare_transfer(Instance& instance, MDMA_LinkNodeTypeDef* first_node);
    static Instance& get_instance(uint8_t id);
    inline static std::array<Instance,8> instances{};
    static std::unordered_map<uint8_t, MDMA_Channel_TypeDef*> instance_to_channel;
    static std::unordered_map<MDMA_Channel_TypeDef*, uint8_t> channel_to_instance;
    static std::bitset<8> instance_free_map;
    inline static Stack<std::pair<MDMA::LinkedListNode*,bool*>,50> transfer_queue{};

    static void TransferCompleteCallback(MDMA_HandleTypeDef *hmdma);
    static void TransferErrorCallback(MDMA_HandleTypeDef *hmdma);

    /**
	 * @brief A method to add MDMA channels in linked list mode.

	 * This method will be called internally for each channel during the MDMA::start() process.
	 *
     * @param instance	The reference to the MDMA instance to be initialised	 
	 */

    static void inscribe(Instance& instance,uint8_t id);

    public:

    
    

    // Pool for MDMA_LinkNodeTypeDef, uses external non-cached memory
    static Pool<LinkedListNode, NODES_MAX, true> link_node_pool;
    //To be reviewed when we make mdma in compile time

    static void start();
    static void irq_handler();
    static void update();


    

    /**
     * @brief A method to start a transfer from source to destination using MDMA linked list
     *  
     * @param source_address The source address for the transfer.
     * @param destination_address The destination address for the transfer.
     * @param data_length The length of data to be transferred.
     * @param check A reference boolean that will be set to true if the transfer was successfully started, false otherwise.
     */
    static void transfer_data(uint8_t* source_address, uint8_t* destination_address, const uint32_t data_length, bool* done=nullptr);

    /**
     * @brief A method to transfer using MDMA linked 
     *  
     * @param first_node The linked list node representing the first node in the linked list.
     * @param check A reference boolean that will be set to true if the transfer was successfully queued, false otherwise.
     */
    static void transfer_list(MDMA::LinkedListNode* first_node,bool* check=nullptr);

};