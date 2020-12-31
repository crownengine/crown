/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "tool_api.h"
#include "core/guid.h"
#include "core/math/math.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/string_stream.inl"
#include "core/strings/dynamic_string.inl"
#include "world/types.h"

namespace crown
{
namespace tool
{
const char* lua_bool(const bool b)
{
	return b ? "true" : "false";
}

void lua_vector2(StringStream& out, const Vector2& v)
{
	out << "Vector2(";
	out << v.x << ",";
	out << v.y << ")";
}

void lua_vector3(StringStream& out, const Vector3& v)
{
	out << "Vector3(";
	out << v.x << ",";
	out << v.y << ",";
	out << v.z << ")";
}

void lua_quaternion(StringStream& out, const Quaternion& q)
{
	out << "Quaternion.from_elements(";
	out << q.x << ",";
	out << q.y << ",";
	out << q.z << ",";
	out << q.w << ",";
}

void device_quit(StringStream& out)
{
	out << "Device.quit()";
}

void set_mouse_state(StringStream& out, f32 x, f32 y, bool left, bool middle, bool right)
{
	out << "LevelEditor:set_mouse_state(";
	out << x << ",";
	out << y << ",";
	out << lua_bool(left) << ",";
	out << lua_bool(middle) << ",";
	out << lua_bool(right) << ")";
}

void mouse_down(StringStream& out, f32 x, f32 y)
{
	out << "LevelEditor:mouse_down(";
	out << x << ",";
	out << y << ")";
}

void mouse_up(StringStream& out, f32 x, f32 y)
{
	out << "LevelEditor:mouse_up(";
	out << x << ",";
	out << y << ")";
}

void mouse_wheel(StringStream& out, f32 delta)
{
	out << "LevelEditor:mouse_wheel(";
	out << delta << ")";
}

void key_down(StringStream& out, const char* key)
{
	out << "LevelEditor:key_down(";
	out << "'" << key << "'" << ")";
}

void key_up(StringStream& out, const char* key)
{
	out << "LevelEditor:key_up(";
	out << "'" << key << "'" << ")";
}

void set_grid_size(StringStream& out, f32 size)
{
	out << "LevelEditor:set_grid_size(";
	out << size << ")";
}

void set_rotation_snap(StringStream& out, f32 degrees)
{
	out << "LevelEditor:set_rotation_snap(";
	out << frad(degrees) << ")";
}

void enable_show_grid(StringStream& out, bool enable)
{
	out << "LevelEditor:enable_show_grid(";
	out << lua_bool(enable) << ")";
}

void enable_snap_to_grid(StringStream& out, bool enable)
{
	out << "LevelEditor:enable_snap_to_grid(";
	out << lua_bool(enable) << ")";
}

void enable_debug_render_world(StringStream& out, bool enable)
{
	out << "LevelEditor:enable_debug_render_world(";
	out << lua_bool(enable) << ")";
}

void enable_debug_physics_world(StringStream& out, bool enable)
{
	out << "LevelEditor:enable_debug_physics_world(";
	out << lua_bool(enable) << ")";
}

void set_tool_type(StringStream& out, const ToolType::Enum tt)
{
	out << "LevelEditor:set_tool(LevelEditor.";
	switch (tt)
	{
		case ToolType::PLACE: out << "place_tool"; break;
		case ToolType::MOVE: out << "move_tool"; break;
		case ToolType::ROTATE: out << "rotate_tool"; break;
		case ToolType::SCALE: out << "scale_tool"; break;
	}
	out << ")";
}

void set_snap_mode(StringStream& out, const SnapMode::Enum sm)
{
	out << "LevelEditor:set_snap_mode('";
	out << (sm == SnapMode::RELATIVE ? "relative" : "absolute");
	out << "')";
}

void set_reference_system(StringStream& out, const ReferenceSystem::Enum rs)
{
	out << "LevelEditor:set_reference_system('";
	out << (rs == ReferenceSystem::LOCAL ? "local" : "world");
	out << "')";
}

void spawn_unit(StringStream& out
	, const Guid& id
	, const char* name
	, const Vector3& pos
	, const Quaternion& rot
	, const Vector3& scl)
{
	TempAllocator128 ta;
	DynamicString ds(ta);
	ds.from_guid(id);

	out << "LevelEditor:spawn_unit(";
	out << "\"" << ds.c_str() << "\",";
	out << "\"" << name << "\",";
	lua_vector3(out, pos);	out << ",";
	lua_quaternion(out, rot); out << ",";
	lua_vector3(out, scl); out << ")";
}

void spawn_empty_unit(StringStream& /*out*/, Guid& /*id*/)
{

}

void spawn_sound(StringStream& /*out*/
	, const Guid& /*id*/
	, const char* /*name*/
	, const Vector3& /*pos*/
	, const Quaternion& /*rot*/
	, const f64 /*vol*/
	, const bool /*loop*/)
{

}

void add_tranform_component(StringStream& /*out*/
	, const Guid& /*id*/
	, const Guid& /*component_id*/
	, const Vector3& /*pos*/
	, const Quaternion& /*rot*/
	, const Vector3& /*scl*/)
{

}

void add_mesh_component(StringStream& /*out*/
	, const Guid& /*id*/
	, const Guid& /*component_id*/
	, const char* /*mesh_resource*/
	, const char* /*geometry_name*/
	, const char* /*material_resource*/
	, const bool /*visible*/)
{

}

void add_material_component(StringStream& /*out*/
	, const Guid& /*id*/
	, const Guid& /*component_id*/
	, const char* /*sprite_resource*/
	, const char* /*material_resource*/
	, const bool /*visible*/)
{

}

void add_camera_content(StringStream& /*out*/
	, const Guid& /*id*/
	, const Guid& /*component_id*/
	, const ProjectionType::Enum /*projection*/
	, const f64 /*fov*/
	, const f64 /*far_range*/
	, const f64 /*near_range*/)
{

}

void add_light_component(StringStream& /*out*/
	, const Guid& /*id*/
	, const Guid& /*component_id*/
	, const LightType::Enum /*type*/
	, const f64 /*range*/
	, const f64 /*intensity*/
	, const f64 /*spot_angle*/
	, const Vector3& /*color*/)
{

}

void move_object(StringStream& /*out*/
	, const Guid& /*id*/
	, const Vector3& /*pos*/
	, const Quaternion& /*rot*/
	, const Vector3& /*scl*/)
{

}

void set_light(StringStream& /*out*/
	, const Guid& /*id*/
	, const LightType::Enum /*type*/
	, const f64 /*range*/
	, const f64 /*intensity*/
	, const f64 /*spot_angle*/
	, const Vector3& /*color*/)
{

}

void set_sound_range(StringStream& /*out*/
	, const Guid& /*id*/
	, f64 /*range*/)
{

}

void set_placeable(StringStream& out
	, const char* type
	, const char* name)
{
	out << "LevelEditor:set_placeable(";
	out << "'" << type << "'" << ",";
	out << "'" << name << "'" << ")";
}

void selection_set(StringStream& /*out*/, const Array<Guid>& /*ids*/)
{

}

void camera_view_perspective(StringStream& out)
{
	out << "LevelEditor:camera_view_perspective()";
}

void camera_view_front(StringStream& out)
{
	out << "LevelEditor:camera_view_front()";
}

void camera_view_back(StringStream& out)
{
	out << "LevelEditor:camera_view_back()";
}

void camera_view_right(StringStream& out)
{
	out << "LevelEditor:camera_view_right()";
}

void camera_view_left(StringStream& out)
{
	out << "LevelEditor:camera_view_left()";
}

void camera_view_top(StringStream& out)
{
	out << "LevelEditor:camera_view_top()";
}

void camera_view_bottom(StringStream& out)
{
	out << "LevelEditor:camera_view_bottom()";
}

} // namespace tool

} // namespace crown
