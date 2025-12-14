#pragma once

#include "HALAL/Models/GPIO.hpp"
#include "HALAL/Models/Pin.hpp"

#include "HALAL/Models/HALconfig/HALconfig.hpp"
#include "HALAL/Models/DMA/DMA.hpp"

#include "HALAL/Services/DigitalOutputService/DigitalOutputService.hpp"
#include "HALAL/Services/DigitalInputService/DigitalInputService.hpp"

#include "HALAL/Services/Flash/Flash.hpp"
#include "HALAL/Services/Flash/FlashTests/Flash_Test.hpp"

#include "HALAL/Services/ADC/ADC.hpp"

#include "HALAL/Services/PWM/PWM/PWM.hpp"
#include "HALAL/Services/PWM/DualPWM/DualPWM.hpp"
#include "HALAL/Services/PWM/DualCenterPWM/DualCenterPWM.hpp"
#include "HALAL/Services/PWM/PhasedPWM/PhasedPWM.hpp"
#include "HALAL/Services/PWM/DualPhasedPWM/DualPhasedPWM.hpp"

#include "HALAL/Services/Time/Time.hpp"
#include "HALAL/Services/Time/Scheduler.hpp"
#include "HALAL/Services/Time/RTC.hpp"

#include "HALAL/Services/InputCapture/InputCapture.hpp"
#include "HALAL/Services/Encoder/Encoder.hpp"
#include "HALAL/Services/EXTI/EXTI.hpp"

#include "HALAL/Services/Communication/SPI/SPI.hpp"
#include "HALAL/Services/Communication/UART/UART.hpp"
#include "HALAL/Services/Communication/I2C/I2C.hpp"
#include "HALAL/Services/Communication/FDCAN/FDCAN.hpp"

#include "HALAL/Services/CORDIC/CORDIC.hpp"
#include "HALAL/Services/FMAC/FMAC.hpp"

#include "HALAL/Models/MPUManager/MPUManager.hpp"
#include "HALAL/Services/InfoWarning/InfoWarning.hpp"
#include "HALAL/Services/Watchdog/Watchdog.hpp"

#include "HALAL/Models/TimerPeripheral/TimerPeripheral.hpp"

#include "HALAL/Models/BoardID/BoardID.hpp"
#include "HALAL/Models/Concepts/Concepts.hpp"

#ifdef STLIB_ETH
#include "HALAL/Models/Packets/Packet.hpp"
#include "HALAL/Models/Packets/Order.hpp"
#include "HALAL/Models/Packets/OrderProtocol.hpp"
#include "HALAL/Models/Packets/ForwardOrder.hpp"

#include "HALAL/Models/IPV4/IPV4.hpp"
#include "HALAL/Models/MAC/MAC.hpp"

#include "HALAL/Services/Communication/Ethernet/UDP/DatagramSocket.hpp"
#include "HALAL/Services/Communication/Ethernet/TCP/ServerSocket.hpp"
#include "HALAL/Services/Communication/Ethernet/TCP/Socket.hpp"
#include "HALAL/Services/Communication/Ethernet/Ethernet.hpp"
#include "HALAL/Services/Communication/SNTP/SNTP.hpp"
#endif

namespace HALAL {

#ifdef STLIB_ETH
    void start(MAC mac,
               IPV4 ip,
               IPV4 subnet_mask,
               IPV4 gateway,
               UART::Peripheral& printf_peripheral);
#else
    void start(UART::Peripheral& printf_peripheral);
#endif

} // namespace HALAL