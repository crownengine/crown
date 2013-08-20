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

#include "ALRenderer.h"
#include "StringUtils.h"
#include <vorbis/vorbisfile.h>
#include "OggBufferCallback.h"

namespace crown
{

//-----------------------------------------------------------------------------
static const char* al_error_to_string(ALenum error)
{
	switch (error)
	{
		case AL_INVALID_ENUM: return "AL_INVALID_ENUM";
		case AL_INVALID_VALUE: return "AL_INVALID_VALUE";
		case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION";
		case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY";
		default: return "UNKNOWN_AL_ERROR";
	}
}

//-----------------------------------------------------------------------------
#ifdef CROWN_DEBUG
	#define AL_CHECK(function)\
		function;\
		do { ALenum error; CE_ASSERT((error = alGetError()) == AL_NO_ERROR,\
				"OpenAL error: %s", al_error_to_string(error)); } while (0)
#else
	#define AL_CHECK(function)\
		function;
#endif


//-----------------------------------------------------------------------------
AudioRenderer* AudioRenderer::create(Allocator& a)
{
	return CE_NEW(a, ALRenderer);
}

//-----------------------------------------------------------------------------
void AudioRenderer::destroy(Allocator& a, AudioRenderer* renderer)
{
	CE_DELETE(a, renderer);
}

//-----------------------------------------------------------------------------
ALRenderer::ALRenderer() :
	m_buffers_id_table(m_allocator, MAX_BUFFERS),
	m_sources_id_table(m_allocator, MAX_SOURCES)
{

}

//-----------------------------------------------------------------------------
void ALRenderer::init()
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
void ALRenderer::shutdown()
{
    AL_CHECK(alcDestroyContext(m_context));
    AL_CHECK(alcCloseDevice(m_device));
}

//-----------------------------------------------------------------------------
void ALRenderer::set_listener(const Vec3& pos, const Vec3& vel, const Vec3& or_up, const Vec3& or_at) const
{
	AL_CHECK(alListener3f(AL_POSITION, pos.x, pos.y, pos.z));
	AL_CHECK(alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z));

	ALfloat orientation[] = { or_up.x, or_up.y, or_up.z,
								or_at.x, or_at.y, or_at.z };

	AL_CHECK(alListenerfv(AL_ORIENTATION, orientation));
}

//-----------------------------------------------------------------------------
SoundBufferId ALRenderer::create_buffer(const void* data, const uint32_t size, const uint32_t sample_rate, const uint32_t channels, const uint32_t bxs)
{
	SoundBufferId id = m_buffers_id_table.create();

	SoundBuffer& al_buffer = m_buffers[id.index];

	// Generates AL buffer
	AL_CHECK(alGenBuffers(1, &al_buffer.id));

	ALenum format;

	bool stereo = (channels > 1);

	// Sets sound's format
	switch(bxs)
	{
		case 8:
		{
			if (stereo)
			{
				format = AL_FORMAT_STEREO8;
			}
			else
			{
				format = AL_FORMAT_MONO8;
			}

			break;
		}

		case 16:
		{
			if (stereo)
			{
				format = AL_FORMAT_STEREO16;
			}
			else
			{
				format = AL_FORMAT_MONO16;
			}

			break;
		}

		default:
		{
			CE_ASSERT(false, "Wrong number of bits per sample.");
			break;
		}
	}

	// Fills AL buffer
	AL_CHECK(alBufferData(al_buffer.id, format, data, size, sample_rate));

	return id;
}

//-----------------------------------------------------------------------------
void ALRenderer::destroy_buffer(SoundBufferId id)
{
	CE_ASSERT(m_buffers_id_table.has(id), "SoundBuffer does not exist");

	SoundBuffer& al_buffer = m_buffers[id.index];

	AL_CHECK(alDeleteBuffers(1, &al_buffer.id));

	m_buffers_id_table.destroy(id);
}


//-----------------------------------------------------------------------------
SoundSourceId ALRenderer::create_source()
{
	SoundSourceId id = m_sources_id_table.create();

	SoundSource& al_source = m_sources[id.index];

	// Creates AL source
	AL_CHECK(alGenSources(1, &al_source.id));

	AL_CHECK(alSourcef(al_source.id, AL_PITCH, 1.0f));

	AL_CHECK(alSourcef(al_source.id, AL_REFERENCE_DISTANCE, 0.1f));

	AL_CHECK(alSourcef(al_source.id, AL_MAX_DISTANCE, 1000.0f));

	return id;
}

//-----------------------------------------------------------------------------
SoundSourceId ALRenderer::create_loop_source()
{
	SoundSourceId id = m_sources_id_table.create();

	SoundSource& al_source = m_sources[id.index];

	// Creates AL source
	AL_CHECK(alGenSources(1, &al_source.id));

	AL_CHECK(alSourcef(al_source.id, AL_PITCH, 1.0f));

	AL_CHECK(alSourcef(al_source.id, AL_REFERENCE_DISTANCE, 0.1f));

	AL_CHECK(alSourcef(al_source.id, AL_MAX_DISTANCE, 1000.0f));

	AL_CHECK(alSourcef(al_source.id, AL_LOOPING, AL_TRUE));

	return id;
}


