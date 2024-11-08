#include "HALALMock/Services/Communication/Ethernet/TCP/ServerSocket.hpp"

#define MAX_SIZE_BUFFER 1024

uint8_t ServerSocket::priority = 1;
unordered_map<uint32_t,ServerSocket*> ServerSocket::listening_sockets = {};
ServerSocket::ServerSocket() = default;

ServerSocket::ServerSocket(IPV4 local_ip, uint32_t local_port) : local_ip(local_ip),local_port(local_port){
	if(not Ethernet::is_running) {
		cout<<"Cannot declare UDP socket before Ethernet::start()\n";
		return;
	}
	tx_packet_buffer = {};
	rx_packet_buffer = {};
	state = INACTIVE;
	configure_server_socket_and_listen();
}


ServerSocket::ServerSocket(IPV4 local_ip, uint32_t local_port, uint32_t inactivity_time_until_keepalive_ms, uint32_t space_between_tries_ms, uint32_t tries_until_disconnection): ServerSocket(local_ip, local_port){
	keepalive_config.inactivity_time_until_keepalive_ms = inactivity_time_until_keepalive_ms;
	keepalive_config.space_between_tries_ms = space_between_tries_ms;
	keepalive_config.tries_until_disconnection = tries_until_disconnection;
}


ServerSocket::ServerSocket(ServerSocket&& other) :  local_ip(move(other.local_ip)), local_port(move(other.local_port)),state(other.state){
	listening_sockets[local_port] = this;
	tx_packet_buffer = {};
	rx_packet_buffer = {};
}

void ServerSocket::operator=(ServerSocket&& other){
	local_ip = move(other.local_ip);
	local_port = move(other.local_port);
	state = other.state;
	listening_sockets[local_port] = this;
	tx_packet_buffer = {};
	rx_packet_buffer = {};
	if(not (std::find(OrderProtocol::sockets.begin(), OrderProtocol::sockets.end(), this) != OrderProtocol::sockets.end()))
		OrderProtocol::sockets.push_back(this);
}

ServerSocket::~ServerSocket(){
	//el destructor no destruye
	auto it = std::find(OrderProtocol::sockets.begin(), OrderProtocol::sockets.end(), this);
	if(it == OrderProtocol::sockets.end()) return;
	else OrderProtocol::sockets.erase(it);
	tcp_abort(client_control_block);
	tcp_abort(server_control_block);
	while(!tx_packet_buffer.empty()){
		free(tx_packet_buffer.front());
		tx_packet_buffer.pop();
	}
	while(!rx_packet_buffer.empty()){
		free(rx_packet_buffer.front());
		rx_packet_buffer.pop();
	}

}

ServerSocket::ServerSocket(EthernetNode local_node) : ServerSocket(local_node.ip,local_node.port){};

void ServerSocket::close(){
	// Clean all callbacks
	tcp_arg(client_control_block, nullptr);
	tcp_sent(client_control_block, nullptr);
	tcp_recv(client_control_block, nullptr);
	tcp_err(client_control_block, nullptr);
	tcp_poll(client_control_block, nullptr, 0);

	tcp_close(client_control_block);
	while(!tx_packet_buffer.empty()){
		pbuf_free(tx_packet_buffer.front());
		tx_packet_buffer.pop();
	}
	while(!rx_packet_buffer.empty()){
		pbuf_free(rx_packet_buffer.front());
		rx_packet_buffer.pop();
	}

    tcp_pcb_remove(&tcp_active_pcbs, client_control_block);
    tcp_free(client_control_block);

	listening_sockets[local_port] = this;
	state = CLOSED;

	priority--;

}

void ServerSocket::process_data(){
	while(!rx_packet_buffer.empty()){
		{
			std::lock_guard<std::mutex> lock(mtx); 
			Packet *packet = rx_packet_buffer.front();
			rx_packet_buffer.pop();
		}
		uint8_t* new_data = (uint8_t*)(packet->build());
		Order::process_data(this, new_data);
	}
}

bool ServerSocket::add_order_to_queue(Order& order){
	if(state == ACCEPTED){
		return false; 
	}
    if (order.get_size() == 0) {
        std::cout << "Error: order is empty\n";
        return false; 
    }
    {
        std::lock_guard<std::mutex> lock(mutex); 
        tx_packet_buffer.push(order); 
    }
	return true;
}

void ServerSocket::send(){
	pbuf* temporal_packet_buffer;
	err_t error = ERR_OK;
	while(error == ERR_OK && !tx_packet_buffer.empty() && tx_packet_buffer.front()->len <= tcp_sndbuf(client_control_block)){
		temporal_packet_buffer = tx_packet_buffer.front();
		error = tcp_write(client_control_block, temporal_packet_buffer->payload, temporal_packet_buffer->len, TCP_WRITE_FLAG_COPY);
		if(error == ERR_OK){
			tx_packet_buffer.pop();
			tcp_output(client_control_block);
			memp_free_pool(memp_pools[PBUF_POOL_MEMORY_DESC_POSITION],temporal_packet_buffer);
		}else{
			ErrorHandler("Cannot write to socket, error: %d", error);
		}
	}
}

