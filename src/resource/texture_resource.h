/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "core/memory/types.h"
#include "resource/types.h"
#include "resource/types.h"
#include <bgfx/bgfx.h>

namespace crown
{
struct TextureResource
{
	const bgfx::Memory* mem;
	bgfx::TextureHandle handle;
};

namespace texture_resource_internal
{
	s32 compile(CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void offline(StringId64 id, ResourceManager& rm);
	void online(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* resource);

} // namespace texture_resource_internal

} // namespace crown
