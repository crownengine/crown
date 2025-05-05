/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/containers/array.inl"
#include "core/filesystem/file.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/allocator.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "resource/compile_options.inl"
#include "resource/sound_resource.h"

namespace crown
{
namespace sound_resource
{
	const u8 *pcm_data(const SoundResource *sr)
	{
		return (u8 *)&sr[1];
	}

} // namespace sound_resource

#if CROWN_CAN_COMPILE
namespace sound_resource_internal
{
	struct WAVHeader
	{
		char riff[4];        // Should contain 'RIFF'
		s32 chunk_size;      // Not Needed
		char wave[4];        // Should contain 'WAVE'
		char fmt[4];         // Should contain 'fmt '
		s32 fmt_size;        // Size of format chunk
		s16 fmt_tag;         // Identifies way data is stored, 1 means no compression
		s16 fmt_channels;    // Channel, 1 means mono, 2 means stereo
		s32 fmt_sample_rate; // Samples per second
		s32 fmt_avarage;     // Average bytes per sample
		s16 fmt_block_align; // Block alignment
		s16 fmt_bits_ps;     // Number of bits per sample
		char data[4];        // Should contain 'data'
		s32 data_size;       // Data dimension
	};

	s32 compile(CompileOptions &opts)
	{
		Buffer buf = opts.read();

		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, buf), opts);

		DynamicString name(ta);
		RETURN_IF_ERROR(sjson::parse_string(name, obj["source"]), opts);

		Buffer sound = opts.read(name.c_str());
		const WAVHeader *wav = (const WAVHeader *)array::begin(sound);

		// Write.
		SoundResource sr;
		sr.version     = RESOURCE_HEADER(RESOURCE_VERSION_SOUND);
		sr.format      = SoundFormat::PCM;
		sr.size        = wav->data_size;
		sr.sample_rate = wav->fmt_sample_rate;
		sr.channels    = wav->fmt_channels;
		sr.bit_depth   = wav->fmt_bits_ps;

		opts.write(sr.version);
		opts.write(sr.format);
		opts.write(sr.size);
		opts.write(sr.sample_rate);
		opts.write(sr.channels);
		opts.write(sr.bit_depth);

		opts.write(&wav[1], sr.size);

		return 0;
	}

} // namespace sound_resource_internal
#endif // if CROWN_CAN_COMPILE

} // namespace crown
