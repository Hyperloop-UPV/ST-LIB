/*
 * Server.cpp
 *
 * Created on: Oct 12, 2023
 * 		Author: Ricardo
 */
#ifdef STLIB_ETH
#include "Communication/Server/Server.hpp"

vector<Server*> Server::running_servers = {};

Server::Server(IPV4 local_ip, uint32_t local_port)
    : local_ip(local_ip), local_port(local_port), status(RUNNING) {
    open_connection = new ServerSocket(local_ip, local_port);
    running_servers.push_back(this);
    running_connections = {};
    running_connections_count = 0;
}

Server::~Server() {
    open_connection->~ServerSocket();

    for (ServerSocket* s : running_connections) {
        s->~ServerSocket();
    }

    running_servers.erase(find(running_servers.begin(), running_servers.end(), this));
}

void Server::update() {
    if (open_connection->is_connected()) {
        running_connections[running_connections_count] = open_connection;
        running_connections_count++;
        open_connection = new ServerSocket(local_ip, local_port);
    }

    for (uint16_t s = 0; s < running_connections_count; s++) {
        if (status == RUNNING && !running_connections[s]->is_connected()) {
            ErrorHandler(
                "ip %s disconnected, going to FAULT",
                running_connections[s]->remote_ip.string_address.c_str()
            );
            status = CLOSING;
            break;
        }
    }

    if (status == CLOSING) {
        close_all();
    }
}

void Server::broadcast_order(Order& order) {
    for (uint16_t s = 0; s < running_connections_count; s++) {
        if (running_connections[s]->send_order(order)) {
            ErrorHandler(
                "Couldn t put Order %d into buffer of ip's %s ServerSocket, buffer may be full or "
                "the ServerSocket may be ill formed",
                order.get_id(),
                running_connections[s]->remote_ip.string_address.c_str()
            );
        }
    }
}

void Server::close_all() {
    for (uint16_t s = 0; s < running_connections_count; s++) {
        running_connections[s]->close();
        running_connections[s] = nullptr;
    }
    running_connections_count = 0;
    status = CLOSED;
}

uint32_t Server::connections_count() { return running_connections_count; }

void Server::update_servers() {
    for (Server* s : running_servers) {
        s->update();
    }
}

#endif // STLIB_ETH
