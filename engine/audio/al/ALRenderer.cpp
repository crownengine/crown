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

	SoundBuffer 			m_buffers[MAX_SOUND_BUFFERS];
	SoundSource 			m_sources[MAX_SOUND_SOURCES];

private:

	friend class 			SoundRenderer;
};
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
SoundRenderer::SoundRenderer(Allocator& allocator) : 
	m_allocator(allocator), 
	m_buffers_id_table(default_allocator(), MAX_SOUND_BUFFERS),
	m_sources_id_table(default_allocator(), MAX_SOUND_SOURCES),
	m_num_buffers(0),
	m_num_sources(0)
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
uint32_t SoundRenderer::num_buffers()
{
	return m_num_buffers;
}

//-----------------------------------------------------------------------------
uint32_t SoundRenderer::num_sources()
{
	return m_num_sources;
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_listener(const Vec3& pos, const Vec3& vel, const Vec3& or_up, const Vec3& or_at) const
{
	m_backend->set_listener(pos, vel, or_up, or_at);
}

//-----------------------------------------------------------------------------
SoundBufferId SoundRenderer::create_buffer(const void* data, const uint32_t size, const uint32_t sample_rate, const uint32_t channels, const uint32_t bits)
{
	SoundBufferId id = m_buffers_id_table.create();

	m_backend->m_buffers[id.index].create(data, size, sample_rate, channels, bits);

	m_num_buffers++;

	return id;
}

//-----------------------------------------------------------------------------
void SoundRenderer::update_buffer(SoundBufferId id, const void* data, const uint32_t size)
{
	CE_ASSERT(m_buffers_id_table.has(id), "SoundBuffer does not exists");

	m_backend->m_buffers[id.index].update(data, size);
}

//-----------------------------------------------------------------------------
void SoundRenderer::destroy_buffer(SoundBufferId id)
{
	CE_ASSERT(m_buffers_id_table.has(id), "SoundBuffer does not exists");

	m_backend->m_buffers[id.index].destroy();

	m_buffers_id_table.destroy(id);

	m_num_buffers--;
}

//-----------------------------------------------------------------------------
void SoundRenderer::bind_buffer_to_source(SoundBufferId bid, SoundSourceId sid)
{
	CE_ASSERT(m_buffers_id_table.has(bid), "SoundBuffer does not exists");
	CE_ASSERT(m_sources_id_table.has(sid), "SoundSource does not exists");

	SoundSource& source = m_backend->m_sources[sid.index];
	SoundBuffer& buffer = m_backend->m_buffers[bid.index];

	source.bind_buffer(buffer.m_id);
}

//-----------------------------------------------------------------------------
void SoundRenderer::unbind_buffer_from_source(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	SoundSource& source = m_backend->m_sources[id.index];

	source.unbind_buffer();
}

//-----------------------------------------------------------------------------
SoundSourceId SoundRenderer::create_source()
{
	SoundSourceId id = m_sources_id_table.create();

	m_backend->m_sources[id.index].create();

	m_num_sources++;

	return id;
}

//-----------------------------------------------------------------------------
bool SoundRenderer::update_source(SoundSourceId sid, SoundBufferId bid, const void* data, const size_t size)
{
	CE_ASSERT(m_sources_id_table.has(sid), "SoundSource does not exists");
	CE_ASSERT(m_buffers_id_table.has(bid), "SoundBuffer does not exists");

	CE_ASSERT_NOT_NULL(data);

	bool ready = false;

	int32_t processed = m_backend->m_sources[sid.index].processed_buffers();

	while (processed--)
	{
		uint32_t buffer = m_backend->m_sources[sid.index].unbind_buffer();

		update_buffer(bid, data, size);

		m_backend->m_sources[sid.index].bind_buffer(buffer);

		ready = true;
	}

	return ready;
}

//-----------------------------------------------------------------------------
void SoundRenderer::destroy_source(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_backend->m_sources[id.index].destroy();

	m_sources_id_table.destroy(id);

	m_num_sources--;
}

//-----------------------------------------------------------------------------
void SoundRenderer::play_source(SoundSourceId id, bool loop)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_backend->m_sources[id.index].play(loop);
}

//-----------------------------------------------------------------------------
void SoundRenderer::pause_source(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_backend->m_sources[id.index].pause();
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_source_min_distance(SoundSourceId id, const float min_distance)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_backend->m_sources[id.index].set_min_distance(min_distance);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_source_max_distance(SoundSourceId id, const float max_distance)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_backend->m_sources[id.index].set_max_distance(max_distance);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_source_position(SoundSourceId id, const Vec3& pos)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_backend->m_sources[id.index].set_position(pos);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_source_velocity(SoundSourceId id, const Vec3& vel)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_backend->m_sources[id.index].set_velocity(vel);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_source_direction(SoundSourceId id, const Vec3& dir)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_backend->m_sources[id.index].set_direction(dir);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_source_pitch(SoundSourceId id, const float pitch)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_backend->m_sources[id.index].set_pitch(pitch);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_source_gain(SoundSourceId id, const float gain)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_backend->m_sources[id.index].set_gain(gain);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_source_rolloff(SoundSourceId id, const float rolloff)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	m_backend->m_sources[id.index].set_rolloff(rolloff);
}

//-----------------------------------------------------------------------------
float SoundRenderer::source_min_distance(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_backend->m_sources[id.index].min_distance();
}

//-----------------------------------------------------------------------------
float SoundRenderer::source_max_distance(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_backend->m_sources[id.index].max_distance();
}

//-----------------------------------------------------------------------------
Vec3 SoundRenderer::source_position(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_backend->m_sources[id.index].position();
}

//-----------------------------------------------------------------------------
Vec3 SoundRenderer::source_velocity(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_backend->m_sources[id.index].velocity();
}

//-----------------------------------------------------------------------------
Vec3 SoundRenderer::source_direction(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_backend->m_sources[id.index].direction();
}

//-----------------------------------------------------------------------------
float SoundRenderer::source_pitch(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_backend->m_sources[id.index].pitch();
}

//-----------------------------------------------------------------------------
float SoundRenderer::source_gain(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_backend->m_sources[id.index].gain();
}

//-----------------------------------------------------------------------------
float SoundRenderer::source_rolloff(SoundSourceId id) const
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_backend->m_sources[id.index].rolloff();
}

//-----------------------------------------------------------------------------
bool SoundRenderer::source_playing(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exists");

	return m_backend->m_sources[id.index].is_playing();
}

} // namespace crown