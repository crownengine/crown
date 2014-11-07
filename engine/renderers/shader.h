/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "resource_manager.h"
#include "bundle.h"
#include "file.h"
#include "reader_writer.h"
#include "memory.h"
#include "compile_options.h"
#include <bgfx.h>

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
