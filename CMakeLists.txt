cmake_minimum_required(VERSION 3.14)

if(DEFINED STLIB_NAME_SUFFIX)
  project(ST-LIB-${STLIB_NAME_SUFFIX} C CXX)
  set(STLIB_LIBRARY st-lib-${STLIB_NAME_SUFFIX})
else()
  project(ST-LIB C CXX)
  set(STLIB_LIBRARY st-lib)
endif()
if(NOT PROJECT_IS_TOP_LEVEL)
  set(STLIB_LIBRARY ${STLIB_LIBRARY} PARENT_SCOPE)
endif()

include(CTest)
enable_testing()

option(USE_ETHERNET "Enable ethernet peripheral" OFF)
option(TARGET_NUCLEO "Targets the STM32H723 Nucleo development board" OFF)
option(USE_CCACHE "Use ccache if available" ON)
option(STLIB_USE_PCH "Enable precompiled headers for ST-LIB" ON)
if(NOT DEFINED ENABLE_LTO)
  if(CMAKE_CROSSCOMPILING)
    set(ENABLE_LTO OFF)
  else()
    set(ENABLE_LTO ON)
  endif()
endif()
set(ENABLE_LTO "${ENABLE_LTO}" CACHE BOOL "Enable link-time optimization (LTO)")
set(LAN8742_PHY_ADDRESS 0 CACHE STRING "LAN8742 PHY address (strap-dependent)")
set(LAN8700_PHY_ADDRESS 31 CACHE STRING "LAN8700 PHY address (strap-dependent)")
set(KSZ8041_PHY_ADDRESS 1 CACHE STRING "KSZ8041 PHY address (strap-dependent)")
set(LAN8700_FORCE_100FD OFF CACHE BOOL "Force LAN8700 to 100Mbps Full Duplex (debug)")

if(USE_CCACHE)
  find_program(CCACHE_PROGRAM ccache)
  if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_ASM_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
  endif()
endif()

# ============================
# PHY selection (Ethernet)
# ============================

set(SELECTED_PHY "")
set(USE_PHY_LAN8742 OFF)
set(USE_PHY_LAN8700 OFF)
set(USE_PHY_KSZ8041 OFF)

if(USE_ETHERNET)

  if(TARGET_NUCLEO)
    # Nucleo boards always use LAN8742
    set(SELECTED_PHY "LAN8742")
  else()
    if(NOT DEFINED PHY_TYPE)
      message(FATAL_ERROR
        "USE_ETHERNET=ON and TARGET_NUCLEO=OFF but PHY_TYPE not set (KSZ8041, LAN8742, or LAN8700)")
    endif()
    set(SELECTED_PHY "${PHY_TYPE}")
  endif()

  if(SELECTED_PHY STREQUAL "LAN8742")
    set(USE_PHY_LAN8742 ON)
  elseif(SELECTED_PHY STREQUAL "LAN8700")
    set(USE_PHY_LAN8700 ON)
  elseif(SELECTED_PHY STREQUAL "KSZ8041")
    set(USE_PHY_KSZ8041 ON)
  else()
    message(FATAL_ERROR "Unknown PHY: ${SELECTED_PHY}")
  endif()

  message(STATUS "${PROJECT_NAME} PHY: ${SELECTED_PHY}")

endif()

message(STATUS "${PROJECT_NAME} Ethernet: ${USE_ETHERNET}")
message(STATUS "${PROJECT_NAME} Nucleo:   ${TARGET_NUCLEO}")
message(STATUS "${PROJECT_NAME} Crosscompiling: ${CMAKE_CROSSCOMPILING}")

if(CMAKE_CROSSCOMPILING)
  if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Drivers)
    execute_process(
      COMMAND git submodule update --init --depth 1
      WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )
  endif()

  if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Drivers/CMSIS/Device/ST/STM32H7xx/Include)
    execute_process(
      COMMAND git submodule update --init --depth 1 Drivers/CMSIS/Device/ST/STM32H7xx Drivers/STM32H7xx_HAL_Driver
      WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7
    )
  endif()
endif()

if(NOT CMAKE_CROSSCOMPILING)
  message(STATUS "Compiling for simulator")
  add_subdirectory(Tests)
else()
  if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Drivers/BSP/Components/lan8742)
    execute_process(
      COMMAND git submodule update --init --depth 1 Drivers/BSP/Components/lan8742
      WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7
    )
  endif()
endif()

# ============================
# STM32CubeH7 (HAL + LWIP)
# ============================
set(STM32CUBEH7 "${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7")
if(CMAKE_CROSSCOMPILING)
  message(STATUS "STM32CUBEH7: ${STM32CUBEH7}")
endif()

