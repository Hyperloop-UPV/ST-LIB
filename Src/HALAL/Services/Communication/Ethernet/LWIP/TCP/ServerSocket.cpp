/*
 * ServerSocket.cpp
 *
 *  Created on: Nov 23, 2022
 *      Author: stefa
 *  Edited on: Nov 6, 2026
 *      Editor: aleks
 */
#ifdef STLIB_ETH
#include "HALAL/Services/Communication/Ethernet/LWIP/TCP/ServerSocket.hpp"
#include "HALAL/Services/Communication/Ethernet/LWIP/TCP/TcpOrderStreamParser.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "lwip/priv/tcp_priv.h"
#include "ST-LIB_HIGH/Protections/ProtectionMessages.hpp"
#ifdef HAL_ETH_MODULE_ENABLED

uint8_t ServerSocket::priority = 1;
unordered_map<uint32_t, ServerSocket*> ServerSocket::listening_sockets = {};

ServerSocket::ServerSocket() = default;

ServerSocket::ServerSocket(IPV4 local_ip, uint32_t local_port)
    : local_ip(local_ip), local_port(local_port) {
    if (not Ethernet::is_running) {
        PANIC("Cannot declare TCP server socket before Ethernet::start()");
        return;
    }
    tx_packet_buffer = {};
    rx_packet_buffer = {};
    rx_stream_buffer = {};
    rx_stream_buffer.reserve(TcpOrderStreamParser::MAX_RX_STREAM_BUFFER_BYTES);
    state = INACTIVE;
    server_control_block = tcp_new();
    if (server_control_block == nullptr) {
        PANIC("Cannot allocate TCP server control block");
        return;
    }
    tcp_nagle_disable(server_control_block);
    ip_set_option(server_control_block, SOF_REUSEADDR);
    err_t error = tcp_bind(server_control_block, &local_ip.address, local_port);

    if (error == ERR_OK) {
        server_control_block = tcp_listen(server_control_block);
        if (server_control_block == nullptr) {
            PANIC("Cannot switch TCP server socket into LISTEN mode");
            return;
        }
        state = LISTENING;
        listening_sockets[local_port] = this;
        tcp_arg(server_control_block, this);
        tcp_accept(server_control_block, accept_callback);
    } else {
        tcp_abort(server_control_block);
        server_control_block = nullptr;
        PANIC("Cannot bind server socket, error %d", (int16_t)error);
        return;
    }
    if (std::find(OrderProtocol::sockets.begin(), OrderProtocol::sockets.end(), this) ==
        OrderProtocol::sockets.end()) {
        OrderProtocol::sockets.push_back(this);
    }
}

ServerSocket::ServerSocket(
    IPV4 local_ip,
    uint32_t local_port,
    uint32_t inactivity_time_until_keepalive_ms,
    uint32_t space_between_tries_ms,
    uint32_t tries_until_disconnection
)
    : ServerSocket(local_ip, local_port) {
    keepalive_config.inactivity_time_until_keepalive_ms = inactivity_time_until_keepalive_ms;
    keepalive_config.space_between_tries_ms = space_between_tries_ms;
    keepalive_config.tries_until_disconnection = tries_until_disconnection;
}

ServerSocket::ServerSocket(ServerSocket&& other)
    : local_ip(move(other.local_ip)), local_port(other.local_port),
      remote_ip(move(other.remote_ip)), state(other.state),
      keepalive_config(other.keepalive_config), server_control_block(other.server_control_block),
      tx_packet_buffer(move(other.tx_packet_buffer)),
      rx_packet_buffer(move(other.rx_packet_buffer)),
      rx_stream_buffer(move(other.rx_stream_buffer)),
      client_control_block(other.client_control_block) {
    other.server_control_block = nullptr;
    other.client_control_block = nullptr;
    other.state = INACTIVE;

    if (server_control_block != nullptr) {
        tcp_arg(server_control_block, this);
    }
    if (client_control_block != nullptr) {
        tcp_arg(client_control_block, this);
    }

    auto it = listening_sockets.find(local_port);
    if (it != listening_sockets.end() && it->second == &other) {
        it->second = this;
    }

    if (std::find(OrderProtocol::sockets.begin(), OrderProtocol::sockets.end(), this) ==
        OrderProtocol::sockets.end()) {
        OrderProtocol::sockets.push_back(this);
    }
}

