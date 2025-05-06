cmake_minimum_required(VERSION 3.22)
project(elbaf)
add_executable(elbaf elbaf.cc)
target_include_directories(elbaf PRIVATE ../include/)
