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

//-----------------------------------------------------------------------------
struct Listener
{
	ALfloat gain;
	ALfloat position[3];
	ALfloat velocity[3];
	ALfloat orientation[6];
};

//-----------------------------------------------------------------------------
struct Buffer
{
	ALuint 		bufferid;
	ALenum 		format;
	ALsizei		size;
	ALsizei		freq;
};


//-----------------------------------------------------------------------------
struct Source
{
	ALuint		id;
};

//-----------------------------------------------------------------------------
struct Sound
{
	Source 		source;
	Buffer 		buffer;		// Should be an array of buffer
};

//-----------------------------------------------------------------------------
class ALRenderer
{
public:

					ALRenderer();

	void			init();
	void			shutdown();

	// FIXME: public APIs, They should be placed in AudioSystem
	SoundId			create_sound(const void* data, uint32_t size, uint32_t sample_rate, uint32_t channels, uint32_t bxs);
	void 			play_sound(SoundId id);
	void			pause_sound(SoundId id);
	void 			destroy_sound(SoundId id);

	bool			is_sound_playing(SoundId id);

public:

	static const uint32_t MAX_SOUNDS = 128;
	// END

private:

	// FIXME: the following methods are the real wrapper of needed OpenAL functions
	Buffer			create_buffer(const void* data, uint32_t size, uint32_t sample_rate, uint32_t channels, uint32_t bxs);
	void			destroy_buffer(Buffer b);

	Source			create_source();
	void			play_source(Source s);
	void			pause_source(Source s);
	void			destroy_source(Source s);
	bool			is_source_playing(Source s);

	Listener		create_listener();
	void			destroy_listener(Listener);
	// END

private:

	HeapAllocator 	m_allocator;

	ALCdevice*		m_device;
	ALCcontext*		m_context;

	IdTable			m_sounds_id_table;
	Sound 			m_sounds[MAX_SOUNDS];
};

} // namespace crown