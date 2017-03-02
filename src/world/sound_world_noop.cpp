/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if SOUND_WORLD_NOOP

#include "sound_world.h"
#include "audio.h"
#include "memory.h"

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
}

class SoundWorldNoop : public SoundWorld
{
public:

	SoundWorldNoop()
	{
	}

	virtual ~SoundWorldNoop()
	{
	}

	virtual SoundInstanceId play(const SoundResource& /*sr*/, bool /*loop*/, f32 /*volume*/, f32 /*range*/, const Vector3& /*pos*/)
	{
		return 0;
	}

	virtual void stop(SoundInstanceId /*id*/)
	{
	}

	virtual bool is_playing(SoundInstanceId /*id*/)
	{
		return false;
	}

	virtual void stop_all()
	{
	}

	virtual void pause_all()
	{
	}

	virtual void resume_all()
	{
	}

	virtual void set_sound_positions(u32 /*num*/, const SoundInstanceId* /*ids*/, const Vector3* /*positions*/)
	{
	}

	virtual void set_sound_ranges(u32 /*num*/, const SoundInstanceId* /*ids*/, const f32* /*ranges*/)
	{
	}

	virtual void set_sound_volumes(u32 /*num*/, const SoundInstanceId* /*ids*/, const f32* /*volumes*/)
	{
	}

	virtual void reload_sounds(const SoundResource& /*old_sr*/, const SoundResource& /*new_sr*/)
	{
	}

	virtual void set_listener_pose(const Matrix4x4& /*pose*/)
	{
	}

	virtual void update()
	{
	}
};

namespace sound_world
{
	SoundWorld* create(Allocator& a)
	{
		return CE_NEW(a, SoundWorldNoop)();
	}

	void destroy(Allocator& a, SoundWorld* sw)
	{
		CE_DELETE(a, sw);
	}
} // namespace sound_world
} // namespace crown

#endif // SOUND_WORLD_NOOP
