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

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <cstring>

#include "Types.h"
#include "Assert.h"
#include "SoundRenderer.h"
#include "SLESRenderer.h"
#include "Vector3.h"

namespace crown
{

//-----------------------------------------------------------------------------
class SoundRendererImpl
{
public:
	//-----------------------------------------------------------------------------
	void init()
	{
		// SLresult result;

		// const SLInterfaceID ids[] = {SL_IID_ENGINE};
		// const SLboolean reqs[] = {SL_BOOLEAN_TRUE};

		// result = slCreateEngine(&m_engine_obj, 0, NULL, 1, ids, reqs);
		// result = (*m_engine_obj)->Realize(m_engine_obj, SL_BOOLEAN_FALSE);

		// result = (*m_engine_obj)->GetInterface(m_engine_obj, SL_IID_ENGINE, &m_engine);

		// const SLInterfaceID ids1[] = {SL_IID_VOLUME};
		// const SLboolean reqs1[] = {SL_BOOLEAN_FALSE};

		// result = (*m_engine)->CreateOutputMix(m_engine, &m_out_mix_obj, 1, ids1, reqs1); 
		// result = (*m_out_mix_obj)->Realize(m_out_mix_obj, SL_BOOLEAN_FALSE);
	}

	//-----------------------------------------------------------------------------
	void shutdown()
	{
		// for (uint32_t i = 0; i < MAX_SOUNDS; i++)
		// {
		// 	m_sounds[i].destroy();
		// }

		// if (m_out_mix_obj)
		// {
		// 	(*m_out_mix_obj)->Destroy(m_out_mix_obj);
		// 	m_out_mix_obj = NULL;
		// }

		// if (m_engine_obj)
		// {
		// 	(*m_engine_obj)->Destroy(m_engine_obj);
		// 	m_engine_obj = NULL;
		// 	m_engine = NULL;
		// }
	}

public:

	SLObjectItf						m_engine_obj;
	SLEngineItf						m_engine;

	SLObjectItf 					m_out_mix_obj;

private:

	friend class SoundRenderer;
};

SoundRenderer::SoundRenderer(Allocator& allocator) : m_allocator(allocator) {}
SoundRenderer::~SoundRenderer() {}
void SoundRenderer::init() {}
void SoundRenderer::shutdown() {}
void SoundRenderer::pause() {}
void SoundRenderer::unpause() {}
void SoundRenderer::frame() {}
void SoundRenderer::set_listener(const Vector3& pos, const Vector3& vel, const Vector3& or_up, const Vector3& or_at) const {}
SoundBufferId SoundRenderer::create_sound_buffer(void* data, size_t size, uint32_t sample_rate, uint32_t num_channels, uint16_t bits_ps) {}
void SoundRenderer::destroy_sound_buffer(SoundBufferId id) {}
SoundSourceId SoundRenderer::create_sound_source() {}
void SoundRenderer::destroy_sound_source(SoundSourceId id) {}
void SoundRenderer::bind_buffer(SoundBufferId buffer, SoundSourceId source) {}
void SoundRenderer::unbind_buffer(SoundSourceId id) {}
void SoundRenderer::play_sound(SoundSourceId id) {}
void SoundRenderer::pause_sound(SoundSourceId id) {}
void SoundRenderer::set_sound_loop(SoundSourceId id, bool loop) {}
void SoundRenderer::set_sound_min_distance(SoundSourceId id, const float min_distance) {}
void SoundRenderer::set_sound_max_distance(SoundSourceId id, const float max_distance) {}
void SoundRenderer::set_sound_position(SoundSourceId id, const Vector3& pos) {}
void SoundRenderer::set_sound_velocity(SoundSourceId id, const Vector3& vel) {}
void SoundRenderer::set_sound_direction(SoundSourceId id, const Vector3& dir) {}
void SoundRenderer::set_sound_pitch(SoundSourceId id, const float pitch) {}
void SoundRenderer::set_sound_gain(SoundSourceId id, const float gain) {}
void SoundRenderer::set_sound_rolloff(SoundSourceId id, const float rolloff) {}
float SoundRenderer::sound_min_distance(SoundSourceId id) const {}
float SoundRenderer::sound_max_distance(SoundSourceId id) const {}
Vector3 SoundRenderer::sound_position(SoundSourceId id) const {}
Vector3 SoundRenderer::sound_velocity(SoundSourceId id) const {}
Vector3 SoundRenderer::sound_direction(SoundSourceId id) const {}
float SoundRenderer::sound_pitch(SoundSourceId id) const {}
float SoundRenderer::sound_gain(SoundSourceId id) const {}
float SoundRenderer::sound_rolloff(SoundSourceId id) const {}
int32_t SoundRenderer::sound_queued_buffers(SoundSourceId id) const {}
int32_t SoundRenderer::sound_processed_buffers(SoundSourceId id) const {}
bool SoundRenderer::sound_playing(SoundSourceId id) {}

} // namespace crown