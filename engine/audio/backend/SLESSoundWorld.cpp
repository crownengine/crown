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

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

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
#include "Queue.h"

namespace crown
{

//-----------------------------------------------------------------------------
static const char* sles_error_to_string(SLresult result)
{
	switch (result)
	{
		case SL_RESULT_SUCCESS: return "SL_RESULT_SUCCESS";
		case SL_RESULT_PARAMETER_INVALID: return "SL_RESULT_PARAMETER_INVALID";
		case SL_RESULT_MEMORY_FAILURE: return "SL_RESULT_MEMORY_FAILURE";
		case SL_RESULT_FEATURE_UNSUPPORTED: return "SL_RESULT_FEATURE_UNSUPPORTED";
		case SL_RESULT_RESOURCE_ERROR: return "SL_RESULT_RESOURCE_ERROR";
		case SL_RESULT_IO_ERROR: return "SL_RESULT_IO_ERROR";
		case SL_RESULT_PRECONDITIONS_VIOLATED: return "SL_RESULT_PRECONDITIONS_VIOLATED";
		case SL_RESULT_CONTENT_CORRUPTED: return "SL_RESULT_CONTENT_CORRUPTED";
		case SL_RESULT_CONTENT_UNSUPPORTED: return "SL_RESULT_CONTENT_UNSUPPORTED";
		case SL_RESULT_CONTENT_NOT_FOUND: return "SL_RESULT_CONTENT_NOT_FOUND";
		case SL_RESULT_PERMISSION_DENIED: return "SL_RESULT_PERMISSION_DENIED";
		case SL_RESULT_BUFFER_INSUFFICIENT: return "SL_RESULT_BUFFER_INSUFFICIENT";
		default: return "UNKNOWN_SL_ERROR";
	}
}

//-----------------------------------------------------------------------------
#if defined(CROWN_DEBUG) || defined(CROWN_DEVELOPMENT)
	#define SL_CHECK(function)\
		do { SLresult result = function;\
				CE_ASSERT(result == SL_RESULT_SUCCESS, "OpenSL|ES error: %s", sles_error_to_string(result)); } while (0)
#else
	#define SL_CHECK(function) function;
#endif

namespace audio_system
{
	static SLObjectItf s_sl_engine;
	static SLEngineItf s_sl_engine_itf;
	static SLObjectItf s_sl_output_mix;

	void init()
	{
		const SLInterfaceID ids[] = {SL_IID_ENGINE};
		const SLboolean reqs[] = {SL_BOOLEAN_TRUE};
		const SLEngineOption opts[] = { (SLuint32) SL_ENGINEOPTION_THREADSAFE, (SLuint32) SL_BOOLEAN_TRUE };

		// Create OpenSL engine
		SL_CHECK(slCreateEngine(&s_sl_engine, 1, opts, 1, ids, reqs));
		SL_CHECK((*s_sl_engine)->Realize(s_sl_engine, SL_BOOLEAN_FALSE));

		// Obtain OpenSL engine interface
		SL_CHECK((*s_sl_engine)->GetInterface(s_sl_engine, SL_IID_ENGINE, &s_sl_engine_itf));

		// Create global output mix
		const SLInterfaceID ids1[] = {SL_IID_VOLUME};
		const SLboolean reqs1[] = {SL_BOOLEAN_FALSE};

		SL_CHECK((*s_sl_engine_itf)->CreateOutputMix(s_sl_engine_itf, &s_sl_output_mix, 1, ids1, reqs1)); 
		SL_CHECK((*s_sl_output_mix)->Realize(s_sl_output_mix, SL_BOOLEAN_FALSE));
	}

	void shutdown()
	{
		(*s_sl_output_mix)->Destroy(s_sl_output_mix);
		(*s_sl_engine)->Destroy(s_sl_engine);
	}
} // namespace audio_system

namespace sles_sound_world
{
	// Queue of instances to stop at next update()
	static Queue<SoundInstanceId>* s_stop_queue = NULL;

	void init()
	{
		s_stop_queue = CE_NEW(default_allocator(), Queue<SoundInstanceId>)(default_allocator());
	}

	void shutdown()
	{
		CE_DELETE(default_allocator(), s_stop_queue);
	}

	static void player_callback(SLPlayItf caller, void* context, SLuint32 event)
	{
		SoundInstanceId id;
		id.decode((uint32_t) context);

		s_stop_queue->push_back(id);
	}

