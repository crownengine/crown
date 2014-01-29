cmake_minimum_required(VERSION 2.8)

include(FindFreetype)

if (FREETYPE_FOUND)
	include_directories(${FREETYPE_INCLUDE_DIRS})
	link_libraries(${FREETYPE_LIBRARIES})
endif (FREETYPE_FOUND)

add_executable(fontgen main.cpp)

target_link_libraries(fontgen crown)

# fontgen compili with the following string
#g++ -w -o fontgen main.cpp stb_image.c lodepng.cpp BinPacker.cpp -I/usr/include/freetype2 -L/usr/local/lib -lfreetype