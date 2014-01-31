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

#include "Matrix4x4.h"
#include "Quaternion.h"
#include "RendererTypes.h"
#include "StringUtils.h"
#include "Log.h"
#include "FontResource.h"

namespace crown
{

struct Vector3;
struct Vector2;
struct Color4;

//-------------------------------------------------------------------------
struct VertexData
{
	float x;
	float y;
	float u;
	float v;
};

//-------------------------------------------------------------------------
struct IndexData
{
	uint16_t a;
	uint16_t b;
};

#define UTF8_ACCEPT 0

//-------------------------------------------------------------------------
static const uint8_t s_utf8d[364] =
{
	// The first part of the table maps bytes to character classes that
	// to reduce the size of the transition table and create bitmasks.
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

	// The second part is a transition table that maps a combination
	// of a state of the automaton and a character class to a state.
	 0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
	12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
	12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
	12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
	12,36,12,12,12,12,12,12,12,12,12,12
};

//-------------------------------------------------------------------------
uint32_t utf8_decode(uint32_t* state, uint32_t* code_point, uint8_t character)
{
	uint32_t byte = character;
	uint32_t type = s_utf8d[byte];

	*code_point = (*state != UTF8_ACCEPT) ? (byte & 0x3fu) | (*code_point << 6) : (0xff >> type) & (byte);
	*state = s_utf8d[256 + *state + type];

	return *state;
}

//-------------------------------------------------------------------------
struct GuiText
{
	//-------------------------------------------------------------------------
	GuiText(RenderWorld& render_world, Renderer& r, const char* str, const FontResource* fr, uint32_t font_size, const Vector3& pos)
		: m_render_world(render_world)
		, m_r(r)
		, m_resource(fr)
		, m_str(str)
		, m_font_size(font_size)
		, m_pos(pos)
	{
		MaterialResource* mat = (MaterialResource*) device()->resource_manager()->data(m_resource->material());
		m_material = m_render_world.create_material(mat);
	}

	//-------------------------------------------------------------------------
	~GuiText()
	{
		m_render_world.destroy_material(m_material);
	}

	//-------------------------------------------------------------------------
	void update(const char* str, uint32_t font_size, const Vector3& pos)
	{
		m_str = str;
		m_font_size = font_size;
		m_pos = pos;
	}

	//-------------------------------------------------------------------------
	void render(UniformId uniform)
	{
		Material* material = m_render_world.lookup_material(m_material);
		material->bind(m_r, uniform);

		const char* str = m_str.c_str();
		const float scale = ((float)m_font_size / (float)m_resource->font_size());

		TransientVertexBuffer vb;
		TransientIndexBuffer ib;

		m_r.reserve_transient_vertex_buffer(&vb, 4 * string::strlen(str), VertexFormat::P2_T2);
		m_r.reserve_transient_index_buffer(&ib, 6 * string::strlen(str));

		uint16_t index = 0;
		float x_pen_advance = 0.0f;
		float y_pen_advance = 0.0f;

		uint32_t state = 0;
		uint32_t code_point = 0;
		for (uint32_t i = 0; i < string::strlen(str); i++)
		{
			switch (str[i])
			{
				case '\n':
				{
					x_pen_advance = 0.0f;
					y_pen_advance += m_resource->font_size();
					continue;
				}
				case '\t':
				{
					x_pen_advance += m_font_size * 4;
					continue;
				}
			}
			
			if (utf8_decode(&state, &code_point, str[i]) == UTF8_ACCEPT)
			{
				FontGlyphData g = m_resource->get_glyph(code_point);

				// Set pen position
				m_pen.x = m_pos.x + g.x_offset;
				m_pen.y = m_pos.y + (g.height - g.y_offset);

				// Position coords
				const float x0 = (m_pen.x + x_pen_advance) * scale;
				const float y0 = (m_pen.y + y_pen_advance) * scale;
				const float x1 = (m_pen.x + g.width + x_pen_advance) * scale;
				const float y1 = (m_pen.y - g.height + y_pen_advance) * scale;

				// Texture coords
				const float u0 = (float) g.x / m_resource->texture_size();
				const float v0 = (float) g.y / m_resource->texture_size();
				const float u1 = u0 + ((float) g.width) / m_resource->texture_size();
				const float v1 = v0 - ((float) g.height) / m_resource->texture_size();

				// Fill vertex buffer
				(*(VertexData*)(vb.data)).x		= x0;
				(*(VertexData*)(vb.data)).y		= y0;
				(*(VertexData*)(vb.data)).u		= u0;
				(*(VertexData*)(vb.data)).v		= v1;
				vb.data += sizeof(VertexData);

				(*(VertexData*)(vb.data)).x		= x1;
				(*(VertexData*)(vb.data)).y		= y0;
				(*(VertexData*)(vb.data)).u		= u1; 
				(*(VertexData*)(vb.data)).v		= v1;
				vb.data += sizeof(VertexData);

				(*(VertexData*)(vb.data)).x		= x1;
				(*(VertexData*)(vb.data)).y		= y1;
				(*(VertexData*)(vb.data)).u		= u1;
				(*(VertexData*)(vb.data)).v		= v0;
				vb.data += sizeof(VertexData);

				(*(VertexData*)(vb.data)).x		= x0;
				(*(VertexData*)(vb.data)).y		= y1;
				(*(VertexData*)(vb.data)).u		= u0;
				(*(VertexData*)(vb.data)).v		= v0;
				vb.data += sizeof(VertexData);

				// Fill index buffer
				(*(IndexData*)(ib.data)).a		= index;
				(*(IndexData*)(ib.data)).b		= index + 1;
				ib.data += sizeof(IndexData);

				(*(IndexData*)(ib.data)).a		= index + 2;
				(*(IndexData*)(ib.data)).b		= index;
				ib.data += sizeof(IndexData);

				(*(IndexData*)(ib.data)).a		= index + 2;
				(*(IndexData*)(ib.data)).b		= index + 3;
				ib.data += sizeof(IndexData);

				// Advance pen position
				x_pen_advance += g.x_advance;

				index += 4;
			}
		}

		m_r.set_vertex_buffer(vb);
		m_r.set_index_buffer(ib);
		m_r.commit(1);
	}

public:

	RenderWorld& 		m_render_world;
	Renderer& 			m_r;
	const FontResource* m_resource;

	DynamicString		m_str;
	uint32_t			m_font_size;

	// position states
	Vector3				m_pos;
	Vector2				m_pen;

	MaterialId 			m_material;
};

} // namespace crown

