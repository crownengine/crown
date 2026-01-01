/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
/// Functions to encode Vala types to Lua.
namespace Lua
{
	public string bool(bool b)
	{
		return b == true ? "true" : "false";
	}

	public string vector2(Vector2 v)
	{
		return "Vector2(%.17g, %.17g)".printf(v.x, v.y);
	}

	public string vector3(Vector3 v)
	{
		return "Vector3(%.17g, %.17g, %.17g)".printf(v.x, v.y, v.z);
	}

	public string quaternion(Quaternion q)
	{
		return "Quaternion.from_elements(%.17g, %.17g, %.17g, %.17g)".printf(q.x, q.y, q.z, q.w);
	}

} /* namespace Lua */

namespace RuntimeApi
{
	public string quit()
	{
		return "{\"type\":\"quit\"}";
	}

} /* namespace RuntimeApi */

namespace DataCompilerApi
{
	public string compile(Guid id, string data_dir, string platform)
	{
		return "{\"type\":\"compile\",\"id\":\"%s\",\"data_dir\":\"%s\",\"platform\":\"%s\"}".printf(id.to_string()
			, data_dir.replace("\\", "\\\\").replace("\"", "\\\"")
			, platform
			);
	}

	public string refresh_list(uint since_revision)
	{
		return "{\"type\":\"refresh_list\",\"revision\":%u}".printf(since_revision);
	}

} /* namespace DataCompilerApi */

namespace DeviceApi
{
	public string command(string[] args)
	{
		StringBuilder sb = new StringBuilder();
		for (int i = 0; i < args.length; ++i) {
			string arg = args[i].replace("\\", "\\\\").replace("\"", "\\\"");
			sb.append("\"%s\",".printf(arg));
		}

		return "{\"type\":\"command\",\"args\":[%s]}".printf(sb.str);
	}

	public string reload(string type, string name)
	{
		return command({ "reload", type, name });
	}

	public string pause()
	{
		return command({ "pause" });
	}

	public string unpause()
	{
		return command({ "unpause" });
	}

	public string resize(int width, int height)
	{
		return "{\"type\":\"resize\",\"width\":%d,\"height\":%d}".printf(width, height);
	}

	public string frame()
	{
		return "{\"type\":\"frame\"}";
	}

	public string refresh(Gee.ArrayList<Value?> resources)
	{
		StringBuilder sb = new StringBuilder();
		foreach (var res in resources) {
			sb.append("\"%s\",".printf((string)res));
		}

		return "{\"type\":\"refresh\",\"list\":[%s]}".printf(sb.str);
	}

} /* namespace DeviceApi */

namespace LevelEditorApi
{
	public string reset()
	{
		return "LevelEditor:reset()";
	}

	public string set_mouse_state(int x, int y, bool left, bool middle, bool right)
	{
		return "LevelEditor:set_mouse_state(%d,%d,%s,%s,%s)".printf(x
			, y
			, Lua.bool(left)
			, Lua.bool(middle)
			, Lua.bool(right)
			);
	}

	public string mouse_wheel(double delta)
	{
		return "LevelEditor:mouse_wheel(%.17g)".printf(delta);
	}

	public string mouse_down(int x, int y)
	{
		return "LevelEditor:mouse_down(%d,%d)".printf(x, y);
	}

	public string mouse_up(int x, int y)
	{
		return "LevelEditor:mouse_up(%d,%d)".printf(x, y);
	}

	public string key_down(string key)
	{
		return "LevelEditor:key_down(\"%s\")".printf(key);
	}

	public string key_up(string key)
	{
		return "LevelEditor:key_up(\"%s\")".printf(key);
	}

	public string set_tool_type(ToolType type)
	{
		const string _tools[] =
		{
			"place_tool",
			"move_tool",
			"rotate_tool",
			"scale_tool"
		};
		GLib.static_assert(_tools.length == ToolType.COUNT);

		return "LevelEditor:set_tool(LevelEditor.%s)".printf(_tools[(int)type]);
	}

	public string set_snap_mode(SnapMode sm)
	{
		return """LevelEditor:set_snap_mode("%s")""".printf(sm == SnapMode.RELATIVE ? "relative" : "absolute");
	}

	public string set_reference_system(ReferenceSystem rs)
	{
		return """LevelEditor:set_reference_system("%s")""".printf(rs == ReferenceSystem.LOCAL ? "local" : "world");
	}

