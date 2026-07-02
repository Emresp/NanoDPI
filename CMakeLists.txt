cmake_minimum_required(VERSION 4.0)
project(NanoDPI)

set(CMAKE_CXX_STANDARD 26)

add_executable(NanoDPI src/main.cpp
        include/ProxyServer.h
        src/ProxyServer.cpp
        include/Config.h
        include/Logger.h
        src/Logger.cpp
)

target_link_libraries(NanoDPI ws2_32 wininet)