void ServerSocket::operator=(ServerSocket&& other) {
    if (this == &other) {
        return;
    }
    close();

    local_ip = move(other.local_ip);
    local_port = other.local_port;
    remote_ip = move(other.remote_ip);
    server_control_block = other.server_control_block;
    client_control_block = other.client_control_block;
    tx_packet_buffer = move(other.tx_packet_buffer);
    rx_packet_buffer = move(other.rx_packet_buffer);
    rx_stream_buffer = move(other.rx_stream_buffer);
    keepalive_config = other.keepalive_config;
    state = other.state;

    other.server_control_block = nullptr;
    other.client_control_block = nullptr;
    other.state = INACTIVE;

    if (server_control_block != nullptr) {
        tcp_arg(server_control_block, this);
    }
    if (client_control_block != nullptr) {
        tcp_arg(client_control_block, this);
    }

    auto it = listening_sockets.find(local_port);
    if (it != listening_sockets.end() && it->second == &other) {
        it->second = this;
    }

    if (std::find(OrderProtocol::sockets.begin(), OrderProtocol::sockets.end(), this) ==
        OrderProtocol::sockets.end()) {
        OrderProtocol::sockets.push_back(this);
    }
}

void ServerSocket::clear_packet_queues() {
    while (!tx_packet_buffer.empty()) {
        pbuf* packet = tx_packet_buffer.front();
        tx_packet_buffer.pop();
        if (packet != nullptr) {
            pbuf_free(packet);
        }
    }
    while (!rx_packet_buffer.empty()) {
        pbuf* packet = rx_packet_buffer.front();
        rx_packet_buffer.pop();
        if (packet != nullptr) {
            pbuf_free(packet);
        }
    }
}

ServerSocket::~ServerSocket() {
    close();

    auto listener_it = listening_sockets.find(local_port);
    if (listener_it != listening_sockets.end() && listener_it->second == this) {
        listening_sockets.erase(listener_it);
    }

    auto it = std::find(OrderProtocol::sockets.begin(), OrderProtocol::sockets.end(), this);
    if (it != OrderProtocol::sockets.end()) {
        OrderProtocol::sockets.erase(it);
    }
}

ServerSocket::ServerSocket(EthernetNode local_node)
    : ServerSocket(local_node.ip, local_node.port){};

void ServerSocket::close() {
    if (client_control_block != nullptr) {
        // Clean all callbacks
        tcp_arg(client_control_block, nullptr);
        tcp_sent(client_control_block, nullptr);
        tcp_recv(client_control_block, nullptr);
        tcp_err(client_control_block, nullptr);
        tcp_poll(client_control_block, nullptr, 0);

        err_t close_error = tcp_close(client_control_block);
        if (close_error != ERR_OK) {
            tcp_abort(client_control_block);
        }
        client_control_block = nullptr;
    }
    if (server_control_block != nullptr) {
        tcp_arg(server_control_block, nullptr);
        tcp_accept(server_control_block, nullptr);
        err_t close_error = tcp_close(server_control_block);
        if (close_error != ERR_OK) {
            tcp_abort(server_control_block);
        }
        server_control_block = nullptr;
    }
    clear_packet_queues();
    rx_stream_buffer.clear();
    state = CLOSED;

    ST_LIB::ProtectionADJv3Packet::tpcb =
        nullptr; // Protection from hard Fault (idk how it works by the moment)
    ST_LIB::ProtectionADJv3Packet::is_connected = false;

    auto listener_it = listening_sockets.find(local_port);
    if (listener_it != listening_sockets.end() && listener_it->second == this) {
        listening_sockets.erase(listener_it);
    }

    if (priority > 1) {
        priority--;
    }
}

void ServerSocket::process_data() {
    while (!rx_packet_buffer.empty()) {
        pbuf* packet = rx_packet_buffer.front();
        rx_packet_buffer.pop();
        if (packet == nullptr) {
            continue;
        }

        if (client_control_block != nullptr) {
            tcp_recved(client_control_block, packet->tot_len);
        }

        const size_t previous_size = rx_stream_buffer.size();
        const size_t append_size = packet->tot_len;
        rx_stream_buffer.resize(previous_size + append_size);
        if (pbuf_copy_partial(
                packet,
                rx_stream_buffer.data() + previous_size,
                packet->tot_len,
                0
            ) == static_cast<u16_t>(packet->tot_len)) {
            TcpOrderStreamParser::process(this, remote_ip, rx_stream_buffer);
        } else {
            rx_stream_buffer.resize(previous_size);
        }
        pbuf_free(packet);
    }
}

