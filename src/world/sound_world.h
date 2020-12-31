/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/math/types.h"
#include "core/memory/types.h"
#include "core/types.h"
#include "resource/types.h"
#include "world/types.h"

namespace crown
{
struct SoundWorldImpl;

/// Manages sound objects in a World.
///
/// @ingroup World
struct SoundWorld
{
	u32 _marker;
	Allocator* _allocator;
	SoundWorldImpl* _impl;

	///
	SoundWorld(Allocator& a);

	///
	~SoundWorld();

	/// Plays the sound @a sr at the given @a volume [0 .. 1].
	/// If loop is true the sound will be played looping.
	SoundInstanceId play(const SoundResource& sr, bool loop, f32 volume, f32 range, const Vector3& pos);

	/// Stops the sound with the given @a id.
	/// After this call, the instance will be destroyed.
	void stop(SoundInstanceId id);

	/// Returns whether the sound @a id is playing.
	bool is_playing(SoundInstanceId id);

	/// Stops all the sounds in the world.
	void stop_all();

	/// Pauses all the sounds in the world
	void pause_all();

	/// Resumes all previously paused sounds in the world.
	void resume_all();

	/// Sets the @a positions (in world space) of @a num sound instances @a ids.
	void set_sound_positions(u32 num, const SoundInstanceId* ids, const Vector3* positions);

	/// Sets the @a ranges (in meters) of @a num sound instances @a ids.
	void set_sound_ranges(u32 num, const SoundInstanceId* ids, const f32* ranges);

	/// Sets the @a volumes of @a num sound instances @a ids.
	void set_sound_volumes(u32 num, const SoundInstanceId* ids, const f32* volumes);

	///
	void reload_sounds(const SoundResource& old_sr, const SoundResource& new_sr);

	/// Sets the @a pose of the listener in world space.
	void set_listener_pose(const Matrix4x4& pose);

	///
	void update();
};

} // namespace crown
