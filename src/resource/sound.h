/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"

#if CROWN_CAN_COMPILE
#   include "core/filesystem/types.h"
#   include "core/math/types.h"
#   include "core/memory/types.h"
#   include "resource/types.h"

namespace crown
{
struct Sound
{
	u32 _sample_rate;
	u32 _channels;
	u32 _bit_depth;
	u32 _stream_format;
	Buffer _stream_metadata;
	Array<f32> _samples;

	///
	explicit Sound(Allocator &a);
};

namespace sound
{
	///
	s32 parse(Sound &s, const char *path, CompileOptions &opts);

	///
	s32 parse(Sound &s, CompileOptions &opts);

	///
	s32 write(Sound &s, CompileOptions &opts);

} // namespace sound

} // namespace crown

#endif // if CROWN_CAN_COMPILE
