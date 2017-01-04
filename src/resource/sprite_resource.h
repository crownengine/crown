/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "compiler_types.h"
#include "filesystem_types.h"
#include "memory_types.h"
#include "resource_types.h"
#include "string_id.h"
#include "types.h"
#include <bgfx/bgfx.h>

namespace crown
{
// header
// num_verts
// verts[num_verts]
// num_inds
// inds[num_inds]

struct SpriteResource
{
	u32 version;
	const bgfx::Memory* vbmem;
	const bgfx::Memory* ibmem;
	bgfx::VertexBufferHandle vb;
	bgfx::IndexBufferHandle ib;
};

namespace sprite_resource_internal
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& rm);
	void offline(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* resource);
} // namespace sprite_resource_internal

struct SpriteAnimationResource
{
	u32 version;
	u32 num_frames;
	f32 total_time;
};

namespace sprite_animation_resource_internal
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& rm);
	void offline(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* resource);
} // namespace sprite_animation_resource_internal

namespace sprite_animation_resource
{
	/// Returns the frames of the sprite animation @a sar.
	const u32* frames(const SpriteAnimationResource* sar);
} // namespace sprite_animation_resource

} // namespace crown
