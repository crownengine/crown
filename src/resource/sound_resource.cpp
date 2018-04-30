/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/filesystem/file.h"
#include "core/json/json_object.h"
#include "core/json/sjson.h"
#include "core/memory/allocator.h"
#include "core/memory/temp_allocator.h"
#include "core/strings/dynamic_string.h"
#include "resource/compile_options.h"
#include "resource/sound_resource.h"

namespace crown
{
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
		s32 fmt_avarage;     // Avarage bytes per sample
		s16 fmt_block_align; // Block alignment
		s16 fmt_bits_ps;     // Number of bits per sample
		char data[4];        // Should contain 'data'
		s32 data_size;       // Data dimension
	};

	void compile(CompileOptions& opts)
	{
		Buffer buf = opts.read();

		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(buf, object);

		DynamicString name(ta);
		sjson::parse_string(object["source"], name);

		Buffer sound = opts.read(name.c_str());
		const WAVHeader* wav = (const WAVHeader*)array::begin(sound);
		const char* wavdata = (const char*)&wav[1];

		// Write
		SoundResource sr;
		sr.version      = RESOURCE_VERSION_SOUND;
		sr.size         = wav->data_size;
		sr.sample_rate  = wav->fmt_sample_rate;
		sr.avg_bytes_ps = wav->fmt_avarage;
		sr.channels     = wav->fmt_channels;
		sr.block_size   = wav->fmt_block_align;
		sr.bits_ps      = wav->fmt_bits_ps;
		sr.sound_type   = SoundType::WAV;

		opts.write(sr.version);
		opts.write(sr.size);
		opts.write(sr.sample_rate);
		opts.write(sr.avg_bytes_ps);
		opts.write(sr.channels);
		opts.write(sr.block_size);
		opts.write(sr.bits_ps);
		opts.write(sr.sound_type);

		opts.write(wavdata, wav->data_size);
	}

} // namespace sound_resource_internal

namespace sound_resource
{
	const char* data(const SoundResource* sr)
	{
		return (char*)&sr[1];
	}

} // namespace sound_resource

} // namespace crown
