cmake_minimum_required(VERSION 2.8)

set (FONTGEN_SRC
	BinPacker.cpp
	lodepng.cpp
)

set (FONTGEN_HEADERS
	BinPacker.hpp
	lodepng.h
)

include_directories(${CROWN_INCLUDES} ${CROWN_THIRD_INCLUDES})
add_executable(crown-fontgen main.cpp ${FONTGEN_SRC} ${FONTGEN_HEADERS})
install (TARGETS crown-fontgen DESTINATION bin)
