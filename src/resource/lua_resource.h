/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "compiler_types.h"
#include "filesystem_types.h"
#include "memory_types.h"
#include "resource_types.h"

namespace crown
{
struct LuaResource
{
	u32 version;
	u32 size;
//	char program[size]
};

namespace lua_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);

	/// Returns the lua program.
	const char* program(const LuaResource* lr);
} // namespace lua_resource
} // namespace crown
