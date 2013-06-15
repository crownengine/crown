cmake_minimum_required(VERSION 2.8)

set (SRC
	terrain_main.cpp
	Terrain.cpp
)

set (HEADERS
	Terrain.h
)

add_library (terrain STATIC ${SRC} ${HEADERS})
add_executable (game-terrain ${CROWN_MAIN_SRC})
target_link_libraries (terrain crown)
target_link_libraries (game-terrain terrain)
set_target_properties (game-terrain PROPERTIES OUTPUT_NAME game)

install (DIRECTORY terrain DESTINATION samples)
install (TARGETS game-terrain DESTINATION samples/terrain)
