cmake_minimum_required(VERSION 2.8)

project(crown-samples)

# platform independent includes
set (CROWN_INCLUDES
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/src/core
	${CMAKE_SOURCE_DIR}/src/core/bv
	${CMAKE_SOURCE_DIR}/src/core/containers
	${CMAKE_SOURCE_DIR}/src/core/math
	${CMAKE_SOURCE_DIR}/src/core/mem
	${CMAKE_SOURCE_DIR}/src/core/compressors
	${CMAKE_SOURCE_DIR}/src/core/filesystem
	${CMAKE_SOURCE_DIR}/src/core/strings
	${CMAKE_SOURCE_DIR}/src/core/settings
	${CMAKE_SOURCE_DIR}/src/os
	${CMAKE_SOURCE_DIR}/src/input
	${CMAKE_SOURCE_DIR}/src/renderers
	${CMAKE_SOURCE_DIR}/src/network
	${CMAKE_SOURCE_DIR}/src/lua
)

# detect operating system
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set (LINUX 1)

	list (APPEND CROWN_INCLUDES
		${CMAKE_SOURCE_DIR}/src/os/linux
		${CMAKE_SOURCE_DIR}/src/renderers/glx
	)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(WINDOWS 1)

	list (APPEND CROWN_INCLUDES
		${CMAKE_SOURCE_DIR}/src/os/win
		${CMAKE_SOURCE_DIR}/src/renderers/gl/wgl
	)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

include_directories(${CROWN_INCLUDES})
link_directories(${CROWN_BINARY_DIR} ${CROWN_THIRD_LIBS})

#add_subdirectory(terrain)

# Install samples
install (DIRECTORY 01.hello-world DESTINATION samples)

