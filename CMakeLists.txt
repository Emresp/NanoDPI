cmake_minimum_required(VERSION 4.0)
project(NanoDPI)

set(CMAKE_CXX_STANDARD 26)

add_executable(NanoDPI src/main.cpp
        include/ProxyServer.h
        src/ProxyServer.cpp)

target_link_libraries(NanoDPI ws2_32 wininet)