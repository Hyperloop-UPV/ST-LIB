#include "HALAL/Benchmarking_toolkit/Benchmarking.hpp"

Frequency_Packet freq_packet;
void increment_overflow(){
    uint16_t high_part = freq_packet.control_field >> 16 & 0xFFFF;
    high_part++;
    freq_packet.control_field |= high_part << 16;
}