if(CMAKE_CROSSCOMPILING)
  message(STATUS "Compiling for STM32H723G")

  set(HAL_DRIVER_SRC_DIR ${STM32CUBEH7}/Drivers/STM32H7xx_HAL_Driver/Src)

  set(HAL_SOURCES_COMMON
    # Core HAL stuff
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal.c

    # Common "infrastructure"
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_cortex.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_rcc.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_rcc_ex.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_flash.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_flash_ex.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_pwr.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_pwr_ex.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_dma.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_dma_ex.c

    # GPIO / EXTI
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_gpio.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_exti.c

    # MDMA
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_mdma.c

    # ADC
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_adc.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_adc_ex.c

    # DFSDM
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_dfsdm.c

    # PWM -> TIM
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_tim.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_tim_ex.c

    # UART
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_uart.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_uart_ex.c

    # SD
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_sd.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_sd_ex.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_ll_sdmmc.c

    # CAN / FDCAN
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_fdcan.c

    # SPI
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_spi.c

    # LPTIM (low-power timers)
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_lptim.c

    # RTC
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_rtc.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_rtc_ex.c

    # I2C
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_i2c.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_i2c_ex.c

    # FMAC
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_fmac.c

    # CORDIC
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_cordic.c

    # Watchdog
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_wwdg.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_iwdg.c
  )

  # HAL Ethernet
  set(HAL_SOURCES_ETH
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_eth.c
    ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_eth_ex.c
  )

  if(USE_ETHERNET)
    set(LWIP_DIR ${STM32CUBEH7}/Middlewares/Third_Party/LwIP)

    set(LWIP_SOURCES
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/def.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/init.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/ip.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/inet_chksum.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/mem.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/memp.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/netif.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/pbuf.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/raw.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/stats.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/sys.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/tcp.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/tcp_in.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/tcp_out.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/timeouts.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/udp.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/ipv4/etharp.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/ipv4/icmp.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/ipv4/ip4.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/ipv4/ip4_addr.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/core/ipv4/ip4_frag.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Middlewares/Third_Party/LwIP/src/netif/ethernet.c
    )

    set(USER_LWIP_SOURCES
      ${CMAKE_CURRENT_LIST_DIR}/LWIP/App/lwip.c
      ${CMAKE_CURRENT_LIST_DIR}/LWIP/Target/ethernetif.c
    )
  endif()
endif()

# ============================
# HALAL: ETH / NO_ETH
# ============================
set(HALAL_C_NO_ETH
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/HardFault/HardfaultTrace.c
)

set(HALAL_CPP_NO_ETH
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/HALAL.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/DMA/DMA.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/HALconfig/Halconfig.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/LowPowerTimer/LowPowerTimer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/MDMA/MDMA.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/MPUManager/MPUManager.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/Packets/SPIOrder.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/SPI/SPI2.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/DMA/DMA2.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/PinModel/Pin.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/TimerDomain/TimerDomain.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/TimerPeripheral/TimerPeripheral.cpp
  # ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/ADC/ADC.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/CORDIC/CORDIC.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/FDCAN/FDCAN.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/I2C/I2C.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/SPI/SPI.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/UART/UART.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/DigitalInputService/DigitalInputService.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/DigitalOutputService/DigitalOutputService.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/EXTI/EXTI.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Encoder/Encoder.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/FMAC/FMAC.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Flash/Flash.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Flash/FlashTests/Flash_Test.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/InfoWarning/InfoWarning.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/InputCapture/InputCapture.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/PWM/DualCenterPWM/DualCenterPWM.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/PWM/DualPWM/DualPWM.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/PWM/DualPhasedPWM/DualPhasedPWM.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/PWM/PWM/PWM.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/PWM/PhasedPWM/PhasedPWM.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Time/RTC.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Time/Scheduler.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Time/Time.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Watchdog/Watchdog.cpp
)


set(HALAL_C_ETH_CORE)
set(HALAL_CPP_ETH_CORE
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/IPV4/IPV4.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/MAC/MAC.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/Packets/Packet.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/SNTP/SNTP.cpp
)

set(HALAL_C_ETH_LWIP)
set(HALAL_CPP_ETH_LWIP
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/Ethernet/LWIP/Ethernet.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/Ethernet/LWIP/EthernetNode.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/Ethernet/LWIP/TCP/OrderProtocol.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/Ethernet/LWIP/TCP/ServerSocket.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/Ethernet/LWIP/TCP/Socket.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/Ethernet/LWIP/UDP/DatagramSocket.cpp
)

