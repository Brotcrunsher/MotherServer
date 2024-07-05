cmake_minimum_required(VERSION 3.11)

if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

project("MotherServer" CXX)

add_executable("MotherServer")

target_sources(MotherServer PRIVATE server.cpp)

add_subdirectory(libsodium-cmake)
target_link_libraries(MotherServer sodium)
include_directories(libsodium-cmake/libsodium/src/libsodium/include)
