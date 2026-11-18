cmake_minimum_required(VERSION 3.16)
project(kline_quant_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(kline_quant cli.cpp)
add_executable(kline_quant_tests test.cpp)
add_executable(kline_quant_advanced_tests test_advanced.cpp)

if(MSVC)
  target_compile_options(kline_quant PRIVATE /W4)
  target_compile_options(kline_quant_tests PRIVATE /W4)
  target_compile_options(kline_quant_advanced_tests PRIVATE /W4)
else()
  target_compile_options(kline_quant PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(kline_quant_tests PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(kline_quant_advanced_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_test(NAME kline_quant_numeric_tests COMMAND kline_quant_tests)
add_test(NAME kline_quant_advanced_numeric_tests COMMAND kline_quant_advanced_tests)
