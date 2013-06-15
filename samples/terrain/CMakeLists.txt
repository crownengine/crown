cmake_minimum_required(VERSION 2.8)

set (SRC
	terrain_main.cpp
	Terrain.cpp
)

set (HEADERS
	Terrain.h
)

add_executable(${CROWN_EXECUTABLE_NAME} ${CROWN_MAIN_SRC} ${SRC} ${HEADERS})
target_link_libraries(${CROWN_EXECUTABLE_NAME} crown)

install (DIRECTORY terrain DESTINATION samples)
install (TARGETS ${CROWN_EXECUTABLE_NAME} DESTINATION samples/terrain)
