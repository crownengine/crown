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

#include <cstring>

#include "Types.h"
#include "SLESRenderer.h"
#include "Assert.h"
#include "MathUtils.h"

namespace crown
{

//-----------------------------------------------------------------------------
AudioRenderer* AudioRenderer::create(Allocator& a)
{
	return CE_NEW(a, SLESRenderer);
}

//-----------------------------------------------------------------------------
void AudioRenderer::destroy(Allocator& a, AudioRenderer* renderer)
{
	CE_DELETE(a, renderer);
}

//-----------------------------------------------------------------------------
SLESRenderer::SLESRenderer() :
	m_buffers_id_table(m_allocator, MAX_BUFFERS),
	m_sources_id_table(m_allocator, MAX_SOURCES)
{
}

//-----------------------------------------------------------------------------
void SLESRenderer::init()
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
void SLESRenderer::shutdown()
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
SoundBufferId SLESRenderer::create_buffer(const void* data, const uint32_t size, const uint32_t sample_rate, const uint32_t channels, const uint32_t bxs)
{
	SoundBufferId id = m_buffers_id_table.create();

	SoundBuffer& sl_buffer = m_buffers[id.index];

	sl_buffer.data = (uint16_t*)data;
	sl_buffer.size = size;
	sl_buffer.sample_rate = sample_rate;
	sl_buffer.channels = channels;
	sl_buffer.bits_per_sample = bxs;

	return id;
}

//-----------------------------------------------------------------------------
void SLESRenderer::destroy_buffer(SoundBufferId id)
{
	CE_ASSERT(m_buffers_id_table.has(id), "SoundBuffer does not exist");

	m_buffers_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
SoundSourceId SLESRenderer::create_source()
{
	SoundSourceId id = m_sources_id_table.create();

	SoundSource& sl_source = m_sources[id.index];

	create_bufferqueue_player(sl_source);

	return id;
}

//-----------------------------------------------------------------------------
void SLESRenderer::play_source(SoundSourceId sid, SoundBufferId bid)
{
	CE_ASSERT(m_sources_id_table.has(sid), "SoundSource does not exist");
	CE_ASSERT(m_buffers_id_table.has(bid), "SoundBuffer does not exist");

	SoundSource& s = m_sources[sid.index];
	SoundBuffer& b = m_buffers[bid.index];

	SLresult result;

	result = (*s.player_bufferqueue)->Enqueue(s.player_bufferqueue, b.data, b.size);
	check_sles_errors(result);

	s.playing = true;
}

//-----------------------------------------------------------------------------
void SLESRenderer::pause_source(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& s = m_sources[id.index];

	SLresult result;

	result = (*s.player_play)->SetPlayState(s.player_play, SL_PLAYSTATE_STOPPED);

	check_sles_errors(result);

	s.playing = false;
}

//-----------------------------------------------------------------------------
void SLESRenderer::destroy_source(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& s = m_sources[id.index];

	destroy_bufferqueue_player(s);
}

//-----------------------------------------------------------------------------
void SLESRenderer::set_listener(const Vec3& pos, const Vec3& vel, const Vec3& or_up, const Vec3& or_at) const
{
	Log::w("Stub");

	(void)pos;
	(void)vel;
	(void)or_up;
	(void)or_at;
}

//-----------------------------------------------------------------------------
void SLESRenderer::set_source_min_distance(SoundSourceId id,  const float min_distance)
{
	Log::w("Stub");

	(void)id;
	(void)min_distance;
}

//-----------------------------------------------------------------------------
void SLESRenderer::set_source_max_distance(SoundSourceId id,  const float max_distance)
{
	Log::w("Stub");

	(void)id;
	(void)max_distance;
}

//-----------------------------------------------------------------------------
void SLESRenderer::set_source_position(SoundSourceId id, const Vec3& pos)
{
	Log::w("Stub");

	(void)id;
	(void)pos;
}

//-----------------------------------------------------------------------------
void SLESRenderer::set_source_velocity(SoundSourceId id, const Vec3& vel)
{
	Log::w("Stub");

	(void)id;
	(void)vel;
}

//-----------------------------------------------------------------------------
void SLESRenderer::set_source_direction(SoundSourceId id, const Vec3& dir)
{
	Log::w("Stub");

	(void)id;
	(void)dir;
}

//-----------------------------------------------------------------------------
void SLESRenderer::set_source_pitch(SoundSourceId id, const float pitch)
{
	Log::w("Stub");

	(void)id;
	(void)pitch;	
}

//-----------------------------------------------------------------------------
void SLESRenderer::set_source_gain(SoundSourceId id, const float gain)
{
	Log::w("Stub");

	(void)id;
	(void)gain;
}

//-----------------------------------------------------------------------------
void SLESRenderer::set_source_rolloff(SoundSourceId id, const float rolloff)
{
	Log::w("Stub");

	(void)id;
	(void)rolloff;
}

//-----------------------------------------------------------------------------
bool SLESRenderer::source_playing(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& s = m_sources[id.index];

    SLuint32 state;

    (*s.player_play)->GetPlayState(s.player_play, &state);

    return state == SL_PLAYSTATE_PLAYING;
}	

//-----------------------------------------------------------------------------
void SLESRenderer::check_sles_errors(SLresult result)
{
	CE_ASSERT(result == SL_RESULT_SUCCESS, "SL_ERROR_CODE: %d", result);
}

//-----------------------------------------------------------------------------
void SLESRenderer::create_bufferqueue_player(SoundSource& s)
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

	result = (*m_engine)->CreateAudioPlayer(m_engine, &s.player_obj, &audio_source, &audio_sink, 3, ids, reqs);
	check_sles_errors(result);

	result = (*s.player_obj)->Realize(s.player_obj, SL_BOOLEAN_FALSE);
	check_sles_errors(result);

	// Gets interfaces
	result = (*s.player_obj)->GetInterface(s.player_obj, SL_IID_PLAY, &s.player_play);
	check_sles_errors(result);

	result = (*s.player_obj)->GetInterface(s.player_obj, SL_IID_BUFFERQUEUE, &s.player_bufferqueue);
	check_sles_errors(result);

	result = (*s.player_obj)->GetInterface(s.player_obj, SL_IID_VOLUME, &s.player_volume);
	check_sles_errors(result);

	(*s.player_bufferqueue)->RegisterCallback(s.player_bufferqueue, player_callback, &s);

	result = (*s.player_play)->SetPlayState(s.player_play, SL_PLAYSTATE_PLAYING);
	check_sles_errors(result);
}


//-----------------------------------------------------------------------------
void SLESRenderer::destroy_bufferqueue_player(SoundSource& s)
{
	if (s.player_obj)
	{
		SLuint32 state;
		(*s.player_obj)->GetState(s.player_obj, &state);

		if (state == SL_OBJECT_STATE_REALIZED)
		{
			(*s.player_bufferqueue)->Clear(s.player_bufferqueue);
			(*s.player_obj)->AbortAsyncOperation(s.player_obj);
			(*s.player_obj)->Destroy(s.player_obj);

  			s.player_obj = NULL;
  			s.player_play = NULL;
  			s.player_bufferqueue = NULL;
  			s.player_volume = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
void player_callback(SLAndroidSimpleBufferQueueItf caller, void* source)
{
	(void)caller;

	SoundSource* s = (SoundSource*)source;

	(*s->player_play)->SetPlayState(s->player_play, SL_PLAYSTATE_STOPPED);
}

} // namespace crown