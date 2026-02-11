#pragma once

#include "HALAL/Models/Packets/PacketValue.hpp"
#include "HALAL/Models/DataStructures/StackTuple.hpp"

class Packet{
public:
    size_t size;
    virtual uint8_t* build() = 0;
    virtual void parse(uint8_t* data) = 0;
    virtual size_t get_size() = 0;
    virtual uint16_t get_id() = 0; 
    virtual void set_pointer(size_t index, void* pointer) = 0;
    static uint16_t get_id(uint8_t* data) {
        return *((uint16_t*)data);
    }
    static void parse_data(uint8_t* data){
        uint16_t id = get_id(data);
        if(packets.contains(id)) packets[id]->parse(data);
    }
protected:
    static map<uint16_t, Packet*> packets;
};

template<size_t BufferLength, class... Types> 
class StackPacket : public Packet{
public:
    uint16_t id;
    PacketValue<>* values[sizeof...(Types)];
    uint8_t buffer[BufferLength + sizeof(id)];
    size_t& size = Packet::size;
    stack_tuple<PacketValue<Types>...> packetvalue_warehouse;
    StackPacket(uint16_t id, Types*... values): id(id), packetvalue_warehouse{PacketValue<Types>(values)...} {
        int i = 0;
        packetvalue_warehouse.for_each([this, &i](auto& value) {
            this->values[i++] = &value;
        });
        packets[id] = this;
        size = BufferLength + sizeof(id);
    }
    void parse(uint8_t* data) override {
        data+=sizeof(id);
        for (PacketValue<>* value : values) {
            value->parse(data);
            data += value->get_size();
        }
    }
    size_t get_size() override {
        return size;
    }
    uint8_t* build() override {
        uint8_t* data = buffer;
        memcpy(data, &id, sizeof(id));
        data += sizeof(id);
        for (PacketValue<>* value : values) {
            value->copy_to(data);
            data += value->get_size();
        }
        return buffer;
    }

    uint16_t get_id() override {
        return id;
    }

    void set_pointer(size_t index, void* pointer) override{
    	values[index]->set_pointer(pointer);
    }
};

template<class... Types>
class StackPacket<0, Types...> : public Packet{
public:
    uint16_t id;
    PacketValue<>* values[sizeof...(Types)];
    stack_tuple<PacketValue<Types>...> packetvalue_warehouse;
    size_t buffer_size = 0;
    size_t& data_size = Packet::size;
    uint8_t* buffer = nullptr;

    StackPacket(uint16_t id, Types*... values): id(id), packetvalue_warehouse{PacketValue<Types>(values)...} {
        int i = 0;
        packetvalue_warehouse.for_each([this, &i](auto& value) {
            this->values[i++] = &value;
        });
        packets[id] = this;
    }

    void parse(uint8_t* data) override {
        data+=sizeof(id);
        for (PacketValue<>* value : values) {
            value->parse(data);
            data += value->get_size();
        }
    }

    size_t get_size()override {
        size_t new_size = 0;
        for (PacketValue<>* value : values) {
            new_size += value->get_size();
        }
        return new_size + sizeof(id);
    }

    uint8_t* build() override {
        data_size = get_size();
        if (buffer != nullptr && (data_size > buffer_size || data_size < buffer_size/2)){
            delete[] buffer;
            buffer = new uint8_t[data_size]; 
            buffer_size = data_size;
        }
        else if (buffer == nullptr) {
            buffer = new uint8_t[data_size];
            buffer_size = data_size;
        }

        uint8_t* data = buffer;
        memcpy(data, &id, sizeof(id));
        data += sizeof(id);
        for (PacketValue<>* value : values) {
            value->copy_to(data);
            data += value->get_size();
        }
        return buffer;
    }

    uint16_t get_id() override {
        return id;
    }

    void set_pointer(size_t index, void* pointer) override{
		values[index]->set_pointer(pointer);
	}

    ~StackPacket() {
        if (buffer != nullptr) delete[] buffer;
    }
};

template<>
class StackPacket<0> : public Packet{
public:
    uint16_t id;
    const size_t size = sizeof(id);
    StackPacket() = default;
    StackPacket(uint16_t id) : id(id){packets[id] = this;}
    void parse(uint8_t* data) override {}
    uint8_t* build() override {
        return (uint8_t*)&id;
    }
    uint16_t get_id() override {
        return id;
    }
    size_t get_size() override {
        return sizeof(id); 
    }

    void set_pointer(size_t index, void* pointer) override{};
};


#if __cpp_deduction_guides >= 201606
template<class... Types>
StackPacket(uint16_t, Types*... values)->StackPacket<(!has_container<Types...>::value)*total_sizeof<Types...>::value, Types...>;

StackPacket()->StackPacket<0>;
#endif

class HeapPacket : public Packet{
public:
    uint16_t id;
    vector<unique_ptr<PacketValue<>>> values;
    uint8_t* buffer = nullptr;
    size_t& data_size = Packet::size;
    size_t buffer_size = 0;
    HeapPacket() = default;

    template<class... Types>
    HeapPacket(uint16_t id, Types*... values): id(id) {
    	(this->values.emplace_back(unique_ptr<PacketValue<>>(new PacketValue(values))) , ...);
    	packets[id] = this;
    }

    void parse(uint8_t* data) override {
        data += sizeof(id);
        for (unique_ptr<PacketValue<>>& value : values) {
            value.get()->parse(data);
            data += value.get()->get_size();
        }
    }

    size_t get_size() override {
        size_t new_size = 0;
        for (unique_ptr<PacketValue<>>& value : values) {
            new_size += value.get()->get_size();
        }
        return new_size + sizeof(id);
    }

    uint8_t* build() override {
        data_size = get_size();
        if (buffer != nullptr && (data_size > buffer_size || data_size < buffer_size/2)){
            delete[] buffer;
            buffer = new uint8_t[data_size]; 
            buffer_size = data_size;
        }
        else if (buffer == nullptr) {
            buffer = new uint8_t[data_size];
            buffer_size = data_size;
        }
        uint8_t* data = buffer;
        memcpy(data, &id, sizeof(id));
        data += sizeof(id);
        for (unique_ptr<PacketValue<>>& value : values) {
            value->copy_to(data);
            data += value->get_size();
        }
        return buffer;
    }
    uint16_t get_id() override {
        return id;
    }

    void set_pointer(size_t index, void* pointer) override{
		values[index]->set_pointer(pointer);
	}

    ~HeapPacket() {
        if(buffer != nullptr) delete[] buffer;
    }
};
