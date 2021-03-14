/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gee;

namespace Crown
{
public class SpriteResource
{
	public static int import(Project project, string destination_dir, SList<string> filenames)
	{
		Hashtable importer_settings = null;
		string importer_settings_path = null;
		{
			GLib.File file_src = File.new_for_path(filenames.nth_data(0));
			GLib.File file_dst = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));

			string resource_filename = project._source_dir.get_relative_path(file_dst);
			string resource_path     = resource_filename.substring(0, resource_filename.last_index_of_char('.'));

			importer_settings_path = Path.build_filename(project.source_dir(), resource_path) + ".importer_settings";
		}

		SpriteImportDialog sid = new SpriteImportDialog(filenames.nth_data(0));
		sid.show_all();

		if (File.new_for_path(importer_settings_path).query_exists())
		{
			importer_settings = SJSON.load_from_path(importer_settings_path);
			sid.load(importer_settings);
		}
		else
		{
			importer_settings = new Hashtable();
		}

		if (sid.run() != Gtk.ResponseType.OK)
		{
			sid.destroy();
			return 1;
		}

		sid.save(importer_settings);

		int width     = (int)sid._pixbuf.width;
		int height    = (int)sid._pixbuf.height;
		int num_h     = (int)sid.cells.value.x;
		int num_v     = (int)sid.cells.value.y;
		int cell_w    = (int)sid.cell.value.x;
		int cell_h    = (int)sid.cell.value.y;
		int offset_x  = (int)sid.offset.value.x;
		int offset_y  = (int)sid.offset.value.y;
		int spacing_x = (int)sid.spacing.value.x;
		int spacing_y = (int)sid.spacing.value.y;
		double layer  = sid.layer.value;
		double depth  = sid.depth.value;

		Vector2 pivot_xy = sprite_cell_pivot_xy(cell_w, cell_h, sid.pivot.active);

		bool collision_enabled         = sid.collision_enabled.active;
		string shape_active_name       = (string)sid.shape.visible_child_name;
		int circle_collision_center_x  = (int)sid.circle_collision_center.value.x;
		int circle_collision_center_y  = (int)sid.circle_collision_center.value.y;
		int circle_collision_radius    = (int)sid.circle_collision_radius.value;
		int capsule_collision_center_x = (int)sid.capsule_collision_center.value.x;
		int capsule_collision_center_y = (int)sid.capsule_collision_center.value.y;
		int capsule_collision_radius   = (int)sid.capsule_collision_radius.value;
		int capsule_collision_height   = (int)sid.capsule_collision_height.value;
		int collision_x                = (int)sid.collision_xy.value.x;
		int collision_y                = (int)sid.collision_xy.value.y;
		int collision_w                = (int)sid.collision_wh.value.x;
		int collision_h                = (int)sid.collision_wh.value.y;
		string actor_class             = (string)sid.actor_class.value;
		bool lock_rotation_y           = sid.lock_rotation_y.active;
		double mass                    = (double)sid.mass.value;

		sid.destroy();

