cmake_minimum_required(VERSION 2.8)

project(crown-lua)

set(LUA_SRC
	MathBinds.cpp
)

add_definitions(-Wl,-E)
add_library(crownlua SHARED ${LUA_SRC})
target_link_libraries(crownlua crown)

install (TARGETS crownlua DESTINATION lib/${CMAKE_PROJECT_NAME})

