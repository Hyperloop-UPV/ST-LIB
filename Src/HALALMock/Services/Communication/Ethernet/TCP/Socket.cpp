//#ifdef STLIB_ETH

#include "HALALMock/Services/Communication/Ethernet/TCP/Socket.hpp"
#define MAX_SIZE_BUFFER 1024
std::unordered_map<EthernetNode,Socket*> Socket::connecting_sockets = {};


Socket::Socket() = default;

Socket::Socket(Socket&& other):remote_port(move(remote_port)),state(other.state){
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
	close();
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
	if(!create_socket()){
		return;
	}
	EthernetNode remote_node(remote_ip, remote_port);
	if(!configure_socket()){
		std::cout<<"error configuring socket\n";
		return;
	}
	connecting_sockets[remote_node] = this;
	connect_attempt();
	OrderProtocol::sockets.push_back(this);
}

bool Socket::create_socket(){
	//create socket not blocking
	socket_fd = ::socket(AF_INET,SOCK_STREAM,0);
	//inset the local address and port
	struct sockaddr_in socket_Address;
	socket_Address.sin_family = AF_INET;
	socket_Address.sin_addr.s_addr = local_ip.address;
	socket_Address.sin_port = htons(local_port);
	if(bind(socket_fd, (struct sockaddr*)&socket_Address, sizeof(socket_Address)) < 0){
		std::cout<<"Bind error in TCP socket\n";
		::close(socket_fd);
		return false;
	}
	return true;
}
bool Socket::configure_socket(){
	//disable naggle algorithm
	int flag = 1;
	if (setsockopt(socket_fd,IPPROTO_TCP,TCP_NODELAY,(char *) &flag, sizeof(int)) < 0){
		std::cout<<"It has been an error disabling Nagle's algorithm\n";
		::close(socket_fd);
		return false;
	}
	//make the socket to be reuse
	int optval_reuse = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval_reuse, sizeof(optval_reuse)) < 0) {
        std::cerr << "Error setting SO_REUSEADDR\n";
        ::close(socket_fd);
        return false;
    }
	//habilitate keepalives
    int optval = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
        std::cout << "ERROR configuring KEEPALIVES\n";
        ::close(socket_fd);
        return false;
    }
	// Configure TCP_KEEPIDLE it sets what time to wait to start sending keepalives 
    // Using the minimum linux keepalives time 
	uint32_t tcp_keepidle_time = keepalive_config.inactivity_time_until_keepalive; 
    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPIDLE, &tcp_keepidle_time, sizeof(tcp_keepidle_time)) < 0) {
        std::cout << "Error configuring TCP_KEEPIDLE\n";
		::close(socket_fd);
        return false;
    }
	  //interval between keepalives
    uint32_t keep_interval_time = keepalive_config.space_between_tries;
    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keep_interval_time, sizeof(keep_interval_time)) < 0) {
        std::cout << "Error configuring TCP_KEEPINTVL\n";
        ::close(socket_fd);
        return false;
    }
	 // Configure TCP_KEEPCNT (number keepalives are send before considering the connection down)
	uint32_t keep_cnt = keepalive_config.tries_until_disconnection;
    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &keep_cnt, sizeof(keep_cnt)) < 0) {
        std::cout << "Error to configure TCP_KEEPCNT\n";
        ::close(socket_fd);
        return false;
    }
	return true;
}



void Socket::connect_attempt(){
	//insert the remote address and port and connect
	struct sockaddr_in remote_addr;
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = remote_ip.address; 
	remote_addr.sin_port = htons(remote_port);
	if(connect(socket_fd, (struct sockaddr*)&remote_addr, sizeof(remote_addr)) < 0){
		state = INACTIVE;
		return;
	}
	std::cout<<"Connection established with the remote socket\n";
	connection_callback();
}

void Socket::connection_callback(){
	EthernetNode remote_node(remote_ip, remote_port);
	if(connecting_sockets.contains(remote_node)){
		connecting_sockets.erase(remote_node);
		state = CONNECTED;
	}
	//start receiving
	is_receiving = true;
    receiving_thread = std::jthread(&Socket::receive, this); 
}

Socket::Socket(IPV4 local_ip, uint32_t local_port, IPV4 remote_ip, uint32_t remote_port, uint32_t inactivity_time_until_keepalive, uint32_t space_between_tries, uint32_t tries_until_disconnection): Socket(local_ip, local_port, remote_ip, remote_port){
	keepalive_config.inactivity_time_until_keepalive = inactivity_time_until_keepalive;
	keepalive_config.space_between_tries = space_between_tries;
	keepalive_config.tries_until_disconnection = tries_until_disconnection;
}

Socket::Socket(EthernetNode local_node, EthernetNode remote_node):Socket(local_node.ip, local_node.port, remote_node.ip, remote_node.port){}

void Socket::close(){
	::close(socket_fd);
    if (is_receiving) {
        is_receiving = false;
        if (receiving_thread.joinable()) {
            receiving_thread.join();  
        }
    }
	while(!tx_packet_buffer.empty()){
		tx_packet_buffer.pop();
	}
	state = CLOSING;
}

void Socket::reconnect(){ //I'm going to do in reconnect a total reset due to at the end in linux sockets you will have to close the socket and configure to reconnect
	connect_attempt();
}

void Socket::reset(){
	EthernetNode remote_node(remote_ip, remote_port);
	
	state = INACTIVE;
	close();
	if(!create_socket()){
		return;
	}
	if(!configure_socket()){
		std::cout<<"error configuring socket\n";
		return;
	}
	if(!connecting_sockets.contains(remote_node)){
		connecting_sockets[remote_node] = this;
	}
	connect_attempt();
}


void Socket::send(){
	std::lock_guard<std::mutex> lock(mutex);
	while (!tx_packet_buffer.empty()) {
        Packet *packet = tx_packet_buffer.front();
        size_t total_sent = 0;
		size_t packet_size = packet->get_size();
		uint8_t *packet_data = packet->build();
		while(total_sent < packet_size){
			ssize_t sent_bytes = ::send(socket_fd, packet_data, packet_size, 0);
			if (sent_bytes < 0) {
				std::cerr << "Error sending the order\n";
				return;
			}
			total_sent += sent_bytes;
		}
		tx_packet_buffer.pop();
    }
}
void Socket::receive(){
    while (is_receiving) {
        uint8_t buffer[MAX_SIZE_BUFFER]; // Buffer for the data
		
        ssize_t received_bytes = ::recv(socket_fd, buffer, sizeof(buffer), 0);
        if(received_bytes > 0) {
			uint8_t* received_data = new uint8_t[received_bytes];
			std::memcpy(received_data,buffer,received_bytes);
			Order::process_data(this, received_data);
			delete[] received_data;

        }else if (received_bytes < 0) {

            std::cout << "Error receiving data\n";
			state = CLOSING;
			::close(socket_fd);
			while(!tx_packet_buffer.empty()){
				tx_packet_buffer.pop();
			}
			return;
        }
    }
}

bool Socket::add_order_to_queue(Order& order){
	if(state == Socket::SocketState::CONNECTED){
		return false;
	}
    if (order.get_size() == 0) {
        std::cout << "Error: order empty\n";
        return false; 
    }
    {
        std::lock_guard<std::mutex> lock(mutex); 
        tx_packet_buffer.push(&order); 
    }
	return true;
}

bool Socket::is_connected(){
	return state == Socket::SocketState::CONNECTED;
}
//#endif //STLIB_ETH

