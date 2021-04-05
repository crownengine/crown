/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/guid.h"
#include "core/math/color4.inl"
#include "core/math/constants.h"
#include "core/math/intersection.h"
#include "core/math/math.h"
#include "core/math/matrix4x4.inl"
#include "core/math/plane3.inl"
#include "core/math/quaternion.inl"
#include "core/math/types.h"
#include "core/math/vector2.inl"
#include "core/math/vector3.inl"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_stream.inl"
#include "device/console_server.h"
#include "device/device.h"
#include "device/input_device.h"
#include "device/input_manager.h"
#include "device/profiler.h"
#include "lua/lua_environment.h"
#include "lua/lua_stack.inl"
#include "resource/resource_id.inl"
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

struct MouseCursorInfo
{
	const char* name;
	MouseCursor::Enum type;
};

static const MouseCursorInfo s_cursor[] =
{
	{ "arrow",              MouseCursor::ARROW               },
	{ "hand",               MouseCursor::HAND                },
	{ "text_input",         MouseCursor::TEXT_INPUT          },
	{ "corner_top_left",    MouseCursor::CORNER_TOP_LEFT     },
	{ "corner_top_right",   MouseCursor::CORNER_TOP_RIGHT    },
	{ "corner_bottom_left", MouseCursor::CORNER_BOTTOM_LEFT  },
	{ "corner_bottom_right",MouseCursor::CORNER_BOTTOM_RIGHT },
	{ "size_horizontal",    MouseCursor::SIZE_HORIZONTAL     },
	{ "size_vertical",      MouseCursor::SIZE_VERTICAL       },
	{ "wait",               MouseCursor::WAIT                }
};
CE_STATIC_ASSERT(countof(s_cursor) == MouseCursor::COUNT);

struct CursorModeInfo
{
	const char*name;
	CursorMode::Enum type;
};

static const CursorModeInfo s_mode[] =
{
	{ "disabled", CursorMode::DISABLED },
	{ "normal", CursorMode::NORMAL }
};
CE_STATIC_ASSERT(countof(s_mode) == CursorMode::COUNT);

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

static MouseCursor::Enum name_to_mouse_cursor(const char* name)
{
	for (u32 i = 0; i < countof(s_cursor); ++i)
	{
		if (strcmp(s_cursor[i].name, name) == 0)
			return s_cursor[i].type;
	}

	return MouseCursor::COUNT;
}

