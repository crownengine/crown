/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/lua_resource.h"

namespace crown
{
namespace lua_resource
{
	const char *program(const LuaResource *lr)
	{
		return (char *)&lr[1];
	}

} // namespace lua_resource

} // namespace crown
