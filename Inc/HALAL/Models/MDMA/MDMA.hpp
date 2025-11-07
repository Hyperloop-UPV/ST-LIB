#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "stm32h7xx_hal.h"
#include "main.h"

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
        Instance() = default;
        Instance(MDMA_HandleTypeDef handle, uint8_t id, uint8_t* data_buffer): handle(handle), id(id), data_buffer(data_buffer) {}


    };
    inline static std::unordered_map<uint8_t, std::vector<MDMA_LinkNodeTypeDef>> linked_lists{};
    inline static std::unordered_map<uint8_t, Instance> instances{};
    inline static uint8_t number_of_packets{0};
    static std::unordered_map<uint8_t, uint32_t> dst_size_to_flags;
    static std::unordered_map<uint8_t, uint32_t> src_size_to_flags;
    inline static MDMA_LinkNodeTypeDef transfer_node = {};

    static void start();


    public:

    /**
	 * @brief A method to add MDMA channels in linked list mode.

	 * This method has to be invoked before the ST-LIB::start()
	 *
	 * @param data_buffer	the buffer where the MDMA will write the data, must be a non-cached buffer
	 *
	 * @return the id that represents the MDMA channel with its designated buffer inside this utility class, used in all its functions.
	 */

    static uint8_t inscribe(uint8_t* data_buffer);

    template<typename... pointers>
    static uint8_t add_packet(const uint8_t MDMA_id,const std::tuple<pointers...>& values);

    static uint8_t merge_packets(const uint8_t packet_id1, const uint8_t packet_id2);

    static void transfer_data(const uint8_t MDMA_id,uint8_t* source_address,uint8_t* destination_address, const uint32_t data_length);

    static void transfer_packet(const uint8_t MDMA_id, const uint8_t packet_id,uint8_t* destination_address);


};