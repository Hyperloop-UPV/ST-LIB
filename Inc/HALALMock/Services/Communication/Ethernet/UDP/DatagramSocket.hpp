#pragma once

#include "HALALMock/Services/Communication/Ethernet/EthernetNode.hpp"
#include "HALALMock/Services/Communication/Ethernet/Ethernet.hpp"
#include "HALALMock/Models/Packets/Packet.hpp"
#include <iostream>


class DatagramSocket{
	private:
		std::jthread receiving_udp_thread;
		std::atomic<bool> is_receiving;
		void create_udp_socket();
	public:
		int udp_socket;
		IPV4 local_ip;
		uint32_t local_port;
		IPV4 remote_ip;
		uint32_t remote_port;
		bool is_disconnected = true;
		
		DatagramSocket();
		DatagramSocket(DatagramSocket&& other);
		DatagramSocket(IPV4 local_ip, uint32_t local_port, IPV4 remote_ip, uint32_t remote_port);
		DatagramSocket(EthernetNode local_node, EthernetNode remote_node);
		~DatagramSocket();

		void operator=(DatagramSocket&&);

		bool send_packet(Packet& packet){
			uint8_t* packet_buffer = packet.build();
			size_t size = packet.get_size();
			size_t total_sent = 0;
			size_t sent = 0;
			//put the remote direction
			struct sockaddr_in remote_socket_addr;
			size_t addr_len =  sizeof(remote_socket_addr);
			memset(&remote_socket_addr, 0, addr_len);
			remote_socket_addr.sin_family = AF_INET;
		remote_socket_addr.sin_port = htons(remote_port); 
		remote_socket_addr.sin_addr.s_addr = remote_ip.address; 
		while(total_sent < size){
				sent = sendto(udp_socket,packet_buffer + total_sent,size - total_sent,0,(struct sockaddr *)&remote_socket_addr, addr_len);
				if(sent < 0){//something failed
					std::cout<<"Error sending the UDP packet\n";
					close();
					return false;
				}
			total_sent += sent;		
		}
			return true;
		}
		void reconnect();

		void close();
};


