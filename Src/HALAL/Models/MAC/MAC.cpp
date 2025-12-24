#include "HALAL/Models/MAC/MAC.hpp"

#ifdef HAL_ETH_MODULE_ENABLED

// MAC::MAC(string address) : string_address(address) {
//     stringstream sstream(address);
//     for (u8_t& byte : this->address) {
//         string temp;
//         getline(sstream, temp, ':');
//         byte = stoi(temp, nullptr, 16);
//     }
// }

MAC::MAC(u8_t addr[6])
    : address{addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]}
      /*, string_address([&]() {
          stringstream sstream;
          for (int i = 0; i < 6; ++i) {
              if (i > 0) sstream << ":";
              sstream << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(addr[i]);
          }
          return sstream.str();
      }())*/ {}

MAC::MAC() : MAC((uint8_t*)"\0\0\0\0\0\0") {}

MAC MAC::parse_string(const char* address) {
    uint8_t addr[6];
    unsigned int i_addr[6];
    sscanf(address, "%x:%x:%x:%x:%x:%x", &i_addr[0], &i_addr[1], &i_addr[2], &i_addr[3], &i_addr[4], &i_addr[5]);
    for(int i=0; i<6; i++) addr[i] = (uint8_t)i_addr[i];
    return MAC(addr);
}

#endif
