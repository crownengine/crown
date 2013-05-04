cmake_minimum_required(VERSION 2.8)

set (SRC
	LuaCompiler.cpp
	main.cpp
)

set (HEADERS
	LuaCompiler.h
)

add_executable(lua-compiler ${SRC} ${HEADERS})
target_link_libraries(lua-compiler crown-compiler-utils)

install (TARGETS lua-compiler DESTINATION bin)
install (FILES bytecode-generator.lua DESTINATION bin)