set(HALAL_C_ETH_PHY)
if(USE_ETHERNET)
  list(APPEND HALAL_C_ETH_PHY
    ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/Ethernet/PHY/phy_select.c
  )

  if(USE_PHY_LAN8742)
    list(APPEND HALAL_C_ETH_PHY
      ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/Ethernet/PHY/lan8742.c
      ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7/Drivers/BSP/Components/lan8742/lan8742.c
    )
  endif()

  if(USE_PHY_LAN8700)
    list(APPEND HALAL_C_ETH_PHY
      ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/Ethernet/PHY/lan8700.c
    )
  endif()

  if(USE_PHY_KSZ8041)
    list(APPEND HALAL_C_ETH_PHY
      ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/Ethernet/PHY/ksz8041.c
    )
  endif()
endif()

# ============================
# C++ Utilities
# ============================
set(CPP_UTILITIES_C)
set(CPP_UTILITIES_CPP)

# ============================
# ST-LIB_LOW: ETH / NO_ETH
# ============================
set(STLIB_LOW_C_ETH)

set(STLIB_LOW_CPP_ETH
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/Communication/Server/Server.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/StateMachine/StateOrder.cpp
)

set(STLIB_LOW_C_NO_ETH)

set(STLIB_LOW_CPP_NO_ETH
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/Clocks/Counter.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/Clocks/Stopwatch.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/DigitalOutput/DigitalOutput.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/ErrorHandler/ErrorHandler.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/HalfBridge/HalfBridge.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/Math/Math.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/ST-LIB_LOW.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/Sd/Sd.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/Sensors/DigitalSensor/DigitalSensor.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/Sensors/LookupSensor/LookupSensor.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/Sensors/NTC/NTC.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/Sensors/Sensor/Sensor.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/Sensors/SensorInterrupt/SensorInterrupt.cpp
)

# ============================
# ST-LIB_HIGH
# ============================

set(STLIB_HIGH_C_ETH)

set(STLIB_HIGH_CPP_ETH
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_HIGH/Protections/Boundary.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_HIGH/Protections/ProtectionManager.cpp
)

set(STLIB_HIGH_C_NO_ETH)

set(STLIB_HIGH_CPP_NO_ETH
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_HIGH/FlashStorer/FlashStorer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_HIGH/FlashStorer/FlashVariable.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_HIGH/ST-LIB_HIGH.cpp
)

# ============================
# Librería STLIB_LIBRARY
# ============================

add_library(${STLIB_LIBRARY} OBJECT
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HAL_SOURCES_COMMON}>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${HAL_SOURCES_ETH}>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_SOURCES}>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${USER_LWIP_SOURCES}>

  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HALAL_C_NO_ETH}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HALAL_CPP_NO_ETH}>

  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${HALAL_C_ETH_CORE}>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${HALAL_CPP_ETH_CORE}>

  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${HALAL_C_ETH_LWIP}>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${HALAL_CPP_ETH_LWIP}>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${HALAL_C_ETH_PHY}>


  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${CPP_UTILITIES_C}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${CPP_UTILITIES_CPP}>

  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_LOW_C_NO_ETH}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_LOW_CPP_NO_ETH}>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${STLIB_LOW_C_ETH}>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${STLIB_LOW_CPP_ETH}>

  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_HIGH_C_NO_ETH}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_HIGH_CPP_NO_ETH}>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${STLIB_HIGH_C_ETH}>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${STLIB_HIGH_CPP_ETH}>

  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB.cpp>

  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Time/Scheduler.cpp>
  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/TimerDomain/TimerDomain.cpp>
  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:${CMAKE_CURRENT_LIST_DIR}/Src/MockedDrivers/mocked_hal_adc.cpp>
  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:${CMAKE_CURRENT_LIST_DIR}/Src/MockedDrivers/mocked_ll_tim.cpp>
  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:${CMAKE_CURRENT_LIST_DIR}/Src/MockedDrivers/mocked_system_stm32h7xx.c>
  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:${CMAKE_CURRENT_LIST_DIR}/Src/MockedDrivers/stm32h723xx_wrapper.c>
  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:${CMAKE_CURRENT_LIST_DIR}/Src/MockedDrivers/NVIC.cpp>
)

set_target_properties(${STLIB_LIBRARY} PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED YES
  C_STANDARD 17
  C_STANDARD_REQUIRED YES
)

