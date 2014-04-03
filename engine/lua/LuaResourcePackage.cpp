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

#include "ResourcePackage.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"

namespace crown
{

//-----------------------------------------------------------------------------
static int resource_package_load(lua_State* L)
{
	LuaStack stack(L);

	ResourcePackage* package = stack.get_resource_package(1);
	package->load();

	return 0;
}

//-----------------------------------------------------------------------------
static int resource_package_unload(lua_State* L)
{
	LuaStack stack(L);

	ResourcePackage* package = stack.get_resource_package(1);
	package->unload();

	return 0;
}

//-----------------------------------------------------------------------------
static int resource_package_flush(lua_State* L)
{
	LuaStack stack(L);

	ResourcePackage* package = stack.get_resource_package(1);
	package->flush();

	return 0;
}

//-----------------------------------------------------------------------------
static int resource_package_has_loaded(lua_State* L)
{
	LuaStack stack(L);

	ResourcePackage* package = stack.get_resource_package(1);
	stack.push_bool(package->has_loaded());

	return 1;
}

//-----------------------------------------------------------------------------
static int resource_package_tostring(lua_State* L)
{
	LuaStack stack(L);
	ResourcePackage* package = stack.get_resource_package(1);
	stack.push_fstring("ResourcePackage (%p)", package);
	return 1;
}

//-----------------------------------------------------------------------------
void load_resource_package(LuaEnvironment& env)
{
	env.load_module_function("ResourcePackage", "load",       resource_package_load);
	env.load_module_function("ResourcePackage", "unload",     resource_package_unload);
	env.load_module_function("ResourcePackage", "flush",      resource_package_flush);
	env.load_module_function("ResourcePackage", "has_loaded", resource_package_has_loaded);
	env.load_module_function("ResourcePackage", "__index",    "ResourcePackage");
	env.load_module_function("ResourcePackage", "__tostring", resource_package_tostring);
}

} // namespace crown
