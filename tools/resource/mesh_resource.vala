/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gee;
using Gtk;

namespace Crown
{
public class MeshResource
{
	public static void create_components(Database db, Guid parent_unit_id, Guid unit_id, string material_name, string resource_name, string node_name, Hashtable node)
	{
		Unit unit = new Unit(db, unit_id);

		Matrix4x4 matrix_local = Matrix4x4.from_array((ArrayList<Value?>)node["matrix_local"]);
		Vector3 position = matrix_local.t.to_vector3();
		Quaternion rotation = matrix_local.rotation();
		Vector3 scale = matrix_local.scale();

		// Create transform
		{
			Guid component_id;
			if (!unit.has_component(out component_id, "transform"))
			{
				component_id = Guid.new_guid();
				db.create(component_id);
				db.add_to_set(unit_id, "components", component_id);
			}

			unit.set_component_property_vector3   (component_id, "data.position", position);
			unit.set_component_property_quaternion(component_id, "data.rotation", rotation);
			unit.set_component_property_vector3   (component_id, "data.scale", scale);
			unit.set_component_property_string    (component_id, "type", "transform");
		}

		// Create mesh_renderer
		{
			Guid component_id;
			if (!unit.has_component(out component_id, "mesh_renderer"))
			{
				component_id = Guid.new_guid();
				db.create(component_id);
				db.add_to_set(unit_id, "components", component_id);
			}

			unit.set_component_property_string(component_id, "data.geometry_name", node_name);
			unit.set_component_property_string(component_id, "data.material", material_name);
			unit.set_component_property_string(component_id, "data.mesh_resource", resource_name);
			unit.set_component_property_bool  (component_id, "data.visible", true);
			unit.set_component_property_string(component_id, "type", "mesh_renderer");
		}

		// Create collider
		{
			Guid component_id;
			if (!unit.has_component(out component_id, "collider"))
			{
				component_id = Guid.new_guid();
				db.create(component_id);
				db.add_to_set(unit_id, "components", component_id);
			}

			unit.set_component_property_string(component_id, "data.shape", "mesh");
			unit.set_component_property_string(component_id, "data.scene", resource_name);
			unit.set_component_property_string(component_id, "data.name", node_name);
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

			unit.set_component_property_string(component_id, "data.class", "static");
			unit.set_component_property_string(component_id, "data.collision_filter", "default");
			unit.set_component_property_double(component_id, "data.mass", 10);
			unit.set_component_property_string(component_id, "data.material", "default");
			unit.set_component_property_string(component_id, "type", "actor");
		}

		if (parent_unit_id != unit_id)
			db.add_to_set(parent_unit_id, "children", unit_id);

		if (node.has_key("children"))
		{
			Hashtable children = (Hashtable)node["children"];
			foreach (var child in children.entries)
			{
				Guid new_unit_id = Guid.new_guid();
				db.create(new_unit_id);
				create_components(db, unit_id, new_unit_id, material_name, resource_name, child.key,(Hashtable)child.value);
			}
		}
	}

	public static int import(Project project, string destination_dir, SList<string> filenames)
	{
		foreach (unowned string filename_i in filenames)
		{
			GLib.File file_src = File.new_for_path(filename_i);
			GLib.File file_dst = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));

			string resource_filename = project._source_dir.get_relative_path(file_dst);
			string resource_path     = resource_filename.substring(0, resource_filename.last_index_of_char('.'));
			string resource_name     = project.resource_path_to_resource_name(resource_path);

			// Choose material or create new one
			FileChooserDialog mtl = new FileChooserDialog("Select material... (Cancel to create a new one)"
				, null
				, FileChooserAction.OPEN
				, "Cancel"
				, ResponseType.CANCEL
				, "Select"
				, ResponseType.ACCEPT
				);
			mtl.set_current_folder(project.source_dir());

			FileFilter fltr = new FileFilter();
			fltr.set_filter_name("Material (*.material)");
			fltr.add_pattern("*.material");
			mtl.add_filter(fltr);

			string material_path = resource_path;
			if (mtl.run() == (int)ResponseType.ACCEPT)
			{
				material_path = project._source_dir.get_relative_path(File.new_for_path(mtl.get_filename()));
				material_path = material_path.substring(0, material_path.last_index_of_char('.'));
			}
			else
			{
				Hashtable material = new Hashtable();
				material["shader"]   = "mesh+DIFFUSE_MAP";
				material["textures"] = new Hashtable();
				material["uniforms"] = new Hashtable();
				SJSON.save(material, Path.build_filename(project.source_dir(), material_path) + ".material");
			}
			mtl.destroy();
			string material_name = project.resource_path_to_resource_name(material_path);

			try
			{
				file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
			}
			catch (Error e)
			{
				loge(e.message);
			}

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

			Hashtable mesh = SJSON.load_from_path(filename_i);
			Hashtable mesh_nodes = (Hashtable)mesh["nodes"];

			if (mesh_nodes.size > 1)
			{
				// Create an extra "root" unit to accommodate multiple root objects. This
				// "root" unit will only have a transform centered at origin to allow other
				// objects to be linked to it via the SceneGraph.
				Unit unit = new Unit(db, unit_id);

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

			Guid new_unit_id = unit_id;
			foreach (var entry in mesh_nodes.entries)
			{
				if (mesh_nodes.size > 1)
				{
					// If the mesh contains multiple root objects, create a new unit for each
					// one of those, otherwise put the components inside the base unit.
					new_unit_id = Guid.new_guid();
					db.create(new_unit_id);
				}
				create_components(db, unit_id, new_unit_id, material_name, resource_name, entry.key, (Hashtable)entry.value);
			}

			db.save(Path.build_filename(project.source_dir(), resource_path + ".unit"), unit_id);
		}

		return 0;
	}
}

}
