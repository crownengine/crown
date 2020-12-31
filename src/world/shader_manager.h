/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
#include "resource/shader_resource.h"
#include "resource/types.h"
#include <bgfx/bgfx.h>

namespace crown
{
/// Manages shaders.
///
/// @ingroup World
struct ShaderManager
{
	struct ShaderData
	{
		u64 state;
		ShaderResource::Sampler samplers[4];
		bgfx::ProgramHandle program;
	};

	typedef HashMap<StringId32, ShaderData> ShaderMap;
	ShaderMap _shader_map;

	///
	ShaderManager(Allocator& a);

	///
	void* load(File& file, Allocator& a);

	///
	void online(StringId64 id, ResourceManager& rm);

	///
	void offline(StringId64 id, ResourceManager& rm);

	///
	void unload(Allocator& a, void* res);

	///
	void add_shader(StringId32 name, u64 state, const ShaderResource::Sampler samplers[4], bgfx::ProgramHandle program);

	///
	u32 sampler_state(StringId32 shader_id, StringId32 sampler_name);

	///
	void submit(StringId32 shader_id, u8 view_id, s32 depth = 0, u64 state = UINT64_MAX);
};

} // namespace crown