static CursorMode::Enum name_to_cursor_mode(const char* name)
{
	for (u32 i = 0; i < countof(s_mode); ++i)
	{
		if (strcmp(s_mode[i].name, name) == 0)
			return s_mode[i].type;
	}

	return CursorMode::COUNT;
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

static int quaternion_to_string(lua_State* L)
{
	LuaStack stack(L);
	char buf[256];
	stack.push_string(to_string(stack.get_quaternion(1), buf, sizeof(buf)));
	return 1;
}

static int quaternionbox_store(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternionbox(1);

	if (stack.num_args() == 2)
		q = stack.get_quaternion(2);
	else
		q = from_elements(stack.get_float(2)
			, stack.get_float(3)
			, stack.get_float(4)
			, stack.get_float(5)
			);

	return 0;
}

static int quaternionbox_unbox(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternionbox(1);

	stack.push_quaternion(q);
	return 1;
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
	const u8 button = dev.any_pressed();
	if (button != UINT8_MAX)
		stack.push_int(button);
	else
		stack.push_nil();
	return 1;
}

static int input_device_any_released(lua_State* L, InputDevice& dev)
{
	LuaStack stack(L);
	const u8 button = dev.any_released();
	if (button != UINT8_MAX)
		stack.push_int(button);
	else
		stack.push_nil();
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

void load_api(LuaEnvironment& env)
{
	env.add_module_function("Math", "ray_plane_intersection", [](lua_State* L)
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
		});
	env.add_module_function("Math", "ray_disc_intersection", [](lua_State* L)
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
		});
	env.add_module_function("Math", "ray_sphere_intersection", [](lua_State* L)
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
		});
	env.add_module_function("Math", "ray_obb_intersection", [](lua_State* L)
		{
			LuaStack stack(L);
			const f32 t = ray_obb_intersection(stack.get_vector3(1)
				, stack.get_vector3(2)
				, stack.get_matrix4x4(3)
				, stack.get_vector3(4)
				);
			stack.push_float(t);
			return 1;
		});
	env.add_module_function("Math", "ray_triangle_intersection", [](lua_State* L)
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
		});
	env.add_module_function("Math", "obb_intersects_frustum", [](lua_State* L)
		{
			LuaStack stack(L);

			u32 param_i = 1;

			OBB obb;
			obb.tm = stack.get_matrix4x4(param_i++);
			obb.half_extents = stack.get_vector3(param_i++);

			Frustum f;
			for (u32 ii = 0; ii < countof(f.planes); ++ii)
			{
				f.planes[ii].n = stack.get_vector3(param_i++);
				f.planes[ii].d = stack.get_float(param_i++);
			}

			stack.push_bool(obb_intersects_frustum(obb, f));
			return 1;
		});

	env.add_module_function("Vector3", "x", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(stack.get_vector3(1).x);
			return 1;
		});
	env.add_module_function("Vector3", "y", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(stack.get_vector3(1).y);
			return 1;
		});
	env.add_module_function("Vector3", "z", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(stack.get_vector3(1).z);
			return 1;
		});
	env.add_module_function("Vector3", "set_x", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_vector3(1).x = stack.get_float(2);
			return 0;
		});
	env.add_module_function("Vector3", "set_y", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_vector3(1).y = stack.get_float(2);
			return 0;
		});
	env.add_module_function("Vector3", "set_z", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_vector3(1).z = stack.get_float(2);
			return 0;
		});
	env.add_module_function("Vector3", "elements", [](lua_State* L)
		{
			LuaStack stack(L);
			Vector3& a = stack.get_vector3(1);
			stack.push_float(a.x);
			stack.push_float(a.y);
			stack.push_float(a.z);
			return 3;
		});
	env.add_module_function("Vector3", "add", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(stack.get_vector3(1) + stack.get_vector3(2));
			return 1;
		});
	env.add_module_function("Vector3", "subtract", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(stack.get_vector3(1) - stack.get_vector3(2));
			return 1;
		});
	env.add_module_function("Vector3", "multiply", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(stack.get_vector3(1) * stack.get_float(2));
			return 1;
		});
	env.add_module_function("Vector3", "dot", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(dot(stack.get_vector3(1), stack.get_vector3(2)));
			return 1;
		});
	env.add_module_function("Vector3", "cross", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(cross(stack.get_vector3(1), stack.get_vector3(2)));
			return 1;
		});
	env.add_module_function("Vector3", "equal", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_bool(stack.get_vector3(1) == stack.get_vector3(2));
			return 1;
		});
	env.add_module_function("Vector3", "length", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(length(stack.get_vector3(1)));
			return 1;
		});
	env.add_module_function("Vector3", "length_squared", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(length_squared(stack.get_vector3(1)));
			return 1;
		});
	env.add_module_function("Vector3", "set_length", [](lua_State* L)
		{
			LuaStack stack(L);
			set_length(stack.get_vector3(1), stack.get_float(2));
			return 0;
		});
	env.add_module_function("Vector3", "normalize", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(normalize(stack.get_vector3(1)));
			return 1;
		});
	env.add_module_function("Vector3", "distance", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(distance(stack.get_vector3(1), stack.get_vector3(2)));
			return 1;
		});
	env.add_module_function("Vector3", "distance_squared", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(distance_squared(stack.get_vector3(1), stack.get_vector3(2)));
			return 1;
		});
	env.add_module_function("Vector3", "angle", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(angle(stack.get_vector3(1), stack.get_vector3(2)));
			return 1;
		});
	env.add_module_function("Vector3", "max", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(max(stack.get_vector3(1), stack.get_vector3(2)));
			return 1;
		});
	env.add_module_function("Vector3", "min", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(min(stack.get_vector3(1), stack.get_vector3(2)));
			return 1;
		});
	env.add_module_function("Vector3", "lerp", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(lerp(stack.get_vector3(1), stack.get_vector3(2), stack.get_float(3)));
			return 1;
		});
	env.add_module_function("Vector3", "forward", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(VECTOR3_FORWARD);
			return 1;
		});
	env.add_module_function("Vector3", "backward", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(VECTOR3_BACKWARD);
			return 1;
		});
	env.add_module_function("Vector3", "left", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(VECTOR3_LEFT);
			return 1;
		});
	env.add_module_function("Vector3", "right", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(VECTOR3_RIGHT);
			return 1;
		});
	env.add_module_function("Vector3", "up", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(VECTOR3_UP);
			return 1;
		});
	env.add_module_function("Vector3", "down", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(VECTOR3_DOWN);
			return 1;
		});
	env.add_module_function("Vector3", "zero", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(VECTOR3_ZERO);
			return 1;
		});
	env.add_module_function("Vector3", "to_string", [](lua_State* L)
		{
			LuaStack stack(L);
			char buf[256];
			stack.push_string(to_string(stack.get_vector3(1), buf, sizeof(buf)));
			return 1;
		});
	env.add_module_metafunction("Vector3", "__call", [](lua_State* L)
		{
			LuaStack stack(L);
			Vector3 v;
			v.x = stack.get_float(1 + 1);
			v.y = stack.get_float(2 + 1);
			v.z = stack.get_float(3 + 1);
			stack.push_vector3(v);
			return 1;
		});

	env.add_module_metafunction("Vector2", "__call", [](lua_State* L)
		{
			LuaStack stack(L);
			Vector2 v;
			v.x = stack.get_float(1 + 1);
			v.y = stack.get_float(2 + 1);
			stack.push_vector2(v);
			return 1;
		});

	env.add_module_function("Vector3Box", "store", vector3box_store);
	env.add_module_function("Vector3Box", "unbox", vector3box_unbox);
	env.add_module_metafunction("Vector3Box", "store",      vector3box_store);
	env.add_module_metafunction("Vector3Box", "unbox",      vector3box_unbox);
	env.add_module_metafunction("Vector3Box", "__index",    NULL);
	env.add_module_metafunction("Vector3Box", "__tostring", [](lua_State* L)
		{
			LuaStack stack(L);
			Vector3& v = stack.get_vector3box(1);
			stack.push_fstring("Vector3Box (%p)", &v);
			return 1;
		});
	env.add_module_metafunction("Vector3Box", "__call", [](lua_State* L)
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
		});

	env.add_module_function("Matrix4x4", "from_quaternion", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_matrix4x4(from_quaternion_translation(stack.get_quaternion(1), VECTOR3_ZERO));
			return 1;
		});
	env.add_module_function("Matrix4x4", "from_translation", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_matrix4x4(from_translation(stack.get_vector3(1)));
			return 1;
		});
	env.add_module_function("Matrix4x4", "from_quaternion_translation", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_matrix4x4(from_quaternion_translation(stack.get_quaternion(1), stack.get_vector3(2)));
			return 1;
		});
	env.add_module_function("Matrix4x4", "from_axes", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_matrix4x4(from_axes(stack.get_vector3(1), stack.get_vector3(2), stack.get_vector3(3), stack.get_vector3(4)));
			return 1;
		});
	env.add_module_function("Matrix4x4", "copy", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_matrix4x4(stack.get_matrix4x4(1));
			return 1;
		});
	env.add_module_function("Matrix4x4", "add", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_matrix4x4(stack.get_matrix4x4(1) + stack.get_matrix4x4(2));
			return 1;
		});
	env.add_module_function("Matrix4x4", "subtract", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_matrix4x4(stack.get_matrix4x4(1) - stack.get_matrix4x4(2));
			return 1;
		});
	env.add_module_function("Matrix4x4", "multiply", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_matrix4x4(stack.get_matrix4x4(1) * stack.get_matrix4x4(2));
			return 1;
		});
	env.add_module_function("Matrix4x4", "equal", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_bool(stack.get_matrix4x4(1) == stack.get_matrix4x4(2));
			return 1;
		});
	env.add_module_function("Matrix4x4", "transpose", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_matrix4x4(transpose(stack.get_matrix4x4(1)));
			return 1;
		});
	env.add_module_function("Matrix4x4", "invert", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_matrix4x4(invert(stack.get_matrix4x4(1)));
			return 1;
		});
	env.add_module_function("Matrix4x4", "x", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(x(stack.get_matrix4x4(1)));
			return 1;
		});
	env.add_module_function("Matrix4x4", "y", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(y(stack.get_matrix4x4(1)));
			return 1;
		});
	env.add_module_function("Matrix4x4", "z", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(z(stack.get_matrix4x4(1)));
			return 1;
		});
	env.add_module_function("Matrix4x4", "set_x", [](lua_State* L)
		{
			LuaStack stack(L);
			set_x(stack.get_matrix4x4(1), stack.get_vector3(2));
			return 0;
		});
	env.add_module_function("Matrix4x4", "set_y", [](lua_State* L)
		{
			LuaStack stack(L);
			set_y(stack.get_matrix4x4(1), stack.get_vector3(2));
			return 0;
		});
	env.add_module_function("Matrix4x4", "set_z", [](lua_State* L)
		{
			LuaStack stack(L);
			set_z(stack.get_matrix4x4(1), stack.get_vector3(2));
			return 0;
		});
	env.add_module_function("Matrix4x4", "translation", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(translation(stack.get_matrix4x4(1)));
			return 1;
		});
	env.add_module_function("Matrix4x4", "set_translation", [](lua_State* L)
		{
			LuaStack stack(L);
			set_translation(stack.get_matrix4x4(1), stack.get_vector3(2));
			return 0;
		});
	env.add_module_function("Matrix4x4", "rotation", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_quaternion(rotation(stack.get_matrix4x4(1)));
			return 1;
		});
	env.add_module_function("Matrix4x4", "set_rotation", [](lua_State* L)
		{
			LuaStack stack(L);
			set_rotation(stack.get_matrix4x4(1), stack.get_quaternion(2));
			return 0;
		});
	env.add_module_function("Matrix4x4", "identity", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_matrix4x4(MATRIX4X4_IDENTITY);
			return 1;
		});
	env.add_module_function("Matrix4x4", "transform", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(stack.get_vector3(2) * stack.get_matrix4x4(1));
			return 1;
		});
	env.add_module_function("Matrix4x4", "to_string", [](lua_State* L)
		{
			LuaStack stack(L);
			char buf[1024];
			stack.push_string(to_string(stack.get_matrix4x4(1), buf, sizeof(buf)));
			return 1;
		});
	env.add_module_metafunction("Matrix4x4", "__call", [](lua_State* L)
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
		});

	env.add_module_function("Matrix4x4Box", "store", matrix4x4box_store);
	env.add_module_function("Matrix4x4Box", "unbox", matrix4x4box_unbox);
	env.add_module_metafunction("Matrix4x4Box", "store",      matrix4x4box_store);
	env.add_module_metafunction("Matrix4x4Box", "unbox",      matrix4x4box_unbox);
	env.add_module_metafunction("Matrix4x4Box", "__index",    NULL);
	env.add_module_metafunction("Matrix4x4Box", "__tostring", [](lua_State* L)
		{
			LuaStack stack(L);
			Matrix4x4& m = stack.get_matrix4x4box(1);
			stack.push_fstring("Matrix4x4Box (%p)", &m);
			return 1;
		});
	env.add_module_metafunction("Matrix4x4Box", "__call", [](lua_State* L)
		{
			LuaStack stack(L);

			if (stack.num_args() == 0 + 1)
				stack.push_matrix4x4box(MATRIX4X4_IDENTITY);
			else
				stack.push_matrix4x4box(stack.get_matrix4x4(1 + 1));

			return 1;
		});

	env.add_module_function("Quaternion", "from_axis_angle", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_quaternion(from_axis_angle(stack.get_vector3(1), stack.get_float(2)));
			return 1;
		});
	env.add_module_function("Quaternion", "from_elements", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_quaternion(from_elements(stack.get_float(1), stack.get_float(2), stack.get_float(3), stack.get_float(4)));
			return 1;
		});
	env.add_module_function("Quaternion", "negate", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_quaternion(-stack.get_quaternion(1));
			return 1;
		});
	env.add_module_function("Quaternion", "identity", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_quaternion(QUATERNION_IDENTITY);
			return 1;
		});
	env.add_module_function("Quaternion", "multiply", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_quaternion(stack.get_quaternion(1) * stack.get_quaternion(2));
			return 1;
		});
	env.add_module_function("Quaternion", "multiply_by_scalar", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_quaternion(stack.get_quaternion(1) * stack.get_float(2));
			return 1;
		});
	env.add_module_function("Quaternion", "dot", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(dot(stack.get_quaternion(1), stack.get_quaternion(2)));
			return 1;
		});
	env.add_module_function("Quaternion", "length", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(length(stack.get_quaternion(1)));
			return 1;
		});
	env.add_module_function("Quaternion", "normalize", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_quaternion(normalize(stack.get_quaternion(1)));
			return 1;
		});
	env.add_module_function("Quaternion", "conjugate", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_quaternion(conjugate(stack.get_quaternion(1)));
			return 1;
		});
	env.add_module_function("Quaternion", "inverse", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_quaternion(inverse(stack.get_quaternion(1)));
			return 1;
		});
	env.add_module_function("Quaternion", "power", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_quaternion(power(stack.get_quaternion(1), stack.get_float(2)));
			return 1;
		});
	env.add_module_function("Quaternion", "elements", [](lua_State* L)
		{
			LuaStack stack(L);
			const Quaternion& q = stack.get_quaternion(1);
			stack.push_float(q.x);
			stack.push_float(q.y);
			stack.push_float(q.z);
			stack.push_float(q.w);
			return 4;
		});
	env.add_module_function("Quaternion", "look", [](lua_State* L)
		{
			LuaStack stack(L);
			const Vector3 up = stack.num_args() == 2 ? stack.get_vector3(2) : VECTOR3_YAXIS;
			stack.push_quaternion(look(stack.get_vector3(1), up));
			return 1;
		});
	env.add_module_function("Quaternion", "right", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(right(stack.get_quaternion(1)));
			return 1;
		});
	env.add_module_function("Quaternion", "up", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(up(stack.get_quaternion(1)));
			return 1;
		});
	env.add_module_function("Quaternion", "forward", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(forward(stack.get_quaternion(1)));
			return 1;
		});
	env.add_module_function("Quaternion", "lerp", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_quaternion(lerp(stack.get_quaternion(1), stack.get_quaternion(2), stack.get_float(3)));
			return 1;
		});
	env.add_module_function("Quaternion", "to_string",          quaternion_to_string);
	env.add_module_metafunction("Quaternion", "__call", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_quaternion(from_axis_angle(stack.get_vector3(1 + 1), stack.get_float(2 + 1)));
			return 1;
		});

	env.add_module_function("QuaternionBox", "store", quaternionbox_store);
	env.add_module_function("QuaternionBox", "unbox", quaternionbox_unbox);
	env.add_module_metafunction("QuaternionBox", "store",      quaternionbox_store);
	env.add_module_metafunction("QuaternionBox", "unbox",      quaternionbox_unbox);
	env.add_module_metafunction("QuaternionBox", "__index",    NULL);
	env.add_module_metafunction("QuaternionBox", "__tostring", [](lua_State* L)
		{
			LuaStack stack(L);
			Quaternion& q = stack.get_quaternionbox(1);
			stack.push_fstring("QuaternionBox (%p)", &q);
			return 1;
		});
	env.add_module_metafunction("QuaternionBox", "__call", [](lua_State* L)
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
				Quaternion q = from_elements(stack.get_float(1 + 1)
					, stack.get_float(2 + 1)
					, stack.get_float(3 + 1)
					, stack.get_float(4 + 1)
					);
				stack.push_quaternionbox(q);
			}

			return 1;
		});

	env.add_module_function("Color4", "lerp", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_color4(lerp(stack.get_color4(1), stack.get_color4(2), stack.get_float(3)));
			return 1;
		});
	env.add_module_function("Color4", "black", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_color4(COLOR4_BLACK);
			return 1;
		});
	env.add_module_function("Color4", "white", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_color4(COLOR4_WHITE);
			return 1;
		});
	env.add_module_function("Color4", "red", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_color4(COLOR4_RED);
			return 1;
		});
	env.add_module_function("Color4", "green", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_color4(COLOR4_GREEN);
			return 1;
		});
	env.add_module_function("Color4", "blue", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_color4(COLOR4_BLUE);
			return 1;
		});
	env.add_module_function("Color4", "yellow", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_color4(COLOR4_YELLOW);
			return 1;
		});
	env.add_module_function("Color4", "orange", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_color4(COLOR4_ORANGE);
			return 1;
		});
	env.add_module_function("Color4", "to_string", quaternion_to_string);
	env.add_module_metafunction("Color4", "__call", [](lua_State* L)
		{
			LuaStack stack(L);
			u8 r = (u8)stack.get_int(1 + 1);
			u8 g = (u8)stack.get_int(2 + 1);
			u8 b = (u8)stack.get_int(3 + 1);
			u8 a = (u8)stack.get_int(4 + 1);
			stack.push_color4(from_rgba(r, g, b, a));
			return 1;
		});

	env.add_module_metafunction("Lightuserdata_mt", "__add", [](lua_State* L)
		{
			LuaStack stack(L);
			const Vector3& a = stack.get_vector3(1);
			const Vector3& b = stack.get_vector3(2);
			stack.push_vector3(a + b);
			return 1;
		});
	env.add_module_metafunction("Lightuserdata_mt", "__sub", [](lua_State* L)
		{
			LuaStack stack(L);
			const Vector3& a = stack.get_vector3(1);
			const Vector3& b = stack.get_vector3(2);
			stack.push_vector3(a - b);
			return 1;
		});
	env.add_module_metafunction("Lightuserdata_mt", "__mul", [](lua_State* L)
		{
			LuaStack stack(L);
			const int i = stack.is_number(1) ? 1 : 2;
			stack.push_vector3(stack.get_float(i) * stack.get_vector3(3-i));
			return 1;
		});
	env.add_module_metafunction("Lightuserdata_mt", "__unm", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(-stack.get_vector3(1));
			return 1;
		});
	env.add_module_metafunction("Lightuserdata_mt", "__index", [](lua_State* L)
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
		});
	env.add_module_metafunction("Lightuserdata_mt", "__newindex", [](lua_State* L)
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
		});

