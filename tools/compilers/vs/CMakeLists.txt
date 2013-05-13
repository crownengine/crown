cmake_minimum_required(VERSION 2.8)

# vs-compiler
set (SRC
	VSCompiler.cpp
	main.cpp
)

set (HEADERS
	VSCompiler.h
)

add_executable(vs-compiler ${SRC} ${HEADERS})
target_link_libraries(vs-compiler crown-compiler-utils)

install (TARGETS vs-compiler DESTINATION bin)
