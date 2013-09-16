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
#include <cstring>

#include "Types.h"
#include "Assert.h"
#include "SoundRenderer.h"
#include "SLESRenderer.h"
#include "Vec3.h"

namespace crown
{

uint16_t* 	loop_data;
size_t		loop_size;

//-----------------------------------------------------------------------------
void check_sles_errors(SLresult result)
{
	CE_ASSERT(result == SL_RESULT_SUCCESS, "SL_ERROR_CODE: %d", result);
}

//-----------------------------------------------------------------------------
void source_callback(SLAndroidSimpleBufferQueueItf caller, void* source)
{
	(void)caller;

	SoundSource* s = (SoundSource*)source;
	
	if (s->m_loop)
	{	// FIXME FIXME FIXME
		(*s->m_player_bufferqueue)->Enqueue(s->m_player_bufferqueue, loop_data, loop_size);
	}
}

//-----------------------------------------------------------------------------
class SoundRendererBackend
{
public:
	//-----------------------------------------------------------------------------
	void init()
	{
		SLresult result;

	    const SLInterfaceID ids[] = {SL_IID_ENGINE};
	    const SLboolean reqs[] = {SL_BOOLEAN_TRUE};

		result = slCreateEngine(&m_engine_obj, 0, NULL, 1, ids, reqs);
		result = (*m_engine_obj)->Realize(m_engine_obj, SL_BOOLEAN_FALSE);

		result = (*m_engine_obj)->GetInterface(m_engine_obj, SL_IID_ENGINE, &m_engine);

	    const SLInterfaceID ids1[] = {SL_IID_VOLUME};
	    const SLboolean reqs1[] = {SL_BOOLEAN_FALSE};

	    result = (*m_engine)->CreateOutputMix(m_engine, &m_out_mix_obj, 1, ids1, reqs1); 
	    result = (*m_out_mix_obj)->Realize(m_out_mix_obj, SL_BOOLEAN_FALSE);
	}

	//-----------------------------------------------------------------------------
	void shutdown()
	{
		if (m_out_mix_obj)
		{
			(*m_out_mix_obj)->Destroy(m_out_mix_obj);
			m_out_mix_obj = NULL;
		}

		if (m_engine_obj)
		{
			(*m_engine_obj)->Destroy(m_engine_obj);
			m_engine_obj = NULL;
			m_engine = NULL;
		}
	}