		foreach (unowned string filename_i in filenames)
		{
			GLib.File file_src = File.new_for_path(filename_i);
			GLib.File file_dst = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));

			string resource_filename = project._source_dir.get_relative_path(file_dst);
			string resource_path     = resource_filename.substring(0, resource_filename.last_index_of_char('.'));
			string resource_name     = project.resource_path_to_resource_name(resource_path);

			SJSON.save(importer_settings, Path.build_filename(project.source_dir(), resource_path) + ".importer_settings");

			Hashtable textures = new Hashtable();
			textures["u_albedo"] = resource_name;

			Hashtable uniform = new Hashtable();
			uniform["type"]  = "vector4";
			uniform["value"] = Vector4(1.0, 1.0, 1.0, 1.0).to_array();

			Hashtable uniforms = new Hashtable();
			uniforms["u_color"] = uniform;

			Hashtable material = new Hashtable();
			material["shader"]   = "sprite";
			material["textures"] = textures;
			material["uniforms"] = uniforms;
			SJSON.save(material, Path.build_filename(project.source_dir(), resource_path) + ".material");

			try
			{
				file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
			}
			catch (Error e)
			{
				loge(e.message);
			}

			Hashtable texture = new Hashtable();
			texture["source"]        = project.resource_path_to_resource_name(resource_filename);
			texture["generate_mips"] = false;
			texture["normal_map"]    = false;
			SJSON.save(texture, Path.build_filename(project.source_dir(), resource_path) + ".texture");

			Hashtable sprite = new Hashtable();
			sprite["width"]  = width;
			sprite["height"] = height;

			ArrayList<Value?> frames = new ArrayList<Value?>();
			for (int r = 0; r < num_v; ++r)
			{
				for (int c = 0; c < num_h; ++c)
				{
					Vector2 cell_xy = sprite_cell_xy(r
						, c
						, offset_x
						, offset_y
						, cell_w
						, cell_h
						, spacing_x
						, spacing_y
						);

					// Pivot is relative to the top-left corner of the cell
					int x = (int)cell_xy.x;
					int y = (int)cell_xy.y;

					Hashtable data = new Hashtable();
					data["name"]   = "sprite_%d".printf(c+num_h*r);
					data["region"] = Vector4(x, y, cell_w, cell_h).to_array();
					data["pivot"]  = Vector2(x+pivot_xy.x, y+pivot_xy.y).to_array();
					frames.add(data);
				}
			}
			sprite["frames"] = frames;

			SJSON.save(sprite, Path.build_filename(project.source_dir(), resource_path) + ".sprite");

			// Generate .unit
			project.load_unit(resource_name);

			Guid unit_id;
			Database db = project._database;

			if (db.has_property(GUID_ZERO, resource_name + ".unit"))
			{
				unit_id = db.get_property_guid(GUID_ZERO, resource_name + ".unit");
			}
			else
			{
				db = new Database();
				unit_id = Guid.new_guid();
				db.create(unit_id);
			}

			Unit unit = new Unit(db, unit_id);

			// Create transform
			{
				Guid component_id;
				if (!unit.has_component(out component_id, "transform"))
				{
					component_id = Guid.new_guid();
					db.create(component_id);
					db.add_to_set(unit_id, "components", component_id);
				}

				unit.set_component_property_vector3   (component_id, "data.position", VECTOR3_ZERO);
				unit.set_component_property_quaternion(component_id, "data.rotation", QUATERNION_IDENTITY);
				unit.set_component_property_vector3   (component_id, "data.scale", VECTOR3_ONE);
				unit.set_component_property_string    (component_id, "type", "transform");
			}

			// Create sprite_renderer
			{
				Guid component_id;
				if (!unit.has_component(out component_id, "sprite_renderer"))
				{
					component_id = Guid.new_guid();
					db.create(component_id);
					db.add_to_set(unit_id, "components", component_id);
				}

				unit.set_component_property_string(component_id, "data.material", resource_name);
				unit.set_component_property_string(component_id, "data.sprite_resource", resource_name);
				unit.set_component_property_double(component_id, "data.layer", layer);
				unit.set_component_property_double(component_id, "data.depth", depth);
				unit.set_component_property_bool  (component_id, "data.visible", true);
				unit.set_component_property_string(component_id, "type", "sprite_renderer");
			}

			if (collision_enabled)
			{
				// Create collider
				double PIXELS_PER_METER = 32.0;
				{
					Quaternion rotation = QUATERNION_IDENTITY;

					Guid component_id;
					if (!unit.has_component(out component_id, "collider"))
					{
						component_id = Guid.new_guid();
						db.create(component_id);
						db.add_to_set(unit_id, "components", component_id);
					}

					unit.set_component_property_string(component_id, "data.source", "inline");
					if (shape_active_name == "square_collider")
					{
						double pos_x =  (collision_x + collision_w/2.0 - pivot_xy.x) / PIXELS_PER_METER;
						double pos_y = -(collision_y + collision_h/2.0 - pivot_xy.y) / PIXELS_PER_METER;
						Vector3 position = Vector3(pos_x, 0, pos_y);
						Vector3 half_extents = Vector3(collision_w/2/PIXELS_PER_METER, 0.5/PIXELS_PER_METER, collision_h/2/PIXELS_PER_METER);
						unit.set_component_property_vector3   (component_id, "data.collider_data.position", position);
						unit.set_component_property_quaternion(component_id, "data.collider_data.rotation", rotation);
						unit.set_component_property_string    (component_id, "data.shape", "box");
						unit.set_component_property_vector3   (component_id, "data.collider_data.half_extents", half_extents);
					}
					else if (shape_active_name == "circle_collider")
					{
						double pos_x =  (circle_collision_center_x - pivot_xy.x) / PIXELS_PER_METER;
						double pos_y = -(circle_collision_center_y - pivot_xy.y) / PIXELS_PER_METER;
						Vector3 position = Vector3(pos_x, 0, pos_y);
						double radius = circle_collision_radius / PIXELS_PER_METER;
						unit.set_component_property_vector3   (component_id, "data.collider_data.position", position);
						unit.set_component_property_quaternion(component_id, "data.collider_data.rotation", rotation);
						unit.set_component_property_string    (component_id, "data.shape", "sphere");
						unit.set_component_property_double    (component_id, "data.collider_data.radius", radius);
					}
					else if (shape_active_name == "capsule_collider")
					{
						double pos_x =  (capsule_collision_center_x - pivot_xy.x) / PIXELS_PER_METER;
						double pos_y = -(capsule_collision_center_y - pivot_xy.y) / PIXELS_PER_METER;
						Vector3 position = Vector3(pos_x, 0, pos_y);
						double radius = capsule_collision_radius / PIXELS_PER_METER;
						double capsule_height = (capsule_collision_height - 2*capsule_collision_radius) / PIXELS_PER_METER;
						unit.set_component_property_vector3   (component_id, "data.collider_data.position", position);
						unit.set_component_property_quaternion(component_id, "data.collider_data.rotation", Quaternion.from_axis_angle(Vector3(0, 0, 1), (float)Math.PI/2));
						unit.set_component_property_string    (component_id, "data.shape", "capsule");
						unit.set_component_property_double    (component_id, "data.collider_data.radius", radius);
						unit.set_component_property_double    (component_id, "data.collider_data.height", capsule_height);
					}
					unit.set_component_property_string(component_id, "type", "collider");
				}

				// Create actor
				{
					Guid component_id;
					if (!unit.has_component(out component_id, "actor"))
					{
						component_id = Guid.new_guid();
						db.create(component_id);
						db.add_to_set(unit_id, "components", component_id);
					}

					unit.set_component_property_string(component_id, "data.class", actor_class);
					unit.set_component_property_string(component_id, "data.collision_filter", "default");
					unit.set_component_property_bool  (component_id, "data.lock_rotation_x", true);
					unit.set_component_property_bool  (component_id, "data.lock_rotation_y", lock_rotation_y);
					unit.set_component_property_bool  (component_id, "data.lock_rotation_z", true);
					unit.set_component_property_bool  (component_id, "data.lock_translation_x", false);
					unit.set_component_property_bool  (component_id, "data.lock_translation_y", true);
					unit.set_component_property_bool  (component_id, "data.lock_translation_z", false);
					unit.set_component_property_double(component_id, "data.mass", mass);
					unit.set_component_property_string(component_id, "data.material", "default");
					unit.set_component_property_string(component_id, "type", "actor");
				}
			}
			else /* if (collision_enabled) */
			{
				// Destroy collider and actor if any
				Guid component_id;
				if (unit.has_component(out component_id, "collider"))
				{
					db.remove_from_set(unit_id, "components", component_id);
					db.destroy(component_id);
				}
				if (unit.has_component(out component_id, "actor"))
				{
					db.remove_from_set(unit_id, "components", component_id);
					db.destroy(component_id);
				}
			}

			db.save(Path.build_filename(project.source_dir(), resource_path + ".unit"), unit_id);
		}

		return 0;
	}
}

}
