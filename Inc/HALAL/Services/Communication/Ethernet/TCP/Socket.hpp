/*
 * Socket.hpp
 *
 *  Created on: 14 nov. 2022
 *      Author: stefa
 */
#pragma once

#include "HALAL/Services/Communication/Ethernet/EthernetNode.hpp"
#include "HALAL/Services/Communication/Ethernet/Ethernet.hpp"
#include "HALAL/Models/Packets/Packet.hpp"
#include "HALAL/Models/Packets/Order.hpp"
#include "HALAL/Models/Packets/OrderProtocol.hpp"
#include "HALAL/Models/Packets/Packet.hpp"
#ifdef HAL_ETH_MODULE_ENABLED

#define PBUF_POOL_MEMORY_DESC_POSITION 8

class Socket : public OrderProtocol{
private:
	tcp_pcb* connection_control_block;
	tcp_pcb* socket_control_block;
	queue<struct pbuf*> tx_packet_buffer;
	queue<struct pbuf*> rx_packet_buffer;
	void process_data();
	static err_t connect_callback(void* arg, struct tcp_pcb* client_control_block, err_t error);
	static err_t receive_callback(void* arg, struct tcp_pcb* client_control_block, struct pbuf* packet_buffer, err_t error);
	static err_t poll_callback(void* arg, struct tcp_pcb* client_control_block);
	static err_t send_callback(void* arg, struct tcp_pcb* client_control_block, uint16_t length);
	static void error_callback(void *arg, err_t error);

	static err_t connection_poll_callback(void* arg, struct tcp_pcb* connection_control_block);
	static void connection_error_callback(void *arg, err_t error);
	static void config_keepalive(tcp_pcb* control_block, Socket* socket);
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
		struct memp* next_memory_pointer_in_packet_buffer_pool = (*(memp_pools[PBUF_POOL_MEMORY_DESC_POSITION]->tab))->next;
		if(next_memory_pointer_in_packet_buffer_pool == nullptr){
			if(socket_control_block->unsent != nullptr){
				tcp_output(socket_control_block);
			}else{
				memp_free_pool(memp_pools[PBUF_POOL_MEMORY_DESC_POSITION], next_memory_pointer_in_packet_buffer_pool);
			}
			return false;
		}

		uint8_t* order_buffer = order.build();
		if(order.get_size() > tcp_sndbuf(socket_control_block)){
			return false;
		}

		struct pbuf* packet = pbuf_alloc(PBUF_TRANSPORT, order.get_size(), PBUF_POOL);
		pbuf_take(packet, order_buffer, order.get_size());
		tx_packet_buffer.push(packet);
		send();
		return true;
	}
	void send();
	bool is_connected();
};
#endif
