#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "stm32h7xx_hal.h"

class MDMA{

    private:
    static std::unordered_map<std::vector<MDMA_LinkNodeTypeDef>,uint8_t> linked_lists;
    static std::unordered_map<Instance, uint8_t> instances;
    inline static number_of_packets{0};
    
    class Instance{
        public:
        MDMA_HandleTypeDef handle;
        uint8_t id;
        uint8_t *data_buffer;
        Instance() = default;
        Instance(MDMA_HandleTypeDef handle, uint8_t id, uint8_t* data_buffer): handle(handle), id(id), data_buffer(data_buffer) {}


    };

    const static uint32_t get_flag(const uint8_t size);

    static void start();


    public:

    /**
	 * @brief A method to add MDMA channels in linked list mode.

	 * This method has to be invoked before the ST-LIB::start()
	 *
	 * @param data_buffer	the buffer where the MDMA will write the data, very important to be a non-cached buffer
	 *
	 * @return the id that represents the MDMA channel with its designated buffer inside this utility class, used in all its functions.
	 */

    static uint8_t inscribe(uint8_t* data_buffer);

    template<typename... pointers>
    static uint8_t add_packet(const uint8_t MDMA_id,const std::tuple<pointers...>& values);//Utilizar std::apply y std::dcltype

    static uint8_t merge_packets(const uint8_t packet_id1, const uint8_t packet_id2);


    static void transfer_data(const uint8_t MDMA_id, const uint8_t packet_id);


};