/*
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

#include "DebugRenderer.h"
#include "Renderer.h"
#include "MathUtils.h"

namespace crown
{

//-----------------------------------------------------------------------------
DebugRenderer::DebugRenderer(Renderer& renderer) :
	m_renderer(renderer),
	m_lines_count(0)
{
}

//-----------------------------------------------------------------------------
DebugRenderer::~DebugRenderer()
{
}

//-----------------------------------------------------------------------------
void DebugRenderer::add_line(const Vec3& start, const Vec3& end, const Color4& color, bool depth_write)
{
	if (m_lines_count >= MAX_DEBUG_LINES)
	{
		return;
	}

	m_lines[m_lines_count * 2 + 0] = start;
	m_lines[m_lines_count * 2 + 1] = end;

	m_colors[m_lines_count * 2 + 0] = color;
	m_colors[m_lines_count * 2 + 1] = color;

	m_depth_writes[m_lines_count * 2 + 0] = depth_write;
	m_depth_writes[m_lines_count * 2 + 1] = depth_write;

	m_lines_count++;
}

//-----------------------------------------------------------------------------
void DebugRenderer::add_sphere(const Vec3& center, const float radius, const Color4& color, bool depth_write)
{
	const uint32_t deg_step = 15;

	// XZ plane
	for (uint32_t deg = 0; deg < 360; deg += deg_step)
	{
		float rad0 = math::deg_to_rad(deg);
		float rad1 = math::deg_to_rad(deg + deg_step);

		Vec3 start(math::cos(rad0) * radius, 0, -math::sin(rad0) * radius);
		Vec3 end  (math::cos(rad1) * radius, 0, -math::sin(rad1) * radius);

		add_line(center + start, center + end, color, depth_write);
	}

	// XY plane
	for (uint32_t deg = 0; deg < 360; deg += deg_step)
	{
		float rad0 = math::deg_to_rad(deg);
		float rad1 = math::deg_to_rad(deg + deg_step);

		Vec3 start(math::cos(rad0) * radius, math::sin(rad0) * radius, 0);
		Vec3 end  (math::cos(rad1) * radius, math::sin(rad1) * radius, 0);

		add_line(center + start, center + end, color, depth_write);
	}

	// YZ plane
	for (uint32_t deg = 0; deg < 360; deg += deg_step)
	{
		float rad0 = math::deg_to_rad(deg);
		float rad1 = math::deg_to_rad(deg + deg_step);

		Vec3 start(0, math::sin(rad0) * radius, -math::cos(rad0) * radius);
		Vec3 end  (0, math::sin(rad1) * radius, -math::cos(rad1) * radius);

		add_line(center + start, center + end, color, depth_write);
	}
}

//-----------------------------------------------------------------------------
void DebugRenderer::add_box(const Vec3& min, const Vec3& max, const Color4& color, bool depth_write)
{
	// Back lines
	add_line(min                      , Vec3(max.x, min.y, min.z), color, depth_write);
	add_line(Vec3(max.x, min.y, min.z), Vec3(max.x, max.y, min.z), color, depth_write);
	add_line(Vec3(max.x, max.y, min.z), Vec3(min.x, max.y, min.z), color, depth_write);
	add_line(Vec3(min.x, max.y, min.z), min                      , color, depth_write);

	// Front lines
	add_line(Vec3(min.x, min.y, max.z), Vec3(max.x, min.y, max.z), color, depth_write);
	add_line(Vec3(max.x, min.y, max.z), Vec3(max.x, max.y, max.z), color, depth_write);
	add_line(Vec3(max.x, max.y, max.z), Vec3(min.x, max.y, max.z), color, depth_write);
	add_line(Vec3(min.x, max.y, max.z), Vec3(min.x, min.y, max.z), color, depth_write);

	// Connect back and front vertices
	add_line(min                      , Vec3(min.x, min.y, max.z), color, depth_write);
	add_line(Vec3(max.x, min.y, min.z), Vec3(max.x, min.y, max.z), color, depth_write);
	add_line(Vec3(max.x, max.y, min.z), Vec3(max.x, max.y, max.z), color, depth_write);
	add_line(Vec3(min.x, max.y, min.z), Vec3(min.x, max.y, max.z), color, depth_write);
}

//-----------------------------------------------------------------------------
void DebugRenderer::draw_all()
{
	if (m_lines_count > 0)
	{
		m_renderer.set_lighting(false);
		m_renderer.set_texturing(0, false);

		m_renderer.draw_lines(m_lines[0].to_float_ptr(), m_colors[0].to_float_ptr(), m_lines_count * 2);
	}

	m_lines_count = 0;
}

} // namespace crown

