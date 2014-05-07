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

#include "Gui.h"
#include "LuaEnvironment.h"
#include "LuaStack.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Color4.h"
#include "Device.h"
#include "ResourceManager.h"
#include "FontResource.h"

namespace crown
{

//-----------------------------------------------------------------------------
static int gui_resolution(lua_State* L)
{
	LuaStack stack(L);

	Gui* gui = stack.get_gui(1);
	const Vector2 resolution = gui->resolution();

	stack.push_int32((uint32_t)resolution.x);
	stack.push_int32((uint32_t)resolution.y);

	return 2;
}

//-----------------------------------------------------------------------------
static int gui_move(lua_State* L)
{
	LuaStack stack(L);

	Gui* gui = stack.get_gui(1);
	gui->move(stack.get_vector2(2));
	return 0;
}

//-----------------------------------------------------------------------------
static int gui_screen_to_gui(lua_State* L)
{
	LuaStack stack(L);

	Gui* gui = stack.get_gui(1);
	stack.push_vector2(gui->screen_to_gui(stack.get_vector2(2)));
	return 1;
}

//-----------------------------------------------------------------------------
static int gui_show(lua_State* L)
{
	LuaStack stack(L);

	Gui* gui = stack.get_gui(1);

	gui->show();

	return 0;
}

//-----------------------------------------------------------------------------
static int gui_hide(lua_State* L)
{
	LuaStack stack(L);

	Gui* gui = stack.get_gui(1);

	gui->hide();

	return 0;
}

//-----------------------------------------------------------------------------
static int gui_draw_rectangle(lua_State* L)
{
	LuaStack stack(L);

	Gui* gui = stack.get_gui(1);
	gui->draw_rectangle(stack.get_vector3(2), stack.get_vector2(3), stack.get_color4(4));
	return 0;
}

//-----------------------------------------------------------------------------
static int gui_draw_image(lua_State* L)
{
	LuaStack stack(L);

	Gui* gui = stack.get_gui(1);
	gui->draw_image(stack.get_string(2), stack.get_vector3(3), stack.get_vector2(4), stack.get_color4(5));
	return 0;
}

//-----------------------------------------------------------------------------
static int gui_draw_text(lua_State* L)
{
	LuaStack stack(L);

	Gui* gui = stack.get_gui(1);
	gui->draw_text(stack.get_string(2), stack.get_string(3), stack.get_int(4), stack.get_vector3(5), stack.get_color4(6));
	return 0;
}

//-----------------------------------------------------------------------------
void load_gui(LuaEnvironment& env)
{
	env.load_module_function("Gui", "resolution",		gui_resolution);
	env.load_module_function("Gui", "move",				gui_move);
	env.load_module_function("Gui", "screen_to_gui",	gui_screen_to_gui);
	env.load_module_function("Gui", "show",				gui_show);
	env.load_module_function("Gui", "hide",				gui_hide);

	env.load_module_function("Gui", "draw_rectangle",	gui_draw_rectangle);
	env.load_module_function("Gui", "draw_image",		gui_draw_image);
	env.load_module_function("Gui", "draw_text",		gui_draw_text);
}

} // namespace crown
