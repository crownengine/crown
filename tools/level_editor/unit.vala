/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gee;

namespace Crown
{
public struct Unit
{
	public static Hashtable _component_registry;
	public Database _db;
	public Guid _id;

	public Unit(Database db, Guid id)
	{
		_db = db;
		_id = id;
	}

	/// Loads the unit @a name.
	public static void load_unit(Database db, string name)
	{
		string resource_path = name + ".unit";

		Guid prefab_id = GUID_ZERO;
		if (db.add_from_resource_path(out prefab_id, resource_path) != 0)
			return; // Caller can query the database to check for error.
		assert(prefab_id != GUID_ZERO);
	}

	public void create_empty()
	{
		_db.create(_id, OBJECT_TYPE_UNIT);
	}

	public void create(string? prefab, Vector3 pos, Quaternion rot, Vector3 scl)
	{
		create_empty();

		if (prefab != null)
			_db.set_property_string(_id, "prefab", prefab);

		set_local_position(pos);
		set_local_rotation(rot);
		set_local_scale(scl);
	}
	public bool export_to_file(string path)
	{
		try
		{
			Gee.ArrayList<string> resources_to_export = new Gee.ArrayList<string>();
			string unit_data = generate_export_data(resources_to_export); // [!] Add 'project'
			
			// Write the unit data to the export file
			FileUtils.set_contents(path, unit_data);
			
			// Get the export directory and the base name of the unit (without extension)
			string export_dir = Path.get_dirname(path);
			string unit_basename = Path.get_basename(path).replace(".unit", "");  // Base name of the .unit file (without extension)
	
			print("Export directory: " + export_dir);
	
			foreach (string resource_path in resources_to_export)
			{
				// Resolve the absolute path of the resource (if relative)
				string abs_path = resource_path;
				if (!Path.is_absolute(abs_path))
				{
					abs_path = _db.get_project().absolute_path(resource_path);
				}
	
				print("Resource absolute path: " + abs_path);
				
				// Manually get the file extension using Path (rindex_of for the last dot)
				string ext = abs_path.substring(abs_path.last_index_of_char('.') + 1);  // Get the extension manually
				
				// Rename the resource to match the unit's base name (preserving the extension)
				string new_filename = unit_basename + "." + ext;
				
				// Create the destination path using the new filename
				string dest_path = Path.build_filename(export_dir, new_filename);
				print("Destination path: " + dest_path);
				
				// Copy the resource file to the new destination
				File src = File.new_for_path(abs_path);
				File dest = File.new_for_path(dest_path);
	
				// Copy with overwrite option
				src.copy(dest, FileCopyFlags.OVERWRITE);
				print("Copied from: " + abs_path + " to: " + dest_path);
			}
	
			return true;
		}
		catch (Error e)
		{
			loge("Failed to export unit: " + e.message);
			return false;
		}
	}
	
	private string generate_export_data(Gee.List<string> resources)
	{
		StringBuilder sb = new StringBuilder();
		
		sb.append("_guid = \"%s\"\n".printf(_id.to_string()));
		sb.append("_type = \"unit\"\n");
		
		sb.append("components = [\n");
		
		HashSet<Guid?> all_components = new HashSet<Guid?>(Guid.hash_func, Guid.equal_func);
		collect_all_components(_id, all_components);
		
		if (all_components.size > 0) {
			foreach (Guid? component_id in all_components) {
				if (component_id == null) continue;
				
				string component_type = _db.object_type(component_id);
				
				sb.append("\t{\n");
				sb.append("\t\t_guid = \"%s\"\n".printf(component_id.to_string()));
				sb.append("\t\t_type = \"%s\"\n".printf(component_type));
				
				sb.append("\t\tdata = {\n");
				string[] keys = _db.get_keys(component_id);
				foreach (string key in keys) {
					if (key == "_type" || key == "_guid" || key == "type") continue;
					
					string cleaned_key = key.replace("data.", "");
					Value? val = get_component_property(component_id, key);
					if (val != null) {
						sb.append("\t\t\t%s = %s\n".printf(cleaned_key, value_to_lua(val)));
						
						// Si c'est un mesh_renderer, collecte les ressources
						if (component_type == "mesh_renderer" && (cleaned_key == "mesh_resource" || cleaned_key == "material")) {
							string resource_path = (string)val;
						
							// Add .mesh or .material extension if missing
							if (cleaned_key == "mesh_resource" && !resource_path.has_suffix(".mesh")) {
								resource_path += ".mesh";
							} else if (cleaned_key == "material" && !resource_path.has_suffix(".material")) {
								resource_path += ".material";
							}
						
							resources.add(resource_path);
						}						
					}
				}
				sb.append("\t\t}\n");
				
				sb.append("\t\ttype = \"%s\"\n".printf(component_type));
				sb.append("\t},\n");
			}
		}
		
		sb.append("]\n");
		return sb.str;
	}

