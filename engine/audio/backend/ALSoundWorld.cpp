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

#include <AL/al.h>
#include <AL/alc.h>

#include "SoundWorld.h"
#include "IdArray.h"
#include "Allocator.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Resource.h"
#include "Device.h"
#include "ResourceManager.h"
#include "SoundResource.h"
#include "List.h"
#include "TempAllocator.h"

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
#if defined(CROWN_DEBUG) || defined(CROWN_DEVELOPMENT)
	#define AL_CHECK(function)\
		function;\
		do { ALenum error; CE_ASSERT((error = alGetError()) == AL_NO_ERROR,\
				"OpenAL error: %s", al_error_to_string(error)); } while (0)
#else
	#define AL_CHECK(function) function;
#endif

/// Global audio-related functions
namespace audio_system
{
	static ALCdevice* s_al_device;
	static ALCcontext* s_al_context;

	void init()
	{
		s_al_device = alcOpenDevice(NULL);
		CE_ASSERT(s_al_device, "Cannot open OpenAL audio device");

		s_al_context = alcCreateContext(s_al_device, NULL);
		CE_ASSERT(s_al_context, "Cannot create OpenAL context");

		AL_CHECK(alcMakeContextCurrent(s_al_context));

		Log::d("OpenAL Vendor   : %s", alGetString(AL_VENDOR));
		Log::d("OpenAL Version  : %s", alGetString(AL_VERSION));
		Log::d("OpenAL Renderer : %s", alGetString(AL_RENDERER));

		AL_CHECK(alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED));
		AL_CHECK(alDopplerFactor(1.0f));
		AL_CHECK(alDopplerVelocity(343.0f));
	}

	void shutdown()
	{
		alcDestroyContext(s_al_context);
	    alcCloseDevice(s_al_device);
	}
}

//-----------------------------------------------------------------------------
struct SoundInstance
{
	void create(SoundResource* sr, const Vector3& pos)
	{
		AL_CHECK(alGenSources(1, &m_source));
		CE_ASSERT(alIsSource(m_source), "Bad OpenAL source");

		// AL_CHECK(alSourcef(m_source, AL_PITCH, 1.0f));
		// AL_CHECK(alSourcef(m_source, AL_REFERENCE_DISTANCE, 0.1f));
		// AL_CHECK(alSourcef(m_source, AL_MAX_DISTANCE, 1000.0f));

		// Generates AL buffers
		AL_CHECK(alGenBuffers(1, &m_buffer));
		CE_ASSERT(alIsBuffer(m_buffer), "Bad OpenAL buffer");

		ALenum format;
		switch (sr->bits_ps())
		{
			case 8: format = sr->channels() > 1 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8; break;
			case 16: format = sr->channels() > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16; break;
			default: CE_FATAL("Number of bits per sample not supported."); break;
		}
		AL_CHECK(alBufferData(m_buffer, format, sr->data(), sr->size(), sr->sample_rate()));

		m_resource = sr;
		set_position(pos);
	}

	void destroy()
	{
		stop();
		AL_CHECK(alSourcei(m_source, AL_BUFFER, 0));
		AL_CHECK(alDeleteBuffers(1, &m_buffer));
		AL_CHECK(alDeleteSources(1, &m_source));
	}

	void reload(SoundResource* new_sr)
	{
		destroy();
		create(new_sr, m_position);
	}

	void play(bool loop, float volume)
	{
		set_volume(volume);
		AL_CHECK(alSourcei(m_source, AL_LOOPING, (loop ? AL_TRUE : AL_FALSE)));
		AL_CHECK(alSourceQueueBuffers(m_source, 1, &m_buffer));
		AL_CHECK(alSourcePlay(m_source));
	}

	void pause()
	{
		AL_CHECK(alSourcePause(m_source));
	}

	void resume()
	{
		AL_CHECK(alSourcePlay(m_source));
	}

	void stop()
	{
		AL_CHECK(alSourceStop(m_source));
		AL_CHECK(alSourceRewind(m_source)); // Workaround
		ALint processed;
		AL_CHECK(alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed));

		if (processed > 0)
		{
			ALuint removed;
			AL_CHECK(alSourceUnqueueBuffers(m_source, 1, &removed));
		}
	}

	bool finished()
	{
		ALint state;
		AL_CHECK(alGetSourcei(m_source, AL_SOURCE_STATE, &state));
		return (state != AL_PLAYING && state != AL_PAUSED);
	}

	void set_position(const Vector3& pos)
	{
		AL_CHECK(alSourcefv(m_source, AL_POSITION, pos.to_float_ptr()));
		m_position = pos;
	}

	void set_range(float range)
	{
		AL_CHECK(alSourcef(m_source, AL_MAX_DISTANCE, range));
	}

	void set_volume(float volume)
	{
		AL_CHECK(alSourcef(m_source, AL_GAIN, volume));
	}

	SoundResource* resource()
	{
		return m_resource;
	}

