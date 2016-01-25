/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "memory_types.h"
#include "resource_types.h"
#include "filesystem_types.h"
#include "compiler_types.h"

namespace crown
{
struct LuaResource
{
	uint32_t version;
	uint32_t size;    // Size of lua code
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
