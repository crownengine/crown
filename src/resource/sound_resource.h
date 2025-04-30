/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
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
		WAV, ///< Waveform Audio File Format.
		OGG  ///< Ogg Vorbis audio files.
	};
};

struct SoundResource
{
	u32 version;
	u32 type;        ///< SoundType::Enum
	u32 size;        ///< Audio data size in bytes.
	u32 sample_rate; ///< Sampling rate in Hz.
	u16 channels;    ///< Number of audio channels.
	u16 bit_depth;   ///< Audio samples resolution in bits.
	// u8 data[size]
};

namespace sound_resource_internal
{
	s32 compile(CompileOptions &opts);

} // namespace	sound_resource_internal

namespace sound_resource
{
	/// Returns the sound data.
	const char *data(const SoundResource *sr);

} // namespace sound_resource

} // namespace crown
