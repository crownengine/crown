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
#include "IdTable.h"
#include "SoundResource.h"
#include "HeapAllocator.h"
#include "Vec3.h"

namespace crown
{

typedef Id SoundId;
typedef Id SoundBufferId;
typedef Id SoundSourceId;

//-----------------------------------------------------------------------------
struct SoundBuffer
{
	ALuint 		id;

	ALenum 		format;
	ALsizei		size;
	ALsizei		freq;
};

//-----------------------------------------------------------------------------
struct SoundSource
{
	ALuint		id;

	ALfloat		pos[3];
	ALfloat		vel[3];
	ALfloat		dir[3];
};



//-----------------------------------------------------------------------------
class ALRenderer
{
public:

	static const uint32_t	MAX_SOURCES = 128;
	static const uint32_t	MAX_BUFFERS_PER_SOURCES = 16;
	static const uint32_t 	MAX_BUFFERS = MAX_SOURCES * MAX_BUFFERS_PER_SOURCES;

public:

							ALRenderer();

	/// Init AL renderer. Must be called first
	void					init();

	/// Shutdown AL Renderer
	void					shutdown();

	/// Sets listener parameters. In OpenAL, @a position affects audibility of sounds, 
	/// @a velocity affects doppler shift and @a orientation affects how a sound could be heard
	void					set_listener(const Vec3& pos, const Vec3& vel, const Vec3& or_up, const Vec3& or_at) const;

	/// Creates AL buffer's @a data, with a specific @a size, which contains sound raw data.
	/// More parameters must be specified, such as @a sample_rate, that is the number of sample per unit of time 
	/// taken from a continuous signal to make a discrete signal, @a channels which specifies if sound is mono or stereo and @ bxs
	/// (bits per sample) which specifies the magnitude of samples information.
	/// N.B: stereo sound can not be attenuated
	SoundBufferId			create_buffer(const void* data, const uint32_t size, const uint32_t sample_rate, const uint32_t channels, const uint32_t bxs);

	/// Destroys AL buffer
	void					destroy_buffer(SoundBufferId id);

	/// Creates AL source of sound at the given @position in 3D space
	/// @a velocity affects doppler shift and @a direction affects how a sound could be heard
	SoundSourceId			create_source(const Vec3& pos, const Vec3& vel, const Vec3& dir, const bool loop);

	/// Plays a sound, specified by @a id, previously created
	void 					play_source(SoundSourceId id);

	/// Pauses a sound, specified by @a id, previously created
	void					pause_source(SoundSourceId id);

	/// Destroys a sound, specified by @a id, previously created
	void 					destroy_source(SoundSourceId id);

	/// Binds a AL buffer to AL source
	void					bind_buffer(SoundSourceId sid, SoundBufferId bid);
	///	Sets source's @a min_distance. From @a min_distance to @a max_distance, sound
	/// scales from full volume to silence
	void					set_source_min_distance(SoundSourceId id,  const float min_distance);
	///	Sets source's @a max_distance. From @a min_distance to @a max_distance, sound
	/// scales from full volume to silence
	void					set_source_max_distance(SoundSourceId id,  const float max_distance);
	/// Sets source's @a position. It affects sound audibility
	void					set_source_position(SoundSourceId id, const Vec3& pos);
	/// Sets source's @a velocity. It affects doppler shift
	void					set_source_velocity(SoundSourceId id, const Vec3& vel);
	/// Sets source's @a direction. It affects how a sound could be heard
	void					set_source_direction(SoundSourceId id, const Vec3& dir);
	/// Sets source's @a pitch.
	void					set_source_pitch(SoundSourceId id, const float pitch);
	/// Sets source's @a gain, that is measure sound's amplification
	void 					set_source_gain(SoundSourceId id, const float gain);
	/// Sets source's @a rolloff factor. Greater it is, greater sound's attenuation is
	void					set_source_rolloff(SoundSourceId id, const float rolloff);
	/// Is source #@a id playing?
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