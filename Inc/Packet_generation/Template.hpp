#include "ST-LIB.hpp"
class DataPacket{
    
%enums%

    private:
        constexpr static size_t size =%size%;
        uint32_t id{0};
    public:
        std::array<StackPacket*,size> packets; 
%packetnames%
        
    DataPacket(%data%)
{

%packets%

}
};