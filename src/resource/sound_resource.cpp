/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#include "core/containers/array.inl"
#include "core/filesystem/file.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/allocator.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "device/log.h"
#include "resource/compile_options.inl"
#include "resource/resource_id.inl"
#include "resource/sound.h"
#include "resource/sound_resource.h"

LOG_SYSTEM(SOUND_RESOURCE, "sound_resource")

namespace crown
{
namespace sound_resource_internal
{
	s32 compile(CompileOptions &opts)
	{
		Sound s(default_allocator());

		if (opts._resource_id._id == resource_id(RESOURCE_TYPE_SOUND, STRING_ID_64("core/fallback/fallback", 0xd09058ae71962248))._id) {
			opts.read();

			s._sample_rate = 44100;
			s._channels = 1;
			s._bit_depth = 16;
			array::push_back(s._samples, 0.0f);

			return sound::write(s, opts);
		}

		s32 err = sound::parse(s, opts);
		ENSURE_OR_RETURN(SOUND_RESOURCE, err == 0, opts);
		return sound::write(s, opts);
	}

} // namespace sound_resource_internal

} // namespace crown
#endif // if CROWN_CAN_COMPILE