bool ServerSocket::add_order_to_queue(Order& order) {
    if (state != ACCEPTED || client_control_block == nullptr) {
        return false;
    }
    if (tx_packet_buffer.size() >= MAX_TX_QUEUE_DEPTH) {
        return false;
    }

    const size_t order_size = order.get_size();
    if (order_size == 0 || order_size > TCP_SND_BUF) {
        return false;
    }

    uint8_t* order_buffer = order.build();

    pbuf* packet = pbuf_alloc(PBUF_RAW, order_size, PBUF_RAM);
    if (packet == nullptr) {
        return false;
    }
    if (pbuf_take(packet, order_buffer, order_size) != ERR_OK) {
        pbuf_free(packet);
        return false;
    }
    tx_packet_buffer.push(packet);
    return true;
}

bool ServerSocket::try_send_immediately(Order& order) {
    if (state != ACCEPTED || client_control_block == nullptr || !tx_packet_buffer.empty()) {
        return false;
    }

    const size_t order_size = order.get_size();
    if (order_size == 0 || order_size > TCP_SND_BUF ||
        order_size > tcp_sndbuf(client_control_block)) {
        return false;
    }

    uint8_t* order_buffer = order.build();
    if (order_buffer == nullptr) {
        return false;
    }

    err_t error = tcp_write(client_control_block, order_buffer, order_size, TCP_WRITE_FLAG_COPY);
    if (error == ERR_OK) {
        if (client_control_block != nullptr) {
            tcp_output(client_control_block);
        }
        return true;
    }
    if (error == ERR_MEM) {
        return false;
    }

    state = CLOSING;
    return false;
}

bool ServerSocket::send_order(Order& order) {
    if (state != ACCEPTED || client_control_block == nullptr) {
        return false;
    }

    if (try_send_immediately(order)) {
        return true;
    }

    if (!add_order_to_queue(order)) {
        // One opportunistic flush avoids false negatives when TX queue is momentarily full.
        send();
        if (!add_order_to_queue(order)) {
            return false;
        }
    }
    send();
    return true;
}

void ServerSocket::send() {
    if (client_control_block == nullptr || state != ACCEPTED) {
        return;
    }

    while (!tx_packet_buffer.empty()) {
        pbuf* temporal_packet_buffer = tx_packet_buffer.front();
        if (temporal_packet_buffer == nullptr) {
            tx_packet_buffer.pop();
            continue;
        }

        if (temporal_packet_buffer->tot_len > tcp_sndbuf(client_control_block)) {
            break;
        }

        err_t error = tcp_write(
            client_control_block,
            temporal_packet_buffer->payload,
            temporal_packet_buffer->tot_len,
            TCP_WRITE_FLAG_COPY
        );
        if (error == ERR_OK) {
            tx_packet_buffer.pop();
            pbuf_free(temporal_packet_buffer);
        } else if (error == ERR_MEM) {
            break;
        } else {
            state = CLOSING;
            break;
        }
    }
    if (client_control_block != nullptr) {
        tcp_output(client_control_block);
    }
}

bool ServerSocket::is_connected() { return state == ServerSocket::ServerState::ACCEPTED; }

bool ServerSocket::is_listening() const { return state == ServerSocket::ServerState::LISTENING; }

err_t ServerSocket::accept_callback(
    void* arg,
    struct tcp_pcb* incoming_control_block,
    err_t error
) {
    if (error != ERR_OK || incoming_control_block == nullptr) {
        if (incoming_control_block != nullptr) {
            tcp_abort(incoming_control_block);
        }
        return error;
    }

    ServerSocket* server_socket = static_cast<ServerSocket*>(arg);
    if (server_socket == nullptr) {
        auto it = listening_sockets.find(incoming_control_block->local_port);
        if (it == listening_sockets.end()) {
            tcp_abort(incoming_control_block);
            return ERR_ABRT;
        }
        server_socket = it->second;
    }

    server_socket->state = ACCEPTED;
    server_socket->client_control_block = incoming_control_block;
    server_socket->remote_ip = IPV4(incoming_control_block->remote_ip);
    server_socket->rx_packet_buffer = {};
    server_socket->rx_stream_buffer.clear();
    server_socket->rx_stream_buffer.reserve(TcpOrderStreamParser::MAX_RX_STREAM_BUFFER_BYTES);

    tcp_setprio(incoming_control_block, priority);
    tcp_nagle_disable(incoming_control_block);
    ip_set_option(incoming_control_block, SOF_REUSEADDR);

    tcp_arg(incoming_control_block, server_socket);
    tcp_recv(incoming_control_block, receive_callback);
    tcp_sent(incoming_control_block, send_callback);
    tcp_err(incoming_control_block, error_callback);
    tcp_poll(incoming_control_block, poll_callback, 1);
    config_keepalive(incoming_control_block, server_socket);

    if (server_socket->server_control_block != nullptr) {
        tcp_arg(server_socket->server_control_block, nullptr);
        tcp_accept(server_socket->server_control_block, nullptr);
        err_t close_error = tcp_close(server_socket->server_control_block);
        if (close_error != ERR_OK) {
            tcp_abort(server_socket->server_control_block);
        }
        server_socket->server_control_block = nullptr;
    }

    if (priority < 255) {
        priority++;
    }

    return ERR_OK;
}

