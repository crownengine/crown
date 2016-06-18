/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

namespace Crown
{
	namespace LevelEditorAPI
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

		public string set_mouse_move(int x, int y, int dx, int dy)
		{
			return "LevelEditor:set_mouse_move(%d,%d,%d,%d)".printf(x, y, dx, dy);
		}

		public string set_mouse_wheel(double delta)
		{
			return "LevelEditor:set_mouse_wheel(%f)".printf(delta);
		}

		public string set_mouse_down(int x, int y)
		{
			return "LevelEditor:mouse_down(%d,%d)".printf(x, y);
		}

		public string set_mouse_up(int x, int y)
		{
			return "LevelEditor:mouse_up(%d,%d)".printf(x, y);
		}

		public string set_key_down(string key)
		{
			return "LevelEditor:key_down(\"%s\")".printf(key);
		}

		public string set_key_up(string key)
		{
			return "LevelEditor:key_up(\"%s\")".printf(key);
		}

		private const string[] _tools =
		{
			"place_tool",
			"move_tool",
			"rotate_tool",
			"scale_tool"
		};

		public string set_tool_type(ToolType type)
		{
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
			return "LevelEditor:set_grid_size(%f)".printf(size);
		}

		public string set_rotation_snap(double deg)
		{
			return "LevelEditor:set_rotation_snap(%f)".printf(MathUtils.rad(deg));
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

		public string spawn_sound(Guid id, Vector3 pos, Quaternion rot, double range, double volume, bool loop)
		{
			return "LevelEditor:spawn_sound(\"%s\", %s, %s, %f, %f, %s)".printf(id.to_string()
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

		public string add_mesh_component(Guid id, Guid component_id, string mesh_resource, string geometry_name, string material_resource, bool visible)
		{
			return "LevelEditor:add_mesh_component(\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", %s)".printf(id.to_string()
				, component_id.to_string()
				, mesh_resource
				, geometry_name
				, material_resource
				, Lua.bool(visible)
				);
		}

		public string add_light_component(Guid id, Guid component_id, string type, double range, double intensity, double spot_angle, Vector3 color)
		{
			return "LevelEditor:add_light_component(\"%s\", \"%s\", \"%s\", %f, %f, %f, %s)".printf(id.to_string()
				, component_id.to_string()
				, type
				, range
				, intensity
				, spot_angle
				, Lua.vector3(color)
				);
		}

		public string move_object(Guid id, Vector3 pos, Quaternion rot, Vector3 scl)
		{
			return @"LevelEditor:move_object(\"%s\", %s, %s, %s)".printf(id.to_string()
				, Lua.vector3(pos)
				, Lua.quaternion(rot)
				, Lua.vector3(scl)
				);
		}

		public string set_placeable(PlaceableType type, string name)
		{
			return "LevelEditor:set_placeable(\"%s\", \"%s\")".printf((type == PlaceableType.UNIT ? "unit" : "sound"), name);
		}

		public string set_selected_unit(Guid id)
		{
			return @"LevelEditor:set_selected_unit(\"%s\")".printf(id.to_string());
		}

		public string destroy(Guid id)
		{
			return @"LevelEditor:destroy(\"%s\")".printf(id.to_string());
		}

		public string set_color(string name, Vector3 color)
		{
			Quaternion c = Quaternion(color.x, color.y, color.z, 1.0);
			return @"Colors.%s = function() return %s end".printf(name, Lua.quaternion(c));
		}
	}
}
