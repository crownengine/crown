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

#include "Color4.h"
#include "Mat4.h"
#include "IdTable.h"

namespace crown
{

typedef Id VertexBufferId;
typedef Id IndexBufferId;
typedef Id RenderTargetId;
typedef Id TextureId;
typedef Id ShaderId;
typedef Id GPUProgramId;
typedef Id UniformId;

enum UniformType
{
	UNIFORM_INTEGER,
	UNIFORM_UNSIGNED,
	UNIFORM_FLOAT,
	UNIFORM_MAT3,
	UNIFORM_MAT4
};

struct Uniform
{
	uint32_t m_name;
	UniformType m_type;
	uint32_t m_size;
};

#define MAX_RENDER_LAYERS			32
#define MAX_RENDER_STATES			1024

// State flags
#define STATE_NONE					0x0000000000000000

#define STATE_DEPTH_WRITE			0x0000000000000001
#define STATE_COLOR_WRITE			0x0000000000000002
#define STATE_ALPHA_WRITE			0x0000000000000004

#define STATE_CULL_CW				0x0000000000000010
#define STATE_CULL_CCW				0x0000000000000020

#define STATE_TEXTURE_0				0x0000000000000100
#define STATE_TEXTURE_1				0x0000000000000200
#define STATE_TEXTURE_2				0x0000000000000400
#define STATE_TEXTURE_3				0x0000000000000800
#define STATE_TEXTURE_MASK			0x0000000000000F00
#define STATE_MAX_TEXTURES			4

#define STATE_PRIMITIVE_TRIANGLES	0x0000000000000000
#define STATE_PRIMITIVE_POINTS		0x0000000000001000
#define STATE_PRIMITIVE_LINES		0x0000000000002000
#define STATE_PRIMITIVE_MASK		0x000000000000f000
#define STATE_PRIMITIVE_SHIFT		12

#define CLEAR_COLOR					0x1
#define CLEAR_DEPTH					0x2

// Texture flags
#define TEXTURE_FILTER_NEAREST		0x00000001
#define TEXTURE_FILTER_LINEAR		0x00000002
#define TEXTURE_FILTER_BILINEAR		0x00000003
#define TEXTURE_FILTER_TRILINEAR	0x00000004
#define TEXTURE_FILTER_MASK			0x0000000F
#define TEXTURE_FILTER_SHIFT		0

#define TEXTURE_WRAP_CLAMP			0x00000010
#define TEXTURE_WRAP_CLAMP_EDGE		0x00000020
#define TEXTURE_WRAP_CLAMP_BORDER	0x00000030
#define TEXTURE_WRAP_CLAMP_REPEAT	0x00000040
#define TEXTURE_WRAP_MASK			0x000000F0
#define TEXTURE_WRAP_SHIFT			4

// Sampler flags
#define SAMPLER_TEXTURE				0x10000000
#define SAMPLER_MASK				0xF0000000
#define SAMPLER_SHIFT				28

struct ViewRect
{
	void clear()
	{
		m_x = 0;
		m_y = 0;
		m_width = 0;
		m_height = 0;
	}

	uint32_t area() const
	{
		return (m_width - m_x) * (m_height - m_y);
	}

	uint16_t m_x;
	uint16_t m_y;
	uint16_t m_width;
	uint16_t m_height;
};

struct ClearState
{
	void clear()
	{
		m_color = Color4::GRAY;
		m_depth = 1.0f;
	}

public:

	uint8_t		m_flags;
	Color4		m_color;
	float		m_depth;
};

struct Sampler
{
	Id			sampler_id;
	uint32_t	flags;
};

struct RenderState
{
	void clear()
	{
		m_flags = STATE_NONE;

		pose = Mat4::IDENTITY;
		program.id = INVALID_ID;
		vb.id = INVALID_ID;
		ib.id = INVALID_ID;

		for (uint32_t i = 0; i < STATE_MAX_TEXTURES; i++)
		{
			samplers[i].sampler_id.id = INVALID_ID;
			samplers[i].flags = SAMPLER_TEXTURE;
		}
	}

public:

	uint64_t		m_flags;

