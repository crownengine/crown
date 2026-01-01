/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"

namespace crown
{
struct OggStreamMetadata
{
	int alloc_buffer_size; ///< Size required by stb_vorbis_alloc.
	int headers_size;      ///< OGG headers size to pass to open_pushdata().
	int max_frame_size;
	int num_samples_skip;  ///< Number of samples to skip from the stream's start.
};

} // namespace crown

#if CROWN_CAN_COMPILE
#include "resource/sound.h"
#include "resource/types.h"

namespace crown
{
namespace ogg
{
	///
	s32 parse(Sound &s, Buffer &buf, CompileOptions &opts);

} // namespace ogg

} // namespace crown

#endif
