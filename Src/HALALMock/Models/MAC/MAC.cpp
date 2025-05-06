#include "HALALMock/Models/MAC/MAC.hpp"

MAC::MAC(string address) : string_address(address) {
    stringstream sstream(address);
    for (uint8_t& byte : this->address) {
        string temp;
        getline(sstream, temp, ':');
        byte = stoi(temp, nullptr, 16);
    }
}

MAC::MAC(uint8_t addr[6])
    : address{addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]},
      string_address([&]() {
          stringstream sstream;
          for (int i = 0; i < 6; ++i) {
              if (i > 0) sstream << ":";
              sstream << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(addr[i]);
          }
          return sstream.str();
      }()) {}

MAC::MAC() : MAC({0, 0, 0, 0, 0, 0}) {}
