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

struct GuiText
{
	//-------------------------------------------------------------------------
	GuiText(RenderWorld& render_world, Renderer& r, const FontResource* fr, const char* str, uint32_t font_size, const Vector3& pos)
		: m_render_world(render_world)
		, m_r(r)
		, m_resource(fr)
	{
		MaterialResource* mat = (MaterialResource*) device()->resource_manager()->data(m_resource->material());
		m_material = m_render_world.create_material(mat);

		FontGlyphData g = m_resource->get_glyph(str[0]);

		// update(pos, Vector2(100, 100));
		float x 		= (float) g.x / 512.0f;
		float y 		= (float) (512 - g.y) / 512.0f;
		float width 	= (float) g.width / 512.0f;
		float height 	= (float) g.height / 512.0f;

		float u0 = x;
		float v0 = y;
		float u1 = x + width;
		float v1 = y - height;

		m_vertices[0] = pos.x;
		m_vertices[1] = pos.y;
		m_vertices[2] = u0;
		m_vertices[3] = v1;

		m_vertices[4] = pos.x + font_size;
		m_vertices[5] = pos.y;
		m_vertices[6] = u1; 
		m_vertices[7] = v1;

		m_vertices[8] = pos.x + font_size; 
		m_vertices[9] = pos.y - font_size;
		m_vertices[10] = u1;
		m_vertices[11] = v0;

		m_vertices[12] = pos.x; 
		m_vertices[13] = pos.y - font_size;
		m_vertices[14] = u0;
		m_vertices[15] = v0;

		m_indices[0] = 0; m_indices[1] = 1;
		m_indices[2] = 2; m_indices[3] = 0;
		m_indices[4] = 2; m_indices[5] = 3;

		m_vb = m_r.create_vertex_buffer(4 * Vertex::bytes_per_vertex(VertexFormat::P2_T2), m_vertices, VertexFormat::P2_T2);
		m_ib = m_r.create_index_buffer(6 * sizeof(uint16_t), m_indices);
	}

	//-------------------------------------------------------------------------
	~GuiText()
	{
		m_r.destroy_vertex_buffer(m_vb);
		m_r.destroy_index_buffer(m_ib);

		m_render_world.destroy_material(m_material);
	}

	//-------------------------------------------------------------------------
	void update(const Vector3& pos, const Vector2& size)
	{
	}

	//-------------------------------------------------------------------------
	void render(UniformId uniform)
	{
		Material* material = m_render_world.lookup_material(m_material);
		material->bind(m_r, uniform);

		m_r.set_vertex_buffer(m_vb);
		m_r.set_index_buffer(m_ib);
		m_r.commit(1);
	}

public:

	RenderWorld& m_render_world;
	Renderer& m_r;
	const FontResource* m_resource;

	float m_vertices[4*4];
	uint16_t m_indices[2*3];

	VertexBufferId m_vb;
	IndexBufferId m_ib;
	MaterialId m_material;
};

} // namespace crown

