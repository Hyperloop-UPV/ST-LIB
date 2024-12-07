
#include "HALALMock/Services/Communication/Ethernet/UDP/DatagramSocket.hpp"
#define MAX_SIZE_PACKET 1024

DatagramSocket::DatagramSocket() = default;

DatagramSocket::DatagramSocket(DatagramSocket&& other):udp_socket(move(other.udp_socket)), local_ip(move(other.local_ip)) , local_port(move(other.local_port)) ,remote_ip(move(other.remote_ip)),
		remote_port(move(other.remote_port))
		{}

DatagramSocket::DatagramSocket(IPV4 local_ip, uint32_t local_port, IPV4 remote_ip, uint32_t remote_port): local_ip(local_ip), 
local_port(local_port), remote_ip(remote_ip), remote_port(remote_port){
		if(not Ethernet::is_running) {
			std::cout<<"Cannot declare UDP socket before Ethernet::start()\n";
			return;
		}
		create_udp_socket();
	}

DatagramSocket::DatagramSocket(EthernetNode local_node, EthernetNode remote_node): DatagramSocket(local_node.ip, local_node.port, remote_node.ip, remote_node.port){}

DatagramSocket::~DatagramSocket(){
	if(not is_disconnected)
		close();
}
void DatagramSocket::create_udp_socket(){
	udp_socket = socket(AF_INET,SOCK_DGRAM,0);
	if(udp_socket < 0){
		std::cout<<"Socket creation failed\n";
	}
	struct sockaddr_in servaddr; 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(local_port); 
	servaddr.sin_addr.s_addr = local_ip.address; 
	if(bind(udp_socket, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
		std::cout<<"Bind error\n";
		::close(udp_socket);
		is_disconnected = true;
		return;
	}
	is_disconnected = false;
	//receiving callback
	receiving_udp_thread = std::jthread([&](){
		is_receiving = true;
		while(true){
			
			uint8_t received_data[1024];
			struct sockaddr_in src_addr;
			socklen_t addr_len = sizeof(src_addr);
			ssize_t size = recvfrom(udp_socket,(uint8_t*)received_data,MAX_SIZE_PACKET,0,(struct sockaddr *)&src_addr, &addr_len);
			if(size < 0){
				if (errno == EBADF){
					std::cout<< "The  udp_socket has been close\n";
					break;
				}
				else{
					std::cout<< "Error in function recvfrom\n";
					continue;
				}
			}
			//receive callback
			Packet::parse_data(received_data);
		}
		is_receiving = false;
			
	});
	Ethernet::update();
}
void DatagramSocket::operator=(DatagramSocket&& other){
	udp_socket = move(other.udp_socket);
	local_ip = move(other.local_ip);
	local_port = move(other.local_port);
	remote_ip = other.remote_ip;
	remote_port = other.remote_port;
	other.is_disconnected = true;
}

void DatagramSocket::reconnect(){
	is_disconnected = true;
	close();
	create_udp_socket();
}

void DatagramSocket::close(){
	if (!is_disconnected){
		if(::close(udp_socket)){
			std::cout<<"Error closing the udp_socket\n";
		}
		if(is_receiving){
		receiving_udp_thread.request_stop();
		receiving_udp_thread.~jthread();
	}
	is_disconnected = true;
	}
}


