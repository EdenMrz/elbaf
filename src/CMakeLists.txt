cmake_minimum_required(VERSION 3.22)
project(elbaf)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(elbaf main.cc elbaf.cc symbol.cc)
target_include_directories(elbaf PRIVATE ../include/)
