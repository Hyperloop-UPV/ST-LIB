
#pragma once
//#ifdef STLIB_ETH
#include "HALALMock/Services/Communication/Ethernet/EthernetNode.hpp"
#include "HALALMock/Services/Communication/Ethernet/Ethernet.hpp"
#include "HALALMock/Models/Packets/Packet.hpp"
#include "HALALMock/Models/Packets/Order.hpp"
#include "HALALMock/Models/Packets/OrderProtocol.hpp"
#include <iostream>
#include <thread>
#include <poll.h>
#include <atomic>
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
private:
	void create_server_socket();
	bool configure_server_socket(int& socket);
	void listen_for_connection();
	void close_inside_thread();
	bool accept_callback(int& client_fd, sockaddr_in& client_address);
	void receive();
	queue<Packet*> tx_packet_buffer;
	std::jthread listening_thread;
	std::jthread receive_thread;
	std::mutex mutex; 
	int server_socket_fd{-1};
	int client_fd{-1};

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
	
	struct KeepaliveConfig{
		uint32_t inactivity_time_until_keepalive = TCP_INACTIVITY_TIME_UNTIL_KEEPALIVE;
		uint32_t space_between_tries = TCP_SPACE_BETWEEN_KEEPALIVE_TRIES;
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
	ServerSocket(IPV4 local_ip, uint32_t local_port, uint32_t inactivity_time_until_keepalive, uint32_t space_between_tries, uint32_t tries_until_disconnection);
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
		tx_packet_buffer.push(&order);
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

};

//#endif //STLIB_ETH
