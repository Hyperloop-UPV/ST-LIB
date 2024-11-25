/*
 * Ethernet.cpp
 *
 *  Created on: Nov 23, 2022
 *      Author: stefa
 */

#include "HALALMock/Services/Communication/Ethernet/Ethernet.hpp"
#include <iostream>
#include <HALALMock/Services/SharedMemory/SharedMemory.hpp>

in_addr_t ipaddr, netmask, gw;
uint8_t IP_ADDRESS[4], NETMASK_ADDRESS[4], GATEWAY_ADDRESS[4];

bool Ethernet::is_ready = false;
bool Ethernet::is_running = false;
void Ethernet::start(string local_ip, string subnet_mask, string gateway){
	start(IPV4(local_ip), IPV4(subnet_mask), IPV4(gateway));
}

void Ethernet::start(IPV4 local_ip, IPV4 subnet_mask, IPV4 gateway){
	if(!is_running && is_ready){
		ipaddr = local_ip.address;
		netmask = subnet_mask.address;
		gw = gateway.address;
		IP_ADDRESS[0] = ipaddr & 0xFF;
		IP_ADDRESS[1] = (ipaddr >> 8) & 0xFF;
		IP_ADDRESS[2] = (ipaddr >> 16) & 0xFF;
		IP_ADDRESS[3] = (ipaddr >> 24) & 0xFF;
		NETMASK_ADDRESS[0] = netmask & 0xFF;
		NETMASK_ADDRESS[1] = (netmask >> 8) & 0xFF;
		NETMASK_ADDRESS[2] = (netmask >> 16) & 0xFF;
		NETMASK_ADDRESS[3] = (netmask >> 24) & 0xFF;
		GATEWAY_ADDRESS[0] = gw & 0xFF;
		GATEWAY_ADDRESS[1] = (gw >> 8) & 0xFF;
		GATEWAY_ADDRESS[2] = (gw >> 16) & 0xFF;
		GATEWAY_ADDRESS[3] = (gw >> 24) & 0xFF;
		is_running = true;
	}else{
		std::cout<<"Unable to start Ethernet!\n";
	}

	if (not is_ready) {
		std::cout<<"Ethernet is not ready\n";
		return;
	}

}

void Ethernet::inscribe(){
	constexpr static uint8_t number_pin_ethernet = 10;
	const static Pin pin_list_ethernet[number_pin_ethernet] = {PA1,PA2,PA7,PB13,PC1,PC4,PC5,PG11,PG0,PG13};
	if(!is_ready){
		for(size_t i = 0; i < number_pin_ethernet; i++){
			EmulatedPin &pin_data = SharedMemory::get_pin(pin_list_ethernet[i]);
			if(pin_data.type == PinType::NOT_USED){
			pin_data.type = PinType::Ethernet;
			}else{
				std::cout<<"Error inscribing ethernet Pins, PA1,PA2,PA7,PB13,PC1,PC4,PC5,PG11,PG0,PG13 must be free\n";
				return;
			}
		}
		is_ready = true;
	}else{
		std::cout<<"Unable to inscribe Ethernet because is already ready!\n";
	}
}
void Ethernet::update(){
	//I'm going to leave the case is not running so it warn the user to check if you have done inscribed
	if(not is_running) {
		std::cout<<"Ethernet is not running, check if its been inscribed\n";
		return;
	}
}
