cmake_minimum_required(VERSION 2.8)

project(crown-tests)

enable_testing()

add_executable(allocators allocators.cpp)
add_executable(containers containers.cpp)
add_executable(compressors compressors.cpp)
add_executable(strings strings.cpp)
add_executable(paths paths.cpp)

target_link_libraries(allocators crown)
target_link_libraries(containers crown)
target_link_libraries(compressors crown)
target_link_libraries(strings crown)
target_link_libraries(paths crown)

add_test(allocators-test ${EXECUTABLE_OUTPUT_PATH}/allocators)
add_test(containers-test ${EXECUTABLE_OUTPUT_PATH}/containers)
add_test(compressors-test ${EXECUTABLE_OUTPUT_PATH}/compressors)
add_test(strings-test ${EXECUTABLE_OUTPUT_PATH}/strings)
add_test(paths-test ${EXECUTABLE_OUTPUT_PATH}/paths)

