/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/guid.h"
#include "core/strings/string_stream.h"
#include "core/types.h"
#include "world/types.h"

#if CROWN_PLATFORM_WINDOWS
	#undef RELATIVE
	#undef ABSOLUTE
#endif // CROWN_PLATFORM_WINDOWS

namespace crown
{
namespace tool
{
struct ToolType
{
	enum Enum
	{
		PLACE,
		MOVE,
		ROTATE,
		SCALE
	};
};

struct SnapMode
{
	enum Enum
	{
		RELATIVE,
		ABSOLUTE
	};
};

struct ReferenceSystem
{
	enum Enum
	{
		LOCAL,
		WORLD
	};
};

const char* lua_bool(const bool b);

void lua_vector2(StringStream& out, const Vector2& v);

void lua_vector3(StringStream& out, const Vector3& v);

void lua_quaternion(StringStream& out, const Quaternion& q);

void device_quit(StringStream& out);

void set_mouse_state(StringStream& out, f32 x, f32 y, bool left, bool middle, bool right);

void mouse_down(StringStream& out, f32 x, f32 y);

void mouse_up(StringStream& out, f32 x, f32 y);

void mouse_wheel(StringStream& out, f32 delta);

void key_down(StringStream& out, const char* key);

void key_up(StringStream& out, const char* key);

void set_grid_size(StringStream& out, f32 size);

void set_rotation_snap(StringStream& out, f32 degrees);

void enable_show_grid(StringStream& out, bool enable);

void enable_snap_to_grid(StringStream& out, bool enable);

void enable_debug_render_world(StringStream& out, bool enable);

void enable_debug_physics_world(StringStream& out, bool enable);

void set_tool_type(StringStream& out, const ToolType::Enum tt);

void set_snap_mode(StringStream& out, const SnapMode::Enum sm);

void set_reference_system(StringStream& out, const ReferenceSystem::Enum rs);

void spawn_unit(StringStream& out
	, const Guid& id
	, const char* name
	, const Vector3& pos
	, const Quaternion& rot
	, const Vector3& scl
	);

void spawn_empty_unit(StringStream& /*out*/, Guid& /*id*/);

void spawn_sound(StringStream& /*out*/
	, const Guid& /*id*/
	, const char* /*name*/
	, const Vector3& /*pos*/
	, const Quaternion& /*rot*/
	, const f64 /*vol*/
	, const bool /*loop*/
	);

void add_tranform_component(StringStream& /*out*/
	, const Guid& /*id*/
	, const Guid& /*component_id*/
	, const Vector3& /*pos*/
	, const Quaternion& /*rot*/
	, const Vector3& /*scl*/
	);

void add_mesh_component(StringStream& /*out*/
	, const Guid& /*id*/
	, const Guid& /*component_id*/
	, const char* /*mesh_resource*/
	, const char* /*geometry_name*/
	, const char* /*material_resource*/
	, const bool /*visible*/
	);

void add_material_component(StringStream& /*out*/
	, const Guid& /*id*/
	, const Guid& /*component_id*/
	, const char* /*sprite_resource*/
	, const char* /*material_resource*/
	, const bool /*visible*/
	);

void add_camera_content(StringStream& /*out*/
	, const Guid& /*id*/
	, const Guid& /*component_id*/
	, const ProjectionType::Enum /*projection*/
	, const f64 /*fov*/
	, const f64 /*far_range*/
	, const f64 /*near_range*/
	);

void add_light_component(StringStream& /*out*/
	, const Guid& /*id*/
	, const Guid& /*component_id*/
	, const LightType::Enum /*type*/
	, const f64 /*range*/
	, const f64 /*intensity*/
	, const f64 /*spot_angle*/
	, const Vector3& /*color*/
	);

void move_object(StringStream& /*out*/
	, const Guid& /*id*/
	, const Vector3& /*pos*/
	, const Quaternion& /*rot*/
	, const Vector3& /*scl*/
	);

void set_light(StringStream& /*out*/
	, const Guid& /*id*/
	, const LightType::Enum /*type*/
	, const f64 /*range*/
	, const f64 /*intensity*/
	, const f64 /*spot_angle*/
	, const Vector3& /*color*/
	);

void set_sound_range(StringStream& /*out*/
	, const Guid& /*id*/
	, f64 /*range*/
	);

void set_placeable(StringStream& out
	, const char* type
	, const char* name
	);

void selection_set(StringStream& /*out*/, const Array<Guid>& /*ids*/);

void camera_view_perspective(StringStream& out);

void camera_view_front(StringStream& out);

void camera_view_back(StringStream& out);

void camera_view_right(StringStream& out);

void camera_view_left(StringStream& out);

void camera_view_top(StringStream& out);

void camera_view_bottom(StringStream& out);

} // namespace tool

} // namespace crown
