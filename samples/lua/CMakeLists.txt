cmake_minimum_required(VERSION 2.8)

set (SRC
	lua.cpp
)

add_definitions(-Wl,-E)
add_library (game-lua SHARED ${SRC})
target_link_libraries(game-lua crown crownlua)

set_target_properties (game-lua PROPERTIES OUTPUT_NAME game)

install (TARGETS game-lua DESTINATION samples/lua)

