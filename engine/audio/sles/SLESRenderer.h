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

#include "SoundResource.h"
#include "OggDecoder.h"
#include "Vec3.h"
#include "Log.h"

namespace crown
{

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
	default: return "SL_RESULT_UNKNOWN";
	}
}

//-----------------------------------------------------------------------------
void check_sles_errors(SLresult result)
{
	CE_ASSERT(result == SL_RESULT_SUCCESS, "SL_ERROR_CODE: %s", sles_error_to_string(result));
}

//-----------------------------------------------------------------------------
struct Sound
{
					Sound();

	void 			create(SLEngineItf engine, SLObjectItf out_mix_obj, SoundResource* resource);
	void 			update();
	void 			destroy();
	void 			play();
	void 			pause();
	void			unpause();
	void 			loop(bool loop);

	bool 			is_created() const;
	bool 			is_playing() const;

	static void 	buffer_callback(SLAndroidSimpleBufferQueueItf caller, void* sound);

public:

	SoundResource*					m_res;

	SLEngineItf						m_engine;
	SLObjectItf 					m_out_mix_obj;
	SLObjectItf 					m_player_obj;
	SLPlayItf 						m_player_play;
	SLAndroidSimpleBufferQueueItf 	m_player_bufferqueue;
	SLVolumeItf						m_player_volume;

	uint32_t						m_processed_buffers;
	
	uint16_t* 						m_data;
	size_t	  						m_size;
	uint32_t  						m_sample_rate;
	uint32_t  						m_channels;
	uint32_t  						m_bits;

	bool 							m_created :1;
	bool							m_playing :1;
	bool							m_looping :1;
	bool							m_streaming :1;
	bool 							m_positional :1;

	OggDecoder						m_decoder;
};

//-----------------------------------------------------------------------------
inline void Sound::buffer_callback(SLAndroidSimpleBufferQueueItf caller, void* sound)
{
	Sound* s = (Sound*)sound;

	if (s->is_playing())
	{
		s->m_processed_buffers++;

		if (s->m_decoder.stream())
		{
			(*s->m_player_bufferqueue)->Enqueue(s->m_player_bufferqueue, s->m_decoder.data(), s->m_decoder.size());
		}
		else if (s->m_looping)
		{
			s->m_decoder.rewind();
			s->m_decoder.stream();
			(*s->m_player_bufferqueue)->Enqueue(s->m_player_bufferqueue, s->m_decoder.data(), s->m_decoder.size());	
		}
		else
		{
			s->pause();
		}
	}
}

//-----------------------------------------------------------------------------
inline Sound::Sound()
	: m_res(NULL)
	, m_player_obj(NULL)
	, m_player_play(NULL)
	, m_player_bufferqueue(NULL)
	, m_player_volume(NULL)
	, m_processed_buffers(0)
	, m_data(NULL)
	, m_size(0)
	, m_sample_rate(0)
	, m_channels(0)
	, m_bits(0)
	, m_created(false)
	, m_playing(false)
	, m_looping(false)
	, m_streaming(false)
	, m_positional(false)
{
}

