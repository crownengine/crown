/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
namespace MeshResource
{
	public static void create_components(Database db, Guid parent_unit_id, Guid unit_id, string material_name, string resource_name, string node_name, Hashtable node)
	{
		Unit unit = Unit(db, unit_id);

		Matrix4x4 matrix_local = Matrix4x4.from_array((Gee.ArrayList<Value?>)node["matrix_local"]);
		Vector3 position = matrix_local.t.to_vector3();
		Quaternion rotation = matrix_local.rotation();
		Vector3 scale = matrix_local.scale();

		// Create transform
		{
			Guid component_id;
			if (!unit.has_component(out component_id, OBJECT_TYPE_TRANSFORM)) {
				component_id = Guid.new_guid();
				db.create(component_id, OBJECT_TYPE_TRANSFORM);
				db.add_to_set(unit_id, "components", component_id);
			}

			unit.set_component_vector3   (component_id, "data.position", position);
			unit.set_component_quaternion(component_id, "data.rotation", rotation);
			unit.set_component_vector3   (component_id, "data.scale", scale);
			unit.set_component_string    (component_id, "data.name", node_name);
		}

		// Create mesh_renderer
		{
			Guid component_id;
			if (!unit.has_component(out component_id, OBJECT_TYPE_MESH_RENDERER)) {
				component_id = Guid.new_guid();
				db.create(component_id, OBJECT_TYPE_MESH_RENDERER);
				db.add_to_set(unit_id, "components", component_id);
			}

			unit.set_component_string(component_id, "data.geometry_name", node_name);
			unit.set_component_string(component_id, "data.material", material_name);
			unit.set_component_string(component_id, "data.mesh_resource", resource_name);
			unit.set_component_bool  (component_id, "data.visible", true);
		}

		// Create collider
		{
			Guid component_id;
			if (!unit.has_component(out component_id, OBJECT_TYPE_COLLIDER)) {
				component_id = Guid.new_guid();
				db.create(component_id, OBJECT_TYPE_COLLIDER);
				db.add_to_set(unit_id, "components", component_id);
			}

			unit.set_component_string(component_id, "data.shape", "mesh");
			unit.set_component_string(component_id, "data.scene", resource_name);
			unit.set_component_string(component_id, "data.name", node_name);
		}

		// Create actor
		{
			Guid component_id;
			if (!unit.has_component(out component_id, OBJECT_TYPE_ACTOR)) {
				component_id = Guid.new_guid();
				db.create(component_id, OBJECT_TYPE_ACTOR);
				db.add_to_set(unit_id, "components", component_id);
			}

			unit.set_component_string(component_id, "data.class", "static");
			unit.set_component_string(component_id, "data.collision_filter", "default");
			unit.set_component_double(component_id, "data.mass", 10);
			unit.set_component_string(component_id, "data.material", "default");
		}

		if (parent_unit_id != unit_id)
			db.add_to_set(parent_unit_id, "children", unit_id);

		if (node.has_key("children")) {
			Hashtable children = (Hashtable)node["children"];
			foreach (var child in children.entries) {
				Guid new_unit_id = Guid.new_guid();
				db.create(new_unit_id, OBJECT_TYPE_UNIT);
				create_components(db, unit_id, new_unit_id, material_name, resource_name, child.key, (Hashtable)child.value);
			}
		}
	}

	public static ImportResult do_import(Database database, string destination_dir, SList<string> filenames)
	{
		Project project = database._project;

		foreach (unowned string filename_i in filenames) {
			GLib.File file_src = File.new_for_path(filename_i);

			GLib.File file_dst       = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));
			string resource_filename = project.resource_filename(file_dst.get_path());
			string resource_path     = ResourceId.normalize(resource_filename);
			string resource_name     = ResourceId.name(resource_path);

			Database db = new Database(project);

			string material_name = resource_name;
			MaterialResource material_resource = MaterialResource.mesh(db, Guid.new_guid());
			if (material_resource.save(project, material_name) != 0)
				return ImportResult.ERROR;

			try {
				file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
			} catch (Error e) {
				loge(e.message);
				return ImportResult.ERROR;
			}

			// Generate or modify existing .unit.
			Guid unit_id;
			if (db.add_from_resource_path(out unit_id, resource_name + ".unit") != 0) {
				unit_id = Guid.new_guid();
				db.create(unit_id, OBJECT_TYPE_UNIT);
			}

			try {
				Hashtable mesh = SJSON.load_from_path(filename_i);
				Hashtable mesh_nodes = (Hashtable)mesh["nodes"];

				if (mesh_nodes.size > 1) {
					// Create an extra "root" unit to accommodate multiple root objects. This
					// "root" unit will only have a transform centered at origin to allow other
					// objects to be linked to it via the SceneGraph.
					Unit unit = Unit(db, unit_id);

					Guid component_id;
					if (!unit.has_component(out component_id, OBJECT_TYPE_TRANSFORM)) {
						component_id = Guid.new_guid();
						db.create(component_id, OBJECT_TYPE_TRANSFORM);
						db.add_to_set(unit_id, "components", component_id);
					}

					unit.set_component_vector3   (component_id, "data.position", VECTOR3_ZERO);
					unit.set_component_quaternion(component_id, "data.rotation", QUATERNION_IDENTITY);
					unit.set_component_vector3   (component_id, "data.scale", VECTOR3_ONE);
				}

				Guid new_unit_id = unit_id;
				foreach (var entry in mesh_nodes.entries) {
					if (mesh_nodes.size > 1) {
						// If the mesh contains multiple root objects, create a new unit for each
						// one of those, otherwise put the components inside the base unit.
						new_unit_id = Guid.new_guid();
						db.create(new_unit_id, OBJECT_TYPE_UNIT);
					}
					create_components(db
						, unit_id
						, new_unit_id
						, material_name
						, resource_name
						, entry.key
						, (Hashtable)entry.value
						);
				}

				if (db.save(project.absolute_path(resource_name) + ".unit", unit_id) != 0)
					return ImportResult.ERROR;
			} catch (JsonSyntaxError e) {
				loge(e.message);
				return ImportResult.ERROR;
			}
		}

		return ImportResult.SUCCESS;
	}

	public static void import(Import import_result, Database database, string destination_dir, SList<string> filenames, Gtk.Window? parent_window)
	{
		SList<string> fbx_filenames = new SList<string>();
		SList<string> mesh_filenames = new SList<string>();
		foreach (unowned string filename_i in filenames) {
			string fi = filename_i.down();
			if (fi.has_suffix(".fbx"))
				fbx_filenames.append(filename_i);
			else if (fi.has_suffix(".mesh"))
				mesh_filenames.append(filename_i);
			else
				assert(false);
		}

		ImportResult res = ImportResult.SUCCESS;
		if (mesh_filenames != null)
			res = MeshResource.do_import(database, destination_dir, mesh_filenames);
		if (res == ImportResult.SUCCESS && fbx_filenames != null)
			FBXImporter.import(import_result, database, destination_dir, fbx_filenames, parent_window);
		else
			import_result(res);
	}

} /* namespace MeshResource */

} /* namespace Crown */
