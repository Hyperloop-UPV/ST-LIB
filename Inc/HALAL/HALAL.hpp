#pragma once

#include "HALAL/Models/HALconfig/HALconfig.hpp"
#include "HALAL/Models/DMA/DMA.hpp"
#include "HALAL/Services/Flash/Flash.hpp"


// #include "HALAL/Services/EXTI/EXTI.hpp"
#include "HALAL/Services/Communication/UART/UART.hpp"     

#include "HALAL/Services/Communication/Ethernet/UDP/DatagramSocket.hpp"
#include "HALAL/Services/Communication/Ethernet/Ethernet.hpp"
// #include "HALAL/Models/TimerPeripheral/TimerPeripheral.hpp"



#include "HALAL/Models/MPUManager/MPUManager.hpp"        
// #include "HALAL/Services/Watchdog/Watchdog.hpp"           

namespace HALAL {

#ifdef STLIB_ETH
    void start(MAC mac,
               IPV4 ip,
               IPV4 subnet_mask,
               IPV4 gateway
                );
#else
    void start();
#endif

} // namespace HALAL