	private string value_to_lua(Value val)
	{
		Type type = val.type();
		if (type == typeof(string)) {
			return "\"%s\"".printf((string)val);
		}
		if (type == typeof(Guid)) {
			return "\"%s\"".printf(((Guid)val).to_string());
		}
		if (type == typeof(bool)) {
			return ((bool)val) ? "true" : "false";
		}
		if (type == typeof(double)) {
			return ((double)val).to_string();
		}
		if (type == typeof(Vector3)) {
			Vector3 v = (Vector3)val;
			return "[%.3f, %.3f, %.3f]".printf(v.x, v.y, v.z);
		}
		if (type == typeof(Quaternion)) {
			Quaternion q = (Quaternion)val;
			return "[%.3f, %.3f, %.3f, %.3f]".printf(q.x, q.y, q.z, q.w); 
		}
		return "nil";
	}
	
	private void collect_all_components(Guid unit_id, Gee.Set<Guid?> components)
	{
		Value? direct_components = _db.get_property(unit_id, "components");
		if (direct_components != null) {
			var component_set = direct_components as Gee.HashSet<Guid?>;
			if (component_set != null) {
				components.add_all(component_set);
			}
		}
		
		Value? prefab = _db.get_property(unit_id, "prefab");
		if (prefab != null) {
			string prefab_path = (string)prefab;
			Unit.load_unit(_db, prefab_path);
			Guid prefab_id = _db.get_property_guid(GUID_ZERO, prefab_path + ".unit");
			collect_all_components(prefab_id, components);
		}
		
		string[] deleted_keys = _db.get_keys(unit_id);
		foreach (string key in deleted_keys) {
			if (key.has_prefix("deleted_components.#")) {
				Guid deleted_id = Guid.parse(key.split("#")[1]);
				components.remove(deleted_id);
			}
		}
	}
	
	public Value? get_component_property(Guid component_id, string key)
	{
		Value? val;

		// Search in components
		val = _db.get_property(_id, "components");
		if (val != null) {
			if (((HashSet<Guid?>)val).contains(component_id))
				return _db.get_property(component_id, key);
		}

		// Search in modified_components
		val = _db.get_property(_id, "modified_components.#" + component_id.to_string() + "." + key);
		if (val != null)
			return val;

		// Search in prefab
		val = _db.get_property(_id, "prefab");
		if (val != null) {
			// Convert prefab path to object ID.
			string prefab = (string)val;
			Unit.load_unit(_db, prefab);
			Guid prefab_id = _db.get_property_guid(GUID_ZERO, prefab + ".unit");

			Unit unit = Unit(_db, prefab_id);
			return unit.get_component_property(component_id, key);
		}

		return null;
	}

	public bool get_component_property_bool(Guid component_id, string key)
	{
		return (bool)get_component_property(component_id, key);
	}

	public double get_component_property_double(Guid component_id, string key)
	{
		return (double)get_component_property(component_id, key);
	}

	public string get_component_property_string(Guid component_id, string key)
	{
		return (string)get_component_property(component_id, key);
	}

	public Guid get_component_property_guid(Guid component_id, string key)
	{
		return (Guid)get_component_property(component_id, key);
	}

	public Vector3 get_component_property_vector3(Guid component_id, string key)
	{
		return (Vector3)get_component_property(component_id, key);
	}

	public Quaternion get_component_property_quaternion(Guid component_id, string key)
	{
		return (Quaternion)get_component_property(component_id, key);
	}

	public void set_component_property_bool(Guid component_id, string key, bool val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((HashSet<Guid?>)components).contains(component_id)) {
			_db.set_property_bool(component_id, key, val);
			return;
		}

