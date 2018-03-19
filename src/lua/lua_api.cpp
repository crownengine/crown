/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/guid.h"
#include "core/math/color4.h"
#include "core/math/intersection.h"
#include "core/math/math.h"
#include "core/math/matrix4x4.h"
#include "core/math/plane3.h"
#include "core/math/quaternion.h"
#include "core/math/types.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/memory/temp_allocator.h"
#include "core/strings/dynamic_string.h"
#include "core/strings/string_stream.h"
#include "device/console_server.h"
#include "device/device.h"
#include "device/input_device.h"
#include "device/input_manager.h"
#include "device/profiler.h"
#include "lua/lua_environment.h"
#include "lua/lua_stack.h"
#include "resource/resource_manager.h"
#include "resource/resource_package.h"
#include "world/animation_state_machine.h"
#include "world/debug_line.h"
#include "world/gui.h"
#include "world/material.h"
#include "world/physics_world.h"
#include "world/render_world.h"
#include "world/scene_graph.h"
#include "world/sound_world.h"
#include "world/unit_manager.h"
#include "world/world.h"

namespace crown
{
struct LightInfo
{
	const char* name;
	LightType::Enum type;
};

static const LightInfo s_light[] =
{
	{ "directional", LightType::DIRECTIONAL },
	{ "omni",        LightType::OMNI        },
	{ "spot",        LightType::SPOT        }
};
CE_STATIC_ASSERT(countof(s_light) == LightType::COUNT);

struct ProjectionInfo
{
	const char* name;
	ProjectionType::Enum type;
};

static const ProjectionInfo s_projection[] =
{
	{ "orthographic", ProjectionType::ORTHOGRAPHIC },
	{ "perspective",  ProjectionType::PERSPECTIVE  }
};
CE_STATIC_ASSERT(countof(s_projection) == ProjectionType::COUNT);

static LightType::Enum name_to_light_type(const char* name)
{
	for (u32 i = 0; i < countof(s_light); ++i)
	{
		if (strcmp(s_light[i].name, name) == 0)
			return s_light[i].type;
	}

	return LightType::COUNT;
}

static ProjectionType::Enum name_to_projection_type(const char* name)
{
	for (u32 i = 0; i < countof(s_projection); ++i)
	{
		if (strcmp(s_projection[i].name, name) == 0)
			return s_projection[i].type;
	}

	return ProjectionType::COUNT;
}

static int math_ray_plane_intersection(lua_State* L)
{
	LuaStack stack(L);
	const Plane3 p = plane3::from_point_and_normal(stack.get_vector3(3)
		, stack.get_vector3(4)
		);
	const f32 t = ray_plane_intersection(stack.get_vector3(1)
		, stack.get_vector3(2)
		, p
		);
	stack.push_float(t);
	return 1;
}

static int math_ray_disc_intersection(lua_State* L)
{
	LuaStack stack(L);
	const f32 t = ray_disc_intersection(stack.get_vector3(1)
		, stack.get_vector3(2)
		, stack.get_vector3(3)
		, stack.get_float(4)
		, stack.get_vector3(5)
		);
	stack.push_float(t);
	return 1;
}

static int math_ray_sphere_intersection(lua_State* L)
{
	LuaStack stack(L);
	Sphere s;
	s.c = stack.get_vector3(3);
	s.r = stack.get_float(4);
	const f32 t = ray_sphere_intersection(stack.get_vector3(1)
		, stack.get_vector3(2)
		, s
		);
	stack.push_float(t);
	return 1;
}

static int math_ray_obb_intersection(lua_State* L)
{
	LuaStack stack(L);
	const f32 t = ray_obb_intersection(stack.get_vector3(1)
		, stack.get_vector3(2)
		, stack.get_matrix4x4(3)
		, stack.get_vector3(4)
		);
	stack.push_float(t);
	return 1;
}

static int math_ray_triangle_intersection(lua_State* L)
{
	LuaStack stack(L);
	const f32 t = ray_triangle_intersection(stack.get_vector3(1)
		, stack.get_vector3(2)
		, stack.get_vector3(3)
		, stack.get_vector3(4)
		, stack.get_vector3(5)
		);
	stack.push_float(t);
	return 1;
}

static int vector3_ctor(lua_State* L)
{
	LuaStack stack(L);
	Vector3 v;
	v.x = stack.get_float(1 + 1);
	v.y = stack.get_float(2 + 1);
	v.z = stack.get_float(3 + 1);
	stack.push_vector3(v);
	return 1;
}

static int vector3_x(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_vector3(1).x);
	return 1;
}

static int vector3_y(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_vector3(1).y);
	return 1;
}

static int vector3_z(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_vector3(1).z);
	return 1;
}

static int vector3_set_x(lua_State* L)
{
	LuaStack stack(L);
	stack.get_vector3(1).x = stack.get_float(2);
	return 0;
}

static int vector3_set_y(lua_State* L)
{
	LuaStack stack(L);
	stack.get_vector3(1).y = stack.get_float(2);
	return 0;
}

static int vector3_set_z(lua_State* L)
{
	LuaStack stack(L);
	stack.get_vector3(1).z = stack.get_float(2);
	return 0;
}

static int vector3_elements(lua_State* L)
{
	LuaStack stack(L);
	Vector3& a = stack.get_vector3(1);
	stack.push_float(a.x);
	stack.push_float(a.y);
	stack.push_float(a.z);
	return 3;
}

static int vector3_add(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3(1) + stack.get_vector3(2));
	return 1;
}

static int vector3_subtract(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3(1) - stack.get_vector3(2));
	return 1;
}

static int vector3_multiply(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3(1) * stack.get_float(2));
	return 1;
}

