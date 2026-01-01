/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#define STB_VORBIS_NO_PULLDATA_API
#include <stb_vorbis.c>
#if CROWN_CAN_COMPILE
#   include "core/containers/array.inl"
#   include "core/filesystem/file_buffer.inl"
#   include "core/memory/globals.h"
#   include "core/time.h"
#   include "device/log.h"
#   include "resource/compile_options.inl"
#   include "resource/sound.h"
#   include "resource/sound_ogg.h"
#   include "resource/sound_resource.h"

LOG_SYSTEM(OGG, "ogg")

namespace crown
{
namespace ogg
{
	s32 parse(Sound &s, Buffer &buf, CompileOptions &opts)
	{
		OggStreamMetadata ogg;
		const u8 *data = (u8 *)array::begin(buf);
		const u32 size = array::size(buf);
		int p = 0;
		int q = 1;

		int used;
		int error;
		stb_vorbis *v;
		while (true) {
			v = stb_vorbis_open_pushdata(data, q, &used, &error, NULL);
			if (v != NULL)
				break;

			RETURN_IF_FALSE(error == VORBIS_need_more_data
				, opts
				, "stb_vorbis_open_pushdata: error: %d"
				, error
				);
			q += 1;
		}

		RETURN_IF_FALSE(v != NULL, opts, "stb_vorbis_open_pushdata: error: %d", error);
		p += used;
		ogg.headers_size = p;

		stb_vorbis_info info = stb_vorbis_get_info(v);
		ogg.alloc_buffer_size = info.setup_memory_required
			+ info.setup_temp_memory_required
			+ info.temp_memory_required
			;
		ogg.max_frame_size = info.max_frame_size;

		// Decode SOUND_PCM_MS worth of audio samples.
		unsigned int wanted_n = info.sample_rate / 1000 * SOUND_PCM_MS;
		ogg.num_samples_skip = wanted_n;

		FileBuffer fb(buf);
		fb.seek(p);

		p = 0;
		unsigned int total_n = 0;
		unsigned char *mem = (unsigned char *)default_allocator().allocate(info.max_frame_size);

		// Decode up to wanted_n samples.
		while (total_n < wanted_n) {
			float **output;
			int n;
			q = info.max_frame_size;

		retry:
			if (q > (int)size - p)
				q = (int)size - p;
			if (p < q)
				fb.read(&mem[p], q - p);

			used = stb_vorbis_decode_frame_pushdata(v
				, mem
				, q
				, NULL
				, &output
				, &n
				);

			if (used == 0) {
				if (fb.end_of_file())
					break; // No more data.

				goto retry;
			}

			p = q - used;
			memmove(mem, &mem[used], p);
			if (n == 0)
				continue; // Seek/error recovery.

			// Frame successfully decoded.
			for (int i = 0; i < n; ++i) {
				for (int c = 0; c < info.channels; ++c)
					array::push_back(s._samples, output[c][i]);
			}
			total_n += n;
		}

		default_allocator().deallocate(mem);

		// Discard any unwanted samples.
		if (array::size(s._samples) > wanted_n * info.channels)
			array::resize(s._samples, wanted_n * info.channels);

		s._sample_rate = info.sample_rate;
		s._channels = info.channels;
		s._bit_depth = 32;
		s._stream_format = StreamFormat::OGG;

		// Write stream metadata.
		FileBuffer meta_fb(s._stream_metadata);
		BinaryWriter bw(meta_fb);
		bw.write(ogg.alloc_buffer_size);
		bw.write(ogg.headers_size);
		bw.write(ogg.max_frame_size);
		bw.write(ogg.num_samples_skip);

		// Copy entire vorbis to stream output.
		opts._stream_output.write(array::begin(buf), array::size(buf));

		stb_vorbis_close(v);
		return 0;
	}

} // namespace fbx

} // namespace crown

#endif // if CROWN_CAN_COMPILE
