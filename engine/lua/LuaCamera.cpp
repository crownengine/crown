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

#include "Camera.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"

namespace crown
{

//-----------------------------------------------------------------------------
static int camera_local_position(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);

	stack.push_vector3(camera->local_position());
	return 1;
}

//-----------------------------------------------------------------------------
static int camera_local_rotation(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);

	stack.push_quaternion(camera->local_rotation());
	return 1;
}

//-----------------------------------------------------------------------------
static int camera_local_pose(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);

	stack.push_matrix4x4(camera->local_pose());
	return 1;
}

//-----------------------------------------------------------------------------
static int camera_world_position(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);

	stack.push_vector3(camera->world_position());
	return 1;
}

//-----------------------------------------------------------------------------
static int camera_world_rotation(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);

	stack.push_quaternion(camera->world_rotation());
	return 1;
}

//-----------------------------------------------------------------------------
static int camera_world_pose(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);

	stack.push_matrix4x4(camera->world_pose());
	return 1;
}

//-----------------------------------------------------------------------------
static int camera_set_local_position(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);
	Unit* unit = stack.get_unit(2);
	const Vector3 pos = stack.get_vector3(3);

	camera->set_local_position(unit, pos);
	return 0;
}

//-----------------------------------------------------------------------------
static int camera_set_local_rotation(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);
	Unit* unit = stack.get_unit(2);
	const Quaternion rot = stack.get_quaternion(3);

	camera->set_local_rotation(unit, rot);
	return 0;
}

//-----------------------------------------------------------------------------
static int camera_set_local_pose(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);
	Unit* unit = stack.get_unit(2);
	const Matrix4x4 pose = stack.get_matrix4x4(3);

	camera->set_local_pose(unit, pose);
	return 0;
}

//-----------------------------------------------------------------------------
static int camera_set_projection_type(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);
	ProjectionType::Enum proj_type = (ProjectionType::Enum) stack.get_int(2);

	camera->set_projection_type(proj_type);
	return 0;
}

//-----------------------------------------------------------------------------
static int camera_projection_type(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);

	stack.push_uint32(camera->projection_type());
	return 1;
}

//-----------------------------------------------------------------------------
static int camera_fov(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);

	stack.push_float(camera->fov());
	return 1;
}

//-----------------------------------------------------------------------------
static int camera_set_fov(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);
	const float fov = stack.get_float(2);

	camera->set_fov(fov);
	return 0;
}

//-----------------------------------------------------------------------------
static int camera_aspect(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);

	stack.push_float(camera->aspect());
	return 1;
}

//-----------------------------------------------------------------------------
static int camera_set_aspect(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);
	const float aspect = stack.get_float(2);

	camera->set_aspect(aspect);
	return 0;
}

//-----------------------------------------------------------------------------
static int camera_near_clip_distance(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);

	stack.push_float(camera->near_clip_distance());
	return 1;
}

//-----------------------------------------------------------------------------
static int camera_set_near_clip_distance(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);
	const float near = stack.get_float(2);

	camera->set_near_clip_distance(near);
	return 0;
}

//-----------------------------------------------------------------------------
static int camera_far_clip_distance(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);

	stack.push_float(camera->far_clip_distance());
	return 1;
}

//-----------------------------------------------------------------------------
static int camera_set_far_clip_distance(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);
	const float far = stack.get_float(2);

	camera->set_far_clip_distance(far);
	return 0;
}

//-----------------------------------------------------------------------------
static int camera_set_orthographic_metrics(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);
	const float left = stack.get_float(2);
	const float right = stack.get_float(3);
	const float bottom = stack.get_float(4);
	const float top = stack.get_float(5);

	camera->set_orthographic_metrics(left, right, bottom, top);
	return 0;
}

//-----------------------------------------------------------------------------
static int camera_set_viewport_metrics(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);
	const int16_t x = stack.get_int(2);
	const int16_t y = stack.get_int(3);
	const int16_t width = stack.get_int(4);
	const int16_t height = stack.get_int(5);

	camera->set_viewport_metrics(x, y, width, height);
	return 0;	
}

//-----------------------------------------------------------------------------
static int camera_screen_to_world(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);
	const Vector3 pos = stack.get_vector3(2);

	stack.push_vector3(camera->screen_to_world(pos));
	return 1;
}

//-----------------------------------------------------------------------------
static int camera_world_to_screen(lua_State* L)
{
	LuaStack stack(L);

	Camera* camera = stack.get_camera(1);
	const Vector3 pos = stack.get_vector3(2);

	stack.push_vector3(camera->world_to_screen(pos));
	return 1;
}

//-----------------------------------------------------------------------------
void load_camera(LuaEnvironment& env)
{
	env.load_module_function("Camera", "local_position",            camera_local_position);
	env.load_module_function("Camera", "local_rotation",            camera_local_rotation);
	env.load_module_function("Camera", "local_pose",                camera_local_pose);
	env.load_module_function("Camera", "world_position",            camera_world_position);
	env.load_module_function("Camera", "world_rotation",            camera_world_rotation);
	env.load_module_function("Camera", "world_pose",                camera_world_pose);
	env.load_module_function("Camera", "set_local_position",        camera_set_local_position);
	env.load_module_function("Camera", "set_local_rotation",        camera_set_local_rotation);
	env.load_module_function("Camera", "set_local_pose",            camera_set_local_pose);
	env.load_module_function("Camera", "set_projection_type",       camera_set_projection_type);
	env.load_module_function("Camera", "projection_type",           camera_projection_type);
	env.load_module_function("Camera", "fov",                       camera_fov);
	env.load_module_function("Camera", "set_fov",                   camera_set_fov);
	env.load_module_function("Camera", "aspect",                    camera_aspect);
	env.load_module_function("Camera", "set_aspect",                camera_set_aspect);
	env.load_module_function("Camera", "near_clip_distance",        camera_near_clip_distance);
	env.load_module_function("Camera", "set_near_clip_distance",    camera_set_near_clip_distance);
	env.load_module_function("Camera", "far_clip_distance",         camera_far_clip_distance);
	env.load_module_function("Camera", "set_far_clip_distance",     camera_set_far_clip_distance);
	env.load_module_function("Camera", "set_orthographic_metrics",  camera_set_orthographic_metrics);
	env.load_module_function("Camera", "set_viewport_metrics",      camera_set_viewport_metrics);
	env.load_module_function("Camera", "screen_to_world",  			camera_screen_to_world);
	env.load_module_function("Camera", "world_to_screen",  			camera_world_to_screen);

	env.load_module_enum("Camera", "ORTHOGRAPHIC", ProjectionType::ORTHOGRAPHIC);
	env.load_module_enum("Camera", "PERSPECTIVE", ProjectionType::PERSPECTIVE);
}

} // namespace crown