/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

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

} // namespace crown
