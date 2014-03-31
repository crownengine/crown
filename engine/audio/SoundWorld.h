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

#include "Types.h"

namespace crown
{

/// Global audio-related functions
///
/// @ingroup Audio
namespace audio_system
{
	/// Initializes the audio system.
	/// This is the place where to create and initialize per-application objects.
	void init();

	/// It should reverse the actions performed by audio_system::init().
	void shutdown();
} // namespace audio_system

class Allocator;
struct Vector3;
struct Matrix4x4;
struct SoundResource;
typedef Id SoundInstanceId;

///
/// @ingroup Audio
class SoundWorld
{
public:

	virtual ~SoundWorld() {};

	/// Plays the sound @a name at the given @a volume [0 .. 1].
	/// If loop is true the sound will be played looping.
	virtual SoundInstanceId play(const char* name, bool loop, float volume, const Vector3& pos) = 0;

	/// Stops the sound with the given @a id.
	/// After this call, the instance will be destroyed.
	virtual void stop(SoundInstanceId id) = 0;

	/// Returns wheter the sound @a id is playing.
	virtual bool is_playing(SoundInstanceId id) = 0;

	/// Stops all the sounds in the world.
	virtual void stop_all() = 0;

	/// Pauses all the sounds in the world
	virtual void pause_all() = 0;

	/// Resumes all previously paused sounds in the world.
	virtual void resume_all() = 0;

	/// Sets the @a positions (in world space) of @a count sound instances @a ids.
	virtual void set_sound_positions(uint32_t count, const SoundInstanceId* ids, const Vector3* positions) = 0;

	/// Sets the @a ranges (in meters) of @a count sound instances @a ids.
	virtual void set_sound_ranges(uint32_t count, const SoundInstanceId* ids, const float* ranges) = 0;

	/// Sets the @a volumes of @a count sound instances @a ids.
	virtual void set_sound_volumes(uint32_t count, const SoundInstanceId* ids, const float* volumes) = 0;

	virtual void reload_sounds(SoundResource* old_sr, SoundResource* new_sr) = 0;

	/// Sets the @a pose of the listener in world space.
	virtual void set_listener_pose(const Matrix4x4& pose) = 0;

	virtual void update() = 0;

	static SoundWorld* create(Allocator& a);
	static void destroy(Allocator& a, SoundWorld* sw);
};


} // namespace crown 
