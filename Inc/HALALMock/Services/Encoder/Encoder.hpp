/*
 * Encoder.hpp
 *
 *  Created on: 27 oct. 2022
 *      Author: Pablo
 */

#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALALMock/Models/PinModel/Pin.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"


class Encoder {
   public:
    static uint8_t id_counter;
    //TimePeriferal* is useless and not declared, now is void*
    static map<pair<Pin, Pin>, void*> pin_timer_map;
    static map<uint8_t, pair<Pin, Pin>> registered_encoder;

    /**
     * @brief This method registers a new encoder
     *
     * @param pin1	First pin of the encoder
     * @param pin2 	Second pin of the encoder
     *
     * @retval uint8_t Id of the service
     */
    static uint8_t inscribe(Pin& pin1, Pin& pin2);

    static void start();

    /**
     * @brief Starts the timer of the encoder
     *
     * @param id Id of the encoder
     */
    static void turn_on(uint8_t id);

    /**
     * @brief Stop the timer of the encoder
     *
     * @param id Id of the encoder
     */
    static void turn_off(uint8_t id);

    /**
     * @brief Resets the encoder by setting the CNT register to 0
     *
     * @param id Id of the encoder
     */
    static void reset(uint8_t id);

    /**
     * @brief Get the CNT value of the encoder
     *
     * @param id Id of the encoder
     * @return uint32_t CNT value if the id is valid
     */
    static uint32_t get_counter(uint8_t id);

    /**
     * @brief Get the encoder direction
     *
     * @param id Id
     * @return bool Encoder direction if id is valid
     */
    static bool get_direction(uint8_t id);

    static void init(void* encoder);

    static uint32_t get_initial_counter_value(uint8_t id);
};
