/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
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
#define SOUND_MAX_BUFFERS   4 ///< Maximum number of streaming buffers.
#define SOUND_BUFFER_MS   200 ///< Capacity of a sound buffer in milliseconds.
#define SOUND_PCM_MS      (SOUND_MAX_BUFFERS*SOUND_BUFFER_MS) ///< Maximum size of decoded PCM data in milliseconds.

struct StreamFormat
{
	enum Enum
	{
		NONE, ///< Sound has no stream resource.
		PCM,  ///< Raw PCM audio samples.
		OGG   ///< Ogg Vorbis audio files.
	};
};

struct SoundResource
{
	u32 version;
	u32 sample_rate;          ///< Sampling rate in Hz.
	u16 channels;             ///< Number of audio channels.
	u16 bit_depth;            ///< Audio samples resolution in bits.
	u32 stream_format;        ///< StreamFormat::Enum
	u32 stream_metadata_size; ///< Size of stream resource's metadata.
	u32 pcm_offset;           ///< PCM data offset.
	u32 pcm_size;             ///< PCM data size in bytes.
	char _pad[4];             ///< 16-bytes boundary.
	// u8 stream_metadata[stream_metadata_size]
	// u8 pcm_data[pcm_size]
};
CE_STATIC_ASSERT(sizeof(SoundResource) % 16 == 0);

namespace sound_resource
{
	/// Returns the sound resource's stream metadata.
	/// @note Only useful when sr->stream_format != StreamFormat::NONE.
	const u8 *stream_metadata(const SoundResource *sr);

	/// Returns the raw PCM data.
	const u8 *pcm_data(const SoundResource *sr);

} // namespace sound_resource

} // namespace crown
