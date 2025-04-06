/*
 *  FDCAN.hpp
 *
 *  Created on: 5 nov. 2022
 *      Author: Pablo
 */

#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "stm32h7xx_hal.h"

#ifdef HAL_FDCAN_MODULE_ENABLED

using std::unordered_map;
using std::vector;
using std::queue;

enum class CANBitRatesSpeed{
    CAN_125_kbit = 0,
    CAN_250_kbit = 1,
    CAN_500_kbit = 2,
    CAN_1_Mbit = 3
};

enum class CANFormat{
    CAN_NORMAL_FORMAT = 0,
    CAN_FDCAN_FORMAT = 1
};
enum class CANIdentifier{
    CAN_11_BIT_IDENTIFIER = 0,
    CAN_29_BIT_IDENTIFIER = 1
};
enum class CANMode{
    CAN_MODE_NORMAL = 0,
    CAN_MODE_LOOPBACK = 4
};

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
    enum ID{
        FAULT_ID = 1
    };

	struct Packet{
		array<uint8_t,64> rx_data;
		uint32_t identifier;
		DLC data_length;

	};

private:
    /**
     * @brief Struct which defines all data referring to FDCAN peripherals. It is
     *        declared private in order to prevent unwanted use. Only
     *        predefined instances should be used.
     *
     */
    struct Instance{
        Pin TX;
        Pin RX;
        FDCAN_HandleTypeDef* hfdcan;
        FDCAN_GlobalTypeDef* instance;
        DLC dlc;
        FDCAN_TxHeaderTypeDef tx_header;
        uint32_t rx_location;
        queue<FDCAN::Packet> rx_queue;
        uint8_t rx_queue_max_size = 64;
        vector<uint8_t> tx_data;
        uint8_t fdcan_number;
        bool start = false;

    };
public:
    /**
	 * @brief Enum which abstracts the use of the Instance struct to facilitate the mocking of the HALAL.Struct
	 *
	 */
	enum Peripheral{
		peripheral1 = 0,
		peripheral2 = 1,
		peripheral3 = 2,
	};


    static uint16_t id_counter;

    static unordered_map<uint8_t, FDCAN::Instance*> registered_fdcan;
    static unordered_map<FDCAN::Peripheral, FDCAN::Instance*> available_fdcans;
    static unordered_map<FDCAN_HandleTypeDef*, FDCAN::Instance*> handle_to_fdcan;
    static unordered_map<FDCAN::Instance*,uint8_t> instance_to_id;
    static unordered_map<FDCAN_HandleTypeDef*,uint8_t> handle_to_id;
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
template<CANBitRatesSpeed Speed,CANFormat Format,CANIdentifier id,CANMode Mode>
    static uint8_t inscribe(FDCAN::Peripheral& fdcan);

    static void start();

    static bool transmit(uint8_t id, uint32_t message_id, const char* data, FDCAN::DLC dlc = FDCAN::DLC::DEFAULT);

    static bool read(uint8_t id, FDCAN::Packet* data);

    /**
	 * @brief This method is used to check if the FDCAN have received any new packet.
	 *
	 * @param id Id of the FDCAN
	 * @return bool Return true if the data queue has any packet.
	 */
	static bool received_test(uint8_t id);
    static Packet packet;
private:

    static void init(FDCAN::Instance* fdcan);



};

