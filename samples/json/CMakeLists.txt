cmake_minimum_required(VERSION 2.8)

set (SRC
	json.cpp
)

set (HEADERS
)

add_executable (json ${SRC} ${HEADERS})
target_link_libraries(json crown)
