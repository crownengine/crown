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

// FIXME FIXME FIXME
#include "Device.h"
#include "ResourceManager.h"
#include "Material.h"

namespace crown
{

struct GuiImage
{
	//-------------------------------------------------------------------------
	GuiImage(RenderWorld& render_world, Renderer& r, ResourceId material, const Vector3& pos, const Vector2& size)
		: m_render_world(render_world)
		, m_r(r)
	{
		update(pos, size);

		// FIXME FIXME FIXME
		MaterialResource* mat = (MaterialResource*) device()->resource_manager()->data(material);
		m_material = m_render_world.create_material(mat);

		m_vb = m_r.create_vertex_buffer(4, VertexFormat::P2_T2, m_vertices);
		m_ib = m_r.create_index_buffer(6, m_indices);
	}

	//-------------------------------------------------------------------------
	~GuiImage()
	{
		m_r.destroy_vertex_buffer(m_vb);
		m_r.destroy_index_buffer(m_ib);

		m_render_world.destroy_material(m_material);
	}

	//-------------------------------------------------------------------------
	void update(const Vector3& pos, const Vector2& size)
	{
		m_vertices[0] = pos.x; m_vertices[1] = pos.y;
		m_vertices[2] = 0; m_vertices[3] = 0;

		m_vertices[4] = pos.x + size.x; m_vertices[5] = pos.y;
		m_vertices[6] = 1; m_vertices[7] = 0;

		m_vertices[8] = pos.x + size.x; m_vertices[9] = pos.y - size.y;
		m_vertices[10] = 1;	m_vertices[11] = 1;

		m_vertices[12] = pos.x; m_vertices[13] = pos.y - size.y;
		m_vertices[14] = 0;	m_vertices[15] = 1;

		m_indices[0] = 0; m_indices[1] = 1;
		m_indices[2] = 2; m_indices[3] = 0;
		m_indices[4] = 2; m_indices[5] = 3;
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

	float m_vertices[6*4];
	uint16_t m_indices[2*3];

	MaterialId m_material;
	VertexBufferId m_vb;
	IndexBufferId m_ib;
};

} // namespace crown