

#include "HALALMock/Services/Communication/Ethernet/TCP/ServerSocket.hpp"

#define MAX_SIZE_BUFFER 1024

uint8_t ServerSocket::priority = 1;
unordered_map<uint32_t,ServerSocket*> ServerSocket::listening_sockets = {};
ServerSocket::ServerSocket() = default;

ServerSocket::ServerSocket(IPV4 local_ip, uint32_t local_port) : local_ip(local_ip),local_port(local_port){
	if(not Ethernet::is_running) {
		std::cout<<"ServerSocket: Cannot declare ServerSocket before Ethernet::start()\n";
		return;
	}
	tx_packet_buffer = {};
	state = INACTIVE;
	
	create_server_socket();//create _server_socket
	//configure server socket
	if(!configure_server_socket(this->server_socket_fd)){
		std::cout<<"ServerSocket: Error configuring ServerSocket\n";
		close();
		return;
	}
	//create listening thread
	listening_thread = std::jthread(&ServerSocket::listen_for_connection,this);
}	

ServerSocket::ServerSocket(IPV4 local_ip, uint32_t local_port, uint32_t inactivity_time_until_keepalive, uint32_t space_between_tries, uint32_t tries_until_disconnection): ServerSocket(local_ip, local_port){
	keepalive_config.inactivity_time_until_keepalive = inactivity_time_until_keepalive;
	keepalive_config.space_between_tries = space_between_tries;
	keepalive_config.tries_until_disconnection = tries_until_disconnection;
}

//I don't recommend this constructor
ServerSocket::ServerSocket(ServerSocket&& other) : 
tx_packet_buffer(std::move(other.tx_packet_buffer)),
listening_thread(std::move(other.listening_thread)),
receive_thread(std::move(other.receive_thread)),
server_socket_fd(other.server_socket_fd),
client_fd(other.client_fd),
local_ip(move(other.local_ip)), 
local_port(move(other.local_port)),
state(other.state)
{
	other.client_fd = -1;
	other.server_socket_fd = -1;
	listening_sockets[local_port] = this;
	tx_packet_buffer = {};
}
//not recommended in simulator
void ServerSocket::operator=(ServerSocket&& other){
	local_ip = move(other.local_ip);
	local_port = move(other.local_port);
	state = other.state;
	listening_sockets[local_port] = this;
	tx_packet_buffer = {};
	if(not (std::find(OrderProtocol::sockets.begin(), OrderProtocol::sockets.end(), this) != OrderProtocol::sockets.end()))
		OrderProtocol::sockets.push_back(this);
}

ServerSocket::~ServerSocket(){
	auto it = std::find(OrderProtocol::sockets.begin(), OrderProtocol::sockets.end(), this);
	if(it == OrderProtocol::sockets.end()) return;
	else OrderProtocol::sockets.erase(it);
	close();
}

ServerSocket::ServerSocket(EthernetNode local_node) : ServerSocket(local_node.ip,local_node.port){};

	
//The ServerSocket will only accept one connection
void ServerSocket::listen_for_connection(){
	if (listen(server_socket_fd, SOMAXCONN) < 0) {
        std::cout<<"ServerSocket: Error listening\n";
        ::close(server_socket_fd);
		state = CLOSED;
        return;
    }
	state = LISTENING;
	listening_sockets[local_port] = this;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	client_fd = accept(server_socket_fd,(struct sockaddr*)&client_addr, &client_len);
	if(client_fd < 0){
		std::cout<< "ServerSocket: Error accepting\n";
		close_inside_thread();
	}
	if(accept_callback(client_fd,client_addr) == true){
		OrderProtocol::sockets.push_back(this);
	}else{
		std::cout<<"ServerSocket: Something went wrong in accept_callback\n";
	}			
}


void ServerSocket::close(){
	// Clean all descriptors
	if (client_fd != -1) {
        ::close(client_fd);
        client_fd = -1;
    }
    if (server_socket_fd != -1) {
        ::close(server_socket_fd);
        server_socket_fd = -1;
    }
	//clean the transmision buffer
	while (!tx_packet_buffer.empty()) {
        tx_packet_buffer.pop();
    }
	//eliminate the threads
	 if (state == LISTENING && listening_thread.joinable()) {
            listening_thread.join();
        } else if (state == ACCEPTED && receive_thread.joinable()) {
            receive_thread.join();
        }
	listening_sockets[local_port] = this;
	state = CLOSED;
}


bool ServerSocket::add_order_to_queue(Order& order){
	if(state == ACCEPTED){
		return false; 
	}
    if (order.get_size() == 0) {
        std::cout << "ServerSocket: order is empty\n";
        return false; 
    }
    {
        std::lock_guard<std::mutex> lock(mutex); 
        tx_packet_buffer.push(&order); 
    }
	return true;
}

