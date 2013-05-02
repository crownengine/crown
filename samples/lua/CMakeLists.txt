cmake_minimum_required(VERSION 2.8)

include_directories(${CROWN_THIRD}/luajit/include/luajit-2.0)
link_directories(${CROWN_THIRD}/luajit/lib)
add_executable(luas lua.cpp)
target_link_libraries(luas crown luajit-5.1)