public:

	SoundInstanceId m_id;
	SoundResource* m_resource;
	Vector3 m_position;
	ALuint m_buffer;
	ALuint m_source;
};

class ALSoundWorld : public SoundWorld
{
public:

	ALSoundWorld()
	{
		set_listener_pose(Matrix4x4::IDENTITY);
	}

	virtual ~ALSoundWorld()
	{
	}

	virtual SoundInstanceId play(const char* name, bool loop, float volume, const Vector3& pos)
	{
		return play((SoundResource*) device()->resource_manager()->lookup(SOUND_EXTENSION, name), loop, volume, pos);
	}

	SoundInstanceId play(SoundResource* sr, bool loop, float volume, const Vector3& pos)
	{
		SoundInstance instance;
		instance.create(sr, pos);
		SoundInstanceId id = m_playing_sounds.create(instance);
		m_playing_sounds.lookup(id).m_id = id;
		instance.play(loop, volume);
		return id;
	}

	virtual void stop(SoundInstanceId id)
	{
		SoundInstance& instance = m_playing_sounds.lookup(id);
		instance.destroy();
		m_playing_sounds.destroy(id);
	}

	virtual void stop_all()
	{
		for (uint32_t i = 0; i < m_playing_sounds.size(); i++)
		{
			m_playing_sounds[i].stop();
		}
	}

	virtual void pause_all()
	{
		for (uint32_t i = 0; i < m_playing_sounds.size(); i++)
		{
			m_playing_sounds[i].pause();
		}
	}

	virtual void resume_all()
	{
		for (uint32_t i = 0; i < m_playing_sounds.size(); i++)
		{
			m_playing_sounds[i].resume();
		}
	}

	virtual void set_sound_positions(uint32_t count, const SoundInstanceId* ids, const Vector3* positions)
	{
		for (uint32_t i = 0; i < m_playing_sounds.size(); i++)
		{
			m_playing_sounds[i].set_position(positions[i]);
		}
	}

	virtual void set_sound_ranges(uint32_t count, const SoundInstanceId* ids, const float* ranges)
	{
		for (uint32_t i = 0; i < m_playing_sounds.size(); i++)
		{
			m_playing_sounds[i].set_range(ranges[i]);
		}
	}

	virtual void set_sound_volumes(uint32_t count, const SoundInstanceId* ids, const float* volumes)
	{
		for (uint32_t i = 0; i < m_playing_sounds.size(); i++)
		{
			m_playing_sounds[i].set_volume(volumes[i]);
		}		
	}

	virtual void reload_sounds(SoundResource* old_sr, SoundResource* new_sr)
	{
		for (uint32_t i = 0; i < m_playing_sounds.size(); i++)
		{
			if (m_playing_sounds[i].resource() == old_sr)
			{
				m_playing_sounds[i].reload(new_sr);
			}
		}
	}

	virtual void set_listener_pose(const Matrix4x4& pose)
	{
		const Vector3 pos = pose.translation();
		const Vector3 up = pose.y();
		const Vector3 at = pose.z();

		AL_CHECK(alListener3f(AL_POSITION, pos.x, pos.y, pos.z));
		//AL_CHECK(alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z));

		const ALfloat orientation[] = { up.x, up.y, up.z, at.x, at.y, at.z };
		AL_CHECK(alListenerfv(AL_ORIENTATION, orientation));
		m_listener_pose = pose;
	}

	virtual void update()
	{
		TempAllocator256 alloc;
		List<SoundInstanceId> to_delete(alloc);

		// Check what sounds finished playing
		for (uint32_t i = 0; i < m_playing_sounds.size(); i++)
		{
			SoundInstance& instance = m_playing_sounds[i];
			if (instance.finished())
			{
				to_delete.push_back(instance.m_id);
			}
		}

		// Destroy instances which finished playing
		for (uint32_t i = 0; i < to_delete.size(); i++)
		{
			stop(to_delete[i]);
		}
	}

private:

	IdArray<MAX_SOUND_INSTANCES, SoundInstance> m_playing_sounds;
	Matrix4x4 m_listener_pose;
};

SoundWorld* SoundWorld::create(Allocator& a)
{
	return CE_NEW(a, ALSoundWorld)();
}

void SoundWorld::destroy(Allocator& a, SoundWorld* sw)
{
	CE_DELETE(a, sw);
}

} // namespace crown
