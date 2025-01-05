/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/math/color4.inl"
#include "core/math/constants.h"
#include "core/math/frustum.inl"
#include "core/math/intersection.h"
#include "core/math/math.h"
#include "core/math/matrix4x4.inl"
#include "core/math/obb.inl"
#include "core/math/vector3.inl"
#include "core/strings/string_id.inl"
#include "device/pipeline.h"
#include "resource/mesh_resource.h"
#include "resource/resource_manager.h"
#include "resource/sprite_resource.h"
#include "resource/unit_resource.h"
#include "world/debug_line.h"
#include "world/shader_manager.h"
#include <string.h> // memcpy

namespace crown
{
static const StringId32 debug_line_depth_enabled = STRING_ID_32("debug_line+DEPTH_ENABLED", UINT32_C(0x8819e848));
static const StringId32 debug_line = STRING_ID_32("debug_line", UINT32_C(0xbc06e973));

DebugLine::DebugLine(ShaderManager &sm, bool depth_test)
	: _marker(DEBUG_LINE_MARKER)
	, _shader_manager(&sm)
	, _shader(depth_test ? debug_line_depth_enabled : debug_line)
	, _num(0)
{
	_vertex_layout.begin();
	_vertex_layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
	_vertex_layout.add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true);
	_vertex_layout.end();
}

DebugLine::~DebugLine()
{
	_marker = 0;
}

void DebugLine::add_line(const Vector3 &start, const Vector3 &end, const Color4 &color)
{
	if (_num >= MAX_LINES)
		return;

	_lines[_num].p0 = start;
	_lines[_num].c0 = to_abgr(color);
	_lines[_num].p1 = end;
	_lines[_num].c1 = to_abgr(color);

	++_num;
}

void DebugLine::add_axes(const Matrix4x4 &m, f32 length)
{
	const Vector3 pos = translation(m);
	add_line(pos, pos + x(m)*length, COLOR4_RED);
	add_line(pos, pos + y(m)*length, COLOR4_GREEN);
	add_line(pos, pos + z(m)*length, COLOR4_BLUE);
}

void DebugLine::add_arc(const Vector3 &center, f32 radius, const Vector3 &plane_normal, const Vector3 &midpoint_normal, const Color4 &color, u32 circle_segments)
{
	const Vector3 x = midpoint_normal * radius;
	const Vector3 y = cross(midpoint_normal, plane_normal) * radius;
	const u32 segments = circle_segments / 2;
	const f32 step = PI / (f32)(segments > 3 ? segments : 3);
	Vector3 from = center - y;

	for (u32 i = 0; i <= segments; ++i) {
		const f32 t = step * i - PI_HALF;
		const Vector3 to = center + x*fcos(t) + y*fsin(t);
		add_line(from, to, color);
		from = to;
	}
}

void DebugLine::add_circle(const Vector3 &center, f32 radius, const Vector3 &normal, const Color4 &color, u32 segments)
{
	const Vector3 arr[] =
	{
		{ normal.z,             normal.z, -normal.x - normal.y },
		{ -normal.y - normal.z, normal.x, normal.x             }
	};
	const int idx = ((normal.z != 0.0f) && (-normal.x != normal.y));
	Vector3 right = arr[idx];
	normalize(right);

	const Vector3 x = right * radius;
	const Vector3 y = cross(right, normal) * radius;
	const f32 step = PI_TWO / (f32)(segments > 3 ? segments : 3);
	Vector3 from = center - y;

	for (u32 i = 0; i <= segments; ++i) {
		const f32 t = step * i - PI_HALF;
		const Vector3 to = center + x*fcos(t) + y*fsin(t);
		add_line(from, to, color);
		from = to;
	}
}

