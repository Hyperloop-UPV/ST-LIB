if(DEFINED STLIB_NAME_SUFFIX)
    set(STLIB_TEST_EXECUTABLE st-lib-${STLIB_NAME_SUFFIX}-test)
else()
    set(STLIB_TEST_EXECUTABLE st-lib-test)
endif()

project( ${STLIB_TEST_EXECUTABLE})
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

message(STATUS "Generating test executable for ST-LIB")

add_executable(${STLIB_TEST_EXECUTABLE}
    ${CMAKE_CURRENT_LIST_DIR}/../Src/HALAL/Models/SPI/SPI2.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Src/HALAL/Models/DMA/DMA2.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Time/scheduler_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Time/timer_wrapper_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/adc_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spi2_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dma2_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Time/common_tests.cpp
)

set_target_properties(${STLIB_TEST_EXECUTABLE} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
)

target_link_libraries(
    ${STLIB_TEST_EXECUTABLE}
    GTest::gtest_main
    ${STLIB_LIBRARY}
)

if(MINGW OR CYGWIN)
    target_link_options(${STLIB_TEST_EXECUTABLE} PRIVATE -static)
endif()

if(MINGW OR CYGWIN)
    target_link_options(${STLIB_TEST_EXECUTABLE} PRIVATE -static)
endif()

target_compile_definitions(${STLIB_TEST_EXECUTABLE} PRIVATE
  SIM_ON
)

target_include_directories(${STLIB_TEST_EXECUTABLE} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../Inc
    ${CMAKE_CURRENT_LIST_DIR}/../Inc/ST-LIB_LOW
    ${CMAKE_CURRENT_LIST_DIR}/../Inc/ST-LIB_HIGH
)

include(GoogleTest)
gtest_discover_tests(
  ${STLIB_TEST_EXECUTABLE}
)

add_custom_command(
    TARGET ${STLIB_TEST_EXECUTABLE}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
      ${CMAKE_CURRENT_BINARY_DIR}/${STLIB_TEST_EXECUTABLE}
      ${CMAKE_SOURCE_DIR}/out/build/test.elf
)
