cmake_minimum_required(VERSION 2.8)

project(crown-tools)

include(FindFreetype)

link_directories(${CROWN_BINARY_DIR})

if (FREETYPE_FOUND)
include_directories(${FREETYPE_INCLUDE_DIRS})
link_libraries(${FREETYPE_LIBRARIES})
endif (FREETYPE_FOUND)

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0)

link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})

add_executable(resource-hash resource-compilers/resource-hash.cpp)

add_executable(txt-compiler resource-compilers/txt-compiler.cpp)
add_executable(tga-compiler resource-compilers/tga-compiler.cpp)
add_executable(lua-compiler resource-compilers/lua-compiler.cpp)

add_executable(resource-linker resource-compilers/resource-linker.cpp)

target_link_libraries(resource-hash crown)

target_link_libraries(txt-compiler crown)
target_link_libraries(tga-compiler crown)
target_link_libraries(lua-compiler crown)

target_link_libraries(resource-linker crown)

add_subdirectory(gtk)

