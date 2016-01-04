/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "resource_types.h"
#include "memory_types.h"
#include "math_types.h"

namespace crown
{

typedef uint32_t SoundInstanceId;

/// Manages sound objects in a World.
///
/// @ingroup Audio
class SoundWorld
{
public:

	virtual ~SoundWorld() {};

	/// Plays the sound @a sr at the given @a volume [0 .. 1].
	/// If loop is true the sound will be played looping.
	virtual SoundInstanceId play(const SoundResource& sr, bool loop, float volume, const Vector3& pos) = 0;

	/// Stops the sound with the given @a id.
	/// After this call, the instance will be destroyed.
	virtual void stop(SoundInstanceId id) = 0;

	/// Returns whether the sound @a id is playing.
	virtual bool is_playing(SoundInstanceId id) = 0;

	/// Stops all the sounds in the world.
	virtual void stop_all() = 0;

	/// Pauses all the sounds in the world
	virtual void pause_all() = 0;

	/// Resumes all previously paused sounds in the world.
	virtual void resume_all() = 0;

	/// Sets the @a positions (in world space) of @a num sound instances @a ids.
	virtual void set_sound_positions(uint32_t num, const SoundInstanceId* ids, const Vector3* positions) = 0;

	/// Sets the @a ranges (in meters) of @a num sound instances @a ids.
	virtual void set_sound_ranges(uint32_t num, const SoundInstanceId* ids, const float* ranges) = 0;

	/// Sets the @a volumes of @a num sound instances @a ids.
	virtual void set_sound_volumes(uint32_t num, const SoundInstanceId* ids, const float* volumes) = 0;

	virtual void reload_sounds(const SoundResource& old_sr, const SoundResource& new_sr) = 0;

	/// Sets the @a pose of the listener in world space.
	virtual void set_listener_pose(const Matrix4x4& pose) = 0;

	virtual void update() = 0;

	static SoundWorld* create(Allocator& a);
	static void destroy(Allocator& a, SoundWorld* sw);
};

} // namespace crown
