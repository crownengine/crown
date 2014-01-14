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

namespace crown
{

struct GuiTriangle
{
	GuiTriangle(Renderer& r, const Vector2& p1, const Vector2& p2, const Vector2& p3, const Color4& color)
		: m_r(r)
	{
		update(p1, p2, p3, color);

		m_vb = m_r.create_vertex_buffer(3, VertexFormat::P2_C4, m_vertices);
		m_ib = m_r.create_index_buffer(6, m_indices);
	}

	~GuiTriangle()
	{
		m_r.destroy_vertex_buffer(m_vb);
		m_r.destroy_index_buffer(m_ib);
	}

	void update(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Color4& color)
	{
		m_vertices[0] = p1.x; m_vertices[1] = p1.y;
		m_vertices[2] = color.r; m_vertices[3] = color.g;
		m_vertices[4] = color.b; m_vertices[5] = color.a;

		m_vertices[6] = p2.x; m_vertices[7] = p2.y;
		m_vertices[8] = color.r; m_vertices[9] = color.g;
		m_vertices[10] = color.b; m_vertices[11] = color.a;

		m_vertices[12] = p3.x; m_vertices[13] = p3.y;
		m_vertices[14] = color.r; m_vertices[15] = color.g;
		m_vertices[16] = color.b; m_vertices[17] = color.a;

		m_indices[0] = 0; m_indices[1] = 1;
		m_indices[2] = 1; m_indices[3] = 2;
		m_indices[4] = 2; m_indices[5] = 0;
	}

	void render()
	{
		m_r.set_vertex_buffer(m_vb);
		m_r.set_index_buffer(m_ib);
		m_r.commit(1);		
	}

public:

	Renderer& m_r;

	float m_vertices[6*3];
	uint16_t m_indices[2*3];

	VertexBufferId m_vb;
	IndexBufferId m_ib;
};

} // namespace crown