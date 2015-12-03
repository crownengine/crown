/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "camera.h"
#include "vector3.h"
#include "quaternion.h"
#include "matrix4x4.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

static int camera_set_projection_type(lua_State* L)
{
	LuaStack stack(L);
	stack.get_camera(1)->set_projection_type((ProjectionType::Enum) stack.get_int(2));
	return 0;
}

static int camera_projection_type(lua_State* L)
{
	LuaStack stack(L);
	stack.push_int(stack.get_camera(1)->projection_type());
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
