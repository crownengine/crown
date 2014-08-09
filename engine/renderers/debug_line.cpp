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

#include "debug_line.h"
#include "math_utils.h"
#include "color4.h"
#include "vector3.h"
#include "device.h"
#include "renderer.h"
#include "render_world.h"

namespace crown
{

//-----------------------------------------------------------------------------
DebugLine::DebugLine(bool depth_test)
	: m_depth_test(depth_test)
	, m_num_lines(0)
{
}

//-----------------------------------------------------------------------------
void DebugLine::add_line(const Color4& color, const Vector3& start, const Vector3& end)
{
	if (m_num_lines >= CE_MAX_DEBUG_LINES)
		 return;

	m_lines[m_num_lines].position_0[0] = start.x;
	m_lines[m_num_lines].position_0[1] = start.y;
	m_lines[m_num_lines].position_0[2] = start.z;
	m_lines[m_num_lines].color_0[0] = color.r;
	m_lines[m_num_lines].color_0[1] = color.g;
	m_lines[m_num_lines].color_0[2] = color.b;
	m_lines[m_num_lines].color_0[3] = color.a;
	m_lines[m_num_lines].position_1[0] = end.x;
	m_lines[m_num_lines].position_1[1] = end.y;
	m_lines[m_num_lines].position_1[2] = end.z;
	m_lines[m_num_lines].color_1[0] = color.r;
	m_lines[m_num_lines].color_1[1] = color.g;
	m_lines[m_num_lines].color_1[2] = color.b;
	m_lines[m_num_lines].color_1[3] = color.a;

	m_num_lines++;
}

//-----------------------------------------------------------------------------
void DebugLine::add_sphere(const Color4& color, const Vector3& center, const float radius)
{
	const uint32_t deg_step = 15;

	for (uint32_t deg = 0; deg < 360; deg += deg_step)
	{
		const float rad0 = math::deg_to_rad((float) deg);
		const float rad1 = math::deg_to_rad((float) deg + deg_step);

		// XZ plane
		const Vector3 start0(math::cos(rad0) * radius, 0, -math::sin(rad0) * radius);
		const Vector3 end0  (math::cos(rad1) * radius, 0, -math::sin(rad1) * radius);
		add_line(color, center + start0, center + end0);

		// XY plane
		const Vector3 start1(math::cos(rad0) * radius, math::sin(rad0) * radius, 0);
		const Vector3 end1  (math::cos(rad1) * radius, math::sin(rad1) * radius, 0);
		add_line(color, center + start1, center + end1);

		// YZ plane
		const Vector3 start2(0, math::sin(rad0) * radius, -math::cos(rad0) * radius);
		const Vector3 end2  (0, math::sin(rad1) * radius, -math::cos(rad1) * radius);
		add_line(color, center + start2, center + end2);
	}
}

//-----------------------------------------------------------------------------
void DebugLine::clear()
{
	m_num_lines = 0;
}

//-----------------------------------------------------------------------------
void DebugLine::commit()
{
	if (!m_num_lines)
		return;

	Renderer* r = device()->renderer();

	TransientVertexBuffer tvb;
	TransientIndexBuffer tib;
	r->reserve_transient_vertex_buffer(&tvb, m_num_lines * 2, VertexFormat::P3_C4);
	r->reserve_transient_index_buffer(&tib, m_num_lines * 2);

	memcpy(tvb.data, m_lines, sizeof(Line) * m_num_lines);

	uint16_t* indices = (uint16_t*) tib.data;
	for (uint32_t i = 0; i < m_num_lines * 2; i++)
	{
		indices[i] = i;
	}

	r->set_state((m_depth_test ? STATE_DEPTH_TEST_LESS : 0) | STATE_COLOR_WRITE | STATE_CULL_CW | STATE_PRIMITIVE_LINES);
	r->set_vertex_buffer(tvb);
	r->set_index_buffer(tib);
	r->set_program(render_world_globals::default_program());
	r->set_pose(matrix4x4::IDENTITY);
	r->commit(0);
}

} // namespace crown
