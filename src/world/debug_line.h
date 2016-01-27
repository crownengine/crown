/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "config.h"
#include "math_types.h"
#include "string_id.h"
#include <bgfx/bgfx.h>

namespace crown
{
/// Draws lines.
///
/// @ingroup World
struct DebugLine
{
	/// Whether to enable @a depth_test
	DebugLine(bool depth_test);
	~DebugLine();

	/// Adds a line from @a start to @a end with the given @a color.
	void add_line(const Vector3& start, const Vector3& end, const Color4& color);

	/// Adds lines for each axis with the given @a length.
	void add_axes(const Matrix4x4& m, f32 length = 1.0f);

	/// Adds a circle at @a center with the given @a radius and @a normal vector.
	void add_circle(const Vector3& center, f32 radius, const Vector3& normal, const Color4& color, u32 segments = NUM_SEGMENTS);

	/// Adds a cone with the base centered at @a from and the tip at @a to.
	void add_cone(const Vector3& from, const Vector3& to, f32 radius, const Color4& color, u32 segments = NUM_SEGMENTS);

	/// Adds a sphere at @a center with the given @a radius and @a color.
	void add_sphere(const Vector3& center, const f32 radius, const Color4& color, u32 segments = NUM_SEGMENTS);

	/// Adds an orientd bounding box. @a tm describes the position and orientation of
	/// the box. @a half_extents describes the size of the box along the axis.
	void add_obb(const Matrix4x4& tm, const Vector3& half_extents, const Color4& color);

	/// Resets all the lines.
	void reset();

	/// Submits the lines to renderer for drawing.
	void submit();

	/// Default number of segments.
	static const u32 NUM_SEGMENTS = 36;
	static const u32 MARKER = 0xd7c17715;

private:

	struct Line
	{
		Vector3 p0;
		u32 c0;
		Vector3 p1;
		u32 c1;
	};

	u32 _marker;

	StringId32 _shader;
	bgfx::VertexDecl _vertex_decl;

	u32 _num;
	Line _lines[CROWN_MAX_DEBUG_LINES];
};

} // namespace crown
