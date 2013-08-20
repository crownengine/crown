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
#include "SoundResource.h"

namespace crown
{

class Vec3;

typedef		Id 		SoundId;
typedef 	Id 		SoundBufferId;
typedef 	Id 		SoundSourceId;

class AudioRenderer
{
public:

	static const uint32_t	MAX_SOURCES = 32;
	static const uint32_t	MAX_BUFFERS_PER_SOURCES = 16;
	static const uint32_t 	MAX_BUFFERS = MAX_SOURCES * MAX_BUFFERS_PER_SOURCES;
	
public:

	static AudioRenderer*	create(Allocator& a);
	static void				destroy(Allocator& a, AudioRenderer* renderer);

	virtual void			init() = 0;
	virtual void			shutdown() = 0;

	/// Sets listener parameters. @a position affects audibility of sounds, 
	/// @a velocity affects doppler shift and @a orientation affects how a sound could be heard
	virtual void			set_listener(const Vec3& pos, const Vec3& vel, const Vec3& or_up, const Vec3& or_at) const = 0;

	/// Creates buffer's @a data, with a specific @a size, which contains sound raw data.
	/// More parameters must be specified, such as @a sample_rate, that is the number of sample per unit of time 
	/// taken from a continuous signal to make a discrete signal, @a channels which specifies if sound is mono or stereo and @ bxs
	/// (bits per sample) which specifies the magnitude of samples information.
	/// N.B: stereo sound can not be attenuated
	virtual SoundBufferId	create_buffer(const void* data, const uint32_t size, const uint32_t sample_rate, const uint32_t channels, const uint32_t bxs) = 0;
	/// Destroys buffer
	virtual void			destroy_buffer(SoundBufferId id) = 0;

	/// Creates a source of sound 
	virtual SoundSourceId	create_source() = 0;
	/// Creates a perpetual source of sound
	virtual SoundSourceId	create_loop_source() = 0;
	/// Plays a sound, specified by @a id, previously created
	virtual void			play_source(SoundSourceId sid, SoundBufferId bid) = 0;
	/// Pauses a sound, specified by @a id, previously created
	virtual void			pause_source(SoundSourceId id) = 0;
	/// Destroys a sound, specified by @a id, previously created
	virtual void			destroy_source(SoundSourceId id) = 0;


	///	Sets source's @a min_distance. From @a min_distance to @a max_distance, sound
	/// scales from full volume to silence
	virtual void			set_source_min_distance(SoundSourceId id,  const float min_distance) = 0;
	///	Sets source's @a max_distance. From @a min_distance to @a max_distance, sound
	/// scales from full volume to silence
	virtual void			set_source_max_distance(SoundSourceId id,  const float max_distance) = 0;
	/// Sets source's @a position. It affects sound audibility
	virtual void			set_source_position(SoundSourceId id, const Vec3& pos) = 0;
	/// Sets source's @a velocity. It affects doppler shift
	virtual void			set_source_velocity(SoundSourceId id, const Vec3& vel) = 0;
	/// Sets source's @a direction. It affects how a sound could be heard
	virtual void			set_source_direction(SoundSourceId id, const Vec3& dir) = 0;
	/// Sets source's @a pitch.
	virtual void			set_source_pitch(SoundSourceId id, const float pitch) = 0;
	/// Sets source's @a gain, that is measure sound's amplification
	virtual void			set_source_gain(SoundSourceId id, const float gain) = 0;
	/// Sets source's @a rolloff factor. Greater it is, greater sound's attenuation is
	virtual void			set_source_rolloff(SoundSourceId id, const float rolloff) = 0;
	/// Is source #@a id playing?
	virtual bool			source_playing(SoundSourceId id) = 0;

	// Tests
	virtual void			create_stream(const void* data, const uint32_t size, const uint32_t sample_rate, 
								const uint32_t channels, const uint32_t bxs, SoundBufferId* ids) = 0;
	virtual void			play_source(SoundSourceId sid, SoundBufferId* bids) = 0;

};

} // namespace crown