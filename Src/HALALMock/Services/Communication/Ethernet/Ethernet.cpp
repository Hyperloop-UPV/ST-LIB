/*
 * Ethernet.cpp
 *
 *  Created on: Nov 23, 2022
 *      Author: stefa
 */

#include "HALALMock/Services/Communication/Ethernet/Ethernet.hpp"
#include "HALALMock/Models/MPUManager/MPUManager.hpp"
#include "HALALMock/Models/PinModel/Pin.hpp"
#include <iostream>
#ifdef HAL_ETH_MODULE_ENABLED
//We won't need EthernetLinkerTimer and gnetif however the other extenrn should be in a file to work
//extern uint32_t EthernetLinkTimer;
//extern struct netif gnetif;
extern ip4_addr_t ipaddr, netmask, gw;
extern uint8_t IP_ADDRESS[4], NETMASK_ADDRESS[4], GATEWAY_ADDRESS[4];

bool Ethernet::is_ready = false;
bool Ethernet::is_running = false;

void Ethernet::mpu_start(){}

void Ethernet::start(string local_ip, string subnet_mask, string gateway){
	start(IPV4(local_ip), IPV4(subnet_mask), IPV4(gateway));
}

void Ethernet::start(IPV4 local_ip, IPV4 subnet_mask, IPV4 gateway){
	if(!is_running && is_ready){
		ipaddr = local_ip.address;
		netmask = subnet_mask.address;
		gw = gateway.address;
		IP_ADDRESS[0] = ipaddr.addr & 0xFF;
		IP_ADDRESS[1] = (ipaddr.addr >> 8) & 0xFF;
		IP_ADDRESS[2] = (ipaddr.addr >> 16) & 0xFF;
		IP_ADDRESS[3] = (ipaddr.addr >> 24) & 0xFF;
		NETMASK_ADDRESS[0] = netmask.addr & 0xFF;
		NETMASK_ADDRESS[1] = (netmask.addr >> 8) & 0xFF;
		NETMASK_ADDRESS[2] = (netmask.addr >> 16) & 0xFF;
		NETMASK_ADDRESS[3] = (netmask.addr >> 24) & 0xFF;
		GATEWAY_ADDRESS[0] = gw.addr & 0xFF;
		GATEWAY_ADDRESS[1] = (gw.addr >> 8) & 0xFF;
		GATEWAY_ADDRESS[2] = (gw.addr >> 16) & 0xFF;
		GATEWAY_ADDRESS[3] = (gw.addr >> 24) & 0xFF;
		is_running = true;
	}else{
		std::cout<<"Unable to start Ethernet!"<<std::endl;
	}

	if (not is_ready) {
		std::cout<<"Ethernet is not ready"<<std::endl;
		return;
	}

}

void Ethernet::inscribe(){
	if(!is_ready){
		uint8_t number_pin_ethernet = 10;
		Pin pin_list_ethernet[number_pin_ethernet] = {PA1,PA2,PA7,PB13,PC1,PC4,PC5,PG11,PG0,PG13};
		for(size_t i = 0; i < number_pin_ethernet; i++){
			EmulatedPin &pin_data = SharedMemory::get_pin(pin_list_ethernet[i]);
			if(pin_data.type == PinType::NOT_USED){
			pin_data.type = PinType::Ethernet;
			}else{
				std::cout<<"Error inscribing ethernet Pins, PA1,PA2,PA7,PB13,PC1,PC4,PC5,PG11,PG0,PG13 must be free"<<std::endl;
				return;
			}
		}
	}else{
		std::cout<<"Unable to inscribe Ethernet because is already ready!"<<std::endl;
	}
}
void Ethernet::update(){
	//I'm going to leave the case is not running so it warn the user to check if you have done inscribed
	if(not is_running) {
		std::cout<<"Ethernet is not running, check if its been inscribed"<<std::endl;
		return;
	}
}

#endif
