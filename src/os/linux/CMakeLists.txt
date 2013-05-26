cmake_minimum_required(VERSION 2.8)

project(crown-linux)

set (LINUX_SRC
	LinuxOS.cpp
	OsWindow.cpp
	../posix/TCPSocket.cpp
	../posix/UDPSocket.cpp	
	../posix/File.cpp
	../posix/Thread.cpp
	../posix/Mutex.cpp
	../posix/Cond.cpp

	../../renderers/gl/GLXContext.cpp
)

set (LINUX_HEADERS
	OsWindow.h
	TCPSocket.h
	UDPSocket.h
	File.h
	Thread.h
	Mutex.h
	Cond.h

	../../renderers/gl/GLXContext.h
)

link_libraries(X11 Xrandr pthread)

add_library(crown-linux-support ${LINUX_SRC} ${LINUX_HEADERS})
add_executable(crown-linux main.cpp)

target_link_libraries(crown-linux crown)

install (TARGETS crown-linux DESTINATION bin)
