/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#include "core/filesystem/file.h"
#include "core/memory/allocator.h"
#include "core/memory/memory.inl"
#include "resource/sound_mp3.h"
#if CROWN_CAN_COMPILE
#   include "core/containers/array.inl"
#   include "core/filesystem/file_buffer.inl"
#   include "core/memory/globals.h"
#   include "device/log.h"
#   include "resource/compile_options.inl"
#   include "resource/sound.h"
#   include "resource/sound_resource.h"
#endif

#define DR_MP3_IMPLEMENTATION
#if CROWN_COMPILER_GCC || CROWN_COMPILER_CLANG
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include <dr_mp3.h>
#if CROWN_COMPILER_GCC || CROWN_COMPILER_CLANG
#   pragma GCC diagnostic pop
#endif

namespace crown
{
namespace mp3
{
	static size_t read(File *file, void *data, size_t size)
	{
		size_t total = 0;
		while (size != 0) {
			const u32 chunk_size = size > UINT32_MAX ? UINT32_MAX : u32(size);
			const u32 num_read = file->read((u8 *)data + total, chunk_size);
			total += num_read;
			size -= num_read;
			if (num_read != chunk_size)
				break;
		}
		return total;
	}

	static size_t read_callback(void *user_data, void *data, size_t size)
	{
		return read((File *)user_data, data, size);
	}

	static drmp3_bool32 seek_callback(void *user_data, int offset, drmp3_seek_origin origin)
	{
		File *file = (File *)user_data;
		s64 base;
		switch (origin) {
		case DRMP3_SEEK_SET: base = 0; break;
		case DRMP3_SEEK_CUR: base = file->position(); break;
		case DRMP3_SEEK_END: base = file->size(); break;
		default: return DRMP3_FALSE;
		}

		const s64 position = base + offset;
		if (position < 0 || u64(position) > file->size())
			return DRMP3_FALSE;

		file->seek(u32(position));
		return file->position() == u32(position) ? DRMP3_TRUE : DRMP3_FALSE;
	}

	static drmp3_bool32 tell_callback(void *user_data, drmp3_int64 *cursor)
	{
		if (cursor == NULL)
			return DRMP3_FALSE;

		*cursor = ((File *)user_data)->position();
		return DRMP3_TRUE;
	}

	static void *malloc_callback(size_t size, void *user_data)
	{
		if (size > UINT32_MAX)
			return NULL;
		return ((Allocator *)user_data)->allocate(u32(size));
	}

	static void *realloc_callback(void *data, size_t size, void *user_data)
	{
		if (size > UINT32_MAX)
			return NULL;
		return ((Allocator *)user_data)->reallocate(data, u32(size));
	}

	static void free_callback(void *data, void *user_data)
	{
		((Allocator *)user_data)->deallocate(data);
	}

	drmp3 *create(File &file, Allocator &a)
	{
		drmp3 *decoder = CE_NEW(a, drmp3)();
		drmp3_allocation_callbacks allocation_callbacks;
		allocation_callbacks.pUserData = &a;
		allocation_callbacks.onMalloc = malloc_callback;
		allocation_callbacks.onRealloc = realloc_callback;
		allocation_callbacks.onFree = free_callback;

		const drmp3_bool32 success = drmp3_init(decoder
			, read_callback
			, seek_callback
			, tell_callback
			, NULL
			, &file
			, &allocation_callbacks
			);
		if (!success) {
			CE_DELETE(a, decoder);
			return NULL;
		}

		return decoder;
	}

	void destroy(drmp3 *decoder, Allocator &a)
	{
		if (decoder == NULL)
			return;

		drmp3_uninit(decoder);
		CE_DELETE(a, decoder);
	}

} // namespace mp3

} // namespace crown

#if CROWN_CAN_COMPILE
LOG_SYSTEM(MP3, "mp3")

namespace crown
{
namespace mp3
{
	s32 parse(Sound &s, Buffer &buf, CompileOptions &opts)
	{
		FileBuffer fb(buf);
		drmp3 *decoder = create(fb, default_allocator());
		RETURN_IF_FALSE(MP3, decoder != NULL, opts, "drmp3_init: failed");

		const u32 sample_rate_hz = decoder->sampleRate;
		const u32 num_channels = decoder->channels;
		const bool format_supported = sample_rate_hz != 0
			&& num_channels > 0
			&& num_channels <= 2
			;
		if (!format_supported)
			destroy(decoder, default_allocator());
		RETURN_IF_FALSE(MP3, format_supported, opts, "Unsupported MP3 format");

		const u32 wanted_frames = sample_rate_hz / 1000 * SOUND_PCM_MS;
		array::resize(s._samples, wanted_frames * num_channels);
		const u64 num_frames = drmp3_read_pcm_frames_f32(decoder
			, wanted_frames
			, array::begin(s._samples)
			);
		const bool decoded = num_frames > 0 && num_frames <= wanted_frames;
		destroy(decoder, default_allocator());
		RETURN_IF_FALSE(MP3, decoded, opts, "Malformed source");
		array::resize(s._samples, u32(num_frames) * num_channels);

		s._sample_rate = sample_rate_hz;
		s._channels = num_channels;
		s._bit_depth = 32;
		s._stream_format = StreamFormat::MP3;

		Mp3StreamMetadata mp3;
		mp3.num_frames_skip = u32(num_frames);
		FileBuffer meta_fb(s._stream_metadata);
		BinaryWriter bw(meta_fb);
		bw.write(mp3.num_frames_skip);

		// Copy the entire MP3 file to the stream output.
		opts._stream_output.write(array::begin(buf), array::size(buf));
		return 0;
	}

} // namespace mp3

} // namespace crown
#endif // if CROWN_CAN_COMPILE
