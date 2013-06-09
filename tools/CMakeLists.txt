cmake_minimum_required(VERSION 2.8)

project(crown-tools)

set (INCLUDES
	compilers
)

set (COMPILERS_SRC
	compilers/Compiler.cpp
	compilers/tga/TGACompiler.cpp
	compilers/txt/TXTCompiler.cpp
	compilers/ps/PSCompiler.cpp
	compilers/vs/VSCompiler.cpp
)

set (COMPILER_HEADERS
	compilers/Compiler.h
	compilers/tga/TGACompiler.h
	compilers/txt/TXTCompiler.h
	compilers/ps/PSCompiler.h
	compilers/vs/VSCompiler.h
)

set (TOOLS_SRC
	${COMPILERS_SRC}
)

set (TOOLS_HEADERS
	${COMPILERS_HEADERS}
)

include_directories(${INCLUDES})

# tools library
add_library(crown-tools ${TOOLS_SRC} ${TOOLS_HEADERS})

# resource-linker
add_executable(resource-linker cli/resource-linker.cpp)
target_link_libraries(resource-linker crown-tools)

# resource-hash
add_executable(resource-hash cli/resource-hash.cpp)
target_link_libraries(resource-hash crown-tools)

# resource-compiler
add_executable(resource-compiler cli/resource-compiler.cpp)
target_link_libraries(resource-compiler crown-tools)

install (TARGETS crown-tools DESTINATION lib/${CMAKE_PROJECT_NAME})

install (TARGETS resource-linker DESTINATION bin)
install (TARGETS resource-hash DESTINATION bin)
install (TARGETS resource-compiler DESTINATION bin)
install (FILES cli/resource-compiler.py PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
	GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE DESTINATION bin)

add_subdirectory(editors/resource-browser)
add_subdirectory(editors/toolchain)
add_subdirectory(pycrown)
