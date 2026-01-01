/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
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
	typedef HashMap<StringId32, ShaderData> ShaderMap;
	ShaderMap _shader_map;

	///
	explicit ShaderManager(Allocator &a);

	///
	void *load(File &file, Allocator &a);

	///
	void online(StringId64 id, ResourceManager &rm);

	///
	void offline(StringId64 id, ResourceManager &rm);

	///
	void unload(Allocator &a, void *res);

	///
	ShaderData shader(StringId32 name);
};

} // namespace crown
