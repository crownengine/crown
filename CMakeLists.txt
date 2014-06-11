cmake_minimum_required(VERSION 2.8)

project(crown)

set (CROWN_VERSION_MAJOR 0)
set (CROWN_VERSION_MINOR 1)
set (CROWN_VERSION_MICRO 13)

option (CROWN_BUILD_SAMPLES "Whether to build the samples" ON)
option (CROWN_BUILD_TOOLS "Whether to build the tools" ON)
option (CROWN_BUILD_TESTS "Whether to build unit tests" ON)
option (CROWN_BUILD_DOC "Whether to build docs" ON)

# build configuration variables
if (CROWN_BUILD MATCHES "linux-debug-32")
	set (LINUX 1)
	set (CROWN_DEBUG 1)
	set (CROWN_ARCH "x86")
elseif (CROWN_BUILD MATCHES "linux-development-32")
	set (LINUX 1)
	set (CROWN_DEVELOPMENT 1)
	set (CROWN_ARCH "x86")
elseif (CROWN_BUILD MATCHES "linux-release-32")
	set (LINUX 1)
	set (CROWN_RELEASE 1)
	set (CROWN_ARCH "x86")
elseif (CROWN_BUILD MATCHES "linux-debug-64")
	set (LINUX 1)
	set (CROWN_DEBUG 1)
	set (CROWN_ARCH "x86_64")
elseif (CROWN_BUILD MATCHES "linux-development-64")
	set (LINUX 1)
	set (CROWN_DEVELOPMENT 1)
	set (CROWN_ARCH "x86_64")
elseif (CROWN_BUILD MATCHES "linux-release-64")
	set (LINUX 1)
	set (CROWN_RELEASE 1)
	set (CROWN_ARCH "x86_64")
elseif (CROWN_BUILD MATCHES "windows-debug-32")
	set (WINDOWS 1)
	set (CROWN_DEBUG 1)
	set (CROWN_ARCH "win32")
elseif (CROWN_BUILD MATCHES "windows-development-32")
	set (WINDOWS 1)
	set (CROWN_DEVELOPMENT 1)
	set (CROWN_ARCH "win32")
elseif (CROWN_BUILD MATCHES "windows-release-32")
	set (WINDOWS 1)
	set (CROWN_RELEASE 1)
	set (CROWN_ARCH "win32")
elseif (CROWN_BUILD MATCHES "windows-debug-64")
	set (WINDOWS 1)
	set (CROWN_DEBUG 1)
	set (CROWN_ARCH "win64")
elseif (CROWN_BUILD MATCHES "windows-development-64")
	set (WINDOWS 1)
	set (CROWN_DEVELOPMENT 1)
	set (CROWN_ARCH "win64")
elseif (CROWN_BUILD MATCHES "windows-release-64")
	set (WINDOWS 1)
	set (CROWN_RELEASE 1)
	set (CROWN_ARCH "win64")
else ()
	message (FATAL_ERROR "Wrong build configuration")
endif (CROWN_BUILD MATCHES "linux-debug-32")

# executable name
set (CROWN_EXECUTABLE_NAME crown-${CROWN_BUILD})
set (CROWN_LIBRARY_NAME crown-lib-${CROWN_BUILD})

# detect architecture
if (CROWN_ARCH MATCHES "x86")
	set (CROWN_THIRD ${CMAKE_SOURCE_DIR}/third/x86)
endif (CROWN_ARCH MATCHES "x86")

if (CROWN_ARCH MATCHES "x86_64")
	set (CROWN_THIRD ${CMAKE_SOURCE_DIR}/third/x86_64)
endif (CROWN_ARCH MATCHES "x86_64")

if (CROWN_ARCH MATCHES "win32")
	set (CROWN_THIRD ${CMAKE_SOURCE_DIR}/third/win32)
endif (CROWN_ARCH MATCHES "win32")

if (CROWN_ARCH MATCHES "win64")
	set (CROWN_THIRD ${CMAKE_SOURCE_DIR}/third/win64)
endif (CROWN_ARCH MATCHES "win64")

if (CROWN_ARCH MATCHES "ARMv7")
	set (CROWN_THIRD ${CMAKE_SOURCE_DIR}/third/ARMv7)
endif (CROWN_ARCH MATCHES "ARMv7")

# architecture dependent includes
set (CROWN_THIRD_INCLUDES
	${CROWN_THIRD}/luajit/include/luajit-2.0
	${CROWN_THIRD}/glew-1.9.0/include
	${CROWN_THIRD}/openAL/include
	${CROWN_THIRD}/physx/include
	${CROWN_THIRD}/physx/include/common
	${CROWN_THIRD}/physx/include/characterkinematic
	${CROWN_THIRD}/physx/include/cloth
	${CROWN_THIRD}/physx/include/common
	${CROWN_THIRD}/physx/include/cooking
	${CROWN_THIRD}/physx/include/extensions
	${CROWN_THIRD}/physx/include/foundation
	${CROWN_THIRD}/physx/include/geometry
	${CROWN_THIRD}/physx/include/particles
	${CROWN_THIRD}/physx/include/physxprofilesdk
	${CROWN_THIRD}/physx/include/physxvisualdebuggersdk
	${CROWN_THIRD}/physx/include/pvd
	${CROWN_THIRD}/physx/include/pxtask
	${CROWN_THIRD}/physx/include/RepX
	${CROWN_THIRD}/physx/include/RepXUpgrader
	${CROWN_THIRD}/physx/include/vehicle
	${CMAKE_SOURCE_DIR}/third/freetype
	${CMAKE_SOURCE_DIR}/third/stb_image
)

# architecture dependent libraries
set (CROWN_THIRD_LIBS
	${CROWN_THIRD}/luajit/lib
	${CROWN_THIRD}/glew-1.9.0/lib
	${CROWN_THIRD}/openAL/lib
	${CROWN_THIRD}/physx/lib
)

add_subdirectory(${CROWN_THIRD})

# always build the engine
add_subdirectory(engine)

# select optional projects to build
if (CROWN_BUILD_TOOLS)
	add_subdirectory(tools)
endif (CROWN_BUILD_TOOLS)

if (CROWN_BUILD_SAMPLES)
	add_subdirectory(samples)
endif (CROWN_BUILD_SAMPLES)

add_subdirectory(tests)

# add a target to generate API documentation with Doxygen
if (CROWN_BUILD_DOC)
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
              WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
              COMMENT "Generating API documentation with Doxygen" VERBATIM)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/documentation/html DESTINATION documentation)
  endif(DOXYGEN_FOUND)
endif (CROWN_BUILD_DOC)
