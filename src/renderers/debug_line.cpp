/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "debug_line.h"
#include "math_utils.h"
#include "color4.h"
#include "vector3.h"
#include "matrix4x4.h"
#include "error.h"
#include "device.h"
#include <string.h> // memcpy

namespace crown
{

DebugLine::DebugLine(bool depth_test)
	: _shader(depth_test ? "debug_line" : "debug_line_noz")
	, _num(0)
{
	_vertex_decl.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
		.end();
}

void DebugLine::add_line(const Vector3& start, const Vector3& end, const Color4& color)
{
	if (_num >= CROWN_MAX_DEBUG_LINES)
		 return;

	_lines[_num].p0 = start;
	_lines[_num].c0 = to_abgr(color);
	_lines[_num].p1 = end;
	_lines[_num].c1 = to_abgr(color);

	++_num;
}

void DebugLine::add_axes(const Matrix4x4& m, float length)
{
	const Vector3 pos = translation(m);
	add_line(pos, pos + x(m)*length, COLOR4_RED);
	add_line(pos, pos + y(m)*length, COLOR4_GREEN);
	add_line(pos, pos + z(m)*length, COLOR4_BLUE);
}

void DebugLine::add_circle(const Vector3& center, float radius, const Vector3& normal, const Color4& color, uint32_t segments)
{
	const Vector3 dir = normal;
	const Vector3 arr[] =
	{
		{ dir.z, dir.z, -dir.x -dir.y },
		{ -dir.y -dir.z, dir.x, dir.x }
	};
	const int idx = ((dir.z != 0.0f) && (-dir.x != dir.y));
	Vector3 right = arr[idx];
	normalize(right);

	const float incr = 360.0f / (float)(segments >= 3 ? segments : 3);
	float deg0 = 0.0f;
	for (uint32_t ss = 0; ss < segments; ++ss, deg0 += incr)
	{
		const float rad0 = to_rad(deg0);
		const float rad1 = to_rad(deg0 + incr);

		const Vector3 from0 = right*cos(-rad0) + cross(dir, right)*sin(-rad0) + dir*dot(dir, right)*(1.0f-cos(-rad0));
		const Vector3 from1 = right*cos(-rad1) + cross(dir, right)*sin(-rad1) + dir*dot(dir, right)*(1.0f-cos(-rad1));

		add_line(center + radius*from0, center + radius*from1, color);
	}
}

void DebugLine::add_cone(const Vector3& from, const Vector3& to, float radius, const Color4& color, uint32_t segments)
{
	Vector3 dir = to - from;
	normalize(dir);
	const Vector3 arr[] =
	{
		{ dir.z, dir.z, -dir.x -dir.y },
		{ -dir.y -dir.z, dir.x, dir.x }
	};
	const int idx = ((dir.z != 0.0f) && (-dir.x != dir.y));
	Vector3 right = arr[idx];
	normalize(right);

	const float incr = 360.0f / (float)(segments >= 3 ? segments : 3);
	float deg0 = 0.0f;
	for (uint32_t ss = 0; ss < segments; ++ss, deg0 += incr)
	{
		const float rad0 = to_rad(deg0);
		const float rad1 = to_rad(deg0 + incr);

		const Vector3 from0 = right*cos(-rad0) + cross(dir, right)*sin(-rad0) + dir*dot(dir, right)*(1.0f-cos(-rad0));
		const Vector3 from1 = right*cos(-rad1) + cross(dir, right)*sin(-rad1) + dir*dot(dir, right)*(1.0f-cos(-rad1));

		add_line(from + radius*from0, to, color);
		add_line(from + radius*from0, from + radius*from1, color);
	}
}

void DebugLine::add_sphere(const Vector3& center, const float radius, const Color4& color, uint32_t segments)
{
	add_circle(center, radius, VECTOR3_XAXIS, color, segments);
	add_circle(center, radius, VECTOR3_YAXIS, color, segments);
	add_circle(center, radius, VECTOR3_ZAXIS, color, segments);
}

void DebugLine::add_obb(const Matrix4x4& tm, const Vector3& half_extents, const Color4& color)
{
	const Vector3 o = vector3(tm.t.x, tm.t.y, tm.t.z);
	const Vector3 x = vector3(tm.x.x, tm.x.y, tm.x.z) * half_extents.x;
	const Vector3 y = vector3(tm.y.x, tm.y.y, tm.y.z) * half_extents.y;
	const Vector3 z = vector3(tm.z.x, tm.z.y, tm.z.z) * half_extents.z;

	// Back face
	add_line(o - x - y - z, o + x - y - z, color);
	add_line(o + x - y - z, o + x + y - z, color);
	add_line(o + x + y - z, o - x + y - z, color);
	add_line(o - x + y - z, o - x - y - z, color);

	add_line(o - x - y + z, o + x - y + z, color);
	add_line(o + x - y + z, o + x + y + z, color);
	add_line(o + x + y + z, o - x + y + z, color);
	add_line(o - x + y + z, o - x - y + z, color);

	add_line(o - x - y - z, o - x - y + z, color);
	add_line(o + x - y - z, o + x - y + z, color);
	add_line(o + x + y - z, o + x + y + z, color);
	add_line(o - x + y - z, o - x + y + z, color);
}

void DebugLine::reset()
{
	_num = 0;
}

void DebugLine::submit()
{
	if (!_num)
		return;

	if (!checkAvailTransientVertexBuffer(_num * 2, _vertex_decl))
		return;

	bgfx::TransientVertexBuffer tvb;
	bgfx::allocTransientVertexBuffer(&tvb, _num * 2, _vertex_decl);
	memcpy(tvb.data, _lines, sizeof(Line) * _num);

	const ShaderManager::ShaderData& sd = device()->shader_manager()->get(_shader);

	bgfx::setVertexBuffer(&tvb, 0, _num * 2);
	bgfx::setState(sd.state);
	bgfx::submit(0, sd.program);
}

} // namespace crown
