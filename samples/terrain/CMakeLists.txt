cmake_minimum_required(VERSION 2.8)

set (SRC
	terrain_main.cpp
	Terrain.cpp
)

set (HEADERS
	Terrain.h
)

add_library (game-terrain SHARED ${SRC} ${HEADERS})
target_link_libraries(game-terrain crown)

set_target_properties (game-terrain PROPERTIES OUTPUT_NAME game)

install (DIRECTORY terrain DESTINATION samples)
install (TARGETS game-terrain DESTINATION samples/terrain)

