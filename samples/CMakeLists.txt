cmake_minimum_required(VERSION 2.8)

project(samples)

link_directories(${CROWN_BINARY_DIR})

link_libraries(GLU pthread)
add_executable(terrain terrain/terrain.cpp)
add_executable(texturing texturing/texturing.cpp)
add_executable(trash trash/trash.cpp)
add_executable(fs filesystem/filesystem.cpp)
add_executable(textrenderer textrenderer/textrenderer.cpp)
add_executable(collision collision/collision.cpp)
add_executable(resource-manager resource-manager/resource-manager.cpp)

target_link_libraries(terrain crown)
target_link_libraries(texturing crown)
target_link_libraries(trash crown)
target_link_libraries(fs crown)
target_link_libraries(textrenderer crown)
target_link_libraries(collision crown)
target_link_libraries(resource-manager crown)
