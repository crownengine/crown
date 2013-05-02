cmake_minimum_required(VERSION 2.8)

set (SRC
	LuaCompiler.cpp
	main.cpp
)

set (HEADERS
	LuaCompiler.h
)

add_executable(lua-compiler ${SRC} ${HEADERS})
link_directories(${CROWN_THIRD}/luajit/lib)
include_directories(${CROWN_THIRD}/luajit/include/luajit-2.0)
target_link_libraries(lua-compiler crown-compiler-utils luajit-5.1)

install (TARGETS lua-compiler DESTINATION bin)
