
#include "HALALMock/Services/Communication/Ethernet/TCP/Socket.hpp"
#ifdef HAL_ETH_MODULE_ENABLED

unordered_map<EthernetNode,Socket*> Socket::connecting_sockets = {};


Socket::Socket() = default;

Socket::Socket(Socket&& other):remote_port(move(remote_port)), connection_control_block(move(other.connection_control_block)),
	 state(other.state){
	EthernetNode remote_node(other.remote_ip, other.remote_port);
	connecting_sockets[remote_node] = this;
}

void Socket::operator=(Socket&& other){
	connection_control_block = move(other.connection_control_block);
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
		ErrorHandler("Cannot declare TCP socket before Ethernet::start()");
		return;
	}
	state = INACTIVE;
	tx_packet_buffer = {};
	rx_packet_buffer = {};
	EthernetNode remote_node(remote_ip, remote_port);

	connection_control_block = tcp_new();
	tcp_bind(connection_control_block, &local_ip.address, local_port);
	tcp_nagle_disable(connection_control_block);
	tcp_arg(connection_control_block, this);
	tcp_poll(connection_control_block,connection_poll_callback,1);
	tcp_err(connection_control_block, connection_error_callback);

	connecting_sockets[remote_node] = this;
	tcp_connect(connection_control_block, &remote_ip.address , remote_port, connect_callback);
	OrderProtocol::sockets.push_back(this);
}

Socket::Socket(IPV4 local_ip, uint32_t local_port, IPV4 remote_ip, uint32_t remote_port, uint32_t inactivity_time_until_keepalive_ms, uint32_t space_between_tries_ms, uint32_t tries_until_disconnection): Socket(local_ip, local_port, remote_ip, remote_port){
	keepalive_config.inactivity_time_until_keepalive_ms = inactivity_time_until_keepalive_ms;
	keepalive_config.space_between_tries_ms = space_between_tries_ms;
	keepalive_config.tries_until_disconnection = tries_until_disconnection;
}

Socket::Socket(EthernetNode local_node, EthernetNode remote_node):Socket(local_node.ip, local_node.port, remote_node.ip, remote_node.port){}

void Socket::close(){
  tcp_arg(socket_control_block, nullptr);
  tcp_sent(socket_control_block, nullptr);
  tcp_recv(socket_control_block, nullptr);
  tcp_err(socket_control_block, nullptr);
  tcp_poll(socket_control_block, nullptr, 0);

	while(!tx_packet_buffer.empty()){
		pbuf_free(tx_packet_buffer.front());
		tx_packet_buffer.pop();
	}
	while(!rx_packet_buffer.empty()){
		pbuf_free(rx_packet_buffer.front());
		rx_packet_buffer.pop();
	}

  tcp_close(socket_control_block);
  state = INACTIVE;
}

void Socket::reconnect(){
	EthernetNode remote_node(remote_ip, remote_port);
	if(!connecting_sockets.contains(remote_node)){
		connecting_sockets[remote_node] = this;
	}
	tcp_connect(connection_control_block, &remote_ip.address , remote_port, connect_callback);
}

void Socket::reset(){
	EthernetNode remote_node(remote_ip, remote_port);
	if(!connecting_sockets.contains(remote_node)){
		connecting_sockets[remote_node] = this;
	}
	state = INACTIVE;
	tcp_abort(connection_control_block);
	connection_control_block = tcp_new();

	tcp_bind(connection_control_block, &local_ip.address, local_port);
	tcp_nagle_disable(connection_control_block);
	tcp_arg(connection_control_block, this);
	tcp_poll(connection_control_block,connection_poll_callback,1);
	tcp_err(connection_control_block, connection_error_callback);

	tcp_connect(connection_control_block, &remote_ip.address , remote_port, connect_callback);

}


void Socket::send(){
	pbuf* temporal_packet_buffer;
	err_t error = ERR_OK;
	while(error == ERR_OK && !tx_packet_buffer.empty() && tx_packet_buffer.front()->len <= tcp_sndbuf(socket_control_block)){
		temporal_packet_buffer = tx_packet_buffer.front();
		error = tcp_write(socket_control_block, temporal_packet_buffer->payload, temporal_packet_buffer->len, TCP_WRITE_FLAG_COPY);
		if(error == ERR_OK){
			tx_packet_buffer.pop();
			tcp_output(socket_control_block);
			memp_free_pool(memp_pools[PBUF_POOL_MEMORY_DESC_POSITION],temporal_packet_buffer);
		}else{
			if(error == ERR_MEM){
				close();
				ErrorHandler("Too many unacked messages on client socket, disconnecting...");
			}else{
				ErrorHandler("Cannot write to client socket. Error code: %d",error);
			}
		}
	}
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

err_t Socket::connect_callback(void* arg, struct tcp_pcb* client_control_block, err_t error){
	IPV4 remote_ip;
	remote_ip.address = client_control_block->remote_ip;
	EthernetNode remote_node(remote_ip,client_control_block->remote_port);

	if(connecting_sockets.contains(remote_node)){
		Socket* socket = connecting_sockets[remote_node];
		connecting_sockets.erase(remote_node);

		socket->socket_control_block = client_control_block;
		socket->state = CONNECTED;
		
		tcp_nagle_disable(client_control_block);
		tcp_arg(client_control_block, socket);
		tcp_recv(client_control_block, receive_callback);
		tcp_poll(client_control_block, poll_callback,0);
		tcp_sent(client_control_block, send_callback);
		tcp_err(client_control_block, error_callback);
		if(socket->use_keep_alives)
			config_keepalive(client_control_block, socket);

		return ERR_OK;
	}else return ERROR;
}

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

