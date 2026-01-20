cmake_minimum_required (VERSION 3.14)

if(DEFINED STLIB_NAME_SUFFIX)
  project (ST-LIB-${STLIB_NAME_SUFFIX} C CXX)
  set(STLIB_LIBRARY st-lib-${STLIB_NAME_SUFFIX})
else()
  project (ST-LIB C CXX)
  set(STLIB_LIBRARY st-lib)
endif()

if (NOT PROJECT_IS_TOP_LEVEL)
  set(STLIB_LIBRARY ${STLIB_LIBRARY} PARENT_SCOPE)
endif()

option(USE_ETHERNET "Enable ethernet peripheral" ON)
option(TARGET_NUCLEO "Targets the STM32H723 Nucleo development board" OFF)

if ((PROJECT_IS_TOP_LEVEL AND (NOT CMAKE_CROSSCOMPILING)))
  option(BUILD_GMOCK OFF)
  option(INSTALL_GTEST OFF)
endif()

set(STM32CUBEH7 "${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7")

if (CMAKE_CROSSCOMPILING)
  
  set(HAL_DRIVER_SRC_DIR ${STM32CUBEH7}/Drivers/STM32H7xx_HAL_Driver/Src)

  set(HAL_SOURCES
      ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal.c
      ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_cortex.c
      ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_rcc.c
      ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_rcc_ex.c
      ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_flash.c
      ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_flash_ex.c
      ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_pwr.c
      ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_pwr_ex.c
      ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_gpio.c
      
      ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_dma.c
      ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_dma_ex.c
      ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_eth.c
      ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_eth_ex.c
      
  
  )

  set(LWIP_DIR ${STM32CUBEH7}/Middlewares/Third_Party/LwIP)
  set(LAN8742_DIR ${STM32CUBEH7}/Drivers/BSP/Components/lan8742)

  file(GLOB LWIP_SOURCES
      ${LWIP_DIR}/src/core/*.c
      ${LWIP_DIR}/src/core/ipv4/*.c
      # ${LWIP_DIR}/src/api/*.c
      ${LWIP_DIR}/src/netif/*.c
      ${LWIP_DIR}/system/*.c
      ${LWIP_DIR}/src/apps/tftp/tftp_server.c
      ${LAN8742_DIR}/lan8742.c
  )
  
  file(GLOB_RECURSE USER_SOURCE_LWIP ${CMAKE_CURRENT_LIST_DIR}/LWIP/*.c)

  
  set(HALAL_C 
      ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/HALconfig/Halconfig.cpp 
  )
  
  set(HALAL_CPP
      ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/HALAL.cpp
      ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/HALconfig/Halconfig.cpp
      ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/PinModel/Pin.cpp
      ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/DMA/DMA.cpp
      ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/MPUManager/MPUManager.cpp
      ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/IPV4/IPV4.cpp
      ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/MAC/MAC.cpp
      
      ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/Ethernet/Ethernet.cpp
      ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/Ethernet/EthernetNode.cpp
      ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Communication/Ethernet/UDP/DatagramSocket.cpp
      
      ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Flash/Flash.cpp
      ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/DigitalOutputService/DigitalOutputService.cpp
      
    
  )
  

  set(STLIB_LOW_CPP 
      ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/ErrorHandler/ErrorHandler.cpp
  )
  set(STLIB_LOW_C "")

  set(STLIB_HIGH_CPP "")
  set(STLIB_HIGH_C "")

endif()

file(GLOB_RECURSE CPP_UTILITIES_C ${CMAKE_CURRENT_LIST_DIR}/Src/C++Utilities/*.c)
file(GLOB_RECURSE CPP_UTILITIES_CPP ${CMAKE_CURRENT_LIST_DIR}/Src/C++Utilities/*.cpp)



add_library(${STLIB_LIBRARY} STATIC
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HAL_SOURCES}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_SOURCES}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${USER_SOURCE_LWIP}>

  ${CPP_UTILITIES_C}
  ${CPP_UTILITIES_CPP}

  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HALAL_C}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HALAL_CPP}>
  
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_LOW_C}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_LOW_CPP}>
  
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB.cpp>
  # $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Services/Time/Time.cpp>
  # $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/Models/TimerPeripheral/TimerPeripheral.cpp>
  
  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:${HALAL_MOCK_C}>
  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:${HALAL_MOCK_CPP}>
)

set_target_properties(${STLIB_LIBRARY} PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED C_STANDARD 11 C_STANDARD_REQUIRED)

target_compile_definitions(${STLIB_LIBRARY} PRIVATE
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:USE_HAL_DRIVER>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:STM32H723xx>
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
  
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-Os> 
  
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-specs=nano.specs>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-ffunction-sections>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-fdata-sections>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-fno-exceptions>
  
  -Wno-psabi
  $<$<COMPILE_LANGUAGE:C>:-w>
  $<$<COMPILE_LANGUAGE:CXX>:-Wall>
  $<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-gnu-zero-variadic-macro-arguments>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-inconsistent-missing-override>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-address-of-packed-member>
)

target_include_directories(${STLIB_LIBRARY} PUBLIC
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/CMSIS/Device/ST/STM32H7xx/Include>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/CMSIS/Include>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/CMSIS/Core_A/Include>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/STM32H7xx_HAL_Driver/Inc>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/src/include>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/system>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/src/include/lwip>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/src/include/netif>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/system/arch>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/BSP/Components/lan8742>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${CMAKE_CURRENT_LIST_DIR}/LWIP/App>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${CMAKE_CURRENT_LIST_DIR}/LWIP/Target>
  ${CMAKE_CURRENT_LIST_DIR}/Inc
  ${CMAKE_CURRENT_LIST_DIR}/Inc/ST-LIB_LOW
  ${CMAKE_CURRENT_LIST_DIR}/Inc/ST-LIB_HIGH
)

if (PROJECT_IS_TOP_LEVEL)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    )
endif()