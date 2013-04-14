cmake_minimum_required(VERSION 2.8)

project(tools)

include(FindFreetype)

link_directories(${CROWN_BINARY_DIR})

if (FREETYPE_FOUND)
include_directories(${FREETYPE_INCLUDE_DIRS})
link_libraries(${FREETYPE_LIBRARIES})
endif (FREETYPE_FOUND)

add_executable(fontgen fontgen/fontgen.cpp)

add_executable(txt-compiler resource-compilers/txt-compiler.cpp)
add_executable(tga-compiler resource-compilers/tga-compiler.cpp)
add_executable(script-compiler resource-compilers/script-compiler.cpp)

add_executable(resource-linker resource-compilers/resource-linker.cpp)

target_link_libraries(fontgen crown)

target_link_libraries(txt-compiler crown)
target_link_libraries(tga-compiler crown)
target_link_libraries(script-compiler crown)

target_link_libraries(resource-linker crown)

