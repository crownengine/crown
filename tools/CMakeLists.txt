cmake_minimum_required(VERSION 2.8)

project(crown-tools)

set (INCLUDES
	${CMAKE_SOURCE_DIR}/tools
	${CMAKE_SOURCE_DIR}/tools/core
	${CMAKE_SOURCE_DIR}/tools/core/formats
	${CMAKE_SOURCE_DIR}/tools/core/strings
	${CMAKE_SOURCE_DIR}/tools/compilers
	${CMAKE_SOURCE_DIR}/tools/compilers/tga
	${CMAKE_SOURCE_DIR}/tools/compilers/wav
)

set (CORE_SRC
	core/Args.cpp
)

set (CORE_HEADERS
	core/Args.h
	core/Assert.h
	core/Types.h

	core/formats/PixelFormat.h
	core/formats/ResourceFormat.h
	core/formats/TextureFormat.h
	core/formats/SoundFormat.h

	core/strings/StringUtils.h
	core/strings/Hash.h
	core/strings/Path.h
)

set (COMPILERS_SRC
	compilers/Compiler.cpp
	compilers/tga/TGACompiler.cpp
	compilers/wav/WAVCompiler.cpp
#	compilers/ps/PSCompiler.cpp
#	compilers/vs/VSCompiler.cpp
)

set (COMPILER_HEADERS
	compilers/Compiler.h
	compilers/tga/TGACompiler.h
	compilers/wav/WAVCompiler.h
#	compilers/ps/PSCompiler.h
#	compilers/vs/VSCompiler.h
)

set (TOOLS_SRC
	${CORE_SRC}
	${COMPILERS_SRC}
)

set (TOOLS_HEADERS
	${CORE_HEADERS}
	${COMPILERS_HEADERS}
)

include_directories(${INCLUDES})

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/Config.h)

# tools library
add_library(crown-tools ${TOOLS_SRC} ${TOOLS_HEADERS})

# resource-hash
add_executable(resource-hash cli/resource-hash.cpp)
target_link_libraries(resource-hash crown-tools)

# resource-compiler
add_executable(resource-compiler cli/resource-compiler.cpp)
target_link_libraries(resource-compiler crown-tools)

# Install tools
install (TARGETS crown-tools DESTINATION tools)

install (TARGETS resource-hash DESTINATION tools)
install (TARGETS resource-compiler DESTINATION tools)
install (FILES cli/resource-compiler.py PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
	GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE DESTINATION tools)

add_subdirectory(gui/resource-browser)
add_subdirectory(gui/toolchain)
add_subdirectory(gui/console)
add_subdirectory(pycrown)
