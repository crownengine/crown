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
	// See: https://web.archive.org/web/20260617050002/https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
	struct WavHeader
	{
		char fourcc[4];      // Must contain 'RIFF'.
		s32 file_size;       // Total file size minus 8.
		char wave_fourcc[4]; // Must contain 'WAVE'.
	};

	struct WavFormat
	{
		char fourcc[4];     // Must contain 'fmt '.
		s32 size;           // Size of this chunk minus 8.
		s16 tag;            // See WAVE_FORMAT_*
		s16 channels;       // Number of channels.
		s32 sample_rate;    // Sample rate in Hz.
		s32 byte_per_sec;   // Number of bytes to read per second (sample_rate * byte_per_block).
		s16 byte_per_block; // Number of bytes per block (channels * bit_depth / 8).
		s16 bit_depth;      // Bits per sample.
	};

	struct WavFormatExt
	{
		s16 size; // Size of extended data (0 or 22).
	};

	struct WavFormatExt22
	{
		s16 valid_bits_per_sample; // Number of valid bits.
		s32 channel_mask;          // Speaker position mask.
		char sub_format[16];       // GUID.
	};

	struct WavData
	{
		char fourcc[4]; // Must contain 'data'.
		s32 size;       // Samples size in bytes.
		// Data.
	};

#define WAVE_FORMAT_PCM        INT16_C(0x0001) // PCM
#define WAVE_FORMAT_IEEE_FLOAT INT16_C(0x0003) // IEEE float
#define WAVE_FORMAT_ALAW       INT16_C(0x0006) // 8-bit ITU-T G.711 A-law
#define WAVE_FORMAT_MULAW      INT16_C(0x0007) // 8-bit ITU-T G.711 µ-law
#define WAVE_FORMAT_EXTENSIBLE INT16_C(0xFFFE) // Determined by SubFormat(s32)

	s32 parse(Sound &s, Buffer &buf, CompileOptions &opts)
	{
		RETURN_IF_FALSE(WAV, array::size(buf) >= sizeof(WavHeader), opts, "Malformed source");
		const WavHeader *wav = (WavHeader *)array::begin(buf);

		// Validate header chunk.
		RETURN_IF_FALSE(WAV, strncmp(wav->fourcc, "RIFF", 4) == 0, opts, "Bad header chunk");
		RETURN_IF_FALSE(WAV, strncmp(wav->wave_fourcc, "WAVE", 4) == 0, opts, "Bad header chunk");
		RETURN_IF_FALSE(WAV, (s32)array::size(buf) == wav->file_size + 8, opts, "Truncated source");

		const WavFormat *fmt = (WavFormat *)&wav[1];
		const WavFormatExt *fmt_ext = NULL;
		const WavFormatExt22 *fmt_ext22 = NULL;
		const WavData *data = NULL;

		// Validate format chunk.
		RETURN_IF_FALSE(WAV, strncmp(fmt->fourcc, "fmt ", 4) == 0, opts, "Bad data format chunk");
		RETURN_IF_FALSE(WAV, fmt->tag == WAVE_FORMAT_PCM, opts, "Unsupported data format");

		if (fmt->size == 40 || fmt->size == 18) {
			fmt_ext = (WavFormatExt *)&fmt[1];

			if (fmt->size == 40) {
				RETURN_IF_FALSE(WAV, fmt_ext->size == 22, opts, "Bad extended data format size");
				fmt_ext22 = (WavFormatExt22 *)&fmt_ext[1];
				data = (WavData *)&fmt_ext22[1];
			} else {
				RETURN_IF_FALSE(WAV, fmt_ext->size == 0, opts, "Bad extended data format size");
				data = (WavData *)&fmt_ext[1];
			}
		} else {
			RETURN_IF_FALSE(WAV, fmt->size == 16, opts, "Bad data format size");
			// Some encoders write extended format data even though they report the format chunk to
			// be 16-bytes long (i.e. standard, non-extended length). Scan the buffer for the 'data'
			// chunk.
			const void *data_fourcc = NULL;
			const char *p = (char *)&fmt[1];
			u32 n = u32(array::end(buf) - p);
			for (u32 i = 0; i + 4 <= n; ++i) {
				if (strncmp(p + i, "data", 4) == 0) {
					data_fourcc = p + i;
					break;
				}
			}

			RETURN_IF_FALSE(WAV, data_fourcc != NULL, opts, "Data chunk not found");
			data = (WavData *)data_fourcc;
		}
		CE_ENSURE(data != NULL);

		RETURN_IF_FALSE(WAV, fmt->channels > 0
			&& fmt->channels <= 2
			, opts
			, "Unsupported number of channels"
			);
		RETURN_IF_FALSE(WAV, fmt->bit_depth == 8
			|| fmt->bit_depth == 16
			, opts
			, "Unsupported bit depth"
			);

		// Validate data chunk.
		RETURN_IF_FALSE(WAV, strncmp(data->fourcc, "data", 4) == 0, opts, "Bad data chunk");
		RETURN_IF_FALSE(WAV, data->size <= s32(array::size(buf) - sizeof(*wav)), opts, "Bad data chunk size");

		// Convert to intermediate 32-bit float.
		if (fmt->bit_depth == 8) {
			const f32 scale = 255.0f;
			const u8 *pcm = (u8 *)&data[1];
			for (s32 i = 0; i < data->size; i += fmt->byte_per_block) {
				for (s16 c = 0; c < fmt->channels; ++c) {
					const f32 conv = *pcm++ / scale * 2.0f - 1.0f;
					array::push_back(s._samples, clamp(conv, -1.0f, 1.0f));
				}
			}
		} else if (fmt->bit_depth == 16) {
			const f32 scale = 32768.0f;
			const s16 *pcm = (s16 *)&data[1];
			for (s32 i = 0; i < data->size; i += fmt->byte_per_block) {
				for (s16 c = 0; c < fmt->channels; ++c) {
					const f32 conv = *pcm++ / scale;
					array::push_back(s._samples, clamp(conv, -1.0f, 1.0f));
				}
			}
		} else {
			CE_FATAL("Unknown bit depth");
		}

		s._sample_rate = fmt->sample_rate;
		s._channels = fmt->channels;
		s._bit_depth = fmt->bit_depth;
		return 0;
	}

} // namespace wav

} // namespace crown

#endif // if CROWN_CAN_COMPILE
