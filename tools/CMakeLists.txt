cmake_minimum_required(VERSION 2.8)

project(tools)

include(FindFreetype)

link_directories(${CROWN_BINARY_DIR})

if (FREETYPE_FOUND)
include_directories(${FREETYPE_INCLUDE_DIRS})
link_libraries(${FREETYPE_LIBRARIES})
endif (FREETYPE_FOUND)

add_executable(fontgen fontgen/fontgen.cpp)
add_executable(rescomp rescomp/rescomp.cpp)

target_link_libraries(fontgen crown)
target_link_libraries(rescomp crown)

