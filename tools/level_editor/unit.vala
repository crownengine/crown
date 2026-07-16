/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
private enum ObjectExists
{
	UNKNOWN,
	MISSING,
	EXISTS
}

public struct Unit
{
	public static GLib.HashTable<string, Value?> _component_registry;
	public Database _db;
	public Guid _id;

	public Unit(Database db, Guid id)
	{
		_db = db;
		_id = id;
	}

	/// Loads the unit @a name.
	public static LoadError load_unit(out Guid prefab_id, Database db, string name)
	{
		return db.add_from_resource_path(out prefab_id, name + ".unit");
	}

	private static ObjectExists component_exists_internal(Database db, Guid unit_id, Guid component_id, bool apply_unit_deletes, GLib.GenericSet<Guid?> visited = new GLib.GenericSet<Guid?>(Guid.hash_func, Guid.equal_func))
	{
		if (!visited.add(unit_id))
			return ObjectExists.UNKNOWN;

		ObjectExists exists = ObjectExists.MISSING;
		foreach (unowned Guid? local_component_id in db.get_set(unit_id, "components")) {
			if (Guid.equal_func(local_component_id, component_id)) {
				exists = ObjectExists.EXISTS;
				break;
			}
		}

		if (exists == ObjectExists.MISSING) {
			string? prefab = db.get_resource(unit_id, "prefab");
			if (prefab != null) {
				Guid prefab_id;
				if (Unit.load_unit(out prefab_id, db, prefab) != LoadError.SUCCESS)
					return ObjectExists.UNKNOWN;

				exists = component_exists_internal(db, prefab_id, component_id, true, visited);
			}
		}

		if (exists == ObjectExists.EXISTS
			&& apply_unit_deletes
			&& db.get_property(unit_id, "deleted_components.#" + component_id.to_string()) != null
			)
			exists = ObjectExists.MISSING;

		return exists;
	}

	private static ObjectExists child_exists_internal(Database db, Guid unit_id, Guid child_id, bool apply_unit_deletes, GLib.GenericSet<Guid?> visited = new GLib.GenericSet<Guid?>(Guid.hash_func, Guid.equal_func))
	{
		if (!visited.add(unit_id))
			return ObjectExists.UNKNOWN;

		ObjectExists exists = ObjectExists.MISSING;
		Guid?[] children = db.get_set(unit_id, "children");
		foreach (unowned Guid? local_child_id in children) {
			if (local_child_id == child_id) {
				exists = ObjectExists.EXISTS;
				break;
			}

			ObjectExists child_exists = child_exists_internal(db, local_child_id, child_id, true, visited);
			if (child_exists == ObjectExists.UNKNOWN)
				return ObjectExists.UNKNOWN;
			if (child_exists == ObjectExists.EXISTS) {
				exists = ObjectExists.EXISTS;
				break;
			}
		}

		if (exists == ObjectExists.MISSING) {
			string? prefab = db.get_resource(unit_id, "prefab");
			if (prefab != null) {
				Guid prefab_id;
				if (Unit.load_unit(out prefab_id, db, prefab) != LoadError.SUCCESS)
					return ObjectExists.UNKNOWN;

				exists = child_exists_internal(db, prefab_id, child_id, true, visited);
			}
		}

		if (exists == ObjectExists.EXISTS && apply_unit_deletes) {
			Guid?[] deleted_children = db.get_set(unit_id, "deleted_children");
			foreach (unowned Guid? deleted_child_id in deleted_children) {
				if (deleted_child_id == child_id) {
					exists = ObjectExists.MISSING;
					break;
				}

				ObjectExists deleted_subtree_contains_child = child_exists_internal(db, deleted_child_id, child_id, true);
				if (deleted_subtree_contains_child == ObjectExists.UNKNOWN)
					return ObjectExists.UNKNOWN;
				if (deleted_subtree_contains_child == ObjectExists.EXISTS) {
					exists = ObjectExists.MISSING;
					break;
				}
			}
		}

		return exists;
	}

	public void prune_stale_overrides()
	{
		string[] unit_keys = _db.get_keys(_id);
		foreach (unowned string key in unit_keys) {
			if (key.has_prefix("deleted_components.#")
				&& key.length == "deleted_components.#".length + 36
				) {
				Guid component_id = Guid.parse(key.substring("deleted_components.#".length, 36));
				if (component_exists_internal(_db, _id, component_id, false) == ObjectExists.MISSING)
					_db.set_null(_id, key);
			} else if (key.has_prefix("modified_components.#")
				&& key.length > "modified_components.#".length + 36
				&& key["modified_components.#".length + 36] == '.'
				) {
				Guid component_id = Guid.parse(key.substring("modified_components.#".length, 36));
				if (component_exists_internal(_db, _id, component_id, true) == ObjectExists.MISSING)
					_db.set_null(_id, key);
			}
		}

		prune_stale_child_override_set("deleted_children", false);
		prune_stale_child_override_set("modified_children", true);
	}

	private void prune_stale_child_override_set(string key, bool apply_unit_deletes)
	{
		foreach (unowned Guid? child_id in _db.get_set(_id, key)) {
			if (child_exists_internal(_db, _id, child_id, apply_unit_deletes) == ObjectExists.MISSING)
				_db.remove_from_set(_id, key, child_id);
		}
	}