template<CANBitRatesSpeed Speed,CANFormat format,CANIdentifier message_id,CANMode mode>
uint8_t FDCAN::inscribe(FDCAN::Peripheral& fdcan){
	if (!FDCAN::available_fdcans.contains(fdcan)) {
		ErrorHandler(" The FDCAN peripheral %d is already used or does not exists.", (uint16_t)fdcan);
		return 0;
	}

	FDCAN::Instance* fdcan_instance = FDCAN::available_fdcans[fdcan];
    if constexpr(format == CANFormat::CAN_FDCAN_FORMAT){
        fdcan_instance->tx_header.FDFormat = FDCAN_FD_CAN;
    }else{
        fdcan_instance->tx_header.FDFormat = FDCAN_CLASSIC_CAN;
    }
	fdcan_instance->tx_header.DataLength = fdcan_instance->dlc;
	fdcan_instance->tx_header.TxFrameType = FDCAN_DATA_FRAME;
	fdcan_instance->tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    fdcan_instance->tx_header.BitRateSwitch = FDCAN_BRS_OFF;

    if constexpr(message_id == CANIdentifier::CAN_29_BIT_IDENTIFIER){	   
        fdcan_instance->tx_header.IdType = FDCAN_EXTENDED_ID;
        fdcan_instance->hfdcan->Init.FrameFormat = FDCAN_FRAME_FD_NO_BRS;
    }else{
        fdcan_instance->tx_header.IdType = FDCAN_STANDARD_ID;
        fdcan_instance->hfdcan->Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    }
	fdcan_instance->tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	fdcan_instance->tx_header.MessageMarker = 0;
	fdcan_instance->tx_header.Identifier = 0x0;

	fdcan_instance->hfdcan->Instance = fdcan_instance->instance;
// use NORMAL or EXTERNAL_LOOPBACK mode
	fdcan_instance->hfdcan->Init.Mode = static_cast<uint32_t>(mode);
	fdcan_instance->hfdcan->Init.AutoRetransmission = DISABLE;
	fdcan_instance->hfdcan->Init.TransmitPause = DISABLE;
	fdcan_instance->hfdcan->Init.ProtocolException = DISABLE;
///////////////////////////////////////////////////////////
    if constexpr(Speed == CANBitRatesSpeed::CAN_125_kbit){
        fdcan_instance->hfdcan->Init.NominalPrescaler = 20;
        fdcan_instance->hfdcan->Init.NominalSyncJumpWidth = 2;
        fdcan_instance->hfdcan->Init.NominalTimeSeg1 = 5;
        fdcan_instance->hfdcan->Init.NominalTimeSeg2 = 2;
    }else if constexpr(Speed == CANBitRatesSpeed::CAN_250_kbit){
        fdcan_instance->hfdcan->Init.NominalPrescaler = 10;
        fdcan_instance->hfdcan->Init.NominalSyncJumpWidth = 2;
        fdcan_instance->hfdcan->Init.NominalTimeSeg1 = 5;
        fdcan_instance->hfdcan->Init.NominalTimeSeg2 = 2;
    }else if constexpr(Speed == CANBitRatesSpeed::CAN_500_kbit){
        fdcan_instance->hfdcan->Init.NominalPrescaler = 5;
        fdcan_instance->hfdcan->Init.NominalSyncJumpWidth = 2;
        fdcan_instance->hfdcan->Init.NominalTimeSeg1 = 5;
        fdcan_instance->hfdcan->Init.NominalTimeSeg2 = 2;
    }else if constexpr(Speed == CANBitRatesSpeed::CANN_1_Mbit){
        fdcan_instance->hfdcan->Init.NominalPrescaler = 1;
        fdcan_instance->hfdcan->Init.NominalSyncJumpWidth = 4;
        fdcan_instance->hfdcan->Init.NominalTimeSeg1 = 15;
        fdcan_instance->hfdcan->Init.NominalTimeSeg2 = 4;
    }
////////////////////////////////////////////////////////////
	fdcan_instance->hfdcan->Init.DataPrescaler = 11;
	fdcan_instance->hfdcan->Init.DataSyncJumpWidth = 4;
	fdcan_instance->hfdcan->Init.DataTimeSeg1 = 17;
	fdcan_instance->hfdcan->Init.DataTimeSeg2 = 8;
	fdcan_instance->hfdcan->Init.MessageRAMOffset = 0;
	fdcan_instance->hfdcan->Init.StdFiltersNbr = 0;
	fdcan_instance->hfdcan->Init.ExtFiltersNbr = 0;
	fdcan_instance->hfdcan->Init.RxFifo0ElmtsNbr = 16;
	fdcan_instance->hfdcan->Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_64;
	fdcan_instance->hfdcan->Init.RxFifo1ElmtsNbr = 0;
	fdcan_instance->hfdcan->Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_64;
	fdcan_instance->hfdcan->Init.RxBuffersNbr = 0;
	fdcan_instance->hfdcan->Init.RxBufferSize = FDCAN_DATA_BYTES_64;
	fdcan_instance->hfdcan->Init.TxEventsNbr = 0;
	fdcan_instance->hfdcan->Init.TxBuffersNbr = 0;
	fdcan_instance->hfdcan->Init.TxFifoQueueElmtsNbr = 16;
	fdcan_instance->hfdcan->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
	fdcan_instance->hfdcan->Init.TxElmtSize = FDCAN_DATA_BYTES_8;

	Pin::inscribe(fdcan_instance->TX, ALTERNATIVE);
	Pin::inscribe(fdcan_instance->RX, ALTERNATIVE);

	uint8_t id = FDCAN::id_counter++;

	FDCAN::registered_fdcan[id] = fdcan_instance;

	return id;
}

#endif
