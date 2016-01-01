/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "memory_types.h"
#include "resource_types.h"
#include "filesystem_types.h"
#include "compiler_types.h"
#include "string_id.h"
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
	uint32_t version;
	const bgfx::Memory* vbmem;
	const bgfx::Memory* ibmem;
	bgfx::VertexBufferHandle vb;
	bgfx::IndexBufferHandle ib;
};

namespace sprite_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& rm);
	void offline(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* resource);
} // namespace sprite_resource

struct SpriteAnimationResource
{
	uint32_t version;
	uint32_t num_animations;
	uint32_t num_frames;
	uint32_t frames_offset;
};

struct SpriteAnimationName
{
	StringId32 id;
};

struct SpriteAnimationData
{
	uint32_t num_frames;
	uint32_t first_frame;
	float time;
};

namespace sprite_animation_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& rm);
	void offline(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* resource);
	const SpriteAnimationData* get_animation(const SpriteAnimationResource* sar, StringId32 name);
	const uint32_t* get_animation_frames(const SpriteAnimationResource* sar);
}

} // namespace crown