#define KEYBOARD(name)                                                          \
	[](lua_State* L)                                                            \
	{                                                                           \
		return input_device_ ## name(L, *device()->_input_manager->keyboard()); \
	}

#define MOUSE(name)                                                          \
	[](lua_State* L)                                                         \
	{                                                                        \
		return input_device_ ## name(L, *device()->_input_manager->mouse()); \
	}

#define TOUCH(name)                                                          \
	[](lua_State* L)                                                         \
	{                                                                        \
		return input_device_ ## name(L, *device()->_input_manager->touch()); \
	}

#define PAD(index, name)                                                           \
	[](lua_State* L)                                                               \
	{                                                                              \
		return input_device_ ## name(L, *device()->_input_manager->joypad(index)); \
	}

	env.add_module_function("Keyboard", "name",         KEYBOARD(name));
	env.add_module_function("Keyboard", "connected",    KEYBOARD(connected));
	env.add_module_function("Keyboard", "num_buttons",  KEYBOARD(num_buttons));
	env.add_module_function("Keyboard", "num_axes",     KEYBOARD(num_axes));
	env.add_module_function("Keyboard", "pressed",      KEYBOARD(pressed));
	env.add_module_function("Keyboard", "released",     KEYBOARD(released));
	env.add_module_function("Keyboard", "any_pressed",  KEYBOARD(any_pressed));
	env.add_module_function("Keyboard", "any_released", KEYBOARD(any_released));
	env.add_module_function("Keyboard", "button",       KEYBOARD(button));
	env.add_module_function("Keyboard", "button_name",  KEYBOARD(button_name));
	env.add_module_function("Keyboard", "button_id",    KEYBOARD(button_id));

	env.add_module_function("Mouse", "name",         MOUSE(name));
	env.add_module_function("Mouse", "connected",    MOUSE(connected));
	env.add_module_function("Mouse", "num_buttons",  MOUSE(num_buttons));
	env.add_module_function("Mouse", "num_axes",     MOUSE(num_axes));
	env.add_module_function("Mouse", "pressed",      MOUSE(pressed));
	env.add_module_function("Mouse", "released",     MOUSE(released));
	env.add_module_function("Mouse", "any_pressed",  MOUSE(any_pressed));
	env.add_module_function("Mouse", "any_released", MOUSE(any_released));
	env.add_module_function("Mouse", "button",       MOUSE(button));
	env.add_module_function("Mouse", "axis",         MOUSE(axis));
	env.add_module_function("Mouse", "button_name",  MOUSE(button_name));
	env.add_module_function("Mouse", "axis_name",    MOUSE(axis_name));
	env.add_module_function("Mouse", "button_id",    MOUSE(button_id));
	env.add_module_function("Mouse", "axis_id",      MOUSE(axis_id));

	env.add_module_function("Touch", "name",         TOUCH(name));
	env.add_module_function("Touch", "connected",    TOUCH(connected));
	env.add_module_function("Touch", "num_buttons",  TOUCH(num_buttons));
	env.add_module_function("Touch", "num_axes",     TOUCH(num_axes));
	env.add_module_function("Touch", "pressed",      TOUCH(pressed));
	env.add_module_function("Touch", "released",     TOUCH(released));
	env.add_module_function("Touch", "any_pressed",  TOUCH(any_pressed));
	env.add_module_function("Touch", "any_released", TOUCH(any_released));
	env.add_module_function("Touch", "button",       TOUCH(button));
	env.add_module_function("Touch", "axis",         TOUCH(axis));
	env.add_module_function("Touch", "button_name",  TOUCH(button_name));
	env.add_module_function("Touch", "axis_name",    TOUCH(axis_name));
	env.add_module_function("Touch", "button_id",    TOUCH(button_id));
	env.add_module_function("Touch", "axis_id",      TOUCH(axis_id));

	env.add_module_function("Pad1", "name",         PAD(0, name));
	env.add_module_function("Pad1", "connected",    PAD(0, connected));
	env.add_module_function("Pad1", "num_buttons",  PAD(0, num_buttons));
	env.add_module_function("Pad1", "num_axes",     PAD(0, num_axes));
	env.add_module_function("Pad1", "pressed",      PAD(0, pressed));
	env.add_module_function("Pad1", "released",     PAD(0, released));
	env.add_module_function("Pad1", "any_pressed",  PAD(0, any_pressed));
	env.add_module_function("Pad1", "any_released", PAD(0, any_released));
	env.add_module_function("Pad1", "button",       PAD(0, button));
	env.add_module_function("Pad1", "axis",         PAD(0, axis));
	env.add_module_function("Pad1", "button_name",  PAD(0, button_name));
	env.add_module_function("Pad1", "axis_name",    PAD(0, axis_name));
	env.add_module_function("Pad1", "button_id",    PAD(0, button_id));
	env.add_module_function("Pad1", "axis_id",      PAD(0, axis_id));
	env.add_module_function("Pad1", "deadzone",     PAD(0, deadzone));
	env.add_module_function("Pad1", "set_deadzone", PAD(0, set_deadzone));

	env.add_module_function("Pad2", "name",         PAD(1, name));
	env.add_module_function("Pad2", "connected",    PAD(1, connected));
	env.add_module_function("Pad2", "num_buttons",  PAD(1, num_buttons));
	env.add_module_function("Pad2", "num_axes",     PAD(1, num_axes));
	env.add_module_function("Pad2", "pressed",      PAD(1, pressed));
	env.add_module_function("Pad2", "released",     PAD(1, released));
	env.add_module_function("Pad2", "any_pressed",  PAD(1, any_pressed));
	env.add_module_function("Pad2", "any_released", PAD(1, any_released));
	env.add_module_function("Pad2", "button",       PAD(1, button));
	env.add_module_function("Pad2", "axis",         PAD(1, axis));
	env.add_module_function("Pad2", "button_name",  PAD(1, button_name));
	env.add_module_function("Pad2", "axis_name",    PAD(1, axis_name));
	env.add_module_function("Pad2", "button_id",    PAD(1, button_id));
	env.add_module_function("Pad2", "axis_id",      PAD(1, axis_id));
	env.add_module_function("Pad2", "deadzone",     PAD(1, deadzone));
	env.add_module_function("Pad2", "set_deadzone", PAD(1, set_deadzone));

	env.add_module_function("Pad3", "name",         PAD(2, name));
	env.add_module_function("Pad3", "connected",    PAD(2, connected));
	env.add_module_function("Pad3", "num_buttons",  PAD(2, num_buttons));
	env.add_module_function("Pad3", "num_axes",     PAD(2, num_axes));
	env.add_module_function("Pad3", "pressed",      PAD(2, pressed));
	env.add_module_function("Pad3", "released",     PAD(2, released));
	env.add_module_function("Pad3", "any_pressed",  PAD(2, any_pressed));
	env.add_module_function("Pad3", "any_released", PAD(2, any_released));
	env.add_module_function("Pad3", "button",       PAD(2, button));
	env.add_module_function("Pad3", "axis",         PAD(2, axis));
	env.add_module_function("Pad3", "button_name",  PAD(2, button_name));
	env.add_module_function("Pad3", "axis_name",    PAD(2, axis_name));
	env.add_module_function("Pad3", "button_id",    PAD(2, button_id));
	env.add_module_function("Pad3", "axis_id",      PAD(2, axis_id));
	env.add_module_function("Pad3", "deadzone",     PAD(2, deadzone));
	env.add_module_function("Pad3", "set_deadzone", PAD(2, set_deadzone));

	env.add_module_function("Pad4", "name",         PAD(3, name));
	env.add_module_function("Pad4", "connected",    PAD(3, connected));
	env.add_module_function("Pad4", "num_buttons",  PAD(3, num_buttons));
	env.add_module_function("Pad4", "num_axes",     PAD(3, num_axes));
	env.add_module_function("Pad4", "pressed",      PAD(3, pressed));
	env.add_module_function("Pad4", "released",     PAD(3, released));
	env.add_module_function("Pad4", "any_pressed",  PAD(3, any_pressed));
	env.add_module_function("Pad4", "any_released", PAD(3, any_released));
	env.add_module_function("Pad4", "button",       PAD(3, button));
	env.add_module_function("Pad4", "axis",         PAD(3, axis));
	env.add_module_function("Pad4", "button_name",  PAD(3, button_name));
	env.add_module_function("Pad4", "axis_name",    PAD(3, axis_name));
	env.add_module_function("Pad4", "button_id",    PAD(3, button_id));
	env.add_module_function("Pad4", "axis_id",      PAD(3, axis_id));
	env.add_module_function("Pad4", "deadzone",     PAD(3, deadzone));
	env.add_module_function("Pad4", "set_deadzone", PAD(3, set_deadzone));

