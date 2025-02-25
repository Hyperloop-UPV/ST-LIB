#pragma once


#include "HALALMock/Services/Communication/Ethernet/EthernetNode.hpp"
#include "HALALMock/Services/Communication/Ethernet/Ethernet.hpp"
#include "HALALMock/Models/Packets/Packet.hpp"
#include "HALALMock/Models/Packets/Order.hpp"
#include "HALALMock/Models/Packets/OrderProtocol.hpp"
#include <iostream>
#include <thread>
#include <poll.h>
class Socket : public OrderProtocol{
private:
	
	std::jthread receiving_thread;
	std::atomic<bool> is_receiving = false;
	std::mutex mutex; 
	std::queue<Packet*> tx_packet_buffer;
	//socket_descriptor
	int socket_fd;
	void start_receiving();
	void receive();
	bool create_socket();
	bool configure_socket();
	void connection_callback();
	void connect_attempt();

public:
	enum SocketState{
		INACTIVE,
		CONNECTED,
		CLOSING
	};

	IPV4 local_ip;
	uint32_t local_port;
	IPV4 remote_ip;
	uint32_t remote_port;
	SocketState state;
	static unordered_map<EthernetNode,Socket*> connecting_sockets;
	bool pending_connection_reset = false;
	bool use_keep_alives{true};
	struct KeepaliveConfig{
		uint32_t inactivity_time_until_keepalive = TCP_INACTIVITY_TIME_UNTIL_KEEPALIVE;
		uint32_t space_between_tries = TCP_SPACE_BETWEEN_KEEPALIVE_TRIES;
		uint32_t tries_until_disconnection = TCP_KEEPALIVE_TRIES_UNTIL_DISCONNECTION;
	}keepalive_config;

	Socket();
	Socket(Socket&& other);
	Socket(IPV4 local_ip, uint32_t local_port, IPV4 remote_ip, uint32_t remote_port,bool use_keep_alives = true);
	Socket(IPV4 local_ip, uint32_t local_port, IPV4 remote_ip, uint32_t remote_port, uint32_t inactivity_time_until_keepalive, uint32_t space_between_tries, uint32_t tries_until_disconnection);
	Socket(EthernetNode local_node, EthernetNode remote_node);
	~Socket();

	void operator=(Socket&& other);
	void close();

	void reconnect();
	void reset();

	/*
	 * @brief puts the order data into the tx_packet_buffer so it can be sent when a connection is accepted
	 * @return true if the data could be allocated in the buffer, false otherwise
	 */
	bool add_order_to_queue(Order& order);

	/*
	 * @brief puts the order data into the tx_packet_buffer and sends it
	 * @return true if the data was sent successfully, false otherwise
	 */
	bool send_order(Order& order) override{
		if(state != CONNECTED){
			reconnect();
			return false;
		}
		tx_packet_buffer.push(&order);
		send();
		return true;
	}

	void send();
	
	bool is_connected();
	
};

