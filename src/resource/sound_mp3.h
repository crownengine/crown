/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"
#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "core/types.h"
#if CROWN_CAN_COMPILE
#   include "resource/sound.h"
#   include "resource/types.h"
#endif

#ifndef DR_MP3_NO_STDIO
#   define DR_MP3_NO_STDIO
#endif
#include <dr_mp3.h>

namespace crown
{
struct Mp3StreamMetadata
{
	u32 num_frames_skip; ///< Number of PCM frames to skip from the stream's start.
};

namespace mp3
{
	/// Creates an MP3 decoder reading from @a file.
	drmp3 *create(File &file, Allocator &a);

	/// Destroys @a decoder and releases all of its allocations.
	void destroy(drmp3 *decoder, Allocator &a);

} // namespace mp3

} // namespace crown

#if CROWN_CAN_COMPILE
namespace crown
{
namespace mp3
{
	///
	s32 parse(Sound &s, Buffer &buf, CompileOptions &opts);

} // namespace mp3

} // namespace crown
#endif
