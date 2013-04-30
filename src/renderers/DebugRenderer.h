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

#pragma once

#include "Types.h"
#include "Vec3.h"
#include "Color4.h"

namespace crown
{

const uint32_t MAX_DEBUG_LINES = 4096;

class Renderer;

/// Util class to render various types of primiteves
/// for debugging purposes only.
/// @note
/// All the coordinates are in world-space.
class DebugRenderer
{
public:

				DebugRenderer(Renderer& renderer);
				~DebugRenderer();

	void		add_line(const Vec3& start, const Vec3& end, const Color4& color, bool depth_write);

	/// Total cost: 72 lines
	void		add_sphere(const Vec3& center, const float radius, const Color4& color, bool depth_write);

	/// Total cost: 12 lines
	void		add_box(const Vec3& min, const Vec3& max, const Color4& color, bool depth_write);

private:

	void		draw_all();

private:

	Renderer&	m_renderer;

	uint32_t	m_lines_count;

	Vec3		m_lines[MAX_DEBUG_LINES * 2];
	Color4		m_colors[MAX_DEBUG_LINES * 2];
	bool		m_depth_writes[MAX_DEBUG_LINES * 2];

	friend class	Device;
};

} // namespace crown

