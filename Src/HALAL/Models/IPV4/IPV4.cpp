#include "HALAL/Models/IPV4/IPV4.hpp"
#ifdef HAL_ETH_MODULE_ENABLED

// IPV4::IPV4(string address) : string_address(address){
// 	stringstream sstream(address);
// 	int ip_bytes[4];
// 	for(int& byte : ip_bytes){
// 		string temp;
// 		getline(sstream, temp, '.');
// 		byte = stoi(temp);
// 	}
// 	IP_ADDR4(&(this->address), ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3]);
// }

IPV4::IPV4(const char* address) {
    int ip_bytes[4];
    sscanf(address, "%d.%d.%d.%d", &ip_bytes[0], &ip_bytes[1], &ip_bytes[2], &ip_bytes[3]);
	IP_ADDR4(&(this->address), ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3]);
}

IPV4::IPV4(ip_addr_t address) : address(address){
	// string_address = std::to_string((u8_t) address.addr) + "." + std::to_string((u8_t) (address.addr >> 8))+ "."
	// 		+ std::to_string((uint8_t) (address.addr >> 16)) + "." + std::to_string((uint8_t) (address.addr >> 24));
}

IPV4::IPV4() = default;

IPV4 IPV4::parse_string(const char* address) {
    return IPV4(address);
}

void IPV4::operator =(const char* address){
    int ip_bytes[4];
    sscanf(address, "%d.%d.%d.%d", &ip_bytes[0], &ip_bytes[1], &ip_bytes[2], &ip_bytes[3]);
	IP_ADDR4(&(this->address), ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3]);
}



#endif
