cmake_minimum_required(VERSION 2.8)

set (SRC
	TGACompiler.cpp
	main.cpp
)

set (HEADERS
	TGACompiler.h
)

add_executable(tga-compiler ${SRC} ${HEADERS})
target_link_libraries(tga-compiler crown-compiler-utils)

install (TARGETS tga-compiler DESTINATION bin)
