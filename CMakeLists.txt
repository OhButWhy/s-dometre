cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(soup VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(program tomyam.cpp)
target_link_libraries(program pthread)

add_executable(server server.cpp)
target_link_libraries(server pthread)

add_executable(test_server server_check.cpp)
target_link_libraries(test_server pthread)