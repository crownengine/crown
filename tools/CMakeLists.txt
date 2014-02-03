cmake_minimum_required(VERSION 2.8)

project(crown-tools)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/Config.h)

#add_subdirectory(gui/resource-browser)
#add_subdirectory(gui/toolchain)
#add_subdirectory(gui/console)
#add_subdirectory(pycrown)
add_subdirectory(gui/fontgen)