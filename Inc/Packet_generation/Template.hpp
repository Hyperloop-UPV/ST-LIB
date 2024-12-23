#include "ST-LIB.hpp"

class DataPacket {

    private:
        constexpr static size_t size =%size%;
        uint32_t id{0};
    public:
        StackPacket* packet;
        std::array<StackPacket*,size> packets;
    
    DataPacket(%cosas por referencia%){
        StackPacket* packet = new StackPacket(%cosas por referencia%);
        packets[id] = packet;
        id++;
    }




};