cmake_minimum_required(VERSION 3.22)
project(elbaf)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src ${CMAKE_BINARY_DIR}/bin)
