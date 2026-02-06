/*
 * Server.hpp
 *
 * Created on: Oct 12, 2023
 * 		Author: Ricardo
 */

#pragma once
#ifdef STLIB_ETH
#include <array>
#include <cstdint>
#include <vector>

#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Models/IPV4/IPV4.hpp"
#include "HALAL/Models/Packets/Order.hpp"
#include "HALAL/Services/Communication/Ethernet/LWIP/TCP/ServerSocket.hpp"

#ifndef MAX_CONNECTIONS_TCP_SERVER
	#define MAX_CONNECTIONS_TCP_SERVER 10
#endif

class Server{
public:

	enum ServerState{
		RUNNING,
		CLOSING,
		CLOSED
	};

	ServerSocket *open_connection;
	array<ServerSocket*,MAX_CONNECTIONS_TCP_SERVER> running_connections;
	uint16_t running_connections_count;
	IPV4 local_ip;
	uint32_t local_port;
	ServerState status;

	static vector<Server*> running_servers;

	Server(IPV4 local_ip, uint32_t local_port);
	~Server();
	void update();
	void broadcast_order(Order& order);
	void close_all();
	uint32_t connections_count();

	static void update_servers();
};

#endif //STLIB_ETH
