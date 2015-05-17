/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"
#include "lua.hpp"

#if CROWN_DEBUG
	#define LUA_ASSERT(condition, stack, msg, ...) do { if (!(condition)) {\
		stack.push_fstring("\nLua assertion failed: %s\n\t" msg "\n", #condition, ##__VA_ARGS__); lua_error(stack.state()); }} while (0);
#else
	#define LUA_ASSERT(...) ((void)0)
#endif // CROWN_DEBUG
