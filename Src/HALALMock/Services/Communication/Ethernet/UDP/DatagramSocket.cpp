

#include "HALALMock/Services/Communication/Ethernet/UDP/DatagramSocket.hpp"

#ifdef HAL_ETH_MODULE_ENABLED

DatagramSocket::DatagramSocket() = default;

DatagramSocket::DatagramSocket(DatagramSocket&& other):udp_control_block(move(other.udp_control_block)), local_ip(move(other.local_ip)) , local_port(move(other.local_port)) ,remote_ip(move(other.remote_ip)),
		remote_port(move(remote_port))
		{}

DatagramSocket::DatagramSocket(IPV4 local_ip, uint32_t local_port, IPV4 remote_ip, uint32_t remote_port): local_ip(local_ip), local_port(local_port), remote_ip(remote_ip), remote_port(remote_port){
		if(not Ethernet::is_running) {
			ErrorHandler("Cannot declare UDP socket before Ethernet::start()");
			return;
		}
		udp_control_block = udp_new();
		err_t error = udp_bind(udp_control_block, &local_ip.address, local_port);

		if(error == ERR_OK){
		   udp_recv(udp_control_block, receive_callback, nullptr);
		   udp_connect(udp_control_block, &remote_ip.address, remote_port);
		   is_disconnected = false;
		   Ethernet::update();
		}
		else{
		   udp_remove(udp_control_block);
		   is_disconnected = true;
		   ErrorHandler("Error binding UDP socket");
		}
	}

DatagramSocket::DatagramSocket(EthernetNode local_node, EthernetNode remote_node): DatagramSocket(local_node.ip, local_node.port, remote_node.ip, remote_node.port){}

DatagramSocket::~DatagramSocket(){
	if(not is_disconnected)
		close();
}

void DatagramSocket::operator=(DatagramSocket&& other){
	udp_control_block = move(other.udp_control_block);
	local_ip = move(other.local_ip);
	local_port = move(other.local_port);
	remote_ip = other.remote_ip;
	remote_port = other.remote_port;
	other.is_disconnected = true;
}

void DatagramSocket::reconnect(){
	udp_disconnect(udp_control_block);
	is_disconnected = true;
	err_t error = udp_bind(udp_control_block, &local_ip.address, local_port);

	if(error == ERR_OK){
	   udp_recv(udp_control_block, receive_callback, nullptr);
	   udp_connect(udp_control_block, &remote_ip.address, remote_port);
	   is_disconnected = false;
	   Ethernet::update();
	}
	else{
	   udp_remove(udp_control_block);
	   is_disconnected = true;
	   ErrorHandler("Error binding UDP socket");
	}
}

void DatagramSocket::close(){
	udp_disconnect(udp_control_block);
	udp_remove(udp_control_block);
	is_disconnected = true;
}

void DatagramSocket::receive_callback(void *args, struct udp_pcb *udp_control_block, struct pbuf *packet_buffer, const ip_addr_t *remote_address, u16_t port){
	uint8_t* received_data = (uint8_t*)packet_buffer->payload;
	Packet::parse_data(received_data);

	pbuf_free(packet_buffer);
}

#endif
