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

#include "device.h"
#include "resource_package.h"
#include "world.h"
#include "world_manager.h"
#include "lua_environment.h"
#include "lua_stack.h"
#include "temp_allocator.h"
#include "array.h"

namespace crown
{

//-----------------------------------------------------------------------------
static int device_platform(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(device()->platform());
	return 1;
}

//-----------------------------------------------------------------------------
static int device_architecture(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(device()->architecture());
	return 1;
}

//-----------------------------------------------------------------------------
static int device_version(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(device()->version());
	return 1;
}

//-----------------------------------------------------------------------------
static int device_last_delta_time(lua_State* L)
{
	LuaStack stack(L);

	float delta = device()->last_delta_time();

	stack.push_float(delta);

	return 1;
}

//-----------------------------------------------------------------------------
static int device_quit(lua_State* /*L*/)
{
	device()->quit();
	return 0;
}

//-----------------------------------------------------------------------------
static int device_resolution(lua_State* L)
{
	LuaStack stack(L);
	uint16_t w, h;
	device()->resolution(w, h);
	stack.push_int32(w);
	stack.push_int32(h);
	return 2;
}

//-----------------------------------------------------------------------------
static int device_create_world(lua_State* L)
{
	LuaStack stack(L);
	const WorldId world_id = device()->create_world();
	stack.push_world(device()->world_manager()->lookup_world(world_id));
	return 1;
}

//-----------------------------------------------------------------------------
static int device_destroy_world(lua_State* L)
{
	LuaStack stack(L);
	device()->destroy_world(stack.get_world(1)->id());
	return 0;
}

//-----------------------------------------------------------------------------
static int device_render_world(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	Camera* camera = stack.get_camera(2);

	device()->render_world(world, camera);
	return 0;
}

//-----------------------------------------------------------------------------
static int device_create_resource_package(lua_State* L)
{
	LuaStack stack(L);

	const char* package = stack.get_string(1);
	stack.push_resource_package(device()->create_resource_package(package));

	return 1;
}

//-----------------------------------------------------------------------------
static int device_destroy_resource_package(lua_State* L)
{
	LuaStack stack(L);

	ResourcePackage* package = stack.get_resource_package(1);
	device()->destroy_resource_package(package);

	return 0;
}

//-----------------------------------------------------------------------------
void load_device(LuaEnvironment& env)
{
	env.load_module_function("Device", "platform",                 device_platform);
	env.load_module_function("Device", "architecture",             device_architecture);
	env.load_module_function("Device", "version",                  device_version);
	env.load_module_function("Device", "last_delta_time",          device_last_delta_time);
	env.load_module_function("Device", "quit",                     device_quit);
	env.load_module_function("Device", "resolution",               device_resolution);
	env.load_module_function("Device", "create_world",             device_create_world);
	env.load_module_function("Device", "destroy_world",            device_destroy_world);
	env.load_module_function("Device", "render_world",             device_render_world);
	env.load_module_function("Device", "create_resource_package",  device_create_resource_package);
	env.load_module_function("Device", "destroy_resource_package", device_destroy_resource_package);
}

} // namespace crown
