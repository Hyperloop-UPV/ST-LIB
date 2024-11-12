cmake_minimum_required (VERSION 3.14)

project (ST-LIB C CXX)
set(LIBRARY st-lib)
set(STLIB_DIR "$ENV{STLIB_PATH}")

option(USE_ETHERNET "Enable ethernet peripheral" OFF)
option(TARGET_NUCLEO "Targets the STM32H723 Nucleo development board" OFF)

if (PROJECT_IS_TOP_LEVEL)
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

# This should be an explicit list of files, but there are just too many files :(
file(GLOB_RECURSE HEADER_DRIVER ${STLIB_DIR}/Drivers/*.h )
file(GLOB_RECURSE SOURCE_DRIVER ${STLIB_DIR}/Drivers/*.c )
file(GLOB_RECURSE HEADER_MIDDLEWARE ${STLIB_DIR}/Middlewares/*.h)
file(GLOB_RECURSE SOURCE_MIDDLEWARE ${STLIB_DIR}/Middlewares/*.c)
file(GLOB_RECURSE HEADER_LWIP ${STLIB_DIR}/LWIP/*.h)
file(GLOB_RECURSE SOURCE_LWIP ${STLIB_DIR}/LWIP/*.c)

# CPP UTILITIES
file(GLOB_RECURSE CPP_UTILITIES_H ${STLIB_DIR}/Inc/C++Utilities/*.h)
file(GLOB_RECURSE CPP_UTILITIES_HPP ${STLIB_DIR}/Inc/C++Utilities/*.hpp)
file(GLOB_RECURSE CPP_UTILITIES_C ${STLIB_DIR}/Src/C++Utilities/*.c)
file(GLOB_RECURSE CPP_UTILITIES_CPP ${STLIB_DIR}/Src/C++Utilities/*.cpp)

# HALAL (and Mock)
file(GLOB_RECURSE HALAL_H ${STLIB_DIR}/Inc/HALAL/*.h)
file(GLOB_RECURSE HALAL_MOCK_H ${STLIB_DIR}/Inc/HALALMock/*.h)
list(REMOVE_ITEM HALAL_H HALAL_MOCK_H)
file(GLOB_RECURSE HALAL_HPP ${STLIB_DIR}/Inc/HALAL/*.hpp)
file(GLOB_RECURSE HALAL_MOCK_HPP ${STLIB_DIR}/Inc/HALALMock/*.hpp)
list(REMOVE_ITEM HALAL_HPP HALAL_MOCK_HPP)
file(GLOB_RECURSE HALAL_C ${STLIB_DIR}/Src/HALAL/*.c)
file(GLOB_RECURSE HALAL_MOCK_C ${STLIB_DIR}/Src/HALALMock/*.c)
list(REMOVE_ITEM HALAL_C HALAL_MOCK_C)
file(GLOB_RECURSE HALAL_CPP ${STLIB_DIR}/Src/HALAL/*.cpp)
file(GLOB_RECURSE HALAL_MOCK_CPP ${STLIB_DIR}/Src/HALALMock/*.cpp)
list(REMOVE_ITEM HALAL_CPP HALAL_MOCK_CPP)

# STLIB_LOW
file(GLOB_RECURSE STLIB_LOW_H ${STLIB_DIR}/Inc/ST-LIB_LOW/*.h)
file(GLOB_RECURSE STLIB_LOW_HPP ${STLIB_DIR}/Inc/ST-LIB_LOW/*.hpp)
file(GLOB_RECURSE STLIB_LOW_C ${STLIB_DIR}/Src/ST-LIB_LOW/*.c)
file(GLOB_RECURSE STLIB_LOW_CPP ${STLIB_DIR}/Src/ST-LIB_LOW/*.cpp)

# STLIB_HIGH
file(GLOB_RECURSE STLIB_HIGH_H ${STLIB_DIR}/Inc/ST-LIB_HIGH/*.h)
file(GLOB_RECURSE STLIB_HIGH_HPP ${STLIB_DIR}/Inc/ST-LIB_HIGH/*.hpp)
file(GLOB_RECURSE STLIB_HIGH_C ${STLIB_DIR}/Src/ST-LIB_HIGH/*.c)
file(GLOB_RECURSE STLIB_HIGH_CPP ${STLIB_DIR}/Src/ST-LIB_HIGH/*.cpp)

add_library(${LIBRARY} STATIC
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HEADER_DRIVER}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${SOURCE_DRIVER}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HEADER_MIDDLEWARE}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${SOURCE_MIDDLEWARE}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HEADER_LWIP}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${SOURCE_LWIP}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HALAL_H}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HALAL_HPP}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HALAL_C}>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${HALAL_CPP}>
  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:${HALAL_MOCK_H}>
  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:${HALAL_MOCK_HPP}>
  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:${HALAL_MOCK_C}>
  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:${HALAL_MOCK_CPP}>
  ${CPP_UTILITIES_H}
  ${CPP_UTILITIES_HPP}
  ${CPP_UTILITIES_C}
  ${CPP_UTILITIES_CPP}
  ${STLIB_LOW_H}
  ${STLIB_LOW_HPP}
  ${STLIB_LOW_C}
  ${STLIB_LOW_CPP}
  ${STLIB_HIGH_H}
  ${STLIB_HIGH_HPP}
  ${STLIB_HIGH_C}
  ${STLIB_HIGH_CPP}
  ${STLIB_DIR}/Src/ST-LIB.cpp
)

target_link_libraries(${LIBRARY} PUBLIC 
  GTest::gtest_main
)

set_target_properties(${LIBRARY} PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED C_STANDARD 11 C_STANDARD_REQUIRED)

target_compile_definitions(${LIBRARY} PRIVATE
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:USE_HAL_DRIVER>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:STM32H723xx>

  $<$<NOT:$<BOOL:${CMAKE_CROSSCOMPILING}>>:SIM_ON>

  $<$<BOOL:${USING_ETHERNET}>:STLIB_ETH>
  $<IF:$<BOOL:${TARGET_NUCLEO}>,NUCLEO,BOARD>
  $<IF:$<BOOL:${TARGET_NUCLEO}>,HSE_VALUE=8000000,HSE_VALUE=25000000>
)

target_compile_options(${LIBRARY} PRIVATE
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-mcpu=cortex-m7>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-mfpu=fpv5-d16>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-mfloat-abi=hard>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-mthumb>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-specs=nosys.specs>
  
  -ffunction-sections
  -fdata-sections
  -fno-exceptions
  
  -Wno-psabi
  
  $<$<COMPILE_LANGUAGE:C>:-w>
  
  $<$<COMPILE_LANGUAGE:CXX>:-Wall>
  $<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>
  $<$<COMPILE_LANGUAGE:CXX>:-Werror>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

target_include_directories(${LIBRARY} PRIVATE
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Drivers/CMSIS/Include>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Middlewares/Third_Party/LwIP/src/include>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Middlewares/Third_Party/LwIP/system>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Middlewares/Third_Party/LwIP/src/netif/ppp>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Middlewares/Third_Party/LwIP/src/include/lwip>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Middlewares/Third_Party/LwIP/src/include/lwip/apps>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Middlewares/Third_Party/LwIP/src/include/lwip/priv>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Middlewares/Third_Party/LwIP/src/include/lwip/prot>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Middlewares/Third_Party/LwIP/src/include/netif>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Middlewares/Third_Party/LwIP/src/include/compat>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Middlewares/Third_Party/LwIP/src/include/compat/posix>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Middlewares/Third_Party/LwIP/src/include/compat/posix/net>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Middlewares/Third_Party/LwIP/src/include/compat/posix/sys>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Middlewares/Third_Party/LwIP/src/include/compat/stdc>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Middlewares/Third_Party/LwIP/system/arch>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Drivers/BSP/Components>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/Drivers/BSP/Components/lan8742>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/LWIP/App>
  $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/LWIP/Target>
  ${STLIB_DIR}/Inc
  ${STLIB_DIR}/Inc/ST-LIB_LOW
  ${STLIB_DIR}/Inc/ST-LIB_HIGH
)

if (PROJECT_IS_TOP_LEVEL)
    # Create symlink to compile_commands.json for IDE to pick it up
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    )
endif()

include(GoogleTest)
gtest_discover_tests(${LIBRARY})
