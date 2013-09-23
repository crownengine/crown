/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "SoundRenderer.h"
#include "ALRenderer.h"
#include "SoundResource.h"
#include "StringUtils.h"
#include "Log.h"

namespace crown
{

// ALRenderer
class SoundRendererBackend
{
public:
	
	//-----------------------------------------------------------------------------
	SoundRendererBackend() :
		m_device(NULL),
		m_context(NULL)
	{
	}

	//-----------------------------------------------------------------------------
	void init()
	{
		m_device = alcOpenDevice(NULL);
		
		if (!m_device)
		{
			CE_ASSERT(false, "Cannot open audio device");
		}

		m_context = alcCreateContext(m_device, NULL);

		if (!m_context)
		{
			CE_ASSERT(false, "Cannot create context");		
		}

		AL_CHECK(alcMakeContextCurrent(m_context));

		AL_CHECK(alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED));

		AL_CHECK(alDopplerFactor(1.0f));
		AL_CHECK(alDopplerVelocity(343.0f));

		// Default listener
		Vec3 pos(0.0f, 0.0f, 0.0f);
		Vec3 vel(0.0f, 0.0f, 0.0f);
		Vec3 at(0.0f, 0.0f, -1.0f);
		Vec3 up(0.0f, 1.0f, 0.0f);

		set_listener(pos, vel, at, up);		
	}

	//-----------------------------------------------------------------------------
	void shutdown()
	{
		alcDestroyContext(m_context);
	    alcCloseDevice(m_device);
	}

	//-----------------------------------------------------------------------------
	void set_listener(const Vec3& pos, const Vec3& vel, const Vec3& or_up, const Vec3& or_at)
	{
		AL_CHECK(alListener3f(AL_POSITION, pos.x, pos.y, pos.z));
		AL_CHECK(alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z));

		ALfloat orientation[] = { or_up.x, or_up.y, or_up.z,
									or_at.x, or_at.y, or_at.z };

		AL_CHECK(alListenerfv(AL_ORIENTATION, orientation));
	}

private:

	ALCdevice*				m_device;
	ALCcontext*				m_context;

	Sound 		 			m_sounds[MAX_SOUNDS];

private:

	friend class 			SoundRenderer;
};
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
SoundRenderer::SoundRenderer(Allocator& allocator) : 
	m_allocator(allocator),
	m_num_sounds(0)
{
	m_backend = CE_NEW(m_allocator, SoundRendererBackend);
}

//-----------------------------------------------------------------------------
SoundRenderer::~SoundRenderer()
{
	if (m_backend)
	{
		CE_DELETE(m_allocator, m_backend);
	}
}

//-----------------------------------------------------------------------------
void SoundRenderer::init()
{
	m_backend->init();
}

//-----------------------------------------------------------------------------
void SoundRenderer::shutdown()
{
	m_backend->shutdown();
}

//-----------------------------------------------------------------------------
void SoundRenderer::frame()
{
	// TODO: needs additional works, but it's ok right now
	for (uint32_t i = 0; i < m_num_sounds; i++)
	{
		if (m_backend->m_sounds[i].is_playing())
		{
			m_backend->m_sounds[i].update();
		}
	}
}

//-----------------------------------------------------------------------------
uint32_t SoundRenderer::num_sounds()
{
	return m_num_sounds;
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_listener(const Vec3& pos, const Vec3& vel, const Vec3& or_up, const Vec3& or_at) const
{
	m_backend->set_listener(pos, vel, or_up, or_at);
}

//-----------------------------------------------------------------------------
SoundId SoundRenderer::create_sound(SoundResource* resource)
{
	SoundId id = m_sounds_id_table.create();

	m_backend->m_sounds[id.index].create(resource);

	m_num_sounds++;

	return id;
}

//-----------------------------------------------------------------------------
void SoundRenderer::destroy_sound(SoundId id)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	m_backend->m_sounds[id.index].destroy();

	m_sounds_id_table.destroy(id);

	m_num_sounds--;
}

//-----------------------------------------------------------------------------
void SoundRenderer::play_sound(SoundId id)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	m_backend->m_sounds[id.index].play();
}

//-----------------------------------------------------------------------------
void SoundRenderer::pause_sound(SoundId id)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	m_backend->m_sounds[id.index].pause();
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_loop(SoundId id, bool loop)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	m_backend->m_sounds[id.index].loop(loop);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_min_distance(SoundId id, const float min_distance)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	m_backend->m_sounds[id.index].set_min_distance(min_distance);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_max_distance(SoundId id, const float max_distance)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	m_backend->m_sounds[id.index].set_max_distance(max_distance);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_position(SoundId id, const Vec3& pos)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	m_backend->m_sounds[id.index].set_position(pos);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_velocity(SoundId id, const Vec3& vel)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	m_backend->m_sounds[id.index].set_velocity(vel);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_direction(SoundId id, const Vec3& dir)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	m_backend->m_sounds[id.index].set_direction(dir);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_pitch(SoundId id, const float pitch)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	m_backend->m_sounds[id.index].set_pitch(pitch);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_gain(SoundId id, const float gain)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	m_backend->m_sounds[id.index].set_gain(gain);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_rolloff(SoundId id, const float rolloff)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	m_backend->m_sounds[id.index].set_rolloff(rolloff);
}

//-----------------------------------------------------------------------------
float SoundRenderer::sound_min_distance(SoundId id) const
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	return m_backend->m_sounds[id.index].min_distance();
}

//-----------------------------------------------------------------------------
float SoundRenderer::sound_max_distance(SoundId id) const
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	return m_backend->m_sounds[id.index].max_distance();
}

//-----------------------------------------------------------------------------
Vec3 SoundRenderer::sound_position(SoundId id) const
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	return m_backend->m_sounds[id.index].position();
}

//-----------------------------------------------------------------------------
Vec3 SoundRenderer::sound_velocity(SoundId id) const
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	return m_backend->m_sounds[id.index].velocity();
}

//-----------------------------------------------------------------------------
Vec3 SoundRenderer::sound_direction(SoundId id) const
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	return m_backend->m_sounds[id.index].direction();
}

//-----------------------------------------------------------------------------
float SoundRenderer::sound_pitch(SoundId id) const
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	return m_backend->m_sounds[id.index].pitch();
}

//-----------------------------------------------------------------------------
float SoundRenderer::sound_gain(SoundId id) const
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	return m_backend->m_sounds[id.index].gain();
}

//-----------------------------------------------------------------------------
float SoundRenderer::sound_rolloff(SoundId id) const
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	return m_backend->m_sounds[id.index].rolloff();
}

//-----------------------------------------------------------------------------
bool SoundRenderer::sound_playing(SoundId id)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	return m_backend->m_sounds[id.index].is_playing();
}

} // namespace crown