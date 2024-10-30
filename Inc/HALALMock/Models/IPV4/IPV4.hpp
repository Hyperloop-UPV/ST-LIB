#pragma once
#include "C++Utilities/CppUtils.hpp"
#include <atomic> 
#include <mutex> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#ifdef HAL_ETH_MODULE_ENABLED

using std::stringstream;
using std::getline;

class IPV4{
private:
	void set_address_from_bytes(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4);
public:
	in_addr_t  address;
	string string_address;

	IPV4();
	IPV4(const char* address);
	IPV4(string address);
	IPV4(ip_addr_t address);

	void operator=(const char* address);
};

#endif