err_t ServerSocket::receive_callback(
    void* arg,
    struct tcp_pcb* client_control_block,
    struct pbuf* packet_buffer,
    err_t error
) {
    ServerSocket* server_socket = static_cast<ServerSocket*>(arg);
    if (server_socket == nullptr) {
        if (packet_buffer != nullptr) {
            pbuf_free(packet_buffer);
        }
        return ERR_VAL;
    }
    server_socket->client_control_block = client_control_block;

    if (packet_buffer == nullptr) { // FIN has been received
        server_socket->state = CLOSING;
        return ERR_OK;
    }

    if (error != ERR_OK) { // Check if packet is valid
        if (packet_buffer != nullptr) {
            pbuf_free(packet_buffer);
        }
        // Keep the socket alive on transient lwIP receive errors.
        return ERR_OK;
    }
    if (server_socket->state == ACCEPTED) {
        server_socket->rx_packet_buffer.push(packet_buffer);
        server_socket->process_data();
        return ERR_OK;
    }

    pbuf_free(packet_buffer);
    return ERR_OK;
}

void ServerSocket::error_callback(void* arg, err_t error) {
    (void)error;
    ServerSocket* server_socket = static_cast<ServerSocket*>(arg);
    if (server_socket == nullptr) {
        return;
    }

    // tcp_err is called once the pcb is already freed by lwIP.
    server_socket->client_control_block = nullptr;
    server_socket->state = CLOSED;
    server_socket->clear_packet_queues();
    server_socket->rx_stream_buffer.clear();
}

err_t ServerSocket::poll_callback(void* arg, struct tcp_pcb* client_control_block) {
    ServerSocket* server_socket = static_cast<ServerSocket*>(arg);
    if (server_socket == nullptr) { // Polling non existing pcb, fatal error
        tcp_abort(client_control_block);
        return ERR_ABRT;
    }
    server_socket->client_control_block = client_control_block;

    if (!server_socket->tx_packet_buffer.empty()) { // TX FIFO is not empty
        server_socket->send();
    }

    if (!server_socket->rx_packet_buffer.empty()) { // RX FIFO is not empty
        server_socket->process_data();
    }

    if (server_socket->state == CLOSING) { // pcb has been polled to close
        server_socket->close();
    }

    return ERR_OK;
}

err_t ServerSocket::send_callback(void* arg, struct tcp_pcb* client_control_block, u16_t len) {
    (void)len;
    ServerSocket* server_socket = static_cast<ServerSocket*>(arg);
    if (server_socket == nullptr) {
        tcp_abort(client_control_block);
        return ERR_ABRT;
    }
    server_socket->client_control_block = client_control_block;
    if (!server_socket->tx_packet_buffer.empty()) {
        server_socket->send();
    } else if (server_socket->state == CLOSING) {
        server_socket->close();
    }
    return ERR_OK;
}

void ServerSocket::config_keepalive(tcp_pcb* control_block, ServerSocket* server_socket) {
    control_block->so_options |= SOF_KEEPALIVE;
    control_block->keep_idle = server_socket->keepalive_config.inactivity_time_until_keepalive_ms;
    control_block->keep_intvl = server_socket->keepalive_config.space_between_tries_ms;
    control_block->keep_cnt = server_socket->keepalive_config.tries_until_disconnection;
}

#endif // HAL_ETH_MODULE_ENABLED
#endif // STLIB_ETH
