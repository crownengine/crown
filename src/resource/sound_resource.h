/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
#include "core/types.h"
#include "resource/types.h"
#include "resource/types.h"

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
	s32 compile(CompileOptions& opts);

} // namespace	sound_resource_internal

namespace sound_resource
{
	/// Returns the sound data.
	const char* data(const SoundResource* sr);

} // namespace sound_resource

} // namespace crown