		_db.set_property_bool(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_property_double(Guid component_id, string key, double val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((HashSet<Guid?>)components).contains(component_id)) {
			_db.set_property_double(component_id, key, val);
			return;
		}

		_db.set_property_double(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_property_string(Guid component_id, string key, string val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((HashSet<Guid?>)components).contains(component_id)) {
			_db.set_property_string(component_id, key, val);
			return;
		}

		_db.set_property_string(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_property_guid(Guid component_id, string key, Guid val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((HashSet<Guid?>)components).contains(component_id)) {
			_db.set_property_guid(component_id, key, val);
			return;
		}

		_db.set_property_guid(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_property_vector3(Guid component_id, string key, Vector3 val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((HashSet<Guid?>)components).contains(component_id)) {
			_db.set_property_vector3(component_id, key, val);
			return;
		}

		_db.set_property_vector3(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_property_quaternion(Guid component_id, string key, Quaternion val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((HashSet<Guid?>)components).contains(component_id)) {
			_db.set_property_quaternion(component_id, key, val);
			return;
		}

		_db.set_property_quaternion(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	/// Returns whether the @a unit_id has a component of type @a component_type.
	public static bool has_component_static(out Guid component_id, out Guid owner_id, string component_type, Database db, Guid unit_id)
	{
		Value? val;
		component_id = GUID_ZERO;
		owner_id = GUID_ZERO;
		bool prefab_has_component = false;

		// If the component type is found inside the "components" array, the unit has the component
		// and it owns it.
		val = db.get_property(unit_id, "components");
		if (val != null) {
			foreach (Guid id in (HashSet<Guid?>)val) {
				if ((string)db.object_type(id) == component_type) {
					component_id = id;
					owner_id = unit_id;
					return true;
				}
			}
		}

		// Otherwise, search if any prefab has the component.
		val = db.get_property(unit_id, "prefab");
		if (val != null) {
			// Convert prefab path to object ID.
			string prefab = (string)val;
			Unit.load_unit(db, prefab);
			Guid prefab_id = db.get_property_guid(GUID_ZERO, prefab + ".unit");

			prefab_has_component = has_component_static(out component_id
				, out owner_id
				, component_type
				, db
				, prefab_id
				);
		}

		// If the prefab does not have the component, so does this unit.
		if (prefab_has_component)
			return db.get_property(unit_id, "deleted_components.#" + component_id.to_string()) == null;

		component_id = GUID_ZERO;
		owner_id = GUID_ZERO;
		return false;
	}

	/// Returns whether the unit has the component_type.
	public bool has_component_with_owner(out Guid component_id, out Guid owner_id, string component_type)
	{
		return Unit.has_component_static(out component_id, out owner_id, component_type, _db, _id);
	}

	/// Returns whether the unit has the component_type.
	public bool has_component(out Guid component_id, string component_type)
	{
		Guid owner_id;
		return has_component_with_owner(out component_id, out owner_id, component_type);
	}

	public Vector3 local_position()
	{
		Vector3 position;

		Guid component_id;
		if (has_component(out component_id, OBJECT_TYPE_TRANSFORM))
			position = get_component_property_vector3(component_id, "data.position");
		else
			position = _db.get_property_vector3(_id, "position");

		return position;
	}

	public Quaternion local_rotation()
	{
		Quaternion rotation;

		Guid component_id;
		if (has_component(out component_id, OBJECT_TYPE_TRANSFORM))
			rotation = get_component_property_quaternion(component_id, "data.rotation");
		else
			rotation = _db.get_property_quaternion(_id, "rotation");

		return rotation;
	}

	public Vector3 local_scale()
	{
		Vector3 scale;

		Guid component_id;
		if (has_component(out component_id, OBJECT_TYPE_TRANSFORM))
			scale = get_component_property_vector3(component_id, "data.scale");
		else
			scale = _db.get_property_vector3(_id, "scale");

		return scale;
	}

	public void set_local_position(Vector3 position)
	{
		Guid component_id;
		if (has_component(out component_id, OBJECT_TYPE_TRANSFORM))
			set_component_property_vector3(component_id, "data.position", position);
		else
			_db.set_property_vector3(_id, "position", position);
	}

	public void set_local_rotation(Quaternion rotation)
	{
		Guid component_id;
		if (has_component(out component_id, OBJECT_TYPE_TRANSFORM))
			set_component_property_quaternion(component_id, "data.rotation", rotation);
		else
			_db.set_property_quaternion(_id, "rotation", rotation);
	}

	public void set_local_scale(Vector3 scale)
	{
		Guid component_id;
		if (has_component(out component_id, OBJECT_TYPE_TRANSFORM))
			set_component_property_vector3(component_id, "data.scale", scale);
		else
			_db.set_property_vector3(_id, "scale", scale);
	}

	// Adds the @a component_type to the unit and returns its ID.
	public Guid add_component_type(string component_type)
	{
		// Create a new component.
		Guid component_id = Guid.new_guid();
		_db.create(component_id, component_type);

		// Initialize component data based on its type.
		if (component_type == OBJECT_TYPE_TRANSFORM) {
			_db.set_property_vector3   (component_id, "data.position", VECTOR3_ZERO);
			_db.set_property_quaternion(component_id, "data.rotation", QUATERNION_IDENTITY);
			_db.set_property_vector3   (component_id, "data.scale", VECTOR3_ONE);
		} else if (component_type == OBJECT_TYPE_CAMERA) {
			_db.set_property_string(component_id, "data.projection", "perspective");
			_db.set_property_double(component_id, "data.fov", 45.0 * (Math.PI/180.0));
			_db.set_property_double(component_id, "data.far_range", 0.01);
			_db.set_property_double(component_id, "data.near_range", 1000.0);
		} else if (component_type == OBJECT_TYPE_MESH_RENDERER) {
			_db.set_property_string(component_id, "data.mesh_resource", "core/components/noop");
			_db.set_property_string(component_id, "data.geometry_name", "Noop");
			_db.set_property_string(component_id, "data.material", "core/components/noop");
			_db.set_property_bool  (component_id, "data.visible", true);
		} else if (component_type == OBJECT_TYPE_SPRITE_RENDERER) {
			_db.set_property_string(component_id, "data.sprite_resource", "core/components/noop");
			_db.set_property_string(component_id, "data.material", "core/components/noop");
			_db.set_property_double(component_id, "data.layer", 0);
			_db.set_property_double(component_id, "data.depth", 0);
			_db.set_property_bool  (component_id, "data.visible", true);
		} else if (component_type == OBJECT_TYPE_LIGHT) {
			_db.set_property_string (component_id, "data.type", "directional");
			_db.set_property_double (component_id, "data.range", 1.0);
			_db.set_property_double (component_id, "data.intensity", 1.0);
			_db.set_property_double (component_id, "data.spot_angle", 45.0 * (Math.PI/180.0));
			_db.set_property_vector3(component_id, "data.color", VECTOR3_ONE);
		} else if (component_type == OBJECT_TYPE_SCRIPT) {
			_db.set_property_string(component_id, "data.script_resource", "core/components/noop");
		} else if (component_type == OBJECT_TYPE_COLLIDER) {
			_db.set_property_string    (component_id, "data.shape", "box");
			_db.set_property_string    (component_id, "data.source", "mesh"); // "inline" or "mesh"
			// if "mesh"
			_db.set_property_string    (component_id, "data.scene", "core/components/noop");
			_db.set_property_string    (component_id, "data.name", "Noop");
			// if "inline"
			_db.set_property_vector3   (component_id, "data.collider_data.position", VECTOR3_ZERO);
			_db.set_property_quaternion(component_id, "data.collider_data.rotation", QUATERNION_IDENTITY);
			_db.set_property_vector3   (component_id, "data.collider_data.half_extents", VECTOR3_ZERO); // for "box"
			_db.set_property_double    (component_id, "data.collider_data.radius", 0.0); // for "sphere" and "capsule"
			_db.set_property_double    (component_id, "data.collider_data.height", 0.0); // for "capsule"
		} else if (component_type == OBJECT_TYPE_ACTOR) {
			_db.set_property_bool  (component_id, "data.lock_translation_x", false);
			_db.set_property_bool  (component_id, "data.lock_translation_y", false);
			_db.set_property_bool  (component_id, "data.lock_translation_z", false);
			_db.set_property_bool  (component_id, "data.lock_rotation_x", false);
			_db.set_property_bool  (component_id, "data.lock_rotation_y", false);
			_db.set_property_bool  (component_id, "data.lock_rotation_z", false);
			_db.set_property_string(component_id, "data.class", "static");
			_db.set_property_double(component_id, "data.mass", 1.0);
			_db.set_property_string(component_id, "data.collision_filter", "default");
			_db.set_property_string(component_id, "data.material", "default");
		} else if (component_type == OBJECT_TYPE_ANIMATION_STATE_MACHINE) {
			_db.set_property_string(component_id, "data.state_machine_resource", "core/components/noop");
		} else {
			logw("Unregistered component type `%s`".printf(component_type));
		}

		_db.add_to_set(_id, "components", component_id);
		return component_id;
	}

	/// Removes the @a component_type from the unit.
	public void remove_component_type(string component_type)
	{
		Guid component_id;
		Guid owner_id;
		if (has_component_with_owner(out component_id, out owner_id, component_type)) {
			if (_id == owner_id) {
				_db.remove_from_set(_id, "components", component_id);
			} else {
				_db.set_property_bool(_id, "deleted_components.#" + component_id.to_string(), false);

				// Clean all modified_components keys that matches the deleted component ID.
				string[] unit_keys = _db.get_keys(_id);
				for (int ii = 0; ii < unit_keys.length; ++ii) {
					if (unit_keys[ii].has_prefix("modified_components.#" + component_id.to_string()))
						_db.set_property_null(_id, unit_keys[ii]);
				}
			}

			_db.add_restore_point((int)ActionType.UNIT_REMOVE_COMPONENT, new Guid?[] { _id, component_id });
		} else {
			logw("The unit has no such component type `%s`".printf(component_type));
		}
	}

	public static void register_component_type(string type, string depends_on)
	{
		if (_component_registry == null)
			_component_registry = new Hashtable();
		_component_registry[type] = depends_on;
	}

	public string? prefab()
	{
		return _db.has_property(_id, "prefab")
			? _db.get_property_string(_id, "prefab")
			: null
			;
	}

	/// Returns whether the unit has a prefab.
	public bool has_prefab()
	{
		return _db.has_property(_id, "prefab");
	}

	/// Returns whether the unit is a light unit.
	public bool is_light()
	{
		return has_prefab()
			&& _db.get_property_string(_id, "prefab") == "core/units/light";
	}

	/// Returns whether the unit is a camera unit.
	public bool is_camera()
	{
		return has_prefab()
			&& _db.get_property_string(_id, "prefab") == "core/units/camera";
	}

	public static int generate_spawn_unit_commands(StringBuilder sb, Guid?[] object_ids, Database db)
	{
		int i;

		if (object_ids.length > 1 && Unit.is_component(object_ids[1], db)) {
			for (i = 1; i < object_ids.length; ++i) {
				if (!is_component(object_ids[i], db))
					break;

				Guid unit_id = object_ids[0];
				Guid component_id = object_ids[i];
				string component_type = db.object_type(component_id);
				Unit unit = Unit(db, unit_id);

				sb.append("editor_nv, editor_nq, editor_nm = Device.temp_count()");

				if (component_type == OBJECT_TYPE_TRANSFORM) {
					sb.append(LevelEditorApi.add_tranform_component(unit_id
						, component_id
						, unit.get_component_property_vector3   (component_id, "data.position")
						, unit.get_component_property_quaternion(component_id, "data.rotation")
						, unit.get_component_property_vector3   (component_id, "data.scale")
						));
				} else if (component_type == OBJECT_TYPE_CAMERA) {
					sb.append(LevelEditorApi.add_camera_component(unit_id
						, component_id
						, unit.get_component_property_string(component_id, "data.projection")
						, unit.get_component_property_double(component_id, "data.fov")
						, unit.get_component_property_double(component_id, "data.far_range")
						, unit.get_component_property_double(component_id, "data.near_range")
						));
				} else if (component_type == OBJECT_TYPE_MESH_RENDERER) {
					sb.append(LevelEditorApi.add_mesh_renderer_component(unit_id
						, component_id
						, unit.get_component_property_string(component_id, "data.mesh_resource")
						, unit.get_component_property_string(component_id, "data.geometry_name")
						, unit.get_component_property_string(component_id, "data.material")
						, unit.get_component_property_bool  (component_id, "data.visible")
						));
				} else if (component_type == OBJECT_TYPE_SPRITE_RENDERER) {
					sb.append(LevelEditorApi.add_sprite_renderer_component(unit_id
						, component_id
						, unit.get_component_property_string(component_id, "data.sprite_resource")
						, unit.get_component_property_string(component_id, "data.material")
						, unit.get_component_property_double(component_id, "data.layer")
						, unit.get_component_property_double(component_id, "data.depth")
						, unit.get_component_property_bool  (component_id, "data.visible")
						));
				} else if (component_type == OBJECT_TYPE_LIGHT) {
					sb.append(LevelEditorApi.add_light_component(unit_id
						, component_id
						, unit.get_component_property_string (component_id, "data.type")
						, unit.get_component_property_double (component_id, "data.range")
						, unit.get_component_property_double (component_id, "data.intensity")
						, unit.get_component_property_double (component_id, "data.spot_angle")
						, unit.get_component_property_vector3(component_id, "data.color")
						));
				} else if (component_type == OBJECT_TYPE_SCRIPT) {
					/*
					 * sb.append(LevelEditorApi.add_script_component(unit_id
					 *  , component_id
					 *  , unit.get_component_property_string(component_id, "data.script_resource")
					 *  ));
					 */
				} else if (component_type == OBJECT_TYPE_COLLIDER) {
					/*
					 * sb.append(LevelEditorApi.add_collider_component(unit_id
					 *  , component_id
					 *  , unit.get_component_property_string    (component_id, "data.shape")
					 *  , unit.get_component_property_string    (component_id, "data.source")
					 *  // if "mesh"
					 *  , unit.get_component_property_string    (component_id, "data.scene")
					 *  , unit.get_component_property_string    (component_id, "data.name")
					 *  // if "inline"
					 *  , unit.get_component_property_vector3   (component_id, "data.collider_data.position")
					 *  , unit.get_component_property_quaternion(component_id, "data.collider_data.rotation")
					 *  , unit.get_component_property_vector3   (component_id, "data.collider_data.half_extents")
					 *  , unit.get_component_property_double    (component_id, "data.collider_data.radius")
					 *  , unit.get_component_property_double    (component_id, "data.collider_data.height")
					 *  ));
					 */
				} else if (component_type == OBJECT_TYPE_ACTOR) {
					/*
					 * sb.append(LevelEditorApi.add_actor_component(unit_id
					 *  , component_id
					 *  , unit.get_component_property_bool  (component_id, "data.lock_translation_x")
					 *  , unit.get_component_property_bool  (component_id, "data.lock_translation_y")
					 *  , unit.get_component_property_bool  (component_id, "data.lock_translation_z")
					 *  , unit.get_component_property_bool  (component_id, "data.lock_rotation_x")
					 *  , unit.get_component_property_bool  (component_id, "data.lock_rotation_y")
					 *  , unit.get_component_property_bool  (component_id, "data.lock_rotation_z")
					 *  , unit.get_component_property_string(component_id, "data.class")
					 *  , unit.get_component_property_double(component_id, "data.mass")
					 *  , unit.get_component_property_string(component_id, "data.collision_filter")
					 *  , unit.get_component_property_string(component_id, "data.material")
					 *  ));
					 */
				} else if (component_type == OBJECT_TYPE_ANIMATION_STATE_MACHINE) {
					/*
					 * sb.append(LevelEditorApi.add_animation_state_machine_component(unit_id
					 *  , component_id
					 *  , unit.get_component_property_string(component_id, "data.state_machine_resource")
					 *  ));
					 */
				} else {
					logw("Unregistered component type `%s`".printf(component_type));
				}

				sb.append("Device.set_temp_count(editor_nv, editor_nq, editor_nm)");
			}
		} else {
			for (i = 0; i < object_ids.length; ++i) {
				if (db.object_type(object_ids[i]) != OBJECT_TYPE_UNIT)
					break;

				Guid unit_id = object_ids[i];
				Unit unit = Unit(db, unit_id);
				string? prefab = unit.prefab();

				sb.append("editor_nv, editor_nq, editor_nm = Device.temp_count()");

				if (prefab != null) {
					sb.append(LevelEditorApi.spawn_unit(unit_id
						, prefab
						, unit.local_position()
						, unit.local_rotation()
						, unit.local_scale()
						));

					foreach (var entry in Unit._component_registry.entries) {
						Guid component_id;
						if (!unit.has_component(out component_id, entry.key))
							continue;

						generate_change_commands(sb, { unit_id, component_id }, db);
					}
				} else {
					sb.append(LevelEditorApi.spawn_empty_unit(unit_id));

					Guid component_id;
					if (unit.has_component(out component_id, "transform")) {
						string s = LevelEditorApi.add_tranform_component(unit_id
							, component_id
							, unit.get_component_property_vector3   (component_id, "data.position")
							, unit.get_component_property_quaternion(component_id, "data.rotation")
							, unit.get_component_property_vector3   (component_id, "data.scale")
							);
						sb.append(s);
					}
					if (unit.has_component(out component_id, "camera")) {
						string s = LevelEditorApi.add_camera_component(unit_id
							, component_id
							, unit.get_component_property_string(component_id, "data.projection")
							, unit.get_component_property_double(component_id, "data.fov")
							, unit.get_component_property_double(component_id, "data.far_range")
							, unit.get_component_property_double(component_id, "data.near_range")
							);
						sb.append(s);
					}
					if (unit.has_component(out component_id, "mesh_renderer")) {
						string s = LevelEditorApi.add_mesh_renderer_component(unit_id
							, component_id
							, unit.get_component_property_string(component_id, "data.mesh_resource")
							, unit.get_component_property_string(component_id, "data.geometry_name")
							, unit.get_component_property_string(component_id, "data.material")
							, unit.get_component_property_bool  (component_id, "data.visible")
							);
						sb.append(s);
					}
					if (unit.has_component(out component_id, "sprite_renderer")) {
						string s = LevelEditorApi.add_sprite_renderer_component(unit_id
							, component_id
							, unit.get_component_property_string(component_id, "data.sprite_resource")
							, unit.get_component_property_string(component_id, "data.material")
							, unit.get_component_property_double(component_id, "data.layer")
							, unit.get_component_property_double(component_id, "data.depth")
							, unit.get_component_property_bool  (component_id, "data.visible")
							);
						sb.append(s);
					}
					if (unit.has_component(out component_id, "light")) {
						string s = LevelEditorApi.add_light_component(unit_id
							, component_id
							, unit.get_component_property_string (component_id, "data.type")
							, unit.get_component_property_double (component_id, "data.range")
							, unit.get_component_property_double (component_id, "data.intensity")
							, unit.get_component_property_double (component_id, "data.spot_angle")
							, unit.get_component_property_vector3(component_id, "data.color")
							);
						sb.append(s);
					}
				}

				sb.append("Device.set_temp_count(editor_nv, editor_nq, editor_nm)");
			}
		}

		return i;
	}

	public static int generate_destroy_commands(StringBuilder sb, Guid?[] object_ids, Database db)
	{
		int i;

		if (object_ids.length > 1 && Unit.is_component(object_ids[1], db)) {
			for (i = 1; i < object_ids.length; ++i) {
				if (!is_component(object_ids[i], db))
					break;

				Guid unit_id = object_ids[0];
				Guid component_id = object_ids[i];
				string component_type = db.object_type(component_id);

				sb.append(LevelEditorApi.unit_destroy_component_type(unit_id, component_type));
			}
		} else {
			for (i = 0; i < object_ids.length; ++i) {
				if (db.object_type(object_ids[i]) != OBJECT_TYPE_UNIT)
					break;

				sb.append(LevelEditorApi.destroy(object_ids[i]));
			}
		}

		return i;
	}

	public static int generate_change_commands(StringBuilder sb, Guid?[] object_ids, Database db)
	{
		int i;

		if (object_ids.length > 1 && Unit.is_component(object_ids[1], db)) {
			for (i = 1; i < object_ids.length; ++i) {
				if (!is_component(object_ids[i], db))
					break;

				Guid unit_id = object_ids[0];
				Guid component_id = object_ids[i];
				string component_type = db.object_type(component_id);
				Unit unit = Unit(db, unit_id);

				sb.append("editor_nv, editor_nq, editor_nm = Device.temp_count()");

				if (component_type == OBJECT_TYPE_TRANSFORM) {
					sb.append(LevelEditorApi.move_object(unit_id
						, unit.get_component_property_vector3   (component_id, "data.position")
						, unit.get_component_property_quaternion(component_id, "data.rotation")
						, unit.get_component_property_vector3   (component_id, "data.scale")
						));
				} else if (component_type == OBJECT_TYPE_CAMERA) {
					sb.append(LevelEditorApi.set_camera(unit_id
						, unit.get_component_property_string(component_id, "data.projection")
						, unit.get_component_property_double(component_id, "data.fov")
						, unit.get_component_property_double(component_id, "data.far_range")
						, unit.get_component_property_double(component_id, "data.near_range")
						));
				} else if (component_type == OBJECT_TYPE_MESH_RENDERER) {
					sb.append(LevelEditorApi.set_mesh(unit_id
						, unit.get_component_property_string(component_id, "data.mesh_resource")
						, unit.get_component_property_string(component_id, "data.geometry_name")
						, unit.get_component_property_string(component_id, "data.material")
						, unit.get_component_property_bool  (component_id, "data.visible")
						));
				} else if (component_type == OBJECT_TYPE_SPRITE_RENDERER) {
					sb.append(LevelEditorApi.set_sprite(unit_id
						, unit.get_component_property_string(component_id, "data.sprite_resource")
						, unit.get_component_property_string(component_id, "data.material")
						, unit.get_component_property_double(component_id, "data.layer")
						, unit.get_component_property_double(component_id, "data.depth")
						, unit.get_component_property_bool  (component_id, "data.visible")
						));
				} else if (component_type == OBJECT_TYPE_LIGHT) {
					sb.append(LevelEditorApi.set_light(unit_id
						, unit.get_component_property_string (component_id, "data.type")
						, unit.get_component_property_double (component_id, "data.range")
						, unit.get_component_property_double (component_id, "data.intensity")
						, unit.get_component_property_double (component_id, "data.spot_angle")
						, unit.get_component_property_vector3(component_id, "data.color")
						));
				} else if (component_type == OBJECT_TYPE_SCRIPT) {
					/* No sync. */
				} else if (component_type == OBJECT_TYPE_COLLIDER) {
					/* No sync. */
				} else if (component_type == OBJECT_TYPE_ACTOR) {
					/* No sync. */
				} else if (component_type == OBJECT_TYPE_ANIMATION_STATE_MACHINE) {
					/* No sync. */
				} else {
					logw("Unregistered component type `%s`".printf(component_type));
				}

				sb.append("Device.set_temp_count(editor_nv, editor_nq, editor_nm)");
			}
		} else {
			for (i = 0; i < object_ids.length; ++i) {
				if (db.object_type(object_ids[i]) != OBJECT_TYPE_UNIT)
					break;

				Guid unit_id = object_ids[i];
				Unit unit = Unit(db, unit_id);

				sb.append("editor_nv, editor_nq, editor_nm = Device.temp_count()");
				sb.append(LevelEditorApi.move_object(unit_id
					, unit.local_position()
					, unit.local_rotation()
					, unit.local_scale()
					));
				sb.append("Device.set_temp_count(editor_nv, editor_nq, editor_nm)");
			}
		}

		return i;
	}

	public static bool is_component(Guid id, Database db)
	{
		string type = db.object_type(id);

		return type == OBJECT_TYPE_TRANSFORM
			|| type == OBJECT_TYPE_CAMERA
			|| type == OBJECT_TYPE_MESH_RENDERER
			|| type == OBJECT_TYPE_SPRITE_RENDERER
			|| type == OBJECT_TYPE_LIGHT
			|| type == OBJECT_TYPE_SCRIPT
			|| type == OBJECT_TYPE_COLLIDER
			|| type == OBJECT_TYPE_ACTOR
			|| type == OBJECT_TYPE_ANIMATION_STATE_MACHINE
			;
	}

	public void add_component_type_dependencies(ref ArrayList<Guid?> components_added, string component_type)
	{
		Guid dummy;
		if (has_component(out dummy, component_type))
			return;

		string[] component_type_dependencies = ((string)Unit._component_registry[component_type]).split(", ");
		foreach (unowned string dependency in component_type_dependencies) {
			Guid dependency_component_id;
			if (!has_component(out dependency_component_id, dependency))
				add_component_type_dependencies(ref components_added, dependency);
		}

		components_added.add(add_component_type(component_type));
	}
}

} /* namespace Crown */
