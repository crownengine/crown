cmake_minimum_required(VERSION 2.8)

project(crown)

set (CROWN_VERSION_MAJOR 0)
set (CROWN_VERSION_MINOR 1)
set (CROWN_VERSION_MICRO 0)

option (CROWN_USE_FLOAT "Whether to use float or double for representing real numbers." ON)
option (CROWN_BUILD_OPENGL "Whether to build the OpenGL renderer or not." ON)
option (CROWN_BUILD_OPENGLES "Whether to build the OpenGL|ES 1.0 renderer or not." OFF)
option (CROWN_BUILD_SAMPLES "Whether to build the samples" ON)
option (CROWN_BUILD_TOOLS "Whether to build the tools" ON)
option (CROWN_BUILD_TESTS "Whether to build unit tests" ON)

set (INCLUDES
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/src/core
	${CMAKE_SOURCE_DIR}/src/core/bv
	${CMAKE_SOURCE_DIR}/src/core/containers
	${CMAKE_SOURCE_DIR}/src/core/math
	${CMAKE_SOURCE_DIR}/src/core/mem
	${CMAKE_SOURCE_DIR}/src/core/compressors
	${CMAKE_SOURCE_DIR}/src/core/streams
	${CMAKE_SOURCE_DIR}/src/core/strings
	${CMAKE_SOURCE_DIR}/src/core/threads
	${CMAKE_SOURCE_DIR}/src/core/settings
	${CMAKE_SOURCE_DIR}/src/os
	${CMAKE_SOURCE_DIR}/src/os/linux
	${CMAKE_SOURCE_DIR}/src/input
	${CMAKE_SOURCE_DIR}/src/renderers
	${CMAKE_SOURCE_DIR}/src/network

	${CMAKE_SOURCE_DIR}/game
)

include_directories(${INCLUDES})
add_definitions(-pipe -ansi -W -Wall -Wextra -Wno-long-long -pedantic -fPIC -g -pg)

# detect operating system
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(LINUX 1)
	add_subdirectory(src/os/linux)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set (WINDOWS 1)
	add_subdirectory(src/os/win)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

#detect architecture
if (CROWN_ARCH MATCHES "x86")
	set (CROWN_THIRD third/x86)
endif (CROWN_ARCH MATCHES "x86")

if (CROWN_ARCH MATCHES "x86_64")
	set (CROWN_THIRD third/x86_64)
endif (CROWN_ARCH MATCHES "x86_64")

add_subdirectory(${CROWN_THIRD})

# add build directories
add_subdirectory(src)

if (CROWN_BUILD_OPENGL)
	add_subdirectory(src/renderers/gl)
endif (CROWN_BUILD_OPENGL)

if (CROWN_BUILD_OPENGLES)
	add_subdirectory(src/renderers/gles)
endif (CROWN_BUILD_OPENGLES)

if (CROWN_BUILD_TOOLS)
	add_subdirectory(tools)
endif (CROWN_BUILD_TOOLS)

if (CROWN_BUILD_SAMPLES)
	add_subdirectory(samples)
endif (CROWN_BUILD_SAMPLES)

if (CROWN_BUILD_TESTS)
	add_subdirectory(tests)
endif (CROWN_BUILD_TESTS)

