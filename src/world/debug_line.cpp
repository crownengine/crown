/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/math/color4.inl"
#include "core/math/constants.h"
#include "core/math/frustum.inl"
#include "core/math/intersection.h"
#include "core/math/math.h"
#include "core/math/matrix4x4.inl"
#include "core/math/obb.inl"
#include "core/math/vector3.inl"
#include "core/memory/memory.inl"
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
static bgfx::VertexLayout vertex_layout;

DebugLine::DebugLine(Allocator &a, ShaderData *shader)
	: _marker(DEBUG_LINE_MARKER)
	, _lines(a)
	, _shader(shader)
{
	if (vertex_layout.m_hash == 0) {
		vertex_layout.begin();
		vertex_layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
		vertex_layout.add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true);
		vertex_layout.end();
	}
}

DebugLine::~DebugLine()
{
	_marker = 0;
}

void DebugLine::add_line(const Vector3 &start, const Vector3 &end, const Color4 &color)
{
	Line l;
	l.p0 = start;
	l.c0 = to_abgr(color);
	l.p1 = end;
	l.c1 = to_abgr(color);

	array::push_back(_lines, l);
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

void DebugLine::add_cone(const Vector3 &base_center, const Vector3 &tip, f32 radius, const Color4 &color, u32 segments, u32 rays)
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
	Vector3 from;

	// Draw base.
	const f32 base_step = PI_TWO / (f32)(segments > 3 ? segments : 3);
	from = base_center - y;
	for (u32 i = 0; i <= segments; ++i) {
		const f32 t = base_step * i - PI_HALF;
		const Vector3 to = base_center + x*fcos(t) + y*fsin(t);
		add_line(from, to, color);
		from = to;
	}

	// Draw rays.
	const f32 ray_step = PI_TWO / (f32)(rays > 1 ? rays : 1);
	from = base_center - y;
	for (u32 i = 0; i <= rays; ++i) {
		const f32 t = ray_step * i - PI_HALF;
		const Vector3 to = base_center + x*fcos(t) + y*fsin(t);
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

void DebugLine::add_frustum(const Frustum &f, const Color4 &color)
{
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

		const Vector3 &v0 = *(Vector3 *)((char *)vertices + i0*stride) * tm;
		const Vector3 &v1 = *(Vector3 *)((char *)vertices + i1*stride) * tm;
		const Vector3 &v2 = *(Vector3 *)((char *)vertices + i2*stride) * tm;

		add_line(v0, v1, color);
		add_line(v1, v2, color);
		add_line(v2, v0, color);
	}
}

void DebugLine::reset()
{
	array::clear(_lines);
}

void DebugLine::submit(u8 view_id)
{
	u32 num = array::size(_lines);
	if (!num)
		return;

	bgfx::TransientVertexBuffer tvb;
	uint32_t num_vertices = bgfx::getAvailTransientVertexBuffer(2*num, vertex_layout);
	bgfx::allocTransientVertexBuffer(&tvb, num_vertices, vertex_layout);
	memcpy(tvb.data, array::begin(_lines), sizeof(Line)/2 * num_vertices);

	bgfx::setVertexBuffer(0, &tvb, 0, num * 2);
	bgfx::setState(_shader->state);
	bgfx::submit(view_id, _shader->program);
}

namespace debug_line
{
	DebugLine *create(Allocator &a, Pipeline &pl, bool depth_enabled)
	{
		return CE_NEW(a, DebugLine)(a, depth_enabled ? &pl._debug_line_depth_enabled_shader : &pl._debug_line_shader);
	}

} // namespace debug_line

} // namespace crown
