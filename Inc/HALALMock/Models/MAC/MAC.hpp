#pragma once

#include <iomanip>

#include "C++Utilities/CppUtils.hpp"

using std::getline;
using std::stringstream;

class MAC {
   public:
    uint8_t address[6];
    string string_address;

    MAC();
    MAC(string address);
    MAC(uint8_t address[6]);
};
