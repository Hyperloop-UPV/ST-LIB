/*
 * ServerSocket.hpp
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
/**
* @brief class that handles a single point to point server client connection, emulating the server side.
*
* The flow of this class goes as follows:
*
* 1. When the constructor is called, the listener is activated and starts working immediately
*
* 2. After a client issues a connection to the ServerSocket and Ethernet#update is executed, the ServerSocket accepts the request
*
* 3. Accepting the request raises an interrupt that calls accept_callback, which closes the listener socket (on server_control_block) and opens the connection socket (on client_control_block)
*
* 4. The connection goes on until one of the ends closes it, which calls the ErrorHandler to send the board into fault as a default behaviour.
*
* @see Ethernet#update
*/
class ServerSocket : public OrderProtocol{
public:

	enum ServerState{
		INACTIVE,
		LISTENING,
		ACCEPTED,
		CLOSING,
		CLOSED
	};

	static unordered_map<uint32_t,ServerSocket*> listening_sockets;
	IPV4 local_ip;
	uint32_t local_port;
	IPV4 remote_ip;
	ServerState state;
	static uint8_t priority;
	//socket_descriptor
	int server_socket_fd;
	struct KeepaliveConfig{
		uint32_t inactivity_time_until_keepalive_ms = TCP_INACTIVITY_TIME_UNTIL_KEEPALIVE_MS;
		uint32_t space_between_tries_ms = TCP_SPACE_BETWEEN_KEEPALIVE_TRIES_MS;
		uint32_t tries_until_disconnection = TCP_KEEPALIVE_TRIES_UNTIL_DISCONNECTION;
	}keepalive_config;

	ServerSocket();


	ServerSocket(ServerSocket&& other);

	/**
	 * @brief ServerSocket constructor that receives the server ip on the net as a binary value.
	 *
	 * @param local_ip the server ip on.
	 * @param local_port the port number that the server listens for connections.
	 */
	ServerSocket(IPV4 local_ip, uint32_t local_port);
	ServerSocket(IPV4 local_ip, uint32_t local_port, uint32_t inactivity_time_until_keepalive_ms, uint32_t space_between_tries_ms, uint32_t tries_until_disconnection);
	/**
	 * @brief ServerSocket constructor that uses the EthernetNode class as a parameter
	 *
	 * @param local_node the EthernetNode to listen to
	 *
	 * @see EthernetNode
	 */
	ServerSocket(EthernetNode local_node);
	~ServerSocket();

	void operator=(ServerSocket&& other);

	/**
	* @brief ends the connection between the server and the client. 
	*/
	void close();

	/**
	* @brief process the data received by the client orders. It is meant to be called only by Lwip on the receive_callback
	*
	* reads all the data received by the server in the ethernet buffer, packet by packet.
	* Then, for each packet, it processes it depending on the order id (default behavior is not process) and removes it from the buffer. 
	* This makes so the receive_callback (and thus the Socket) can only process declared orders, and ignores all other packets. 
	*/
	void process_data();

	/**
	 * @brief saves the order data into the tx_packet_buffer so it can be sent when a connection is accepted
	 *
	 * @param order the order to send, which contains the data and id of the message
	 * @return true if the data could be allocated in the buffer, false otherwise
	 */
	bool add_order_to_queue(Order& order);

	/**
	 * @brief puts the order data into the tx_packet_buffer and sends all the data in the buffer to the client
	 *
	 * @param order the order to send, which contains the data and id of the message
	 * @return true if the data was sent successfully, false otherwise
	 */
	bool send_order(Order& order) override{
		if(state != ACCEPTED){
			return false;
		}
		struct memp* next_memory_pointer_in_packet_buffer_pool = (*(memp_pools[PBUF_POOL_MEMORY_DESC_POSITION]->tab))->next;
		if(next_memory_pointer_in_packet_buffer_pool == nullptr){
			if(client_control_block->unsent != nullptr){
				tcp_output(client_control_block);
			}else{
				memp_free_pool(memp_pools[PBUF_POOL_MEMORY_DESC_POSITION], next_memory_pointer_in_packet_buffer_pool);
			}
			return false;
		}

		uint8_t* order_buffer = order.build();
		if(order.get_size() > tcp_sndbuf(client_control_block)){
			return false;
		}

		struct pbuf* packet = pbuf_alloc(PBUF_TRANSPORT, order.get_size(), PBUF_POOL);
		pbuf_take(packet, order_buffer, order.get_size());
		tx_packet_buffer.push(packet);
		send();
		return true;
	}

	/**
	* @brief sends all the binary data saved in the tx_packet_buffer to the connected client. 
	*
	* This function is the one that actually handles outgoing communication, sending one by one the packets in the tx_packet_buffer
	* The messages in the buffer are all immediately sent after calling this function, unless an error of any kind happened, in which case ErrorHandler is raised
	*/
	void send();

	/**
	* @brief function that returns wether or not a client is connected to the ServerSocket
	*
	* This functions returns a comparison to the state of the ServerSocket, checking wether or not it is on the ACCEPTED state
	* This function is equivalent to doing instance->state == ServerSocket#ACCEPT
	*
	* @return true if a connection with the client was established, false otherwise
	*/
	bool is_connected();

private:
	void create_server_socket();
	void configure_server_socket();
	void configure_server_socket_and_listen();
	/**
	 * @brief the callback for the listener socket receiving a request for connection into the ServerSocket.
	 *
	 * This function is called on an interrupt when a packet containing a connection request to the same port of the listener socket is received.
	 * accept_callback builds the pcb that acts as the connection socket and saves it in the client_control_block pointer
	 * It then closes the listener socket and makes the server_control_block point to nullptr.
	 *
	 * server_control_block shouldn't be accessed in any way while the ServerSocket is in the ServerSocket#ACCEPT state, as it will lead into a nullptr exception.
	 * This in an intended behavior as there shouldn't be more than one listener socket on the same port, and a ServerSocket shouldn't be able to handle more than one connection by design.
	 */
	static err_t accept_callback(void* arg, struct tcp_pcb* incomming_control_block, err_t error);

	/**
	 * @brief callback that handles receiving a packet.
	 *
	 * On a received packet on an ACCEPTED state receive_callback calls the ServerSocket process_data,
	 * which calls the Packet process_data() if it is inscribed as an Order.
	 */
	static err_t receive_callback(void* arg, struct tcp_pcb* client_control_block, struct pbuf* packet_buffer, err_t error);
	static void error_callback(void *arg, err_t error);

	/**
	 * @brief callback called each 500ms to do housekeeping tasks
	 *
	 * The housekeeping tasks made now are basically checks to ensure no data remains unsent and that the ServerSocket is not left in a middle state (such as CLOSING)
	 */
	static err_t poll_callback(void *arg, struct tcp_pcb *client_control_block);
	static err_t send_callback(void *arg, struct tcp_pcb *client_control_block, u16_t len);

	static void config_keepalive(tcp_pcb* control_block, ServerSocket* server_socket);

};

#endif
