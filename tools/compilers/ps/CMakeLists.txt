cmake_minimum_required(VERSION 2.8)

# ps-compiler
set (SRC
	PSCompiler.cpp
	main.cpp
)

set (HEADERS
	PSCompiler.h
)

add_executable(ps-compiler ${SRC} ${HEADERS})
target_link_libraries(ps-compiler crown-compiler-utils)

install (TARGETS ps-compiler DESTINATION bin)
