cmake_minimum_required(VERSION 2.8)

set (SRC
	terrain.cpp
	Terrain.cpp
)

set (HEADERS
	Terrain.h
)

add_library (game SHARED ${SRC} ${HEADERS})
target_link_libraries(game crown)

install (DIRECTORY terrain DESTINATION samples)
install (TARGETS game DESTINATION samples/terrain)