target_compile_definitions(${STLIB_LIBRARY} PUBLIC
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:USE_HAL_DRIVER>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:STM32H723xx>

  $<$<BOOL:${USE_PHY_LAN8742}>:USE_PHY_LAN8742>
  $<$<BOOL:${USE_PHY_LAN8700}>:USE_PHY_LAN8700>
  $<$<BOOL:${USE_PHY_KSZ8041}>:USE_PHY_KSZ8041>
  $<$<BOOL:${USE_PHY_LAN8742}>:LAN8742_PHY_ADDRESS=${LAN8742_PHY_ADDRESS}>
  $<$<BOOL:${USE_PHY_LAN8700}>:LAN8700_PHY_ADDRESS=${LAN8700_PHY_ADDRESS}>
  $<$<BOOL:${USE_PHY_KSZ8041}>:KSZ8041_PHY_ADDRESS=${KSZ8041_PHY_ADDRESS}>
  $<$<AND:$<BOOL:${USE_PHY_LAN8700}>,$<BOOL:${LAN8700_FORCE_100FD}>>:LAN8700_FORCE_100FD>

  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:SIM_ON>

  $<$<BOOL:${USE_ETHERNET}>:STLIB_ETH>
  $<IF:$<BOOL:${TARGET_NUCLEO}>,NUCLEO,BOARD>
  $<IF:$<BOOL:${TARGET_NUCLEO}>,HSE_VALUE=8000000,HSE_VALUE=25000000>
)

target_compile_options(${STLIB_LIBRARY} PRIVATE
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-mcpu=cortex-m7>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-mfpu=fpv5-d16>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-mfloat-abi=hard>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-mthumb>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-specs=nosys.specs>
  $<$<CONFIG:Debug>:-g>
  $<$<CONFIG:RelWithDebInfo>:-g>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-ffunction-sections>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-fdata-sections>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-fno-exceptions>

  -Wno-psabi

  $<$<COMPILE_LANGUAGE:C>:-w>

  $<$<COMPILE_LANGUAGE:CXX>:-Wall>
  $<$<COMPILE_LANGUAGE:CXX>:-Werror>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-gnu-zero-variadic-macro-arguments>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-inconsistent-missing-override>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-address-of-packed-member>
)

target_include_directories(${STLIB_LIBRARY} PUBLIC
  # CMSIS + HAL
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/CMSIS/Device/ST/STM32H7xx/Include>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/CMSIS/Include>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/CMSIS/Core_A/Include>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/STM32H7xx_HAL_Driver/Inc>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy>
  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:${CMAKE_CURRENT_LIST_DIR}/Inc/MockedDrivers>
  ${CMAKE_CURRENT_LIST_DIR}/Inc/STM32CubeH7Wrapper

  # LWIP includes 
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_DIR}/src/include>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_DIR}/system>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_DIR}/src/netif/ppp>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_DIR}/src/include/lwip>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_DIR}/src/include/lwip/apps>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_DIR}/src/include/lwip/priv>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_DIR}/src/include/lwip/prot>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_DIR}/src/include/netif>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_DIR}/src/include/compat>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_DIR}/src/include/compat/posix>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_DIR}/src/include/compat/posix/arpa>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_DIR}/src/include/compat/posix/net>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_DIR}/src/include/compat/posix/sys>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_DIR}/src/include/compat/stdc>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_DIR}/system/arch>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${STM32CUBEH7}/Drivers/BSP/Components>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>,$<BOOL:${USE_PHY_LAN8742}>>:${STM32CUBEH7}/Drivers/BSP/Components/lan8742>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/Ethernet>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${CMAKE_CURRENT_LIST_DIR}/LWIP/App>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${CMAKE_CURRENT_LIST_DIR}/LWIP/Target>

  ${CMAKE_CURRENT_LIST_DIR}/Inc
  ${CMAKE_CURRENT_LIST_DIR}/Inc/ST-LIB_LOW
  ${CMAKE_CURRENT_LIST_DIR}/Inc/ST-LIB_HIGH
)

if(ENABLE_LTO)
  set_property(TARGET ${STLIB_LIBRARY} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
  set_property(TARGET ${STLIB_LIBRARY} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
endif()

if(STLIB_USE_PCH)
  if(COMMAND target_precompile_headers)
    target_precompile_headers(${STLIB_LIBRARY} PRIVATE
      $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_LIST_DIR}/Inc/C++Utilities/CppImports.hpp>
    )
  else()
    message(WARNING "STLIB_USE_PCH=ON but target_precompile_headers is not available in this CMake version")
  endif()
endif()


if(PROJECT_IS_TOP_LEVEL)
  configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/Inc/stm32h7xx_hal_conf_template.h
    ${CMAKE_CURRENT_BINARY_DIR}/stm32h7xx_hal_conf.h
    COPYONLY
  )

  target_include_directories(${STLIB_LIBRARY} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
  )
else()
  if(CMAKE_CROSSCOMPILING)
    target_include_directories(${STLIB_LIBRARY} PUBLIC
      ${CMAKE_SOURCE_DIR}/Core/Inc
    )
  endif()
endif()

if(PROJECT_IS_TOP_LEVEL)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
  )
endif()
