cmake_minimum_required(VERSION 2.8)

set (SRC
	lua.cpp
)

add_library (game SHARED ${SRC})
target_link_libraries(game crown)

install (TARGETS game DESTINATION bin/lua_sample)

