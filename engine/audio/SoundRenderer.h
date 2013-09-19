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

#define	MAX_SOUND_SOURCES 64
#define	MAX_SOUND_BUFFERS 128

namespace crown
{
//-----------------------------------------------------------------------------
typedef 	Id 		SoundBufferId;
typedef 	Id 		SoundSourceId;

//-----------------------------------------------------------------------------
class SoundRendererBackend;
class Vec3;

//-----------------------------------------------------------------------------
class SoundRenderer
{
public:
							SoundRenderer(Allocator& allocator);

							~SoundRenderer();

	void					init();

	void					shutdown();

	/// Returns number of buffers actually in use
	uint32_t				num_buffers();

	/// Returns number of sources actually in use
	uint32_t				num_sources();

	/// Sets listener parameters. @a position affects audibility of sounds, 
	/// @a velocity affects doppler shift and @a orientation affects how a sound could be heard
	void					set_listener(const Vec3& pos, const Vec3& vel, const Vec3& or_up, const Vec3& or_at) const;

	/// Creates buffer's @a data, with a specific @a size, which contains sound raw data.
	/// More parameters must be specified, such as @a sample_rate, that is the number of sample per unit of time 
	/// taken from a continuous signal to make a discrete signal, @a channels which specifies if sound is mono or stereo and @a bits
	/// which specifies the magnitude of samples information.
	/// N.B: stereo sound cannot be attenuated
	SoundBufferId			create_buffer(const void* data, const uint32_t size, const uint32_t sample_rate, const uint32_t channels, const uint32_t bits);

	void					update_buffer(SoundBufferId id, const void* data, const uint32_t size);
	/// Destroys buffer
	void					destroy_buffer(SoundBufferId id);

	/// Binds buffer @a bid to source @a sid
	void					bind_buffer_to_source(SoundBufferId bid, SoundSourceId sid);

	/// Unbind the first buffer from @a id
	void					unbind_buffer_from_source(SoundSourceId id);

	/// Creates a source of sound 
	SoundSourceId			create_source();

	/// Updates buffer @a bid of source @a sid with new @a data of size @a size
	/// Updates only happens when there is a free buffer;
	bool 					update_source(SoundSourceId sid, SoundBufferId bid, const void* data, const size_t size);

	/// Destroys a sound, specified by @a id, previously created
	void					destroy_source(SoundSourceId id);

	/// Plays a sound, specified by source @a sid and a buffer @a bid
	void					play_source(SoundSourceId id, bool loop);

	/// Pauses a sound, specified by @a id
	void					pause_source(SoundSourceId id);

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
	void					set_source_gain(SoundSourceId id, const float gain);

	/// Sets source's @a rolloff factor. Greater it is, greater sound's attenuation is
	void					set_source_rolloff(SoundSourceId id, const float rolloff);

	/// Returns minimum distance of @a id
	float					source_min_distance(SoundSourceId id) const;

	/// Returns maximum distance of @a id
	float					source_max_distance(SoundSourceId id) const;
	
	/// Returns position of @a id
	Vec3					source_position(SoundSourceId id) const;

	/// Returns velocity of @a id
	Vec3					source_velocity(SoundSourceId id) const;

	/// Returns direction of @a id
	Vec3					source_direction(SoundSourceId id) const;

	/// Returns pitch of @a id
	float					source_pitch(SoundSourceId id) const;

	/// Returns gain of @a id
	float					source_gain(SoundSourceId id) const;

	/// Returns rolloff factor of @a id
	float					source_rolloff(SoundSourceId id) const;

	/// Returns number of queued buffers of @a id
	int32_t					source_queued_buffers(SoundSourceId id) const;

	/// Returns number of processed buffers of @a id
	int32_t					source_processed_buffers(SoundSourceId id) const;

	/// Is source @a id playing?
	bool					source_playing(SoundSourceId id);

private:

	Allocator& 				m_allocator;

	SoundRendererBackend*	m_backend;

	IdTable<MAX_SOUND_BUFFERS>		m_buffers_id_table;
	IdTable<MAX_SOUND_SOURCES>		m_sources_id_table;

	uint32_t				m_num_buffers;
	uint32_t				m_num_sources;
};

} // namespace crown