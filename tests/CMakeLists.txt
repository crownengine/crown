cmake_minimum_required(VERSION 2.8)

project(crown-tests)

link_directories(${CROWN_BINARY_DIR})

enable_testing()

add_executable(allocators allocators.cpp)
add_executable(containers containers.cpp)
add_executable(messages messages.cpp)
add_executable(compressors compressors.cpp)
add_executable(connections connections.cpp)
add_executable(strings strings.cpp)
add_executable(paths paths.cpp)
add_executable(script script.cpp)

target_link_libraries(allocators crown)
target_link_libraries(containers crown)
target_link_libraries(messages crown)
target_link_libraries(compressors crown)
target_link_libraries(connections crown)
target_link_libraries(strings crown)
target_link_libraries(paths crown)

link_directories(${CROWN_THIRD}/luajit/lib)
include_directories(${CROWN_THIRD}/luajit/include/luajit-2.0)
target_link_libraries(script crown luajit-5.1)

add_test(allocators-test ${EXECUTABLE_OUTPUT_PATH}/allocators)
add_test(containers-test containers)
add_test(messages-test messages)
add_test(compressors-test compressors)
add_test(connections-test connections)
add_test(strings-test strings)
add_test(paths-test paths)