//-----------------------------------------------------------------------------
inline void Sound::create(SLEngineItf engine, SLObjectItf out_mix_obj, SoundResource* resource)
{
	CE_ASSERT_NOT_NULL(resource);

	m_engine = engine;
	m_out_mix_obj = out_mix_obj;

	m_res = resource;

	// Configures buffer queue
	SLDataLocator_AndroidSimpleBufferQueue buffer_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

	// Configures audio format
	SLDataFormat_PCM format_pcm;
	format_pcm.formatType = SL_DATAFORMAT_PCM;

	// Sets channels
	switch (m_res->channels())
	{
	case 1:
		format_pcm.numChannels = 1;
		format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
		break;
	case 2:
	default:
		format_pcm.numChannels = 2;
		format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
		break;
	}

	// Sets sample rate
	switch (m_res->sample_rate())
	{
	case 8000:
		format_pcm.samplesPerSec = SL_SAMPLINGRATE_8;
		break;
	case 11025:
		format_pcm.samplesPerSec = SL_SAMPLINGRATE_11_025;
		break;
	case 16000:
		format_pcm.samplesPerSec = SL_SAMPLINGRATE_16;
		break;
	case 22050:
		format_pcm.samplesPerSec = SL_SAMPLINGRATE_22_05;
		break;
	case 24000:
		format_pcm.samplesPerSec = SL_SAMPLINGRATE_24;
		break;
	case 32000:
		format_pcm.samplesPerSec = SL_SAMPLINGRATE_32;
		break;
	case 44100:
		format_pcm.samplesPerSec = SL_SAMPLINGRATE_44_1;
		break;
	case 48000:
		format_pcm.samplesPerSec = SL_SAMPLINGRATE_48;
		break;
	case 64000:
		format_pcm.samplesPerSec = SL_SAMPLINGRATE_64;
		break;
	case 88200:
		format_pcm.samplesPerSec = SL_SAMPLINGRATE_88_2;
		break;
	case 96000:
		format_pcm.samplesPerSec = SL_SAMPLINGRATE_96;
		break;
	case 192000:
		format_pcm.samplesPerSec = SL_SAMPLINGRATE_192;
		break;
	}	
	
	format_pcm.samplesPerSec = SL_SAMPLINGRATE_44_1;
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
	out_mix.outputMix = m_out_mix_obj;

	// Configures audio sink
	SLDataSink audio_sink;
	audio_sink.pLocator = &out_mix;
	audio_sink.pFormat = NULL;

	// Creates sound player
	const SLInterfaceID ids[] = {SL_IID_PLAY, SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
	const SLboolean reqs[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

	SLresult result = (*m_engine)->CreateAudioPlayer(m_engine, &m_player_obj, &audio_source, &audio_sink, 3, ids, reqs);
	check_sles_errors(result);

	result = (*m_player_obj)->Realize(m_player_obj, SL_BOOLEAN_FALSE);
	check_sles_errors(result);

	// Gets interfaces
	result = (*m_player_obj)->GetInterface(m_player_obj, SL_IID_PLAY, &m_player_play);
	check_sles_errors(result);

	result = (*m_player_obj)->GetInterface(m_player_obj, SL_IID_BUFFERQUEUE, &m_player_bufferqueue);
	check_sles_errors(result);

	result = (*m_player_obj)->GetInterface(m_player_obj, SL_IID_VOLUME, &m_player_volume);
	check_sles_errors(result);

	(*m_player_bufferqueue)->RegisterCallback(m_player_bufferqueue, Sound::buffer_callback, this);

	// Manage simple sound or stream
	m_streaming = m_res->sound_type() == SoundType::OGG;

	if (m_streaming)
	{
		m_decoder.init((char*)m_res->data(), m_res->size());

		m_decoder.stream();
		(*m_player_bufferqueue)->Enqueue(m_player_bufferqueue, m_decoder.data(), m_decoder.size());
	}
	else
	{
		(*m_player_bufferqueue)->Enqueue(m_player_bufferqueue, m_res->data(), m_res->size());
	}

	m_created = true;
}

//-----------------------------------------------------------------------------
inline void Sound::update()
{
	// See SLESRenderer.cpp, row 160
}

//-----------------------------------------------------------------------------
inline void Sound::destroy()
{
	if (m_player_obj)
	{
		SLuint32 state;
		(*m_player_obj)->GetState(m_player_obj, &state);

		if (state == SL_OBJECT_STATE_REALIZED)
		{
			(*m_player_bufferqueue)->Clear(m_player_bufferqueue);
			(*m_player_obj)->AbortAsyncOperation(m_player_obj);
			(*m_player_obj)->Destroy(m_player_obj);

			m_player_obj = NULL;
			m_player_play = NULL;
			m_player_bufferqueue = NULL;
			m_player_volume = NULL;
		}

		m_created = false;
		m_playing = false;
		m_looping = false;
		m_streaming = false;
	}
}

//-----------------------------------------------------------------------------
inline void Sound::play()
{
	SLresult result = (*m_player_play)->SetPlayState(m_player_play, SL_PLAYSTATE_PLAYING);
	check_sles_errors(result);

	m_playing = true;
}

//-----------------------------------------------------------------------------
inline void Sound::pause()
{
	SLresult result = (*m_player_play)->SetPlayState(m_player_play, SL_PLAYSTATE_PAUSED);
	check_sles_errors(result);

	m_playing = false;
}

//-----------------------------------------------------------------------------
inline void Sound::unpause()
{
	m_decoder.stream();
	(*m_player_bufferqueue)->Enqueue(m_player_bufferqueue, m_decoder.data(), m_decoder.size());
	play();
}

//-----------------------------------------------------------------------------
inline void Sound::loop(bool loop)
{
	m_looping = loop;
}

//-----------------------------------------------------------------------------
bool Sound::is_created() const
{
	return m_created;
}

//-----------------------------------------------------------------------------
inline bool Sound::is_playing() const
{
	return m_playing;
}

} // namespace crown