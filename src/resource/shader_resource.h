/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
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

	struct Sampler
	{
		u32 name;
		u32 state;
	};

	struct Data
	{
		StringId32 name;
		u64 state;
		Sampler samplers[4];
		const bgfx::Memory* vsmem;
		const bgfx::Memory* fsmem;
	};

	Array<Data> _data;
};

namespace shader_resource_internal
{
	s32 compile(CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& rm);
	void offline(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* res);

} // namespace shader_resource_internal

} // namespace crown
