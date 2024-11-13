#include "HALALMock/Models/IPV4/IPV4.hpp"


IPV4::IPV4(string address) : string_address(address){
	stringstream sstream(address);
	int ip_bytes[4];
	for(int& byte : ip_bytes){
		string temp;
		getline(sstream, temp, '.');
		byte = stoi(temp);
	}
	set_address_from_bytes(ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3]);
}

IPV4::IPV4(const char* address) {
    char* temp_ip = (char*)malloc(strlen(address)+1);
    strcpy(temp_ip,address);
    string_address = temp_ip;
    const char* token = strtok(temp_ip,".");
    int i = 0;
    uint8_t ip_bytes[4];
    while(token!=nullptr){
        ip_bytes[i++] = atoi(token);
        token = strtok(nullptr,".");
    }
    free(temp_ip);
	set_address_from_bytes(ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3]);
}

IPV4::IPV4(in_addr_t address) : address(address){
	string_address = std::to_string((u8_t) address) + "." + std::to_string((u8_t) (address >> 8))+ "."
			+ std::to_string((uint8_t) (address >> 16)) + "." + std::to_string((uint8_t) (address >> 24));
}

IPV4::IPV4() = default;

void IPV4::operator =(const char* address){
    char* temp_ip = (char*)malloc(strlen(address)+1);
    strcpy(temp_ip,address);
    string_address = temp_ip;
    const char* token = strtok(temp_ip,".");
    int i = 0;
    uint8_t ip_bytes[4];
    while(token!=nullptr){
        ip_bytes[i++] = atoi(token);
        token = strtok(nullptr,".");
    }
    free(temp_ip);
	set_address_from_bytes(ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3]);
}

void IPV4::set_address_from_bytes(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4){
    this->address = (static_cast<in_addr_t>(byte1) << 24) |
                    (static_cast<in_addr_t>(byte2) << 16) |
                    (static_cast<in_addr_t>(byte3) << 8) |
                    (static_cast<in_addr_t>(byte4));
}

