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

#include "IdTable.h"
#include "Allocator.h"
#include "Types.h"

#define	MAX_SOUND_BUFFERS 64
#define MAX_SOUND_SOURCES 64

namespace crown
{
//-----------------------------------------------------------------------------
class SoundRendererImpl;
struct SoundResource;
struct Vector3;

//-----------------------------------------------------------------------------
typedef Id	SoundBufferId;
typedef	Id	SoundSourceId;

//-----------------------------------------------------------------------------
class SoundRenderer
{
public:
							SoundRenderer(Allocator& allocator);

							~SoundRenderer();

	void					init();

	void					shutdown();

	void					pause();

	void					unpause();

	void					frame();

	void					set_listener(const Vector3& pos, const Vector3& vel, const Vector3& or_up, const Vector3& or_at) const;

	SoundBufferId			create_sound_buffer(void* data, size_t size, uint32_t sample_rate, uint32_t num_channels, uint16_t bits_ps);

	void					destroy_sound_buffer(SoundBufferId id);

	SoundSourceId			create_sound_source();

	void					destroy_sound_source(SoundSourceId id);

	void					bind_buffer(SoundBufferId buffer, SoundSourceId source);

	void					unbind_buffer(SoundSourceId id);

	void					play_sound(SoundSourceId id);

	void					pause_sound(SoundSourceId id);

	void					set_sound_loop(SoundSourceId id, bool loop);

	void					set_sound_min_distance(SoundSourceId id, const float min_distance);

	void					set_sound_max_distance(SoundSourceId id, const float max_distance);

	void					set_sound_position(SoundSourceId id, const Vector3& pos);

	void					set_sound_velocity(SoundSourceId id, const Vector3& vel);

	void					set_sound_direction(SoundSourceId id, const Vector3& dir);

	void					set_sound_pitch(SoundSourceId id, const float pitch);

	void					set_sound_gain(SoundSourceId id, const float gain);

	void					set_sound_rolloff(SoundSourceId id, const float rolloff);

	float					sound_min_distance(SoundSourceId id) const;

	float					sound_max_distance(SoundSourceId id) const;
	
	Vector3					sound_position(SoundSourceId id) const;

	Vector3					sound_velocity(SoundSourceId id) const;

	Vector3					sound_direction(SoundSourceId id) const;

	float					sound_pitch(SoundSourceId id) const;

	float					sound_gain(SoundSourceId id) const;

	float					sound_rolloff(SoundSourceId id) const;

	int32_t					sound_queued_buffers(SoundSourceId id) const;

	int32_t					sound_processed_buffers(SoundSourceId id) const;

	bool					sound_playing(SoundSourceId id);

private:

	Allocator& 					m_allocator;
	SoundRendererImpl*			m_impl;
	bool 						m_is_paused;

	IdTable<MAX_SOUND_BUFFERS>	m_buffers_id_table;
	IdTable<MAX_SOUND_SOURCES>	m_sources_id_table;
};

} // namespace crown