#undef PAD
#undef TOUCH
#undef MOUSE
#undef KEYBOARD

	env.add_module_function("World", "spawn_unit", [](lua_State* L)
		{
			LuaStack stack(L);
			const int nargs = stack.num_args();

			World* world          = stack.get_world(1);
			const StringId64 name = stack.get_resource_name(2);
			const Vector3& pos    = nargs > 2 ? stack.get_vector3(3)    : VECTOR3_ZERO;
			const Quaternion& rot = nargs > 3 ? stack.get_quaternion(4) : QUATERNION_IDENTITY;
			const Vector3& scl    = nargs > 4 ? stack.get_vector3(5)    : VECTOR3_ONE;

			char name_str[RESOURCE_ID_BUF_LEN];
			LUA_ASSERT(device()->_resource_manager->can_get(RESOURCE_TYPE_UNIT, name)
				, stack
				, "Unit not loaded: " RESOURCE_ID_FMT_STR
				, resource_id(RESOURCE_TYPE_UNIT, name).to_string(name_str, sizeof(name_str))
				);
			CE_UNUSED(name_str);

			stack.push_unit(world->spawn_unit(name, pos, rot, scl));
			return 1;
		});
	env.add_module_function("World", "spawn_empty_unit", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_unit(stack.get_world(1)->spawn_empty_unit());
			return 1;
		});
	env.add_module_function("World", "destroy_unit", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->destroy_unit(stack.get_unit(2));
			return 0;
		});
	env.add_module_function("World", "num_units", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_int(stack.get_world(1)->num_units());
			return 1;
		});
	env.add_module_function("World", "units", [](lua_State* L)
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
		});
	env.add_module_function("World", "unit_by_name", [](lua_State* L)
		{
			LuaStack stack(L);
			UnitId unit = stack.get_world(1)->unit_by_name(stack.get_string_id_32(2));

			if (unit == UNIT_INVALID)
				stack.push_nil();
			else
				stack.push_unit(unit);

			return 1;
		});
	env.add_module_function("World", "camera_create", [](lua_State* L)
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
		});
	env.add_module_function("World", "camera_instance", [](lua_State* L)
		{
			LuaStack stack(L);
			CameraInstance inst = stack.get_world(1)->camera_instance(stack.get_unit(2));
			if (is_valid(inst))
				stack.push_camera(inst);
			else
				stack.push_nil();
			return 1;
		});
	env.add_module_function("World", "camera_set_projection_type", [](lua_State* L)
		{
			LuaStack stack(L);

			const char* name = stack.get_string(3);
			const ProjectionType::Enum pt = name_to_projection_type(name);
			LUA_ASSERT(pt != ProjectionType::COUNT, stack, "Unknown projection type: '%s'", name);

			stack.get_world(1)->camera_set_projection_type(stack.get_camera_instance(2), pt);
			return 0;
		});
	env.add_module_function("World", "camera_projection_type", [](lua_State* L)
		{
			LuaStack stack(L);
			ProjectionType::Enum type = stack.get_world(1)->camera_projection_type(stack.get_camera_instance(2));
			stack.push_string(s_projection[type].name);
			return 1;
		});
	env.add_module_function("World", "camera_fov", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(stack.get_world(1)->camera_fov(stack.get_camera_instance(2)));
			return 1;
		});
	env.add_module_function("World", "camera_set_fov", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->camera_set_fov(stack.get_camera_instance(2), stack.get_float(3));
			return 0;
		});
	env.add_module_function("World", "camera_near_clip_distance", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(stack.get_world(1)->camera_near_clip_distance(stack.get_camera_instance(2)));
			return 1;
		});
	env.add_module_function("World", "camera_set_near_clip_distance", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->camera_set_near_clip_distance(stack.get_camera_instance(2), stack.get_float(3));
			return 0;
		});
	env.add_module_function("World", "camera_far_clip_distance", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(stack.get_world(1)->camera_far_clip_distance(stack.get_camera_instance(2)));
			return 1;
		});
	env.add_module_function("World", "camera_set_far_clip_distance", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->camera_set_far_clip_distance(stack.get_camera_instance(2), stack.get_float(3));
			return 0;
		});
	env.add_module_function("World", "camera_set_orthographic_size", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->camera_set_orthographic_size(stack.get_camera_instance(2), stack.get_float(3));
			return 0;
		});
	env.add_module_function("World", "camera_screen_to_world", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(stack.get_world(1)->camera_screen_to_world(stack.get_camera_instance(2), stack.get_vector3(3)));
			return 1;
		});
	env.add_module_function("World", "camera_world_to_screen", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(stack.get_world(1)->camera_world_to_screen(stack.get_camera_instance(2), stack.get_vector3(3)));
			return 1;
		});
	env.add_module_function("World", "update_animations", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->update_animations(stack.get_float(2));
			return 0;
		});
	env.add_module_function("World", "update_scene", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->update_scene(stack.get_float(2));
			return 0;
		});
	env.add_module_function("World", "update", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->update(stack.get_float(2));
			return 0;
		});
	env.add_module_function("World", "play_sound", [](lua_State* L)
		{
			LuaStack stack(L);
			const s32 nargs       = stack.num_args();
			World* world          = stack.get_world(1);
			const StringId64 name = stack.get_resource_name(2);
			const bool loop       = nargs > 2 ? stack.get_bool(3)    : false;
			const f32 volume      = nargs > 3 ? stack.get_float(4)   : 1.0f;
			const Vector3& pos    = nargs > 4 ? stack.get_vector3(5) : VECTOR3_ZERO;
			const f32 range       = nargs > 5 ? stack.get_float(6)   : 1000.0f;

			char name_str[RESOURCE_ID_BUF_LEN];
			LUA_ASSERT(device()->_resource_manager->can_get(RESOURCE_TYPE_SOUND, name)
				, stack
				, "Sound not loaded: " RESOURCE_ID_FMT_STR
				, resource_id(RESOURCE_TYPE_SOUND, name).to_string(name_str, sizeof(name_str))
				);
			CE_UNUSED(name_str);

			stack.push_sound_instance_id(world->play_sound(name, loop, volume, pos, range));
			return 1;
		});
	env.add_module_function("World", "stop_sound", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->stop_sound(stack.get_sound_instance_id(2));
			return 0;
		});
	env.add_module_function("World", "link_sound", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->link_sound(stack.get_sound_instance_id(2)
				, stack.get_unit(3)
				, stack.get_int(4)
				);
			return 0;
		});
	env.add_module_function("World", "set_listener_pose", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->set_listener_pose(stack.get_matrix4x4(2));
			return 0;
		});
	env.add_module_function("World", "set_sound_position", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->set_sound_position(stack.get_sound_instance_id(2), stack.get_vector3(3));
			return 0;
		});
	env.add_module_function("World", "set_sound_range", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->set_sound_range(stack.get_sound_instance_id(2), stack.get_float(3));
			return 0;
		});
	env.add_module_function("World", "set_sound_volume", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->set_sound_volume(stack.get_sound_instance_id(2), stack.get_float(3));
			return 0;
		});
	env.add_module_function("World", "create_debug_line", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_debug_line(stack.get_world(1)->create_debug_line(stack.get_bool(2)));
			return 1;
		});
	env.add_module_function("World", "destroy_debug_line", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->destroy_debug_line(*stack.get_debug_line(2));
			return 0;
		});
	env.add_module_function("World", "create_screen_gui", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_gui(stack.get_world(1)->create_screen_gui());
			return 1;
		});
	env.add_module_function("World", "destroy_gui", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->destroy_gui(*stack.get_gui(2));
			return 0;
		});
	env.add_module_function("World", "load_level", [](lua_State* L)
		{
			LuaStack stack(L);
			const int nargs = stack.num_args();

			World* world          = stack.get_world(1);
			const StringId64 name = stack.get_resource_name(2);
			const Vector3& pos    = nargs > 2 ? stack.get_vector3(3)    : VECTOR3_ZERO;
			const Quaternion& rot = nargs > 3 ? stack.get_quaternion(4) : QUATERNION_IDENTITY;

			char name_str[RESOURCE_ID_BUF_LEN];
			LUA_ASSERT(device()->_resource_manager->can_get(RESOURCE_TYPE_LEVEL, name)
				, stack
				, "Level not loaded: " RESOURCE_ID_FMT_STR
				, resource_id(RESOURCE_TYPE_LEVEL, name).to_string(name_str, sizeof(name_str))
				);
			CE_UNUSED(name_str);

			stack.push_level(world->load_level(name, pos, rot));
			return 1;
		});
	env.add_module_function("World", "scene_graph", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_scene_graph(stack.get_world(1)->_scene_graph);
			return 1;
		});
	env.add_module_function("World", "render_world", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_render_world(stack.get_world(1)->_render_world);
			return 1;
		});
	env.add_module_function("World", "physics_world", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_physics_world(stack.get_world(1)->_physics_world);
			return 1;
		});
	env.add_module_function("World", "sound_world", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_sound_world(stack.get_world(1)->_sound_world);
			return 1;
		});
	env.add_module_function("World", "animation_state_machine", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_animation_state_machine(stack.get_world(1)->_animation_state_machine);
			return 1;
		});
	env.add_module_function("World", "disable_unit_callbacks", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_world(1)->disable_unit_callbacks();
			return 0;
		});
	env.add_module_metafunction("World", "__tostring", [](lua_State* L)
		{
			LuaStack stack(L);
			World* w = stack.get_world(1);
			stack.push_fstring("World (%p)", w);
			return 1;
		});

	env.add_module_function("SceneGraph", "create", [](lua_State* L)
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
		});
	env.add_module_function("SceneGraph", "destroy", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_scene_graph(1)->destroy(stack.get_transform_instance(2));
			return 0;
		});
	env.add_module_function("SceneGraph", "instance", [](lua_State* L)
		{
			LuaStack stack(L);
			TransformInstance inst = stack.get_scene_graph(1)->instance(stack.get_unit(2));
			if (is_valid(inst))
				stack.push_transform(inst);
			else
				stack.push_nil();
			return 1;
		});
	env.add_module_function("SceneGraph", "local_position", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(stack.get_scene_graph(1)->local_position(stack.get_transform_instance(2)));
			return 1;
		});
	env.add_module_function("SceneGraph", "local_rotation", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_quaternion(stack.get_scene_graph(1)->local_rotation(stack.get_transform_instance(2)));
			return 1;
		});
	env.add_module_function("SceneGraph", "local_scale", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(stack.get_scene_graph(1)->local_scale(stack.get_transform_instance(2)));
			return 1;
		});
	env.add_module_function("SceneGraph", "local_pose", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_matrix4x4(stack.get_scene_graph(1)->local_pose(stack.get_transform_instance(2)));
			return 1;
		});
	env.add_module_function("SceneGraph", "world_position", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(stack.get_scene_graph(1)->world_position(stack.get_transform_instance(2)));
			return 1;
		});
	env.add_module_function("SceneGraph", "world_rotation", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_quaternion(stack.get_scene_graph(1)->world_rotation(stack.get_transform_instance(2)));
			return 1;
		});
	env.add_module_function("SceneGraph", "world_pose", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_matrix4x4(stack.get_scene_graph(1)->world_pose(stack.get_transform_instance(2)));
			return 1;
		});
	env.add_module_function("SceneGraph", "set_local_position", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_scene_graph(1)->set_local_position(stack.get_transform_instance(2), stack.get_vector3(3));
			return 0;
		});
	env.add_module_function("SceneGraph", "set_local_rotation", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_scene_graph(1)->set_local_rotation(stack.get_transform_instance(2), stack.get_quaternion(3));
			return 0;
		});
	env.add_module_function("SceneGraph", "set_local_scale", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_scene_graph(1)->set_local_scale(stack.get_transform_instance(2), stack.get_vector3(3));
			return 0;
		});
	env.add_module_function("SceneGraph", "set_local_pose", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_scene_graph(1)->set_local_pose(stack.get_transform_instance(2), stack.get_matrix4x4(3));
			return 0;
		});
	env.add_module_function("SceneGraph", "link", [](lua_State* L)
		{
			LuaStack stack(L);
			const int nargs = stack.num_args();

			const TransformInstance parent_ti = stack.get_transform_instance(2);
			const TransformInstance child_ti  = stack.get_transform_instance(3);
			const Vector3& pos                = nargs > 3 ? stack.get_vector3(4)    : VECTOR3_ZERO;
			const Quaternion& rot             = nargs > 4 ? stack.get_quaternion(5) : QUATERNION_IDENTITY;
			const Vector3& scl                = nargs > 5 ? stack.get_vector3(6)    : VECTOR3_ONE;

			stack.get_scene_graph(1)->link(parent_ti, child_ti, pos, rot, scl);
			return 0;
		});
	env.add_module_function("SceneGraph", "unlink", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_scene_graph(1)->unlink(stack.get_transform_instance(2));
			return 0;
		});

	env.add_module_function("UnitManager", "create", [](lua_State* L)
		{
			LuaStack stack(L);

			if (stack.num_args() == 1)
				stack.push_unit(device()->_unit_manager->create(*stack.get_world(1)));
			else
				stack.push_unit(device()->_unit_manager->create());

			return 1;
		});
	env.add_module_function("UnitManager", "alive", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_bool(device()->_unit_manager->alive(stack.get_unit(1)));
			return 1;
		});

	env.add_module_function("RenderWorld", "mesh_create", [](lua_State* L)
		{
			LuaStack stack(L);
			RenderWorld* rw = stack.get_render_world(1);
			UnitId unit = stack.get_unit(2);

			MeshRendererDesc desc;
			desc.mesh_resource     = stack.get_resource_name(3);
			desc.geometry_name     = stack.get_string_id_32(4);
			desc.material_resource = stack.get_resource_name(5);
			desc.visible           = stack.get_bool(6);

			Matrix4x4 pose = stack.get_matrix4x4(7);

			stack.push_mesh_instance(rw->mesh_create(unit, desc, pose));
			return 1;
		});
	env.add_module_function("RenderWorld", "mesh_destroy", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->mesh_destroy(stack.get_mesh_instance(2));
			return 0;
		});
	env.add_module_function("RenderWorld", "mesh_instance", [](lua_State* L)
		{
			LuaStack stack(L);
			MeshInstance inst = stack.get_render_world(1)->mesh_instance(stack.get_unit(2));
			if (is_valid(inst))
				stack.push_mesh_instance(inst);
			else
				stack.push_nil();

			return 1;
		});
	env.add_module_function("RenderWorld", "mesh_obb", [](lua_State* L)
		{
			LuaStack stack(L);
			OBB obb = stack.get_render_world(1)->mesh_obb(stack.get_mesh_instance(2));
			stack.push_matrix4x4(obb.tm);
			stack.push_vector3(obb.half_extents);
			return 2;
		});
	env.add_module_function("RenderWorld", "mesh_cast_ray", [](lua_State* L)
		{
			LuaStack stack(L);
			RenderWorld* rw = stack.get_render_world(1);
			float t = rw->mesh_cast_ray(stack.get_mesh_instance(2)
				, stack.get_vector3(3)
				, stack.get_vector3(4)
				);
			stack.push_float(t);
			return 1;
		});
	env.add_module_function("RenderWorld", "mesh_material", [](lua_State* L)
		{
			LuaStack stack(L);
			Material* material = stack.get_render_world(1)->mesh_material(stack.get_mesh_instance(2));
			stack.push_pointer(material);
			return 1;
		});
	env.add_module_function("RenderWorld", "mesh_set_material", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->mesh_set_material(stack.get_mesh_instance(2), stack.get_string_id_64(3));
			return 0;
		});
	env.add_module_function("RenderWorld", "mesh_set_visible", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->mesh_set_visible(stack.get_mesh_instance(2), stack.get_bool(3));
			return 0;
		});
	env.add_module_function("RenderWorld", "sprite_create", [](lua_State* L)
		{
			LuaStack stack(L);
			RenderWorld* rw = stack.get_render_world(1);
			UnitId unit = stack.get_unit(2);

			SpriteRendererDesc desc;
			desc.sprite_resource = stack.get_resource_name(3);
			desc.material_resource = stack.get_resource_name(4);
			desc.layer = stack.get_int(5);
			desc.depth = stack.get_int(6);
			desc.visible = stack.get_bool(7);

			Matrix4x4 pose = stack.get_matrix4x4(8);

			stack.push_sprite_instance(rw->sprite_create(unit, desc, pose));
			return 1;
		});
	env.add_module_function("RenderWorld", "sprite_destroy", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->sprite_destroy(stack.get_sprite_instance(2));
			return 0;
		});
	env.add_module_function("RenderWorld", "sprite_instance", [](lua_State* L)
		{
			LuaStack stack(L);
			SpriteInstance inst = stack.get_render_world(1)->sprite_instance(stack.get_unit(2));
			if (is_valid(inst))
				stack.push_sprite_instance(inst);
			else
				stack.push_nil();
			return 1;
		});
	env.add_module_function("RenderWorld", "sprite_material", [](lua_State* L)
		{
			LuaStack stack(L);
			Material* material = stack.get_render_world(1)->sprite_material(stack.get_sprite_instance(2));
			stack.push_pointer(material);
			return 1;
		});
	env.add_module_function("RenderWorld", "sprite_set_material", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->sprite_set_material(stack.get_sprite_instance(2), stack.get_string_id_64(3));
			return 0;
		});
	env.add_module_function("RenderWorld", "sprite_set_frame", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->sprite_set_frame(stack.get_sprite_instance(2), stack.get_int(3));
			return 0;
		});
	env.add_module_function("RenderWorld", "sprite_set_visible", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->sprite_set_visible(stack.get_sprite_instance(2), stack.get_bool(3));
			return 0;
		});
	env.add_module_function("RenderWorld", "sprite_flip_x", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->sprite_flip_x(stack.get_sprite_instance(2), stack.get_bool(3));
			return 0;
		});
	env.add_module_function("RenderWorld", "sprite_flip_y", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->sprite_flip_y(stack.get_sprite_instance(2), stack.get_bool(3));
			return 0;
		});
	env.add_module_function("RenderWorld", "sprite_set_layer", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->sprite_set_layer(stack.get_sprite_instance(2), stack.get_int(3));
			return 0;
		});
	env.add_module_function("RenderWorld", "sprite_set_depth", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->sprite_set_depth(stack.get_sprite_instance(2), stack.get_int(3));
			return 0;
		});
	env.add_module_function("RenderWorld", "sprite_obb", [](lua_State* L)
		{
			LuaStack stack(L);
			OBB obb = stack.get_render_world(1)->sprite_obb(stack.get_sprite_instance(2));
			stack.push_matrix4x4(obb.tm);
			stack.push_vector3(obb.half_extents);
			return 2;
		});
	env.add_module_function("RenderWorld", "sprite_cast_ray", [](lua_State* L)
		{
			LuaStack stack(L);
			RenderWorld* rw = stack.get_render_world(1);
			u32 layer;
			u32 depth;
			float t = rw->sprite_cast_ray(stack.get_sprite_instance(2)
				, stack.get_vector3(3)
				, stack.get_vector3(4)
				, layer
				, depth
				);
			stack.push_float(t);
			stack.push_int(layer);
			stack.push_int(depth);
			return 3;
		});
	env.add_module_function("RenderWorld", "light_create", [](lua_State* L)
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
		});
	env.add_module_function("RenderWorld", "light_destroy", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->light_destroy(stack.get_light_instance(2));
			return 0;
		});
	env.add_module_function("RenderWorld", "light_instance", [](lua_State* L)
		{
			LuaStack stack(L);
			LightInstance inst = stack.get_render_world(1)->light_instance(stack.get_unit(2));
			if (is_valid(inst))
				stack.push_light_instance(inst);
			else
				stack.push_nil();
			return 1;
		});
	env.add_module_function("RenderWorld", "light_type", [](lua_State* L)
		{
			LuaStack stack(L);
			LightType::Enum type = stack.get_render_world(1)->light_type(stack.get_light_instance(2));
			stack.push_string(s_light[type].name);
			return 1;
		});
	env.add_module_function("RenderWorld", "light_color", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_color4(stack.get_render_world(1)->light_color(stack.get_light_instance(2)));
			return 1;
		});
	env.add_module_function("RenderWorld", "light_range", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(stack.get_render_world(1)->light_range(stack.get_light_instance(2)));
			return 1;
		});
	env.add_module_function("RenderWorld", "light_intensity", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(stack.get_render_world(1)->light_intensity(stack.get_light_instance(2)));
			return 1;
		});
	env.add_module_function("RenderWorld", "light_spot_angle", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(stack.get_render_world(1)->light_spot_angle(stack.get_light_instance(2)));
			return 1;
		});
	env.add_module_function("RenderWorld", "light_set_type", [](lua_State* L)
		{
			LuaStack stack(L);

			const char* name = stack.get_string(3);
			const LightType::Enum lt = name_to_light_type(name);
			LUA_ASSERT(lt != LightType::COUNT, stack, "Unknown light type: '%s'", name);

			stack.get_render_world(1)->light_set_type(stack.get_light_instance(2), lt);
			return 0;
		});
	env.add_module_function("RenderWorld", "light_set_color", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->light_set_color(stack.get_light_instance(2), stack.get_color4(3));
			return 0;
		});
	env.add_module_function("RenderWorld", "light_set_range", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->light_set_range(stack.get_light_instance(2), stack.get_float(3));
			return 0;
		});
	env.add_module_function("RenderWorld", "light_set_intensity", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->light_set_intensity(stack.get_light_instance(2), stack.get_float(3));
			return 0;
		});
	env.add_module_function("RenderWorld", "light_set_spot_angle", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->light_set_spot_angle(stack.get_light_instance(2), stack.get_float(3));
			return 0;
		});
	env.add_module_function("RenderWorld", "light_debug_draw", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->light_debug_draw(stack.get_light_instance(2), *stack.get_debug_line(3));
			return 0;
		});
	env.add_module_function("RenderWorld", "enable_debug_drawing", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_render_world(1)->enable_debug_drawing(stack.get_bool(2));
			return 0;
		});

	env.add_module_function("PhysicsWorld", "actor_instance", [](lua_State* L)
		{
			LuaStack stack(L);
			ActorInstance inst = stack.get_physics_world(1)->actor(stack.get_unit(2));
			if (is_valid(inst))
				stack.push_actor(inst);
			else
				stack.push_nil();
			return 1;
		});
	env.add_module_function("PhysicsWorld", "actor_destroy", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_destroy(stack.get_actor_instance(2));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_world_position", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(stack.get_physics_world(1)->actor_world_position(stack.get_actor_instance(2)));
			return 1;
		});
	env.add_module_function("PhysicsWorld", "actor_world_rotation", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_quaternion(stack.get_physics_world(1)->actor_world_rotation(stack.get_actor_instance(2)));
			return 1;
		});
	env.add_module_function("PhysicsWorld", "actor_world_pose", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_matrix4x4(stack.get_physics_world(1)->actor_world_pose(stack.get_actor_instance(2)));
			return 1;
		});
	env.add_module_function("PhysicsWorld", "actor_teleport_world_position", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_teleport_world_position(stack.get_actor_instance(2), stack.get_vector3(3));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_teleport_world_rotation", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_teleport_world_rotation(stack.get_actor_instance(2), stack.get_quaternion(3));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_teleport_world_pose", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_teleport_world_pose(stack.get_actor_instance(2), stack.get_matrix4x4(3));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_center_of_mass", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(stack.get_physics_world(1)->actor_center_of_mass(stack.get_actor_instance(2)));
			return 1;
		});
	env.add_module_function("PhysicsWorld", "actor_enable_gravity", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_enable_gravity(stack.get_actor_instance(2));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_disable_gravity", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_disable_gravity(stack.get_actor_instance(2));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_enable_collision", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_enable_collision(stack.get_actor_instance(2));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_disable_collision", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_disable_collision(stack.get_actor_instance(2));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_set_collision_filter", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_set_collision_filter(stack.get_actor_instance(2), stack.get_string_id_32(3));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_set_kinematic", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_set_kinematic(stack.get_actor_instance(2), stack.get_bool(3));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_is_static", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_bool(stack.get_physics_world(1)->actor_is_static(stack.get_actor_instance(2)));
			return 1;
		});
	env.add_module_function("PhysicsWorld", "actor_is_dynamic", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_bool(stack.get_physics_world(1)->actor_is_dynamic(stack.get_actor_instance(2)));
			return 1;
		});
	env.add_module_function("PhysicsWorld", "actor_is_kinematic", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_bool(stack.get_physics_world(1)->actor_is_kinematic(stack.get_actor_instance(2)));
			return 1;
		});
	env.add_module_function("PhysicsWorld", "actor_is_nonkinematic", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_bool(stack.get_physics_world(1)->actor_is_nonkinematic(stack.get_actor_instance(2)));
			return 1;
		});
	env.add_module_function("PhysicsWorld", "actor_linear_damping", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(stack.get_physics_world(1)->actor_linear_damping(stack.get_actor_instance(2)));
			return 1;
		});
	env.add_module_function("PhysicsWorld", "actor_set_linear_damping", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_set_linear_damping(stack.get_actor_instance(2), stack.get_float(3));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_angular_damping", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_float(stack.get_physics_world(1)->actor_angular_damping(stack.get_actor_instance(2)));
			return 1;
		});
	env.add_module_function("PhysicsWorld", "actor_set_angular_damping", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_set_angular_damping(stack.get_actor_instance(2), stack.get_float(3));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_linear_velocity", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(stack.get_physics_world(1)->actor_linear_velocity(stack.get_actor_instance(2)));
			return 1;
		});
	env.add_module_function("PhysicsWorld", "actor_set_linear_velocity", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_set_linear_velocity(stack.get_actor_instance(2), stack.get_vector3(3));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_angular_velocity", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(stack.get_physics_world(1)->actor_angular_velocity(stack.get_actor_instance(2)));
			return 1;
		});
	env.add_module_function("PhysicsWorld", "actor_set_angular_velocity", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_set_angular_velocity(stack.get_actor_instance(2), stack.get_vector3(3));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_add_impulse", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_add_impulse(stack.get_actor_instance(2), stack.get_vector3(3));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_add_impulse_at", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_add_impulse_at(stack.get_actor_instance(2), stack.get_vector3(3), stack.get_vector3(4));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_add_actor_torque_impulse", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_add_torque_impulse(stack.get_actor_instance(2), stack.get_vector3(3));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_push", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_push(stack.get_actor_instance(2), stack.get_vector3(3), stack.get_float(4));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_push_at", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_push_at(stack.get_actor_instance(2), stack.get_vector3(3), stack.get_float(4), stack.get_vector3(5));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "actor_is_sleeping", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_bool(stack.get_physics_world(1)->actor_is_sleeping(stack.get_actor_instance(2)));
			return 1;
		});
	env.add_module_function("PhysicsWorld", "actor_wake_up", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->actor_wake_up(stack.get_actor_instance(2));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "joint_create", [](lua_State* L)
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
			stack.get_physics_world(1)->joint_create(stack.get_actor_instance(2), stack.get_actor_instance(3), jd);
			return 0;
		});
	env.add_module_function("PhysicsWorld", "gravity", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_vector3(stack.get_physics_world(1)->gravity());
			return 1;
		});
	env.add_module_function("PhysicsWorld", "set_gravity", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->set_gravity(stack.get_vector3(2));
			return 0;
		});
	env.add_module_function("PhysicsWorld", "cast_ray", [](lua_State* L)
		{
			LuaStack stack(L);

			RaycastHit hit;
			if (stack.get_physics_world(1)->cast_ray(hit
				, stack.get_vector3(2)
				, stack.get_vector3(3)
				, stack.get_float(4)
				))
			{
				stack.push_bool(true);
				stack.push_vector3(hit.position);
				stack.push_vector3(hit.normal);
				stack.push_float(hit.time);
				stack.push_unit(hit.unit);
				stack.push_actor(hit.actor);
				return 6;
			}

			stack.push_bool(false);
			return 1;
		});
	env.add_module_function("PhysicsWorld", "cast_ray_all", [](lua_State* L)
		{
			LuaStack stack(L);

			TempAllocator1024 ta;
			Array<RaycastHit> hits(ta);
			if (stack.get_physics_world(1)->cast_ray_all(hits
				, stack.get_vector3(2)
				, stack.get_vector3(3)
				, stack.get_float(4)
				))
			{
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
						stack.push_float(hits[i].time);
						stack.push_key_end();

						stack.push_key_begin(4);
						stack.push_unit(hits[i].unit);
						stack.push_key_end();

						stack.push_key_begin(5);
						stack.push_actor(hits[i].actor);
						stack.push_key_end();
					}
					stack.push_key_end();
				}
			}

			return 1;
		});
	env.add_module_function("PhysicsWorld", "cast_sphere", [](lua_State* L)
		{
			LuaStack stack(L);

			RaycastHit hit;
			if (stack.get_physics_world(1)->cast_sphere(hit
				, stack.get_vector3(2)
				, stack.get_float(3)
				, stack.get_vector3(4)
				, stack.get_float(5)
				))
			{
				stack.push_bool(true);
				stack.push_vector3(hit.position);
				stack.push_vector3(hit.normal);
				stack.push_float(hit.time);
				stack.push_unit(hit.unit);
				stack.push_actor(hit.actor);
				return 6;
			}

			stack.push_bool(false);
			return 1;
		});
	env.add_module_function("PhysicsWorld", "cast_box", [](lua_State* L)
		{
			LuaStack stack(L);

			RaycastHit hit;
			if (stack.get_physics_world(1)->cast_box(hit
				, stack.get_vector3(2)
				, stack.get_vector3(3)
				, stack.get_vector3(4)
				, stack.get_float(5)
				))
			{
				stack.push_bool(true);
				stack.push_vector3(hit.position);
				stack.push_vector3(hit.normal);
				stack.push_float(hit.time);
				stack.push_unit(hit.unit);
				stack.push_actor(hit.actor);
				return 6;
			}

			stack.push_bool(false);
			return 1;
		});
	env.add_module_function("PhysicsWorld", "enable_debug_drawing", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_physics_world(1)->enable_debug_drawing(stack.get_bool(2));
			return 0;
		});
	env.add_module_metafunction("PhysicsWorld", "__tostring", [](lua_State* L)
		{
			LuaStack stack(L);
			PhysicsWorld* pw = stack.get_physics_world(1);
			stack.push_fstring("PhysicsWorld (%p)", pw);
			return 1;
		});

	env.add_module_function("SoundWorld", "stop_all", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_sound_world(1)->stop_all();
			return 0;
		});
	env.add_module_function("SoundWorld", "pause_all", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_sound_world(1)->pause_all();
			return 0;
		});
	env.add_module_function("SoundWorld", "resume_all", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_sound_world(1)->resume_all();
			return 0;
		});
	env.add_module_function("SoundWorld", "is_playing", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_bool(stack.get_sound_world(1)->is_playing(stack.get_sound_instance_id(2)));
			return 1;
		});
	env.add_module_metafunction("SoundWorld", "__tostring", [](lua_State* L)
		{
			LuaStack stack(L);
			SoundWorld* sw = stack.get_sound_world(1);
			stack.push_fstring("SoundWorld (%p)", sw);
			return 1;
		});

	env.add_module_function("AnimationStateMachine", "instance", [](lua_State* L)
		{
			LuaStack stack(L);
			StateMachineInstance inst = stack.get_animation_state_machine(1)->instance(stack.get_unit(2));
			if (is_valid(inst))
				stack.push_state_machine_instance(inst);
			else
				stack.push_nil();
			return 1;
		});
	env.add_module_function("AnimationStateMachine", "trigger", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_animation_state_machine(1)->trigger(stack.get_state_machine_instance(2)
				, stack.get_string_id_32(3)
				);
			return 0;
		});
	env.add_module_function("AnimationStateMachine", "variable_id", [](lua_State* L)
		{
			LuaStack stack(L);
			u32 variable_id = stack.get_animation_state_machine(1)->variable_id(stack.get_state_machine_instance(2)
				, stack.get_string_id_32(3)
				);
			LUA_ASSERT(variable_id != UINT32_MAX, stack, "Variable does not exist");
			stack.push_id(variable_id);
			return 1;
		});
	env.add_module_function("AnimationStateMachine", "variable", [](lua_State* L)
		{
			LuaStack stack(L);
			const float v = stack.get_animation_state_machine(1)->variable(stack.get_state_machine_instance(2)
				, stack.get_id(3)
				);
			stack.push_float(v);
			return 1;
		});
	env.add_module_function("AnimationStateMachine", "set_variable", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_animation_state_machine(1)->set_variable(stack.get_state_machine_instance(2)
				, stack.get_id(3)
				, stack.get_float(4)
				);
			return 0;
		});

	env.add_module_function("Device", "argv", [](lua_State* L)
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
		});
	env.add_module_function("Device", "platform", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_string(CROWN_PLATFORM_NAME);
			return 1;
		});
	env.add_module_function("Device", "architecture", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_string(CROWN_ARCH_NAME);
			return 1;
		});
	env.add_module_function("Device", "version", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_string(CROWN_VERSION);
			return 1;
		});
	env.add_module_function("Device", "quit", [](lua_State* /*L*/)
		{
			device()->quit();
			return 0;
		});
	env.add_module_function("Device", "resolution", [](lua_State* L)
		{
			LuaStack stack(L);
			u16 w, h;
			device()->resolution(w, h);
			stack.push_int(w);
			stack.push_int(h);
			return 2;
		});
	env.add_module_function("Device", "create_world", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_world(device()->create_world());
			return 1;
		});
	env.add_module_function("Device", "destroy_world", [](lua_State* L)
		{
			LuaStack stack(L);
			device()->destroy_world(*stack.get_world(1));
			return 0;
		});
	env.add_module_function("Device", "render", [](lua_State* L)
		{
			LuaStack stack(L);
			device()->render(*stack.get_world(1), stack.get_unit(2));
			return 0;
		});
	env.add_module_function("Device", "create_resource_package", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_resource_package(device()->create_resource_package(stack.get_resource_name(1)));
			return 1;
		});
	env.add_module_function("Device", "destroy_resource_package", [](lua_State* L)
		{
			LuaStack stack(L);
			device()->destroy_resource_package(*stack.get_resource_package(1));
			return 0;
		});
	env.add_module_function("Device", "console_send", [](lua_State* L)
		{
			LuaStack stack(L);
			LUA_ASSERT(stack.is_table(1), stack, "Table expected");

			TempAllocator1024 alloc;
			StringStream json(alloc);
			lua_dump_table(L, 1, json);

			console_server()->broadcast(string_stream::c_str(json));
			return 0;
		});
	env.add_module_function("Device", "can_get", [](lua_State* L)
		{
			LuaStack stack(L);
			const StringId64 type(stack.get_string(1));
			stack.push_bool(device()->_resource_manager->can_get(type, stack.get_resource_name(2)));
			return 1;
		});
	env.add_module_function("Device", "enable_resource_autoload", [](lua_State* L)
		{
			LuaStack stack(L);
			device()->_resource_manager->enable_autoload(stack.get_bool(1));
			return 0;
		});
	env.add_module_function("Device", "temp_count", [](lua_State* L)
		{
			LuaStack stack(L);
			u32 nv, nq, nm;
			device()->_lua_environment->temp_count(nv, nq, nm);
			stack.push_int(nv);
			stack.push_int(nq);
			stack.push_int(nm);
			return 3;
		});
	env.add_module_function("Device", "set_temp_count", [](lua_State* L)
		{
			LuaStack stack(L);
			u32 nv = stack.get_int(1);
			u32 nq = stack.get_int(2);
			u32 nm = stack.get_int(3);
			device()->_lua_environment->set_temp_count(nv, nq, nm);
			return 0;
		});
	env.add_module_function("Device", "guid", [](lua_State* L)
		{
			LuaStack stack(L);
			Guid guid = guid::new_guid();
			char buf[37];
			guid::to_string(buf, sizeof(buf), guid);
			stack.push_string(buf);
			return 1;
		});

	env.add_module_function("Profiler", "enter_scope", [](lua_State* L)
		{
			LuaStack stack(L);
			profiler::enter_profile_scope(stack.get_string(1));
			return 0;
		});
	env.add_module_function("Profiler", "leave_scope", [](lua_State* L)
		{
			LuaStack stack(L);
			profiler::leave_profile_scope();
			return 0;
		});
	env.add_module_function("Profiler", "record", [](lua_State* L)
		{
			LuaStack stack(L);

			const char* name = stack.get_string(1);

			if (stack.is_number(2))
				profiler::record_float(name, stack.get_float(2));
			else
				profiler::record_vector3(name, stack.get_vector3(2));

			return 0;
		});

	env.add_module_function("DebugLine", "add_line", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_debug_line(1)->add_line(stack.get_vector3(2)
				, stack.get_vector3(3)
				, stack.get_color4(4)
				);
			return 0;
		});
	env.add_module_function("DebugLine", "add_axes", [](lua_State* L)
		{
			LuaStack stack(L);
			const f32 len = stack.num_args() == 3
				? stack.get_float(3)
				: 1.0f
				;
			stack.get_debug_line(1)->add_axes(stack.get_matrix4x4(2), len);
			return 0;
		});
	env.add_module_function("DebugLine", "add_arc", [](lua_State* L)
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
		});
	env.add_module_function("DebugLine", "add_circle", [](lua_State* L)
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
		});
	env.add_module_function("DebugLine", "add_cone", [](lua_State* L)
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
		});
	env.add_module_function("DebugLine", "add_sphere", [](lua_State* L)
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
		});
	env.add_module_function("DebugLine", "add_obb", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_debug_line(1)->add_obb(stack.get_matrix4x4(2)
				, stack.get_vector3(3)
				, stack.get_color4(4)
				);
			return 0;
		});
	env.add_module_function("DebugLine", "add_frustum", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_debug_line(1)->add_frustum(stack.get_matrix4x4(2)
				, stack.get_color4(3)
				);
			return 0;
		});
	env.add_module_function("DebugLine", "add_unit", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_debug_line(1)->add_unit(*device()->_resource_manager
				, stack.get_matrix4x4(2)
				, stack.get_resource_name(3)
				, stack.get_color4(4)
				);
			return 0;
		});
	env.add_module_function("DebugLine", "reset", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_debug_line(1)->reset();
			return 0;
		});
	env.add_module_function("DebugLine", "submit", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_debug_line(1)->submit();
			return 0;
		});
	env.add_module_metafunction("DebugLine", "__tostring", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_fstring("DebugLine (%p)", stack.get_debug_line(1));
			return 1;
		});

	env.add_module_function("ResourcePackage", "load", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_resource_package(1)->load();
			return 0;
		});
	env.add_module_function("ResourcePackage", "unload", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_resource_package(1)->unload();
			return 0;
		});
	env.add_module_function("ResourcePackage", "flush", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_resource_package(1)->flush();
			return 0;
		});
	env.add_module_function("ResourcePackage", "has_loaded", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_bool(stack.get_resource_package(1)->has_loaded());
			return 1;
		});
	env.add_module_metafunction("ResourcePackage", "__tostring", [](lua_State* L)
		{
			LuaStack stack(L);
			ResourcePackage* package = stack.get_resource_package(1);
			stack.push_fstring("ResourcePackage (%p)", package);
			return 1;
		});

	env.add_module_function("Material", "set_float", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_material(1)->set_float(stack.get_string_id_32(2), stack.get_float(3));
			return 0;
		});
	env.add_module_function("Material", "set_vector2", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_material(1)->set_vector2(stack.get_string_id_32(2), stack.get_vector2(3));
			return 0;
		});
	env.add_module_function("Material", "set_vector3", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_material(1)->set_vector3(stack.get_string_id_32(2), stack.get_vector3(3));
			return 0;
		});
	env.add_module_function("Material", "set_vector4", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_material(1)->set_vector4(stack.get_string_id_32(2), stack.get_vector4(3));
			return 0;
		});
	env.add_module_function("Material", "set_matrix4x4", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_material(1)->set_matrix4x4(stack.get_string_id_32(2), stack.get_matrix4x4(3));
			return 0;
		});

	env.add_module_function("Gui", "move", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_gui(1)->move(stack.get_vector2(2));
			return 0;
		});
	env.add_module_function("Gui", "triangle", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_gui(1)->triangle(stack.get_vector2(2)
				, stack.get_vector2(3)
				, stack.get_vector2(4)
				, stack.get_color4(5)
				);
			return 0;
		});
	env.add_module_function("Gui", "rect", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_gui(1)->rect(stack.get_vector2(2)
				, stack.get_vector2(3)
				, stack.get_color4(4)
				);
			return 0;
		});
	env.add_module_function("Gui", "image", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_gui(1)->image(stack.get_vector2(2)
				, stack.get_vector2(3)
				, stack.get_resource_name(4)
				, stack.get_color4(5)
				);
			return 0;
		});
	env.add_module_function("Gui", "image_uv", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_gui(1)->image_uv(stack.get_vector2(2)
				, stack.get_vector2(3)
				, stack.get_vector2(4)
				, stack.get_vector2(5)
				, stack.get_resource_name(6)
				, stack.get_color4(7)
				);
			return 0;
		});
	env.add_module_function("Gui", "text", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.get_gui(1)->text(stack.get_vector2(2)
				, stack.get_int(3)
				, stack.get_string(4)
				, stack.get_resource_name(5)
				, stack.get_resource_name(6)
				, stack.get_color4(7)
				);
			return 0;
		});

	env.add_module_function("Display", "modes", [](lua_State* L)
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
		});
	env.add_module_function("Display", "set_mode", [](lua_State* L)
		{
			LuaStack stack(L);
			device()->_display->set_mode(stack.get_int(1));
			return 0;
		});

	env.add_module_function("Window", "show", [](lua_State* L)
		{
			LuaStack stack(L);
			device()->_window->show();
			return 0;
		});
	env.add_module_function("Window", "hide", [](lua_State* L)
		{
			LuaStack stack(L);
			device()->_window->hide();
			return 0;
		});
	env.add_module_function("Window", "resize", [](lua_State* L)
		{
			LuaStack stack(L);
			device()->_window->resize(stack.get_int(1), stack.get_int(2));
			return 0;
		});
	env.add_module_function("Window", "move", [](lua_State* L)
		{
			LuaStack stack(L);
			device()->_window->move(stack.get_int(1), stack.get_int(2));
			return 0;
		});
	env.add_module_function("Window", "minimize", [](lua_State* /*L*/)
		{
			device()->_window->minimize();
			return 0;
		});
	env.add_module_function("Window", "maximize", [](lua_State* /*L*/)
		{
			device()->_window->maximize();
			return 0;
		});
	env.add_module_function("Window", "restore", [](lua_State* /*L*/)
		{
			device()->_window->restore();
			return 0;
		});
	env.add_module_function("Window", "title", [](lua_State* L)
		{
			LuaStack stack(L);
			stack.push_string(device()->_window->title());
			return 1;
		});
	env.add_module_function("Window", "set_title", [](lua_State* L)
		{
			LuaStack stack(L);
			device()->_window->set_title(stack.get_string(1));
			return 0;
		});
	env.add_module_function("Window", "show_cursor", [](lua_State* L)
		{
			LuaStack stack(L);
			device()->_window->show_cursor(stack.get_bool(1));
			return 0;
		});
	env.add_module_function("Window", "set_fullscreen", [](lua_State* L)
		{
			LuaStack stack(L);
			device()->_window->set_fullscreen(stack.get_bool(1));
			return 0;
		});
	env.add_module_function("Window", "set_cursor", [](lua_State* L)
		{
			LuaStack stack(L);
			const char* name = stack.get_string(1);
			const MouseCursor::Enum mc = name_to_mouse_cursor(name);
			LUA_ASSERT(mc != MouseCursor::COUNT, stack, "Unknown mouse cursor: '%s'", name);
			device()->_window->set_cursor(mc);
			return 0;
		});
	env.add_module_function("Window", "set_cursor_mode", [](lua_State* L)
			{
				LuaStack stack(L);
				const char* name = stack.get_string(1);
				const CursorMode::Enum cm = name_to_cursor_mode(name);
				LUA_ASSERT(cm != CursorMode::COUNT, stack, "Unknown cursor mode: '%s'", name);
				device()->_window->set_cursor_mode(cm);
				return 0;
			});
}

} // namespace crown
