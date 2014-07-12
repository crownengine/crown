cmake_minimum_required(VERSION 2.8)

project(crown)

set (CROWN_VERSION_MAJOR 0)
set (CROWN_VERSION_MINOR 1)
set (CROWN_VERSION_MICRO 13)

option (CROWN_BUILD_SAMPLES "Whether to build the samples" ON)
option (CROWN_BUILD_TOOLS "Whether to build the tools" ON)
option (CROWN_BUILD_TESTS "Whether to build unit tests" ON)
#option (CROWN_BUILD_DOC "Whether to build docs" OFF)

# build configuration variables
if (CROWN_PLATFORM MATCHES "linux32")
	set (LINUX 1)
	set (CROWN_ARCH "x86")
elseif (CROWN_PLATFORM MATCHES "linux64")
	set (LINUX 1)
	set (CROWN_ARCH "x86_64")
elseif (CROWN_PLATFORM MATCHES "win32")
	set (WINDOWS 1)
	set (CROWN_ARCH "win32")
elseif (CROWN_PLATFORM MATCHES "win64")
	set (WINDOWS 1)
	set (CROWN_ARCH "win64")
else ()
	message (FATAL_ERROR "Wrong build configuration")
endif (CROWN_PLATFORM MATCHES "linux32")

# executable name
set (CROWN_EXECUTABLE_DEBUG crown-debug-${CROWN_PLATFORM})
set (CROWN_EXECUTABLE_DEVELOPMENT crown-development-${CROWN_PLATFORM})
set (CROWN_EXECUTABLE_RELEASE crown-release-${CROWN_PLATFORM})

set (CROWN_LIBRARY_DEBUG crown-lib-debug-${CROWN_PLATFORM})
set (CROWN_LIBRARY_DEVELOPMENT crown-lib-development-${CROWN_PLATFORM})
set (CROWN_LIBRARY_RELEASE crown-lib-release-${CROWN_PLATFORM})


# architecture dependent includes
set (CROWN_THIRD_INCLUDES
	${CMAKE_SOURCE_DIR}/third/luajit/${CROWN_ARCH}/include/luajit-2.0
	${CMAKE_SOURCE_DIR}/third/opengl
	${CMAKE_SOURCE_DIR}/third/openal/include
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include/common
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include/characterkinematic
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include/cloth
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include/common
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include/cooking
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include/extensions
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include/foundation
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include/geometry
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include/particles
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include/physxprofilesdk
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include/physxvisualdebuggersdk
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include/pvd
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include/pxtask
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include/RepX
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include/RepXUpgrader
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/include/vehicle
	${CMAKE_SOURCE_DIR}/third/freetype
	${CMAKE_SOURCE_DIR}/third/stb_image
	${CMAKE_SOURCE_DIR}/third/stb_vorbis
)

# architecture dependent libraries
set (CROWN_THIRD_LIBS
	${CMAKE_SOURCE_DIR}/third/luajit/${CROWN_ARCH}/lib
	${CMAKE_SOURCE_DIR}/third/openal/lib
	${CMAKE_SOURCE_DIR}/third/physx/${CROWN_ARCH}/lib
)

add_subdirectory(third)

# always build the engine
add_subdirectory(engine)

# select optional projects to build
if (CROWN_BUILD_TOOLS)
	add_subdirectory(tools)
endif (CROWN_BUILD_TOOLS)

if (CROWN_BUILD_SAMPLES)
	add_subdirectory(samples)
endif (CROWN_BUILD_SAMPLES)

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
	add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
endif(DOXYGEN_FOUND)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/documentation/html DESTINATION documentation)