void ServerSocket::send(){
	
	while (!tx_packet_buffer.empty()) {
		size_t packet_size;
		uint8_t *packet_data;
		{
			std::lock_guard<std::mutex> lock(mutex);
			Packet *packet = tx_packet_buffer.front();
		    packet_size = packet->get_size();
			packet_data = packet->build();
		}
		size_t total_sent = 0;
		while(total_sent < packet_size){
			ssize_t sent_bytes = ::send(client_fd, packet_data, packet_size, 0);
			if (sent_bytes < 0) {
				std::cerr << "Error sending the order\n";
				return;
			}
			total_sent += sent_bytes;
		}
		tx_packet_buffer.pop();
    }
}
	
bool ServerSocket::is_connected(){
	return state == ServerSocket::ServerState::ACCEPTED;
}
void ServerSocket::create_server_socket(){
	server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket_fd == -1){
		std::cout<<"ServerSocket: Socket creation failure\n";
		return;
	}
	//inset the local address and port
	struct sockaddr_in server_socket_Address;
	server_socket_Address.sin_family = AF_INET;
	server_socket_Address.sin_addr.s_addr = local_ip.address;
	server_socket_Address.sin_port = htons(local_port);
	if(bind(server_socket_fd, (struct sockaddr*)&server_socket_Address, sizeof(server_socket_Address)) < 0){
		std::cout<<"ServerSocket: Bind error\n";
		close();
		return;
	}
}
bool ServerSocket::configure_server_socket(int& socket_fd){
	//to reuse local address:
	int opt = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "ServerSocket: Error setting SO_REUSEADDR\n";
   		close();
    	return false;
	}
	//disable naggle algorithm
	int flag = 1;
	if (setsockopt(socket_fd,IPPROTO_TCP,TCP_NODELAY,(char *) &flag, sizeof(int)) < 0){
		std::cout<<"ServerSocket: It has been an error disabling Nagle's algorithm\n";
		return false;
	}
	//habilitate keepalives
    int optval = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
        std::cout << "ServerSocket: ERROR configuring KEEPALIVES\n";
        return false;
    }
	// Configure TCP_KEEPIDLE it sets what time to wait to start sending keepalives 
    //different from lwip to linux
	uint32_t tcp_keepidle_time = keepalive_config.inactivity_time_until_keepalive;
    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPIDLE, &tcp_keepidle_time, sizeof(tcp_keepidle_time)) < 0) {
        std::cout << "ServerSocket: Error configuring TCP_KEEPIDLE\n";
        return false;
    }
	  //interval between keepalives
    uint32_t keep_interval_time = keepalive_config.space_between_tries;
    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keep_interval_time, sizeof(keep_interval_time)) < 0) {
        std::cout << "ServerSocket: Error configuring TCP_KEEPINTVL\n";
        return false;
    }
	 // Configure TCP_KEEPCNT (number keepalives are send before considering the connection down)
	uint32_t  keep_cnt = keepalive_config.tries_until_disconnection;
    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &keep_cnt, sizeof(keep_cnt)) < 0) {
        std::cout << "ServerSocket: Error to configure TCP_KEEPCNT\n";
        return false;
    }
	return true;
}

bool ServerSocket::accept_callback(int& client_fd, sockaddr_in& client_address){
	if(listening_sockets.contains(local_port) && state == LISTENING){
		state = ACCEPTED;
		remote_ip = IPV4(client_address.sin_addr.s_addr);
		this->client_fd = client_fd;
		//configure_server_socket
		configure_server_socket(client_fd);
		//create the receive thread
		receive_thread = std::jthread(&ServerSocket::receive,this);
		return true;
	}
	return false;	
}
void ServerSocket::receive(){
	while (state == ACCEPTED) {
        uint8_t buffer[MAX_SIZE_BUFFER]; // Buffer for the data
        ssize_t received_bytes = ::recv(client_fd, buffer, sizeof(buffer), 0);
        if(received_bytes > 0) {
			uint8_t* received_data = new uint8_t[received_bytes];
			std::memcpy(received_data,buffer,received_bytes);
			Order::process_data(this, received_data);
			delete[] received_data;
		}else{
			std::cout << "ServerSocket: Error receiving the data or Client Disconnected, Closing... \n";
			state = CLOSING;
			close_inside_thread();
			return;
		} 
    }
}
void ServerSocket::close_inside_thread(){
	//close descriptors
	if(server_socket_fd >= 0){
		::close(server_socket_fd);
	}
	if(client_fd >= 0){
		::close(client_fd);
	}
	//clean the transmissions buffers
	{
		std::lock_guard<std::mutex> lock(mutex);
		while (!tx_packet_buffer.empty()) {
        tx_packet_buffer.pop();
    	}
	}
	listening_sockets[local_port] = this;
	state = CLOSED;
}

