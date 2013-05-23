cmake_minimum_required(VERSION 2.8)

project(crown-tools)

set (INCLUDES
	compilers
)

link_directories(${CROWN_BINARY_DIR} ${GTKMM_LIBRARY_DIRS})
include_directories(${INCLUDES} ${GTKMM_INCLUDE_DIRS})

add_subdirectory(compilers)
#add_subdirectory(editors/world-editor)
add_subdirectory(editors/resource-browser)
add_subdirectory(editors/toolchain)
add_subdirectory(pycrown)

