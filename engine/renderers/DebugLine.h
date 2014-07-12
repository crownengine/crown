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

#include "Types.h"
#include "Config.h"
#include "MathTypes.h"
#include "Color4.h"

namespace crown
{

namespace debug_line
{
	void init();

	void shutdown();
} // namespace debug_line

struct DebugLine
{
	/// Whether to enable @a depth_test
	DebugLine(bool depth_test);

	/// Adds a line from @start to @end with the given @a color.
	void add_line(const Color4& color, const Vector3& start, const Vector3& end);

	/// Adds a sphere at @a center with the given @a radius and @a color.
	void add_sphere(const Color4& color, const Vector3& center, const float radius);

	/// Clears all the lines.
	void clear();

	/// Sends the lines to renderer for drawing.
	void commit();

private:

	struct Line
	{
		float p0[3];
		uint32_t c0;
		float p1[3];
		uint32_t c1;
	};

	bool m_depth_test;
	uint32_t m_num_lines;
	Line m_lines[CE_MAX_DEBUG_LINES];
};

} // namespace crown
