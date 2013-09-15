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

#pragma once

#include <AL/al.h>
#include <AL/alc.h>

#include "Vec3.h"
#include "Log.h"

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
struct SoundBuffer
{
	//-----------------------------------------------------------------------------
	void create(const void* data, const uint32_t size, const uint32_t sample_rate, const uint32_t channels, const uint32_t bits)
	{
		AL_CHECK(alGenBuffers(1, &m_id));

		m_sample_rate = sample_rate;
		
		bool stereo = (channels > 1);

		switch(bits)
		{
			case 8:
			{
				if (stereo)	
					m_format = AL_FORMAT_STEREO8;
				else
					m_format = AL_FORMAT_MONO8;
				break;
			}
			case 16:
			{
				if (stereo)
					m_format = AL_FORMAT_STEREO16;
				else
					m_format = AL_FORMAT_MONO16;
				break;
			}
			default:
			{
				CE_ASSERT(false, "Wrong number of bits per sample.");
			}
		}

		AL_CHECK(alBufferData(m_id, m_format, data, size, m_sample_rate));
	}

	//-----------------------------------------------------------------------------
	void update(const void* data, const uint32_t size)
	{
		AL_CHECK(alBufferData(m_id, m_format, data, size, m_sample_rate));
	}

	//-----------------------------------------------------------------------------
	void destroy()
	{
		AL_CHECK(alDeleteBuffers(1, &m_id));
	}

public:

	ALuint 		m_id;
	ALenum 		m_format;
	ALuint 		m_sample_rate;
};

//-----------------------------------------------------------------------------
struct SoundSource
{
	//-----------------------------------------------------------------------------	
	void create(bool loop)
	{
		AL_CHECK(alGenSources(1, &m_id));
		AL_CHECK(alSourcef(m_id, AL_PITCH, 1.0f));
		AL_CHECK(alSourcef(m_id, AL_REFERENCE_DISTANCE, 0.1f));
		AL_CHECK(alSourcef(m_id, AL_MAX_DISTANCE, 1000.0f));

		if (loop)
		{
			AL_CHECK(alSourcef(m_id, AL_LOOPING, AL_TRUE));
		}
	}

	//-----------------------------------------------------------------------------
	void bind_buffer(ALuint buffers)
	{
		// AL_CHECK(alSourcei(m_id, AL_BUFFER, buffer));
		alSourceQueueBuffers(m_id, 1, &buffers);
	}

	//-----------------------------------------------------------------------------
	ALuint unbind_buffer()
	{
        ALuint buffer;

        alSourceUnqueueBuffers(m_id, 1, &buffer);

        return buffer;
	}

	//-----------------------------------------------------------------------------
	void destroy()
	{
		AL_CHECK(alDeleteSources(1, &m_id));
	}

	//-----------------------------------------------------------------------------
	void play()
	{
		AL_CHECK(alSourcePlay(m_id));
	}

	//-----------------------------------------------------------------------------
	void pause()
	{
		if (is_playing())
		{
			AL_CHECK(alSourcePause(m_id));
		}
	}

	//-----------------------------------------------------------------------------
	void set_min_distance(const float min_distance)
	{
		AL_CHECK(alSourcef(m_id, AL_REFERENCE_DISTANCE, min_distance));
	}

	//-----------------------------------------------------------------------------
	void set_max_distance( const float max_distance)
	{
		AL_CHECK(alSourcef(m_id, AL_MAX_DISTANCE, max_distance));
	}

	//-----------------------------------------------------------------------------
	void set_position(const Vec3& pos)
	{
		AL_CHECK(alSource3f(m_id, AL_POSITION, pos.x, pos.y, pos.z));
	}

	//-----------------------------------------------------------------------------
	void set_velocity(const Vec3& vel)
	{
		AL_CHECK(alSource3f(m_id, AL_VELOCITY, vel.x, vel.y, vel.z));
	}

	//-----------------------------------------------------------------------------
	void set_direction(const Vec3& dir)
	{
		AL_CHECK(alSource3f(m_id, AL_DIRECTION, dir.x, dir.y, dir.z));
	}

	//-----------------------------------------------------------------------------
	void set_pitch(const float pitch)
	{
		AL_CHECK(alSourcef(m_id, AL_PITCH, pitch));
	}

	//-----------------------------------------------------------------------------
	void set_gain(const float gain)
	{
		AL_CHECK(alSourcef(m_id, AL_GAIN, gain));
	}

	//-----------------------------------------------------------------------------
	void set_rolloff(const float rolloff)
	{
		AL_CHECK(alSourcef(m_id, AL_ROLLOFF_FACTOR, rolloff));
	}

	//-----------------------------------------------------------------------------
	bool is_playing() const
	{
		ALint source_state;
		alGetSourcei(m_id, AL_SOURCE_STATE, &source_state);

		return source_state == AL_PLAYING;
	}

public:

	ALuint		m_id;

	uint32_t	m_num_buffers;
};

} // namespace crown