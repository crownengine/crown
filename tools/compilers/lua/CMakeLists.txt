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
include_directories(${CROWN_THIRD}/luajit/include)
target_link_libraries(lua-compiler crown-compiler-utils lua)

install (TARGETS lua-compiler DESTINATION bin)
