cmake_minimum_required(VERSION 2.8)

project(crown)

set (CROWN_VERSION_MAJOR 0)
set (CROWN_VERSION_MINOR 1)
set (CROWN_VERSION_MICRO 6)

option (CROWN_BUILD_SAMPLES "Whether to build the samples" ON)
option (CROWN_BUILD_TOOLS "Whether to build the tools" ON)
option (CROWN_BUILD_TESTS "Whether to build unit tests" OFF)

# always debug mode for now
set (CROWN_DEBUG 1)

# detect operating system
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set (LINUX 1)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(WINDOWS 1)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

# add build directories
add_subdirectory(src)

if (CROWN_BUILD_TOOLS)
	add_subdirectory(tools)
endif (CROWN_BUILD_TOOLS)
if (CROWN_BUILD_SAMPLES)
	add_subdirectory(samples)
endif (CROWN_BUILD_SAMPLES)

if (CROWN_BUILD_TESTS)
	add_subdirectory(tests)
endif (CROWN_BUILD_TESTS)


