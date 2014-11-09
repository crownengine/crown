/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_SOUND_OPENAL

#include "sound_world.h"
#include "id_array.h"
#include "array.h"
#include "vector3.h"
#include "matrix4x4.h"
#include "sound_resource.h"
#include "temp_allocator.h"
#include "log.h"
#include "audio.h"
#include <AL/al.h>
#include <AL/alc.h>

namespace crown
{

#if defined(CROWN_DEBUG)
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

	#define AL_CHECK(function)\
		function;\
		do { ALenum error; CE_ASSERT((error = alGetError()) == AL_NO_ERROR,\
				"OpenAL error: %s", al_error_to_string(error)); } while (0)
#else
	#define AL_CHECK(function) function;
#endif

/// Global audio-related functions
namespace audio_globals
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

		CE_LOGD("OpenAL Vendor   : %s", alGetString(AL_VENDOR));
		CE_LOGD("OpenAL Version  : %s", alGetString(AL_VERSION));
		CE_LOGD("OpenAL Renderer : %s", alGetString(AL_RENDERER));

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

struct SoundInstance
{
	void create(SoundResource* sr, const Vector3& pos)
	{
		using namespace sound_resource;

		AL_CHECK(alGenSources(1, &_source));
		CE_ASSERT(alIsSource(_source), "Bad OpenAL source");

		// AL_CHECK(alSourcef(_source, AL_PITCH, 1.0f));
		// AL_CHECK(alSourcef(_source, AL_REFERENCE_DISTANCE, 0.1f));
		// AL_CHECK(alSourcef(_source, AL_MAX_DISTANCE, 1000.0f));

		// Generates AL buffers
		AL_CHECK(alGenBuffers(1, &_buffer));
		CE_ASSERT(alIsBuffer(_buffer), "Bad OpenAL buffer");

		ALenum format;
		switch (bits_ps(sr))
		{
			case 8: format = channels(sr) > 1 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8; break;
			case 16: format = channels(sr) > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16; break;
			default: CE_FATAL("Number of bits per sample not supported."); break;
		}
		AL_CHECK(alBufferData(_buffer, format, data(sr), size(sr), sample_rate(sr)));

		_resource = sr;
		set_position(pos);
	}

	void destroy()
	{
		stop();
		AL_CHECK(alSourcei(_source, AL_BUFFER, 0));
		AL_CHECK(alDeleteBuffers(1, &_buffer));
		AL_CHECK(alDeleteSources(1, &_source));
	}

	void reload(SoundResource* new_sr)
	{
		destroy();
		create(new_sr, position());
	}

	void play(bool loop, float volume)
	{
		set_volume(volume);
		AL_CHECK(alSourcei(_source, AL_LOOPING, (loop ? AL_TRUE : AL_FALSE)));
		AL_CHECK(alSourceQueueBuffers(_source, 1, &_buffer));
		AL_CHECK(alSourcePlay(_source));
	}

	void pause()
	{
		AL_CHECK(alSourcePause(_source));
	}

	void resume()
	{
		AL_CHECK(alSourcePlay(_source));
	}

	void stop()
	{
		AL_CHECK(alSourceStop(_source));
		AL_CHECK(alSourceRewind(_source)); // Workaround
		ALint processed;
		AL_CHECK(alGetSourcei(_source, AL_BUFFERS_PROCESSED, &processed));

		if (processed > 0)
		{
			ALuint removed;
			AL_CHECK(alSourceUnqueueBuffers(_source, 1, &removed));
		}
	}

	bool is_playing()
	{
		ALint state;
		AL_CHECK(alGetSourcei(_source, AL_SOURCE_STATE, &state));
		return state == AL_PLAYING;
	}

	bool finished()
	{
		ALint state;
		AL_CHECK(alGetSourcei(_source, AL_SOURCE_STATE, &state));
		return (state != AL_PLAYING && state != AL_PAUSED);
	}

	Vector3 position()
	{
		ALfloat pos[3];
		AL_CHECK(alGetSourcefv(_source, AL_POSITION, pos));
		return Vector3(pos[0], pos[1], pos[2]);
	}

	void set_position(const Vector3& pos)
	{
		AL_CHECK(alSourcefv(_source, AL_POSITION, vector3::to_float_ptr(pos)));
	}

	void set_range(float range)
	{
		AL_CHECK(alSourcef(_source, AL_MAX_DISTANCE, range));
	}

