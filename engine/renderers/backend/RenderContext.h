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

#include <algorithm>

#include "Color4.h"
#include "Matrix4x4.h"
#include "CommandBuffer.h"
#include "ConstantBuffer.h"
#include "RendererTypes.h"
#include "Log.h"

namespace crown
{

#define MAX_RENDER_LAYERS						8
#define MAX_RENDER_STATES						1024

// Layer flags
#define RENDER_LAYER_MASK						0xE000000000000000
#define RENDER_LAYER_SHIFT						61

// State flags
#define STATE_NONE								0x0000000000000000

#define STATE_DEPTH_WRITE						0x0000000000000001
#define STATE_COLOR_WRITE						0x0000000000000002
#define STATE_ALPHA_WRITE						0x0000000000000004

#define STATE_CULL_CW							0x0000000000000010
#define STATE_CULL_CCW							0x0000000000000020

#define STATE_TEXTURE_0							0x0000000000000100
#define STATE_TEXTURE_1							0x0000000000000200
#define STATE_TEXTURE_2							0x0000000000000400
#define STATE_TEXTURE_3							0x0000000000000800
#define STATE_TEXTURE_MASK						0x0000000000000F00
#define STATE_MAX_TEXTURES						4

#define STATE_PRIMITIVE_TRIANGLES				0x0000000000000000
#define STATE_PRIMITIVE_POINTS					0x0000000000001000
#define STATE_PRIMITIVE_LINES					0x0000000000002000
#define STATE_PRIMITIVE_MASK					0x000000000000F000
#define STATE_PRIMITIVE_SHIFT					12

#define STATE_BLEND_FUNC_ZERO					0x0000000000010000			
#define STATE_BLEND_FUNC_ONE					0x0000000000020000
#define STATE_BLEND_FUNC_SRC_COLOR				0x0000000000030000
#define STATE_BLEND_FUNC_ONE_MINUS_SRC_COLOR	0x0000000000040000
#define STATE_BLEND_FUNC_DST_COLOR				0x0000000000050000
#define STATE_BLEND_FUNC_ONE_MINUS_DST_COLOR	0x0000000000060000
#define STATE_BLEND_FUNC_SRC_ALPHA				0x0000000000070000
#define STATE_BLEND_FUNC_ONE_MINUS_SRC_ALPHA	0x0000000000080000
#define STATE_BLEND_FUNC_DST_ALPHA				0x0000000000090000
#define STATE_BLEND_FUNC_ONE_MINUS_DST_ALPHA	0x00000000000A0000
#define STATE_BLEND_FUNC_MASK					0x0000000000FF0000
#define STATE_BLEND_FUNC_SHIFT					16

#define STATE_BLEND_EQUATION_ADD				0x0000000001000000
#define STATE_BLEND_EQUATION_SUBTRACT			0x0000000002000000
#define STATE_BLEND_EQUATION_REVERSE_SUBTRACT	0x0000000003000000
#define STATE_BLEND_EQUATION_MASK				0x0000000003000000
#define STATE_BLEND_EQUATION_SHIFT				24

#define STATE_DEPTH_TEST_NEVER					0x0000000010000000
#define STATE_DEPTH_TEST_LESS					0x0000000020000000
#define STATE_DEPTH_TEST_EQUAL					0x0000000030000000
#define STATE_DEPTH_TEST_LEQUAL					0x0000000040000000
#define STATE_DEPTH_TEST_GREATER				0x0000000050000000
#define STATE_DEPTH_TEST_NOTEQUAL				0x0000000060000000
#define STATE_DEPTH_TEST_GEQUAL					0x0000000070000000
#define STATE_DEPTH_TEST_ALWAYS					0x0000000080000000
#define STATE_DEPTH_TEST_MASK					0x00000000F0000000
#define STATE_DEPTH_TEST_SHIFT					28

#define STATE_BLEND_FUNC(src, dst) (uint64_t(src << 4) | uint64_t(dst))

#define CLEAR_COLOR					0x1
#define CLEAR_DEPTH					0x2

// Texture flags
#define TEXTURE_FILTER_NEAREST		0x00000001
#define TEXTURE_FILTER_LINEAR		0x00000002
#define TEXTURE_FILTER_BILINEAR		0x00000003
#define TEXTURE_FILTER_TRILINEAR	0x00000004
#define TEXTURE_FILTER_MASK			0x0000000F
#define TEXTURE_FILTER_SHIFT		0

#define TEXTURE_WRAP_U_CLAMP_EDGE	0x00000010
#define TEXTURE_WRAP_U_CLAMP_REPEAT	0x00000020
#define TEXTURE_WRAP_U_MASK			0x00000030
#define TEXTURE_WRAP_U_SHIFT		4
#define TEXTURE_WRAP_V_CLAMP_EDGE	0x00000100
#define TEXTURE_WRAP_V_CLAMP_REPEAT	0x00000200
#define TEXTURE_WRAP_V_MASK			0x00000300
#define TEXTURE_WRAP_V_SHIFT		8

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
		m_flags = 0;
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

/// Maintains the states necessary for a single draw call.
struct RenderState
{
	void clear()
	{
		m_flags = STATE_NONE;

		pose = matrix4x4::IDENTITY;
		program.id = INVALID_ID;
		vb.id = INVALID_ID;
		ib.id = INVALID_ID;
		start_vertex = 0;
		num_vertices = 0xFFFFFFFF;
		start_index = 0;
		num_indices = 0xFFFFFFFF;
		vertex_format = VertexFormat::COUNT;

		for (uint32_t i = 0; i < STATE_MAX_TEXTURES; i++)
		{
			samplers[i].sampler_id.id = INVALID_ID;
			samplers[i].flags = SAMPLER_TEXTURE;
		}
	}

public:

