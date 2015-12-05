/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "resource_types.h"
#include "filesystem_types.h"
#include "memory_types.h"
#include "compiler_types.h"
#include "string_id.h"
#include <bgfx/bgfx.h>

namespace crown
{

struct Shader
{
	const bgfx::Memory* vs;
	const bgfx::Memory* fs;
	bgfx::ProgramHandle program;
};

namespace shader_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& rm);
	void offline(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* res);
} // namespace shader_resource
} // namespace crown
