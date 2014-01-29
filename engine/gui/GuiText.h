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
#include "RendererTypes.h"
#include "StringUtils.h"
#include "Log.h"
#include "FontResource.h"

namespace crown
{

struct Vector3;
struct Vector2;
struct Color4;

struct VertexData
{
	float x;
	float y;
	float u;
	float v;
};

struct IndexData
{
	uint16_t a;
	uint16_t b;
};

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

		TransientVertexBuffer vb;
		TransientIndexBuffer ib;

		m_r.reserve_transient_vertex_buffer(&vb, 4 * string::strlen(str), VertexFormat::P2_T2);
		m_r.reserve_transient_index_buffer(&ib, 6 * string::strlen(str));

		uint16_t index = 0;
		float advance = 0.0f;
		for (uint32_t i = 0; i < string::strlen(str); i++)
		{
			FontGlyphData g = m_resource->get_glyph(str[i]);

			const float x		= (float) g.x / m_resource->size();
			const float y		= (float) g.y / m_resource->size();
			const float width	= (float) g.width / m_resource->size();
			const float height	= (float) g.height / m_resource->size();

			const float u0 = x;
			const float v0 = y;
			const float u1 = x + width;
			const float v1 = y - height;

			(*(VertexData*)(vb.data)).x		= m_pos.x + advance;
			(*(VertexData*)(vb.data)).y		= m_pos.y;
			(*(VertexData*)(vb.data)).u		= u0;
			(*(VertexData*)(vb.data)).v		= v1;
			vb.data += sizeof(VertexData);

			(*(VertexData*)(vb.data)).x		= m_pos.x + m_font_size + advance;
			(*(VertexData*)(vb.data)).y		= m_pos.y;
			(*(VertexData*)(vb.data)).u		= u1; 
			(*(VertexData*)(vb.data)).v		= v1;
			vb.data += sizeof(VertexData);

			(*(VertexData*)(vb.data)).x		= m_pos.x + m_font_size + advance;
			(*(VertexData*)(vb.data)).y		= m_pos.y - m_font_size;
			(*(VertexData*)(vb.data)).u		= u1;
			(*(VertexData*)(vb.data)).v		= v0;
			vb.data += sizeof(VertexData);

			(*(VertexData*)(vb.data)).x		= m_pos.x + advance;
			(*(VertexData*)(vb.data)).y		= m_pos.y - m_font_size;
			(*(VertexData*)(vb.data)).u		= u0;
			(*(VertexData*)(vb.data)).v		= v0;
			vb.data += sizeof(VertexData);


			(*(IndexData*)(ib.data)).a		= index;
			(*(IndexData*)(ib.data)).b		= index + 1;
			ib.data += sizeof(IndexData);

			(*(IndexData*)(ib.data)).a		= index + 2;
			(*(IndexData*)(ib.data)).b		= index;
			ib.data += sizeof(IndexData);

			(*(IndexData*)(ib.data)).a		= index + 2;
			(*(IndexData*)(ib.data)).b		= index + 3;
			ib.data += sizeof(IndexData);

			index += 4;

			advance += g.x_advance;
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
	Vector3				m_pos;

	MaterialId 			m_material;
};

} // namespace crown

