/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/types.h"
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
	typedef HashMap<StringId32, ShaderData> ShaderMap;
	typedef HashMap<StringId32, u32> ShaderRefCountMap;
	ShaderMap _shader_map;
	ShaderRefCountMap _shader_ref_count;

	///
	explicit ShaderManager(Allocator &a);

	///
	void create_shaders(const void *shader_resource);

	///
	void destroy_shaders(const void *shader_resource);

	///
	void online(StringId64 id, ResourceManager &rm);

	///
	void offline(StringId64 id, ResourceManager &rm);

	///
	ShaderData shader(StringId32 name);
};

} // namespace crown
