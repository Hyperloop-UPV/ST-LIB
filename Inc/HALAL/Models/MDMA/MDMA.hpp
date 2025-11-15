#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "stm32h7xx_hal.h"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Utils/Promise.hpp"

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
        Instance() = default;
        Instance(MDMA_HandleTypeDef handle, uint8_t id, uint8_t* data_buffer, uint8_t* destination_address,MDMA_LinkNodeTypeDef transfer_node): handle(handle), id(id), data_buffer(data_buffer), destination_address(destination_address), transfer_node(transfer_node) {}


    };
    inline static std::unordered_map<uint8_t, std::vector<MDMA_LinkNodeTypeDef>> linked_lists{};
    inline static std::unordered_map<uint8_t, Instance> instances{};
    static std::unordered_map<uint8_t, uint32_t> dst_size_to_flags;
    static std::unordered_map<uint8_t, uint32_t> src_size_to_flags;
    static std::unordered_map<uint8_t, MDMA_Channel_TypeDef*> instance_to_channel;
    static std::unordered_map<MDMA_Channel_TypeDef*, uint8_t> channel_to_instance;

    inline static uint8_t number_of_packets{0};

    static void TransferCompleteCallback(MDMA_HandleTypeDef *hmdma);

    public:

    static void start();


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
     * @param base_packet_id The ID of the base packet to which other packets will be merged.
     * @param packets_id The IDs of the packets to be merged into the base packet.
     * 
     * @return The ID of the merged linked list.
     */
    template<typename... PacketIds>
    static uint8_t merge_packets(const uint8_t base_packet_id, const PacketIds... packets_id);

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