	public string set_camera_view_type(CameraViewType type)
	{
		if (type == CameraViewType.PERSPECTIVE)
			return "LevelEditor:camera_view_perspective()";
		else if (type == CameraViewType.FRONT)
			return "LevelEditor:camera_view_front()";
		else if (type == CameraViewType.BACK)
			return "LevelEditor:camera_view_back()";
		else if (type == CameraViewType.RIGHT)
			return "LevelEditor:camera_view_right()";
		else if (type == CameraViewType.LEFT)
			return "LevelEditor:camera_view_left()";
		else if (type == CameraViewType.TOP)
			return "LevelEditor:camera_view_top()";
		else if (type == CameraViewType.BOTTOM)
			return "LevelEditor:camera_view_bottom()";
		else
			return "LevelEditor:camera_view_perspective()";
	}

	public string camera_restore(Vector3 position
		, Quaternion rotation
		, double ortho_size
		, double target_distance
		, CameraViewType view_type
		)
	{
		return """LevelEditor._camera:restore(%s, %s, %.17g, %.17g, '%s')""".printf(Lua.vector3(position)
			, Lua.quaternion(rotation)
			, ortho_size
			, target_distance
			, view_type == CameraViewType.PERSPECTIVE ? "perspective" : "orthographic"
			);
	}

	public string frame_objects(Guid?[] ids)
	{
		StringBuilder sb = new StringBuilder();
		sb.append("LevelEditor:frame_objects({");
		for (int i = 0; i < ids.length; ++i)
			sb.append("\"%s\",".printf(ids[i].to_string()));
		sb.append("})");
		return sb.str;
	}

	public string enable_show_grid(bool enabled)
	{
		return "LevelEditor:enable_show_grid(%s)".printf(Lua.bool(enabled));
	}

	public string enable_snap_to_grid(bool enabled)
	{
		return "LevelEditor:enable_snap_to_grid(%s)".printf(Lua.bool(enabled));
	}

	public string enable_debug_render_world(bool enabled)
	{
		return "LevelEditor:enable_debug_render_world(%s)".printf(Lua.bool(enabled));
	}

	public string enable_debug_physics_world(bool enabled)
	{
		return "LevelEditor:enable_debug_physics_world(%s)".printf(Lua.bool(enabled));
	}

	public string set_grid_size(double size)
	{
		return "LevelEditor:set_grid_size(%.17g)".printf(size);
	}

	public string set_rotation_snap(double deg)
	{
		return "LevelEditor:set_rotation_snap(%.17g)".printf(MathUtils.rad(deg));
	}

	public string spawn_unit(Guid id, string name, Vector3 pos, Quaternion rot, Vector3 scl)
	{
		return "LevelEditor:spawn_unit(\"%s\", \"%s\", %s, %s, %s)".printf(id.to_string()
			, name
			, Lua.vector3(pos)
			, Lua.quaternion(rot)
			, Lua.vector3(scl)
			);
	}

	public string spawn_empty_unit(Guid id)
	{
		return "LevelEditor:spawn_empty_unit(\"%s\")".printf(id.to_string());
	}

	public string spawn_skydome(string skydome_name)
	{
		return "LevelEditor:spawn_skydome(\"%s\")".printf(skydome_name);
	}

	public string spawn_sound(Guid id, string name, Vector3 pos, Quaternion rot, double range, double volume, bool loop)
	{
		return "LevelEditor:spawn_sound(\"%s\", \"%s\", %s, %s, %.17g, %.17g, %s)".printf(id.to_string()
			, name
			, Lua.vector3(pos)
			, Lua.quaternion(rot)
			, range
			, volume
			, Lua.bool(loop)
			);
	}

	public string add_tranform_component(Guid id, Guid component_id, Vector3 pos, Quaternion rot, Vector3 scl)
	{
		return "LevelEditor:add_transform_component(\"%s\", \"%s\", %s, %s, %s)".printf(id.to_string()
			, component_id.to_string()
			, Lua.vector3(pos)
			, Lua.quaternion(rot)
			, Lua.vector3(scl)
			);
	}

	public string add_camera_component(Guid id, Guid component_id, string projection, double fov, double far_range, double near_range)
	{
		return "LevelEditor:add_camera_component(\"%s\", \"%s\", \"%s\", %.17g, %.17g, %.17g)".printf(id.to_string()
			, component_id.to_string()
			, projection
			, fov
			, far_range
			, near_range
			);
	}

