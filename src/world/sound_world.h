/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
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
	Allocator *_allocator;
	SoundWorldImpl *_impl;

	///
	SoundWorld(Allocator &a, ResourceManager &rm);

	///
	~SoundWorld();

	/// Plays the sound with the given @a name at the given @a position, with the given
	/// @a volume and @a range. @a loop controls whether the sound must loop or not.
	/// @a group identifies the sound's group, see SoundWorld::set_group_volume().
	SoundInstanceId play(StringId64 name, bool loop, f32 volume, f32 range, const Vector3 &pos, StringId32 group);

	///
	SoundWorld(const SoundWorld &) = delete;

	///
	SoundWorld &operator=(const SoundWorld &) = delete;

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
	void set_sound_positions(u32 num, const SoundInstanceId *ids, const Vector3 *positions);

	/// Sets the @a ranges (in meters) of @a num sound instances @a ids.
	void set_sound_ranges(u32 num, const SoundInstanceId *ids, const f32 *ranges);

	/// Sets the @a volumes of @a num sound instances @a ids.
	void set_sound_volumes(u32 num, const SoundInstanceId *ids, const f32 *volumes);

	///
	void reload_sounds(const SoundResource *old_sr, const SoundResource *new_sr);

	/// Sets the @a pose of the listener in world space.
	void set_listener_pose(const Matrix4x4 &pose);

	/// Sets the @a volume of the sound @a group. The volume of the sounds within
	/// @a group is multiplied by the group's volume.
	void set_group_volume(StringId32 group, f32 volume);

	///
	void update();
};

} // namespace crown
