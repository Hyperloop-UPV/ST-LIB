
#include "HALALMock/Services/Communication/Ethernet/TCP/Socket.hpp"
#ifdef HAL_ETH_MODULE_ENABLED

unordered_map<EthernetNode,Socket*> Socket::connecting_sockets = {};


Socket::Socket() = default;

Socket::Socket(Socket&& other):remote_port(move(remote_port)),
	 state(other.state){
	EthernetNode remote_node(other.remote_ip, other.remote_port);
	connecting_sockets[remote_node] = this;
}

void Socket::operator=(Socket&& other){
	remote_port = move(other.remote_port);
	state = other.state;
	EthernetNode remote_node(other.remote_ip, other.remote_port);
	connecting_sockets[remote_node] = this;
	if(std::find(OrderProtocol::sockets.begin(), OrderProtocol::sockets.end(), this) == OrderProtocol::sockets.end())
		OrderProtocol::sockets.push_back(this);
}

Socket::~Socket(){
	auto it = std::find(OrderProtocol::sockets.begin(), OrderProtocol::sockets.end(), this);
	if(it == OrderProtocol::sockets.end()) return;
	else OrderProtocol::sockets.erase(it);
}

Socket::Socket(IPV4 local_ip, uint32_t local_port, IPV4 remote_ip, uint32_t remote_port,bool use_keep_alive):
		local_ip(local_ip), local_port(local_port),remote_ip(remote_ip), remote_port(remote_port),use_keep_alives{use_keep_alive}
{
	if(not Ethernet::is_running) {
		std::cout<<"Cannot declare TCP socket before Ethernet::start()";
		return;
	}
	state = INACTIVE;
	tx_packet_buffer = {};
	rx_packet_buffer = {};
	EthernetNode remote_node(remote_ip, remote_port);
	tcp_connection_sim();
	OrderProtocol::sockets.push_back(this);
}
Socket::tcp_connection_sim(){
	//create socket
	socket_fd = ::socket(AF_INET,SOCK_STREAM,0);
	//create socket non-blocking
	int status = fcntl(socketfd, F_SETFL, fcntl(socketfd, F_GETFL, 0) | O_NONBLOCK);
	if(status == -1){
		std::cout<< "Error calling fcntl\n";
		close(socket_fd);
		return;
	}
	//inset the local address and port
	struct sockadd_in socket_Address;
	socket_Address.sin_family = AF_INET;
	socket_Address.sin_addr.s_addr = inet_addr(local_ip);
	socket_Address.sin_port = htons(local_port);
	if(bind(socket_fd, (struct sockaddr*)&socket_Address, sizeof(socket_Address)) < 0){
		std::cout<<"Bind error\n";
		close(socket_fd);
		return;
	}
	//disable naggle algorithm
	int flag = 1;
	if (setsockopt(socket_fd,IPPROTO_TCP,TCP_NODELAY,(char *) &flag, sizeof(int)) < 0){
		std::cout<<"It has been an error disabling Nagle's algorithm\n";
		close(socket_fd);
		return;
	}
	//habilitate keepalives
    int optval = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
        std::cout << "ERROR configuring KEEPALIVES\n";
        close(socket_fd);
        return;
    }
	// Configurar TCP_KEEPIDLE it sets what time to wait to start sending keepalives 
    float tcp_keepidle_time = static_cast<float>(keepalive_config.inactivity_time_until_keepalive_ms)/1000.0;
    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPIDLE, &tcp_keepidle_time, sizeof(tcp_keepidle_time)) < 0) {
        std::cout << "Error configuring TCP_KEEPIDLE\n";
		close(socket_fd);
        return;
    }
	  //interval between keepalives
    float keep_interval_time = static_cast<float>(keepalive_config.space_between_tries_ms)/1000.0;
    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keep_interval_time, sizeof(keep_interval_time)) < 0) {
        std::cout << "Error configuring TCP_KEEPINTVL\n";
        close(socket_fd);
        return;
    }
	 // Configure TCP_KEEPCNT (number keepalives are send before considering the connection down)
	 float keep_cnt = static_cast<float>(keepalive_config.tries_until_disconnection)/1000.0;
    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &keep_cnt, sizeof(keep_cnt)) < 0) {
        std::cout << "Error to configure TCP_KEEPCNT\n";
        close(socket_fd);
        return ;
    }
	connecting_sockets[remote_node] = this;

	//insert the remote address and port and connect
	struct sockaddr_in remote_addr;
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = inet_addr(remote_ip); // IP remota en formato adecuado
	remote_addr.sin_port = htons(remote_port);
	
	//CONNECT WILL BE NOT BLOCKING AS IT IS IN A MICROCONTROLLER
	
	connect(socket_fd, (struct sockaddr*)&remote_addr, sizeof(remote_addr));
	
	//SELECT WILL WAIT TIMEOUT AND IF THE CONNECT HAS BE DONE WE WILL CALL A CALL_BACK_FUNCTION

	fd_set write_fds;
	FD_ZERO(&write_fds);
	FD_SET(socket_fd, &write_fds);
	//timeout
	struct timeval timeout;
	timeout.tv_sec = 5; 
	timeout.tv_usec = 0;
	//use select to check if fthe socket has established connection
	int result = select(socket_fd + 1, NULL, &write_fds, NULL, &timeout);
	if (result > 0 && FD_ISSET(socket_fd, &write_fds)) {
		//If it's connected we do as connected_callback
		if(connecting_sockets.contains(remote_node)){
			connecting_sockets.erase(remote_node);
			state = CONNECTED;
		}
	}
}
Socket::Socket(IPV4 local_ip, uint32_t local_port, IPV4 remote_ip, uint32_t remote_port, uint32_t inactivity_time_until_keepalive_ms, uint32_t space_between_tries_ms, uint32_t tries_until_disconnection): Socket(local_ip, local_port, remote_ip, remote_port){
	keepalive_config.inactivity_time_until_keepalive_ms = inactivity_time_until_keepalive_ms;
	keepalive_config.space_between_tries_ms = space_between_tries_ms;
	keepalive_config.tries_until_disconnection = tries_until_disconnection;
}

