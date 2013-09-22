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

#include "SoundResource.h"
#include "OggDecoder.h"
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
struct Sound
{
public:
	//-----------------------------------------------------------------------------
	Sound() : m_res(NULL), m_id(-1), m_looping(false) {}

	//-----------------------------------------------------------------------------
	void create(SoundResource* resource)
	{
		CE_ASSERT_NOT_NULL(resource);

		// Stores resource pointer
		m_res = resource;

		// Generates AL source
		AL_CHECK(alGenSources(1, &m_id));

		AL_CHECK(alSourcef(m_id, AL_PITCH, 1.0f));
		AL_CHECK(alSourcef(m_id, AL_REFERENCE_DISTANCE, 0.1f));
		AL_CHECK(alSourcef(m_id, AL_MAX_DISTANCE, 1000.0f));

		// Generates AL buffers
		AL_CHECK(alGenBuffers(3, m_buffer));
		
		bool stereo = (m_res->channels() > 1);

		switch(m_res->bits_ps())
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

		m_streaming = m_res->sound_type() == ST_OGG;
		// Streams resource if is ogg 
		if (m_streaming)
		{
			create_stream();
		}
		else
		{
			AL_CHECK(alBufferData(m_buffer[0], m_format, m_res->data(), m_res->size(), m_res->sample_rate()));

			AL_CHECK(alSourceQueueBuffers(m_id, 1, &m_buffer[0]));
		}
	}

	//-----------------------------------------------------------------------------
	void update()
	{
		if (m_playing)
		{
			if (m_streaming)
			{
				update_stream();
			}
			// else ... nothing right now
		}
	}

	//-----------------------------------------------------------------------------
	void destroy()
	{
		AL_CHECK(alDeleteSources(1, &m_id));

		AL_CHECK(alDeleteBuffers(3, m_buffer));
	}

	//-----------------------------------------------------------------------------
	void play()
	{
		AL_CHECK(alSourcePlay(m_id));

		m_playing = true;
	}

	//-----------------------------------------------------------------------------
	void pause()
	{
		AL_CHECK(alSourcePause(m_id));

		m_playing = false;
	}

	//-----------------------------------------------------------------------------
	void loop(bool loop)
	{
		if (loop && !m_streaming)
		{
			AL_CHECK(alSourcei(m_id, AL_LOOPING, AL_TRUE));
		}
		else if (!loop && !m_streaming)
		{
			AL_CHECK(alSourcei(m_id, AL_LOOPING, AL_FALSE));
		}

		m_looping = loop;
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
	float min_distance() const
	{
		ALfloat min_distance;

		alGetSourcef(m_id, AL_REFERENCE_DISTANCE, &min_distance);

		return min_distance;
	}

	//-----------------------------------------------------------------------------
	float max_distance() const
	{
		ALfloat max_distance;

		alGetSourcef(m_id, AL_MAX_DISTANCE, &max_distance);

		return max_distance;
	}

	//-----------------------------------------------------------------------------
	Vec3 position() const
	{
		ALfloat tmp[3];
		alGetSourcefv(m_id, AL_POSITION, tmp);

		Vec3 pos;
		pos.x = tmp[0];
		pos.y = tmp[1];
		pos.z = tmp[2];

		return pos;
	}

	//-----------------------------------------------------------------------------
	Vec3 velocity() const
	{
		ALfloat tmp[3];
		alGetSourcefv(m_id, AL_VELOCITY, tmp);

		Vec3 vel;
		vel.x = tmp[0];
		vel.y = tmp[1];
		vel.z = tmp[2];

		return vel;
	}

	//-----------------------------------------------------------------------------
	Vec3 direction() const
	{
		ALfloat tmp[3];
		alGetSourcefv(m_id, AL_DIRECTION, tmp);

		Vec3 dir;
		dir.x = tmp[0];
		dir.y = tmp[1];
		dir.z = tmp[2];

		return dir;	
	}

	//-----------------------------------------------------------------------------
	float pitch() const
	{
		ALfloat pitch;
		alGetSourcef(m_id, AL_PITCH, &pitch);

		return pitch;
	}

	//-----------------------------------------------------------------------------
	float gain() const
	{
		ALfloat gain;
		alGetSourcef(m_id, AL_GAIN, &gain);

		return gain;
	}

	//-----------------------------------------------------------------------------
	float rolloff() const
	{
		ALfloat rolloff;
		alGetSourcef(m_id, AL_ROLLOFF_FACTOR, &rolloff);

		return rolloff;
	}

	//-----------------------------------------------------------------------------
	bool is_playing() const
	{
		ALint source_state;
		alGetSourcei(m_id, AL_SOURCE_STATE, &source_state);

		return source_state == AL_PLAYING;
	}

	//-----------------------------------------------------------------------------
	int32_t queued_buffers()
	{
		int32_t queued;
		alGetSourcei(m_id, AL_BUFFERS_QUEUED, &queued);

		return queued;
	}

	//-----------------------------------------------------------------------------
	int32_t processed_buffers()
	{
		int32_t processed;
		alGetSourcei(m_id, AL_BUFFERS_PROCESSED, &processed);

		return processed;
	}

private:

	//-----------------------------------------------------------------------------
	void create_stream()
	{
		m_decoder.init((char*)m_res->data(), m_res->size());
		m_decoder.stream();
		AL_CHECK(alBufferData(m_buffer[0], m_format, m_decoder.data(), m_decoder.size(), m_res->sample_rate()));
		m_decoder.stream();
		AL_CHECK(alBufferData(m_buffer[1], m_format, m_decoder.data(), m_decoder.size(), m_res->sample_rate()));			
		m_decoder.stream();
		AL_CHECK(alBufferData(m_buffer[2], m_format, m_decoder.data(), m_decoder.size(), m_res->sample_rate()));

		AL_CHECK(alSourceQueueBuffers(m_id, 3, m_buffer));
	}

	//-----------------------------------------------------------------------------
	void update_stream()
	{
		uint32_t processed = processed_buffers();

		while (processed--)
		{
			ALuint buffer;

			AL_CHECK(alSourceUnqueueBuffers(m_id, 1, &buffer));

			if (m_decoder.stream())
			{
				AL_CHECK(alBufferData(buffer, m_format, m_decoder.data(), m_decoder.size(), m_res->sample_rate()));
			}
			else if (m_looping)
			{
				m_decoder.rewind();
				m_decoder.stream();
				AL_CHECK(alBufferData(buffer, m_format, m_decoder.data(), m_decoder.size(), m_res->sample_rate()));
			}

			AL_CHECK(alSourceQueueBuffers(m_id, 1, &buffer));
		}
	}

public:

	SoundResource*	m_res;

	ALuint			m_id;
	ALuint			m_buffer[3];
	ALuint 			m_format;

	bool			m_playing :1;
	bool			m_looping :1;
	bool			m_streaming :1;

	OggDecoder		m_decoder;
};

} // namespace crown