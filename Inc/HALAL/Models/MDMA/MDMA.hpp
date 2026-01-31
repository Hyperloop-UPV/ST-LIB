#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "stm32h7xx_hal.h"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Models/MPUManager/MPUManager.hpp"
#include "HALAL/Models/MPU.hpp"
#include <array>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <utility>



#ifdef MDMA
#undef MDMA
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
    LinkedListNode(T* source_ptr, void* dest_ptr) 
    {
        init_node(source_ptr, dest_ptr, sizeof(T));
    }

    template<typename T>
    LinkedListNode(T* source_ptr, void* dest_ptr, size_t size) 
    {
        init_node(source_ptr, dest_ptr, size);
    }

    void set_next(MDMA_LinkNodeTypeDef* next_node) { node.CLAR = reinterpret_cast<uint32_t>(next_node); }
    void set_destination(void* destination) 
    { 
        uint32_t destination_address = reinterpret_cast<uint32_t>(destination);
        node.CDAR = destination_address;
        
        // TCM memories are accessed by AHBS bus
        if ((destination_address < 0x00010000) || (destination_address >= 0x20000000 && destination_address < 0x20020000)) {
            SET_BIT(node.CTBR, MDMA_CTBR_DBUS);
        } else {
            CLEAR_BIT(node.CTBR, MDMA_CTBR_DBUS);
        }
    }
    void set_source(void* source) { 
        uint32_t source_address = reinterpret_cast<uint32_t>(source); 
        node.CSAR = source_address;
    
        // TCM memories are accessed by AHBS bus
        if ((source_address < 0x00010000) || (source_address >= 0x20000000 && source_address < 0x20020000)) {
            SET_BIT(node.CTBR, MDMA_CTBR_SBUS);
        } else {
            CLEAR_BIT(node.CTBR, MDMA_CTBR_SBUS);
        }
    }
    auto get_node() -> MDMA_LinkNodeTypeDef* { return &node; }
    auto get_size() -> uint32_t { return node.CBNDTR; }
    auto get_destination() -> uint32_t { return node.CDAR; }
    auto get_source() -> uint32_t { return node.CSAR; }
    auto get_next() -> MDMA_LinkNodeTypeDef* { return reinterpret_cast<MDMA_LinkNodeTypeDef*>(node.CLAR); }

private:
    alignas(8) MDMA_LinkNodeTypeDef node;

    void init_node(void* src, void* dst, size_t size) 
    {
        MDMA_LinkNodeConfTypeDef nodeConfig{};
        nodeConfig.Init.DataAlignment = MDMA_DATAALIGN_RIGHT; 
        nodeConfig.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
        nodeConfig.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
        nodeConfig.Init.BufferTransferLength = 128;
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
        
        uint32_t source_data_size;
        uint32_t dest_data_size;
        uint32_t source_inc;
        uint32_t dest_inc;

        size_t effective_size = size;
        if (effective_size == 2 && ((reinterpret_cast<uintptr_t>(src) | reinterpret_cast<uintptr_t>(dst)) & 1)) effective_size = 1; // Odd address, so fallback to byte-wise
        else if (effective_size == 4 && ((reinterpret_cast<uintptr_t>(src) | reinterpret_cast<uintptr_t>(dst)) & 3)) effective_size = 1; // Not word-aligned, so fallback to byte-wise
        else if (effective_size == 8 && ((reinterpret_cast<uintptr_t>(src) | reinterpret_cast<uintptr_t>(dst)) & 7)) effective_size = 1; // Not doubleword-aligned, so fallback to byte-wise

        switch(static_cast<uint32_t>(effective_size)) {
            case 2:
                source_data_size = MDMA_SRC_DATASIZE_HALFWORD;
                dest_data_size = MDMA_DEST_DATASIZE_HALFWORD;
                source_inc = MDMA_SRC_INC_HALFWORD;
                dest_inc = MDMA_DEST_INC_HALFWORD;
                break;
            case 4:
                source_data_size = MDMA_SRC_DATASIZE_WORD;
                dest_data_size = MDMA_DEST_DATASIZE_WORD;
                source_inc = MDMA_SRC_INC_WORD;
                dest_inc = MDMA_DEST_INC_WORD;
                break;
            case 8:
                source_data_size = MDMA_SRC_DATASIZE_DOUBLEWORD;
                dest_data_size = MDMA_DEST_DATASIZE_DOUBLEWORD;
                source_inc = MDMA_SRC_INC_DOUBLEWORD;
                dest_inc = MDMA_DEST_INC_DOUBLEWORD;
                break;
            default:
                source_data_size = MDMA_SRC_DATASIZE_BYTE;
                dest_data_size = MDMA_DEST_DATASIZE_BYTE;
                source_inc = MDMA_SRC_INC_BYTE;
                dest_inc = MDMA_DEST_INC_BYTE;
                break;
        }

        nodeConfig.Init.SourceDataSize = source_data_size;
        nodeConfig.Init.DestDataSize = dest_data_size;
        nodeConfig.Init.SourceInc = source_inc;
        nodeConfig.Init.DestinationInc = dest_inc;

        if (HAL_MDMA_LinkedList_CreateNode(&node, &nodeConfig) != HAL_OK) {
            ErrorHandler("Error creating linked list in MDMA");
        }
    }
};

    private:
    static D1_NC MDMA_LinkNodeTypeDef internal_nodes[8];

    struct Instance{
    public:
        MDMA_HandleTypeDef handle;
        uint8_t id;
        volatile bool* done;
        MDMA_LinkNodeTypeDef* transfer_node;

        Instance()
            : handle{}
            , id(0U)
            , done(nullptr)
            , transfer_node(nullptr)
        {}

        Instance(MDMA_HandleTypeDef handle_,
                 uint8_t id_,
                 volatile bool* done_,
                 MDMA_LinkNodeTypeDef* transfer_node_)
            : handle(handle_)
            , id(id_)
            , done(done_)
            , transfer_node(transfer_node_)
        {}


    };
    static void prepare_transfer(Instance& instance, MDMA::LinkedListNode* first_node);
    static void prepare_transfer(Instance& instance, MDMA_LinkNodeTypeDef* first_node);
    static Instance& get_instance(uint8_t id);
    static MDMA_Channel_TypeDef* get_channel(uint8_t id);
    static uint8_t get_instance_id(MDMA_Channel_TypeDef* channel);

    inline static std::array<Instance,8> instances{};
    static std::bitset<8> instance_free_map;
    inline static Stack<std::pair<MDMA::LinkedListNode*,volatile bool*>,50> transfer_queue{};

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

    static void start();
    static void irq_handler();
    static void update();


    

    /**
     * @brief A method to start a transfer from source to destination using MDMA linked list
     *  
     * @param source_address The source address for the transfer.
     * @param destination_address The destination address for the transfer.
     * @param data_length The length of data to be transferred.
     * @param check A reference boolean that will be set to true if the transfer was successfully done, false otherwise.
     */
    static void transfer_data(uint8_t* source_address, uint8_t* destination_address, const uint32_t data_length,volatile bool* done=nullptr);

    /**
     * @brief A method to transfer using MDMA linked 
     *  
     * @param first_node The linked list node representing the first node in the linked list.
     * @param check A reference boolean that will be set to true if the transfer was successfully done, false otherwise.
     */
    static void transfer_list(MDMA::LinkedListNode* first_node,volatile bool* check=nullptr);

};
