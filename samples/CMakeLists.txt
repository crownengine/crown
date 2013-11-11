cmake_minimum_required(VERSION 2.8)

project(crown-samples)

# platform independent includes
set (CROWN_INCLUDES
	${CMAKE_SOURCE_DIR}/engine
	${CMAKE_SOURCE_DIR}/engine/core
	${CMAKE_SOURCE_DIR}/engine/core/bv
	${CMAKE_SOURCE_DIR}/engine/core/containers
	${CMAKE_SOURCE_DIR}/engine/core/math
	${CMAKE_SOURCE_DIR}/engine/core/mem
	${CMAKE_SOURCE_DIR}/engine/core/compressors
	${CMAKE_SOURCE_DIR}/engine/core/filesystem
	${CMAKE_SOURCE_DIR}/engine/core/json
	${CMAKE_SOURCE_DIR}/engine/core/strings
	${CMAKE_SOURCE_DIR}/engine/core/settings
	${CMAKE_SOURCE_DIR}/engine/os
	${CMAKE_SOURCE_DIR}/engine/input
	${CMAKE_SOURCE_DIR}/engine/renderers
	${CMAKE_SOURCE_DIR}/engine/resource
	${CMAKE_SOURCE_DIR}/engine/network
	${CMAKE_SOURCE_DIR}/engine/lua
)

# detect operating system
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set (LINUX 1)

	list (APPEND CROWN_INCLUDES
		${CMAKE_SOURCE_DIR}/engine/os/linux
		${CMAKE_SOURCE_DIR}/engine/renderers/glx
	)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(WINDOWS 1)

	list (APPEND CROWN_INCLUDES
		${CMAKE_SOURCE_DIR}/engine/os/win
		${CMAKE_SOURCE_DIR}/engine/renderers/gl/wgl
	)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

include_directories(${CROWN_INCLUDES} ${CROWN_THIRD_INCLUDES})
link_directories(${CROWN_BINARY_DIR} ${CROWN_THIRD_LIBS})

# Install samples
install (DIRECTORY 01.hello-world DESTINATION samples)
install (DIRECTORY 02.sprites DESTINATION samples)