//-----------------------------------------------------------------------------
void ALRenderer::play_source(SoundSourceId sid, SoundBufferId bid)
{
	CE_ASSERT(m_sources_id_table.has(sid), "SoundSource does not exist");
	CE_ASSERT(m_buffers_id_table.has(bid), "SoundBuffer does not exist");

	SoundSource& al_source = m_sources[sid.index];
	SoundBuffer& al_buffer = m_buffers[bid.index];

	alSourcei(al_source.id, AL_BUFFER, al_buffer.id);

	AL_CHECK(alSourcePlay(al_source.id));
}

//-----------------------------------------------------------------------------
void ALRenderer::pause_source(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& al_source = m_sources[id.index];

	if (source_playing(id))
	{
		AL_CHECK(alSourcePause(al_source.id));
	}
}

//-----------------------------------------------------------------------------
void ALRenderer::destroy_source(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& al_source = m_sources[id.index];

	alDeleteSources(1, &al_source.id);

	m_sources_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
void ALRenderer::set_source_min_distance(SoundSourceId id, const float min_distance)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& al_source = m_sources[id.index];

	AL_CHECK(alSourcef(al_source.id, AL_REFERENCE_DISTANCE, min_distance));
}

//-----------------------------------------------------------------------------
void ALRenderer::set_source_max_distance(SoundSourceId id, const float max_distance)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& al_source = m_sources[id.index];

	AL_CHECK(alSourcef(al_source.id, AL_MAX_DISTANCE, max_distance));
}

//-----------------------------------------------------------------------------
void ALRenderer::set_source_position(SoundSourceId id, const Vec3& pos)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& al_source = m_sources[id.index];

	AL_CHECK(alSource3f(al_source.id, AL_POSITION, pos.x, pos.y, pos.z));
}

//-----------------------------------------------------------------------------
void ALRenderer::set_source_velocity(SoundSourceId id, const Vec3& vel)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& al_source = m_sources[id.index];

	AL_CHECK(alSource3f(al_source.id, AL_VELOCITY, vel.x, vel.y, vel.z));
}

//-----------------------------------------------------------------------------
void ALRenderer::set_source_direction(SoundSourceId id, const Vec3& dir)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& al_source = m_sources[id.index];

	AL_CHECK(alSource3f(al_source.id, AL_DIRECTION, dir.x, dir.y, dir.z));
}

void ALRenderer::set_source_pitch(SoundSourceId id, const float pitch)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& al_source = m_sources[id.index];

	AL_CHECK(alSourcef(al_source.id, AL_PITCH, pitch));		
}

//-----------------------------------------------------------------------------
void ALRenderer::set_source_gain(SoundSourceId id, const float gain)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& al_source = m_sources[id.index];

	AL_CHECK(alSourcef(al_source.id, AL_GAIN, gain));	
}

//-----------------------------------------------------------------------------
void ALRenderer::set_source_rolloff(SoundSourceId id, const float rolloff)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& al_source = m_sources[id.index];

	AL_CHECK(alSourcef(al_source.id, AL_ROLLOFF_FACTOR, rolloff));		
}

//-----------------------------------------------------------------------------
bool ALRenderer::source_playing(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& al_source = m_sources[id.index];

	ALint source_state;
	alGetSourcei(al_source.id, AL_SOURCE_STATE, &source_state);

	return source_state == AL_PLAYING;
}

//-----------------------------------------------------------------------------
void ALRenderer::create_stream(const void* data, const uint32_t size, const uint32_t sample_rate, 
	const uint32_t channels, const uint32_t bxs, SoundBufferId* ids)
{
	ov_callbacks callbacks;

	callbacks.read_func 	= ogg_buffer_read;
	callbacks.seek_func 	= ogg_buffer_seek;
	callbacks.close_func	= ogg_buffer_close;
	callbacks.tell_func 	= ogg_buffer_tell;

	OggBuffer buffer((char*)data, size);

	OggVorbis_File stream;

	int32_t ret;

	ret = ov_open_callbacks((void *)&buffer, &stream, NULL, -1, callbacks);
	CE_ASSERT(ret == 0, "Unable to open custom callbacks");

	(void)sample_rate;
	(void)channels;
	(void)bxs;

	SoundBufferId id1 = m_buffers_id_table.create();
	SoundBufferId id2 = m_buffers_id_table.create();

	SoundBuffer& al_buffer1 = m_buffers[id1.index];
	SoundBuffer& al_buffer2 = m_buffers[id2.index];

	// Generates first AL buffer
	AL_CHECK(alGenBuffers(1, &al_buffer1.id));
	AL_CHECK(alGenBuffers(1, &al_buffer2.id));

	ids[0] = id1;
	ids[1] = id2;
}

//-----------------------------------------------------------------------------
void ALRenderer::play_source(SoundSourceId sid, SoundBufferId* bids)
{
	CE_ASSERT(m_sources_id_table.has(sid), "SoundSource does not exist");
	CE_ASSERT(m_buffers_id_table.has(bids[0]), "SoundBuffer does not exist");
	CE_ASSERT(m_buffers_id_table.has(bids[1]), "SoundBuffer does not exist");

	SoundSource& al_source = m_sources[sid.index];

	SoundBuffer& al_buffer1 = m_buffers[bids[0].index];
	SoundBuffer& al_buffer2 = m_buffers[bids[1].index];

	ALuint buffers[] = {al_buffer1.id, al_buffer2.id};

	alSourceQueueBuffers(source, 2, buffers);

	AL_CHECK(alSourcePlay(al_source.id));
}


} // namespace crown