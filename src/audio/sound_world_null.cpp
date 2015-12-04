/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_SOUND_NULL

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

class NullSoundWorld : public SoundWorld
{
public:

	NullSoundWorld()
	{
	}

	virtual ~NullSoundWorld()
	{
	}

	virtual SoundInstanceId play(const SoundResource& /*sr*/, bool /*loop*/, float /*volume*/, const Vector3& /*pos*/)
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

	virtual void set_sound_positions(uint32_t /*num*/, const SoundInstanceId* /*ids*/, const Vector3* /*positions*/)
	{
	}

	virtual void set_sound_ranges(uint32_t /*num*/, const SoundInstanceId* /*ids*/, const float* /*ranges*/)
	{
	}

	virtual void set_sound_volumes(uint32_t /*num*/, const SoundInstanceId* /*ids*/, const float* /*volumes*/)
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

SoundWorld* SoundWorld::create(Allocator& a)
{
	return CE_NEW(a, NullSoundWorld)();
}

void SoundWorld::destroy(Allocator& a, SoundWorld* sw)
{
	CE_DELETE(a, sw);
}

} // namespace crown

#endif // CROWN_SOUND_NULL
