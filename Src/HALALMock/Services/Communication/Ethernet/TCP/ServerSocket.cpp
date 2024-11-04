
#include "HALALMock/Services/Communication/Ethernet/TCP/ServerSocket.hpp"


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


ServerSocket::ServerSocket(ServerSocket&& other) : server_control_block(move(other.server_control_block)), local_ip(move(other.local_ip)), local_port(move(other.local_port))
, state(other.state){
	listening_sockets[local_port] = this;
	tx_packet_buffer = {};
	rx_packet_buffer = {};
}

void ServerSocket::operator=(ServerSocket&& other){
	local_ip = move(other.local_ip);
	local_port = move(other.local_port);
	server_control_block = move(other.server_control_block);
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
		struct pbuf* packet = rx_packet_buffer.front();
		rx_packet_buffer.pop();
		uint8_t* new_data = (uint8_t*)(packet->payload);
		tcp_recved(client_control_block, packet->tot_len);
		Order::process_data(this, new_data);
		pbuf_free(packet);
	}
}

bool ServerSocket::add_order_to_queue(Order& order){
	if(state == ACCEPTED){
		return false; //yet to decide if add_order_to_queue should send the order when used after the connection is accepted or just return false
	}
	struct memp* next_memory_pointer_in_packet_buffer_pool = (*(memp_pools[PBUF_POOL_MEMORY_DESC_POSITION]->tab))->next;
	if(next_memory_pointer_in_packet_buffer_pool == nullptr){
		memp_free_pool(memp_pools[PBUF_POOL_MEMORY_DESC_POSITION], next_memory_pointer_in_packet_buffer_pool);
		return false;
	}

	uint8_t* order_buffer = order.build();

	struct pbuf* packet = pbuf_alloc(PBUF_TRANSPORT, order.get_size(), PBUF_POOL);
	pbuf_take(packet, order_buffer, order.get_size());
	tx_packet_buffer.push(packet);
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
ServerSocket::create_server_socket(){
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
	if(bind(socket_fd, (struct sockaddr*)&server_socket_Address, sizeof(server_socket_Address)) < 0){
		std::cout<<"Bind error\n";
		close(socket_fd);
		return;
	}
}
ServerSocket::configure_server_socket(){
	//to reuse local address:
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    	std::cerr << "Error setting SO_REUSEADDR\n";
   		close(server_fd);
    	return;
	}
	//disable naggle algorithm
	int flag = 1;
	if (setsockopt(server_socket_fd,IPPROTO_TCP,TCP_NODELAY,(char *) &flag, sizeof(int)) < 0){
		std::cout<<"It has been an error disabling Nagle's algorithm\n";
		close(server_socket_fd);
		return;
	}
	//habilitate keepalives
    int optval = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
        std::cout << "ERROR configuring KEEPALIVES\n";
        close(server_socket_fd);
        return;
    }
	// Configurar TCP_KEEPIDLE it sets what time to wait to start sending keepalives 
    float tcp_keepidle_time = static_cast<float>(keepalive_config.inactivity_time_until_keepalive_ms)/1000.0;
    if (setsockopt(server_socket_fd, IPPROTO_TCP, TCP_KEEPIDLE, &tcp_keepidle_time, sizeof(tcp_keepidle_time)) < 0) {
        std::cout << "Error configuring TCP_KEEPIDLE\n";
		close(server_socket_fd);
        return;
    }
	  //interval between keepalives
    float keep_interval_time = static_cast<float>(keepalive_config.space_between_tries_ms)/1000.0;
    if (setsockopt(server_socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keep_interval_time, sizeof(keep_interval_time)) < 0) {
        std::cout << "Error configuring TCP_KEEPINTVL\n";
        close(server_socket_fd);
        return;
    }
	 // Configure TCP_KEEPCNT (number keepalives are send before considering the connection down)
	 float keep_cnt = static_cast<float>(keepalive_config.tries_until_disconnection)/1000.0;
    if (setsockopt(server_socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &keep_cnt, sizeof(keep_cnt)) < 0) {
        std::cout << "Error to configure TCP_KEEPCNT\n";
        close(server_socket_fd);
        return ;
    }
}
ServerSocket::configure_server_socket_and_listen(){
	create_server_socket();
	configure_server_socket();
	if (listen(socket_fd, SOMAXCONN) < 0) {
        std::cout"Error listening\n";
        close(socket_fd);
        state = INACTIVE;
        return;
    }
	state = LISTENING;
	listening_sockets[local_port] = this;
	//create a thread to listen
	listening_thread = std::jthread [&](){
		while(true){
			struct sockaddr_in client_addr;
			socklen_t client_len = sizeof(client_addr);
			int client_fd = accept(server_socket_fd,(struct sockaddr*)&client_addr, &client_len);
			if(client_fd > 0){
				clients.push_back(client_addr); 
				accept_callback(client_fd);
				OrderProtocol::sockets.push_back(this);
			}else{
				cout<< " Error accepting\n";
				close(socket_fd);
				state = CLOSED;
				return;
			}
		}
		
		
	}
}
ServerSocket::accept_callback(int client_fd){
	if(listening_sockets.contains(local_port)){
		ServerSocket* server_socket = listening_sockets[local_port];

		server_socket->state = ACCEPTED;
		server_socket->client_control_block = incomming_control_block;
		server_socket->remote_ip = IPV4(incomming_control_block->remote_ip);
		server_socket->rx_packet_buffer = {};

		tcp_setprio(incomming_control_block, priority);
		tcp_nagle_disable(incomming_control_block);
		ip_set_option(incomming_control_block, SOF_REUSEADDR);

		tcp_arg(incomming_control_block, server_socket);
		tcp_recv(incomming_control_block, receive_callback);
		tcp_sent(incomming_control_block, send_callback);
		tcp_err(incomming_control_block, error_callback);
		tcp_poll(incomming_control_block, poll_callback , 0);
		config_keepalive(incomming_control_block, server_socket);


		tcp_close(server_socket->server_control_block);
		priority++;

		return ERR_OK;
	}else
		return ERROR;

}
err_t ServerSocket::accept_callback(void* arg, struct tcp_pcb* incomming_control_block, err_t error){
	if(listening_sockets.contains(incomming_control_block->local_port)){
		ServerSocket* server_socket = listening_sockets[incomming_control_block->local_port];

		server_socket->state = ACCEPTED;
		server_socket->client_control_block = incomming_control_block;
		server_socket->remote_ip = IPV4(incomming_control_block->remote_ip);
		server_socket->rx_packet_buffer = {};

		tcp_setprio(incomming_control_block, priority);
		tcp_nagle_disable(incomming_control_block);
		ip_set_option(incomming_control_block, SOF_REUSEADDR);

		tcp_arg(incomming_control_block, server_socket);
		tcp_recv(incomming_control_block, receive_callback);
		tcp_sent(incomming_control_block, send_callback);
		tcp_err(incomming_control_block, error_callback);
		tcp_poll(incomming_control_block, poll_callback , 0);
		config_keepalive(incomming_control_block, server_socket);


		tcp_close(server_socket->server_control_block);
		priority++;

		return ERR_OK;
	}else
		return ERROR;
}

