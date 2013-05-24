cmake_minimum_required(VERSION 2.8)

project(crown-lua)

set(LUA_SRC
	LuaStack.cpp
	LuaEnvironment.cpp
	Vec2Binds.cpp
	Vec3Binds.cpp
	Mat4Binds.cpp
	QuatBinds.cpp
	MathBinds.cpp

	MouseBinds.cpp
	KeyboardBinds.cpp
	AccelerometerBinds.cpp
)

set(LUA_HEADERS
	LuaStack.h
	LuaEnvironment.h
)

add_definitions(-Wl,-E)
add_library(crownlua SHARED ${LUA_SRC})
target_link_libraries(crownlua crown)

install (TARGETS crownlua DESTINATION lib/${CMAKE_PROJECT_NAME})

