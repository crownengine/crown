cmake_minimum_required(VERSION 2.8)

project(crown-linux)

set (LINUX_SRC
	GLXRenderWindow.cpp
	LinuxOS.cpp
	Input.cpp
	TCPSocket.cpp
	UDPSocket.cpp	
	File.cpp
)

set (LINUX_HEADERS
	TCPSocket.h
	UDPSocket.h
	File.h
)

link_libraries(X11 Xrandr pthread)

add_library(crown-linux-support ${LINUX_SRC} ${LINUX_HEADERS})
add_executable(crown-linux main.cpp)

target_link_libraries(crown-linux crown)

install (TARGETS crown-linux DESTINATION bin)
