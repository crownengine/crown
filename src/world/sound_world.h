/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/math/constants.h"
#include "core/math/types.h"
#include "core/memory/types.h"
#include "core/types.h"
#include "resource/types.h"
#include "world/types.h"

namespace crown
{
/// Manages sound objects in a World.
///
/// @ingroup World
struct SoundWorld
{
	///
	virtual ~SoundWorld() = default;

	/// Plays the sound with the given @a name at the given @a position, with the given
	/// @a volume and @a range. @a loop controls whether the sound must loop or not.
	/// @a group identifies the sound's group, see SoundWorld::set_group_volume().
	virtual SoundInstanceId play(StringId64 name
		, bool loop
		, f32 volume
		, f32 range
		, u32 flags = PlaySoundFlags::NONE
		, const Vector3 &pos = VECTOR3_ZERO
		, StringId32 group = StringId32(0u)
		) = 0;

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
	virtual void set_sound_positions(u32 num, const SoundInstanceId *ids, const Vector3 *positions) = 0;

	/// Sets the @a ranges (in meters) of @a num sound instances @a ids.
	virtual void set_sound_ranges(u32 num, const SoundInstanceId *ids, const f32 *ranges) = 0;

	/// Sets the @a volumes of @a num sound instances @a ids.
	virtual void set_sound_volumes(u32 num, const SoundInstanceId *ids, const f32 *volumes) = 0;

	///
	virtual void reload_sounds(const SoundResource *old_sr, const SoundResource *new_sr) = 0;

	/// Sets the @a pose of the listener in world space.
	virtual void set_listener_pose(const Matrix4x4 &pose) = 0;

	/// Sets the @a volume of the sound @a group. The volume of the sounds within
	/// @a group is multiplied by the group's volume.
	virtual void set_group_volume(StringId32 group, f32 volume) = 0;

	///
	virtual void update() = 0;
};

namespace sound_world
{
	/// Initializes the audio system.
	/// This is the place where to create and initialize per-application objects.
	void init();

	/// It should reverse the actions performed by sound_world::init().
	void shutdown();

	///
	SoundWorld *create(Allocator &a, ResourceManager &rm);

	///
	void destroy(Allocator &a, SoundWorld &sw);

} // namespace sound_world

} // namespace crown