void DebugLine::add_cone(const Vector3 &base_center, const Vector3 &tip, f32 radius, const Color4 &color, u32 segments)
{
	Vector3 normal = tip - base_center;
	normalize(normal);
	const Vector3 arr[] =
	{
		{ normal.z,             normal.z, -normal.x - normal.y },
		{ -normal.y - normal.z, normal.x, normal.x             }
	};
	const int idx = ((normal.z != 0.0f) && (-normal.x != normal.y));
	Vector3 right = arr[idx];
	normalize(right);

	const Vector3 x = right * radius;
	const Vector3 y = cross(right, normal) * radius;
	const f32 step = PI_TWO / (f32)(segments > 3 ? segments : 3);
	Vector3 from = base_center - y;

	for (u32 i = 0; i <= segments; ++i) {
		const f32 t = step * i - PI_HALF;
		const Vector3 to = base_center + x*fcos(t) + y*fsin(t);
		add_line(from, to, color);
		add_line(from, tip, color);
		from = to;
	}
}

void DebugLine::add_sphere(const Vector3 &center, const f32 radius, const Color4 &color, u32 segments)
{
	add_circle(center, radius, VECTOR3_XAXIS, color, segments);
	add_circle(center, radius, VECTOR3_YAXIS, color, segments);
	add_circle(center, radius, VECTOR3_ZAXIS, color, segments);
}

void DebugLine::add_frustum(const Matrix4x4 &mvp, const Color4 &color)
{
	Frustum f;
	frustum::from_matrix(f, mvp);

	Vector3 pt[8];
	frustum::vertices(pt, f);

	add_line(pt[0], pt[1], color);
	add_line(pt[1], pt[2], color);
	add_line(pt[2], pt[3], color);
	add_line(pt[3], pt[0], color);
	add_line(pt[4], pt[5], color);
	add_line(pt[5], pt[6], color);
	add_line(pt[6], pt[7], color);
	add_line(pt[7], pt[4], color);
	add_line(pt[0], pt[4], color);
	add_line(pt[1], pt[5], color);
	add_line(pt[2], pt[6], color);
	add_line(pt[3], pt[7], color);
}

void DebugLine::add_obb(const Matrix4x4 &tm, const Vector3 &half_extents, const Color4 &color)
{
	Vector3 vertices[8];
	obb::to_vertices(vertices, { tm, half_extents });

	// Bottom face.
	add_line(vertices[0], vertices[1], color);
	add_line(vertices[1], vertices[2], color);
	add_line(vertices[2], vertices[3], color);
	add_line(vertices[3], vertices[0], color);

	// Top face.
	add_line(vertices[4], vertices[5], color);
	add_line(vertices[5], vertices[6], color);
	add_line(vertices[6], vertices[7], color);
	add_line(vertices[7], vertices[4], color);

	// Connect faces.
	add_line(vertices[0], vertices[4], color);
	add_line(vertices[1], vertices[5], color);
	add_line(vertices[2], vertices[6], color);
	add_line(vertices[3], vertices[7], color);
}

void DebugLine::add_mesh(const Matrix4x4 &tm, const void *vertices, u32 stride, const u16 *indices, u32 num, const Color4 &color)
{
	for (u32 i = 0; i < num; i += 3) {
		const u32 i0 = indices[i + 0];
		const u32 i1 = indices[i + 1];
		const u32 i2 = indices[i + 2];

		const Vector3 &v0 = *(const Vector3 *)((const char *)vertices + i0*stride) * tm;
		const Vector3 &v1 = *(const Vector3 *)((const char *)vertices + i1*stride) * tm;
		const Vector3 &v2 = *(const Vector3 *)((const char *)vertices + i2*stride) * tm;

		add_line(v0, v1, color);
		add_line(v1, v2, color);
		add_line(v2, v0, color);
	}
}

void DebugLine::reset()
{
	_num = 0;
}

void DebugLine::submit(u8 view_id)
{
	if (!_num)
		return;

	if (!bgfx::getAvailTransientVertexBuffer(_num * 2, _vertex_layout))
		return;

	bgfx::TransientVertexBuffer tvb;
	bgfx::allocTransientVertexBuffer(&tvb, _num * 2, _vertex_layout);
	memcpy(tvb.data, _lines, sizeof(Line) * _num);

	bgfx::setVertexBuffer(0, &tvb, 0, _num * 2);
	_shader_manager->submit(_shader, view_id);
}

} // namespace crown
