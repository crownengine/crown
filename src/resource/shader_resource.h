/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
#include "core/types.h"
#include "resource/types.h"
#include "resource/types.h"
#include <bgfx/bgfx.h>

namespace crown
{
struct ShaderResource
{
	ShaderResource(Allocator& a)
		: _data(a)
	{
	}

	struct Data
	{
		StringId32 name;
		u64 state;
		const bgfx::Memory* vsmem;
		const bgfx::Memory* fsmem;
	};

	Array<Data> _data;
};

namespace shader_resource_internal
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& rm);
	void offline(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* res);

} // namespace shader_resource_internal

} // namespace crown
