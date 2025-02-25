#include "HALALMock/Services/Communication/Ethernet/EthernetNode.hpp"

EthernetNode::EthernetNode(IPV4 ip, uint32_t port):ip(ip), port(port){}

bool EthernetNode::operator==(const EthernetNode& other) const{
	return ip.address == other.ip.address && port == other.port;
}

std::size_t hash<EthernetNode>::operator()(const EthernetNode& key) const
{
  using std::size_t;
  using std::hash;
  using std::string;

  return (hash<uint32_t>()(key.ip.address)) ^ (hash<uint32_t>()(key.port) << 1);
}

