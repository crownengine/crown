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
class SoundRendererImpl
{
public:
	
	//-----------------------------------------------------------------------------
	SoundRendererImpl() :
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
		Vector3 pos(0.0f, 0.0f, 0.0f);
		Vector3 vel(0.0f, 0.0f, 0.0f);
		Vector3 at(0.0f, 0.0f, -1.0f);
		Vector3 up(0.0f, 1.0f, 0.0f);

		set_listener(pos, vel, at, up);		
	}

	//-----------------------------------------------------------------------------
	void shutdown()
	{
		alcDestroyContext(m_context);
	    alcCloseDevice(m_device);
	}

	//-----------------------------------------------------------------------------
	void set_listener(const Vector3& pos, const Vector3& vel, const Vector3& or_up, const Vector3& or_at)
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

	SoundBuffer	 			m_buffers[MAX_SOUND_BUFFERS];
	SoundSource				m_sources[MAX_SOUND_SOURCES];

private:

	friend class 			SoundRenderer;
};

//-----------------------------------------------------------------------------
SoundRenderer::SoundRenderer(Allocator& allocator)
	: m_allocator(allocator)
{
	m_impl = CE_NEW(m_allocator, SoundRendererImpl);
}

//-----------------------------------------------------------------------------
SoundRenderer::~SoundRenderer()
{
	if (m_impl)
	{
		CE_DELETE(m_allocator, m_impl);
	}
}

//-----------------------------------------------------------------------------
void SoundRenderer::init()
{
	m_impl->init();
}

//-----------------------------------------------------------------------------
void SoundRenderer::shutdown()
{
	m_impl->shutdown();
}

//-----------------------------------------------------------------------------
void SoundRenderer::frame()
{

}

//-----------------------------------------------------------------------------
void SoundRenderer::set_listener(const Vector3& pos, const Vector3& vel, const Vector3& or_up, const Vector3& or_at) const
{
	m_impl->set_listener(pos, vel, or_up, or_at);
}

//-----------------------------------------------------------------------------
SoundBufferId SoundRenderer::create_sound_buffer(void* data, size_t size, uint32_t sample_rate, uint32_t num_channels, uint16_t bits_ps)
{
	SoundBufferId id = m_buffers_id_table.create();

	m_impl->m_buffers[id.index].create(sample_rate, num_channels, bits_ps);
	m_impl->m_buffers[id.index].update(data, size);

	return id;
}

//-----------------------------------------------------------------------------
void SoundRenderer::destroy_sound_buffer(SoundBufferId id)
{
	CE_ASSERT(m_buffers_id_table.has(id), "SoundBuffer does not exists");

	m_impl->m_buffers[id.index].destroy();

	m_buffers_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
SoundSourceId SoundRenderer::create_sound_source()
{
	SoundSourceId id = m_sources_id_table.create();

	m_impl->m_sources[id.index].create();

	return id;
}

//-----------------------------------------------------------------------------
void SoundRenderer::destroy_sound_source(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_impl->m_sources[id.index].destroy();

	m_sources_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
void SoundRenderer::bind_buffer(SoundBufferId buffer, SoundSourceId source)
{
	CE_ASSERT(m_buffers_id_table.has(buffer), "SoundBuffer does not exists");
	CE_ASSERT(m_sources_id_table.has(source), "SoundSource does not exists");

	m_impl->m_sources[source.index].bind_buffer(m_impl->m_buffers[buffer.index].m_id);
}

//-----------------------------------------------------------------------------
void SoundRenderer::unbind_buffer(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_impl->m_sources[id.index].unbind_buffer();
}

//-----------------------------------------------------------------------------
void SoundRenderer::play_sound(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_impl->m_sources[id.index].play();
}

//-----------------------------------------------------------------------------
void SoundRenderer::pause_sound(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_impl->m_sources[id.index].pause();
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_loop(SoundSourceId id, bool loop)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_impl->m_sources[id.index].loop(loop);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_min_distance(SoundSourceId id, const float min_distance)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_impl->m_sources[id.index].set_min_distance(min_distance);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_max_distance(SoundSourceId id, const float max_distance)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_impl->m_sources[id.index].set_max_distance(max_distance);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_position(SoundSourceId id, const Vector3& pos)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_impl->m_sources[id.index].set_position(pos);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_velocity(SoundSourceId id, const Vector3& vel)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_impl->m_sources[id.index].set_velocity(vel);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_direction(SoundSourceId id, const Vector3& dir)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_impl->m_sources[id.index].set_direction(dir);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_pitch(SoundSourceId id, const float pitch)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_impl->m_sources[id.index].set_pitch(pitch);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_gain(SoundSourceId id, const float gain)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_impl->m_sources[id.index].set_gain(gain);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_rolloff(SoundSourceId id, const float rolloff)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_impl->m_sources[id.index].set_rolloff(rolloff);
}

//-----------------------------------------------------------------------------
float SoundRenderer::sound_min_distance(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_impl->m_sources[id.index].min_distance();
}

//-----------------------------------------------------------------------------
float SoundRenderer::sound_max_distance(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_impl->m_sources[id.index].max_distance();
}

//-----------------------------------------------------------------------------
Vector3 SoundRenderer::sound_position(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_impl->m_sources[id.index].position();
}

//-----------------------------------------------------------------------------
Vector3 SoundRenderer::sound_velocity(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_impl->m_sources[id.index].velocity();
}

//-----------------------------------------------------------------------------
Vector3 SoundRenderer::sound_direction(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_impl->m_sources[id.index].direction();
}

//-----------------------------------------------------------------------------
float SoundRenderer::sound_pitch(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_impl->m_sources[id.index].pitch();
}

//-----------------------------------------------------------------------------
float SoundRenderer::sound_gain(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_impl->m_sources[id.index].gain();
}

//-----------------------------------------------------------------------------
float SoundRenderer::sound_rolloff(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_impl->m_sources[id.index].rolloff();
}

//-----------------------------------------------------------------------------
int32_t SoundRenderer::sound_queued_buffers(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_impl->m_sources[id.index].queued_buffers();
}

//-----------------------------------------------------------------------------
int32_t SoundRenderer::sound_processed_buffers(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_impl->m_sources[id.index].processed_buffers();
}

//-----------------------------------------------------------------------------
bool SoundRenderer::sound_playing(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_impl->m_sources[id.index].is_playing();
}

} // namespace crown