/*
 *  FDCAN.hpp
 *
 *  Created on: 5 nov. 2022
 *      Author: Pablo
 */

#include "HALALMock/Services/Communication/FDCAN/FDCAN.hpp"

#ifdef HAL_FDCAN_MODULE_ENABLED

uint16_t FDCAN::id_counter = 0;

unordered_map<uint8_t, FDCAN::Instance*> FDCAN::registered_fdcan = {};

unordered_map<FDCAN::DLC, uint8_t> FDCAN::dlc_to_len = {{DLC::BYTES_0, 0}, {DLC::BYTES_1, 1}, {DLC::BYTES_2, 2}, {DLC::BYTES_3, 3}, {DLC::BYTES_4, 4},
														{DLC::BYTES_5, 5}, {DLC::BYTES_6, 6}, {DLC::BYTES_7, 7}, {DLC::BYTES_8, 8}, {DLC::BYTES_12, 12},
														{DLC::BYTES_16, 16}, {DLC::BYTES_20, 20}, {DLC::BYTES_24, 24}, {DLC::BYTES_32, 32}, {DLC::BYTES_48, 48},
														{DLC::BYTES_64, 64}
													    };
unordered_map<FDCAN_HandleTypeDef*,uint8_t> FDCAN::handle_to_id{};
unordered_map<FDCAN::Instance*,uint8_t> FDCAN::instance_to_id{};
FDCAN::Packet packet{.rx_data = array<uint8_t, 64>{},.data_length = FDCAN::BYTES_64};
uint8_t FDCAN::inscribe(FDCAN::Peripheral& fdcan){
	if (!FDCAN::available_fdcans.contains(fdcan)) {
		ErrorHandler(" The FDCAN peripheral %d is already used or does not exists.", (uint16_t)fdcan);
		return 0;
	}

	FDCAN::Instance* fdcan_instance = FDCAN::available_fdcans[fdcan];

	EmulatedPin &TX_data = SharedMemory::get_pin(fdcan_instance->TX);
	EmulatedPin &RX_data = SharedMemory::get_pin(fdcan_instance->RX);
	if(TX_data.type == PinType::NOT_USED){
		pin_data.type = PinType::FDCAN;
	}else{
		ErrorHandler("Pin %d is already in use",fdcan_instance->TX);
		}
	if(RX_data.type == PinType::NOT_USED){
		pin_data.type = PinType::FDCAN;
	}else{
		ErrorHandler("Pin %d is already in use",fdcan_instance->RX);
	}

	uint8_t id = FDCAN::id_counter++;

	FDCAN::registered_fdcan[id] = fdcan_instance;

	return id;
}

ssize_t dlc_to_number_of_bytes(FDCAN::DLC dlc){

	return static_cast<ssize_t>(FDCAN::dlc_to_len[dlc]);
}

void FDCAN::start(){
	for( std::pair<uint8_t, FDCAN::Instance*> inst: FDCAN::registered_fdcan){
		uint8_t id = inst.first;
		FDCAN::Instance* instance = inst.second;

		instance->rx_queue = queue<FDCAN::Packet>();
		instance->tx_data = vector<uint8_t>();

		instance -> socket = socket(AF_NET,SOCK_DGRAM,0);
		if(instance -> socket < 0){
			ErrorHandler("Error creating socket for FDCAN %d", instance->fdcan_number);
		}
		struct sockaddr_in BroadcastAddress;
		BroadcastAdress.sin_family = AF_INET;
		BroadcastAdress.sin_port = FDCAN_PORT_BASE + Port_counter;
		BroadcastAdress.sin_addr.s_addr = fdcan_ip_adress;

		int enabled = 1;
		setsockopt(instance->socket, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled));

		if(bind(instance->socket, (struct sockaddr*)&BroadcastAdress, sizeof(BroadcastAdress)) < 0){
			ErrorHandler("Error binding socket for FDCAN %d", instance->fdcan_number);
		}
	    instance->start = true;
	    Port_counter++;
	    FDCAN::registered_fdcan[id] = instance;
		FDCAN::instance_to_id[instance] = id;
	}
}


bool FDCAN::transmit(uint8_t id, uint32_t message_id, const char* data, FDCAN::DLC dlc){ 
	if (not FDCAN::registered_fdcan.contains(id)) {
		ErrorHandler("There is no registered FDCAN with id: %d.", id);
		return false;
	}

	FDCAN::Instance* instance = registered_fdcan[id];

	if (not instance->start) {
		ErrorHandler("The FDCAN %d is not initialized.", instance->fdcan_number);
		return false;
	}

    size_t buffer_len = sizeof(message_id)+ sizeof(dlc) + dlc_to_number_of_bytes(dlc);
    char* temp_data = new char[buffer_len];
	temp_data[0] = static_cast<char>(message_id >> 24);
	temp_data[1] = static_cast<char>(message_id >> 16);
	temp_data[2] = static_cast<char>(message_id >> 8);
	temp_data[3] = static_cast<char>(message_id);

	temp_data[4] = static_cast<char>(static_cast<uint32_t>(dlc) >> 24);
	temp_data[5] = static_cast<char>(static_cast<uint32_t>(dlc) >> 16);
	temp_data[6] = static_cast<char>(static_cast<uint32_t>(dlc) >> 8);
	temp_data[7] = static_cast<char>(static_cast<uint32_t>(dlc));

	memcpy((temp_data + sizeof(message_id)+ sizeof(dlc)), data, dlc_to_number_of_bytes(dlc));
	ssize_t total_bytes_sent{0};
	while (total_bytes_sent < strlen(temp_data)) {
    	ssize_t bytes_sent = send(instance->socket, temp_data + total_bytes_sent, strlen(temp_data) - total_bytes_sent);
		if (bytes_sent < 0) {
			ErrorHandler("Error sending message with id: 0x%x by FDCAN %d", message_id, instance->fdcan_number);
			delete[] temp_data;
			return false;
		}
		total_bytes_sent += bytes_sent;
	}
	
	delete[] temp_data;
	return true;
}


bool FDCAN::read(uint8_t id, FDCAN::Packet* data){
if (not FDCAN::registered_fdcan.contains(id)) {
		ErrorHandler("There is no FDCAN registered with id: %d.", id);
		return false;
	}
	FDCAN::Instance* instance = registered_fdcan[id];

	if(!FDCAN::received_test(id)) {
		return false;
	}
	
	if(recv(instance->socket, data->rx_data, 64,0)<0){
		ErrorHandler("Error receiving message by FDCAN %d", instance->fdcan_number);
		return false;
	}
	

	data->identifier = (data->rx_data[0] << 24) | (data->rx_data[1] << 16) | (data->rx_data[2] << 8) | data->rx_data[3];
	data->data_length = static_cast<FDCAN::DLC>((data->rx_data[4] << 24) | (data->rx_data[5] << 16) | (data->rx_data[6] << 8) | data->rx_data[7]);
	array<uint8_t, 64> aux_rx_data;
	std::copy(data->rx_data.begin()+8, data->rx_data.end(), aux_rx_data.begin());
	data->rx_data = aux_rx_data;

	return true;
}

bool FDCAN::received_test(uint8_t id){
	if (not FDCAN::registered_fdcan.contains(id)) {
		ErrorHandler("FDCAN with id %u not registered", id);
		return false;
	}

	return true;
}

#endif