bool ServerSocket::is_connected(){
	return state == ServerSocket::ServerState::ACCEPTED;
}
void ServerSocket::create_server_socket(){
	server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket_fd == -1){
		std::cout<<"Socket creation failure\n";
		return;
	}
	//inset the local address and port
	struct sockaddr_in server_socket_Address;
	server_socket_Address.sin_family = AF_INET;
	server_socket_Address.sin_addr.s_addr = local_ip.address;
	server_socket_Address.sin_port = htons(local_port);
	if(bind(server_socket_fd, (struct sockaddr*)&server_socket_Address, sizeof(server_socket_Address)) < 0){
		std::cout<<"Bind error\n";
		close(server_socket_fd);
		return;
	}
}
bool ServerSocket::configure_server_socket(){
	//to reuse local address:
	int opt = 1;
	if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    	std::cerr << "Error setting SO_REUSEADDR\n";
   		close(server_socket_fd);
    	return false;
	}
	//disable naggle algorithm
	int flag = 1;
	if (setsockopt(server_socket_fd,IPPROTO_TCP,TCP_NODELAY,(char *) &flag, sizeof(int)) < 0){
		std::cout<<"It has been an error disabling Nagle's algorithm\n";
		close(server_socket_fd);
		return false;
	}
	//habilitate keepalives
    int optval = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
        std::cout << "ERROR configuring KEEPALIVES\n";
        close(server_socket_fd);
        return false;
    }
	// Configurar TCP_KEEPIDLE it sets what time to wait to start sending keepalives 
    float tcp_keepidle_time = static_cast<float>(keepalive_config.inactivity_time_until_keepalive_ms)/1000.0;
    if (setsockopt(server_socket_fd, IPPROTO_TCP, TCP_KEEPIDLE, &tcp_keepidle_time, sizeof(tcp_keepidle_time)) < 0) {
        std::cout << "Error configuring TCP_KEEPIDLE\n";
		close(server_socket_fd);
        return false;
    }
	  //interval between keepalives
    float keep_interval_time = static_cast<float>(keepalive_config.space_between_tries_ms)/1000.0;
    if (setsockopt(server_socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keep_interval_time, sizeof(keep_interval_time)) < 0) {
        std::cout << "Error configuring TCP_KEEPINTVL\n";
        close(server_socket_fd);
        return false;
    }
	 // Configure TCP_KEEPCNT (number keepalives are send before considering the connection down)
	 float keep_cnt = static_cast<float>(keepalive_config.tries_until_disconnection)/1000.0;
    if (setsockopt(server_socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &keep_cnt, sizeof(keep_cnt)) < 0) {
        std::cout << "Error to configure TCP_KEEPCNT\n";
        close(server_socket_fd);
        return false;
    }
	return true;
}
void ServerSocket::configure_server_socket_and_listen(){
	create_server_socket();
	if(!configure_server_socket()){
		cout<<"Error configuring ServerSocket\n";
		return;
	}
	if (listen(server_socket_fd, SOMAXCONN) < 0) {
        std::cout"Error listening\n";
        close(server_socket_fd);
        state = INACTIVE;
        return;
    }
	state = LISTENING;
	listening_sockets[local_port] = this;
	//create a thread to listen
	listening_thread = std::jthread [&](){
		while(state == LISTENING){
			struct sockaddr_in client_addr;
			socklen_t client_len = sizeof(client_addr);
			int client_fd = accept(server_socket_fd,(struct sockaddr*)&client_addr, &client_len);
			if(client_fd > 0){
				clients.push_back(client_addr); 
				if(!accept_callback(client_fd,client_addr)){
					cout<<"Something went wrong in accept_callback\n";
				}
				OrderProtocol::sockets.push_back(this);
			}else{
				cout<< "Error accepting\n";
				close(server_socket_fd);
				state = CLOSED;
				return;
			}
		}
	}
}
bool ServerSocket::accept_callback(int client_fd, sockaddr_in client_address){
	if(listening_sockets.contains(local_port)){
		ServerSocket* server_socket = listening_sockets[local_port];
		server_socket->state = ACCEPTED;
		server_socket->client_fd = client_fd;
		server_socket->remote_ip = IPV4(client_address.sin_addr.s_addr);
		server_socket->rx_packet_buffer = {};
		server_socket->handle_receive_from_client(client_fd);
		return true;
	}else{
		return false;
	}
	
}
void ServerSocket::handle_receive_from_client(int client_fd){
	receive_thread = std::jthread([client_fd]() {
        
		uint8_t buffer[BUFFER_SIZE]; // Buffer for the data
        ssize_t bytes_received;
        while ((bytes_received = recv(client_fd, buffer, sizeof(buffer), 0)) > 0 && state == ACCEPTED){
			Packet* packet;
			packet->parse(buffer);
			 {
                std::lock_guard<std::mutex> lock(mtx); 
                rx_packet_buffer.push(std::move(packet));
				process_data(); 
            }
        }
		//if receive a 0 means that the client has finished the connection so we will close this server_socket
        if (bytes_received == 0) {
            std::cout << "Client disconnected\n";
			
        } else if (bytes_received < 0) {
            cout << "Error receiving data\n";
		}
		close();
    });
}


err_t ServerSocket::send_callback(void *arg, struct tcp_pcb *client_control_block, u16_t len){
	ServerSocket* server_socket = (ServerSocket*)arg;
	server_socket->client_control_block = client_control_block;
	if(!server_socket->tx_packet_buffer.empty()){
		server_socket->send();
	}
	else if(server_socket->state == CLOSING){
		server_socket->close();
	}
	return ERR_OK;
}

void ServerSocket::config_keepalive(tcp_pcb* control_block, ServerSocket* server_socket){
	control_block->so_options |= SOF_KEEPALIVE;
	control_block->keep_idle = server_socket->keepalive_config.inactivity_time_until_keepalive_ms;
	control_block->keep_intvl = server_socket->keepalive_config.space_between_tries_ms;
	control_block->keep_cnt = server_socket->keepalive_config.tries_until_disconnection;
}

