#include "ST-LIB_HIGH/Transport/PacketTransport.hpp"
#include "lwip/tcp.h"

namespace ST_LIB {

struct tcp_pcb* PacketTransport::tpcb = nullptr;
bool PacketTransport::is_connected = false;

bool PacketTransport::send(const void* data, size_t size) {
    if (tpcb == nullptr || !is_connected) {
        return false;
    }
    err_t err = tcp_write(tpcb, data, size, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        return false;
    }
    return tcp_output(tpcb) == ERR_OK;
}

} // namespace ST_LIB