Socket::Socket(EthernetNode local_node, EthernetNode remote_node):Socket(local_node.ip, local_node.port, remote_node.ip, remote_node.port){}

void Socket::close(){
	while(!tx_packet_buffer.empty()){
		tx_packet_buffer.pop();
	}
	while(!rx_packet_buffer.empty()){
		rx_packet_buffer.pop();
	}
	close(socket_fd);
    state = INACTIVE;
}

void Socket::reconnect(){
	EthernetNode remote_node(remote_ip, remote_port);
	if(!connecting_sockets.contains(remote_node)){
		connecting_sockets[remote_node] = this;
	}
	connect(socket_fd, (struct sockaddr*)&remote_addr, sizeof(remote_addr));
	fd_set write_fds;
	FD_ZERO(&write_fds);
	FD_SET(socket_fd, &write_fds);
	//timeout
	struct timeval timeout;
	timeout.tv_sec = 5; 
	timeout.tv_usec = 0;
	//use select to check if fthe socket has established connection
	int result = select(socket_fd + 1, NULL, &write_fds, NULL, &timeout);
	if (result > 0 && FD_ISSET(socket_fd, &write_fds)) {
		//If it's connected we do as connected_callback
		if(connecting_sockets.contains(remote_node)){
			connecting_sockets.erase(remote_node);
			state = CONNECTED;
		}
	}
}

void Socket::reset(){
	EthernetNode remote_node(remote_ip, remote_port);
	if(!connecting_sockets.contains(remote_node)){
		connecting_sockets[remote_node] = this;
	}
	state = INACTIVE;
	close();
	tcp_connection_sim();
	

}


void Socket::send(){
	
}

void Socket::process_data(){
	while(!rx_packet_buffer.empty()){
		struct pbuf* packet = rx_packet_buffer.front();
		rx_packet_buffer.pop();
		uint8_t* new_data = (uint8_t*)(packet->payload);
		tcp_recved(socket_control_block, packet->tot_len);
		Order::process_data(this, new_data);
		pbuf_free(packet);
	}
}

