/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_SOUND_NOOP
#include "core/memory/allocator.h"
#include "core/memory/memory.inl"
#include "world/audio.h"
#include "world/sound_world.h"

namespace crown
{
namespace audio_globals
{
	void init()
	{
	}

	void shutdown()
	{
	}

} // namespace audio_globals

struct SoundWorldImpl
{
	SoundWorldImpl()
	{
	}

	~SoundWorldImpl()
	{
	}

	SoundInstanceId play(const StringId64 name, bool loop, f32 volume, f32 range, const Vector3 &pos)
	{
		CE_UNUSED_5(name, loop, volume, range, pos);
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

	void update()
	{
	}
};

SoundWorld::SoundWorld(Allocator &a)
	: _marker(SOUND_WORLD_MARKER)
	, _allocator(&a)
	, _impl(NULL)
{
	_impl = CE_NEW(*_allocator, SoundWorldImpl)();
}

SoundWorld::~SoundWorld()
{
	CE_DELETE(*_allocator, _impl);
	_marker = 0;
}

SoundInstanceId SoundWorld::play(StringId64 name, bool loop, f32 volume, f32 range, const Vector3 &pos)
{
	return _impl->play(name, loop, volume, range, pos);
}

void SoundWorld::stop(SoundInstanceId id)
{
	_impl->stop(id);
}

bool SoundWorld::is_playing(SoundInstanceId id)
{
	return _impl->is_playing(id);
}

void SoundWorld::stop_all()
{
	_impl->stop_all();
}

void SoundWorld::pause_all()
{
	_impl->pause_all();
}

void SoundWorld::resume_all()
{
	_impl->resume_all();
}

void SoundWorld::set_sound_positions(u32 num, const SoundInstanceId *ids, const Vector3 *positions)
{
	_impl->set_sound_positions(num, ids, positions);
}

void SoundWorld::set_sound_ranges(u32 num, const SoundInstanceId *ids, const f32 *ranges)
{
	_impl->set_sound_ranges(num, ids, ranges);
}

void SoundWorld::set_sound_volumes(u32 num, const SoundInstanceId *ids, const f32 *volumes)
{
	_impl->set_sound_volumes(num, ids, volumes);
}

void SoundWorld::reload_sounds(const SoundResource &old_sr, const SoundResource &new_sr)
{
	_impl->reload_sounds(old_sr, new_sr);
}

void SoundWorld::set_listener_pose(const Matrix4x4 &pose)
{
	_impl->set_listener_pose(pose);
}

void SoundWorld::update()
{
	_impl->update();
}

} // namespace crown

#endif // if CROWN_SOUND_NOOP
