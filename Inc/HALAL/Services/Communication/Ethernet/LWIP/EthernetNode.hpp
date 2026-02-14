#pragma once

#include "HALAL/Models/IPV4/IPV4.hpp"

#ifdef HAL_ETH_MODULE_ENABLED

struct EthernetNode {
    IPV4 ip;
    uint32_t port;

    EthernetNode(IPV4 ip, uint32_t port);

    bool operator==(const EthernetNode& other) const;
};

namespace std {

template <> struct hash<EthernetNode> {
    std::size_t operator()(const EthernetNode& k) const;
};

} // namespace std

#endif
