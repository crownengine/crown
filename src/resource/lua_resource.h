/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "resource/types.h"
#include "resource/types.h"

namespace crown
{
struct LuaResource
{
	u32 version;
	u32 size;
//	char program[size]
};

namespace lua_resource_internal
{
	s32 compile(CompileOptions& opts);

} // namespace lua_resource_internal

namespace lua_resource
{
	/// Returns the lua program.
	const char* program(const LuaResource* lr);

} // namespace lua_resource

} // namespace crown
