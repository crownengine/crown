/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/math/types.h"
#include "core/strings/string_id.h"
#include "core/types.h"
#include "device/pipeline.h"
#include "resource/shader_resource.h"
#include "resource/types.h"
#include "world/types.h"
#include <bgfx/bgfx.h>

namespace crown
{
/// Draws lines.
///
/// @ingroup World
struct DebugLine
{
	/// Default number of segments.
	static const u32 NUM_SEGMENTS = 36;

	struct Line
	{
		Vector3 p0;
		u32 c0;
		Vector3 p1;
		u32 c1;
	};

	u32 _marker;
	Array<Line> _lines;
	ShaderData *_shader;

	/// Whether to enable @a depth_test
	DebugLine(Allocator &a, ShaderData *shader);

	///
	~DebugLine();

	/// Adds a line from @a start to @a end with the given @a color.
	void add_line(const Vector3 &start, const Vector3 &end, const Color4 &color);

	/// Adds lines for each axis with the given @a length.
	void add_axes(const Matrix4x4 &m, f32 length = 1.0f);

	/// Adds an arc at @a center with the given @a radius and @a plane_normal and @a midpoint_normal vectors.
	void add_arc(const Vector3 &center, f32 radius, const Vector3 &plane_normal, const Vector3 &midpoint_normal, const Color4 &color, u32 circle_segments = NUM_SEGMENTS);

	/// Adds a circle at @a center with the given @a radius and @a normal vector.
	void add_circle(const Vector3 &center, f32 radius, const Vector3 &normal, const Color4 &color, u32 segments = NUM_SEGMENTS);

	/// Adds a cone with the base centered at @a base_center and the tip at @a tip.
	void add_cone(const Vector3 &base_center, const Vector3 &tip, f32 radius, const Color4 &color, u32 segments = NUM_SEGMENTS, u32 rays = NUM_SEGMENTS);

	/// Adds a sphere at @a center with the given @a radius and @a color.
	void add_sphere(const Vector3 &center, const f32 radius, const Color4 &color, u32 segments = NUM_SEGMENTS);

	/// Adds an orientd bounding box. @a tm describes the position and orientation of
	/// the box. @a half_extents describes the size of the box along the axis.
	void add_obb(const Matrix4x4 &tm, const Vector3 &half_extents, const Color4 &color);

	/// Adds the frustum @f.
	void add_frustum(const Frustum &f, const Color4 &color);

	/// Adds the mesh described by (vertices, stride, indices, num).
	void add_mesh(const Matrix4x4 &tm, const void *vertices, u32 stride, const u16 *indices, u32 num, const Color4 &color);

	/// Resets all the lines.
	void reset();

	/// Submits the lines to renderer for drawing.
	void submit(u8 view_id = View::DEBUG);
};

namespace debug_line
{
	DebugLine *create(Allocator &a, Pipeline &pl, bool depth_enabled);

} // namespace debug_line

} // namespace crown
