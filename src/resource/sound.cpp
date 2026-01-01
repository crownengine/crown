/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#   include "core/containers/array.inl"
#   include "core/filesystem/filesystem.h"
#   include "core/json/json_object.inl"
#   include "core/json/sjson.h"
#   include "core/memory/temp_allocator.inl"
#   include "core/strings/dynamic_string.inl"
#   include "resource/compile_options.inl"
#   include "resource/sound.h"
#   include "resource/sound_ogg.h"
#   include "resource/sound_resource.h"
#   include "resource/sound_wav.h"
#   include "device/log.h"

LOG_SYSTEM(SOUND, "sound")

namespace crown
{
namespace sound
{
	s32 write(Sound &s, CompileOptions &opts)
	{
		SoundResource sr;
		sr.version              = RESOURCE_HEADER(RESOURCE_VERSION_SOUND);
		sr.sample_rate          = s._sample_rate;
		sr.channels             = s._channels;
		sr.bit_depth            = s._bit_depth;
		sr.stream_format        = s._stream_format;
		sr.stream_metadata_size = array::size(s._stream_metadata);
		sr.pcm_offset           = sizeof(sr) + sr.stream_metadata_size;
		sr.pcm_size             = array::size(s._samples) * s._bit_depth / 8;

		opts.write(sr.version);
		opts.write(sr.sample_rate);
		opts.write(sr.channels);
		opts.write(sr.bit_depth);
		opts.write(sr.stream_format);
		opts.write(sr.stream_metadata_size);
		opts.write(sr.pcm_offset);
		opts.write(sr.pcm_size);
		opts.write(sr._pad);

		opts.write(array::begin(s._stream_metadata), array::size(s._stream_metadata));

		// Convert samples.
		if (sr.bit_depth == 8) {
			const s32 scale = 255;
			for (u32 i = 0; i < array::size(s._samples); ++i) {
				s32 conv = scale * (s._samples[i] + 1.0f) * 0.5f;
				opts.write((u8)clamp(conv, 0, scale));
			}
		} else if (sr.bit_depth == 16) {
			const s32 scale = 32768;
			for (u32 i = 0; i < array::size(s._samples); ++i) {
				s32 conv = scale * s._samples[i];
				opts.write((s16)clamp(conv, -scale, scale));
			}
		} else if (sr.bit_depth == 32) {
			opts.write(array::begin(s._samples), sr.pcm_size);
		} else {
			RETURN_IF_FALSE(false, opts, "Unsupported bit depth");
		}

		return 0;
	}

	static s32 parse_internal(Sound &s, Buffer &buf, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, buf), opts);

		DynamicString source(ta);
		RETURN_IF_ERROR(sjson::parse_string(source, obj["source"]), opts);
		RETURN_IF_FILE_MISSING(source.c_str(), opts);
		Buffer sound_data = opts.read(source.c_str());

		if (source.has_suffix("wav") || source.has_suffix("WAV")) {
			return wav::parse(s, sound_data, opts);
		} else if (source.has_suffix("ogg") || source.has_suffix("OGG")) {
			return ogg::parse(s, sound_data, opts);
		}

		RETURN_IF_FALSE(false, opts, "Unsupported format");
	}

	s32 parse(Sound &s, const char *path, CompileOptions &opts)
	{
		RETURN_IF_FILE_MISSING(path, opts);
		Buffer buf = opts.read(path);
		return parse_internal(s, buf, opts);
	}

	s32 parse(Sound &s, CompileOptions &opts)
	{
		Buffer buf = opts.read();
		return parse_internal(s, buf, opts);
	}

} // namespace mesh

Sound::Sound(Allocator &a)
	: _sample_rate(0)
	, _channels(0)
	, _bit_depth(0)
	, _stream_format(StreamFormat::NONE)
	, _stream_metadata(a)
	, _samples(a)
{
}

} // namespace crown

#endif // if CROWN_CAN_COMPILE
