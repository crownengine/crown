cmake_minimum_required(VERSION 2.8)

project(crown-samples)

link_directories(${CROWN_BINARY_DIR})

link_libraries(GLU pthread)
add_executable(terrain terrain/terrain.cpp)
#add_executable(texturing texturing/texturing.cpp)
#add_executable(textrenderer textrenderer/textrenderer.cpp)
#add_executable(collision collision/collision.cpp)
add_executable(json json/json.cpp)

target_link_libraries(terrain crown)
#target_link_libraries(texturing crown)
#target_link_libraries(textrenderer crown)
#target_link_libraries(collision crown)
target_link_libraries(json crown)

#install (TARGETS terrain DESTINATION bin)
#install (TARGETS texturing DESTINATION bin)
#install (TARGETS textrenderer DESTINATION bin)
#install (TARGETS collision DESTINATION bin)
#install (TARGETS json DESTINATION bin)
