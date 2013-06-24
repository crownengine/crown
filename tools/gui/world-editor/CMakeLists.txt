cmake_minimum_required(VERSION 2.8)

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0)

link_directories(${GTKMM_LIBRARY_DIRS} ${CROWN_BINARY_DIR})

set (INCLUDES
	terrain
)

set (SRC
	world-editor.cpp
	CrownDrawingArea.cpp
)

set (HEADERS
	CrownDrawingArea.h
)

set (TERRAIN_SRC
	terrain/Heightfield.cpp
)

set (TERRAIN_HEADERS
	terrain/Heightfield.h
)

set (WORLD_EDITOR_SRC
	${SRC}

	${TERRAIN_SRC}
)

set (WORLD_EDITOR_HEADERS
	${HEADERS}

	${TERRAIN_HEADERS}
)

include_directories(${GTKMM_INCLUDE_DIRS} ${INCLUDES})

add_executable(world-editor ${WORLD_EDITOR_SRC} ${WORLD_EDITOR_HEADERS})

target_link_libraries(world-editor ${GTKMM_LIBRARIES} X11 GL crown)

install (TARGETS world-editor DESTINATION bin/world-editor)
install (FILES ui/world-editor.glade DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/world-editor/ui)
