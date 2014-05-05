cmake_minimum_required(VERSION 2.8)

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

include_directories(${CROWN_INCLUDES} ${CROWN_THIRD_INCLUDES})
add_executable(crown-fontgen main.cpp ${FONTGEN_SRC} ${FONTGEN_HEADERS})
install (TARGETS crown-fontgen DESTINATION bin)
