cmake_minimum_required(VERSION 2.8)

include(FindFreetype)

if (FREETYPE_FOUND)
	include_directories(${FREETYPE_INCLUDE_DIRS})
	link_libraries(${FREETYPE_LIBRARIES})
endif (FREETYPE_FOUND)

add_executable(fontgen fontgen.cpp)

target_link_libraries(fontgen crown)

