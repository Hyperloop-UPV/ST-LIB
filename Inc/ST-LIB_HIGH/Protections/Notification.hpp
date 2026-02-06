#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Models/Packets/Order.hpp"
#include "Protection.hpp"

class Notification : public Order{
private:
	typedef uint16_t message_size_t;
	uint16_t id;
    void(*callback)() = nullptr;
    string tx_message;
    message_size_t tx_message_size;
    string rx_message;
    uint8_t* buffer = nullptr;
    OrderProtocol* received_socket = nullptr;
public:
    Notification(uint16_t packet_id,void(*callback)() ,string message) : id(packet_id), callback(callback),tx_message(message), tx_message_size(message.size()){
    	Order::orders[id] = this;
    	Packet::packets[id] = this;
    }

    Notification(uint16_t packet_id,void(*callback)()) : id(packet_id), callback(callback){
    	Order::orders[id] = this;
    	Packet::packets[id] = this;
    }

    void set_callback(void(*callback)()) override {
    	this->callback = callback;
    }

    void process() override{
    	if(callback != nullptr) callback();
    	string aux = tx_message;
    	tx_message = rx_message;
    	for(OrderProtocol* socket : OrderProtocol::sockets){
    		if(socket == received_socket) continue;
    		socket->send_order(*this);
    	}
    	tx_message = aux;
    }

    uint8_t* build() {
    	if(buffer != nullptr) free(buffer);
    	buffer = (uint8_t*)malloc(get_size());

    	memcpy(buffer,&id,sizeof(id));
    	memcpy(buffer + sizeof(id), &tx_message_size, sizeof(tx_message_size));
     	memcpy(buffer+sizeof(id)+sizeof(tx_message_size),tx_message.c_str(), tx_message.size());
    	return buffer;
    }

    void notify(string message){
    	tx_message = message;
    	tx_message_size = message.size();
    	notify();
    }

    void notify(){
    	if(tx_message.empty()){
    		ErrorHandler("Cannot notify empty notification");
    		return;
    	}
    	for(OrderProtocol* socket : OrderProtocol::sockets){
    		socket->send_order(*this);
    	}
    }

    void parse(OrderProtocol* socket, uint8_t* data) {
    	received_socket = socket;
    	char* temp = (char*)malloc(get_string_size(data));
    	memcpy(temp, data+sizeof(id)+sizeof(message_size_t), get_string_size(data));
    	rx_message = string(temp);
    	free(temp);
    }

    size_t get_size() {
    	size = sizeof(id) +  sizeof(tx_message_size) +  tx_message_size;
    	return size;
    }

    uint16_t get_id() {
    	return id;
    }

    void set_pointer(size_t index, void* pointer) override{
    	ErrorHandler("Notification does not suport this method!");
    }

    ~Notification(){
    	if(buffer != nullptr) free(buffer);
    }


private:

    uint16_t get_string_size(uint8_t* buffer){
    	return *(uint16_t*)(buffer + sizeof(id));
    }
};
