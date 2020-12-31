/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

namespace crown
{
/// Global audio-related functions
///
/// @ingroup World
namespace audio_globals
{
	/// Initializes the audio system.
	/// This is the place where to create and initialize per-application objects.
	void init();

	/// It should reverse the actions performed by audio_globals::init().
	void shutdown();

} // namespace audio_globals

} // namespace crown
