cmake_minimum_required(VERSION 2.8)

project(crown-tools)

include(FindFreetype)

if (FREETYPE_FOUND)
	include_directories(${FREETYPE_INCLUDE_DIRS})
	link_libraries(${FREETYPE_LIBRARIES})
endif (FREETYPE_FOUND)

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0)

set (INCLUDES
	compilers
)

link_directories(${CROWN_BINARY_DIR} ${GTKMM_LIBRARY_DIRS})
include_directories(${INCLUDES} ${GTKMM_INCLUDE_DIRS})

add_subdirectory(compilers)
add_subdirectory(editors/world-editor)
add_subdirectory(pycrown)

install (FILES ${PROJECT_SOURCE_DIR}/compilers/resource-compiler.py DESTINATION bin)
install (FILES ${PROJECT_SOURCE_DIR}/editors/resource-browser/resource-browser.py DESTINATION bin)

