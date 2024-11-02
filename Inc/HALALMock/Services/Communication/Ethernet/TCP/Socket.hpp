/*
 * Socket.hpp
 *
 *  Created on: 14 nov. 2022
 *      Author: stefa
 */
#pragma once

#include "HALALMock/Services/Communication/Ethernet/EthernetNode.hpp"
#include "HALALMock/Services/Communication/Ethernet/Ethernet.hpp"
#include "HALALMock/Models/Packets/Packet.hpp"
#include "HALALMock/Models/Packets/Order.hpp"
#include "HALALMock/Models/Packets/OrderProtocol.hpp"
#include <jtrhead>


class Socket : public OrderProtocol{
private:
	void configure_socket_and_connect();
	std::jthread receiving_thread;
	std::jthread connection_thread;
	std::atomic<bool> is_receiving;
	std::atomic<bool> is_connecting;
	std::mutex mtx; 
	void start_receiving();
	void stop_receiving();
	void receive();
	void create_socket();
	void configure_socket();
	void connect_thread();

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
	queue<Packet> tx_packet_buffer;
	queue<Packet> rx_packet_buffer;
	//socket_descriptor
	int socket_fd;
	static unordered_map<EthernetNode,Socket*> connecting_sockets;
	bool pending_connection_reset = false;
	bool use_keep_alives{true};
	struct KeepaliveConfig{
		uint32_t inactivity_time_until_keepalive_ms = TCP_INACTIVITY_TIME_UNTIL_KEEPALIVE_MS;
		uint32_t space_between_tries_ms = TCP_SPACE_BETWEEN_KEEPALIVE_TRIES_MS;
		uint32_t tries_until_disconnection = TCP_KEEPALIVE_TRIES_UNTIL_DISCONNECTION;
	}keepalive_config;

	Socket();
	Socket(Socket&& other);
	Socket(IPV4 local_ip, uint32_t local_port, IPV4 remote_ip, uint32_t remote_port,bool use_keep_alives = true);
	Socket(IPV4 local_ip, uint32_t local_port, IPV4 remote_ip, uint32_t remote_port, uint32_t inactivity_time_until_keepalive_ms, uint32_t space_between_tries_ms, uint32_t tries_until_disconnection);
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
		tx_packet_buffer.push(order);
		send();
		return true;
	}

	void send();

	void process_data();

	bool is_connected();

};
#endif
