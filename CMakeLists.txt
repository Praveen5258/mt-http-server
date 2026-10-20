cmake_minimum_required(VERSION 3.10)
project(MultiThreadedHTTPServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure console subsystem
if(MSVC)
    add_compile_options(/W4 /WX-)
endif()

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(server ${SOURCES})

# Link Winsock2 library for Windows networking
target_link_libraries(server ws2_32)
