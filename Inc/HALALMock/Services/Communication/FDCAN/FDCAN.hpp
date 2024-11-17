#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALALMock/Models/PinModel/Pin.hpp"
#include <netinet/in.h>

using std::queue;
using std::unordered_map;
using std::vector;

class FDCAN{
public:
	enum DLC : uint32_t{
		BYTES_0 = FDCAN_DLC_BYTES_0,
		BYTES_1 = FDCAN_DLC_BYTES_1,
		BYTES_2 = FDCAN_DLC_BYTES_2,
		BYTES_3 = FDCAN_DLC_BYTES_3,
		BYTES_4 = FDCAN_DLC_BYTES_4,
		BYTES_5 = FDCAN_DLC_BYTES_5,
		BYTES_6 = FDCAN_DLC_BYTES_6,
		BYTES_7 = FDCAN_DLC_BYTES_7,
		BYTES_8 = FDCAN_DLC_BYTES_8,
		BYTES_12 = FDCAN_DLC_BYTES_12,
		BYTES_16 = FDCAN_DLC_BYTES_16,
		BYTES_20 = FDCAN_DLC_BYTES_20,
		BYTES_24 = FDCAN_DLC_BYTES_24,
		BYTES_32 = FDCAN_DLC_BYTES_32,
		BYTES_48 = FDCAN_DLC_BYTES_48,
		BYTES_64 = FDCAN_DLC_BYTES_64,
		DEFAULT = UINT32_MAX,
	};

    enum ID { FAULT_ID = 1 };

    struct Packet {
        array<uint8_t, 64> rx_data;
        uint32_t identifier;
        DLC data_length;
    };
    static std::string ip;

private:
    /**
     * @brief Struct which defines all data referring to FDCAN peripherals. It
     * is declared private in order to prevent unwanted use. Only predefined
     * instances should be used.
     *
     */
    static uint8_t Port_counter;
    struct Instance {
        Pin TX;
        Pin RX;
        DLC dlc;
        uint32_t rx_location;
        queue<FDCAN::Packet> rx_queue;
        uint8_t rx_queue_max_size = 64;
        vector<uint8_t> tx_data;
        uint8_t fdcan_number;
        uint16_t socket;
        bool start = false;
        uint16_t port;
    };

   public:
    /**
     * @brief Enum which abstracts the use of the Instance struct to facilitate
     * the mocking of the HALAL.Struct
     *
     */
    enum Peripheral {
        peripheral1 = 0,
        peripheral2 = 1,
        peripheral3 = 2,
    };

    static uint16_t id_counter;

    static unordered_map<uint8_t, FDCAN::Instance*> registered_fdcan;
    static unordered_map<FDCAN::Peripheral, FDCAN::Instance*> available_fdcans;
    static unordered_map<FDCAN::Instance*, uint8_t> instance_to_id;
    static unordered_map<FDCAN::DLC, uint8_t> dlc_to_len;
    /**
     * @brief FDCAN  wrapper enum of the STM32H723.
     *
     */
    static FDCAN::Peripheral fdcan1;
    static FDCAN::Peripheral fdcan2;
    static FDCAN::Peripheral fdcan3;

    /**
     * @brief FDCAN instances of the STM32H723.
     *
     */
    static FDCAN::Instance instance1;
    static FDCAN::Instance instance2;
    static FDCAN::Instance instance3;

    static uint8_t inscribe(FDCAN::Peripheral& fdcan);

    static void start();

    static bool transmit(uint8_t id, uint32_t message_id, const char* data,
                         FDCAN::DLC dlc = FDCAN::DLC::DEFAULT);

    static bool read(uint8_t id, FDCAN::Packet* data);

    /**
     * @brief This method is used to check if the FDCAN have received any new
     * packet.
     *
     * @param id Id of the FDCAN
     * @return bool Return true if the data queue has any packet.
     */
    static bool received_test(uint8_t id);
    static Packet packet;

   private:
    static void init(FDCAN::Instance* fdcan);
};
