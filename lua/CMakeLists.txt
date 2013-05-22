cmake_minimum_required(VERSION 2.8)

project(crown-lua)

set(LUA_SRC
	MathBinds.cpp
)

add_definitions(-Wl,-E)
add_library(Math SHARED ${LUA_SRC})
target_link_libraries(Math crown)

install (TARGETS Math DESTINATION lib/${CMAKE_PROJECT_NAME})

