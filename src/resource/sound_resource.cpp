/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/containers/array.inl"
#include "core/filesystem/file.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/allocator.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "resource/compile_options.inl"
#include "resource/sound.h"
#include "resource/sound_resource.h"

namespace crown
{
namespace sound_resource
{
	const u8 *stream_metadata(const SoundResource *sr)
	{
		return (u8 *)&sr[1];
	}

	const u8 *pcm_data(const SoundResource *sr)
	{
		return (u8 *)sr + sr->pcm_offset;
	}

} // namespace sound_resource

#if CROWN_CAN_COMPILE
namespace sound_resource_internal
{
	s32 compile(CompileOptions &opts)
	{
		Sound s(default_allocator());
		s32 err = sound::parse(s, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		return sound::write(s, opts);
	}

} // namespace sound_resource_internal
#endif // if CROWN_CAN_COMPILE

} // namespace crown
