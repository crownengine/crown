/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "sound_resource.h"
#include "dynamic_string.h"
#include "filesystem.h"
#include "json_parser.h"
#include "compile_options.h"

namespace crown
{
namespace sound_resource
{
	struct WAVHeader
	{
		char    riff[4];			// Should contain 'RIFF'
		int32_t chunk_size;			// Not Needed
		char    wave[4];			// Should contain 'WAVE'
		char    fmt[4];				// Should contain 'fmt '
		int32_t fmt_size;			// Size of format chunk
		int16_t fmt_tag;			// Identifies way data is stored, 1 means no compression
		int16_t fmt_channels;		// Channel, 1 means mono, 2 means stereo
		int32_t fmt_sample_rate;	// Samples per second
		int32_t fmt_avarage;		// Avarage bytes per sample
		int16_t fmt_block_align;	// Block alignment
		int16_t fmt_bits_ps;		// Number of bits per sample
		char    data[4];			// Should contain 'data'
		int32_t data_size;			// Data dimension
	};

	void compile(const char* path, CompileOptions& opts)
	{
		const uint32_t VERSION = 1;

		Buffer buf = opts.read(path);
		JSONParser json(array::begin(buf));
		JSONElement root = json.root();

		DynamicString name;
		root.key("source").to_string(name);

		Buffer sound = opts.read(name.c_str());
		const WAVHeader* wav = (const WAVHeader*)array::begin(sound);
		const char* wavdata = (const char*) (wav + 1);

		// Write
		SoundResource sr;
		sr.version = VERSION;
		sr.size = wav->data_size;
		sr.sample_rate = wav->fmt_sample_rate;
		sr.avg_bytes_ps = wav->fmt_avarage;
		sr.channels = wav->fmt_channels;
		sr.block_size = wav->fmt_block_align;
		sr.bits_ps = wav->fmt_bits_ps;
		sr.sound_type = SoundType::WAV;

		opts.write(sr.version);
		opts.write(sr.size);
		opts.write(sr.sample_rate);
		opts.write(sr.avg_bytes_ps);
		opts.write(sr.channels);
		opts.write(sr.block_size);
		opts.write(sr.bits_ps);
		opts.write(sr.sound_type);
		opts.write(sr._pad[0]);
		opts.write(sr._pad[1]);
		opts.write(sr._pad[2]);

		opts.write(wavdata, wav->data_size);
	}

	void* load(File& file, Allocator& a)
	{
		const size_t file_size = file.size();
		void* res = a.allocate(file_size);
		file.read(res, file_size);
		return res;
	}

	void online(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	void offline(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

	uint32_t size(const SoundResource* sr)
	{
		return sr->size;
	}

	uint32_t sample_rate(const SoundResource* sr)
	{
		return sr->sample_rate;
	}

	uint32_t avg_bytes_ps(const SoundResource* sr)
	{
		return sr->avg_bytes_ps;
	}

	uint32_t channels(const SoundResource* sr)
	{
		return sr->channels;
	}

	uint16_t block_size(const SoundResource* sr)
	{
		return sr->block_size;
	}

	uint16_t bits_ps(const SoundResource* sr)
	{
		return sr->bits_ps;
	}

	uint8_t sound_type(const SoundResource* sr)
	{
		return sr->sound_type;
	}

	const char* data(const SoundResource* sr)
	{
		return (char*)sr + sizeof(SoundResource);
	}
} // namespace sound_resource
} // namespace crown
