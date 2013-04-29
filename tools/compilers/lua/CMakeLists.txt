cmake_minimum_required(VERSION 2.8)

set (SRC
	LuaCompiler.cpp
	main.cpp
)

set (HEADERS
	LuaCompiler.h
)

add_executable(lua-compiler ${SRC} ${HEADERS})
target_link_libraries(lua-compiler crown-compiler-utils crown-luajit)

install (TARGETS lua-compiler DESTINATION bin)
