/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "debug_line.h"
#include "device.h"
#include "gui.h"
#include "level.h"
#include "lua_environment.h"
#include "lua_stack.h"
#include "physics_world.h"
#include "render_world.h"
#include "resource_package.h"
#include "scene_graph.h"
#include "sound_world.h"
#include "vector2.h"
#include "vector3.h"
#include "world.h"

namespace crown
{
bool LuaStack::is_vector3(int i)
{
	return device()->lua_environment()->is_vector3((Vector3*)get_pointer(i));
}

bool LuaStack::is_quaternion(int i)
{
	return device()->lua_environment()->is_quaternion((Quaternion*)get_pointer(i));
}

bool LuaStack::is_matrix4x4(int i)
{
	return device()->lua_environment()->is_matrix4x4((Matrix4x4*)get_pointer(i));
}

void LuaStack::check_temporary(int i, const Vector3* p)
{
	LuaEnvironment* env = device()->lua_environment();
	if (!is_pointer(i) || !env->is_vector3(p))
		luaL_typerror(L, i, "Vector3");
}

void LuaStack::check_temporary(int i, const Quaternion* p)
{
	LuaEnvironment* env = device()->lua_environment();
	if (!is_pointer(i) || !env->is_quaternion(p))
		luaL_typerror(L, i, "Quaternion");
}

void LuaStack::check_temporary(int i, const Matrix4x4* p)
{
	LuaEnvironment* env = device()->lua_environment();
	if (!is_pointer(i) || !env->is_matrix4x4(p))
		luaL_typerror(L, i, "Matrix4x4");
}

Vector2 LuaStack::get_vector2(int i)
{
	Vector3 v = get_vector3(i);
	return vector2(v.x, v.y);
}

Vector3& LuaStack::get_vector3(int i)
{
	Vector3* v = (Vector3*)get_pointer(i);
	check_temporary(i, v);
	return *v;
}

Quaternion& LuaStack::get_quaternion(int i)
{
	Quaternion* q = (Quaternion*)get_pointer(i);
	check_temporary(i, q);
	return *q;
}

Matrix4x4& LuaStack::get_matrix4x4(int i)
{
	Matrix4x4* m = (Matrix4x4*)get_pointer(i);
	check_temporary(i, m);
	return *m;
}

Color4 LuaStack::get_color4(int i)
{
	Quaternion q = get_quaternion(i);
	return color4(q.x, q.y, q.z, q.w);
}

void LuaStack::push_vector2(const Vector2& v)
{
	push_vector3(vector3(v.x, v.y, 0.0f));
}

void LuaStack::push_vector3(const Vector3& v)
{
	lua_pushlightuserdata(L, device()->lua_environment()->next_vector3(v));
}

void LuaStack::push_quaternion(const Quaternion& q)
{
	lua_pushlightuserdata(L, device()->lua_environment()->next_quaternion(q));
}

void LuaStack::push_matrix4x4(const Matrix4x4& m)
{
	lua_pushlightuserdata(L, device()->lua_environment()->next_matrix4x4(m));
}

void LuaStack::push_color4(const Color4& c)
{
	// Color4 represented as Quaternion
	Quaternion q;
	q.x = c.x;
	q.y = c.y;
	q.z = c.z;
	q.w = c.w;
	push_quaternion(q);
}

void LuaStack::check_type(int i, const DebugLine* p)
{
	if (!is_pointer(i) || *(u32*)p != DebugLine::MARKER)
		luaL_typerror(L, i, "DebugLine");
}

void LuaStack::check_type(int i, const ResourcePackage* p)
{
	if (!is_pointer(i) || *(u32*)p != ResourcePackage::MARKER)
		luaL_typerror(L, i, "ResourcePackage");
}

void LuaStack::check_type(int i, const World* p)
{
	if (!is_pointer(i) || *(u32*)p != World::MARKER)
		luaL_typerror(L, i, "World");
}

void LuaStack::check_type(int i, const SceneGraph* p)
{
	if (!is_pointer(i) || *(u32*)p != SceneGraph::MARKER)
		luaL_typerror(L, i, "SceneGraph");
}

void LuaStack::check_type(int i, const RenderWorld* p)
{
	if (!is_pointer(i) || *(u32*)p != RenderWorld::MARKER)
		luaL_typerror(L, i, "RenderWorld");
}

void LuaStack::check_type(int i, const Level* p)
{
	if (!is_pointer(i) || *(u32*)p != Level::MARKER)
		luaL_typerror(L, i, "Level");
}

} // namespace crown
