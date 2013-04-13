cmake_minimum_required(VERSION 2.8)

project(tests)

link_directories(${CROWN_BINARY_DIR})

add_executable(allocators allocators.cpp)
add_executable(containers containers.cpp)
add_executable(messages messages.cpp)
add_executable(compressors compressors.cpp)
add_executable(connections connections.cpp)
add_executable(strings strings.cpp)
add_executable(paths paths.cpp)


target_link_libraries(allocators crown)
target_link_libraries(containers crown)
target_link_libraries(messages crown)
target_link_libraries(compressors crown)
target_link_libraries(connections crown)
target_link_libraries(strings crown)
target_link_libraries(paths crown)
