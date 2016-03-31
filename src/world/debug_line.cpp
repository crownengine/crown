/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "color4.h"
#include "debug_line.h"
#include "device.h"
#include "math_utils.h"
#include "matrix4x4.h"
#include "mesh_resource.h"
#include "resource_manager.h"
#include "shader_manager.h"
#include "unit_resource.h"
#include "vector3.h"
#include <string.h> // memcpy

namespace crown
{
DebugLine::DebugLine(bool depth_test)
	: _marker(MARKER)
	, _shader(depth_test ? "debug_line" : "debug_line_noz")
	, _num(0)
{
	_vertex_decl.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
		.end();
}

DebugLine::~DebugLine()
{
	_marker = 0;
}

void DebugLine::add_line(const Vector3& start, const Vector3& end, const Color4& color)
{
	if (_num >= MAX_LINES)
		 return;

	_lines[_num].p0 = start;
	_lines[_num].c0 = to_abgr(color);
	_lines[_num].p1 = end;
	_lines[_num].c1 = to_abgr(color);

	++_num;
}

void DebugLine::add_axes(const Matrix4x4& m, f32 length)
{
	const Vector3 pos = translation(m);
	add_line(pos, pos + x(m)*length, COLOR4_RED);
	add_line(pos, pos + y(m)*length, COLOR4_GREEN);
	add_line(pos, pos + z(m)*length, COLOR4_BLUE);
}

void DebugLine::add_circle(const Vector3& center, f32 radius, const Vector3& normal, const Color4& color, u32 segments)
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

	const f32 incr = 360.0f / (f32)(segments >= 3 ? segments : 3);
	f32 deg0 = 0.0f;
	for (u32 ss = 0; ss < segments; ++ss, deg0 += incr)
	{
		const f32 rad0 = frad(deg0);
		const f32 rad1 = frad(deg0 + incr);

		const Vector3 from0 = right*cos(-rad0) + cross(dir, right)*sin(-rad0) + dir*dot(dir, right)*(1.0f-cos(-rad0));
		const Vector3 from1 = right*cos(-rad1) + cross(dir, right)*sin(-rad1) + dir*dot(dir, right)*(1.0f-cos(-rad1));

		add_line(center + radius*from0, center + radius*from1, color);
	}
}

void DebugLine::add_cone(const Vector3& from, const Vector3& to, f32 radius, const Color4& color, u32 segments)
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

	const f32 incr = 360.0f / (f32)(segments >= 3 ? segments : 3);
	f32 deg0 = 0.0f;
	for (u32 ss = 0; ss < segments; ++ss, deg0 += incr)
	{
		const f32 rad0 = frad(deg0);
		const f32 rad1 = frad(deg0 + incr);

		const Vector3 from0 = right*cos(-rad0) + cross(dir, right)*sin(-rad0) + dir*dot(dir, right)*(1.0f-cos(-rad0));
		const Vector3 from1 = right*cos(-rad1) + cross(dir, right)*sin(-rad1) + dir*dot(dir, right)*(1.0f-cos(-rad1));

		add_line(from + radius*from0, to, color);
		add_line(from + radius*from0, from + radius*from1, color);
	}
}

void DebugLine::add_sphere(const Vector3& center, const f32 radius, const Color4& color, u32 segments)
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

void DebugLine::add_mesh(const Matrix4x4& tm, const void* vertices, u32 stride, const u16* indices, u32 num, const Color4& color)
{
	for (u32 i = 0; i < num; i += 3)
	{
		const u32 i0 = indices[i + 0];
		const u32 i1 = indices[i + 1];
		const u32 i2 = indices[i + 2];

		const Vector3& v0 = *(const Vector3*)((const char*)vertices + i0*stride) * tm;
		const Vector3& v1 = *(const Vector3*)((const char*)vertices + i1*stride) * tm;
		const Vector3& v2 = *(const Vector3*)((const char*)vertices + i2*stride) * tm;

		add_line(v0, v1, color);
		add_line(v1, v2, color);
		add_line(v2, v0, color);
	}
}

void DebugLine::add_unit(ResourceManager& rm, const Matrix4x4& tm, StringId64 name, const Color4& color)
{
	const UnitResource& ur = *(const UnitResource*)rm.get(RESOURCE_TYPE_UNIT, name);

	const char* component_data = (const char*)(&ur + 1);

	for (u32 cc = 0; cc < ur.num_component_types; ++cc)
	{
		const ComponentData* component = (const ComponentData*)component_data;
		const u32* unit_index = (const u32*)(component + 1);
		const char* data = (const char*)(unit_index + component->num_instances);

		if (component->type == StringId32("mesh_renderer")._id)
		{
			const MeshRendererDesc* mrd = (const MeshRendererDesc*)data;
			for (u32 i = 0; i < component->num_instances; ++i)
			{
				const MeshResource* mr = (const MeshResource*)rm.get(RESOURCE_TYPE_MESH, mrd->mesh_resource);
				const MeshGeometry* mg = mr->geometry(mrd->geometry_name);

				add_mesh(tm
					, mg->vertices.data
					, mg->vertices.stride
					, (u16*)mg->indices.data
					, mg->indices.num
					, color
					);

				++mrd;
			}
		}

		component_data += component->size + sizeof(ComponentData);
	}
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

	const ShaderData& sd = device()->shader_manager()->get(_shader);

	bgfx::setVertexBuffer(&tvb, 0, _num * 2);
	bgfx::setState(sd.state);
	bgfx::submit(1, sd.program);
}

} // namespace crown