	static SLmillibel gain_to_attenuation(SLVolumeItf vol_itf, float volume)
	{
		SLmillibel volume_mb;
		if (volume <= 0.02f) return SL_MILLIBEL_MIN;
		else if (volume >= 1.0f)
		{
			(*vol_itf)->GetMaxVolumeLevel(vol_itf, &volume_mb);
			return volume_mb;
		}

		volume_mb = M_LN2 / log(1.0f / (1.0f - volume)) * -1000.0f;
		if (volume_mb > 0) volume_mb = SL_MILLIBEL_MIN;
		return volume_mb;
	}
}

//-----------------------------------------------------------------------------
struct SoundInstance
{
	void create(SLEngineItf engine, SLObjectItf output_mix, SoundInstanceId id, SoundResource* sr)
	{
		m_resource = sr;
		m_finished = false;
		m_id = id;

		// Configures buffer queue
		SLDataLocator_AndroidSimpleBufferQueue buffer_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

		// Configures audio format
		SLDataFormat_PCM format_pcm;
		format_pcm.formatType = SL_DATAFORMAT_PCM;

		// Sets channels
		switch (sr->channels())
		{
			case 1:
			{
				format_pcm.numChannels = 1;
				format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
				break;
			}
			case 2:
			{
				format_pcm.numChannels = 2;
				format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
				break;
			}
			default:
			{
				CE_FATAL("Oops, wrong number of channels");
				break;
			}
		}

		// Sets sample rate
		switch (sr->sample_rate())
		{
			case 8000: format_pcm.samplesPerSec = SL_SAMPLINGRATE_8; break;
			case 11025: format_pcm.samplesPerSec = SL_SAMPLINGRATE_11_025; break;
			case 16000: format_pcm.samplesPerSec = SL_SAMPLINGRATE_16; break;
			case 22050: format_pcm.samplesPerSec = SL_SAMPLINGRATE_22_05; break;
			case 24000: format_pcm.samplesPerSec = SL_SAMPLINGRATE_24; break;
			case 32000: format_pcm.samplesPerSec = SL_SAMPLINGRATE_32; break;
			case 44100: format_pcm.samplesPerSec = SL_SAMPLINGRATE_44_1; break;
			case 48000: format_pcm.samplesPerSec = SL_SAMPLINGRATE_48; break;
			case 64000: format_pcm.samplesPerSec = SL_SAMPLINGRATE_64; break;
			case 88200: format_pcm.samplesPerSec = SL_SAMPLINGRATE_88_2; break;
			case 96000: format_pcm.samplesPerSec = SL_SAMPLINGRATE_96; break;
			case 192000: format_pcm.samplesPerSec = SL_SAMPLINGRATE_192; break;
			default: CE_FATAL("Oops, sample rate not supported"); break;
		}	

		format_pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
		format_pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
		format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

		// Configures audio source
		SLDataSource audio_source;
		audio_source.pLocator = &buffer_queue;
		audio_source.pFormat = &format_pcm;

		// Configures audio output mix
		SLDataLocator_OutputMix out_mix;
		out_mix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
		out_mix.outputMix = output_mix;

		// Configures audio sink
		SLDataSink audio_sink;
		audio_sink.pLocator = &out_mix;
		audio_sink.pFormat = NULL;

		// Creates sound player
		const SLInterfaceID ids[] = {SL_IID_PLAY, SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
		const SLboolean reqs[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

		SL_CHECK((*engine)->CreateAudioPlayer(engine, &m_player, &audio_source, &audio_sink, 3, ids, reqs));
		SL_CHECK((*m_player)->Realize(m_player, SL_BOOLEAN_FALSE));

		SL_CHECK((*m_player)->GetInterface(m_player, SL_IID_BUFFERQUEUE, &m_player_bufferqueue));

		//(*m_player_bufferqueue)->RegisterCallback(m_player_bufferqueue, SoundInstance::buffer_callback, this);
		(*play_itf())->SetCallbackEventsMask(play_itf(), SL_PLAYEVENT_HEADATEND);
		(*play_itf())->RegisterCallback(play_itf(), sles_sound_world::player_callback, (void*) id.encode());

		// Manage simple sound or stream
		// m_streaming = sr->sound_type() == SoundType::OGG;

		// if (m_streaming)
		// {
		// 	m_decoder.init((char*)sr->data(), sr->size());

		// 	m_decoder.stream();
		// 	(*m_player_bufferqueue)->Enqueue(m_player_bufferqueue, m_decoder.data(), m_decoder.size());
		// }
		// else
		{
			(*m_player_bufferqueue)->Enqueue(m_player_bufferqueue, sr->data(), sr->size());
		}
	}

	void destroy()
	{
		// if (m_streaming)
		// {
		// 	m_decoder.shutdown();
		// }
		stop();
		(*m_player_bufferqueue)->Clear(m_player_bufferqueue);
		(*m_player)->AbortAsyncOperation(m_player);
		(*m_player)->Destroy(m_player);
	}

	void reload(SoundResource* new_sr)
	{
	}

	void play(bool loop, float volume)
	{
		set_volume(volume);
		SL_CHECK((*play_itf())->SetPlayState(play_itf(), SL_PLAYSTATE_PLAYING));
	}

	void pause()
	{
		SL_CHECK((*play_itf())->SetPlayState(play_itf(), SL_PLAYSTATE_PAUSED));
	}

	void resume()
	{
		SL_CHECK((*play_itf())->SetPlayState(play_itf(), SL_PLAYSTATE_PLAYING));
	}

	void stop()
	{
		SL_CHECK((*play_itf())->SetPlayState(play_itf(), SL_PLAYSTATE_STOPPED));
	}

	bool is_playing()
	{
		SLuint32 state;
		SL_CHECK((*play_itf())->GetPlayState(play_itf(), &state));
		return state == SL_PLAYSTATE_PLAYING;	
	}

	bool finished()
	{
		return m_finished;
	}

	void set_volume(float volume)
	{
		SLVolumeItf vol;
		SL_CHECK((*m_player)->GetInterface(m_player, SL_IID_VOLUME, &vol));
		SL_CHECK((*vol)->SetVolumeLevel(vol, sles_sound_world::gain_to_attenuation(vol, volume)));
	}

	void set_range(float range)
	{
	}

	void set_position(const Vector3& pos)
	{
	}

	static void buffer_callback(SLAndroidSimpleBufferQueueItf caller, void* context)
	{
		SoundInstance* s = (SoundInstance*) context;

		// if (s->is_playing())
		// {
		// 	s->m_processed_buffers++;

		// 	if (s->m_decoder.stream())
		// 	{
		// 		(*s->m_player_bufferqueue)->Enqueue(s->m_player_bufferqueue, s->m_decoder.data(), s->m_decoder.size());
		// 	}
		// 	else if (s->m_looping)
		// 	{
		// 		s->m_decoder.rewind();
		// 		s->m_decoder.stream();
		// 		(*s->m_player_bufferqueue)->Enqueue(s->m_player_bufferqueue, s->m_decoder.data(), s->m_decoder.size());	
		// 	}
		// 	else
		// 	{
		// 		s->pause();
		// 	}
		// }
	}

	SoundResource* resource()
	{
		return m_resource;
	}

	SLPlayItf play_itf()
	{
		SLPlayItf play;
		SL_CHECK((*m_player)->GetInterface(m_player, SL_IID_PLAY, &play));
		return play;
	}

public:

	SoundInstanceId m_id;
	SoundResource* m_resource;

	SLObjectItf m_player;
	SLAndroidSimpleBufferQueueItf m_player_bufferqueue;

	uint32_t m_processed_buffers;
	bool m_finished;
	// OggDecoder m_decoder;
};

class SLESSoundWorld : public SoundWorld
{
public:

	SLESSoundWorld()
	{
		sles_sound_world::init();
	}

	virtual ~SLESSoundWorld()
	{
		sles_sound_world::shutdown();
	}

	virtual SoundInstanceId play(const char* name, bool loop, float volume, const Vector3& /*pos*/)
	{
		return play((SoundResource*) device()->resource_manager()->lookup(SOUND_EXTENSION, name), loop, volume);
	}

	SoundInstanceId play(SoundResource* sr, bool loop, float volume)
	{
		SoundInstance dummy;
		SoundInstanceId id = m_playing_sounds.create(dummy);

		SoundInstance& instance = m_playing_sounds.lookup(id);
		instance.create(audio_system::s_sl_engine_itf, audio_system::s_sl_output_mix, id, sr);
		instance.play(loop, volume);
	}

	virtual void stop(SoundInstanceId id)
	{
		SoundInstance& instance = m_playing_sounds.lookup(id);
		instance.destroy();
		m_playing_sounds.destroy(id);
	}

	virtual bool is_playing(SoundInstanceId id)
	{
		return m_playing_sounds.has(id) && m_playing_sounds.lookup(id).is_playing();
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
		m_listener_pose = pose;
	}

	virtual void update()
	{
		const uint32_t num_to_stop = sles_sound_world::s_stop_queue->size();
		for (uint32_t i = 0; i < num_to_stop; i++)
		{
			const SoundInstanceId id = sles_sound_world::s_stop_queue->front();
			sles_sound_world::s_stop_queue->pop_front();

			if (!m_playing_sounds.has(id)) continue;
			stop(id);
		}
	}

private:

	IdArray<CE_MAX_SOUND_INSTANCES, SoundInstance> m_playing_sounds;
	Matrix4x4 m_listener_pose;
};

SoundWorld* SoundWorld::create(Allocator& a)
{
	return CE_NEW(a, SLESSoundWorld)();
}

void SoundWorld::destroy(Allocator& a, SoundWorld* sw)
{
	CE_DELETE(a, sw);
}

} // namespace crown
