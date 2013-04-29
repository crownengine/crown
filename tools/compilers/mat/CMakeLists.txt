cmake_minimum_required(VERSION 2.8)

# mat-compiler
set (SRC
	MATCompiler.cpp
	main.cpp
)

set (HEADERS
	MATCompiler.h
)

add_executable(mat-compiler ${SRC} ${HEADERS})
target_link_libraries(mat-compiler crown-compiler-utils)

install (TARGETS mat-compiler DESTINATION bin)