err_t ServerSocket::receive_callback(void* arg, struct tcp_pcb* client_control_block, struct pbuf* packet_buffer, err_t error){
	ServerSocket* server_socket = (ServerSocket*) arg;
	server_socket->client_control_block = client_control_block;

	if(packet_buffer == nullptr){      //FIN has been received
		server_socket->state = CLOSING;
		return ERR_OK;
	}

	if(error != ERR_OK){										//Check if packet is valid
		if(packet_buffer != nullptr){
			pbuf_free(packet_buffer);
		}
		return error;
	}
	else if(server_socket->state == ACCEPTED){
		server_socket->rx_packet_buffer.push(packet_buffer);
		server_socket->process_data();
		return ERR_OK;
	}

	else if(server_socket->state == CLOSING){ 		//Socket is already closed
		while(not server_socket->rx_packet_buffer.empty()){
			pbuf_free(server_socket->rx_packet_buffer.front());
			server_socket->rx_packet_buffer.pop();
		}
		server_socket->rx_packet_buffer = {};
		pbuf_free(packet_buffer);
		return ERR_OK;
	}
	return ERR_OK;
}

void ServerSocket::error_callback(void *arg, err_t error){
	ServerSocket* server_socket = (ServerSocket*) arg;
	server_socket->close();
	ErrorHandler("Socket error: %d. Socket closed", error);
}

err_t ServerSocket::poll_callback(void *arg, struct tcp_pcb *client_control_block){
	ServerSocket* server_socket = (ServerSocket*)arg;
	server_socket->client_control_block = client_control_block;


	if(server_socket == nullptr){    //Polling non existing pcb, fatal error
		tcp_abort(client_control_block);
		return ERR_ABRT;
	}

	while(not server_socket->tx_packet_buffer.empty()){		//TX FIFO is not empty
		server_socket->send();
	}

	while(not server_socket->rx_packet_buffer.empty()){		//RX FIFO is not empty
		server_socket->process_data();
	}

	if(server_socket->state == CLOSING){ //pcb has been polled to close
		server_socket->close();
	}

	return ERR_OK;
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


#endif
