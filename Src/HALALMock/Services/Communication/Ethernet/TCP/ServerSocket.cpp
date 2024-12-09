
#include "HALALMock/Services/Communication/Ethernet/TCP/ServerSocket.hpp"

#include "HALALMock/Services/Logger/Logger.hpp"

#define MAX_SIZE_BUFFER 1024

uint8_t ServerSocket::priority = 1;
unordered_map<uint32_t, ServerSocket*> ServerSocket::listening_sockets = {};
ServerSocket::ServerSocket() = default;

ServerSocket::ServerSocket(IPV4 local_ip, uint32_t local_port)
    : local_ip(local_ip), local_port(local_port) {
    if (not Ethernet::is_running) {
        LOG_ERROR("Cannot declare UDP socket before Ethernet::start()");
        return;
    }
    tx_packet_buffer = {};
    rx_packet_buffer = {};
    state = INACTIVE;
    configure_server_socket_and_listen();
}

ServerSocket::ServerSocket(IPV4 local_ip, uint32_t local_port,
                           uint32_t inactivity_time_until_keepalive,
                           uint32_t space_between_tries,
                           uint32_t tries_until_disconnection)
    : ServerSocket(local_ip, local_port) {
    keepalive_config.inactivity_time_until_keepalive =
        inactivity_time_until_keepalive;
    keepalive_config.space_between_tries = space_between_tries;
    keepalive_config.tries_until_disconnection = tries_until_disconnection;
}

ServerSocket::ServerSocket(ServerSocket&& other)
    : local_ip(move(other.local_ip)),
      local_port(move(other.local_port)),
      state(other.state) {
    listening_sockets[local_port] = this;
    tx_packet_buffer = {};
    rx_packet_buffer = {};
}

void ServerSocket::operator=(ServerSocket&& other) {
    local_ip = move(other.local_ip);
    local_port = move(other.local_port);
    state = other.state;
    listening_sockets[local_port] = this;
    tx_packet_buffer = {};
    rx_packet_buffer = {};
    if (not(std::find(OrderProtocol::sockets.begin(),
                      OrderProtocol::sockets.end(),
                      this) != OrderProtocol::sockets.end()))
        OrderProtocol::sockets.push_back(this);
}

ServerSocket::~ServerSocket() {
    // el destructor no destruye
    auto it = std::find(OrderProtocol::sockets.begin(),
                        OrderProtocol::sockets.end(), this);
    if (it == OrderProtocol::sockets.end())
        return;
    else
        OrderProtocol::sockets.erase(it);
    // Clean all descriptors
    if (client_fd != -1) {
        ::close(client_fd);
        client_fd = -1;
    }
    if (server_socket_fd != -1) {
        ::close(server_socket_fd);
        server_socket_fd = -1;
    }
    // clean the transmisions buffers
    while (!tx_packet_buffer.empty()) {
        tx_packet_buffer.pop();
    }
    while (!rx_packet_buffer.empty()) {
        rx_packet_buffer.pop();
    }
    // eliminate the threads
    if (state == LISTENING) {
        ~listening_thread();
    } else if (state == ACCEPTED) {
        ~receive_thread();
    }
}

ServerSocket::ServerSocket(EthernetNode local_node)
    : ServerSocket(local_node.ip, local_node.port) {};

void ServerSocket::close() {
    // Clean all descriptors
    if (client_fd != -1) {
        ::close(client_fd);
        client_fd = -1;
    }
    if (server_socket_fd != -1) {
        ::close(server_socket_fd);
        server_socket_fd = -1;
    }
    // clean the transmisions buffers
    while (!tx_packet_buffer.empty()) {
        tx_packet_buffer.pop();
    }
    while (!rx_packet_buffer.empty()) {
        rx_packet_buffer.pop();
    }
    // eliminate the threads
    if (state == LISTENING) {
        ~listening_thread();
    } else if (state == ACCEPTED) {
        ~receive_thread();
    }
    listening_sockets[local_port] = this;
    state = CLOSED;
}

void ServerSocket::process_data() {
    while (!rx_packet_buffer.empty()) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            Packet* packet = rx_packet_buffer.front();
            rx_packet_buffer.pop();
        }
        uint8_t* new_data = (uint8_t*)(packet->build());
        Order::process_data(this, new_data);
    }
}

bool ServerSocket::add_order_to_queue(Order& order) {
    if (state == ACCEPTED) {
        return false;
    }
    if (order.get_size() == 0) {
        LOG_ERROR("Order is empty");
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(mutex);
        tx_packet_buffer.push(order);
    }
    return true;
}

void ServerSocket::send() {
    std::lock_guard<std::mutex> lock(mutex);
    while (!tx_packet_buffer.empty()) {
        Packet* packet = tx_packet_buffer.front();
        ssize_t sent_bytes =
            ::send(client_fd, packet->build(), packet->get_size(), 0);
        if (sent_bytes < 0) {
            LOG_ERROR(std::format("Error sending packet {}", packet->get_id()));
            state = CLOSING;
            close();
            return;
        }
        tx_packet_buffer.pop();
    }
}

