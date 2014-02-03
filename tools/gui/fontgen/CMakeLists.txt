cmake_minimum_required(VERSION 2.8)

include(FindFreetype)

if (FREETYPE_FOUND)
	include_directories(${FREETYPE_INCLUDE_DIRS})
	link_libraries(${FREETYPE_LIBRARIES})
endif (FREETYPE_FOUND)

set (FONTGEN_SRC
	BinPacker.cpp
	lodepng.cpp
	stb_image.c
)

set (FONTGEN_HEADERS
	BinPacker.hpp
	lodepng.h
	stb_image.h
)

add_executable(crown-fontgen main.cpp ${FONTGEN_SRC} ${FONTGEN_HEADERS})
install (TARGETS crown-fontgen DESTINATION bin)