	public void create_empty()
	{
		_db.create(_id, OBJECT_TYPE_UNIT);
	}

	public int create(string? prefab)
	{
		create_empty();
		return prefab == null ? 0 : set_prefab(prefab);
	}

	public Value? get_component_property(Guid component_id, string key, Value? deffault = null)
	{
		assert(component_exists_internal(_db, _id, component_id, true) == ObjectExists.EXISTS);

		Value? val;

		// Search in components
		val = _db.get_property(_id, "components");
		if (val != null) {
			if (((GLib.GenericSet<Guid?>)val).contains(component_id))
				return _db.get_property(component_id, key, deffault);
		}

		// Search in modified_components
		val = _db.get_property(_id, "modified_components.#" + component_id.to_string() + "." + key);
		if (val != null)
			return val;

		// Search in prefab
		string? prefab = prefab();
		if (prefab != null) {
			// Convert prefab path to object ID.
			Guid prefab_id = GUID_ZERO;
			Unit.load_unit(out prefab_id, _db, prefab);

			Unit unit = Unit(_db, prefab_id);
			return unit.get_component_property(component_id, key, deffault);
		}

		return deffault;
	}

	public bool get_component_bool(Guid component_id, string key, bool deffault = false)
	{
		return (bool)get_component_property(component_id, key, deffault);
	}

	public double get_component_double(Guid component_id, string key, double deffault = 0.0)
	{
		return (double)get_component_property(component_id, key, deffault);
	}

	public string get_component_string(Guid component_id, string key, string deffault = "")
	{
		return (string)get_component_property(component_id, key, deffault);
	}

	public Vector3 get_component_vector3(Guid component_id, string key, Vector3 deffault = VECTOR3_ZERO)
	{
		return (Vector3)get_component_property(component_id, key, deffault);
	}

	public Quaternion get_component_quaternion(Guid component_id, string key, Quaternion deffault = QUATERNION_IDENTITY)
	{
		return (Quaternion)get_component_property(component_id, key, deffault);
	}

	public string? get_component_resource(Guid component_id, string key, string? deffault = null)
	{
		Resource deffault_res = { deffault };
		Value? val = get_component_property(component_id, key, deffault_res);
		if (val.holds(typeof(Resource)))
			return ((Resource)val).name;
		return (string?)val;
	}

	public Guid get_component_reference(Guid component_id, string key, Guid deffault = GUID_ZERO)
	{
		return (Guid)get_component_property(component_id, key, deffault);
	}

	public void set_component_bool(Guid component_id, string key, bool val)
	{
		assert(component_exists_internal(_db, _id, component_id, true) == ObjectExists.EXISTS);

		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((GLib.GenericSet<Guid?>)components).contains(component_id)) {
			_db.set_bool(component_id, key, val);
			return;
		}

