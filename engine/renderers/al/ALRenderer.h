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

#include "Types.h"
#include "AudioRenderer.h"
#include "HeapAllocator.h"
#include "Vec3.h"

namespace crown
{

//-----------------------------------------------------------------------------
struct SoundBuffer
{
	ALuint 		id;
};

//-----------------------------------------------------------------------------
struct SoundSource
{
	ALuint		id;
};



//-----------------------------------------------------------------------------
class ALRenderer : public AudioRenderer
{
public:

							ALRenderer();

	void					init();

	void					shutdown();

	void					set_listener(const Vec3& pos, const Vec3& vel, const Vec3& or_up, const Vec3& or_at) const;

	SoundBufferId			create_buffer(const void* data, const uint32_t size, const uint32_t sample_rate, const uint32_t channels, const uint32_t bxs);

	void					destroy_buffer(SoundBufferId id);

	SoundSourceId			create_source(const Vec3& pos, const Vec3& vel, const Vec3& dir, const bool loop);

	void 					play_source(SoundSourceId id);

	void					pause_source(SoundSourceId id);

	void 					destroy_source(SoundSourceId id);

	void					bind_buffer(SoundSourceId sid, SoundBufferId bid);

	void					set_source_min_distance(SoundSourceId id,  const float min_distance);

	void					set_source_max_distance(SoundSourceId id,  const float max_distance);

	void					set_source_position(SoundSourceId id, const Vec3& pos);

	void					set_source_velocity(SoundSourceId id, const Vec3& vel);

	void					set_source_direction(SoundSourceId id, const Vec3& dir);

	void					set_source_pitch(SoundSourceId id, const float pitch);

	void 					set_source_gain(SoundSourceId id, const float gain);

	void					set_source_rolloff(SoundSourceId id, const float rolloff);

	bool					source_playing(SoundSourceId id);

private:

	HeapAllocator 			m_allocator;

	ALCdevice*				m_device;
	ALCcontext*				m_context;

	IdTable 				m_buffers_id_table;
	SoundBuffer 			m_buffers[MAX_BUFFERS];

	IdTable 				m_sources_id_table;
	SoundSource 			m_sources[MAX_SOURCES];
};

} // namespace crown