cmake_minimum_required(VERSION 2.8)

# txt-compiler
set (SRC
	TXTCompiler.cpp
	main.cpp
)

set (HEADERS
	TXTCompiler.h
)

add_executable(txt-compiler ${SRC} ${HEADERS})
target_link_libraries(txt-compiler crown-compiler-utils)

