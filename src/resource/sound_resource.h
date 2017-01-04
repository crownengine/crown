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

namespace crown
{
struct SoundType
{
	enum Enum
	{
		WAV,
		OGG
	};
};

struct SoundResource
{
	u32 version;
	u32 size;
	u32 sample_rate;
	u32 avg_bytes_ps;
	u32 channels;
	u16 block_size;
	u16 bits_ps;
	u32 sound_type;
};

namespace sound_resource_internal
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);
} // namespace	sound_resource_internal

namespace sound_resource
{
	/// Returns the sound data.
	const char* data(const SoundResource* sr);
} // namespace sound_resource

} // namespace crown
