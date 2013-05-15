cmake_minimum_required(VERSION 2.8)

project(crown-samples)

link_directories(${CROWN_BINARY_DIR})

add_subdirectory(terrain)
add_subdirectory(json)
add_subdirectory(lua)
