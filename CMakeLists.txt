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

message(STATUS "${PROJECT_NAME} Ethernet: ${USE_ETHERNET}")
message(STATUS "${PROJECT_NAME} Nucleo:   ${TARGET_NUCLEO}")
message(STATUS "${PROJECT_NAME} Crosscompiling: ${CMAKE_CROSSCOMPILING}")

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

if(NOT CMAKE_CROSSCOMPILING)
  message(STATUS "Compiling for simulator")
  add_subdirectory(Tests)
else()
  execute_process(
    COMMAND git submodule update --init --depth 1 Drivers/BSP/Components/lan8742
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7
  )
endif()

# ============================
# STM32CubeH7 (HAL + LWIP)
# ============================
set(STM32CUBEH7 "${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7")
message(STATUS "STM32CUBEH7: ${STM32CUBEH7}")

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
    set(LAN8742_DIR ${STM32CUBEH7}/Drivers/BSP/Components/lan8742)

    file(GLOB LWIP_SOURCES
      ${LWIP_DIR}/src/core/*.c
      ${LWIP_DIR}/src/core/ipv4/*.c
      ${LWIP_DIR}/src/core/ipv6/*.c
      ${LWIP_DIR}/src/api/*.c
      ${LWIP_DIR}/src/netif/*.c
      ${LWIP_DIR}/system/*.c
      ${LAN8742_DIR}/lan8742.c
    )

    file(GLOB_RECURSE USER_LWIP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/LWIP/*.c)
  endif()
endif()

# ============================
# HALAL: ETH / NO_ETH
# ============================
file(GLOB_RECURSE HALAL_C_ALL ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/*.c)
file(GLOB_RECURSE HALAL_CPP_ALL ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/*.cpp)

set(HALAL_ETH_REGEX ".*/HALAL/(Models/(IPV4|MAC)|Services/Communication/(Ethernet|SNTP))/.*")

set(HALAL_C_ETH ${HALAL_C_ALL})
list(FILTER HALAL_C_ETH INCLUDE REGEX "${HALAL_ETH_REGEX}")

set(HALAL_CPP_ETH ${HALAL_CPP_ALL})
list(FILTER HALAL_CPP_ETH INCLUDE REGEX "${HALAL_ETH_REGEX}")

set(HALAL_C_NO_ETH ${HALAL_C_ALL})
list(FILTER HALAL_C_NO_ETH EXCLUDE REGEX "${HALAL_ETH_REGEX}")

set(HALAL_CPP_NO_ETH ${HALAL_CPP_ALL})
list(FILTER HALAL_CPP_NO_ETH EXCLUDE REGEX "${HALAL_ETH_REGEX}")

# ============================
# C++ Utilities
# ============================
file(GLOB_RECURSE CPP_UTILITIES_C ${CMAKE_CURRENT_LIST_DIR}/Src/C++Utilities/*.c)
file(GLOB_RECURSE CPP_UTILITIES_CPP ${CMAKE_CURRENT_LIST_DIR}/Src/C++Utilities/*.cpp)

# ============================
# ST-LIB_LOW: ETH / NO_ETH
# ============================
file(GLOB_RECURSE STLIB_LOW_C_ALL ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/*.c)
file(GLOB_RECURSE STLIB_LOW_CPP_ALL ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/*.cpp)

set(STLIB_LOW_ETH_REGEX ".*/ST-LIB_LOW/(Communication/.*|StateMachine/(StateOrder|HeapStateOrder|StackStateOrder)\\.cpp)$")
set(STLIB_LOW_C_ETH ${STLIB_LOW_C_ALL})
list(FILTER STLIB_LOW_C_ETH INCLUDE REGEX "${STLIB_LOW_ETH_REGEX}")

set(STLIB_LOW_CPP_ETH ${STLIB_LOW_CPP_ALL})
list(FILTER STLIB_LOW_CPP_ETH INCLUDE REGEX "${STLIB_LOW_ETH_REGEX}")

set(STLIB_LOW_C_NO_ETH ${STLIB_LOW_C_ALL})
list(FILTER STLIB_LOW_C_NO_ETH EXCLUDE REGEX "${STLIB_LOW_ETH_REGEX}")

set(STLIB_LOW_CPP_NO_ETH ${STLIB_LOW_CPP_ALL})
list(FILTER STLIB_LOW_CPP_NO_ETH EXCLUDE REGEX "${STLIB_LOW_ETH_REGEX}")

# ============================
# ST-LIB_HIGH
# ============================

file(GLOB_RECURSE STLIB_HIGH_C_ALL ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_HIGH/*.c)
file(GLOB_RECURSE STLIB_HIGH_CPP_ALL ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_HIGH/*.cpp)

set(STLIB_HIGH_ETH_REGEX ".*/ST-LIB_HIGH/Protections/.*\\.cpp$")
set(STLIB_HIGH_C_ETH ${STLIB_HIGH_C_ALL})
list(FILTER STLIB_HIGH_C_ETH INCLUDE REGEX "${STLIB_HIGH_ETH_REGEX}")

set(STLIB_HIGH_CPP_ETH ${STLIB_HIGH_CPP_ALL})
list(FILTER STLIB_HIGH_CPP_ETH INCLUDE REGEX "${STLIB_HIGH_ETH_REGEX}")

set(STLIB_HIGH_C_NO_ETH ${STLIB_HIGH_C_ALL})
list(FILTER STLIB_HIGH_C_NO_ETH EXCLUDE REGEX "${STLIB_HIGH_ETH_REGEX}")

set(STLIB_HIGH_CPP_NO_ETH ${STLIB_HIGH_CPP_ALL})
list(FILTER STLIB_HIGH_CPP_NO_ETH EXCLUDE REGEX "${STLIB_HIGH_ETH_REGEX}")

# ============================
# Librería STLIB_LIBRARY
# ============================

add_library(${STLIB_LIBRARY} STATIC
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HAL_SOURCES_COMMON}>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${HAL_SOURCES_ETH}>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${LWIP_SOURCES}>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${USER_LWIP_SOURCES}>

  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HALAL_C_NO_ETH}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HALAL_CPP_NO_ETH}>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${HALAL_C_ETH}>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${HALAL_CPP_ETH}>

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

  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:TESTING_ENV>

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
  #$<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/CMSIS/Device/ST/STM32H7xx/Include>
  #$<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/CMSIS/Include>
  ${STM32CUBEH7}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
  ${STM32CUBEH7}/Drivers/CMSIS/Include
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/CMSIS/Core_A/Include>
  #$<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/STM32H7xx_HAL_Driver/Inc>
  ${STM32CUBEH7}/Drivers/STM32H7xx_HAL_Driver/Inc
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy>

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
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${STM32CUBEH7}/Drivers/BSP/Components/lan8742>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${CMAKE_CURRENT_LIST_DIR}/LWIP/App>
  $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<BOOL:${USE_ETHERNET}>>:${CMAKE_CURRENT_LIST_DIR}/LWIP/Target>

  ${CMAKE_CURRENT_LIST_DIR}/Inc
  ${CMAKE_CURRENT_LIST_DIR}/Inc/ST-LIB_LOW
  ${CMAKE_CURRENT_LIST_DIR}/Inc/ST-LIB_HIGH
)


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
  target_include_directories(${STLIB_LIBRARY} PUBLIC
    ${CMAKE_SOURCE_DIR}/Core/Inc
  )
endif()

if(PROJECT_IS_TOP_LEVEL)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
  )
endif()
