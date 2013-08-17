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

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "Types.h"
#include "AudioRenderer.h"
#include "HeapAllocator.h"

namespace crown
{

void player_callback(SLAndroidSimpleBufferQueueItf caller, void* source);

//-----------------------------------------------------------------------------
struct SoundBuffer
{
	uint16_t*						data;
	size_t							size;
	uint32_t						sample_rate;
	uint32_t						channels;
	uint32_t						bits_per_sample;
};

//-----------------------------------------------------------------------------
struct SoundSource
{
  	SLObjectItf 					player_obj;
  	SLPlayItf 						player_play;
  	SLAndroidSimpleBufferQueueItf 	player_bufferqueue;
  	SLSeekItf 						player_seek;
  	SLVolumeItf						player_volume;
  	bool							playing;
};


class SLESRenderer : public AudioRenderer
{	
public:

									SLESRenderer();

	void							init();
	void							shutdown();

	void							set_listener(const Vec3& pos, const Vec3& vel, const Vec3& or_up, const Vec3& or_at) const;

	SoundBufferId					create_buffer(const void* data, const uint32_t size, const uint32_t sample_rate, const uint32_t channels, const uint32_t bxs);
	void							destroy_buffer(SoundBufferId id);

	SoundSourceId					create_source();
	void							play_source(SoundSourceId sid, SoundBufferId bid);
	void							pause_source(SoundSourceId id);
	void							destroy_source(SoundSourceId id);

	void							set_source_min_distance(SoundSourceId id,  const float min_distance);
	void							set_source_max_distance(SoundSourceId id,  const float max_distance);
	void							set_source_position(SoundSourceId id, const Vec3& pos);
	void							set_source_velocity(SoundSourceId id, const Vec3& vel);
	void							set_source_direction(SoundSourceId id, const Vec3& dir);
	void							set_source_pitch(SoundSourceId id, const float pitch);
	void							set_source_gain(SoundSourceId id, const float gain);
	void							set_source_rolloff(SoundSourceId id, const float rolloff);
	bool							source_playing(SoundSourceId id);	

	void 							check_sles_errors(SLresult result);

private:

	void							create_bufferqueue_player(SoundSource& s);

	void							destroy_bufferqueue_player(SoundSource& s);

private:

	HeapAllocator					m_allocator;

	SLObjectItf						m_engine_obj;
	SLEngineItf						m_engine;

	SLObjectItf 					m_out_mix_obj;

//-----------------------------------------------------------------------------

	IdTable 						m_buffers_id_table;
	SoundBuffer 					m_buffers[MAX_BUFFERS];

	IdTable 						m_sources_id_table;
	SoundSource 					m_sources[MAX_SOURCES];
};

} // namespace crown