	//-----------------------------------------------------------------------------
	void create_source(SoundSource& s)
	{
		SLresult result;

		int32_t speakers = SL_SPEAKER_FRONT_CENTER;

		// Configures buffer queue
	    SLDataLocator_AndroidSimpleBufferQueue buffer_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

		// Configures audio format
	    SLDataFormat_PCM format_pcm;
		format_pcm.formatType = SL_DATAFORMAT_PCM;
		format_pcm.numChannels = 1;	// Mono
		format_pcm.samplesPerSec = SL_SAMPLINGRATE_44_1;
		format_pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
		format_pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
		format_pcm.channelMask = speakers;
		format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

	    // Configures audio source
	    SLDataSource audio_source;
	    audio_source.pLocator = &buffer_queue;
	    audio_source.pFormat = &format_pcm;

		// Configures audio output mix
	 	SLDataLocator_OutputMix out_mix;
	 	out_mix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
	 	out_mix.outputMix = m_out_mix_obj;

	 	// Configures audio sink
	    SLDataSink audio_sink;
	 	audio_sink.pLocator = &out_mix;
	 	audio_sink.pFormat = NULL;

		// Creates sound player
		const SLInterfaceID ids[] = {SL_IID_PLAY, SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
		const SLboolean reqs[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

		result = (*m_engine)->CreateAudioPlayer(m_engine, &s.m_player_obj, &audio_source, &audio_sink, 3, ids, reqs);
		check_sles_errors(result);

		result = (*s.m_player_obj)->Realize(s.m_player_obj, SL_BOOLEAN_FALSE);
		check_sles_errors(result);

		// Gets interfaces
		result = (*s.m_player_obj)->GetInterface(s.m_player_obj, SL_IID_PLAY, &s.m_player_play);
		check_sles_errors(result);

		result = (*s.m_player_obj)->GetInterface(s.m_player_obj, SL_IID_BUFFERQUEUE, &s.m_player_bufferqueue);
		check_sles_errors(result);

		result = (*s.m_player_obj)->GetInterface(s.m_player_obj, SL_IID_VOLUME, &s.m_player_volume);
		check_sles_errors(result);

		(*s.m_player_bufferqueue)->RegisterCallback(s.m_player_bufferqueue, source_callback, &s);
	}

	//-----------------------------------------------------------------------------
	void destroy_source(SoundSource& s)
	{
		if (s.m_player_obj)
		{
			SLuint32 state;
			(*s.m_player_obj)->GetState(s.m_player_obj, &state);

			if (state == SL_OBJECT_STATE_REALIZED)
			{
				(*s.m_player_bufferqueue)->Clear(s.m_player_bufferqueue);
				(*s.m_player_obj)->AbortAsyncOperation(s.m_player_obj);
				(*s.m_player_obj)->Destroy(s.m_player_obj);

	  			s.m_player_obj = NULL;
	  			s.m_player_play = NULL;
	  			s.m_player_bufferqueue = NULL;
	  			s.m_player_volume = NULL;
			}
		}
	}

private:

	SLObjectItf						m_engine_obj;
	SLEngineItf						m_engine;

	SLObjectItf 					m_out_mix_obj;

	SoundBuffer 					m_buffers[MAX_SOUND_BUFFERS];
	SoundSource 					m_sources[MAX_SOUND_SOURCES];

private:

	friend class SoundRenderer;
};

//-----------------------------------------------------------------------------
SoundRenderer::SoundRenderer(Allocator& allocator) :
	m_allocator(allocator),
	m_buffers_id_table(default_allocator(), MAX_SOUND_BUFFERS),
	m_sources_id_table(default_allocator(), MAX_SOUND_SOURCES)	
{
	m_backend = CE_NEW(default_allocator(), SoundRendererBackend);
}

//-----------------------------------------------------------------------------
SoundRenderer::~SoundRenderer()
{
	if (m_backend)
	{
		CE_DELETE(default_allocator(), m_backend);
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
void SoundRenderer::set_listener(const Vec3& pos, const Vec3& vel, const Vec3& or_up, const Vec3& or_at) const
{
	Log::w("Stub");

	(void)pos;
	(void)vel;
	(void)or_up;
	(void)or_at;
}

//-----------------------------------------------------------------------------
SoundBufferId SoundRenderer::create_buffer(const void* data, const uint32_t size, const uint32_t sample_rate, const uint32_t channels, const uint32_t bits)
{
	SoundBufferId id = m_buffers_id_table.create();

	m_backend->m_buffers[id.index].create(data, size, sample_rate, channels, bits);

	return id;
}

//-----------------------------------------------------------------------------
void SoundRenderer::destroy_buffer(SoundBufferId id)
{
	CE_ASSERT(m_buffers_id_table.has(id), "SoundBuffer does not exist");

	m_backend->m_buffers[id.index].destroy();

	m_buffers_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
void SoundRenderer::bind_buffer_to_source(SoundBufferId bid, SoundSourceId sid)
{
	CE_ASSERT(m_buffers_id_table.has(bid), "SoundBuffer does not exist");
	CE_ASSERT(m_sources_id_table.has(sid), "SoundSource does not exist");

	SoundBuffer& b = m_backend->m_buffers[bid.index];
	SoundSource& s = m_backend->m_sources[sid.index];

	int32_t result = (*s.m_player_bufferqueue)->Enqueue(s.m_player_bufferqueue, b.m_data, b.m_size);
	check_sles_errors(result);

	if (s.m_loop)
	{
		loop_data = b.m_data;
		loop_size = b.m_size;
	}
}

//-----------------------------------------------------------------------------
SoundSourceId SoundRenderer::create_source(bool loop)
{
	SoundSourceId id = m_sources_id_table.create();

	SoundSource& sl_source = m_backend->m_sources[id.index];

	m_backend->create_source(sl_source);

	sl_source.m_loop = loop;

	return id;
}

//-----------------------------------------------------------------------------
void SoundRenderer::play_source(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& s = m_backend->m_sources[id.index];

	SLresult result = (*s.m_player_play)->SetPlayState(s.m_player_play, SL_PLAYSTATE_PLAYING);

	check_sles_errors(result);
}

//-----------------------------------------------------------------------------
void SoundRenderer::pause_source(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& s = m_backend->m_sources[id.index];

	SLresult result = (*s.m_player_play)->SetPlayState(s.m_player_play, SL_PLAYSTATE_PAUSED);

	check_sles_errors(result);
}

//-----------------------------------------------------------------------------
void SoundRenderer::destroy_source(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& s = m_backend->m_sources[id.index];

	m_backend->destroy_source(s);

	m_sources_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_source_min_distance(SoundSourceId id,  const float min_distance)
{
	Log::w("Stub");

	(void)id;
	(void)min_distance;
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_source_max_distance(SoundSourceId id,  const float max_distance)
{
	Log::w("Stub");

	(void)id;
	(void)max_distance;
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_source_position(SoundSourceId id, const Vec3& pos)
{
	Log::w("Stub");

	(void)id;
	(void)pos;
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_source_velocity(SoundSourceId id, const Vec3& vel)
{
	Log::w("Stub");

	(void)id;
	(void)vel;
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_source_direction(SoundSourceId id, const Vec3& dir)
{
	Log::w("Stub");

	(void)id;
	(void)dir;
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_source_pitch(SoundSourceId id, const float pitch)
{
	Log::w("Stub");

	(void)id;
	(void)pitch;	
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_source_gain(SoundSourceId id, const float gain)
{
	Log::w("Stub");

	(void)id;
	(void)gain;
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_source_rolloff(SoundSourceId id, const float rolloff)
{
	Log::w("Stub");

	(void)id;
	(void)rolloff;
}

//-----------------------------------------------------------------------------
float SoundRenderer::source_min_distance(SoundSourceId id) const
{
	Log::w("Stub");

	(void)id;

	return 0;
}

//-----------------------------------------------------------------------------
float SoundRenderer::source_max_distance(SoundSourceId id) const
{
	Log::w("Stub");

	(void)id;

	return 0;
}

//-----------------------------------------------------------------------------
Vec3 SoundRenderer::source_position(SoundSourceId id) const
{
	Log::w("Stub");

	(void)id;

	return Vec3::ZERO;
}

//-----------------------------------------------------------------------------
Vec3 SoundRenderer::source_velocity(SoundSourceId id) const
{
	Log::w("Stub");

	(void)id;

	return Vec3::ZERO;
}

//-----------------------------------------------------------------------------
Vec3 SoundRenderer::source_direction(SoundSourceId id) const
{
	Log::w("Stub");

	(void)id;

	return Vec3::ZERO;
}

//-----------------------------------------------------------------------------
float SoundRenderer::source_pitch(SoundSourceId id) const
{
	Log::w("Stub");

	(void)id;

	return 0;
}

//-----------------------------------------------------------------------------
float SoundRenderer::source_gain(SoundSourceId id) const
{
	Log::w("Stub");

	(void)id;

	return 0;
}

//-----------------------------------------------------------------------------
float SoundRenderer::source_rolloff(SoundSourceId id) const
{
	Log::w("Stub");

	(void)id;

	return 0;
}

//-----------------------------------------------------------------------------
bool SoundRenderer::source_playing(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& s = m_backend->m_sources[id.index];

    SLuint32 state;

    (*s.m_player_play)->GetPlayState(s.m_player_play, &state);

    return state == SL_PLAYSTATE_PLAYING;
}	

} // namespace crown