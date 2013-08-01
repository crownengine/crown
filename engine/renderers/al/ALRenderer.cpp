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

#include "ALRenderer.h"
#include "StringUtils.h"

namespace crown
{


//-----------------------------------------------------------------------------
static const char* al_error_to_string(ALenum error)
{
	switch (error)
	{
		case AL_INVALID_ENUM: return "AL_INVALID_ENUM";
		case AL_INVALID_VALUE: return "AL_INVALID_VALUE";
		case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION";
		case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY";
		default: return "UNKNOWN_AL_ERROR";
	}
}

//-----------------------------------------------------------------------------
#ifdef CROWN_DEBUG
	#define AL_CHECK(function)\
		function;\
		do { ALenum error; CE_ASSERT((error = alGetError()) == AL_NO_ERROR,\
				"OpenAL error: %s", al_error_to_string(error)); } while (0)
#else
	#define AL_CHECK(function)\
		function;
#endif

//-----------------------------------------------------------------------------
ALRenderer::ALRenderer() :
	m_sounds_id_table(m_allocator, MAX_SOUNDS)
{

}

//-----------------------------------------------------------------------------
void ALRenderer::init()
{
	m_device = alcOpenDevice(NULL);
	
	if (!m_device)
	{
		CE_ASSERT(false, "Cannot open audio device");
	}

	m_context = alcCreateContext(m_device, NULL);

	if (!m_context)
	{
		CE_ASSERT(false, "Cannot create context");		
	}

	AL_CHECK(alcMakeContextCurrent(m_context));

	ALfloat dir[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f};

	AL_CHECK(alListener3f(AL_POSITION, 0, 0, 1.0f));
	AL_CHECK(alListener3f(AL_VELOCITY, 0, 0, 0));
	AL_CHECK(alListenerfv(AL_ORIENTATION, dir));

}

//-----------------------------------------------------------------------------
void ALRenderer::shutdown()
{
    AL_CHECK(alcDestroyContext(m_context));
    AL_CHECK(alcCloseDevice(m_device));
}

//-----------------------------------------------------------------------------
SoundId ALRenderer::create_sound(const void* data, uint32_t size, uint32_t sample_rate, uint32_t channels, uint32_t bxs)
{
	SoundId id = m_sounds_id_table.create();

	Sound& al_sound = m_sounds[id.index];

	// Generates AL buffer
	AL_CHECK(alGenBuffers(1, &al_sound.bufferid));

	bool stereo = (channels > 1);

	// Sets sound's format
	switch(bxs)
	{
		case 8:
		{
			if (stereo)
			{
				al_sound.format = AL_FORMAT_STEREO8;
			}
			else
			{
				al_sound.format = AL_FORMAT_MONO8;
			}

			break;
		}

		case 16:
		{
			if (stereo)
			{
				al_sound.format = AL_FORMAT_STEREO16;
			}
			else
			{
				al_sound.format = AL_FORMAT_MONO16;
			}

			break;
		}

		default:
		{
			CE_ASSERT(false, "Wrong number of bits per sample.");
			break;
		}
	}

	// Sets sound's size
	al_sound.size = size;

	// Sets sound's frequency
	al_sound.freq = sample_rate;

	// Fills AL buffer
	AL_CHECK(alBufferData(al_sound.bufferid, al_sound.format, data, al_sound.size, al_sound.freq));

	// Creates AL source
	AL_CHECK(alGenSources(1, &al_sound.sourceid));

	// Sets tmp source's properties
    AL_CHECK(alSourcef(al_sound.sourceid, AL_PITCH, 1));

    AL_CHECK(alSourcef(al_sound.sourceid, AL_GAIN, 1));

	AL_CHECK(alSource3f(al_sound.sourceid, AL_POSITION, 0, 0, 0));

	AL_CHECK(alSourcei(al_sound.sourceid, AL_LOOPING, AL_FALSE));


	// Binds buffer to sources
	AL_CHECK(alSourcei(al_sound.sourceid, AL_BUFFER, al_sound.bufferid));


	return id;
}

//-----------------------------------------------------------------------------
void ALRenderer::play_sound(SoundId id)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exist");

	Sound& al_sound = m_sounds[id.index];

	AL_CHECK(alSourcePlay(al_sound.sourceid));
}

//-----------------------------------------------------------------------------
void ALRenderer::pause_sound(SoundId id)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exist");

	Sound& al_sound = m_sounds[id.index];

	if (is_sound_playing(id))
	{
		AL_CHECK(alSourcePause(al_sound.sourceid));
	}
}

//-----------------------------------------------------------------------------
void ALRenderer::destroy_sound(SoundId id)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exist");

	Sound& al_sound = m_sounds[id.index];

	// alDeleteSources(1, al_sound.sourceid);
	// alDeleteBuffers(1, al_sound.bufferid);

	

	// Must be implemented
}

//-----------------------------------------------------------------------------
bool ALRenderer::is_sound_playing(SoundId id)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exist");

	Sound& al_sound = m_sounds[id.index];

	ALint source_state;
	alGetSourcei(al_sound.sourceid, AL_SOURCE_STATE, &source_state);

	return source_state == AL_PLAYING;
}


} // namespace crown