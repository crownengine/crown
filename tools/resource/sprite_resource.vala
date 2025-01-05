/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class SpriteResource
{
	public static ImportResult import(Project project, string destination_dir, SList<string> filenames)
	{
		Hashtable importer_settings = null;
		string importer_settings_path = null;
		string image_path;
		string image_type;
		string image_name;
		{
			GLib.File file_src = File.new_for_path(filenames.nth_data(0));
			image_path = file_src.get_path();
			image_type = image_path.substring(image_path.last_index_of_char('.') + 1
				, image_path.length - image_path.last_index_of_char('.') - 1
				);

			GLib.File file_dst       = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));
			string resource_filename = project.resource_filename(file_dst.get_path());
			string resource_path     = ResourceId.normalize(resource_filename);
			string resource_name     = ResourceId.name(resource_path);

			importer_settings_path = project.absolute_path(resource_name) + ".importer_settings";

			int last_slash = resource_name.last_index_of_char('/');
			if (last_slash == -1)
				image_name = resource_name;
			else
				image_name = resource_name.substring(last_slash + 1, resource_name.length - last_slash - 1);
		}

		SpriteImportDialog dlg = new SpriteImportDialog(image_path, image_name);
		dlg._unit_name.sensitive = filenames.length() == 1;
		dlg.show_all();

		if (File.new_for_path(importer_settings_path).query_exists()) {
			importer_settings = SJSON.load_from_path(importer_settings_path);
			dlg.load(importer_settings);
		} else {
			importer_settings = new Hashtable();
		}

		if (dlg.run() != Gtk.ResponseType.OK) {
			dlg.destroy();
			return ImportResult.CANCEL;
		}

		dlg.save(importer_settings);

		int width     = (int)dlg._pixbuf.width;
		int height    = (int)dlg._pixbuf.height;
		int num_h     = (int)dlg.cells.value.x;
		int num_v     = (int)dlg.cells.value.y;
		int cell_w    = (int)dlg.cell.value.x;
		int cell_h    = (int)dlg.cell.value.y;
		int offset_x  = (int)dlg.offset.value.x;
		int offset_y  = (int)dlg.offset.value.y;
		int spacing_x = (int)dlg.spacing.value.x;
		int spacing_y = (int)dlg.spacing.value.y;
		double layer  = dlg.layer.value;
		double depth  = dlg.depth.value;

		Vector2 pivot_xy = sprite_cell_pivot_xy(cell_w, cell_h, dlg.pivot.active);

		bool collision_enabled         = dlg.collision_enabled.active;
		string shape_active_name       = (string)dlg.shape.visible_child_name;
		int circle_collision_center_x  = (int)dlg.circle_collision_center.value.x;
		int circle_collision_center_y  = (int)dlg.circle_collision_center.value.y;
		int circle_collision_radius    = (int)dlg.circle_collision_radius.value;
		int capsule_collision_center_x = (int)dlg.capsule_collision_center.value.x;
		int capsule_collision_center_y = (int)dlg.capsule_collision_center.value.y;
		int capsule_collision_radius   = (int)dlg.capsule_collision_radius.value;
		int capsule_collision_height   = (int)dlg.capsule_collision_height.value;
		int collision_x                = (int)dlg.collision_xy.value.x;
		int collision_y                = (int)dlg.collision_xy.value.y;
		int collision_w                = (int)dlg.collision_wh.value.x;
		int collision_h                = (int)dlg.collision_wh.value.y;
		string actor_class             = (string)dlg.actor_class.value;
		bool lock_rotation_y           = dlg.lock_rotation_y.active;
		double mass                    = (double)dlg.mass.value;
		image_name                     = dlg._unit_name.value;

		dlg.destroy();

		foreach (unowned string filename_i in filenames) {
			GLib.File file_src = File.new_for_path(filename_i);
			string resource_basename;
			if (filenames.length() == 1)
				resource_basename = image_name + "." + image_type;
			else
				resource_basename = file_src.get_basename();

			GLib.File file_dst       = File.new_for_path(Path.build_filename(destination_dir, resource_basename));
			string resource_filename = project.resource_filename(file_dst.get_path());
			string resource_path     = ResourceId.normalize(resource_filename);
			string resource_name     = ResourceId.name(resource_path);

			SJSON.save(importer_settings, project.absolute_path(resource_name) + ".importer_settings");

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
			SJSON.save(material, project.absolute_path(resource_name) + ".material");

			try {
				file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
			} catch (Error e) {
				loge(e.message);
				return ImportResult.ERROR;
			}

			Database db = new Database(project);

			var texture_resource = new TextureResource.sprite(db, Guid.new_guid(), resource_path);
			texture_resource.save(project, resource_name);
			db.reset();

			Guid sprite_id = Guid.new_guid();
			db.create(sprite_id, "sprite");
			db.set_property_double(sprite_id, "width", width);
			db.set_property_double(sprite_id, "height", height);

			double frame_index = 0.0;

			for (int r = 0; r < num_v; ++r) {
				for (int c = 0; c < num_h; ++c) {
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

					Guid frame_id = Guid.new_guid();
					db.create(frame_id, "sprite_frame");
					db.set_property_string    (frame_id, "name", "sprite_%d".printf(c + num_h*r));
					db.set_property_quaternion(frame_id, "region", Quaternion(x, y, cell_w, cell_h));
					db.set_property_vector3   (frame_id, "pivot", Vector3(x + pivot_xy.x, y + pivot_xy.y, 0.0));
					db.set_property_double    (frame_id, "index", frame_index++);

					db.add_to_set(sprite_id, "frames", frame_id);
				}
			}

			db.save(project.absolute_path(resource_name) + ".sprite", sprite_id);
			db.reset();

			// Generate or modify existing .unit.
			Guid unit_id;
			if (db.add_from_resource_path(out unit_id, resource_name + ".unit") != 0) {
				unit_id = Guid.new_guid();
				db.create(unit_id, OBJECT_TYPE_UNIT);
			}

			Unit unit = new Unit(db, unit_id);

			// Create transform
			{
				Guid component_id;
				if (!unit.has_component(out component_id, OBJECT_TYPE_TRANSFORM)) {
					component_id = Guid.new_guid();
					db.create(component_id, OBJECT_TYPE_TRANSFORM);
					db.add_to_set(unit_id, "components", component_id);
				}

				unit.set_component_property_vector3   (component_id, "data.position", VECTOR3_ZERO);
				unit.set_component_property_quaternion(component_id, "data.rotation", QUATERNION_IDENTITY);
				unit.set_component_property_vector3   (component_id, "data.scale", VECTOR3_ONE);
			}

			// Create sprite_renderer
			{
				Guid component_id;
				if (!unit.has_component(out component_id, OBJECT_TYPE_SPRITE_RENDERER)) {
					component_id = Guid.new_guid();
					db.create(component_id, OBJECT_TYPE_SPRITE_RENDERER);
					db.add_to_set(unit_id, "components", component_id);
				}

				unit.set_component_property_string(component_id, "data.material", resource_name);
				unit.set_component_property_string(component_id, "data.sprite_resource", resource_name);
				unit.set_component_property_double(component_id, "data.layer", layer);
				unit.set_component_property_double(component_id, "data.depth", depth);
				unit.set_component_property_bool  (component_id, "data.visible", true);
			}

			if (collision_enabled) {
				// Create collider
				double PIXELS_PER_METER = 32.0;
				{
					Quaternion rotation = QUATERNION_IDENTITY;

					Guid component_id;
					if (!unit.has_component(out component_id, OBJECT_TYPE_COLLIDER)) {
						component_id = Guid.new_guid();
						db.create(component_id, OBJECT_TYPE_COLLIDER);
						db.add_to_set(unit_id, "components", component_id);
					}

					unit.set_component_property_string(component_id, "data.source", "inline");
					if (shape_active_name == "square_collider") {
						double pos_x =  (collision_x + collision_w/2.0 - pivot_xy.x) / PIXELS_PER_METER;
						double pos_y = -(collision_y + collision_h/2.0 - pivot_xy.y) / PIXELS_PER_METER;
						Vector3 position = Vector3(pos_x, 0, pos_y);
						Vector3 half_extents = Vector3(collision_w/2/PIXELS_PER_METER, 0.5/PIXELS_PER_METER, collision_h/2/PIXELS_PER_METER);
						unit.set_component_property_vector3   (component_id, "data.collider_data.position", position);
						unit.set_component_property_quaternion(component_id, "data.collider_data.rotation", rotation);
						unit.set_component_property_string    (component_id, "data.shape", "box");
						unit.set_component_property_vector3   (component_id, "data.collider_data.half_extents", half_extents);
					} else if (shape_active_name == "circle_collider") {
						double pos_x =  (circle_collision_center_x - pivot_xy.x) / PIXELS_PER_METER;
						double pos_y = -(circle_collision_center_y - pivot_xy.y) / PIXELS_PER_METER;
						Vector3 position = Vector3(pos_x, 0, pos_y);
						double radius = circle_collision_radius / PIXELS_PER_METER;
						unit.set_component_property_vector3   (component_id, "data.collider_data.position", position);
						unit.set_component_property_quaternion(component_id, "data.collider_data.rotation", rotation);
						unit.set_component_property_string    (component_id, "data.shape", "sphere");
						unit.set_component_property_double    (component_id, "data.collider_data.radius", radius);
					} else if (shape_active_name == "capsule_collider") {
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
				}

				// Create actor
				{
					Guid component_id;
					if (!unit.has_component(out component_id, OBJECT_TYPE_ACTOR)) {
						component_id = Guid.new_guid();
						db.create(component_id, OBJECT_TYPE_ACTOR);
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
				}
			} else { /* if (collision_enabled) */
				// Destroy collider and actor if any
				Guid component_id;
				if (unit.has_component(out component_id, OBJECT_TYPE_COLLIDER)) {
					db.remove_from_set(unit_id, "components", component_id);
					db.destroy(component_id);
				}
				if (unit.has_component(out component_id, OBJECT_TYPE_ACTOR)) {
					db.remove_from_set(unit_id, "components", component_id);
					db.destroy(component_id);
				}
			}

			db.save(project.absolute_path(resource_name) + ".unit", unit_id);
		}

		return ImportResult.SUCCESS;
	}
}

} /* namespace Crown */
