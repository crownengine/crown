/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "camera.h"
#include "vector3.h"
#include "quaternion.h"
#include "matrix4x4.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

static int camera_local_position(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_camera(1)->local_position());
	return 1;
}

static int camera_local_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_camera(1)->local_rotation());
	return 1;
}

static int camera_local_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_camera(1)->local_pose());
	return 1;
}

static int camera_world_position(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_camera(1)->world_position());
	return 1;
}

static int camera_world_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_camera(1)->world_rotation());
	return 1;
}

static int camera_world_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_camera(1)->world_pose());
	return 1;
}

static int camera_set_local_position(lua_State* L)
{
	LuaStack stack(L);
	stack.get_camera(1)->set_local_position(stack.get_unit(2), stack.get_vector3(3));
	return 0;
}

static int camera_set_local_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.get_camera(1)->set_local_rotation(stack.get_unit(2), stack.get_quaternion(3));
	return 0;
}

static int camera_set_local_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.get_camera(1)->set_local_pose(stack.get_unit(2), stack.get_matrix4x4(3));
	return 0;
}

static int camera_set_projection_type(lua_State* L)
{
	LuaStack stack(L);
	stack.get_camera(1)->set_projection_type((ProjectionType::Enum) stack.get_int(2));
	return 0;
}

static int camera_projection_type(lua_State* L)
{
	LuaStack stack(L);
	stack.push_uint32(stack.get_camera(1)->projection_type());
	return 1;
}

static int camera_fov(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_camera(1)->fov());
	return 1;
}

static int camera_set_fov(lua_State* L)
{
	LuaStack stack(L);
	stack.get_camera(1)->set_fov(stack.get_float(2));
	return 0;
}

static int camera_aspect(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_camera(1)->aspect());
	return 1;
}

static int camera_set_aspect(lua_State* L)
{
	LuaStack stack(L);
	stack.get_camera(1)->set_aspect(stack.get_float(2));
	return 0;
}

static int camera_near_clip_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_camera(1)->near_clip_distance());
	return 1;
}

static int camera_set_near_clip_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.get_camera(1)->set_near_clip_distance(stack.get_float(2));
	return 0;
}

static int camera_far_clip_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_camera(1)->far_clip_distance());
	return 1;
}

static int camera_set_far_clip_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.get_camera(1)->set_far_clip_distance(stack.get_float(2));
	return 0;
}

static int camera_set_orthographic_metrics(lua_State* L)
{
	LuaStack stack(L);
	stack.get_camera(1)->set_orthographic_metrics(stack.get_float(2), stack.get_float(3),
		stack.get_float(4), stack.get_float(5));
	return 0;
}

static int camera_set_viewport_metrics(lua_State* L)
{
	LuaStack stack(L);
	stack.get_camera(1)->set_viewport_metrics(stack.get_int(2), stack.get_int(3),
		stack.get_int(4), stack.get_int(5));
	return 0;
}

static int camera_screen_to_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_camera(1)->screen_to_world(stack.get_vector3(2)));
	return 1;
}

static int camera_world_to_screen(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_camera(1)->world_to_screen(stack.get_vector3(2)));
	return 1;
}

void load_camera(LuaEnvironment& env)
{
	env.load_module_function("Camera", "local_position",           camera_local_position);
	env.load_module_function("Camera", "local_rotation",           camera_local_rotation);
	env.load_module_function("Camera", "local_pose",               camera_local_pose);
	env.load_module_function("Camera", "world_position",           camera_world_position);
	env.load_module_function("Camera", "world_rotation",           camera_world_rotation);
	env.load_module_function("Camera", "world_pose",               camera_world_pose);
	env.load_module_function("Camera", "set_local_position",       camera_set_local_position);
	env.load_module_function("Camera", "set_local_rotation",       camera_set_local_rotation);
	env.load_module_function("Camera", "set_local_pose",           camera_set_local_pose);
	env.load_module_function("Camera", "set_projection_type",      camera_set_projection_type);
	env.load_module_function("Camera", "projection_type",          camera_projection_type);
	env.load_module_function("Camera", "fov",                      camera_fov);
	env.load_module_function("Camera", "set_fov",                  camera_set_fov);
	env.load_module_function("Camera", "aspect",                   camera_aspect);
	env.load_module_function("Camera", "set_aspect",               camera_set_aspect);
	env.load_module_function("Camera", "near_clip_distance",       camera_near_clip_distance);
	env.load_module_function("Camera", "set_near_clip_distance",   camera_set_near_clip_distance);
	env.load_module_function("Camera", "far_clip_distance",        camera_far_clip_distance);
	env.load_module_function("Camera", "set_far_clip_distance",    camera_set_far_clip_distance);
	env.load_module_function("Camera", "set_orthographic_metrics", camera_set_orthographic_metrics);
	env.load_module_function("Camera", "set_viewport_metrics",     camera_set_viewport_metrics);
	env.load_module_function("Camera", "screen_to_world",          camera_screen_to_world);
	env.load_module_function("Camera", "world_to_screen",          camera_world_to_screen);
	
	env.load_module_enum("Camera", "ORTHOGRAPHIC", ProjectionType::ORTHOGRAPHIC);
	env.load_module_enum("Camera", "PERSPECTIVE",  ProjectionType::PERSPECTIVE);
}

} // namespace crown
