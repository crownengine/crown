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

#define	MAX_SOUNDS 64

namespace crown
{
//-----------------------------------------------------------------------------
typedef 	Id 		SoundId;

//-----------------------------------------------------------------------------
class SoundRendererBackend;
class SoundResource;
class Vec3;

//-----------------------------------------------------------------------------
class SoundRenderer
{
public:
							SoundRenderer(Allocator& allocator);

							~SoundRenderer();

	void					init();

	void					shutdown();

	void					frame();

	/// Returns number of sounds actually in use
	uint32_t				num_sounds();

	/// Sets listener parameters. @a position affects audibility of sounds, 
	/// @a velocity affects doppler shift and @a orientation affects how a sound could be heard
	void					set_listener(const Vec3& pos, const Vec3& vel, const Vec3& or_up, const Vec3& or_at) const;

	/// Creates a sound of sound 
	SoundId					create_sound(SoundResource* resource);

	/// Destroys a sound, specified by @a id, previously created
	void					destroy_sound(SoundId id);

	/// Plays a sound, specified by sound @a sid and a buffer @a bid
	void					play_sound(SoundId id);

	/// Pauses a sound, specified by @a id
	void					pause_sound(SoundId id);

	///	Sets sound's @a min_distance. From @a min_distance to @a max_distance, sound
	/// scales from full volume to silence
	void					set_sound_min_distance(SoundId id, const float min_distance);

	///	Sets sound's @a max_distance. From @a min_distance to @a max_distance, sound
	/// scales from full volume to silence
	void					set_sound_max_distance(SoundId id, const float max_distance);

	/// Sets sound's @a position. It affects sound audibility
	void					set_sound_position(SoundId id, const Vec3& pos);

	/// Sets sound's @a velocity. It affects doppler shift
	void					set_sound_velocity(SoundId id, const Vec3& vel);

	/// Sets sound's @a direction. It affects how a sound could be heard
	void					set_sound_direction(SoundId id, const Vec3& dir);

	/// Sets sound's @a pitch.
	void					set_sound_pitch(SoundId id, const float pitch);

	/// Sets sound's @a gain, that is measure sound's amplification
	void					set_sound_gain(SoundId id, const float gain);

	/// Sets sound's @a rolloff factor. Greater it is, greater sound's attenuation is
	void					set_sound_rolloff(SoundId id, const float rolloff);

	/// Returns minimum distance of @a id
	float					sound_min_distance(SoundId id) const;

	/// Returns maximum distance of @a id
	float					sound_max_distance(SoundId id) const;
	
	/// Returns position of @a id
	Vec3					sound_position(SoundId id) const;

	/// Returns velocity of @a id
	Vec3					sound_velocity(SoundId id) const;

	/// Returns direction of @a id
	Vec3					sound_direction(SoundId id) const;

	/// Returns pitch of @a id
	float					sound_pitch(SoundId id) const;

	/// Returns gain of @a id
	float					sound_gain(SoundId id) const;

	/// Returns rolloff factor of @a id
	/// Rolloff: perceived volume decreases at a fixed rate as a sound sound moves away from the listener
	float					sound_rolloff(SoundId id) const;

	/// Returns number of queued buffers of @a id
	int32_t					sound_queued_buffers(SoundId id) const;

	/// Returns number of processed buffers of @a id
	int32_t					sound_processed_buffers(SoundId id) const;

	/// Is sound @a id playing?
	bool					sound_playing(SoundId id);

private:

	Allocator& 				m_allocator;

	SoundRendererBackend*	m_backend;

	IdTable<MAX_SOUNDS>		m_sounds_id_table;

	uint32_t				m_num_sounds;
};

} // namespace crown