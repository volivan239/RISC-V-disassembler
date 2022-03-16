cmake_minimum_required(VERSION 3.11)

include_directories("include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -O2")
add_executable(disasm src/main.cpp src/elf32_parser.cpp src/registers.cpp src/cmd_parser.cpp)