	public string add_mesh_renderer_component(Guid id
		, Guid component_id
		, string mesh_resource
		, string geometry_name
		, string material_resource
		, bool visible
		, bool cast_shadows
		)
	{
		return "LevelEditor:add_mesh_component(\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", %s, %s)".printf(id.to_string()
			, component_id.to_string()
			, mesh_resource
			, geometry_name
			, material_resource
			, Lua.bool(visible)
			, Lua.bool(cast_shadows)
			);
	}

	public string add_sprite_renderer_component(Guid id
		, Guid component_id
		, string sprite_resource
		, string material_resource
		, double layer
		, double depth
		, bool visible
		, bool flip_x
		, bool flip_y
		)
	{
		return "LevelEditor:add_sprite_component(\"%s\", \"%s\", \"%s\", \"%s\", %.17g, %.17g, %s, %s, %s)".printf(id.to_string()
			, component_id.to_string()
			, sprite_resource
			, material_resource
			, layer
			, depth
			, Lua.bool(visible)
			, Lua.bool(flip_x)
			, Lua.bool(flip_y)
			);
	}

	public string add_light_component(Guid id
		, Guid component_id
		, string type
		, double range
		, double intensity
		, double spot_angle
		, Vector3 color
		, double shadow_bias
		, bool cast_shadows
		)
	{
		return "LevelEditor:add_light_component(\"%s\", \"%s\", \"%s\", %.17g, %.17g, %.17g, %s, %.17g, %s)".printf(id.to_string()
			, component_id.to_string()
			, type
			, range
			, intensity
			, spot_angle
			, Lua.vector3(color)
			, shadow_bias
			, Lua.bool(cast_shadows)
			);
	}

	public string add_animation_state_machine_component(Guid id
		, Guid component_id
		, string state_machine_resource
		)
	{
		return "LevelEditor:add_animation_state_machine_component(\"%s\", \"%s\", \"%s\")".printf(id.to_string()
			, component_id.to_string()
			, state_machine_resource
			);
	}

	public string add_mover_component(Guid id
		, Guid component_id
		, double height
		, double radius
		, double max_slope_angle
		, string collision_filter
		)
	{
		return "LevelEditor:add_mover_component(\"%s\", \"%s\", %.17g, %.17g, %.17g, \"%s\")".printf(id.to_string()
			, component_id.to_string()
			, height
			, radius
			, max_slope_angle
			, collision_filter
			);
	}

	public string add_fog_component(Guid id, Guid component_id)
	{
		return "LevelEditor:add_fog_component(\"%s\", \"%s\")".printf(id.to_string(), component_id.to_string());
	}

	public string add_global_lighting_component(Guid id, Guid component_id)
	{
		return "LevelEditor:add_global_lighting_component(\"%s\", \"%s\")".printf(id.to_string(), component_id.to_string());
	}

	public string add_bloom_component(Guid id, Guid component_id)
	{
		return "LevelEditor:add_bloom_component(\"%s\", \"%s\")".printf(id.to_string(), component_id.to_string());
	}

	public string add_tonemap_component(Guid id, Guid component_id)
	{
		return "LevelEditor:add_tonemap_component(\"%s\", \"%s\")".printf(id.to_string(), component_id.to_string());
	}

	public string unit_destroy_component_type(Guid id, string component_type)
	{
		return "LevelEditor:unit_destroy_component_type(\"%s\", \"%s\")".printf(id.to_string()
			, component_type
			);
	}

	public string move_object(Guid id, Vector3 pos, Quaternion rot, Vector3 scl)
	{
		return "LevelEditor:move_object(\"%s\", %s, %s, %s)".printf(id.to_string()
			, Lua.vector3(pos)
			, Lua.quaternion(rot)
			, Lua.vector3(scl)
			);
	}

	public string set_light(Guid id
		, string type
		, double range
		, double intensity
		, double spot_angle
		, Vector3 color
		, double shadow_bias
		, bool cast_shadows
		)
	{
		return "LevelEditor._objects[\"%s\"]:set_light(\"%s\", %.17g, %.17g, %.17g, %s, %.17g, %s)".printf(id.to_string()
			, type
			, range
			, intensity
			, spot_angle
			, Lua.quaternion({color.x, color.y, color.z, 1.0})
			, shadow_bias
			, Lua.bool(cast_shadows)
			);
	}

	public string set_animation_state_machine(Guid id
		, string state_machine_resource
		)
	{
		return "LevelEditor._objects[\"%s\"]:set_animation_state_machine(\"%s\")".printf(id.to_string()
			, state_machine_resource
			);
	}

	public string set_mover(Guid id
		, double height
		, double radius
		, double max_slope_angle
		, Vector3 center
		)
	{
		return "LevelEditor._objects[\"%s\"]:set_mover(%.17g, %.17g, %.17g, %s)".printf(id.to_string()
			, height
			, radius
			, max_slope_angle
			, Lua.vector3(center)
			);
	}

	public string set_fog(Guid id
		, Vector3 color
		, double density
		, double range_min
		, double range_max
		, double sun_blend
		, bool enabled
		)
	{
		return "LevelEditor._objects[\"%s\"]:set_fog(%s, %.17g, %.17g, %.17g, %.17g, %s)".printf(id.to_string()
			, Lua.vector3(color)
			, density
			, range_min
			, range_max
			, sun_blend
			, Lua.bool(enabled)
			);
	}

	public string set_global_lighting(Guid id
		, string skydome_map
		, double skydome_intensity
		, Vector3 ambient_color
		)
	{
		return "LevelEditor._objects[\"%s\"]:set_global_lighting(\"%s\", %.17g, %s)".printf(id.to_string()
			, skydome_map
			, skydome_intensity
			, Lua.quaternion({ambient_color.x, ambient_color.y, ambient_color.z, 1.0})
			);
	}

	public string set_bloom(Guid id
		, bool enabled
		, double threshold
		, double weight
		, double intensity
		)
	{
		return "LevelEditor._objects[\"%s\"]:set_bloom(%s, %.17g, %.17g, %.17g)".printf(id.to_string()
			, Lua.bool(enabled)
			, threshold
			, weight
			, intensity
			);
	}

	public string set_tonemap(Guid id, string type)
	{
		return "LevelEditor._objects[\"%s\"]:set_tonemap(\"%s\")".printf(id.to_string(), type);
	}

	public string set_sound_range(Guid id, double range)
	{
		return "LevelEditor._objects[\"%s\"]:set_range(%.17g)".printf(id.to_string()
			, range
			);
	}

	public string set_mesh(Guid id
		, string mesh_resource
		, string geometry
		, string material
		, bool visible
		, bool cast_shadows
		)
	{
		return "LevelEditor._objects[\"%s\"]:set_mesh(\"%s\", \"%s\", \"%s\", %s, %s)".printf(id.to_string()
			, mesh_resource
			, geometry
			, material
			, Lua.bool(visible)
			, Lua.bool(cast_shadows)
			);
	}

	public string set_sprite(Guid id
		, string sprite_resource_name
		, string material
		, double layer
		, double depth
		, bool visible
		, bool flip_x
		, bool flip_y
		)
	{
		return "LevelEditor._objects[\"%s\"]:set_sprite(\"%s\", \"%s\", %.17g, %.17g, %s, %s, %s)".printf(id.to_string()
			, sprite_resource_name
			, material
			, layer
			, depth
			, Lua.bool(visible)
			, Lua.bool(flip_x)
			, Lua.bool(flip_y)
			);
	}

	public string set_camera(Guid id, string projection, double fov, double near_range, double far_range)
	{
		return "LevelEditor._objects[\"%s\"]:set_camera(\"%s\", %.17g, %.17g, %.17g)".printf(id.to_string()
			, projection
			, fov
			, near_range
			, far_range
			);
	}

	public string set_placeable(string type, string name)
	{
		return "LevelEditor:set_placeable(\"%s\", \"%s\")".printf(type, name);
	}

	public string selection_set(Guid?[] ids)
	{
		StringBuilder sb = new StringBuilder();
		sb.append("LevelEditor._selection:set({");
		for (int i = 0; i < ids.length; ++i)
			sb.append("\"%s\",".printf(ids[i].to_string()));
		sb.append("})");
		return sb.str;
	}

	public string destroy(Guid id)
	{
		return "LevelEditor:destroy(\"%s\")".printf(id.to_string());
	}

	public string set_color(string name, Vector3 color)
	{
		Quaternion c = Quaternion(color.x, color.y, color.z, 1.0);
		return "Colors.%s = function() return %s end".printf(name, Lua.quaternion(c));
	}

	public string unit_freeze(Guid id)
	{
		return "LevelEditor._objects[\"%s\"]:freeze()".printf(id.to_string());
	}

	public string unit_set_parent(Guid parent_id, Guid child_id)
	{
		return "LevelEditor._objects[\"%s\"]:set_parent(\"%s\")".printf(child_id.to_string(), parent_id.to_string());
	}

} /* namespace LevelEditorApi */

namespace UnitPreviewApi
{
	public string set_preview_resource(string placeable_type, string name)
	{
		return "UnitPreview:set_preview_resource(\"%s\", \"%s\")".printf(placeable_type, name);
	}

} /* namespace UnitPreviewApi */

namespace ThumbnailApi
{
	public string add_request(string placeable_type, string name, string thumbnail_path)
	{
		return "Thumbnail:add_request(\"%s\", \"%s\", \"%s\")".printf(placeable_type
			, name
			, thumbnail_path.replace("\\", "\\\\").replace("\"", "\\\"")
			);
	}

} /* namespace UnitPreviewApi */

namespace StateMachineEditorApi
{
	public string set_unit(string unit_name)
	{
		return "LevelEditor:set_unit(\"%s\")".printf(unit_name);
	}

	public string trigger_animation_event(string event_name)
	{
		return "LevelEditor:trigger_animation_event(\"%s\")".printf(event_name);
	}

	public string set_variable(string variable_name, double value)
	{
		return "LevelEditor:set_variable(\"%s\", %f)".printf(variable_name, value);
	}

} /* namespace StateMachineEditorApi */

} /* namespace Crown */
