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

option(USE_ETHERNET "Enable ethernet peripheral" OFF)
option(TARGET_NUCLEO "Targets the STM32H723 Nucleo development board" OFF)

message(STATUS "${PROJECT_NAME} Ethernet: ${USE_ETHERNET}")
message(STATUS "${PROJECT_NAME} Nucleo:   ${TARGET_NUCLEO}")

if ((PROJECT_IS_TOP_LEVEL AND (NOT CMAKE_CROSSCOMPILING)))
  include(FetchContent)
  option(BUILD_GMOCK OFF)
  option(INSTALL_GTEST OFF)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
  )
  FetchContent_MakeAvailable(googletest)
  add_library(GTest::GTest INTERFACE IMPORTED)
  target_link_libraries(GTest::GTest INTERFACE gtest_main)
endif()

if (NOT CMAKE_CROSSCOMPILING)
  add_subdirectory(Tests)
endif()

set(STM32CUBEH7 "${CMAKE_CURRENT_LIST_DIR}/STM32CubeH7")
message("STM32CUBEH7: ${STM32CUBEH7}")

if (CMAKE_CROSSCOMPILING)
  set(HAL_DRIVER_SRC_DIR ${STM32CUBEH7}/Drivers/STM32H7xx_HAL_Driver/Src)

  set(HAL_SOURCES
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

      # Ethernet
      ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_eth.c
      ${HAL_DRIVER_SRC_DIR}/stm32h7xx_hal_eth_ex.c

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
  )

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

  file(GLOB_RECURSE USER_SOURCE_LWIP ${CMAKE_CURRENT_LIST_DIR}/LWIP/*.c)

  # HALAL
  file(GLOB_RECURSE HALAL_C ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/*.c)
  file(GLOB_RECURSE HALAL_CPP ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/*.cpp)
endif()

# CPP UTILITIES
file(GLOB_RECURSE CPP_UTILITIES_C ${CMAKE_CURRENT_LIST_DIR}/Src/C++Utilities/*.c)
file(GLOB_RECURSE CPP_UTILITIES_CPP ${CMAKE_CURRENT_LIST_DIR}/Src/C++Utilities/*.cpp)

# HALAL Mock
file(GLOB_RECURSE HALAL_MOCK_C ${CMAKE_CURRENT_LIST_DIR}/Src/HALALMock/*.c)
file(GLOB_RECURSE HALAL_MOCK_CPP ${CMAKE_CURRENT_LIST_DIR}/Src/HALALMock/*.cpp)

# STLIB_LOW
file(GLOB_RECURSE STLIB_LOW_C ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/*.c)
file(GLOB_RECURSE STLIB_LOW_CPP ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_LOW/*.cpp)

# STLIB_HIGH
file(GLOB_RECURSE STLIB_HIGH_C ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_HIGH/*.c)
file(GLOB_RECURSE STLIB_HIGH_CPP ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB_HIGH/*.cpp)

add_library(${STLIB_LIBRARY} STATIC
  # Only compile these when cross-compiling for STM32
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HAL_SOURCES}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_SOURCES}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${USER_SOURCE_LWIP}>

  # Always-built C / C++ utilities and ST-LIB layers
  ${CPP_UTILITIES_C}
  $<IF:$<BOOL:${CMAKE_CROSSCOMPILING}>,${HALAL_C},${HALAL_MOCK_C}>
  ${STLIB_LOW_C}
  ${STLIB_HIGH_C}

  ${CPP_UTILITIES_CPP}
  $<IF:$<BOOL:${CMAKE_CROSSCOMPILING}>,${HALAL_CPP},${HALAL_MOCK_CPP}>
  ${CMAKE_CURRENT_LIST_DIR}/Src/HALAL/HALAL.cpp
  ${STLIB_LOW_CPP}
  ${STLIB_HIGH_CPP}
  ${CMAKE_CURRENT_LIST_DIR}/Src/ST-LIB.cpp
)

set_target_properties(${STLIB_LIBRARY} PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED C_STANDARD 11 C_STANDARD_REQUIRED)

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
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-specs=nosys.specs>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-ffunction-sections>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-fdata-sections>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-fexceptions>
  
  -Wno-psabi
  
  $<$<COMPILE_LANGUAGE:C>:-w>
  
  $<$<COMPILE_LANGUAGE:CXX>:-Wall>
  $<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>
  $<$<COMPILE_LANGUAGE:CXX>:-Werror> 
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-gnu-zero-variadic-macro-arguments>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-inconsistent-missing-override>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-address-of-packed-member>  # Add this line
)

target_include_directories(${STLIB_LIBRARY} PUBLIC
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/CMSIS/Device/ST/STM32H7xx/Include>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/CMSIS/Include>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/CMSIS/Core_A/Include>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/STM32H7xx_HAL_Driver/Inc>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/src/include>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/system>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/src/netif/ppp>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/src/include/lwip>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/src/include/lwip/apps>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/src/include/lwip/priv>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/src/include/lwip/prot>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/src/include/netif>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/src/include/compat>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/src/include/compat/posix>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/src/include/compat/posix/arpa>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/src/include/compat/posix/net>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/src/include/compat/posix/sys>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/src/include/compat/stdc>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${LWIP_DIR}/system/arch>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/BSP/Components>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STM32CUBEH7}/Drivers/BSP/Components/lan8742>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${CMAKE_CURRENT_LIST_DIR}/LWIP/App>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${CMAKE_CURRENT_LIST_DIR}/LWIP/Target>
  ${CMAKE_CURRENT_LIST_DIR}/Inc
  ${CMAKE_CURRENT_LIST_DIR}/Inc/ST-LIB_LOW
  ${CMAKE_CURRENT_LIST_DIR}/Inc/ST-LIB_HIGH
)

if (PROJECT_IS_TOP_LEVEL)
    # Create symlink to compile_commands.json for IDE to pick it up
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    )
endif()
