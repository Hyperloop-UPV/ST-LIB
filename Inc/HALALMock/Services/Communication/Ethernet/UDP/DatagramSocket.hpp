#pragma once
#include "HALALMock/Services/Communication/Ethernet/EthernetNode.hpp"
#include "HALALMock/Services/Communication/Ethernet/Ethernet.hpp"
#include "HALALMock/Models/Packets/Packet.hpp"


class DatagramSocket{
public:

	IPV4 local_ip;
	uint32_t local_port;
	IPV4 remote_ip;
	uint32_t remote_port;
	bool is_disconnected = true;
	int udp_socket;
	DatagramSocket();
	DatagramSocket(DatagramSocket&& other);
	DatagramSocket(IPV4 local_ip, uint32_t local_port, IPV4 remote_ip, uint32_t remote_port);
	DatagramSocket(EthernetNode local_node, EthernetNode remote_node);
	~DatagramSocket();

	void operator=(DatagramSocket&&);

	bool send_packet(Packet& packet){
		uint8_t* packet_buffer = packet.build();
		size_t size = packet.get_size();
		//put the remote direction
		struct sockaddr_in remote_socket_addr;
		memset(&remote_socket_addr, 0, sizeof(remote_socket_addr));
		remote_socket_addr.sin_family = AF_INET;
    	remote_socket_addr.sin_port = htons(remote_port); 
    	remote_socket_addr.sin_addr.s_addr = remote_ip.address; 
		
		if(sendto(udp_socket,packet_buffer,size,0,(struct sockaddr *)&remote_socket_addr, sizeof(remote_socket_addr)) < 0){
			close(udp_socket);
			return false;
		}
		return true;
	}
	void reconnect();

	void close();

private:
	std::jthread receiving_udp_thread;
	std::atomic<bool> is_receiving;
	void create_udp_socket();
};


#endif
