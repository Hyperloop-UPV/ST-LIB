#include "HALAL/Models/Packets/Packet.hpp"

void Packet::parse_data(uint8_t* raw_data) {
    auto packet = Packet::get_packet(Packet::get_id(raw_data));
    if (packet != nullptr) {
        packet->parse(raw_data);
    }
}
