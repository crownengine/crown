cmake_minimum_required(VERSION 2.8)

project(crown-linux)

set (LINUX_SRC
#	EGLRenderWindow.cpp
	GLXRenderWindow.cpp
	LinuxOS.cpp
	Input.cpp
	LinuxTCPSocket.cpp
	LinuxUDPSocket.cpp	
	File.cpp
)

set (LINUX_HEADERS
	File.h
#	EGLRenderWindow.h
)

link_libraries(X11 Xrandr pthread)

add_library(crown-linux STATIC ${LINUX_SRC} ${LINUX_HEADERS})

target_link_libraries(crown-linux crown)

install (TARGETS crown-linux DESTINATION lib/${CMAKE_PROJECT_NAME})
