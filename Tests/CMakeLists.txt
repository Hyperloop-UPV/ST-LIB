set(STLIB_TEST_EXECUTABLE stlib-test)

message(STATUS "Generating test executable for ST-LIB")
include(CTest)
enable_testing()
add_executable(${STLIB_TEST_EXECUTABLE}
    ${CMAKE_CURRENT_LIST_DIR}/Runes/Pins.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Runes/SimRunes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DigitalOutTest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DigitalInTest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Time/test_time.cpp
    ${CMAKE_CURRENT_LIST_DIR}/EncoderTest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/InputCaptureTest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/EXTITest.cpp
)

set_target_properties(${STLIB_TEST_EXECUTABLE} PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED C_STANDARD 11 C_STANDARD_REQUIRED)

target_link_libraries(
    ${STLIB_TEST_EXECUTABLE}
    ${STLIB_LIBRARY}
    GTest::gtest_main
)

target_compile_definitions(${STLIB_TEST_EXECUTABLE} PRIVATE
  SIM_ON
)

target_include_directories(${STLIB_TEST_EXECUTABLE} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../Inc
    ${CMAKE_CURRENT_LIST_DIR}/../Inc/ST-LIB_LOW
    ${CMAKE_CURRENT_LIST_DIR}/../Inc/ST-LIB_HIGH
)

include(GoogleTest)
gtest_discover_tests(${STLIB_TEST_EXECUTABLE})