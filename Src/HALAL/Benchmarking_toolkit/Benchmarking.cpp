#include "HALAL/Benchmarking_toolkit/Benchmarking.hpp"

Frequency_Packet freq_packet;
void increment_overflow(){
    static uint32_t high_part = 0;
    high_part++;
    freq_packet.control_field &= 0x0000FFFF;
    freq_packet.control_field |= high_part << 16;
}