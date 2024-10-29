/*
 * EthernetNode.cpp
 *
 *  Created on: Nov 23, 2022
 *      Author: stefa
 */
#include "HALAL/Services/Communication/Ethernet/EthernetNode.hpp"
#ifdef HAL_ETH_MODULE_ENABLED

EthernetNode::EthernetNode(IPV4 ip, uint32_t port):ip(ip), port(port){}

bool EthernetNode::operator==(const EthernetNode& other) const{
	return ip.address.addr == other.ip.address.addr && port == other.port;
}

std::size_t hash<EthernetNode>::operator()(const EthernetNode& key) const
{
  using std::size_t;
  using std::hash;
  using std::string;

  return (hash<uint32_t>()(key.ip.address.addr)) ^ (hash<uint32_t>()(key.port) << 1);
}
#endif
