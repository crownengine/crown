#pragma once

#include "lua.hpp"
#include "Types.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Mat4.h"
#include "Quat.h"

namespace crown
{

const int32_t 	LUA_VEC2_BUFFER_SIZE = 4096;
Vec2 			vec2_buffer[LUA_VEC2_BUFFER_SIZE];
uint32_t 		vec2_used = 0;

const int32_t 	LUA_VEC3_BUFFER_SIZE = 4096;
Vec3 			vec3_buffer[LUA_VEC3_BUFFER_SIZE];
uint32_t 		vec3_used = 0;

const int32_t 	LUA_MAT4_BUFFER_SIZE = 4096;
Mat4 			mat4_buffer[LUA_MAT4_BUFFER_SIZE];
uint32_t 		mat4_used = 0;

const int32_t 	LUA_QUAT_BUFFER_SIZE = 4096;
Quat 			quat_buffer[LUA_QUAT_BUFFER_SIZE];
uint32_t 		quat_used = 0;

///
class LuaStack
{
public:

							LuaStack(lua_State* L);

	void					push_bool(bool value);

	void					push_int(int32_t value);

	void 					push_float(float value);

	void 					push_string(const char* str, size_t len);

	void					push_vec2(Vec2* v);

	void					push_vec3(Vec3* v);

	void					push_mat4(Mat4* m);

	void					push_quat(Quat* q);

	bool 					get_bool(int32_t index);

	int32_t					get_int(int32_t index);

	float 					get_float(int32_t index);

	const char*				get_string(int32_t index);

	Vec2*					get_vec2(int32_t index);

	Vec3*					get_vec3(int32_t index);

	Mat4*					get_mat4(int32_t index);

	Quat*					get_quat(int32_t index);

private:

	lua_State* 				m_state;
};

} // namespace crown