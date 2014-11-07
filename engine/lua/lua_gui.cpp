/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "gui.h"
#include "lua_environment.h"
#include "lua_stack.h"
#include "vector3.h"
#include "quaternion.h"
#include "color4.h"
#include "device.h"
#include "resource_manager.h"
#include "font_resource.h"

namespace crown
{

static int gui_resolution(lua_State* L)
{
	LuaStack stack(L);
	const Vector2 resolution = stack.get_gui(1)->resolution();
	stack.push_int32((uint32_t)resolution.x);
	stack.push_int32((uint32_t)resolution.y);
	return 2;
}

static int gui_move(lua_State* L)
{
	LuaStack stack(L);
	stack.get_gui(1)->move(stack.get_vector2(2));
	return 0;
}

static int gui_screen_to_gui(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector2(stack.get_gui(1)->screen_to_gui(stack.get_vector2(2)));
	return 1;
}

static int gui_draw_rectangle(lua_State* L)
{
	LuaStack stack(L);
	stack.get_gui(1)->draw_rectangle(stack.get_vector3(2), stack.get_vector2(3),
		stack.get_color4(4));
	return 0;
}

static int gui_draw_image(lua_State* L)
{
	LuaStack stack(L);
	stack.get_gui(1)->draw_image(stack.get_string(2), stack.get_vector3(3),
		stack.get_vector2(4), stack.get_color4(5));
	return 0;
}

static int gui_draw_image_uv(lua_State* L)
{
	LuaStack stack(L);
	stack.get_gui(1)->draw_image_uv(stack.get_string(2), stack.get_vector3(3),
		stack.get_vector2(4), stack.get_vector2(5), stack.get_vector2(6), stack.get_color4(7));
	return 0;
}

static int gui_draw_text(lua_State* L)
{
	LuaStack stack(L);
	stack.get_gui(1)->draw_text(stack.get_string(2), stack.get_string(3), stack.get_int(4),
		stack.get_vector3(5), stack.get_color4(6));
	return 0;
}

void load_gui(LuaEnvironment& env)
{
	env.load_module_function("Gui", "resolution",     gui_resolution);
	env.load_module_function("Gui", "move",           gui_move);
	env.load_module_function("Gui", "screen_to_gui",  gui_screen_to_gui);
	env.load_module_function("Gui", "draw_rectangle", gui_draw_rectangle);
	env.load_module_function("Gui", "draw_image",     gui_draw_image);
	env.load_module_function("Gui", "draw_image_uv",  gui_draw_image_uv);
	env.load_module_function("Gui", "draw_text",      gui_draw_text);
}

} // namespace crown