	uint64_t		m_flags;

	Matrix4x4		pose;
	GPUProgramId	program;
	VertexBufferId	vb;
	IndexBufferId	ib;
	uint32_t		start_vertex;
	uint32_t		num_vertices;
	uint32_t		start_index;
	uint32_t		num_indices;
	VertexFormat::Enum vertex_format;
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
		return uint64_t(m_layer) << RENDER_LAYER_SHIFT;
	}

	void decode(uint64_t key)
	{
		m_layer = (key & RENDER_LAYER_MASK) >> RENDER_LAYER_SHIFT;
	}

public:

	uint8_t m_layer;
};

/// A vertex buffer valid for one frame only
struct TransientVertexBuffer
{
	VertexBufferId vb;
	VertexFormat::Enum format;
	uint32_t start_vertex;
	size_t size;
	char* data;
};

/// An index buffer valid for one frame only
struct TransientIndexBuffer
{
	IndexBufferId ib;
	uint32_t start_index;
	size_t size;
	char* data;
};

struct RenderContext
{
	RenderContext()
	{
		clear();
	}

	uint32_t reserve_transient_vertex_buffer(uint32_t num, VertexFormat::Enum format)
	{
		const uint32_t offset = m_tvb_offset;
		m_tvb_offset = offset + Vertex::bytes_per_vertex(format) * num;
		return offset;
	}

	uint32_t reserve_transient_index_buffer(uint32_t num)
	{
		const uint32_t offset = m_tib_offset;
		m_tib_offset = offset + sizeof(uint16_t) * num;
		return offset;
	}

	void set_state(uint64_t flags)
	{
		m_state.m_flags = flags;
	}

	void set_pose(const Matrix4x4& pose)
	{
		m_state.pose = pose;
	}

	void set_program(GPUProgramId program)
	{
		m_state.program = program;
	}

	void set_vertex_buffer(VertexBufferId vb, uint32_t num_vertices)
	{
		m_state.vb = vb;
		m_state.start_vertex = 0;
		m_state.num_vertices = num_vertices;
	}

	void set_vertex_buffer(const TransientVertexBuffer& tvb, uint32_t num_vertices)
	{
		m_state.vb = tvb.vb;
		m_state.start_vertex = tvb.start_vertex;
		m_state.num_vertices = math::min((uint32_t) tvb.size / (uint32_t) Vertex::bytes_per_vertex(tvb.format), num_vertices);
		m_state.vertex_format = tvb.format;
	}

	void set_index_buffer(IndexBufferId ib, uint32_t start_index, uint32_t num_indices)
	{
		m_state.ib = ib;
		m_state.start_index = start_index;
		m_state.num_indices = num_indices;
	}

	void set_index_buffer(const TransientIndexBuffer& tib, uint32_t num_indices)
	{
		m_state.ib = tib.ib;
		m_state.start_index = tib.start_index;
		m_state.num_indices = math::min((uint32_t) tib.size / (uint32_t) sizeof(uint16_t), num_indices);
	}

	void set_uniform(UniformId id, UniformType::Enum type, const void* value, uint8_t num)
	{
		m_constants.write_constant(id, type, value, num);
	}

	void set_texture(uint8_t unit, UniformId sampler_uniform, TextureId texture, uint32_t flags)
	{
		m_flags |= STATE_TEXTURE_0 << unit;

		Sampler& sampler = m_state.samplers[unit];
		sampler.sampler_id = texture;
		sampler.flags |= SAMPLER_TEXTURE | flags;

		const uint32_t val = unit;
		set_uniform(sampler_uniform, UniformType::INTEGER_1, &val, 1);
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

	void set_layer_view(uint8_t layer, const Matrix4x4& view)
	{
		CE_ASSERT(layer < MAX_RENDER_LAYERS, "Layer out of bounds");

		m_view_matrices[layer] = view;
	}

	void set_layer_projection(uint8_t layer, const Matrix4x4& projection)
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
		CE_ASSERT(layer < MAX_RENDER_LAYERS, "Layer out of bounds");
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

		m_tvb_offset = 0;
		m_tib_offset = 0;
	}

	void push()
	{
		m_commands.commit();
		m_constants.commit();
	}

	void sort()
	{
		std::sort(m_keys, m_keys + m_num_states);
	}

public:

	uint64_t m_flags;
	RenderKey m_render_key;
	RenderState m_state;

	// Per-state data
	uint32_t m_num_states;
	RenderState m_states[MAX_RENDER_STATES];
	uint64_t m_keys[MAX_RENDER_STATES];

	// Per-layer data
	RenderTargetId m_targets[MAX_RENDER_LAYERS];
	Matrix4x4 m_view_matrices[MAX_RENDER_LAYERS];
	Matrix4x4 m_projection_matrices[MAX_RENDER_LAYERS];
	ViewRect m_viewports[MAX_RENDER_LAYERS];
	ViewRect m_scissors[MAX_RENDER_LAYERS];
	ClearState m_clears[MAX_RENDER_LAYERS];

	CommandBuffer m_commands;
	ConstantBuffer m_constants;

	uint32_t m_tvb_offset;
	uint32_t m_tib_offset;
	TransientVertexBuffer* m_transient_vb;
	TransientIndexBuffer* m_transient_ib;
};

} // namespace crown
