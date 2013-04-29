cmake_minimum_required(VERSION 2.8)

set (SRC
	terrain.cpp
)

set (HEADERS
)

add_library (game SHARED ${SRC} ${HEADERS})
target_link_libraries(game crown)

install (TARGETS game DESTINATION bin/terrain)
