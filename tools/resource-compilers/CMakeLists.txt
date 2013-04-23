cmake_minimum_required(VERSION 2.8)

set (SRC
	Compiler.cpp
)

set (HEADERS
	Compiler.h
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

add_subdirectory(txt)
add_subdirectory(tga)
#add_subdirectory(lua)
