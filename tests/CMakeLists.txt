cmake_minimum_required(VERSION 2.8)

project(tests)

link_directories(${CROWN_BINARY_DIR})

#link_libraries()
add_executable(allocators allocators.cpp)
add_executable(containers containers.cpp)
add_executable(messages messages.cpp)


target_link_libraries(allocators crown)
target_link_libraries(containers crown)
target_link_libraries(messages crown)
