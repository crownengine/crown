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

#include "SLESRenderer.h"

namespace crown
{

//-----------------------------------------------------------------------------
static const char* sl_error_to_string(SLresult error)
{
	switch (error)
	{
		case SL_RESULT_PARAMETER_INVALID: 	return "SL_RESULT_PARAMETER_INVALID";
		case SL_RESULT_MEMORY_FAILURE: 		return "SL_RESULT_MEMORY_FAILURE";
		case SL_RESULT_FEATURE_UNSUPPORTED: return "SL_RESULT_FEATURE_UNSUPPORTED";
		case SL_RESULT_RESOURCE_ERROR: 		return "SL_RESULT_RESOURCE_ERROR";
		default: 							return "SL_UNKNOWN_ERROR";
	}
}

//-----------------------------------------------------------------------------
#ifdef CROWN_DEBUG
	#define SL_CHECK(function)\
		do { SLresult error; CE_ASSERT((error = function()) == SL_RESULT_SUCCESS,\
				"OpenSLES error: %s", sl_error_to_string(error)); } while (0)
#else
	#define SL_CHECK(function)\
		function;
#endif

//-----------------------------------------------------------------------------
AudioRenderer* AudioRenderer::create(Allocator& a)
{
	return CE_NEW(a, SLESRenderer);
}

//-----------------------------------------------------------------------------
void AudioRenderer::destroy(Allocator& a, AudioRenderer* renderer)
{
	CE_DELETE(a, renderer);
}

//-----------------------------------------------------------------------------
SLESRenderer::SLESRenderer() :
	m_buffers_id_table(m_allocator, MAX_BUFFERS),
	m_sources_id_table(m_allocator, MAX_SOURCES)
{

}

//-----------------------------------------------------------------------------
void SLESRenderer::init()
{
	slCreateEngine(&m_engine_obj, 0, NULL, 0, NULL, NULL);
	(*m_engine_obj)->Realize(m_engine_obj, SL_BOOLEAN_FALSE);

	(*m_engine_obj)->GetInterface(m_engine_obj, SL_IID_ENGINE, &m_engine);

    const SLInterfaceID ids[5] = {SL_IID_NULL, SL_IID_NULL, SL_IID_NULL, SL_IID_NULL, SL_IID_NULL};
    const SLboolean req[5] = {SL_BOOLEAN_FALSE, SL_BOOLEAN_FALSE, SL_BOOLEAN_FALSE, SL_BOOLEAN_FALSE, SL_BOOLEAN_FALSE};

    (*m_engine)->CreateOutputMix(m_engine, &m_out_mix_obj, 1, ids, req); 
    (*m_out_mix_obj)->Realize(m_out_mix_obj, SL_BOOLEAN_FALSE);

    (*m_out_mix_obj)->GetInterface(m_out_mix_obj, SL_IID_ENVIRONMENTALREVERB, &m_out_mix_env_reverb);
}

//-----------------------------------------------------------------------------
void SLESRenderer::shutdown()
{

}

//-----------------------------------------------------------------------------
void SLESRenderer::set_listener(const Vec3& pos, const Vec3& vel, const Vec3& or_up, const Vec3& or_at) const
{

}

//-----------------------------------------------------------------------------
SoundBufferId SLESRenderer::create_buffer(const void* data, const uint32_t size, const uint32_t sample_rate, const uint32_t channels, const uint32_t bxs)
{

}

//-----------------------------------------------------------------------------
void SLESRenderer::destroy_buffer(SoundBufferId id)
{

}

//-----------------------------------------------------------------------------
SoundSourceId SLESRenderer::create_source(const Vec3& pos, const Vec3& vel, const Vec3& dir, const bool loop)
{

}

//-----------------------------------------------------------------------------
void SLESRenderer::play_source(SoundSourceId id)
{

}

//-----------------------------------------------------------------------------
void SLESRenderer::pause_source(SoundSourceId id)
{

}

//-----------------------------------------------------------------------------
void SLESRenderer::destroy_source(SoundSourceId id)
{

}

//-----------------------------------------------------------------------------
void SLESRenderer::bind_buffer(SoundSourceId sid, SoundBufferId bid)
{

}

//-----------------------------------------------------------------------------
void SLESRenderer::set_source_min_distance(SoundSourceId id,  const float min_distance)
{

}

//-----------------------------------------------------------------------------
void SLESRenderer::set_source_max_distance(SoundSourceId id,  const float max_distance)
{

}

//-----------------------------------------------------------------------------
void SLESRenderer::set_source_position(SoundSourceId id, const Vec3& pos)
{

}

//-----------------------------------------------------------------------------
void SLESRenderer::set_source_velocity(SoundSourceId id, const Vec3& vel)
{

}

//-----------------------------------------------------------------------------
void SLESRenderer::set_source_direction(SoundSourceId id, const Vec3& dir)
{

}

//-----------------------------------------------------------------------------
void SLESRenderer::set_source_pitch(SoundSourceId id, const float pitch)
{

}

//-----------------------------------------------------------------------------
void SLESRenderer::set_source_gain(SoundSourceId id, const float gain)
{

}

//-----------------------------------------------------------------------------
void SLESRenderer::set_source_rolloff(SoundSourceId id, const float rolloff)
{

}

//-----------------------------------------------------------------------------
bool SLESRenderer::source_playing(SoundSourceId id)
{

}

} // namespace crown