bool ServerSocket::is_connected() {
    return state == ServerSocket::ServerState::ACCEPTED;
}
void ServerSocket::create_server_socket() {
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd == -1) {
        LOG_ERROR("Socket creation failure");
        return;
    }
    // inset the local address and port
    struct sockaddr_in server_socket_Address;
    server_socket_Address.sin_family = AF_INET;
    server_socket_Address.sin_addr.s_addr = local_ip.address;
    server_socket_Address.sin_port = htons(local_port);
    if (bind(server_socket_fd, (struct sockaddr*)&server_socket_Address,
             sizeof(server_socket_Address)) < 0) {
        LOG_ERROR(std::format("Couldn't bind to address {} in port {}",
                              local_ip.string_address, local_port));
        close();
        return;
    }
}
bool ServerSocket::configure_server_socket() {
    // to reuse local address:
    int opt = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
                   sizeof(opt)) < 0) {
        LOG_ERROR("Couldn't set SO_REUSEADDR");
        close();
        return false;
    }
    // disable naggle algorithm
    int flag = 1;
    if (setsockopt(server_socket_fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag,
                   sizeof(int)) < 0) {
        LOG_ERROR("Can't disable Nagle's algorithm");
        return false;
    }
    // habilitate keepalives
    int optval = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_KEEPALIVE, &optval,
                   sizeof(optval)) < 0) {
        LOG_ERROR("Can't configure KEEPALIVES");
        return false;
    }
    // Configure TCP_KEEPIDLE it sets what time to wait to start sending
    // keepalives
    // different from lwip to linux
    uint32_t tcp_keepidle_time =
        stkeepalive_config.inactivity_time_until_keepalive;
    if (setsockopt(server_socket_fd, IPPROTO_TCP, TCP_KEEPIDLE,
                   &tcp_keepidle_time, sizeof(tcp_keepidle_time)) < 0) {
        LOG_ERROR("Can't configure TCP_KEEPIDLE");
        return false;
    }
    // interval between keepalives
    uint32_t keep_interval_time = keepalive_config.space_between_tries;
    if (setsockopt(server_socket_fd, IPPROTO_TCP, TCP_KEEPINTVL,
                   &keep_interval_time, sizeof(keep_interval_time)) < 0) {
        LOG_ERROR("Can't configure TCP_KEEPINTVL");
        return false;
    }
    // Configure TCP_KEEPCNT (number keepalives are send before considering the
    // connection down)
    uint32_t keep_cnt = keepalive_config.tries_until_disconnection;
    if (setsockopt(server_socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &keep_cnt,
                   sizeof(keep_cnt)) < 0) {
        LOG_ERROR("Can't configure TCP_KEEPCNT");
        return false;
    }
    return true;
}
void ServerSocket::configure_server_socket_and_listen() {
    create_server_socket();
    if (!configure_server_socket()) {
        LOG_ERROR("Can't configure ServerSocket");
        close();
        return;
    }
    if (listen(server_socket_fd, SOMAXCONN) < 0) {
        LOG_ERROR("Can't listen");
        close();
        return;
    }
    state = LISTENING;
    listening_sockets[local_port] = this;
    // create a thread to listen
    listening_thread = std::jthread[&]() {
        // solo aceptamos una conexion
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        client_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr,
                           &client_len);
        if (client_fd > 0) {
            if (!accept_callback(client_fd, client_addr)) {
                LOG_ERROR("Something went wrong in accept_callback");
            } else {
                OrderProtocol::sockets.push_back(this);
            }

        } else {
            LOG_ERROR("Can't accept");
            close();
            return;
        }
    }
}
bool ServerSocket::accept_callback(int client_fd, sockaddr_in client_address) {
    if (listening_sockets.contains(local_port) && state == LISTENING) {
        state = ACCEPTED;
        remote_ip = IPV4(client_address.sin_addr.s_addr);
        rx_packet_buffer = {};
        handle_receive_from_client(client_fd);
        return true;
    } else {
        return false;
    }
}
void ServerSocket::handle_receive_from_client(int client_fd) {
    receive_thread = std::jthread([client_fd]() {
        uint8_t buffer[BUFFER_SIZE];  // Buffer for the data
        ssize_t bytes_received;
        while ((bytes_received = recv(client_fd, buffer, sizeof(buffer), 0)) >
                   0 &&
               state == ACCEPTED) {
            Packet* packet;
            packet->parse(*buffer);
            {
                std::lock_guard<std::mutex> lock(mtx);
                rx_packet_buffer.push(&packet);
                process_data();
            }
        }
        // if receive a 0 means that the client has finished the connection so
        // we will close this server_socket
        if (bytes_received == 0) {
            LOG_WARNING("Client disconnected");
        } else if (bytes_received < 0) {
            LOG_ERROR("Error receiving data");
        }
        close();
    });
}