		_db.set_bool(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_double(Guid component_id, string key, double val)
	{
		assert(component_exists_internal(_db, _id, component_id, true) == ObjectExists.EXISTS);

		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((GLib.GenericSet<Guid?>)components).contains(component_id)) {
			_db.set_double(component_id, key, val);
			return;
		}

		_db.set_double(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_string(Guid component_id, string key, string val)
	{
		assert(component_exists_internal(_db, _id, component_id, true) == ObjectExists.EXISTS);

		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((GLib.GenericSet<Guid?>)components).contains(component_id)) {
			_db.set_string(component_id, key, val);
			return;
		}

		_db.set_string(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_vector3(Guid component_id, string key, Vector3 val)
	{
		assert(component_exists_internal(_db, _id, component_id, true) == ObjectExists.EXISTS);

		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((GLib.GenericSet<Guid?>)components).contains(component_id)) {
			_db.set_vector3(component_id, key, val);
			return;
		}

		_db.set_vector3(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_quaternion(Guid component_id, string key, Quaternion val)
	{
		assert(component_exists_internal(_db, _id, component_id, true) == ObjectExists.EXISTS);

		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((GLib.GenericSet<Guid?>)components).contains(component_id)) {
			_db.set_quaternion(component_id, key, val);
			return;
		}

		_db.set_quaternion(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_resource(Guid component_id, string key, string? val)
	{
		assert(component_exists_internal(_db, _id, component_id, true) == ObjectExists.EXISTS);

		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((GLib.GenericSet<Guid?>)components).contains(component_id)) {
			_db.set_resource(component_id, key, val);
			return;
		}

		_db.set_resource(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_reference(Guid component_id, string key, Guid val)
	{
		assert(component_exists_internal(_db, _id, component_id, true) == ObjectExists.EXISTS);

		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((GLib.GenericSet<Guid?>)components).contains(component_id)) {
			_db.set_reference(component_id, key, val);
			return;
		}

		_db.set_reference(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	/// Returns whether the @a unit_id has a component of type @a component_type.
	public static bool has_component_static(out Guid component_id, string component_type, Database db, Guid unit_id)
	{
		Value? val;
		component_id = GUID_ZERO;
		bool prefab_has_component = false;

		// If the component type is found inside the "components" array, the unit has the component
		// and it owns it.
		val = db.get_property(unit_id, "components");
		if (val != null) {
			foreach (Guid? id in (GLib.GenericSet<Guid?>)val) {
				if ((string)db.object_type(id) == component_type) {
					component_id = id;
					return true;
				}
			}
		}

		// Otherwise, search if any prefab has the component.
		string? prefab = db.get_resource(unit_id, "prefab");
		if (prefab != null) {
			// Convert prefab path to object ID.
			Guid prefab_id = GUID_ZERO;
			Unit.load_unit(out prefab_id, db, prefab);

			prefab_has_component = has_component_static(out component_id
				, component_type
				, db
				, prefab_id
				);
		}

		// If the prefab does not have the component, so does this unit.
		if (prefab_has_component)
			return db.get_property(unit_id, "deleted_components.#" + component_id.to_string()) == null;

		component_id = GUID_ZERO;
		return false;
	}

	/// Returns whether the unit has the component_type.
	public bool has_component(out Guid component_id, string component_type)
	{
		return Unit.has_component_static(out component_id, component_type, _db, _id);
	}

	public Vector3 local_position()
	{
		Vector3 position;

		Guid component_id;
		if (has_component(out component_id, OBJECT_TYPE_TRANSFORM))
			position = get_component_vector3(component_id, "data.position");
		else
			position = _db.get_vector3(_id, "position");

		return position;
	}

	public Quaternion local_rotation()
	{
		Quaternion rotation;

		Guid component_id;
		if (has_component(out component_id, OBJECT_TYPE_TRANSFORM))
			rotation = get_component_quaternion(component_id, "data.rotation");
		else
			rotation = _db.get_quaternion(_id, "rotation");

		return rotation;
	}

	public Vector3 local_scale()
	{
		Vector3 scale;

		Guid component_id;
		if (has_component(out component_id, OBJECT_TYPE_TRANSFORM))
			scale = get_component_vector3(component_id, "data.scale");
		else
			scale = _db.get_vector3(_id, "scale", VECTOR3_ONE);

		return scale;
	}

	public void set_local_position(Vector3 position)
	{
		Guid component_id;
		if (has_component(out component_id, OBJECT_TYPE_TRANSFORM))
			set_component_vector3(component_id, "data.position", position);
		else
			_db.set_vector3(_id, "position", position);
	}

	public void set_local_rotation(Quaternion rotation)
	{
		Guid component_id;
		if (has_component(out component_id, OBJECT_TYPE_TRANSFORM))
			set_component_quaternion(component_id, "data.rotation", rotation);
		else
			_db.set_quaternion(_id, "rotation", rotation);
	}

	public void set_local_scale(Vector3 scale)
	{
		Guid component_id;
		if (has_component(out component_id, OBJECT_TYPE_TRANSFORM))
			set_component_vector3(component_id, "data.scale", scale);
		else
			_db.set_vector3(_id, "scale", scale);
	}

	// Adds the @a component_type to the unit and returns its ID.
	public Guid add_component_type(string component_type)
	{
		// Create a new component.
		Guid component_id = Guid.new_guid();
		_db.create(component_id, component_type);
		_db.add_to_set(_id, "components", component_id);
		return component_id;
	}

	/// Removes the @a component_type from the unit.
	public void remove_component_type(string component_type)
	{
		Guid component_id;
		if (has_component(out component_id, component_type)) {
			if (_id == _db.owner(component_id)) {
				_db.remove_from_set(_id, "components", component_id);
				_db.destroy(component_id);
				_db.add_restore_point((int)ActionType.CHANGE_OBJECTS, { _id });
			} else {
				_db.set_bool(_id, "deleted_components.#" + component_id.to_string(), false);

				// Clean all modified_components keys that matches the deleted component ID.
				string[] unit_keys = _db.get_keys(_id);
				for (int ii = 0; ii < unit_keys.length; ++ii) {
					if (unit_keys[ii].has_prefix("modified_components.#" + component_id.to_string()))
						_db.set_null(_id, unit_keys[ii]);
				}
				_db.add_restore_point((int)ActionType.CHANGE_OBJECTS, { _id });
			}
		} else {
			logw("The unit has no such component type `%s`".printf(component_type));
		}
	}

	public static void register_component_type(string type, string depends_on)
	{
		if (_component_registry == null)
			_component_registry = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
		_component_registry[type] = depends_on;
	}

	public string? prefab()
	{
		return _db.get_resource(_id, "prefab");
	}

	/// Returns whether the unit has a prefab.
	public bool has_prefab()
	{
		return prefab() != null;
	}

	public int can_set_prefab(string? prefab_name)
	{
		if (prefab_name == null)
			return 0;

		Database validation_db = new Database(_db._project);
		create_object_types(validation_db);

		Guid prefab_id = GUID_ZERO;
		if (Unit.load_unit(out prefab_id, validation_db, prefab_name) != LoadError.SUCCESS) {
			loge("Failed to load prefab `%s`".printf(prefab_name));
			return -1;
		}

		GLib.GenericSet<Guid?> visited = new GLib.GenericSet<Guid?>(Guid.hash_func, Guid.equal_func);
		while (true) {
			if (Guid.equal_func(prefab_id, _id) || visited.contains(prefab_id)) {
				loge("Cannot set prefab `%s`: prefab cycle detected".printf(prefab_name));
				return -1;
			}
			visited.add(prefab_id);

			string? prefab = validation_db.get_resource(prefab_id, "prefab");
			if (prefab == null)
				return 0;

			Guid inherited_id = GUID_ZERO;
			if (Unit.load_unit(out inherited_id, validation_db, prefab) != LoadError.SUCCESS) {
				loge("Cannot set prefab `%s`: failed to load inherited prefab `%s`".printf(prefab_name, prefab));
				return -1;
			}

			prefab_id = inherited_id;
		}
	}

	public int set_prefab(string? prefab_name)
	{
		if (prefab() == prefab_name)
			return -1;

		if (can_set_prefab(prefab_name) != 0)
			return -1;

		Guid transform_id;
		bool has_transform = has_component(out transform_id, OBJECT_TYPE_TRANSFORM);
		bool restore_position = has_transform || _db.has_property(_id, "position");
		bool restore_rotation = has_transform || _db.has_property(_id, "rotation");
		bool restore_scale = has_transform || _db.has_property(_id, "scale");

		// Keep authored transform channels stable without turning defaults into overrides.
		Vector3 unit_pos = restore_position ? local_position() : VECTOR3_ZERO;
		Quaternion unit_rot = restore_rotation ? local_rotation() : QUATERNION_IDENTITY;
		Vector3 unit_scl = restore_scale ? local_scale() : VECTOR3_ONE;

		_db.set_resource(_id, "prefab", prefab_name);

		// Drop overrides that belonged to the previous prefab hierarchy.
		foreach (unowned string key in _db.get_keys(_id)) {
			if (key.has_prefix("modified_components.")
				|| key.has_prefix("deleted_components.")
				|| key.has_prefix("modified_children.")
				|| key.has_prefix("deleted_children.")
				)
				_db.set_null(_id, key);
		}

		// Remove owned components already provided by the new prefab.
		if (prefab_name != null) {
			Guid prefab_id = GUID_ZERO;
			if (Unit.load_unit(out prefab_id, _db, prefab_name) == LoadError.SUCCESS) {
				foreach (unowned Guid? component_id in _db.get_set(_id, "components")) {
					Guid prefab_component_id;
					if (!Unit.has_component_static(out prefab_component_id, _db.object_type(component_id), _db, prefab_id))
						continue;

					_db.remove_from_set(_id, "components", component_id);
					_db.destroy(component_id);
				}
			}
		}

		if (restore_position)
			set_local_position(unit_pos);
		if (restore_rotation)
			set_local_rotation(unit_rot);
		if (restore_scale)
			set_local_scale(unit_scl);

		return 0;
	}

	/// Returns whether the unit is a light unit.
	public bool is_light()
	{
		return has_prefab()
			&& _db.get_resource(_id, "prefab") == "core/units/light";
	}

	/// Returns whether the unit is a camera unit.
	public bool is_camera()
	{
		return has_prefab()
			&& _db.get_resource(_id, "prefab") == "core/units/camera";
	}

	public static void generate_add_component_commands(StringBuilder sb, Guid unit_id, Guid component_id, Database db)
	{
		Unit unit = Unit(db, unit_id);

		if (db.object_type(component_id) == OBJECT_TYPE_TRANSFORM) {
			string s = LevelEditorApi.add_tranform_component(unit_id
				, component_id
				, unit.get_component_vector3   (component_id, "data.position")
				, unit.get_component_quaternion(component_id, "data.rotation")
				, unit.get_component_vector3   (component_id, "data.scale")
				);
			sb.append(s);
		} else if (db.object_type(component_id) == OBJECT_TYPE_CAMERA) {
			string s = LevelEditorApi.add_camera_component(unit_id
				, component_id
				, unit.get_component_string(component_id, "data.projection")
				, unit.get_component_double(component_id, "data.fov")
				, unit.get_component_double(component_id, "data.far_range")
				, unit.get_component_double(component_id, "data.near_range")
				);
			sb.append(s);
		} else if (db.object_type(component_id) == OBJECT_TYPE_MESH_RENDERER) {
			string s = LevelEditorApi.add_mesh_renderer_component(unit_id
				, component_id
				, unit.get_component_resource(component_id, "data.mesh_resource")
				, unit.get_component_string(component_id, "data.geometry_name")
				, unit.get_component_resource(component_id, "data.material")
				, unit.get_component_bool  (component_id, "data.visible")
				, unit.get_component_bool  (component_id, "data.cast_shadows", true)
				);
			sb.append(s);
		} else if (db.object_type(component_id) == OBJECT_TYPE_SPRITE_RENDERER) {
			string s = LevelEditorApi.add_sprite_renderer_component(unit_id
				, component_id
				, unit.get_component_resource(component_id, "data.sprite_resource")
				, unit.get_component_resource(component_id, "data.material")
				, unit.get_component_double(component_id, "data.layer")
				, unit.get_component_double(component_id, "data.depth")
				, unit.get_component_bool  (component_id, "data.visible")
				, unit.get_component_bool  (component_id, "data.flip_x")
				, unit.get_component_bool  (component_id, "data.flip_y")
				);
			sb.append(s);
		} else if (db.object_type(component_id) == OBJECT_TYPE_LIGHT) {
			string s = LevelEditorApi.add_light_component(unit_id
				, component_id
				, unit.get_component_string (component_id, "data.type")
				, unit.get_component_double (component_id, "data.range")
				, unit.get_component_double (component_id, "data.intensity")
				, unit.get_component_double (component_id, "data.spot_angle")
				, unit.get_component_vector3(component_id, "data.color")
				, unit.get_component_double (component_id, "data.shadow_bias", 0.0001)
				, unit.get_component_bool   (component_id, "data.cast_shadows", true)
				);
			sb.append(s);
		} else if (db.object_type(component_id) == OBJECT_TYPE_ANIMATION_STATE_MACHINE) {
			string s = LevelEditorApi.add_animation_state_machine_component(unit_id
				, component_id
				, unit.get_component_resource(component_id, "data.state_machine_resource")
				);
			sb.append(s);
		} else if (db.object_type(component_id) == OBJECT_TYPE_MOVER) {
			sb.append(LevelEditorApi.add_mover_component(unit_id
				, component_id
				, unit.get_component_double(component_id, "data.height")
				, unit.get_component_double(component_id, "data.radius")
				, unit.get_component_double(component_id, "data.max_slope_angle")
				, unit.get_component_string(component_id, "data.collision_filter")
				));
		} else if (db.object_type(component_id) == OBJECT_TYPE_FIXED_JOINT
			|| db.object_type(component_id) == OBJECT_TYPE_HINGE_JOINT
			|| db.object_type(component_id) == OBJECT_TYPE_SPHERICAL_JOINT
			|| db.object_type(component_id) == OBJECT_TYPE_SPRING_JOINT
			|| db.object_type(component_id) == OBJECT_TYPE_LIMB_JOINT
			|| db.object_type(component_id) == OBJECT_TYPE_D6_JOINT) {
			Guid other_actor_unit_id = unit.get_component_reference(component_id, "data.other_actor");
			if (other_actor_unit_id != GUID_ZERO && !db.is_alive(other_actor_unit_id))
				other_actor_unit_id = GUID_ZERO;
			sb.append(LevelEditorApi.add_joint_component(unit_id
				, component_id
				, db.object_type(component_id)
				, unit.get_component_vector3(component_id, "data.position", VECTOR3_ZERO)
				, unit.get_component_quaternion(component_id, "data.rotation", QUATERNION_IDENTITY)
				, other_actor_unit_id
				, unit.get_component_vector3(component_id, "data.other_position", VECTOR3_ZERO)
				, unit.get_component_quaternion(component_id, "data.other_rotation", QUATERNION_IDENTITY)
				));
		} else if (db.object_type(component_id) == OBJECT_TYPE_LOD_GROUP) {
			Guid?[] lod_levels = db.get_set(component_id, "data.lod_levels");
			GLib.qsort_with_data<Guid?>(lod_levels, sizeof(Guid?), (a, b) => {
					double screen_size_a = db.get_double(a, "data.screen_size");
					double screen_size_b = db.get_double(b, "data.screen_size");
					return screen_size_a > screen_size_b ? -1 : (screen_size_a < screen_size_b ? 1 : 0);
				});

			Guid[] mesh_renderer_ids = new Guid[lod_levels.length];
			double[] screen_sizes = new double[lod_levels.length];
			for (int i = 0; i < lod_levels.length; ++i) {
				mesh_renderer_ids[i] = db.get_reference(lod_levels[i], "data.mesh_renderer");
				screen_sizes[i] = db.get_double(lod_levels[i], "data.screen_size");
			}

			sb.append(LevelEditorApi.add_lod_group_component(unit_id
				, component_id
				, unit.get_component_double(component_id, "data.level", -1.0)
				, unit.get_component_string(component_id, "data.fade_mode", "none")
				, mesh_renderer_ids
				, screen_sizes
				));
		} else if (db.object_type(component_id) == OBJECT_TYPE_FOG) {
			sb.append(LevelEditorApi.add_fog_component(unit_id, component_id));
			sb.append(LevelEditorApi.set_fog(unit_id
				, unit.get_component_vector3(component_id, "data.color")
				, unit.get_component_double (component_id, "data.density")
				, unit.get_component_double (component_id, "data.range_min")
				, unit.get_component_double (component_id, "data.range_max")
				, unit.get_component_double (component_id, "data.sun_blend")
				, unit.get_component_bool   (component_id, "data.enabled")
				));
		} else if (db.object_type(component_id) == OBJECT_TYPE_GLOBAL_LIGHTING) {
			sb.append(LevelEditorApi.add_global_lighting_component(unit_id, component_id));
			sb.append(LevelEditorApi.set_global_lighting(unit_id
				, unit.get_component_resource (component_id, "data.skydome_map")
				, unit.get_component_double (component_id, "data.skydome_intensity")
				, unit.get_component_vector3(component_id, "data.ambient_color")
				));
		} else if (db.object_type(component_id) == OBJECT_TYPE_BLOOM) {
			sb.append(LevelEditorApi.add_bloom_component(unit_id, component_id));
			sb.append(LevelEditorApi.set_bloom(unit_id
				, unit.get_component_bool  (component_id, "data.enabled")
				, unit.get_component_double(component_id, "data.threshold")
				, unit.get_component_double(component_id, "data.weight")
				, unit.get_component_double(component_id, "data.intensity")
				));
		} else if (db.object_type(component_id) == OBJECT_TYPE_TONEMAP) {
			sb.append(LevelEditorApi.add_tonemap_component(unit_id, component_id));
			sb.append(LevelEditorApi.set_tonemap(unit_id
				, unit.get_component_string(component_id, "data.type")
				));
		}
	}

	public static void collect_unit_tree(GLib.GenericArray<Guid?> unit_ids, Guid unit_id, Database db)
	{
		unit_ids.add(unit_id);

		if (db.has_property(unit_id, "children")) {
			Guid?[] children = db.get_set(unit_id, "children");
			foreach (unowned Guid? child_id in children) {
				if (db.is_alive(child_id))
					collect_unit_tree(unit_ids, child_id, db);
			}
		}
	}

	public static int compare_component_spawn_order(Database db, Guid? component_a, Guid? component_b)
	{
		double order_a = db.get_double(component_a, "spawn_order");
		double order_b = db.get_double(component_b, "spawn_order");

		if (order_a < order_b)
			return -1;
		else if (order_a > order_b)
			return 1;
		return 0;
	}

	public static void spawn_unit_tree(StringBuilder sb, Guid unit_id, Database db)
	{
		GLib.GenericArray<Guid?> unit_ids = new GLib.GenericArray<Guid?>();
		GLib.GenericArray<Guid?> components = new GLib.GenericArray<Guid?>();
		collect_unit_tree(unit_ids, unit_id, db);

		sb.append("editor_tree_nv, editor_tree_nq, editor_tree_nm = Device.temp_count()");

		for (int i = 0; i < unit_ids.length; ++i) {
			Guid id = unit_ids[i];
			Unit unit = Unit(db, id);
			if (unit.prefab() != null) {
				spawn_unit(sb, id, db);
			} else {
				sb.append(LevelEditorApi.spawn_empty_unit(id));
				Guid?[] unit_components = db.get_set(id, "components");
				foreach (unowned Guid? component_id in unit_components)
					components.add(component_id);
			}
		}

		components.sort_with_data((a, b) => {
				return compare_component_spawn_order(db, a, b);
			});

		for (int i = 0; i < components.length; ++i) {
			Guid component_id = components[i];
			if (db.object_type(component_id) == OBJECT_TYPE_TRANSFORM)
				generate_add_component_commands(sb, db.owner(component_id), component_id, db);
		}

		for (int i = 0; i < unit_ids.length; ++i) {
			Guid id = unit_ids[i];
			Guid owner_id = db.owner(id);
			if (owner_id != GUID_ZERO && db.object_type(owner_id) == OBJECT_TYPE_UNIT)
				sb.append(LevelEditorApi.unit_set_parent(owner_id, id));
		}

		for (int i = 0; i < components.length; ++i) {
			Guid component_id = components[i];
			if (db.object_type(component_id) != OBJECT_TYPE_TRANSFORM)
				generate_add_component_commands(sb, db.owner(component_id), component_id, db);
		}

		for (int i = 0; i < unit_ids.length; ++i) {
			Guid id = unit_ids[i];
			Unit unit = Unit(db, id);
			if (unit.prefab() == null) {
				sb.append(LevelEditorApi.object_set_hidden(id, db.get_bool(id, Level.OBJECT_HIDDEN_KEY, false)));
				sb.append(LevelEditorApi.object_set_selectable(id, !db.get_bool(id, Level.OBJECT_LOCKED_KEY, false)));
			}
		}

		sb.append("Device.set_temp_count(editor_tree_nv, editor_tree_nq, editor_tree_nm)");
	}

	public static void spawn_unit(StringBuilder sb, Guid unit_id, Database db)
	{
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
			generate_change_commands(sb, { unit_id }, db);
		} else {
			sb.append(LevelEditorApi.spawn_empty_unit(unit_id));

			Guid?[] components = db.get_set(unit_id, "components");
			GLib.qsort_with_data<Guid?>(components, sizeof(Guid?), (a, b) => {
					double order_a = db.get_double(a, "spawn_order");
					double order_b = db.get_double(b, "spawn_order");
					return (int)(order_a - order_b);
				});

			foreach (unowned Guid? component_id in components)
				generate_add_component_commands(sb, unit_id, component_id, db);

			sb.append(LevelEditorApi.object_set_hidden(unit_id, db.get_bool(unit_id, Level.OBJECT_HIDDEN_KEY, false)));
			sb.append(LevelEditorApi.object_set_selectable(unit_id, !db.get_bool(unit_id, Level.OBJECT_LOCKED_KEY, false)));
		}

		sb.append("Device.set_temp_count(editor_nv, editor_nq, editor_nm)");
	}

	public static bool generate_lod_group_subobject_commands(StringBuilder sb, Guid object_id, Database db)
	{
		Guid component_id = db.owner(object_id);
		if (component_id == GUID_ZERO || db.object_type(component_id) != OBJECT_TYPE_LOD_GROUP)
			return false;

		generate_add_component_commands(sb, db.owner(component_id), component_id, db);
		return true;
	}

	public static int generate_spawn_unit_commands(StringBuilder sb, Guid?[] object_ids, Database db, bool respawn = false)
	{
		int i;

		for (i = 0; i < object_ids.length; ++i) {
			if (db.object_type(object_ids[i]) == OBJECT_TYPE_UNIT) {
				if (!db.is_alive(object_ids[i]))
					continue;

				if (respawn)
					sb.append("do local old_object = LevelEditor._objects[\"%s\"];".printf(object_ids[i].to_string()));
				spawn_unit_tree(sb, object_ids[i], db);
				if (respawn)
					sb.append("; if old_object ~= nil then old_object:destroy() end end");
			} else if (respawn) {
				break;
			} else if (Unit.is_component(object_ids[i], db)) {
				if (!db.is_alive(object_ids[i]))
					continue;

				Guid component_id = object_ids[i];
				Guid unit_id = db.owner(component_id);
				generate_add_component_commands(sb, unit_id, component_id, db);
			} else if (!generate_lod_group_subobject_commands(sb, object_ids[i], db)) {
				break;
			}
		}

		return i;
	}

	public static int generate_destroy_commands(StringBuilder sb, Guid?[] object_ids, Database db)
	{
		int i;

		for (i = 0; i < object_ids.length; ++i) {
			if (db.object_type(object_ids[i]) == OBJECT_TYPE_UNIT) {
				sb.append(LevelEditorApi.destroy(object_ids[i]));
			} else if (is_component(object_ids[i], db)) {
				Guid component_id = object_ids[i];
				sb.append(LevelEditorApi.unit_destroy_component_type(db.owner(component_id), db.object_type(component_id)));
			} else if (!generate_lod_group_subobject_commands(sb, object_ids[i], db)) {
				break;
			}
		}

		return i;
	}

	public static void generate_set_component_commands(StringBuilder sb, Guid unit_id, Guid component_id, Database db)
	{
		Unit unit = Unit(db, unit_id);
		string component_type = db.object_type(component_id);

		if (component_type == OBJECT_TYPE_TRANSFORM) {
			sb.append(LevelEditorApi.move_object(unit_id
				, unit.get_component_vector3   (component_id, "data.position")
				, unit.get_component_quaternion(component_id, "data.rotation")
				, unit.get_component_vector3   (component_id, "data.scale")
				));
		} else if (component_type == OBJECT_TYPE_CAMERA) {
			sb.append(LevelEditorApi.set_camera(unit_id
				, unit.get_component_string(component_id, "data.projection")
				, unit.get_component_double(component_id, "data.fov")
				, unit.get_component_double(component_id, "data.far_range")
				, unit.get_component_double(component_id, "data.near_range")
				));
		} else if (component_type == OBJECT_TYPE_MESH_RENDERER) {
			sb.append(LevelEditorApi.set_mesh(unit_id
				, unit.get_component_resource(component_id, "data.mesh_resource")
				, unit.get_component_string(component_id, "data.geometry_name")
				, unit.get_component_resource(component_id, "data.material")
				, unit.get_component_bool  (component_id, "data.visible")
				, unit.get_component_bool  (component_id, "data.cast_shadows", true)
				));
		} else if (component_type == OBJECT_TYPE_SPRITE_RENDERER) {
			sb.append(LevelEditorApi.set_sprite(unit_id
				, unit.get_component_resource(component_id, "data.sprite_resource")
				, unit.get_component_resource(component_id, "data.material")
				, unit.get_component_double(component_id, "data.layer")
				, unit.get_component_double(component_id, "data.depth")
				, unit.get_component_bool  (component_id, "data.visible")
				, unit.get_component_bool  (component_id, "data.flip_x")
				, unit.get_component_bool  (component_id, "data.flip_y")
				));
		} else if (component_type == OBJECT_TYPE_LIGHT) {
			sb.append(LevelEditorApi.set_light(unit_id
				, unit.get_component_string (component_id, "data.type")
				, unit.get_component_double (component_id, "data.range")
				, unit.get_component_double (component_id, "data.intensity")
				, unit.get_component_double (component_id, "data.spot_angle")
				, unit.get_component_vector3(component_id, "data.color")
				, unit.get_component_double (component_id, "data.shadow_bias", 0.0001)
				, unit.get_component_bool   (component_id, "data.cast_shadows", true)
				));
		} else if (component_type == OBJECT_TYPE_ANIMATION_STATE_MACHINE) {
			sb.append(LevelEditorApi.set_animation_state_machine(unit_id
				, unit.get_component_resource(component_id, "data.state_machine_resource")
				));
		} else if (component_type == OBJECT_TYPE_FOG) {
			sb.append(LevelEditorApi.set_fog(unit_id
				, unit.get_component_vector3(component_id, "data.color")
				, unit.get_component_double (component_id, "data.density")
				, unit.get_component_double (component_id, "data.range_min")
				, unit.get_component_double (component_id, "data.range_max")
				, unit.get_component_double (component_id, "data.sun_blend")
				, unit.get_component_bool   (component_id, "data.enabled")
				));
		} else if (component_type == OBJECT_TYPE_GLOBAL_LIGHTING) {
			sb.append(LevelEditorApi.set_global_lighting(unit_id
				, unit.get_component_resource (component_id, "data.skydome_map")
				, unit.get_component_double (component_id, "data.skydome_intensity")
				, unit.get_component_vector3(component_id, "data.ambient_color")
				));
		} else if (component_type == OBJECT_TYPE_BLOOM) {
			sb.append(LevelEditorApi.set_bloom(unit_id
				, unit.get_component_bool  (component_id, "data.enabled")
				, unit.get_component_double(component_id, "data.threshold")
				, unit.get_component_double(component_id, "data.weight")
				, unit.get_component_double(component_id, "data.intensity")
				));
		} else if (component_type == OBJECT_TYPE_TONEMAP) {
			sb.append(LevelEditorApi.set_tonemap(unit_id
				, unit.get_component_string(component_id, "data.type")
				));
		} else if (component_type == OBJECT_TYPE_SCRIPT) {
			/* No sync. */
		} else if (component_type == OBJECT_TYPE_COLLIDER) {
			/* No sync. */
		} else if (component_type == OBJECT_TYPE_ACTOR) {
			/* No sync. */
		} else if (component_type == OBJECT_TYPE_FIXED_JOINT
			|| component_type == OBJECT_TYPE_HINGE_JOINT
			|| component_type == OBJECT_TYPE_SPHERICAL_JOINT
			|| component_type == OBJECT_TYPE_SPRING_JOINT
			|| component_type == OBJECT_TYPE_LIMB_JOINT
			|| component_type == OBJECT_TYPE_D6_JOINT) {
			/* No sync. */
		} else if (component_type == OBJECT_TYPE_MOVER) {
			sb.append(LevelEditorApi.set_mover(unit_id
				, unit.get_component_double(component_id, "data.height")
				, unit.get_component_double(component_id, "data.radius")
				, unit.get_component_double(component_id, "data.max_slope_angle")
				, unit.get_component_vector3(component_id, "data.center")
				));
		} else if (component_type == OBJECT_TYPE_LOD_GROUP) {
			sb.append(LevelEditorApi.set_lod_group(unit_id
				, unit.get_component_double(component_id, "data.level", -1.0)
				, unit.get_component_string(component_id, "data.fade_mode", "none")
				));
		} else if (component_type == OBJECT_TYPE_ANIMATION_STATE_MACHINE) {
			/* No sync. */
		} else {
			logw("Unregistered component type `%s`".printf(component_type));
		}
	}

	public static int generate_change_commands(StringBuilder sb, Guid?[] object_ids, Database db)
	{
		int i;

		for (i = 0; i < object_ids.length; ++i) {
			if (db.object_type(object_ids[i]) == OBJECT_TYPE_UNIT) {
				Guid unit_id = object_ids[i];
				Unit unit = Unit(db, unit_id);

				sb.append("editor_nv, editor_nq, editor_nm = Device.temp_count()");
				sb.append(LevelEditorApi.move_object(unit_id
					, unit.local_position()
					, unit.local_rotation()
					, unit.local_scale()
					));
				sb.append("Device.set_temp_count(editor_nv, editor_nq, editor_nm)");

				_component_registry.foreach((component_type, value) => {
						Guid component_id;

						if (!unit.has_component(out component_id, component_type)) {
							sb.append(LevelEditorApi.unit_destroy_component_type(unit_id, component_type));
							return;
						}

						generate_add_component_commands(sb, unit_id, component_id, db);
						generate_set_component_commands(sb, unit_id, component_id, db);
					});

				sb.append(LevelEditorApi.object_set_hidden(unit_id, db.get_bool(unit_id, Level.OBJECT_HIDDEN_KEY, false)));
				sb.append(LevelEditorApi.object_set_selectable(unit_id, !db.get_bool(unit_id, Level.OBJECT_LOCKED_KEY, false)));
			} else if (Unit.is_component(object_ids[i], db)) {
				Guid component_id = object_ids[i];
				Guid unit_id = db.owner(component_id);
				generate_set_component_commands(sb, unit_id, component_id, db);
			} else if (!generate_lod_group_subobject_commands(sb, object_ids[i], db)) {
				break;
			}
		}

		return i;
	}

	public static bool is_component(Guid id, Database db)
	{
		return (db.type_flags(StringId64(db.object_type(id))) & ObjectTypeFlags.UNIT_COMPONENT) != 0;
	}

	public bool add_component_type_dependencies(string component_type)
	{
		Guid dummy;
		if (has_component(out dummy, component_type))
			return false;

		string[] component_type_dependencies = ((string)Unit._component_registry[component_type]).split(", ");
		foreach (unowned string dependency in component_type_dependencies) {
			Guid dependency_component_id;
			if (!has_component(out dependency_component_id, dependency))
				add_component_type_dependencies(dependency);
		}

		add_component_type(component_type);
		return true;
	}
}

} /* namespace Crown */
