#include "HALAL/Models/Packets/Packet.hpp"
#include "HALAL/Models/Packets/Order.hpp"

map<uint16_t,Order*> Order::orders = {};
map<uint16_t,Packet*> Packet::packets = {};