	void set_volume(float volume)
	{
		AL_CHECK(alSourcef(_source, AL_GAIN, volume));
	}

	SoundResource* resource()
	{
		return _resource;
	}

public:

	SoundInstanceId _id;
	SoundResource* _resource;
	ALuint _buffer;
	ALuint _source;
};

class ALSoundWorld : public SoundWorld
{
public:

	ALSoundWorld()
	{
		set_listener_pose(matrix4x4::IDENTITY);
	}

	virtual ~ALSoundWorld()
	{
	}

	virtual SoundInstanceId play(SoundResource* sr, bool loop, float volume, const Vector3& pos)
	{
		SoundInstance instance;
		instance.create(sr, pos);
		SoundInstanceId id = id_array::create(_playing_sounds, instance);
		id_array::get(_playing_sounds, id)._id = id;
		instance.play(loop, volume);
		return id;
	}

	virtual void stop(SoundInstanceId id)
	{
		SoundInstance& instance = id_array::get(_playing_sounds, id);
		instance.destroy();
		id_array::destroy(_playing_sounds, id);
	}

	virtual bool is_playing(SoundInstanceId id)
	{
		return id_array::has(_playing_sounds, id) && id_array::get(_playing_sounds, id).is_playing();
	}

	virtual void stop_all()
	{
		for (uint32_t i = 0; i < id_array::size(_playing_sounds); i++)
		{
			_playing_sounds[i].stop();
		}
	}

	virtual void pause_all()
	{
		for (uint32_t i = 0; i < id_array::size(_playing_sounds); i++)
		{
			_playing_sounds[i].pause();
		}
	}

	virtual void resume_all()
	{
		for (uint32_t i = 0; i < id_array::size(_playing_sounds); i++)
		{
			_playing_sounds[i].resume();
		}
	}

	virtual void set_sound_positions(uint32_t num, const SoundInstanceId* ids, const Vector3* positions)
	{
		for (uint32_t i = 0; i < num; i++)
		{
			id_array::get(_playing_sounds, ids[i]).set_position(positions[i]);
		}
	}

	virtual void set_sound_ranges(uint32_t num, const SoundInstanceId* ids, const float* ranges)
	{
		for (uint32_t i = 0; i < num; i++)
		{
			id_array::get(_playing_sounds, ids[i]).set_range(ranges[i]);
		}
	}

	virtual void set_sound_volumes(uint32_t num, const SoundInstanceId* ids, const float* volumes)
	{
		for (uint32_t i = 0; i < num; i++)
		{
			id_array::get(_playing_sounds, ids[i]).set_volume(volumes[i]);
		}
	}

	virtual void reload_sounds(SoundResource* old_sr, SoundResource* new_sr)
	{
		for (uint32_t i = 0; i < id_array::size(_playing_sounds); i++)
		{
			if (_playing_sounds[i].resource() == old_sr)
			{
				_playing_sounds[i].reload(new_sr);
			}
		}
	}

	virtual void set_listener_pose(const Matrix4x4& pose)
	{
		const Vector3 pos = matrix4x4::translation(pose);
		const Vector3 up = matrix4x4::y(pose);
		const Vector3 at = matrix4x4::z(pose);

		AL_CHECK(alListener3f(AL_POSITION, pos.x, pos.y, pos.z));
		//AL_CHECK(alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z));

		const ALfloat orientation[] = { up.x, up.y, up.z, at.x, at.y, at.z };
		AL_CHECK(alListenerfv(AL_ORIENTATION, orientation));
		_listener_pose = pose;
	}

	virtual void update()
	{
		TempAllocator256 alloc;
		Array<SoundInstanceId> to_delete(alloc);

		// Check what sounds finished playing
		for (uint32_t i = 0; i < id_array::size(_playing_sounds); i++)
		{
			SoundInstance& instance = _playing_sounds[i];
			if (instance.finished())
			{
				array::push_back(to_delete, instance._id);
			}
		}

		// Destroy instances which finished playing
		for (uint32_t i = 0; i < array::size(to_delete); i++)
		{
			stop(to_delete[i]);
		}
	}

private:

	IdArray<CE_MAX_SOUND_INSTANCES, SoundInstance> _playing_sounds;
	Matrix4x4 _listener_pose;
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

#endif // CROWN_SOUND_OPENAL
