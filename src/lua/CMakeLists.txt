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

	DeviceBinds.cpp
)

set(LUA_HEADERS
	LuaStack.h
	LuaEnvironment.h
)

if (LINUX)
add_definitions(${COMPILER_FLAGS} -Wl,-E)
endif (LINUX)

if (WINDOWS)
add_definitions(/link /OPT:NOREF)
endif (WINDOWS)

add_library(crownlua SHARED ${LUA_SRC} ${LUA_HEADERS})
target_link_libraries(crownlua crown)

set_target_properties (crownlua PROPERTIES OUTPUT_NAME libcrownlua)

install (TARGETS crownlua DESTINATION lib/${CMAKE_PROJECT_NAME})

if (WINDOWS)
install (TARGETS crownlua DESTINATION bin)
endif (WINDOWS)

install (FILES ${LUA_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/lua)