bool Socket::add_order_to_queue(Order& order){
	if(state == Socket::SocketState::CONNECTED){
		return false;
	}
	struct memp* next_memory_pointer_in_packet_buffer_pool = (*(memp_pools[PBUF_POOL_MEMORY_DESC_POSITION]->tab))->next;
	if(next_memory_pointer_in_packet_buffer_pool == nullptr){
		memp_free_pool(memp_pools[PBUF_POOL_MEMORY_DESC_POSITION], next_memory_pointer_in_packet_buffer_pool);
		return false;
	}

	uint8_t* order_buffer = order.build();

	struct pbuf* packet = pbuf_alloc(PBUF_TRANSPORT, order.get_size(), PBUF_POOL);
	pbuf_take(packet, order_buffer, order.get_size());
	Socket::tx_packet_buffer.push(packet);
	return true;
}

bool Socket::is_connected(){
	return state == Socket::SocketState::CONNECTED;
}

err_t Socket::connect_callback(void* arg, struct tcp_pcb* client_control_block, err_t error){}

err_t Socket::receive_callback(void* arg, struct tcp_pcb* client_control_block, struct pbuf* packet_buffer, err_t error){
	Socket* socket = (Socket*)arg;
	socket->socket_control_block = client_control_block;
	if(packet_buffer == nullptr){ 		//FIN is received
		socket->state = CLOSING;
		return ERR_OK;
	}
	if(error != ERR_OK){
		if(packet_buffer != nullptr){
			pbuf_free(packet_buffer);
		}
		return error;
	}else if(socket->state == CONNECTED){
		socket->rx_packet_buffer.push(packet_buffer);
		tcp_recved(client_control_block, packet_buffer->tot_len);
		socket->process_data();
		pbuf_free(packet_buffer);
		return ERR_OK;
	}else{
		tcp_recved(client_control_block, packet_buffer->tot_len);
		socket->tx_packet_buffer = {};
		pbuf_free(packet_buffer);
		return ERR_OK;
	}
}

err_t Socket::poll_callback(void* arg, struct tcp_pcb* client_control_block){
	Socket* socket = (Socket*)arg;
	socket->socket_control_block = client_control_block;
	if(socket != nullptr){
		while(not socket->tx_packet_buffer.empty()){
			socket->send();
		}
		if(socket->state == CLOSING){
			socket->close();
		}else if(socket->state == INACTIVE){
			tcp_connect(socket->connection_control_block, &socket->remote_ip.address , socket->remote_port, connect_callback);
		}
		return ERR_OK;
	}else{
		tcp_abort(client_control_block);
		return ERR_ABRT;
	}
}

err_t Socket::send_callback(void* arg, struct tcp_pcb* client_control_block, uint16_t length){
	Socket* socket = (Socket*)arg;
	socket->socket_control_block = client_control_block;
	if(not socket->tx_packet_buffer.empty()){
		socket->send();
	}else if(socket->state == CLOSING){
		socket->close();
	}
	return ERR_OK;
}

void Socket::error_callback(void *arg, err_t error){
	Socket* socket = (Socket*) arg;
		socket->close();
		ErrorHandler("Client socket error: %d. Socket closed, remote ip: %s",error,socket->remote_ip.string_address);
}

void Socket::connection_error_callback(void *arg, err_t error){
	if(error == ERR_RST){
		Socket* socket = (Socket*) arg;

		socket->pending_connection_reset = true;
		return;
	}else if(error == ERR_ABRT){
		return;
	}
	ErrorHandler("Connection socket error: %d. Couldn t start client socket ", error);
}

err_t Socket::connection_poll_callback(void *arg, struct tcp_pcb* connection_control_block){
	Socket* socket = (Socket*) arg;
	if(socket->pending_connection_reset){
		socket->reset();
		socket->pending_connection_reset = false;
		return ERR_ABRT;
	}
	else if(socket->connection_control_block->state == SYN_SENT){
		socket->pending_connection_reset = true;
	}
	return ERR_OK;
}

void Socket::config_keepalive(tcp_pcb* control_block, Socket* socket){
	control_block->so_options |= SOF_KEEPALIVE;
	control_block->keep_idle = socket->keepalive_config.inactivity_time_until_keepalive_ms;
	control_block->keep_intvl = socket->keepalive_config.space_between_tries_ms;
	control_block->keep_cnt = socket->keepalive_config.tries_until_disconnection;
}

#endif

