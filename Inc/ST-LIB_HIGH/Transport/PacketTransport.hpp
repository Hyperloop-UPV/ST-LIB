#pragma once

#include <cstddef>
#include <cstdint>

struct tcp_pcb;

namespace ST_LIB {

struct PacketTransport {
    static struct tcp_pcb* tpcb;
    static bool is_connected;

    static bool send(const void* data, size_t size);
};

} // namespace ST_LIB