static int vector3_dot(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(dot(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_cross(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(cross(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_equal(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_vector3(1) == stack.get_vector3(2));
	return 1;
}

static int vector3_length(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(length(stack.get_vector3(1)));
	return 1;
}

static int vector3_length_squared(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(length_squared(stack.get_vector3(1)));
	return 1;
}

static int vector3_set_length(lua_State* L)
{
	LuaStack stack(L);
	set_length(stack.get_vector3(1), stack.get_float(2));
	return 0;
}

static int vector3_normalize(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(normalize(stack.get_vector3(1)));
	return 1;
}

static int vector3_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(distance(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_distance_squared(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(distance_squared(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_angle(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(angle(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_max(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(max(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_min(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(min(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_lerp(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(lerp(stack.get_vector3(1), stack.get_vector3(2), stack.get_float(3)));
	return 1;
}

static int vector3_forward(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_FORWARD);
	return 1;
}

static int vector3_backward(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_BACKWARD);
	return 1;
}

static int vector3_left(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_LEFT);
	return 1;
}

static int vector3_right(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_RIGHT);
	return 1;
}

static int vector3_up(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_UP);
	return 1;
}

static int vector3_down(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_DOWN);
	return 1;
}

static int vector3_zero(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_ZERO);
	return 1;
}

static int vector3_to_string(lua_State* L)
{
	LuaStack stack(L);
	const Vector3 v = stack.get_vector3(1);
	char buf[256];
	snprintf(buf, sizeof(buf), "%.4f %.4f %.4f", v.x, v.y, v.z);
	stack.push_string(buf);
	return 1;
}

static int vector2_ctor(lua_State* L)
{
	LuaStack stack(L);
	Vector2 v;
	v.x = stack.get_float(1 + 1);
	v.y = stack.get_float(2 + 1);
	stack.push_vector2(v);
	return 1;
}

static int vector3box_ctor(lua_State* L)
{
	LuaStack stack(L);

	if (stack.num_args() == 0 + 1)
	{
		stack.push_vector3box(VECTOR3_ZERO);
	}
	else if (stack.num_args() == 1 + 1)
	{
		stack.push_vector3box(stack.get_vector3(1 + 1));
	}
	else
	{
		Vector3 v;
		v.x = stack.get_float(1 + 1);
		v.y = stack.get_float(2 + 1);
		v.z = stack.get_float(3 + 1);
		stack.push_vector3box(v);
	}

	return 1;
}

static int vector3box_store(lua_State* L)
{
	LuaStack stack(L);

	Vector3& v = stack.get_vector3box(1);

	if (stack.num_args() == 2)
		v = stack.get_vector3(2);
	else
		v = vector3(stack.get_float(2)
			, stack.get_float(3)
			, stack.get_float(4));

	return 0;
}

static int vector3box_unbox(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3box(1));
	return 1;
}

static int vector3box_tostring(lua_State* L)
{
	LuaStack stack(L);
	Vector3& v = stack.get_vector3box(1);
	stack.push_fstring("Vector3Box (%p)", &v);
	return 1;
}

static int matrix4x4_ctor(lua_State* L)
{
	LuaStack stack(L);
	Matrix4x4 m;
	m.x.x = stack.get_float( 1 + 1);
	m.x.y = stack.get_float( 2 + 1);
	m.x.z = stack.get_float( 3 + 1);
	m.x.w = stack.get_float( 4 + 1);
	m.y.x = stack.get_float( 5 + 1);
	m.y.y = stack.get_float( 6 + 1);
	m.y.z = stack.get_float( 7 + 1);
	m.y.w = stack.get_float( 8 + 1);
	m.z.x = stack.get_float( 9 + 1);
	m.z.y = stack.get_float(10 + 1);
	m.z.z = stack.get_float(11 + 1);
	m.z.w = stack.get_float(12 + 1);
	m.t.x = stack.get_float(13 + 1);
	m.t.y = stack.get_float(14 + 1);
	m.t.z = stack.get_float(15 + 1);
	m.t.w = stack.get_float(16 + 1);
	stack.push_matrix4x4(m);
	return 1;
}

static int matrix4x4_from_quaternion(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(matrix4x4(stack.get_quaternion(1), VECTOR3_ZERO));
	return 1;
}

static int matrix4x4_from_translation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(matrix4x4(stack.get_vector3(1)));
	return 1;
}

static int matrix4x4_from_quaternion_translation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(matrix4x4(stack.get_quaternion(1), stack.get_vector3(2)));
	return 1;
}

static int matrix4x4_from_axes(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(matrix4x4(stack.get_vector3(1), stack.get_vector3(2), stack.get_vector3(3), stack.get_vector3(4)));
	return 1;
}

static int matrix4x4_copy(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_matrix4x4(1));
	return 1;
}

static int matrix4x4_add(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_matrix4x4(1) + stack.get_matrix4x4(2));
	return 1;
}

static int matrix4x4_subtract(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_matrix4x4(1) - stack.get_matrix4x4(2));
	return 1;
}

static int matrix4x4_multiply(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_matrix4x4(1) * stack.get_matrix4x4(2));
	return 1;
}

static int matrix4x4_transpose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(transpose(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_invert(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(invert(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_x(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(x(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_y(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(y(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_z(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(z(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_set_x(lua_State* L)
{
	LuaStack stack(L);
	set_x(stack.get_matrix4x4(1), stack.get_vector3(2));
	return 0;
}

static int matrix4x4_set_y(lua_State* L)
{
	LuaStack stack(L);
	set_y(stack.get_matrix4x4(1), stack.get_vector3(2));
	return 0;
}

static int matrix4x4_set_z(lua_State* L)
{
	LuaStack stack(L);
	set_z(stack.get_matrix4x4(1), stack.get_vector3(2));
	return 0;
}

static int matrix4x4_translation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(translation(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_set_translation(lua_State* L)
{
	LuaStack stack(L);
	set_translation(stack.get_matrix4x4(1), stack.get_vector3(2));
	return 0;
}

static int matrix4x4_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(rotation(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_set_rotation(lua_State* L)
{
	LuaStack stack(L);
	set_rotation(stack.get_matrix4x4(1), stack.get_quaternion(2));
	return 0;
}

static int matrix4x4_identity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(MATRIX4X4_IDENTITY);
	return 1;
}

static int matrix4x4_transform(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3(2) * stack.get_matrix4x4(1));
	return 1;
}

static int matrix4x4_to_string(lua_State* L)
{
	LuaStack stack(L);
	Matrix4x4& a = stack.get_matrix4x4(1);
	char buf[1024];
	snprintf(buf, sizeof(buf),
		"%.4f, %.4f, %.4f, %.4f\n"
		"%.4f, %.4f, %.4f, %.4f\n"
		"%.4f, %.4f, %.4f, %.4f\n"
		"%.4f, %.4f, %.4f, %.4f"
		, a.x.x, a.x.y, a.x.z, a.y.w
		, a.y.x, a.y.y, a.y.z, a.y.w
		, a.z.x, a.z.y, a.z.z, a.z.w
		, a.t.x, a.t.y, a.t.z, a.t.w
		);
	stack.push_string(buf);
	return 1;
}

static int matrix4x4box_ctor(lua_State* L)
{
	LuaStack stack(L);

	if (stack.num_args() == 0 + 1)
		stack.push_matrix4x4(MATRIX4X4_IDENTITY);
	else
		stack.push_matrix4x4box(stack.get_matrix4x4(1 + 1));

	return 1;
}

static int matrix4x4box_store(lua_State* L)
{
	LuaStack stack(L);
	stack.get_matrix4x4box(1) = stack.get_matrix4x4(2);
	return 0;
}

static int matrix4x4box_unbox(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_matrix4x4box(1));
	return 1;
}

static int matrix4x4box_tostring(lua_State* L)
{
	LuaStack stack(L);
	Matrix4x4& m = stack.get_matrix4x4box(1);
	stack.push_fstring("Matrix4x4Box (%p)", &m);
	return 1;
}

static int quaternion_from_axis_angle(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(quaternion(stack.get_vector3(1), stack.get_float(2)));
	return 1;
}

static int quaternion_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(quaternion(stack.get_vector3(1 + 1), stack.get_float(2 + 1)));
	return 1;
}

static int quaternion_from_elements(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(quaternion(stack.get_float(1), stack.get_float(2), stack.get_float(3), stack.get_float(4)));
	return 1;
}

static int quaternion_negate(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(-stack.get_quaternion(1));
	return 1;
}

static int quaternion_identity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(QUATERNION_IDENTITY);
	return 1;
}

static int quaternion_dot(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(dot(stack.get_quaternion(1), stack.get_quaternion(2)));
	return 1;
}

static int quaternion_length(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(length(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_normalize(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(normalize(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_conjugate(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(conjugate(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_inverse(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(inverse(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_multiply(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_quaternion(1) * stack.get_quaternion(2));
	return 1;
}

static int quaternion_multiply_by_scalar(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_quaternion(1) * stack.get_float(2));
	return 1;
}

static int quaternion_power(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(power(stack.get_quaternion(1), stack.get_float(2)));
	return 1;
}

static int quaternion_elements(lua_State* L)
{
	LuaStack stack(L);
	const Quaternion& q = stack.get_quaternion(1);
	stack.push_float(q.x);
	stack.push_float(q.y);
	stack.push_float(q.z);
	stack.push_float(q.w);
	return 4;
}

static int quaternion_look(lua_State* L)
{
	LuaStack stack(L);
	const Vector3 up = stack.num_args() == 2 ? stack.get_vector3(2) : VECTOR3_YAXIS;
	stack.push_quaternion(look(stack.get_vector3(1), up));
	return 1;
}

static int quaternion_right(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(right(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_up(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(up(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_forward(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(forward(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_lerp(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(lerp(stack.get_quaternion(1), stack.get_quaternion(2), stack.get_float(3)));
	return 1;
}

static int quaternion_to_string(lua_State* L)
{
	LuaStack stack(L);
	const Quaternion q = stack.get_quaternion(1);
	char buf[256];
	snprintf(buf, sizeof(buf), "%.4f %.4f %.4f %.4f", q.x, q.y, q.z, q.w);
	stack.push_string(buf);
	return 1;
}

static int quaternionbox_ctor(lua_State* L)
{
	LuaStack stack(L);

	if (stack.num_args() == 0 + 1)
	{
		stack.push_quaternionbox(QUATERNION_IDENTITY);
	}
	else if (stack.num_args() == 1 + 1)
	{
		stack.push_quaternionbox(stack.get_quaternion(1 + 1));
	}
	else
	{
		Quaternion q = quaternion(stack.get_float(1 + 1)
			, stack.get_float(2 + 1)
			, stack.get_float(3 + 1)
			, stack.get_float(4 + 1)
			);
		stack.push_quaternionbox(q);
	}

	return 1;
}

static int quaternionbox_store(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternionbox(1);

	if (stack.num_args() == 2)
		q = stack.get_quaternion(2);
	else
		q = quaternion(stack.get_float(2)
			, stack.get_float(3)
			, stack.get_float(4)
			, stack.get_float(5));

	return 0;
}

static int quaternionbox_unbox(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternionbox(1);

	stack.push_quaternion(q);
	return 1;
}

static int quaternionbox_tostring(lua_State* L)
{
	LuaStack stack(L);
	Quaternion& q = stack.get_quaternionbox(1);
	stack.push_fstring("QuaternionBox (%p)", &q);
	return 1;
}

static int color4_ctor(lua_State* L)
{
	LuaStack stack(L);
	u8 r = (u8)stack.get_int(1 + 1);
	u8 g = (u8)stack.get_int(2 + 1);
	u8 b = (u8)stack.get_int(3 + 1);
	u8 a = (u8)stack.get_int(4 + 1);
	stack.push_color4(from_rgba(r, g, b, a));
	return 1;
}

static int color4_lerp(lua_State* L)
{
	LuaStack stack(L);
	stack.push_color4(lerp(stack.get_color4(1), stack.get_color4(2), stack.get_float(3)));
	return 1;
}

static int color4_black(lua_State* L)
{
	LuaStack stack(L);
	stack.push_color4(COLOR4_BLACK);
	return 1;
}

static int color4_white(lua_State* L)
{
	LuaStack stack(L);
	stack.push_color4(COLOR4_WHITE);
	return 1;
}

static int color4_red(lua_State* L)
{
	LuaStack stack(L);
	stack.push_color4(COLOR4_RED);
	return 1;
}

static int color4_green(lua_State* L)
{
	LuaStack stack(L);
	stack.push_color4(COLOR4_GREEN);
	return 1;
}

static int color4_blue(lua_State* L)
{
	LuaStack stack(L);
	stack.push_color4(COLOR4_BLUE);
	return 1;
}

static int color4_yellow(lua_State* L)
{
	LuaStack stack(L);
	stack.push_color4(COLOR4_YELLOW);
	return 1;
}

static int color4_orange(lua_State* L)
{
	LuaStack stack(L);
	stack.push_color4(COLOR4_ORANGE);
	return 1;
}

static int lightuserdata_add(lua_State* L)
{
	LuaStack stack(L);
	const Vector3& a = stack.get_vector3(1);
	const Vector3& b = stack.get_vector3(2);
	stack.push_vector3(a + b);
	return 1;
}

static int lightuserdata_sub(lua_State* L)
{
	LuaStack stack(L);
	const Vector3& a = stack.get_vector3(1);
	const Vector3& b = stack.get_vector3(2);
	stack.push_vector3(a - b);
	return 1;
}

static int lightuserdata_mul(lua_State* L)
{
	LuaStack stack(L);
	const int i = stack.is_number(1) ? 1 : 2;
	stack.push_vector3(stack.get_float(i) * stack.get_vector3(3-i));
	return 1;
}

static int lightuserdata_unm(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(-stack.get_vector3(1));
	return 1;
}

static int lightuserdata_index(lua_State* L)
{
	LuaStack stack(L);
	Vector3& v = stack.get_vector3(1);
	const char* s = stack.get_string(2);

	switch (s[0])
	{
	case 'x': stack.push_float(v.x); return 1;
	case 'y': stack.push_float(v.y); return 1;
	case 'z': stack.push_float(v.z); return 1;
	default: LUA_ASSERT(false, stack, "Bad index: '%c'", s[0]); break;
	}

	return 0;
}

static int lightuserdata_newindex(lua_State* L)
{
	LuaStack stack(L);
	Vector3& v = stack.get_vector3(1);
	const char* s = stack.get_string(2);
	const f32 value = stack.get_float(3);

	switch (s[0])
	{
	case 'x': v.x = value; break;
	case 'y': v.y = value; break;
	case 'z': v.z = value; break;
	default: LUA_ASSERT(false, stack, "Bad index: '%c'", s[0]); break;
	}

	return 0;
}

static int input_device_name(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	stack.push_string(dev.name());
	return 1;
}

static int input_device_connected(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	stack.push_bool(dev.connected());
	return 1;
}

static int input_device_num_buttons(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	stack.push_int(dev.num_buttons());
	return 1;
}

static int input_device_num_axes(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	stack.push_int(dev.num_axes());
	return 1;
}

static int input_device_pressed(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	stack.push_bool(dev.pressed(stack.get_int(1)));
	return 1;
}

static int input_device_released(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	stack.push_bool(dev.released(stack.get_int(1)));
	return 1;
}

static int input_device_any_pressed(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	stack.push_bool(dev.any_pressed());
	return 1;
}

static int input_device_any_released(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	stack.push_bool(dev.any_released());
	return 1;
}

static int input_device_button(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	stack.push_float(dev.button(stack.get_int(1)));
	return 1;
}

static int input_device_axis(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	stack.push_vector3(dev.axis(stack.get_int(1)));
	return 1;
}

static int input_device_button_name(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	const char* name = dev.button_name(stack.get_int(1));

	if (name != NULL)
		stack.push_string(name);
	else
		stack.push_nil();

	return 1;
}

static int input_device_axis_name(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	const char* name = dev.axis_name(stack.get_int(1));

	if (name != NULL)
		stack.push_string(name);
	else
		stack.push_nil();

	return 1;
}

static int input_device_button_id(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	const u8 id = dev.button_id(stack.get_string_id_32(1));

	if (id != UINT8_MAX)
		stack.push_int(id);
	else
		stack.push_nil();

	return 1;
}

static int input_device_axis_id(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	const u8 id = dev.axis_id(stack.get_string_id_32(1));

	if (id != UINT8_MAX)
		stack.push_int(id);
	else
		stack.push_nil();

	return 1;
}

static int input_device_deadzone(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	int axis_id = stack.get_int(1);
	DeadzoneMode::Enum mode;
	f32 size = dev.deadzone(axis_id, &mode);

	if (mode != DeadzoneMode::COUNT)
	{
		stack.push_int(mode);
		stack.push_float(size);
	}
	else
	{
		stack.push_nil();
		stack.push_nil();
	}

	return 2;
}

static int input_device_set_deadzone(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	int axis_id = stack.get_int(1);
	DeadzoneMode::Enum mode = (DeadzoneMode::Enum)stack.get_int(2);
	f32 size = stack.get_float(3);
	dev.set_deadzone(axis_id, mode, size);
	return 0;
}

#define KEYBOARD(name)                                                          \
	static int keyboard_ ## name(lua_State* L)                                  \
	{                                                                           \
		return input_device_ ## name(L, *device()->_input_manager->keyboard()); \
	}

#define MOUSE(name)                                                          \
	static int mouse_ ## name(lua_State* L)                                  \
	{                                                                        \
		return input_device_ ## name(L, *device()->_input_manager->mouse()); \
	}

#define TOUCH(name)                                                          \
	static int touch_ ## name(lua_State* L)                                  \
	{                                                                        \
		return input_device_ ## name(L, *device()->_input_manager->touch()); \
	}

#define PAD(index, name)                                                           \
	static int pad ## index ## _ ## name(lua_State* L)                             \
	{                                                                              \
		return input_device_ ## name(L, *device()->_input_manager->joypad(index)); \
	}

KEYBOARD(name)
KEYBOARD(connected)
KEYBOARD(num_buttons)
KEYBOARD(num_axes)
KEYBOARD(pressed)
KEYBOARD(released)
KEYBOARD(any_pressed)
KEYBOARD(any_released)
KEYBOARD(button)
// KEYBOARD(axis)
KEYBOARD(button_name)
// KEYBOARD(axis_name)
KEYBOARD(button_id)
// KEYBOARD(axis_id)

MOUSE(name)
MOUSE(connected)
MOUSE(num_buttons)
MOUSE(num_axes)
MOUSE(pressed)
MOUSE(released)
MOUSE(any_pressed)
MOUSE(any_released)
MOUSE(button)
MOUSE(axis)
MOUSE(button_name)
MOUSE(axis_name)
MOUSE(button_id)
MOUSE(axis_id)

TOUCH(name)
TOUCH(connected)
TOUCH(num_buttons)
TOUCH(num_axes)
TOUCH(pressed)
TOUCH(released)
TOUCH(any_pressed)
TOUCH(any_released)
TOUCH(button)
TOUCH(axis)
TOUCH(button_name)
TOUCH(axis_name)
TOUCH(button_id)
TOUCH(axis_id)

PAD(0, name)
PAD(0, connected)
PAD(0, num_buttons)
PAD(0, num_axes)
PAD(0, pressed)
PAD(0, released)
PAD(0, any_pressed)
PAD(0, any_released)
PAD(0, button)
PAD(0, axis)
PAD(0, button_name)
PAD(0, axis_name)
PAD(0, button_id)
PAD(0, axis_id)
PAD(0, deadzone)
PAD(0, set_deadzone)

PAD(1, name)
PAD(1, connected)
PAD(1, num_buttons)
PAD(1, num_axes)
PAD(1, pressed)
PAD(1, released)
PAD(1, any_pressed)
PAD(1, any_released)
PAD(1, button)
PAD(1, axis)
PAD(1, button_name)
PAD(1, axis_name)
PAD(1, button_id)
PAD(1, axis_id)
PAD(1, deadzone)
PAD(1, set_deadzone)

PAD(2, name)
PAD(2, connected)
PAD(2, num_buttons)
PAD(2, num_axes)
PAD(2, pressed)
PAD(2, released)
PAD(2, any_pressed)
PAD(2, any_released)
PAD(2, button)
PAD(2, axis)
PAD(2, button_name)
PAD(2, axis_name)
PAD(2, button_id)
PAD(2, axis_id)
PAD(2, deadzone)
PAD(2, set_deadzone)

PAD(3, name)
PAD(3, connected)
PAD(3, num_buttons)
PAD(3, num_axes)
PAD(3, pressed)
PAD(3, released)
PAD(3, any_pressed)
PAD(3, any_released)
PAD(3, button)
PAD(3, axis)
PAD(3, button_name)
PAD(3, axis_name)
PAD(3, button_id)
PAD(3, axis_id)
PAD(3, deadzone)
PAD(3, set_deadzone)

#undef KEYBOARD
#undef MOUSE
#undef TOUCH
#undef PAD

static int world_spawn_unit(lua_State* L)
{
	LuaStack stack(L);
	const int nargs = stack.num_args();

	const StringId64 name = stack.get_resource_id(2);
	const Vector3& pos    = nargs > 2 ? stack.get_vector3(3)    : VECTOR3_ZERO;
	const Quaternion& rot = nargs > 3 ? stack.get_quaternion(4) : QUATERNION_IDENTITY;

	LUA_ASSERT(device()->_resource_manager->can_get(RESOURCE_TYPE_UNIT, name), stack, "Unit not found");

	stack.push_unit(stack.get_world(1)->spawn_unit(name, pos, rot));
	return 1;
}

static int world_spawn_empty_unit(lua_State* L)
{
	LuaStack stack(L);
	stack.push_unit(stack.get_world(1)->spawn_empty_unit());
	return 1;
}

static int world_destroy_unit(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->destroy_unit(stack.get_unit(2));
	return 0;
}

static int world_num_units(lua_State* L)
{
	LuaStack stack(L);
	stack.push_int(stack.get_world(1)->num_units());
	return 1;
}

static int world_units(lua_State* L)
{
	LuaStack stack(L);

	TempAllocator1024 alloc;
	Array<UnitId> units(alloc);
	stack.get_world(1)->units(units);

	const u32 num = array::size(units);

	stack.push_table(num);
	for (u32 i = 0; i < num; ++i)
	{
		stack.push_key_begin((s32) i + 1);
		stack.push_unit(units[i]);
		stack.push_key_end();
	}

	return 1;
}

static int world_camera_create(lua_State* L)
{
	LuaStack stack(L);
	World* world = stack.get_world(1);
	UnitId unit = stack.get_unit(2);

	CameraDesc cd;
	cd.type       = name_to_projection_type(stack.get_string(3));
	cd.fov        = stack.get_float(4);
	cd.near_range = stack.get_float(5);
	cd.far_range  = stack.get_float(6);

	Matrix4x4 pose = stack.get_matrix4x4(7);

	stack.push_camera(world->camera_create(unit, cd, pose));
	return 1;
}

static int world_camera_instances(lua_State* L)
{
	LuaStack stack(L);
	CameraInstance inst = stack.get_world(1)->camera_instances(stack.get_unit(2));

	if (inst.i == UINT32_MAX)
		return 0;

	stack.push_camera(inst);
	return 1;
}

static int world_camera_set_projection_type(lua_State* L)
{
	LuaStack stack(L);

	const char* name = stack.get_string(3);
	const ProjectionType::Enum pt = name_to_projection_type(name);
	LUA_ASSERT(pt != ProjectionType::COUNT, stack, "Unknown projection type: '%s'", name);

	stack.get_world(1)->camera_set_projection_type(stack.get_unit(2), pt);
	return 0;
}

static int world_camera_projection_type(lua_State* L)
{
	LuaStack stack(L);
	ProjectionType::Enum type = stack.get_world(1)->camera_projection_type(stack.get_unit(2));
	stack.push_string(s_projection[type].name);
	return 1;
}

static int world_camera_fov(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_world(1)->camera_fov(stack.get_unit(2)));
	return 1;
}

static int world_camera_set_fov(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->camera_set_fov(stack.get_unit(2), stack.get_float(3));
	return 0;
}

static int world_camera_near_clip_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_world(1)->camera_near_clip_distance(stack.get_unit(2)));
	return 1;
}

static int world_camera_set_near_clip_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->camera_set_near_clip_distance(stack.get_unit(2), stack.get_float(3));
	return 0;
}

static int world_camera_far_clip_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_world(1)->camera_far_clip_distance(stack.get_unit(2)));
	return 1;
}

static int world_camera_set_far_clip_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->camera_set_far_clip_distance(stack.get_unit(2), stack.get_float(3));
	return 0;
}

static int world_camera_set_orthographic_size(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->camera_set_orthographic_size(stack.get_unit(2), stack.get_float(3));
	return 0;
}

static int world_camera_screen_to_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_world(1)->camera_screen_to_world(stack.get_unit(2), stack.get_vector3(3)));
	return 1;
}

static int world_camera_world_to_screen(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_world(1)->camera_world_to_screen(stack.get_unit(2), stack.get_vector3(3)));
	return 1;
}

static int world_update_animations(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->update_animations(stack.get_float(2));
	return 0;
}

static int world_update_scene(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->update_scene(stack.get_float(2));
	return 0;
}

static int world_update(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->update(stack.get_float(2));
	return 0;
}

static int world_play_sound(lua_State* L)
{
	LuaStack stack(L);
	const s32 nargs = stack.num_args();
	World* world = stack.get_world(1);
	const StringId64 name = stack.get_resource_id(2);
	const bool loop    = nargs > 2 ? stack.get_bool(3)    : false;
	const f32 volume = nargs > 3 ? stack.get_float(4)   : 1.0f;
	const Vector3& pos = nargs > 4 ? stack.get_vector3(5) : VECTOR3_ZERO;
	const f32 range  = nargs > 5 ? stack.get_float(6)   : 1000.0f;

	LUA_ASSERT(device()->_resource_manager->can_get(RESOURCE_TYPE_SOUND, name), stack, "Sound not found");

	stack.push_sound_instance_id(world->play_sound(name, loop, volume, pos, range));
	return 1;
}

static int world_stop_sound(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->stop_sound(stack.get_sound_instance_id(2));
	return 0;
}

static int world_link_sound(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->link_sound(stack.get_sound_instance_id(2)
		, stack.get_unit(3)
		, stack.get_int(4)
		);
	return 0;
}

static int world_set_listener_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_listener_pose(stack.get_matrix4x4(2));
	return 0;
}

static int world_set_sound_position(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_sound_position(stack.get_sound_instance_id(2), stack.get_vector3(3));
	return 0;
}

static int world_set_sound_range(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_sound_range(stack.get_sound_instance_id(2), stack.get_float(3));
	return 0;
}

static int world_set_sound_volume(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_sound_volume(stack.get_sound_instance_id(2), stack.get_float(3));
	return 0;
}

static int world_create_debug_line(lua_State* L)
{
	LuaStack stack(L);
	stack.push_debug_line(stack.get_world(1)->create_debug_line(stack.get_bool(2)));
	return 1;
}

static int world_destroy_debug_line(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->destroy_debug_line(*stack.get_debug_line(2));
	return 0;
}

static int world_create_screen_gui(lua_State* L)
{
	LuaStack stack(L);
	stack.push_gui(stack.get_world(1)->create_screen_gui());
	return 1;
}

static int world_destroy_gui(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->destroy_gui(*stack.get_gui(2));
	return 0;
}

static int world_load_level(lua_State* L)
{
	LuaStack stack(L);
	const int nargs = stack.num_args();

	const StringId64 name = stack.get_resource_id(2);
	const Vector3& pos    = nargs > 2 ? stack.get_vector3(3)    : VECTOR3_ZERO;
	const Quaternion& rot = nargs > 3 ? stack.get_quaternion(4) : QUATERNION_IDENTITY;
	LUA_ASSERT(device()->_resource_manager->can_get(RESOURCE_TYPE_LEVEL, name), stack, "Level not found");
	stack.push_level(stack.get_world(1)->load_level(name, pos, rot));
	return 1;
}

static int world_scene_graph(lua_State* L)
{
	LuaStack stack(L);
	stack.push_scene_graph(stack.get_world(1)->_scene_graph);
	return 1;
}

static int world_render_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_render_world(stack.get_world(1)->_render_world);
	return 1;
}

static int world_physics_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_physics_world(stack.get_world(1)->_physics_world);
	return 1;
}

static int world_sound_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_sound_world(stack.get_world(1)->_sound_world);
	return 1;
}

static int world_animation_state_machine(lua_State *L)
{
	LuaStack stack(L);
	stack.push_animation_state_machine(stack.get_world(1)->_animation_state_machine);
	return 1;
}

static int world_tostring(lua_State* L)
{
	LuaStack stack(L);
	World* w = stack.get_world(1);
	stack.push_fstring("World (%p)", w);
	return 1;
}

static int scene_graph_create(lua_State* L)
{
	LuaStack stack(L);
	SceneGraph* sg = stack.get_scene_graph(1);
	UnitId unit = stack.get_unit(2);
	LUA_ASSERT(!sg->has(unit), stack, "Unit already has transform");
	TransformInstance ti = sg->create(unit
		, stack.get_vector3(3)
		, stack.get_quaternion(4)
		, stack.get_vector3(5)
		);
	stack.push_transform(ti);
	return 1;
}

static int scene_graph_destroy(lua_State* L)
{
	LuaStack stack(L);
	SceneGraph* sg = stack.get_scene_graph(1);
	UnitId unit = stack.get_unit(2);
	LUA_ASSERT(sg->has(unit), stack, "Unit does not have transform");
	sg->destroy(unit, stack.get_transform(3));
	return 0;
}

static int scene_graph_instances(lua_State* L)
{
	LuaStack stack(L);
	TransformInstance ti = stack.get_scene_graph(1)->instances(stack.get_unit(2));

	if (ti.i == UINT32_MAX)
		stack.push_nil();
	else
		stack.push_transform(ti);

	return 1;
}

static int scene_graph_local_position(lua_State* L)
{
	LuaStack stack(L);
	SceneGraph* sg = stack.get_scene_graph(1);
	UnitId unit = stack.get_unit(2);
	LUA_ASSERT(sg->has(unit), stack, "Unit does not have transform");
	stack.push_vector3(sg->local_position(unit));
	return 1;
}

static int scene_graph_local_rotation(lua_State* L)
{
	LuaStack stack(L);
	SceneGraph* sg = stack.get_scene_graph(1);
	UnitId unit = stack.get_unit(2);
	LUA_ASSERT(sg->has(unit), stack, "Unit does not have transform");
	stack.push_quaternion(sg->local_rotation(unit));
	return 1;
}

static int scene_graph_local_scale(lua_State* L)
{
	LuaStack stack(L);
	SceneGraph* sg = stack.get_scene_graph(1);
	UnitId unit = stack.get_unit(2);
	LUA_ASSERT(sg->has(unit), stack, "Unit does not have transform");
	stack.push_vector3(sg->local_scale(unit));
	return 1;
}

static int scene_graph_local_pose(lua_State* L)
{
	LuaStack stack(L);
	SceneGraph* sg = stack.get_scene_graph(1);
	UnitId unit = stack.get_unit(2);
	LUA_ASSERT(sg->has(unit), stack, "Unit does not have transform");
	stack.push_matrix4x4(sg->local_pose(unit));
	return 1;
}

static int scene_graph_world_position(lua_State* L)
{
	LuaStack stack(L);
	SceneGraph* sg = stack.get_scene_graph(1);
	UnitId unit = stack.get_unit(2);
	LUA_ASSERT(sg->has(unit), stack, "Unit does not have transform");
	stack.push_vector3(sg->world_position(unit));
	return 1;
}

static int scene_graph_world_rotation(lua_State* L)
{
	LuaStack stack(L);
	SceneGraph* sg = stack.get_scene_graph(1);
	UnitId unit = stack.get_unit(2);
	LUA_ASSERT(sg->has(unit), stack, "Unit does not have transform");
	stack.push_quaternion(sg->world_rotation(unit));
	return 1;
}

static int scene_graph_world_pose(lua_State* L)
{
	LuaStack stack(L);
	SceneGraph* sg = stack.get_scene_graph(1);
	UnitId unit = stack.get_unit(2);
	LUA_ASSERT(sg->has(unit), stack, "Unit does not have transform");
	stack.push_matrix4x4(sg->world_pose(unit));
	return 1;
}

static int scene_graph_set_local_position(lua_State* L)
{
	LuaStack stack(L);
	SceneGraph* sg = stack.get_scene_graph(1);
	UnitId unit = stack.get_unit(2);
	LUA_ASSERT(sg->has(unit), stack, "Unit does not have transform");
	sg->set_local_position(unit, stack.get_vector3(3));
	return 0;
}

static int scene_graph_set_local_rotation(lua_State* L)
{
	LuaStack stack(L);
	SceneGraph* sg = stack.get_scene_graph(1);
	UnitId unit = stack.get_unit(2);
	LUA_ASSERT(sg->has(unit), stack, "Unit does not have transform");
	sg->set_local_rotation(unit, stack.get_quaternion(3));
	return 0;
}

static int scene_graph_set_local_scale(lua_State* L)
{
	LuaStack stack(L);
	SceneGraph* sg = stack.get_scene_graph(1);
	UnitId unit = stack.get_unit(2);
	LUA_ASSERT(sg->has(unit), stack, "Unit does not have transform");
	sg->set_local_scale(unit, stack.get_vector3(3));
	return 0;
}

static int scene_graph_set_local_pose(lua_State* L)
{
	LuaStack stack(L);
	SceneGraph* sg = stack.get_scene_graph(1);
	UnitId unit = stack.get_unit(2);
	LUA_ASSERT(sg->has(unit), stack, "Unit does not have transform");
	sg->set_local_pose(unit, stack.get_matrix4x4(3));
	return 0;
}

static int scene_graph_link(lua_State* L)
{
	LuaStack stack(L);
	SceneGraph* sg = stack.get_scene_graph(1);
	UnitId child = stack.get_unit(2);
	UnitId parent = stack.get_unit(3);
	LUA_ASSERT(sg->has(child), stack, "Unit child does not have transform");
	LUA_ASSERT(sg->has(parent), stack, "Unit parent does not have transform");
	sg->link(child, parent);
	return 0;
}

static int scene_graph_unlink(lua_State* L)
{
	LuaStack stack(L);
	SceneGraph* sg = stack.get_scene_graph(1);
	UnitId unit = stack.get_unit(2);
	LUA_ASSERT(sg->has(unit), stack, "Unit does not have transform");
	sg->unlink(unit);
	return 0;
}

static int unit_manager_create(lua_State* L)
{
	LuaStack stack(L);

	if (stack.num_args() == 1)
		stack.push_unit(device()->_unit_manager->create(*stack.get_world(1)));
	else
		stack.push_unit(device()->_unit_manager->create());

	return 1;
}

static int unit_manager_alive(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->_unit_manager->alive(stack.get_unit(1)));
	return 1;
}

static int render_world_mesh_create(lua_State* L)
{
	LuaStack stack(L);
	RenderWorld* rw = stack.get_render_world(1);
	UnitId unit = stack.get_unit(2);

	MeshRendererDesc desc;
	desc.mesh_resource     = stack.get_resource_id(3);
	desc.geometry_name     = stack.get_string_id_32(4);
	desc.material_resource = stack.get_resource_id(5);
	desc.visible           = stack.get_bool(6);

	Matrix4x4 pose = stack.get_matrix4x4(7);

	stack.push_mesh_instance(rw->mesh_create(unit, desc, pose));
	return 1;
}

static int render_world_mesh_destroy(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->mesh_destroy(stack.get_mesh_instance(2));
	return 0;
}

static int render_world_mesh_instances(lua_State* L)
{
	LuaStack stack(L);
	RenderWorld* rw = stack.get_render_world(1);
	UnitId unit = stack.get_unit(2);

	TempAllocator512 ta;
	Array<MeshInstance> inst(ta);
	rw->mesh_instances(unit, inst);

	const u32 n = array::size(inst);
	for (u32 i = 0; i < n; ++i)
		stack.push_mesh_instance(inst[i]);

	return n;
}

static int render_world_mesh_obb(lua_State* L)
{
	LuaStack stack(L);
	OBB obb = stack.get_render_world(1)->mesh_obb(stack.get_mesh_instance(2));
	stack.push_matrix4x4(obb.tm);
	stack.push_vector3(obb.half_extents);
	return 2;
}

static int render_world_mesh_raycast(lua_State* L)
{
	LuaStack stack(L);
	RenderWorld* rw = stack.get_render_world(1);
	float t = rw->mesh_raycast(stack.get_mesh_instance(2)
		, stack.get_vector3(3)
		, stack.get_vector3(4)
		);
	stack.push_float(t);
	return 1;
}

static int render_world_mesh_set_visible(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->mesh_set_visible(stack.get_mesh_instance(2), stack.get_bool(3));
	return 0;
}

static int render_world_sprite_create(lua_State* L)
{
	LuaStack stack(L);
	RenderWorld* rw = stack.get_render_world(1);
	UnitId unit = stack.get_unit(2);

	SpriteRendererDesc desc;
	desc.sprite_resource = stack.get_resource_id(3);
	desc.material_resource = stack.get_resource_id(4);
	desc.layer = stack.get_int(5);
	desc.depth = stack.get_int(6);
	desc.visible = stack.get_bool(7);

	Matrix4x4 pose = stack.get_matrix4x4(8);

	stack.push_sprite_instance(rw->sprite_create(unit, desc, pose));
	return 1;
}

static int render_world_sprite_destroy(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->sprite_destroy(stack.get_unit(2), stack.get_sprite_instance(3));
	return 0;
}

static int render_world_sprite_instances(lua_State* L)
{
	LuaStack stack(L);
	SpriteInstance inst = stack.get_render_world(1)->sprite_instances(stack.get_unit(2));

	if (inst.i == UINT32_MAX)
		return 0;

	stack.push_sprite_instance(inst);
	return 1;
}

static int render_world_sprite_set_visible(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->sprite_set_visible(stack.get_unit(2), stack.get_bool(3));
	return 0;
}

static int render_world_sprite_set_frame(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->sprite_set_frame(stack.get_unit(2), stack.get_int(3));
	return 0;
}

static int render_world_sprite_flip_x(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->sprite_flip_x(stack.get_unit(2), stack.get_bool(3));
	return 0;
}

static int render_world_sprite_flip_y(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->sprite_flip_y(stack.get_unit(2), stack.get_bool(3));
	return 0;
}

static int render_world_sprite_set_layer(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->sprite_set_layer(stack.get_unit(2), stack.get_int(3));
	return 0;
}

static int render_world_sprite_set_depth(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->sprite_set_depth(stack.get_unit(2), stack.get_int(3));
	return 0;
}

static int render_world_sprite_obb(lua_State* L)
{
	LuaStack stack(L);
	OBB obb = stack.get_render_world(1)->sprite_obb(stack.get_unit(2));
	stack.push_matrix4x4(obb.tm);
	stack.push_vector3(obb.half_extents);
	return 2;
}

static int render_world_sprite_raycast(lua_State* L)
{
	LuaStack stack(L);
	RenderWorld* rw = stack.get_render_world(1);
	u32 layer;
	u32 depth;
	float t = rw->sprite_raycast(stack.get_unit(2)
		, stack.get_vector3(3)
		, stack.get_vector3(4)
		, layer
		, depth
		);
	stack.push_float(t);
	stack.push_int(layer);
	stack.push_int(depth);
	return 3;
}

static int render_world_light_create(lua_State* L)
{
	LuaStack stack(L);

	const char* name = stack.get_string(3);
	const LightType::Enum lt = name_to_light_type(name);
	LUA_ASSERT(lt != LightType::COUNT, stack, "Unknown light type: '%s'", name);

	LightDesc ld;
	ld.type       = lt;
	ld.range      = stack.get_float(4);
	ld.intensity  = stack.get_float(5);
	ld.spot_angle = stack.get_float(6);
	ld.color      = stack.get_vector3(7);

	Matrix4x4 pose = stack.get_matrix4x4(8);

	stack.push_light_instance(stack.get_render_world(1)->light_create(stack.get_unit(2), ld, pose));
	return 1;
}

static int render_world_light_destroy(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->light_destroy(stack.get_unit(2)
		, stack.get_light_instance(3)
		);
	return 0;
}

static int render_world_light_instances(lua_State* L)
{
	LuaStack stack(L);
	LightInstance inst = stack.get_render_world(1)->light_instances(stack.get_unit(2));

	if (inst.i == UINT32_MAX)
		return 0;

	stack.push_light_instance(inst);
	return 1;
}

static int render_world_light_type(lua_State* L)
{
	LuaStack stack(L);
	LightType::Enum type = stack.get_render_world(1)->light_type(stack.get_unit(2));
	stack.push_string(s_light[type].name);
	return 1;
}

static int render_world_light_color(lua_State* L)
{
	LuaStack stack(L);
	stack.push_color4(stack.get_render_world(1)->light_color(stack.get_unit(2)));
	return 1;
}

static int render_world_light_range(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_render_world(1)->light_range(stack.get_unit(2)));
	return 1;
}

static int render_world_light_intensity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_render_world(1)->light_intensity(stack.get_unit(2)));
	return 1;
}

static int render_world_light_spot_angle(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_render_world(1)->light_spot_angle(stack.get_unit(2)));
	return 1;
}

static int render_world_light_set_type(lua_State* L)
{
	LuaStack stack(L);

	const char* name = stack.get_string(3);
	const LightType::Enum lt = name_to_light_type(name);
	LUA_ASSERT(lt != LightType::COUNT, stack, "Unknown light type: '%s'", name);

	stack.get_render_world(1)->light_set_type(stack.get_unit(2), lt);
	return 0;
}

static int render_world_light_set_color(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->light_set_color(stack.get_unit(2), stack.get_color4(3));
	return 0;
}

static int render_world_light_set_range(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->light_set_range(stack.get_unit(2), stack.get_float(3));
	return 0;
}

static int render_world_light_set_intensity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->light_set_intensity(stack.get_unit(2), stack.get_float(3));
	return 0;
}

static int render_world_light_set_spot_angle(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->light_set_spot_angle(stack.get_unit(2), stack.get_float(3));
	return 0;
}

static int render_world_light_debug_draw(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->light_debug_draw(stack.get_unit(2), *stack.get_debug_line(3));
	return 0;
}

static int render_world_enable_debug_drawing(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->enable_debug_drawing(stack.get_bool(2));
	return 0;
}

static int physics_world_actor_instances(lua_State* L)
{
	LuaStack stack(L);
	ActorInstance inst = stack.get_physics_world(1)->actor(stack.get_unit(2));

	if (inst.i == UINT32_MAX)
		return 0;

	stack.push_actor(inst);
	return 1;
}

static int physics_world_actor_world_position(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_physics_world(1)->actor_world_position(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_world_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_physics_world(1)->actor_world_rotation(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_world_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_physics_world(1)->actor_world_pose(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_teleport_world_position(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_teleport_world_position(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_actor_teleport_world_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_teleport_world_rotation(stack.get_actor(2), stack.get_quaternion(3));
	return 0;
}

static int physics_world_actor_teleport_world_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_teleport_world_pose(stack.get_actor(2), stack.get_matrix4x4(3));
	return 0;
}

static int physics_world_actor_center_of_mass(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_physics_world(1)->actor_center_of_mass(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_enable_gravity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_enable_gravity(stack.get_actor(2));
	return 0;
}

static int physics_world_actor_disable_gravity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_disable_gravity(stack.get_actor(2));
	return 0;
}

static int physics_world_actor_enable_collision(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_enable_collision(stack.get_actor(2));
	return 0;
}

static int physics_world_actor_disable_collision(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_disable_collision(stack.get_actor(2));
	return 0;
}

static int physics_world_actor_set_collision_filter(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_set_collision_filter(stack.get_actor(2), stack.get_string_id_32(3));
	return 0;
}

static int physics_world_actor_set_kinematic(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_set_kinematic(stack.get_actor(2), stack.get_bool(3));
	return 0;
}

static int physics_world_actor_is_static(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_physics_world(1)->actor_is_static(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_is_dynamic(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_physics_world(1)->actor_is_dynamic(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_is_kinematic(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_physics_world(1)->actor_is_kinematic(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_is_nonkinematic(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_physics_world(1)->actor_is_nonkinematic(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_linear_damping(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_physics_world(1)->actor_linear_damping(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_set_linear_damping(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_set_linear_damping(stack.get_actor(2), stack.get_float(3));
	return 0;
}

static int physics_world_actor_angular_damping(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_physics_world(1)->actor_angular_damping(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_set_angular_damping(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_set_angular_damping(stack.get_actor(2), stack.get_float(3));
	return 0;
}

static int physics_world_actor_linear_velocity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_physics_world(1)->actor_linear_velocity(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_set_linear_velocity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_set_linear_velocity(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_actor_angular_velocity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_physics_world(1)->actor_angular_velocity(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_set_angular_velocity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_set_angular_velocity(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_actor_add_impulse(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_add_impulse(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_actor_add_impulse_at(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_add_impulse_at(stack.get_actor(2), stack.get_vector3(3), stack.get_vector3(4));
	return 0;
}

static int physics_world_actor_add_actor_torque_impulse(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_add_torque_impulse(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_actor_push(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_push(stack.get_actor(2), stack.get_vector3(3), stack.get_float(4));
	return 0;
}

static int physics_world_actor_push_at(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_push_at(stack.get_actor(2), stack.get_vector3(3), stack.get_float(4), stack.get_vector3(5));
	return 0;
}

static int physics_world_actor_is_sleeping(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_physics_world(1)->actor_is_sleeping(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_wake_up(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->actor_wake_up(stack.get_actor(2));
	return 0;
}

static int physics_world_joint_create(lua_State* L)
{
	LuaStack stack(L);
	JointDesc jd;
	jd.type = JointType::SPRING;
	jd.anchor_0 = vector3(0, -2, 0);
	jd.anchor_1 = vector3(0, 2, 0);
	jd.break_force = 999999.0f;
	jd.hinge.axis = vector3(1, 0, 0);
	jd.hinge.lower_limit = -3.14f / 4.0f;
	jd.hinge.upper_limit = 3.14f / 4.0f;
	jd.hinge.bounciness = 12.0f;
	stack.get_physics_world(1)->joint_create(stack.get_actor(2), stack.get_actor(3), jd);
	return 0;
}

static int physics_world_gravity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_physics_world(1)->gravity());
	return 1;
}

static int physics_world_set_gravity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->set_gravity(stack.get_vector3(2));
	return 0;
}

static int physics_world_raycast_closest(lua_State* L)
{
	LuaStack stack(L);
	PhysicsWorld* world = stack.get_physics_world(1);

	TempAllocator1024 ta;
	Array<RaycastHit> hits(ta);

	world->raycast(stack.get_vector3(2)
		, stack.get_vector3(3)
		, stack.get_float(4)
		, RaycastMode::CLOSEST
		, hits
		);

	if (array::size(hits))
	{
		stack.push_bool(true);
		stack.push_vector3(hits[0].position);
		stack.push_vector3(hits[0].normal);
		stack.push_unit(hits[0].unit);
		stack.push_actor(hits[0].actor);
		return 5;
	}
	else
	{
		stack.push_bool(false);
		return 1;
	}
}

static int physics_world_raycast_all(lua_State* L)
{
	LuaStack stack(L);
	PhysicsWorld* world = stack.get_physics_world(1);

	TempAllocator1024 ta;
	Array<RaycastHit> hits(ta);

	world->raycast(stack.get_vector3(2)
		, stack.get_vector3(3)
		, stack.get_float(4)
		, RaycastMode::ALL
		, hits
		);

	const u32 num_hits = array::size(hits);

	stack.push_table(num_hits);
	for (u32 i = 0; i < num_hits; ++i)
	{
		stack.push_key_begin(i+1);
		stack.push_table();
		{
			stack.push_key_begin(1);
			stack.push_vector3(hits[i].position);
			stack.push_key_end();

			stack.push_key_begin(2);
			stack.push_vector3(hits[i].normal);
			stack.push_key_end();

			stack.push_key_begin(3);
			stack.push_unit(hits[i].unit);
			stack.push_key_end();

			stack.push_key_begin(4);
			stack.push_actor(hits[i].actor);
			stack.push_key_end();
		}
		stack.push_key_end();
	}

	return 1;
}

static int physics_world_enable_debug_drawing(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->enable_debug_drawing(stack.get_bool(2));
	return 0;
}

static int physics_world_tostring(lua_State* L)
{
	LuaStack stack(L);
	PhysicsWorld* pw = stack.get_physics_world(1);
	stack.push_fstring("PhysicsWorld (%p)", pw);
	return 1;
}

static int sound_world_stop_all(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sound_world(1)->stop_all();
	return 0;
}

static int sound_world_pause_all(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sound_world(1)->pause_all();
	return 0;
}

static int sound_world_resume_all(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sound_world(1)->resume_all();
	return 0;
}

static int sound_world_is_playing(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_sound_world(1)->is_playing(stack.get_sound_instance_id(2)));
	return 1;
}

static int sound_world_tostring(lua_State* L)
{
	LuaStack stack(L);
	SoundWorld* sw = stack.get_sound_world(1);
	stack.push_fstring("SoundWorld (%p)", sw);
	return 1;
}

static int animation_state_machine_trigger(lua_State* L)
{
	LuaStack stack(L);
	stack.get_animation_state_machine(1)->trigger(stack.get_unit(2)
		, stack.get_string_id_32(3)
		);
	return 0;
}

static int animation_state_machine_variable_id(lua_State* L)
{
	LuaStack stack(L);
	u32 variable_id = stack.get_animation_state_machine(1)->variable_id(stack.get_unit(2)
		, stack.get_string_id_32(3)
		);
	LUA_ASSERT(variable_id != UINT32_MAX, stack, "Variable does not exist");
	stack.push_id(variable_id);
	return 1;
}

static int animation_state_machine_variable(lua_State* L)
{
	LuaStack stack(L);
	const float v = stack.get_animation_state_machine(1)->variable(stack.get_unit(2)
		, stack.get_id(3)
		);
	stack.push_float(v);
	return 1;
}

static int animation_state_machine_set_variable(lua_State* L)
{
	LuaStack stack(L);
	stack.get_animation_state_machine(1)->set_variable(stack.get_unit(2)
		, stack.get_id(3)
		, stack.get_float(4)
		);
	return 0;
}

static int device_argv(lua_State* L)
{
	LuaStack stack(L);
	const int argc = device()->argc();
	const char** argv = device()->argv();
	stack.push_table(argc);
	for (int i = 0; i < argc; ++i)
	{
		stack.push_key_begin(i + 1);
		stack.push_string(argv[i]);
		stack.push_key_end();
	}
	return 1;
}

static int device_platform(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(CROWN_PLATFORM_NAME);
	return 1;
}

static int device_architecture(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(CROWN_ARCH_NAME);
	return 1;
}

static int device_version(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(CROWN_VERSION);
	return 1;
}

static int device_quit(lua_State* /*L*/)
{
	device()->quit();
	return 0;
}

static int device_resolution(lua_State* L)
{
	LuaStack stack(L);
	u16 w, h;
	device()->resolution(w, h);
	stack.push_int(w);
	stack.push_int(h);
	return 2;
}

static int device_create_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_world(device()->create_world());
	return 1;
}

static int device_destroy_world(lua_State* L)
{
	LuaStack stack(L);
	device()->destroy_world(*stack.get_world(1));
	return 0;
}

static int device_render(lua_State* L)
{
	LuaStack stack(L);
	device()->render(*stack.get_world(1), stack.get_unit(2));
	return 0;
}

static int device_create_resource_package(lua_State* L)
{
	LuaStack stack(L);
	stack.push_resource_package(device()->create_resource_package(stack.get_resource_id(1)));
	return 1;
}

static int device_destroy_resource_package(lua_State* L)
{
	LuaStack stack(L);
	device()->destroy_resource_package(*stack.get_resource_package(1));
	return 0;
}

static void lua_dump_table(lua_State* L, int i, StringStream& json)
{
	LuaStack stack(L);

	bool comma = false;
	int array_index = 1;

	json << "{";

	stack.push_nil();
	while (stack.next(i) != 0)
	{
		if (comma)
			json << ",";
		comma = true;

		json << "\"";
		if (stack.is_string(-2) && !stack.is_number(-2))
			json << stack.get_string(-2);
		else
			json << array_index++;
		json << "\":";

		if (stack.is_nil(i + 2))
		{
			json << "null";
		}
		else if (stack.is_bool(i + 2))
		{
			const bool b = stack.get_bool(i + 2);
			json << (b ? "true" : "false");
		}
		else if (stack.is_number(i + 2))
		{
			json << stack.get_float(i + 2);
		}
		else if (stack.is_string(i + 2))
		{
			const char* str = stack.get_string(i + 2);
			json << "\"";
			for (; *str; ++str)
			{
				if (*str == '"' || *str == '\\')
					json << '\\';
				json << *str;
			}
			json << "\"";
		}
		else if (stack.is_vector3(i + 2))
		{
			const Vector3 v = stack.get_vector3(i + 2);
			json << "["
				 << v.x << ","
				 << v.y << ","
				 << v.z
				 << "]"
				 ;
		}
		else if (stack.is_quaternion(i + 2))
		{
			const Quaternion q = stack.get_quaternion(i + 2);
			json << "["
				 << q.x << ","
				 << q.y << ","
				 << q.z << ","
				 << q.w
				 << "]"
				 ;
		}
		else if (stack.is_matrix4x4(i + 2))
		{
			const Matrix4x4 m = stack.get_matrix4x4(i + 2);
			json << "["
				 << m.x.x << ","
				 << m.x.y << ","
				 << m.x.z << ","
				 << m.x.w << ","

				 << m.y.x << ","
				 << m.y.y << ","
				 << m.y.z << ","
				 << m.y.w << ","

				 << m.z.x << ","
				 << m.z.y << ","
				 << m.z.z << ","
				 << m.z.w << ","

				 << m.t.x << ","
				 << m.t.y << ","
				 << m.t.z << ","
				 << m.t.w
				 << "]"
				 ;
		}
		else if (stack.is_table(i + 2))
		{
			lua_dump_table(L, i + 2, json);
		}
		else
		{
			LUA_ASSERT(false, stack, "Unsupported key value");
		}

		stack.pop(1);
	}

	json << "}";
}

static int device_console_send(lua_State* L)
{
	LuaStack stack(L);
	LUA_ASSERT(stack.is_table(1), stack, "Table expected");

	TempAllocator1024 alloc;
	StringStream json(alloc);
	lua_dump_table(L, 1, json);

	console_server()->send(string_stream::c_str(json));
	return 0;
}

static int device_can_get(lua_State* L)
{
	LuaStack stack(L);
	const StringId64 type(stack.get_string(1));
	stack.push_bool(device()->_resource_manager->can_get(type, stack.get_resource_id(2)));
	return 1;
}

static int device_enable_resource_autoload(lua_State* L)
{
	LuaStack stack(L);
	device()->_resource_manager->enable_autoload(stack.get_bool(1));
	return 0;
}

static int device_temp_count(lua_State* L)
{
	LuaStack stack(L);
	u32 nv, nq, nm;
	device()->_lua_environment->temp_count(nv, nq, nm);
	stack.push_int(nv);
	stack.push_int(nq);
	stack.push_int(nm);
	return 3;
}

static int device_set_temp_count(lua_State* L)
{
	LuaStack stack(L);
	u32 nv = stack.get_int(1);
	u32 nq = stack.get_int(2);
	u32 nm = stack.get_int(3);
	device()->_lua_environment->set_temp_count(nv, nq, nm);
	return 0;
}

static int device_guid(lua_State* L)
{
	LuaStack stack(L);
	TempAllocator64 ta;
	DynamicString str(ta);
	Guid guid = guid::new_guid();
	guid::to_string(guid, str);
	stack.push_string(str.c_str());
	return 1;
}

static int profiler_enter_scope(lua_State* L)
{
	LuaStack stack(L);
	profiler::enter_profile_scope(stack.get_string(1));
	return 0;
}

static int profiler_leave_scope(lua_State* L)
{
	LuaStack stack(L);
	profiler::leave_profile_scope();
	return 0;
}

static int profiler_record(lua_State* L)
{
	LuaStack stack(L);

	const char* name = stack.get_string(1);

	if (stack.is_number(2))
		profiler::record_float(name, stack.get_float(2));
	else
		profiler::record_vector3(name, stack.get_vector3(2));

	return 0;
}

static int debug_line_add_line(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->add_line(stack.get_vector3(2)
		, stack.get_vector3(3)
		, stack.get_color4(4)
		);
	return 0;
}

static int debug_line_add_axes(lua_State* L)
{
	LuaStack stack(L);
	const f32 len = stack.num_args() == 3
		? stack.get_float(3)
		: 1.0f
		;
	stack.get_debug_line(1)->add_axes(stack.get_matrix4x4(2), len);
	return 0;
}

static int debug_line_add_arc(lua_State* L)
{
	LuaStack stack(L);
	const u32 segments = stack.num_args() >= 7
		? stack.get_int(7)
		: DebugLine::NUM_SEGMENTS
		;
	stack.get_debug_line(1)->add_arc(stack.get_vector3(2)
		, stack.get_float(3)
		, stack.get_vector3(4)
		, stack.get_vector3(5)
		, stack.get_color4(6)
		, segments
		);
	return 0;
}

static int debug_line_add_circle(lua_State* L)
{
	LuaStack stack(L);
	const u32 segments = stack.num_args() >= 6
		? stack.get_int(6)
		: DebugLine::NUM_SEGMENTS
		;
	stack.get_debug_line(1)->add_circle(stack.get_vector3(2)
		, stack.get_float(3)
		, stack.get_vector3(4)
		, stack.get_color4(5)
		, segments
		);
	return 0;
}

static int debug_line_add_cone(lua_State* L)
{
	LuaStack stack(L);
	const u32 segments = stack.num_args() >= 6
		? stack.get_int(6)
		: DebugLine::NUM_SEGMENTS
		;
	stack.get_debug_line(1)->add_cone(stack.get_vector3(2)
		, stack.get_vector3(3)
		, stack.get_float(4)
		, stack.get_color4(5)
		, segments
		);
	return 0;
}

static int debug_line_add_sphere(lua_State* L)
{
	LuaStack stack(L);
	const u32 segments = stack.num_args() >= 5
		? stack.get_int(5)
		: DebugLine::NUM_SEGMENTS
		;
	stack.get_debug_line(1)->add_sphere(stack.get_vector3(2)
		, stack.get_float(3)
		, stack.get_color4(4)
		, segments
		);
	return 0;
}

static int debug_line_add_obb(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->add_obb(stack.get_matrix4x4(2)
		, stack.get_vector3(3)
		, stack.get_color4(4)
		);
	return 0;
}

static int debug_line_add_frustum(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->add_frustum(stack.get_matrix4x4(2)
		, stack.get_color4(3)
		);
	return 0;
}

static int debug_line_add_unit(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->add_unit(*device()->_resource_manager
		, stack.get_matrix4x4(2)
		, stack.get_resource_id(3)
		, stack.get_color4(4)
		);
	return 0;
}

static int debug_line_reset(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->reset();
	return 0;
}

static int debug_line_submit(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->submit();
	return 0;
}

static int debug_line_tostring(lua_State* L)
{
	LuaStack stack(L);
	stack.push_fstring("DebugLine (%p)", stack.get_debug_line(1));
	return 1;
}

static int resource_package_load(lua_State* L)
{
	LuaStack stack(L);
	stack.get_resource_package(1)->load();
	return 0;
}

static int resource_package_unload(lua_State* L)
{
	LuaStack stack(L);
	stack.get_resource_package(1)->unload();
	return 0;
}

static int resource_package_flush(lua_State* L)
{
	LuaStack stack(L);
	stack.get_resource_package(1)->flush();
	return 0;
}

static int resource_package_has_loaded(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_resource_package(1)->has_loaded());
	return 1;
}

static int resource_package_tostring(lua_State* L)
{
	LuaStack stack(L);
	ResourcePackage* package = stack.get_resource_package(1);
	stack.push_fstring("ResourcePackage (%p)", package);
	return 1;
}

static int material_set_float(lua_State* L)
{
	LuaStack stack(L);
	stack.get_material(1)->set_float(stack.get_string_id_32(2), stack.get_float(3));
	return 0;
}

static int material_set_vector2(lua_State* L)
{
	LuaStack stack(L);
	stack.get_material(1)->set_vector2(stack.get_string_id_32(2), stack.get_vector2(3));
	return 0;
}

static int material_set_vector3(lua_State* L)
{
	LuaStack stack(L);
	stack.get_material(1)->set_vector3(stack.get_string_id_32(2), stack.get_vector3(3));
	return 0;
}

static int gui_move(lua_State* L)
{
	LuaStack stack(L);
	stack.get_gui(1)->move(stack.get_vector2(2));
	return 0;
}

static int gui_triangle(lua_State* L)
{
	LuaStack stack(L);
	stack.get_gui(1)->triangle(stack.get_vector2(2)
		, stack.get_vector2(3)
		, stack.get_vector2(4)
		, stack.get_color4(5)
		);
	return 0;
}

static int gui_rect(lua_State* L)
{
	LuaStack stack(L);
	stack.get_gui(1)->rect(stack.get_vector2(2)
		, stack.get_vector2(3)
		, stack.get_color4(4)
		);
	return 0;
}

static int gui_image(lua_State* L)
{
	LuaStack stack(L);
	stack.get_gui(1)->image(stack.get_vector2(2)
		, stack.get_vector2(3)
		, stack.get_resource_id(4)
		, stack.get_color4(5)
		);
	return 0;
}

static int gui_image_uv(lua_State* L)
{
	LuaStack stack(L);
	stack.get_gui(1)->image_uv(stack.get_vector2(2)
		, stack.get_vector2(3)
		, stack.get_vector2(4)
		, stack.get_vector2(5)
		, stack.get_resource_id(6)
		, stack.get_color4(7)
		);
	return 0;
}

static int gui_text(lua_State* L)
{
	LuaStack stack(L);
	stack.get_gui(1)->text(stack.get_vector2(2)
		, stack.get_int(3)
		, stack.get_string(4)
		, stack.get_resource_id(5)
		, stack.get_resource_id(6)
		, stack.get_color4(7)
		);
	return 0;
}

static int display_modes(lua_State* L)
{
	LuaStack stack(L);
	TempAllocator1024 ta;
	Array<DisplayMode> modes(ta);
	device()->_display->modes(modes);
	stack.push_table(array::size(modes));
	for (u32 i = 0; i < array::size(modes); ++i)
	{
		stack.push_key_begin(i+1);
		stack.push_table(3);
		{
			stack.push_key_begin("id");
			stack.push_int(modes[i].id);
			stack.push_key_end();

			stack.push_key_begin("width");
			stack.push_int(modes[i].width);
			stack.push_key_end();

			stack.push_key_begin("height");
			stack.push_int(modes[i].height);
			stack.push_key_end();
		}
		stack.push_key_end();
	}
	return 1;
}

static int display_set_mode(lua_State* L)
{
	LuaStack stack(L);
	device()->_display->set_mode(stack.get_int(1));
	return 0;
}

static int window_show(lua_State* L)
{
	LuaStack stack(L);
	device()->_window->show();
	return 0;
}

static int window_hide(lua_State* L)
{
	LuaStack stack(L);
	device()->_window->hide();
	return 0;
}

static int window_resize(lua_State* L)
{
	LuaStack stack(L);
	device()->_window->resize(stack.get_int(1), stack.get_int(2));
	return 0;
}

static int window_move(lua_State* L)
{
	LuaStack stack(L);
	device()->_window->move(stack.get_int(1), stack.get_int(2));
	return 0;
}

static int window_minimize(lua_State* /*L*/)
{
	device()->_window->minimize();
	return 0;
}

static int window_restore(lua_State* /*L*/)
{
	device()->_window->restore();
	return 0;
}

static int window_title(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(device()->_window->title());
	return 1;
}

static int window_set_title(lua_State* L)
{
	LuaStack stack(L);
	device()->_window->set_title(stack.get_string(1));
	return 0;
}

static int window_show_cursor(lua_State* L)
{
	LuaStack stack(L);
	device()->_window->show_cursor(stack.get_bool(1));
	return 0;
}

void load_api(LuaEnvironment& env)
{
	env.add_module_function("Math", "ray_plane_intersection",    math_ray_plane_intersection);
	env.add_module_function("Math", "ray_disc_intersection",     math_ray_disc_intersection);
	env.add_module_function("Math", "ray_sphere_intersection",   math_ray_sphere_intersection);
	env.add_module_function("Math", "ray_obb_intersection",      math_ray_obb_intersection);
	env.add_module_function("Math", "ray_triangle_intersection", math_ray_triangle_intersection);

	env.add_module_function("Vector3", "x",                vector3_x);
	env.add_module_function("Vector3", "y",                vector3_y);
	env.add_module_function("Vector3", "z",                vector3_z);
	env.add_module_function("Vector3", "set_x",            vector3_set_x);
	env.add_module_function("Vector3", "set_y",            vector3_set_y);
	env.add_module_function("Vector3", "set_z",            vector3_set_z);
	env.add_module_function("Vector3", "elements",         vector3_elements);
	env.add_module_function("Vector3", "add",              vector3_add);
	env.add_module_function("Vector3", "subtract",         vector3_subtract);
	env.add_module_function("Vector3", "multiply",         vector3_multiply);
	env.add_module_function("Vector3", "dot",              vector3_dot);
	env.add_module_function("Vector3", "cross",            vector3_cross);
	env.add_module_function("Vector3", "equal",            vector3_equal);
	env.add_module_function("Vector3", "length",           vector3_length);
	env.add_module_function("Vector3", "length_squared",   vector3_length_squared);
	env.add_module_function("Vector3", "set_length",       vector3_set_length);
	env.add_module_function("Vector3", "normalize",        vector3_normalize);
	env.add_module_function("Vector3", "distance",         vector3_distance);
	env.add_module_function("Vector3", "distance_squared", vector3_distance_squared);
	env.add_module_function("Vector3", "angle",            vector3_angle);
	env.add_module_function("Vector3", "max",              vector3_max);
	env.add_module_function("Vector3", "min",              vector3_min);
	env.add_module_function("Vector3", "lerp",             vector3_lerp);
	env.add_module_function("Vector3", "forward",          vector3_forward);
	env.add_module_function("Vector3", "backward",         vector3_backward);
	env.add_module_function("Vector3", "left",             vector3_left);
	env.add_module_function("Vector3", "right",            vector3_right);
	env.add_module_function("Vector3", "up",               vector3_up);
	env.add_module_function("Vector3", "down",             vector3_down);
	env.add_module_function("Vector3", "zero",             vector3_zero);
	env.add_module_function("Vector3", "to_string",        vector3_to_string);
	env.add_module_metafunction("Vector3", "__call", vector3_ctor);

	env.add_module_metafunction("Vector2", "__call", vector2_ctor);

	env.add_module_function("Vector3Box", "store", vector3box_store);
	env.add_module_function("Vector3Box", "unbox", vector3box_unbox);
	env.add_module_metafunction("Vector3Box", "store",      vector3box_store);
	env.add_module_metafunction("Vector3Box", "unbox",      vector3box_unbox);
	env.add_module_metafunction("Vector3Box", "__index",    NULL);
	env.add_module_metafunction("Vector3Box", "__tostring", vector3box_tostring);
	env.add_module_metafunction("Vector3Box", "__call",     vector3box_ctor);

	env.add_module_function("Matrix4x4", "from_quaternion",             matrix4x4_from_quaternion);
	env.add_module_function("Matrix4x4", "from_translation",            matrix4x4_from_translation);
	env.add_module_function("Matrix4x4", "from_quaternion_translation", matrix4x4_from_quaternion_translation);
	env.add_module_function("Matrix4x4", "from_axes",                   matrix4x4_from_axes);
	env.add_module_function("Matrix4x4", "copy",                        matrix4x4_copy);
	env.add_module_function("Matrix4x4", "add",                         matrix4x4_add);
	env.add_module_function("Matrix4x4", "subtract",                    matrix4x4_subtract);
	env.add_module_function("Matrix4x4", "multiply",                    matrix4x4_multiply);
	env.add_module_function("Matrix4x4", "transpose",                   matrix4x4_transpose);
	env.add_module_function("Matrix4x4", "invert",                      matrix4x4_invert);
	env.add_module_function("Matrix4x4", "x",                           matrix4x4_x);
	env.add_module_function("Matrix4x4", "y",                           matrix4x4_y);
	env.add_module_function("Matrix4x4", "z",                           matrix4x4_z);
	env.add_module_function("Matrix4x4", "set_x",                       matrix4x4_set_x);
	env.add_module_function("Matrix4x4", "set_y",                       matrix4x4_set_y);
	env.add_module_function("Matrix4x4", "set_z",                       matrix4x4_set_z);
	env.add_module_function("Matrix4x4", "translation",                 matrix4x4_translation);
	env.add_module_function("Matrix4x4", "set_translation",             matrix4x4_set_translation);
	env.add_module_function("Matrix4x4", "rotation",                    matrix4x4_rotation);
	env.add_module_function("Matrix4x4", "set_rotation",                matrix4x4_set_rotation);
	env.add_module_function("Matrix4x4", "identity",                    matrix4x4_identity);
	env.add_module_function("Matrix4x4", "transform",                   matrix4x4_transform);
	env.add_module_function("Matrix4x4", "to_string",                   matrix4x4_to_string);
	env.add_module_metafunction("Matrix4x4", "__call", matrix4x4_ctor);

	env.add_module_function("Matrix4x4Box", "store", matrix4x4box_store);
	env.add_module_function("Matrix4x4Box", "unbox", matrix4x4box_unbox);
	env.add_module_metafunction("Matrix4x4Box", "store",      matrix4x4box_store);
	env.add_module_metafunction("Matrix4x4Box", "unbox",      matrix4x4box_unbox);
	env.add_module_metafunction("Matrix4x4Box", "__index",    NULL);
	env.add_module_metafunction("Matrix4x4Box", "__tostring", matrix4x4box_tostring);
	env.add_module_metafunction("Matrix4x4Box", "__call",     matrix4x4box_ctor);

	env.add_module_function("Quaternion", "from_axis_angle",    quaternion_from_axis_angle);
	env.add_module_function("Quaternion", "from_elements",      quaternion_from_elements);
	env.add_module_function("Quaternion", "negate",             quaternion_negate);
	env.add_module_function("Quaternion", "identity",           quaternion_identity);
	env.add_module_function("Quaternion", "multiply",           quaternion_multiply);
	env.add_module_function("Quaternion", "multiply_by_scalar", quaternion_multiply_by_scalar);
	env.add_module_function("Quaternion", "dot",                quaternion_dot);
	env.add_module_function("Quaternion", "length",             quaternion_length);
	env.add_module_function("Quaternion", "normalize",          quaternion_normalize);
	env.add_module_function("Quaternion", "conjugate",          quaternion_conjugate);
	env.add_module_function("Quaternion", "inverse",            quaternion_inverse);
	env.add_module_function("Quaternion", "power",              quaternion_power);
	env.add_module_function("Quaternion", "elements",           quaternion_elements);
	env.add_module_function("Quaternion", "look",               quaternion_look);
	env.add_module_function("Quaternion", "right",              quaternion_right);
	env.add_module_function("Quaternion", "up",                 quaternion_up);
	env.add_module_function("Quaternion", "forward",            quaternion_forward);
	env.add_module_function("Quaternion", "lerp",               quaternion_lerp);
	env.add_module_function("Quaternion", "to_string",          quaternion_to_string);
	env.add_module_metafunction("Quaternion", "__call", quaternion_ctor);

	env.add_module_function("QuaternionBox", "store", quaternionbox_store);
	env.add_module_function("QuaternionBox", "unbox", quaternionbox_unbox);
	env.add_module_metafunction("QuaternionBox", "store",      quaternionbox_store);
	env.add_module_metafunction("QuaternionBox", "unbox",      quaternionbox_unbox);
	env.add_module_metafunction("QuaternionBox", "__index",    NULL);
	env.add_module_metafunction("QuaternionBox", "__tostring", quaternionbox_tostring);
	env.add_module_metafunction("QuaternionBox", "__call",     quaternionbox_ctor);

	env.add_module_function("Color4", "lerp",      color4_lerp);
	env.add_module_function("Color4", "black",     color4_black);
	env.add_module_function("Color4", "white",     color4_white);
	env.add_module_function("Color4", "red",       color4_red);
	env.add_module_function("Color4", "green",     color4_green);
	env.add_module_function("Color4", "blue",      color4_blue);
	env.add_module_function("Color4", "yellow",    color4_yellow);
	env.add_module_function("Color4", "orange",    color4_orange);
	env.add_module_function("Color4", "to_string", quaternion_to_string);
	env.add_module_metafunction("Color4", "__call", color4_ctor);

	env.add_module_metafunction("Lightuserdata_mt", "__add",      lightuserdata_add);
	env.add_module_metafunction("Lightuserdata_mt", "__sub",      lightuserdata_sub);
	env.add_module_metafunction("Lightuserdata_mt", "__mul",      lightuserdata_mul);
	env.add_module_metafunction("Lightuserdata_mt", "__unm",      lightuserdata_unm);
	env.add_module_metafunction("Lightuserdata_mt", "__index",    lightuserdata_index);
	env.add_module_metafunction("Lightuserdata_mt", "__newindex", lightuserdata_newindex);

	env.add_module_function("Keyboard", "name",         keyboard_name);
	env.add_module_function("Keyboard", "connected",    keyboard_connected);
	env.add_module_function("Keyboard", "num_buttons",  keyboard_num_buttons);
	env.add_module_function("Keyboard", "num_axes",     keyboard_num_axes);
	env.add_module_function("Keyboard", "pressed",      keyboard_pressed);
	env.add_module_function("Keyboard", "released",     keyboard_released);
	env.add_module_function("Keyboard", "any_pressed",  keyboard_any_pressed);
	env.add_module_function("Keyboard", "any_released", keyboard_any_released);
	env.add_module_function("Keyboard", "button",       keyboard_button);
	env.add_module_function("Keyboard", "button_name",  keyboard_button_name);
	env.add_module_function("Keyboard", "button_id",    keyboard_button_id);

	env.add_module_function("Mouse", "name",         mouse_name);
	env.add_module_function("Mouse", "connected",    mouse_connected);
	env.add_module_function("Mouse", "num_buttons",  mouse_num_buttons);
	env.add_module_function("Mouse", "num_axes",     mouse_num_axes);
	env.add_module_function("Mouse", "pressed",      mouse_pressed);
	env.add_module_function("Mouse", "released",     mouse_released);
	env.add_module_function("Mouse", "any_pressed",  mouse_any_pressed);
	env.add_module_function("Mouse", "any_released", mouse_any_released);
	env.add_module_function("Mouse", "button",       mouse_button);
	env.add_module_function("Mouse", "axis",         mouse_axis);
	env.add_module_function("Mouse", "button_name",  mouse_button_name);
	env.add_module_function("Mouse", "axis_name",    mouse_axis_name);
	env.add_module_function("Mouse", "button_id",    mouse_button_id);
	env.add_module_function("Mouse", "axis_id",      mouse_axis_id);

	env.add_module_function("Touch", "name",         touch_name);
	env.add_module_function("Touch", "connected",    touch_connected);
	env.add_module_function("Touch", "num_buttons",  touch_num_buttons);
	env.add_module_function("Touch", "num_axes",     touch_num_axes);
	env.add_module_function("Touch", "pressed",      touch_pressed);
	env.add_module_function("Touch", "released",     touch_released);
	env.add_module_function("Touch", "any_pressed",  touch_any_pressed);
	env.add_module_function("Touch", "any_released", touch_any_released);
	env.add_module_function("Touch", "button",       touch_button);
	env.add_module_function("Touch", "axis",         touch_axis);
	env.add_module_function("Touch", "button_name",  touch_button_name);
	env.add_module_function("Touch", "axis_name",    touch_axis_name);
	env.add_module_function("Touch", "button_id",    touch_button_id);
	env.add_module_function("Touch", "axis_id",      touch_axis_id);

	env.add_module_function("Pad1", "name",         pad0_name);
	env.add_module_function("Pad1", "connected",    pad0_connected);
	env.add_module_function("Pad1", "num_buttons",  pad0_num_buttons);
	env.add_module_function("Pad1", "num_axes",     pad0_num_axes);
	env.add_module_function("Pad1", "pressed",      pad0_pressed);
	env.add_module_function("Pad1", "released",     pad0_released);
	env.add_module_function("Pad1", "any_pressed",  pad0_any_pressed);
	env.add_module_function("Pad1", "any_released", pad0_any_released);
	env.add_module_function("Pad1", "button",       pad0_button);
	env.add_module_function("Pad1", "axis",         pad0_axis);
	env.add_module_function("Pad1", "button_name",  pad0_button_name);
	env.add_module_function("Pad1", "axis_name",    pad0_axis_name);
	env.add_module_function("Pad1", "button_id",    pad0_button_id);
	env.add_module_function("Pad1", "axis_id",      pad0_axis_id);
	env.add_module_function("Pad1", "deadzone",     pad0_deadzone);
	env.add_module_function("Pad1", "set_deadzone", pad0_set_deadzone);

	env.add_module_function("Pad2", "name",         pad1_name);
	env.add_module_function("Pad2", "connected",    pad1_connected);
	env.add_module_function("Pad2", "num_buttons",  pad1_num_buttons);
	env.add_module_function("Pad2", "num_axes",     pad1_num_axes);
	env.add_module_function("Pad2", "pressed",      pad1_pressed);
	env.add_module_function("Pad2", "released",     pad1_released);
	env.add_module_function("Pad2", "any_pressed",  pad1_any_pressed);
	env.add_module_function("Pad2", "any_released", pad1_any_released);
	env.add_module_function("Pad2", "button",       pad1_button);
	env.add_module_function("Pad2", "axis",         pad1_axis);
	env.add_module_function("Pad2", "button_name",  pad1_button_name);
	env.add_module_function("Pad2", "axis_name",    pad1_axis_name);
	env.add_module_function("Pad2", "button_id",    pad1_button_id);
	env.add_module_function("Pad2", "axis_id",      pad1_axis_id);
	env.add_module_function("Pad2", "deadzone",     pad1_deadzone);
	env.add_module_function("Pad2", "set_deadzone", pad1_set_deadzone);

	env.add_module_function("Pad3", "name",         pad2_name);
	env.add_module_function("Pad3", "connected",    pad2_connected);
	env.add_module_function("Pad3", "num_buttons",  pad2_num_buttons);
	env.add_module_function("Pad3", "num_axes",     pad2_num_axes);
	env.add_module_function("Pad3", "pressed",      pad2_pressed);
	env.add_module_function("Pad3", "released",     pad2_released);
	env.add_module_function("Pad3", "any_pressed",  pad2_any_pressed);
	env.add_module_function("Pad3", "any_released", pad2_any_released);
	env.add_module_function("Pad3", "button",       pad2_button);
	env.add_module_function("Pad3", "axis",         pad2_axis);
	env.add_module_function("Pad3", "button_name",  pad2_button_name);
	env.add_module_function("Pad3", "axis_name",    pad2_axis_name);
	env.add_module_function("Pad3", "button_id",    pad2_button_id);
	env.add_module_function("Pad3", "axis_id",      pad2_axis_id);
	env.add_module_function("Pad3", "deadzone",     pad2_deadzone);
	env.add_module_function("Pad3", "set_deadzone", pad2_set_deadzone);

	env.add_module_function("Pad4", "name",         pad3_name);
	env.add_module_function("Pad4", "connected",    pad3_connected);
	env.add_module_function("Pad4", "num_buttons",  pad3_num_buttons);
	env.add_module_function("Pad4", "num_axes",     pad3_num_axes);
	env.add_module_function("Pad4", "pressed",      pad3_pressed);
	env.add_module_function("Pad4", "released",     pad3_released);
	env.add_module_function("Pad4", "any_pressed",  pad3_any_pressed);
	env.add_module_function("Pad4", "any_released", pad3_any_released);
	env.add_module_function("Pad4", "button",       pad3_button);
	env.add_module_function("Pad4", "axis",         pad3_axis);
	env.add_module_function("Pad4", "button_name",  pad3_button_name);
	env.add_module_function("Pad4", "axis_name",    pad3_axis_name);
	env.add_module_function("Pad4", "button_id",    pad3_button_id);
	env.add_module_function("Pad4", "axis_id",      pad3_axis_id);
	env.add_module_function("Pad4", "deadzone",     pad3_deadzone);
	env.add_module_function("Pad4", "set_deadzone", pad3_set_deadzone);

	env.add_module_function("World", "spawn_unit",                      world_spawn_unit);
	env.add_module_function("World", "spawn_empty_unit",                world_spawn_empty_unit);
	env.add_module_function("World", "destroy_unit",                    world_destroy_unit);
	env.add_module_function("World", "num_units",                       world_num_units);
	env.add_module_function("World", "units",                           world_units);
	env.add_module_function("World", "camera_create",                   world_camera_create);
	env.add_module_function("World", "camera_instances",                world_camera_instances);
	env.add_module_function("World", "camera_set_projection_type",      world_camera_set_projection_type);
	env.add_module_function("World", "camera_projection_type",          world_camera_projection_type);
	env.add_module_function("World", "camera_fov",                      world_camera_fov);
	env.add_module_function("World", "camera_set_fov",                  world_camera_set_fov);
	env.add_module_function("World", "camera_near_clip_distance",       world_camera_near_clip_distance);
	env.add_module_function("World", "camera_set_near_clip_distance",   world_camera_set_near_clip_distance);
	env.add_module_function("World", "camera_far_clip_distance",        world_camera_far_clip_distance);
	env.add_module_function("World", "camera_set_far_clip_distance",    world_camera_set_far_clip_distance);
	env.add_module_function("World", "camera_set_orthographic_size",    world_camera_set_orthographic_size);
	env.add_module_function("World", "camera_screen_to_world",          world_camera_screen_to_world);
	env.add_module_function("World", "camera_world_to_screen",          world_camera_world_to_screen);
	env.add_module_function("World", "update_animations",               world_update_animations);
	env.add_module_function("World", "update_scene",                    world_update_scene);
	env.add_module_function("World", "update",                          world_update);
	env.add_module_function("World", "play_sound",                      world_play_sound);
	env.add_module_function("World", "stop_sound",                      world_stop_sound);
	env.add_module_function("World", "link_sound",                      world_link_sound);
	env.add_module_function("World", "set_listener_pose",               world_set_listener_pose);
	env.add_module_function("World", "set_sound_position",              world_set_sound_position);
	env.add_module_function("World", "set_sound_range",                 world_set_sound_range);
	env.add_module_function("World", "set_sound_volume",                world_set_sound_volume);
	env.add_module_function("World", "create_debug_line",               world_create_debug_line);
	env.add_module_function("World", "destroy_debug_line",              world_destroy_debug_line);
	env.add_module_function("World", "create_screen_gui",               world_create_screen_gui);
	env.add_module_function("World", "destroy_gui",                     world_destroy_gui);
	env.add_module_function("World", "load_level",                      world_load_level);
	env.add_module_function("World", "scene_graph",                     world_scene_graph);
	env.add_module_function("World", "render_world",                    world_render_world);
	env.add_module_function("World", "physics_world",                   world_physics_world);
	env.add_module_function("World", "sound_world",                     world_sound_world);
	env.add_module_function("World", "animation_state_machine",         world_animation_state_machine);
	env.add_module_metafunction("World", "__tostring", world_tostring);

	env.add_module_function("SceneGraph", "create",             scene_graph_create);
	env.add_module_function("SceneGraph", "destroy",            scene_graph_destroy);
	env.add_module_function("SceneGraph", "instances",          scene_graph_instances);
	env.add_module_function("SceneGraph", "local_position",     scene_graph_local_position);
	env.add_module_function("SceneGraph", "local_rotation",     scene_graph_local_rotation);
	env.add_module_function("SceneGraph", "local_scale",        scene_graph_local_scale);
	env.add_module_function("SceneGraph", "local_pose",         scene_graph_local_pose);
	env.add_module_function("SceneGraph", "world_position",     scene_graph_world_position);
	env.add_module_function("SceneGraph", "world_rotation",     scene_graph_world_rotation);
	env.add_module_function("SceneGraph", "world_pose",         scene_graph_world_pose);
	env.add_module_function("SceneGraph", "set_local_position", scene_graph_set_local_position);
	env.add_module_function("SceneGraph", "set_local_rotation", scene_graph_set_local_rotation);
	env.add_module_function("SceneGraph", "set_local_scale",    scene_graph_set_local_scale);
	env.add_module_function("SceneGraph", "set_local_pose",     scene_graph_set_local_pose);
	env.add_module_function("SceneGraph", "link",               scene_graph_link);
	env.add_module_function("SceneGraph", "unlink",             scene_graph_unlink);

	env.add_module_function("UnitManager", "create", unit_manager_create);
	env.add_module_function("UnitManager", "alive",  unit_manager_alive);

	env.add_module_function("RenderWorld", "mesh_create",          render_world_mesh_create);
	env.add_module_function("RenderWorld", "mesh_destroy",         render_world_mesh_destroy);
	env.add_module_function("RenderWorld", "mesh_instances",       render_world_mesh_instances);
	env.add_module_function("RenderWorld", "mesh_obb",             render_world_mesh_obb);
	env.add_module_function("RenderWorld", "mesh_raycast",         render_world_mesh_raycast);
	env.add_module_function("RenderWorld", "mesh_set_visible",     render_world_mesh_set_visible);
	env.add_module_function("RenderWorld", "sprite_create",        render_world_sprite_create);
	env.add_module_function("RenderWorld", "sprite_destroy",       render_world_sprite_destroy);
	env.add_module_function("RenderWorld", "sprite_instances",     render_world_sprite_instances);
	env.add_module_function("RenderWorld", "sprite_set_frame",     render_world_sprite_set_frame);
	env.add_module_function("RenderWorld", "sprite_set_visible",   render_world_sprite_set_visible);
	env.add_module_function("RenderWorld", "sprite_flip_x",        render_world_sprite_flip_x);
	env.add_module_function("RenderWorld", "sprite_flip_y",        render_world_sprite_flip_y);
	env.add_module_function("RenderWorld", "sprite_set_layer",     render_world_sprite_set_layer);
	env.add_module_function("RenderWorld", "sprite_set_depth",     render_world_sprite_set_depth);
	env.add_module_function("RenderWorld", "sprite_obb",           render_world_sprite_obb);
	env.add_module_function("RenderWorld", "sprite_raycast",       render_world_sprite_raycast);
	env.add_module_function("RenderWorld", "light_create",         render_world_light_create);
	env.add_module_function("RenderWorld", "light_destroy",        render_world_light_destroy);
	env.add_module_function("RenderWorld", "light_instances",      render_world_light_instances);
	env.add_module_function("RenderWorld", "light_type",           render_world_light_type);
	env.add_module_function("RenderWorld", "light_color",          render_world_light_color);
	env.add_module_function("RenderWorld", "light_range",          render_world_light_range);
	env.add_module_function("RenderWorld", "light_intensity",      render_world_light_intensity);
	env.add_module_function("RenderWorld", "light_spot_angle",     render_world_light_spot_angle);
	env.add_module_function("RenderWorld", "light_set_type",       render_world_light_set_type);
	env.add_module_function("RenderWorld", "light_set_color",      render_world_light_set_color);
	env.add_module_function("RenderWorld", "light_set_range",      render_world_light_set_range);
	env.add_module_function("RenderWorld", "light_set_intensity",  render_world_light_set_intensity);
	env.add_module_function("RenderWorld", "light_set_spot_angle", render_world_light_set_spot_angle);
	env.add_module_function("RenderWorld", "light_debug_draw",     render_world_light_debug_draw);
	env.add_module_function("RenderWorld", "enable_debug_drawing", render_world_enable_debug_drawing);

	env.add_module_function("PhysicsWorld", "actor_instances",               physics_world_actor_instances);
	env.add_module_function("PhysicsWorld", "actor_world_position",          physics_world_actor_world_position);
	env.add_module_function("PhysicsWorld", "actor_world_rotation",          physics_world_actor_world_rotation);
	env.add_module_function("PhysicsWorld", "actor_world_pose",              physics_world_actor_world_pose);
	env.add_module_function("PhysicsWorld", "actor_teleport_world_position", physics_world_actor_teleport_world_position);
	env.add_module_function("PhysicsWorld", "actor_teleport_world_rotation", physics_world_actor_teleport_world_rotation);
	env.add_module_function("PhysicsWorld", "actor_teleport_world_pose",     physics_world_actor_teleport_world_pose);
	env.add_module_function("PhysicsWorld", "actor_center_of_mass",          physics_world_actor_center_of_mass);
	env.add_module_function("PhysicsWorld", "actor_enable_gravity",          physics_world_actor_enable_gravity);
	env.add_module_function("PhysicsWorld", "actor_disable_gravity",         physics_world_actor_disable_gravity);
	env.add_module_function("PhysicsWorld", "actor_enable_collision",        physics_world_actor_enable_collision);
	env.add_module_function("PhysicsWorld", "actor_disable_collision",       physics_world_actor_disable_collision);
	env.add_module_function("PhysicsWorld", "actor_set_collision_filter",    physics_world_actor_set_collision_filter);
	env.add_module_function("PhysicsWorld", "actor_set_kinematic",           physics_world_actor_set_kinematic);
	env.add_module_function("PhysicsWorld", "actor_is_static",               physics_world_actor_is_static);
	env.add_module_function("PhysicsWorld", "actor_is_dynamic",              physics_world_actor_is_dynamic);
	env.add_module_function("PhysicsWorld", "actor_is_kinematic",            physics_world_actor_is_kinematic);
	env.add_module_function("PhysicsWorld", "actor_is_nonkinematic",         physics_world_actor_is_nonkinematic);
	env.add_module_function("PhysicsWorld", "actor_linear_damping",          physics_world_actor_linear_damping);
	env.add_module_function("PhysicsWorld", "actor_set_linear_damping",      physics_world_actor_set_linear_damping);
	env.add_module_function("PhysicsWorld", "actor_angular_damping",         physics_world_actor_angular_damping);
	env.add_module_function("PhysicsWorld", "actor_set_angular_damping",     physics_world_actor_set_angular_damping);
	env.add_module_function("PhysicsWorld", "actor_linear_velocity",         physics_world_actor_linear_velocity);
	env.add_module_function("PhysicsWorld", "actor_set_linear_velocity",     physics_world_actor_set_linear_velocity);
	env.add_module_function("PhysicsWorld", "actor_angular_velocity",        physics_world_actor_angular_velocity);
	env.add_module_function("PhysicsWorld", "actor_set_angular_velocity",    physics_world_actor_set_angular_velocity);
	env.add_module_function("PhysicsWorld", "actor_add_impulse",             physics_world_actor_add_impulse);
	env.add_module_function("PhysicsWorld", "actor_add_impulse_at",          physics_world_actor_add_impulse_at);
	env.add_module_function("PhysicsWorld", "actor_add_actor_torque_impulse",physics_world_actor_add_actor_torque_impulse);
	env.add_module_function("PhysicsWorld", "actor_push",                    physics_world_actor_push);
	env.add_module_function("PhysicsWorld", "actor_push_at",                 physics_world_actor_push_at);
	env.add_module_function("PhysicsWorld", "actor_is_sleeping",             physics_world_actor_is_sleeping);
	env.add_module_function("PhysicsWorld", "actor_wake_up",                 physics_world_actor_wake_up);
	env.add_module_function("PhysicsWorld", "joint_create",                  physics_world_joint_create);
	env.add_module_function("PhysicsWorld", "gravity",                       physics_world_gravity);
	env.add_module_function("PhysicsWorld", "set_gravity",                   physics_world_set_gravity);
	env.add_module_function("PhysicsWorld", "raycast_closest",               physics_world_raycast_closest);
	env.add_module_function("PhysicsWorld", "raycast_all",                   physics_world_raycast_all);
	env.add_module_function("PhysicsWorld", "enable_debug_drawing",          physics_world_enable_debug_drawing);
	env.add_module_metafunction("PhysicsWorld", "__tostring", physics_world_tostring);

	env.add_module_function("SoundWorld", "stop_all",   sound_world_stop_all);
	env.add_module_function("SoundWorld", "pause_all",  sound_world_pause_all);
	env.add_module_function("SoundWorld", "resume_all", sound_world_resume_all);
	env.add_module_function("SoundWorld", "is_playing", sound_world_is_playing);
	env.add_module_metafunction("SoundWorld", "__tostring", sound_world_tostring);

	env.add_module_function("AnimationStateMachine", "trigger",      animation_state_machine_trigger);
	env.add_module_function("AnimationStateMachine", "variable_id",  animation_state_machine_variable_id);
	env.add_module_function("AnimationStateMachine", "variable",     animation_state_machine_variable);
	env.add_module_function("AnimationStateMachine", "set_variable", animation_state_machine_set_variable);

	env.add_module_function("Device", "argv",                     device_argv);
	env.add_module_function("Device", "platform",                 device_platform);
	env.add_module_function("Device", "architecture",             device_architecture);
	env.add_module_function("Device", "version",                  device_version);
	env.add_module_function("Device", "quit",                     device_quit);
	env.add_module_function("Device", "resolution",               device_resolution);
	env.add_module_function("Device", "create_world",             device_create_world);
	env.add_module_function("Device", "destroy_world",            device_destroy_world);
	env.add_module_function("Device", "render",                   device_render);
	env.add_module_function("Device", "create_resource_package",  device_create_resource_package);
	env.add_module_function("Device", "destroy_resource_package", device_destroy_resource_package);
	env.add_module_function("Device", "console_send",             device_console_send);
	env.add_module_function("Device", "can_get",                  device_can_get);
	env.add_module_function("Device", "enable_resource_autoload", device_enable_resource_autoload);
	env.add_module_function("Device", "temp_count",               device_temp_count);
	env.add_module_function("Device", "set_temp_count",           device_set_temp_count);
	env.add_module_function("Device", "guid",                     device_guid);

	env.add_module_function("Profiler", "enter_scope", profiler_enter_scope);
	env.add_module_function("Profiler", "leave_scope", profiler_leave_scope);
	env.add_module_function("Profiler", "record",      profiler_record);

	env.add_module_function("DebugLine", "add_line",    debug_line_add_line);
	env.add_module_function("DebugLine", "add_axes",    debug_line_add_axes);
	env.add_module_function("DebugLine", "add_arc",     debug_line_add_arc);
	env.add_module_function("DebugLine", "add_circle",  debug_line_add_circle);
	env.add_module_function("DebugLine", "add_cone",    debug_line_add_cone);
	env.add_module_function("DebugLine", "add_sphere",  debug_line_add_sphere);
	env.add_module_function("DebugLine", "add_obb",     debug_line_add_obb);
	env.add_module_function("DebugLine", "add_frustum", debug_line_add_frustum);
	env.add_module_function("DebugLine", "add_unit",    debug_line_add_unit);
	env.add_module_function("DebugLine", "reset",       debug_line_reset);
	env.add_module_function("DebugLine", "submit",      debug_line_submit);
	env.add_module_metafunction("DebugLine", "__tostring", debug_line_tostring);

	env.add_module_function("ResourcePackage", "load",       resource_package_load);
	env.add_module_function("ResourcePackage", "unload",     resource_package_unload);
	env.add_module_function("ResourcePackage", "flush",      resource_package_flush);
	env.add_module_function("ResourcePackage", "has_loaded", resource_package_has_loaded);
	env.add_module_metafunction("ResourcePackage", "__tostring", resource_package_tostring);

	env.add_module_function("Material", "set_float",   material_set_float);
	env.add_module_function("Material", "set_vector2", material_set_vector2);
	env.add_module_function("Material", "set_vector3", material_set_vector3);

	env.add_module_function("Gui", "move",          gui_move);
	env.add_module_function("Gui", "triangle",      gui_triangle);
	env.add_module_function("Gui", "rect",          gui_rect);
	env.add_module_function("Gui", "image",         gui_image);
	env.add_module_function("Gui", "image_uv",      gui_image_uv);
	env.add_module_function("Gui", "text",          gui_text);

	env.add_module_function("Display", "modes",    display_modes);
	env.add_module_function("Display", "set_mode", display_set_mode);

	env.add_module_function("Window", "show",        window_show);
	env.add_module_function("Window", "hide",        window_hide);
	env.add_module_function("Window", "resize",      window_resize);
	env.add_module_function("Window", "move",        window_move);
	env.add_module_function("Window", "minimize",    window_minimize);
	env.add_module_function("Window", "restore",     window_restore);
	env.add_module_function("Window", "title",       window_title);
	env.add_module_function("Window", "set_title",   window_set_title);
	env.add_module_function("Window", "show_cursor", window_show_cursor);
}

} // namespace crown
