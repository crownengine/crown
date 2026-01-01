/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#include "core/containers/array.inl"
#include "device/log.h"
#include "resource/compile_options.inl"
#include "resource/sound_wav.h"
#include <string.h> // strncmp

LOG_SYSTEM(WAV, "wav")

namespace crown
{
namespace wav
{
	struct WavHeader
	{
		// Header chunk.
		char riff[4];       // Must contain 'RIFF'.
		s32 file_size;      // Total file size minus 8.
		char wave[4];       // Must contain 'WAVE'.

		// Data format chunk.
		char fmt[4];        // Must contain 'fmt '.
		s32 fmt_chunk_size; // Size of format chunk minus 8.
		s16 tag;            // 1 = PCM integer, 3 = float.
		s16 channels;       // Number of channels.
		s32 sample_rate;    // Sample rate in Hz.
		s32 byte_per_sec;   // Number of bytes to read per second (sample_rate * byte_per_block).
		s16 byte_per_block; // Number of bytes per block (channels * bit_depth / 8).
		s16 bit_depth;      // Bits per sample.

		// Data chunk.
		char data[4];       // Must contain 'data'.
		s32 data_size;      // Samples size in bytes.

		// Data.
	};

	s32 parse(Sound &s, Buffer &buf, CompileOptions &opts)
	{
		RETURN_IF_FALSE(array::size(buf) >= sizeof(WavHeader), opts, "Malformed source");
		const WavHeader *wav = (WavHeader *)array::begin(buf);

		// Validate header chunk.
		RETURN_IF_FALSE(strncmp(wav->riff, "RIFF", 4) == 0, opts, "Bad header chunk");
		RETURN_IF_FALSE(strncmp(wav->wave, "WAVE", 4) == 0, opts, "Bad header chunk");
		RETURN_IF_FALSE((s32)array::size(buf) == wav->file_size + 8, opts, "Truncated source");

		// Validate format chunk.
		RETURN_IF_FALSE(strncmp(wav->fmt, "fmt ", 4) == 0, opts, "Bad data format chunk");
		RETURN_IF_FALSE(wav->fmt_chunk_size == 16, opts, "Bad data format chunk size");

		RETURN_IF_FALSE(wav->tag == 1, opts, "Unsupported data format");

		RETURN_IF_FALSE(wav->channels > 0
			&& wav->channels <= 2
			, opts
			, "Unsupported number of channels"
			);
		RETURN_IF_FALSE(wav->bit_depth == 8
			|| wav->bit_depth == 16
			, opts
			, "Unsupported bit depth"
			);

		// Validate data chunk.
		RETURN_IF_FALSE(strncmp(wav->data, "data", 4) == 0, opts, "Bad data chunk");
		RETURN_IF_FALSE(wav->data_size <= s32(array::size(buf) - sizeof(*wav)), opts, "Bad data chunk size");

		// Convert to intermediate 32-bit float.
		if (wav->bit_depth == 8) {
			const f32 scale = 255.0f;
			const u8 *data = (u8 *)&wav[1];
			for (s32 i = 0; i < wav->data_size; i += wav->byte_per_block) {
				for (s16 c = 0; c < wav->channels; ++c) {
					const f32 conv = *data++ / scale * 2.0f - 1.0f;
					array::push_back(s._samples, clamp(conv, -1.0f, 1.0f));
				}
			}
		} else if (wav->bit_depth == 16) {
			const f32 scale = 32768.0f;
			const s16 *data = (s16 *)&wav[1];
			for (s32 i = 0; i < wav->data_size; i += wav->byte_per_block) {
				for (s16 c = 0; c < wav->channels; ++c) {
					const f32 conv = *data++ / scale;
					array::push_back(s._samples, clamp(conv, -1.0f, 1.0f));
				}
			}
		} else {
			CE_FATAL("Unknown bit depth");
		}

		s._sample_rate = wav->sample_rate;
		s._channels = wav->channels;
		s._bit_depth = wav->bit_depth;
		return 0;
	}

} // namespace wav

} // namespace crown

#endif // if CROWN_CAN_COMPILE
