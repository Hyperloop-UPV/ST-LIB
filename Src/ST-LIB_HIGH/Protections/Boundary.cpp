#include "ST-LIB_HIGH/Protections/Boundary.hpp"

const map<type_id_t, uint8_t> BoundaryInterface::format_look_up{
    {type_id<int>, 0},
    {type_id<float>, 1},
    {type_id<double>, 2},
    {type_id<char>, 3},
    {type_id<bool>, 4},
    {type_id<short>, 5},
    {type_id<long>, 6},
    {type_id<uint8_t>, 7},
    {type_id<uint16_t>, 8},
    {type_id<uint32_t>, 9},
    {type_id<uint64_t>, 10},
    {type_id<int8_t>, 11},
};
