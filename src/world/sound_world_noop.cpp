/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/memory/allocator.h"
#include "core/memory/memory.inl"
#include "world/sound_world.h"

namespace crown
{
struct SoundWorldNoop : public SoundWorld
{
	u32 _marker;

	SoundWorldNoop(Allocator &a, ResourceManager &rm)
		: _marker(SOUND_WORLD_MARKER)
	{
		CE_UNUSED_2(a, rm);
	}

	~SoundWorldNoop()
	{
		_marker = 0;
	}

	SoundInstanceId play(const StringId64 name, bool loop, f32 volume, f32 range, u32 flags, const Vector3 &pos, StringId32 group)
	{
		CE_UNUSED_7(name, loop, volume, range, flags, pos, group);
		return 0;
	}

	void stop(SoundInstanceId id)
	{
		CE_UNUSED(id);
	}

	bool is_playing(SoundInstanceId id)
	{
		CE_UNUSED(id);
		return false;
	}

	void stop_all()
	{
	}

	void pause_all()
	{
	}

	void resume_all()
	{
	}

	void set_sound_positions(u32 num, const SoundInstanceId *ids, const Vector3 *positions)
	{
		CE_UNUSED_3(num, ids, positions);
	}

	void set_sound_ranges(u32 num, const SoundInstanceId *ids, const f32 *ranges)
	{
		CE_UNUSED_3(num, ids, ranges);
	}

	void set_sound_volumes(u32 num, const SoundInstanceId *ids, const f32 *volumes)
	{
		CE_UNUSED_3(num, ids, volumes);
	}

	void reload_sounds(const SoundResource *old_sr, const SoundResource *new_sr)
	{
		CE_UNUSED_2(old_sr, new_sr);
	}

	void set_listener_pose(const Matrix4x4 &pose)
	{
		CE_UNUSED(pose);
	}

	void set_group_volume(StringId32 group, f32 volume)
	{
		CE_UNUSED_2(group, volume);
	}

	void update()
	{
	}
};

namespace sound_world_noop
{
	void init()
	{
	}

	void shutdown()
	{
	}

	SoundWorld *create(Allocator &a, ResourceManager &rm)
	{
		return CE_NEW(a, SoundWorldNoop)(a, rm);
	}

	void destroy(Allocator &a, SoundWorld &sw)
	{
		return CE_DELETE(a, &sw);
	}

} // namespace sound_world_noop

} // namespace crown
