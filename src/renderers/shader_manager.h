/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "memory_types.h"
#include "filesystem_types.h"
#include "resource_types.h"
#include "string_id.h"
#include <bgfx/bgfx.h>

namespace crown
{

class ShaderManager
{
public:

	struct ShaderData
	{
		uint64_t state;
		bgfx::ProgramHandle program;
	};

public:

	ShaderManager(Allocator& a);

	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& rm);
	void offline(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* res);

	/// Returns the shader @a id.
	const ShaderData& get(StringId32 id);

private:

	void add_shader(StringId32 name, uint64_t state, bgfx::ProgramHandle program);

private:

	typedef SortMap<StringId32, ShaderData> ShaderMap;

	ShaderMap _shader_map;
};

} // namespace crown
