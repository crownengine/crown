cmake_minimum_required(VERSION 2.8)

set (SRC
	Compiler.cpp

	tga/TGACompiler.cpp
	txt/TXTCompiler.cpp
	lua/LuaCompiler.cpp
	ps/PSCompiler.cpp
	vs/VSCompiler.cpp
)

set (HEADERS
	Compiler.h

	tga/TGACompiler.h
	txt/TXTCompiler.h
	lua/LuaCompiler.h
	ps/PSCompiler.h
	vs/VSCompiler.h
)

# utils
add_library(crown-compiler-utils ${SRC} ${HEADERS})
target_link_libraries(crown-compiler-utils crown)

# resource-linker
add_executable(resource-linker resource-linker.cpp)
target_link_libraries(resource-linker crown)

# resource-hash
add_executable(resource-hash resource-hash.cpp)
target_link_libraries(resource-hash crown)

# resource-compiler
add_executable(resource-compiler resource-compiler.cpp)
target_link_libraries(resource-compiler crown crown-compiler-utils)

install (TARGETS crown-compiler-utils DESTINATION lib/${CMAKE_PROJECT_NAME})

install (TARGETS resource-linker DESTINATION bin)
install (TARGETS resource-hash DESTINATION bin)
install (TARGETS resource-compiler DESTINATION bin)
install (FILES resource-compiler.py DESTINATION bin)