	Mat4			pose;
	GPUProgramId	program;
	VertexBufferId	vb;
	IndexBufferId	ib;
	Sampler			samplers[STATE_MAX_TEXTURES];
};

struct RenderKey
{
	void clear()
	{
		m_layer = 0;
	}

	uint64_t encode()
	{
		return uint64_t(m_layer) << 56;
	}

	void decode(uint64_t key)
	{
		m_layer = (key >> 56) & 0xFF;
	}

public:

	uint8_t m_layer;
};

struct RenderContext
{
	RenderContext()
	{
		clear();
	}

	void set_state(uint64_t flags)
	{
		m_state.m_flags = flags;
	}

	void set_pose(const Mat4& pose)
	{
		m_state.pose = pose;
	}

	void set_program(GPUProgramId program)
	{
		m_state.program = program;
	}

	void set_vertex_buffer(VertexBufferId vb)
	{
		m_state.vb = vb;
	}

	void set_index_buffer(IndexBufferId ib)
	{
		m_state.ib = ib;
	}

	void set_texture(uint8_t unit, TextureId texture, uint32_t flags)
	{
		m_flags |= STATE_TEXTURE_0 << unit;

		Sampler& sampler = m_state.samplers[unit];
		sampler.sampler_id = texture;
		sampler.flags |= SAMPLER_TEXTURE | flags;
	}

	void set_layer_render_target(uint8_t layer, RenderTargetId target)
	{
		CE_ASSERT(layer < MAX_RENDER_LAYERS, "Layer out of bounds");

		m_targets[layer] = target;
	}

	void set_layer_clear(uint8_t layer, uint8_t flags, const Color4& color, float depth)
	{
		CE_ASSERT(layer < MAX_RENDER_LAYERS, "Layer out of bounds");

		m_clears[layer].m_flags = flags;
		m_clears[layer].m_color = color;
		m_clears[layer].m_depth = depth;
	}

	void set_layer_view(uint8_t layer, const Mat4& view)
	{
		CE_ASSERT(layer < MAX_RENDER_LAYERS, "Layer out of bounds");

		m_view_matrices[layer] = view;
	}

	void set_layer_projection(uint8_t layer, const Mat4& projection)
	{
		CE_ASSERT(layer < MAX_RENDER_LAYERS, "Layer out of bounds");

		m_projection_matrices[layer] = projection;
	}

	void set_layer_viewport(uint8_t layer, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		CE_ASSERT(layer < MAX_RENDER_LAYERS, "Layer out of bounds");

		m_viewports[layer].m_x = x;
		m_viewports[layer].m_y = y;
		m_viewports[layer].m_width = width;
		m_viewports[layer].m_height = height;
	}

	void set_layer_scissor(uint8_t layer, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		CE_ASSERT(layer < MAX_RENDER_LAYERS, "Layer out of bounds");

		m_scissors[layer].m_x = x;
		m_scissors[layer].m_y = y;
		m_scissors[layer].m_width = width;
		m_scissors[layer].m_height = height;
	}

	void commit(uint8_t layer)
	{
		m_render_key.m_layer = layer;

		m_states[m_num_states] = m_state;
		m_keys[m_num_states] = m_render_key.encode();
		m_num_states++;

		m_render_key.clear();
		m_state.clear();
	}

	void clear()
	{
		m_flags = STATE_NONE;
		m_render_key.clear();

		m_num_states = 0;
		m_state.clear();
	}

public:

	uint64_t m_flags;
	RenderKey m_render_key;
	RenderState m_state;

	// Per-state data
	uint32_t m_num_states;
	RenderState m_states[MAX_RENDER_STATES];
	ClearState m_clears[MAX_RENDER_STATES];
	uint64_t m_keys[MAX_RENDER_STATES];

	// Per-layer data
	RenderTargetId m_targets[MAX_RENDER_LAYERS];
	Mat4 m_view_matrices[MAX_RENDER_LAYERS];
	Mat4 m_projection_matrices[MAX_RENDER_LAYERS];
	ViewRect m_viewports[MAX_RENDER_LAYERS];
	ViewRect m_scissors[MAX_RENDER_LAYERS];
};

} // namespace crown
