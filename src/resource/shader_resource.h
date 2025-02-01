/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"
#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
#include "core/types.h"
#include "resource/types.h"
#include <bgfx/bgfx.h>

namespace crown
{
struct ShaderResource
{
	explicit ShaderResource(Allocator &a)
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
		const bgfx::Memory *vsmem;
		const bgfx::Memory *fsmem;
	};

	Array<Data> _data;
};

struct ShaderData
{
	u64 state;
	ShaderResource::Sampler samplers[4];
	bgfx::ProgramHandle program;
#if CROWN_CAN_RELOAD
	const ShaderResource *resource;
#endif
};

namespace shader_resource_internal
{
	s32 compile(CompileOptions &opts);
	void *load(File &file, Allocator &a);
	void online(StringId64 id, ResourceManager &rm);
	void offline(StringId64 id, ResourceManager &rm);
	void unload(Allocator &a, void *res);

} // namespace shader_resource_internal

} // namespace crown
