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
#include "Renderer.h"

namespace crown
{

struct Vector3;
struct Vector2;
struct Color4;

struct GuiRect
{
	//-------------------------------------------------------------------------
	GuiRect(Renderer& r, const Vector3& pos, const Vector2& size, const Color4& color)
		: m_r(r)
	{
		update(pos, size, color);

		m_vb = m_r.create_vertex_buffer(4 * Vertex::bytes_per_vertex(VertexFormat::P2_C4), m_vertices, VertexFormat::P2_C4);
		m_ib = m_r.create_index_buffer(8 * sizeof(uint16_t), m_indices);
	}

	//-------------------------------------------------------------------------
	~GuiRect()
	{
		m_r.destroy_index_buffer(m_ib);
		m_r.destroy_vertex_buffer(m_vb);
	}

	//-------------------------------------------------------------------------
	void update(const Vector3& pos, const Vector2& size, const Color4& color)
	{
		m_vertices[0] = pos.x; m_vertices[1] = pos.y;
		m_vertices[2] = color.r; m_vertices[3] = color.g;
		m_vertices[4] = color.b; m_vertices[5] = color.a;
		m_vertices[6] = pos.x + size.x; m_vertices[7] = pos.y;
		m_vertices[8] = color.r; m_vertices[9] = color.g;
		m_vertices[10] = color.b; m_vertices[11] = color.a;
		m_vertices[12] = pos.x + size.x; m_vertices[13] = pos.y - size.y;
		m_vertices[14] = color.r; m_vertices[15] = color.g;
		m_vertices[16] = color.b; m_vertices[17] = color.a;
		m_vertices[18] = pos.x; m_vertices[19] = pos.y - size.y;
		m_vertices[20] = color.r; m_vertices[21] = color.g;
		m_vertices[22] = color.b; m_vertices[23] = color.a;

		m_indices[0] = 0; m_indices[1] = 1;
		m_indices[2] = 1; m_indices[3] = 2;
		m_indices[4] = 2; m_indices[5] = 3;
		m_indices[6] = 3; m_indices[7] = 0;
	}

	//-------------------------------------------------------------------------
	void render()
	{
		m_r.set_vertex_buffer(m_vb);
		m_r.set_index_buffer(m_ib);
		m_r.commit(1);
	}

public:

	Renderer& m_r;

	float m_vertices[6*4];
	uint16_t m_indices[2*4];

	VertexBufferId m_vb;
	IndexBufferId m_ib;
};

} // namespace crown