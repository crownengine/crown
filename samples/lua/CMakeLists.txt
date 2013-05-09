cmake_minimum_required(VERSION 2.8)

add_executable(luas lua.cpp)
target_link_libraries(luas crown)

