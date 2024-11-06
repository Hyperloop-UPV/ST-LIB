cmake_minimum_required (VERSION 3.4)

project (ST-LIB ASM C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LIBRARY st-lib)
  
file(GLOB_RECURSE SOURCE_C ${CMAKE_SOURCE_DIR}/ *.c)
file(GLOB_RECURSE SOURCE_CPP ${CMAKE_SOURCE_DIR}/ *.cpp)
file(GLOB_RECURSE SOURCE_H ${CMAKE_SOURCE_DIR}/ *.h)
file(GLOB_RECURSE SOURCE_HPP ${CMAKE_SOURCE_DIR}/ *.hpp)

if(${SIMULATE})
  include(linux-64.cmake)
  add_definitions(-DSIM_ON)
  list(FILTER SOURCE_C EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Src/HALAL/.*" )
  list(FILTER SOURCE_CPP EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Src/HALAL/.*" )
  list(FILTER SOURCE_H EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Src/HALAL/.*" )
  list(FILTER SOURCE_HPP EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Src/HALAL/.*" )


  list(FILTER SOURCE_C EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Drivers/.*" )
  list(FILTER SOURCE_CPP EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Drivers/.*" )
  list(FILTER SOURCE_H EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Drivers/.*" )
  list(FILTER SOURCE_HPP EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Drivers/.*" )

  list(FILTER SOURCE_C EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/LWIP/.*" )
  list(FILTER SOURCE_CPP EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/LWIP/.*" )
  list(FILTER SOURCE_H EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/LWIP/.*" )
  list(FILTER SOURCE_HPP EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/LWIP/.*" )

  list(FILTER SOURCE_C EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Middlewares/.*" )
  list(FILTER SOURCE_CPP EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Middlewares/.*" )
  list(FILTER SOURCE_H EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Middlewares/.*" )
  list(FILTER SOURCE_HPP EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Middlewares/.*" )
else()
  include(arm-none-eabi.cmake)
  list(FILTER SOURCE_C EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Src/HALALMock/.*" )
  list(FILTER SOURCE_CPP EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Src/HALALMock/.*" )
  list(FILTER SOURCE_H EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Src/HALALMock/.*" )
  list(FILTER SOURCE_HPP EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/Src/HALALMock/.*" )
endif()

message("Using ${CMAKE_C_COMPILER} for C")
message("Using ${CMAKE_ASM_COMPILER} for ASM")
message("Using ${CMAKE_CXX_COMPILER} for CXX")

if(${NUCLEO})
  add_definitions(-DNUCLEO)
  add_definitions(-DHSE_VALUE=8000000)
else()
  add_definitions(-DBOARD)
  add_definitions(-DHSE_VALUE=25000000)
endif()

if(${RELEASE} STREQUAL "Release")
  set(OPTIMIZATION -O3)
  set(DEBUG_CONFIGURATION -g0)
elseif(${RELEASE} STREQUAL "ReleaseDebug")
  set(OPTIMIZATION -O3)
  set(DEBUG_CONFIGURATION -g3)
else()
  set(OPTIMIZATION -O0)
  set(DEBUG_CONFIGURATION -g3)
endif()

message("\n\nPROJECT CONFIGURATION:\n")
if(${ETHERNET})
  set(ETH STLIB_ETH)
  message(STATUS "ETHERNET is ENABLED ")
else()
message(STATUS "ETHERNET is DISABLED ")
endif()
message(STATUS "[DEBUG, RELEASE, RELEASE_DEBUG]:")

if(${RELEASE} STREQUAL "Release")
  message( "\tRELEASE")
elseif(${RELEASE} STREQUAL "ReleaseDebug")
  message( "\tRELEASE_DEBUG")
else()
  message( "\tDEBUG")
endif()

message(STATUS "Optimization value: " ${OPTIMIZATION})
message(STATUS "Debug configuration value: " ${DEBUG_CONFIGURATION})

message(STATUS "[BOARD, NUCLEO]:")

if(${NUCLEO})
  message("\tNUCLEO")
else()
  message("\tBOARD")
endif()
message("\n\nEND OF PROJECT CONFIGURATION\n\n")

add_library(${LIBRARY} STATIC
  ${SOURCE_H}
  ${SOURCE_C}
  ${SOURCE_HPP}
  ${SOURCE_CPP}
)

target_compile_definitions(${LIBRARY} PUBLIC
  -DUSE_HAL_DRIVER
  -DSTM32H723xx
  -DDEBUG
  ${ETH}
  -DUSING_CMAKE
)

target_compile_options(${LIBRARY} PUBLIC
  $<$<STREQUAL:${SIMULATE},FALSE>:-mcpu=cortex-m7>
  $<$<STREQUAL:${SIMULATE},FALSE>:-mfpu=fpv5-d16>
  $<$<STREQUAL:${SIMULATE},FALSE>:-mfloat-abi=hard>
  $<$<STREQUAL:${SIMULATE},FALSE>:-mthumb>
  ${OPTIMIZATION}
  ${DEBUG_CONFIGURATION}
  -ffunction-sections
  -fdata-sections
  -fno-exceptions
  ## disable Warnings for C files, this is Driver code from ST
  $<$<COMPILE_LANGUAGE:C>:-w>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
  $<$<COMPILE_LANGUAGE:CXX>:-Wall>
  $<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>
  $<$<COMPILE_LANGUAGE:CXX>:-Werror>
  -Wno-psabi
  $<$<STREQUAL:${SIMULATE},FALSE>:-specs=nosys.specs>
)

target_include_directories(${LIBRARY} PUBLIC
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/LwIP/system>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/netif/ppp>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/lwip>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/lwip/apps>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/lwip/priv>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/lwip/prot>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/netif>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/compat>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/compat/posix>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/compat/posix/net>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/compat/posix/sys>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/compat/stdc>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/LwIP/system/arch>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/Components>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/Components/lan8742>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/LWIP/App>
  $<$<STREQUAL:${SIMULATE},FALSE>:${CMAKE_CURRENT_SOURCE_DIR}/LWIP/Target>
  ${CMAKE_CURRENT_SOURCE_DIR}/Inc
  ${CMAKE_CURRENT_SOURCE_DIR}/Inc/ST-LIB_LOW
  ${CMAKE_CURRENT_SOURCE_DIR}/Inc/ST-LIB_HIGH
)
