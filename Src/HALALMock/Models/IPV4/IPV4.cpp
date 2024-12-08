#include "HALALMock/Models/IPV4/IPV4.hpp"
IPV4::IPV4(string _address) : string_address(_address){
    address = inet_addr(string_address.c_str());
}

IPV4::IPV4(const char* _address) {
    char* temp_ip = (char*)malloc(strlen(_address)+1);
    strcpy(temp_ip,_address);
    string_address = temp_ip;
    address = inet_addr(string_address.c_str());
}

IPV4::IPV4(in_addr_t _address) : address(_address){
	string_address = std::to_string((uint8_t) address) + "." + std::to_string((uint8_t) (address >> 8))+ "."
			+ std::to_string((uint8_t) (address >> 16)) + "." + std::to_string((uint8_t) (address >> 24));
}

IPV4::IPV4() = default;

void IPV4::operator =(const char* _address){
    char* temp_ip = (char*)malloc(strlen(_address)+1);
    strcpy(temp_ip,_address);
    string_address = temp_ip;
    address = inet_addr(string_address.c_str());
}

void IPV4::set_address_from_bytes(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4){
    this->address = (static_cast<in_addr_t>(byte1) << 24) |
                    (static_cast<in_addr_t>(byte2) << 16) |
                    (static_cast<in_addr_t>(byte3) << 8) |
                    (static_cast<in_addr_t>(byte4));
}

