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

public enum UnitFlags
{
	NONE         = 0,
	CHECK_PREFAB = 1 << 0,
}

public struct Unit
{
	public static GLib.HashTable<StringId64?, GLib.GenericArray<StringId64?>> _component_registry;
	public Database _db;
	public Guid _id;

	public Unit(Database db, Guid id)
	{
		_db = db;
		_id = id;
	}

	public uint32 override_property_index(string key)
	{
		assert(key.has_prefix("modified_components.#") || key.has_prefix("deleted_components.#"));
		return _db.ensure_property_index(_id, key);
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
		foreach (unowned Guid? local_component_id in db.get_set(unit_id, db.property_index(unit_id, STRING_ID_64("components", 0xe71d1687374e5a54)))) {
			if (Guid.equal_func(local_component_id, component_id)) {
				exists = ObjectExists.EXISTS;
				break;
			}
		}

		if (exists == ObjectExists.MISSING) {
			string? prefab = db.get_resource(unit_id, db.property_index(unit_id, STRING_ID_64("prefab", 0xab2f78e885f513c6)));
			if (prefab != null) {
				Guid prefab_id;
				if (Unit.load_unit(out prefab_id, db, prefab) != LoadError.SUCCESS)
					return ObjectExists.UNKNOWN;

				exists = component_exists_internal(db, prefab_id, component_id, true, visited);
			}
		}

		if (exists == ObjectExists.EXISTS
			&& apply_unit_deletes
			&& db.get_property(unit_id, db.property_index(unit_id, StringId64("deleted_components.#" + component_id.to_string()))) != null
			)
			exists = ObjectExists.MISSING;

		return exists;
	}

	private static ObjectExists child_exists_internal(Database db, Guid unit_id, Guid child_id, bool apply_unit_deletes, GLib.GenericSet<Guid?> visited = new GLib.GenericSet<Guid?>(Guid.hash_func, Guid.equal_func))
	{
		if (!visited.add(unit_id))
			return ObjectExists.UNKNOWN;

		ObjectExists exists = ObjectExists.MISSING;
		Guid?[] children = db.get_set(unit_id, db.property_index(unit_id, STRING_ID_64("children", 0x6fbb13de0e1dce0d)));
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
			string? prefab = db.get_resource(unit_id, db.property_index(unit_id, STRING_ID_64("prefab", 0xab2f78e885f513c6)));
			if (prefab != null) {
				Guid prefab_id;
				if (Unit.load_unit(out prefab_id, db, prefab) != LoadError.SUCCESS)
					return ObjectExists.UNKNOWN;

				exists = child_exists_internal(db, prefab_id, child_id, true, visited);
			}
		}

		if (exists == ObjectExists.EXISTS && apply_unit_deletes) {
			Guid?[] deleted_children = db.get_set(unit_id, db.property_index(unit_id, STRING_ID_64("deleted_children", 0xc93de33d9fd602fe)));
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
		const string prefix = "modified_components.#";
		const string suffix = ".data.material";
		foreach (unowned string key in unit_keys) {
			if (!key.has_prefix(prefix)
				|| !key.has_suffix(suffix)
				|| key.length != prefix.length + 36 + suffix.length
				)
				continue;

			Guid component_id = Guid.parse(key.substring(prefix.length, 36));
			if (component_exists_internal(_db, _id, component_id, true) != ObjectExists.EXISTS
				|| _db.object_type(component_id) != STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893)
				)
				continue;

			string? material = get_component_resource(component_id, _db.property_index(component_id, STRING_ID_64("data.material", 0xf014ddbddc53c116)));
			string materials_key = prefix + component_id.to_string() + ".data.materials";
			if (material != null && !_db.has_property(_id, _db.property_index(_id, StringId64(materials_key)))) {
				Guid prefab_id = GUID_ZERO;
				GLib.GenericSet<Guid?> materials = (GLib.GenericSet<Guid?>)get_component_property(component_id, _db.property_index(component_id, STRING_ID_64("data.materials", 0xb4c01840c957402d)), guid_set_new());
				foreach (unowned Guid? id in materials) {
					if (_db.is_alive(id) && _db.get_string(id, _db.property_index(id, STRING_ID_64("data.slot", 0x0de060e1cd2f27fe))) == "default") {
						prefab_id = id;
						break;
					}
				}

				Guid binding_id = Guid.new_guid();
				if (prefab_id != GUID_ZERO) {
					_db.create_from_prefab(binding_id, prefab_id);
				} else {
					_db.create(binding_id, STRING_ID_64(OBJECT_TYPE_MESH_MATERIAL, 0x6701353384dd782d));
					_db.set_string(binding_id, _db.property_index(binding_id, STRING_ID_64("data.slot", 0x0de060e1cd2f27fe)), "default");
				}
				_db.set_resource(binding_id, _db.property_index(binding_id, STRING_ID_64("data.material", 0xf014ddbddc53c116)), material);
				_db.add_to_set(_id, override_property_index(materials_key), binding_id);
			}
		}

		foreach (unowned string key in unit_keys) {
			if (key.has_prefix("deleted_components.#")
				&& key.length == "deleted_components.#".length + 36
				) {
				Guid component_id = Guid.parse(key.substring("deleted_components.#".length, 36));
				if (component_exists_internal(_db, _id, component_id, false) == ObjectExists.MISSING)
					_db.set_null(_id, _db.property_index(_id, StringId64(key)));
			} else if (key.has_prefix("modified_components.#")
				&& key.length > "modified_components.#".length + 36
				&& key["modified_components.#".length + 36] == '.'
				) {
				Guid component_id = Guid.parse(key.substring("modified_components.#".length, 36));
				if (component_exists_internal(_db, _id, component_id, true) == ObjectExists.MISSING)
					_db.set_null(_id, _db.property_index(_id, StringId64(key)));
			}
		}

		prune_stale_child_override_set(_db.property_index(_id, STRING_ID_64("deleted_children", 0xc93de33d9fd602fe)), false);
		prune_stale_child_override_set(_db.property_index(_id, STRING_ID_64("modified_children", 0x9275f52a1eb444c9)), true);
	}

	private void prune_stale_child_override_set(uint32 property, bool apply_unit_deletes)
	{
		foreach (unowned Guid? child_id in _db.get_set(_id, property)) {
			if (child_exists_internal(_db, _id, child_id, apply_unit_deletes) == ObjectExists.MISSING)
				_db.remove_from_set(_id, property, child_id);
		}
	}

	public void create_empty()
	{
		_db.create(_id, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
	}

	public int create(string? prefab, uint32 flags = UnitFlags.NONE)
	{
		create_empty();
		return prefab == null ? 0 : set_prefab(prefab, flags);
	}

	public Value? get_component_property(Guid component_id, uint32 property, Value? deffault = null)
	{
		assert(component_exists_internal(_db, _id, component_id, true) == ObjectExists.EXISTS);
		if (property == uint32.MAX)
			return deffault;

		Value? val;

		// Search in components
		val = _db.get_property(_id, _db.property_index(_id, STRING_ID_64("components", 0xe71d1687374e5a54)));
		if (val != null) {
			if (((GLib.GenericSet<Guid?>)val).contains(component_id))
				return _db.get_property(component_id, property, deffault);
		}

		// Search in modified_components
		val = _db.get_property(_id, _db.property_index(_id, StringId64("modified_components.#" + component_id.to_string() + "." + _db.property_name(component_id, property))));
		if (val != null && !val.holds(typeof(GLib.GenericSet)))
			return val;

		// Search in prefab
		string? prefab = prefab();
		if (prefab != null) {
			// Convert prefab path to object ID.
			Guid prefab_id = GUID_ZERO;
			Unit.load_unit(out prefab_id, _db, prefab);

			Unit unit = Unit(_db, prefab_id);
			return _db.inherit_value(val, unit.get_component_property(component_id, property, deffault));
		}

		return val != null ? val : deffault;
	}

	public bool get_component_bool(Guid component_id, uint32 property, bool deffault = false)
	{
		return (bool)get_component_property(component_id, property, deffault);
	}

	public double get_component_double(Guid component_id, uint32 property, double deffault = 0.0)
	{
		return (double)get_component_property(component_id, property, deffault);
	}

	public string get_component_string(Guid component_id, uint32 property, string deffault = "")
	{
		return (string)get_component_property(component_id, property, deffault);
	}

	public Vector3 get_component_vector3(Guid component_id, uint32 property, Vector3 deffault = VECTOR3_ZERO)
	{
		return (Vector3)get_component_property(component_id, property, deffault);
	}

	public Quaternion get_component_quaternion(Guid component_id, uint32 property, Quaternion deffault = QUATERNION_IDENTITY)
	{
		return (Quaternion)get_component_property(component_id, property, deffault);
	}

	public string? get_component_resource(Guid component_id, uint32 property, string? deffault = null)
	{
		Resource deffault_res = { deffault };
		Value? val = get_component_property(component_id, property, deffault_res);
		if (val.holds(typeof(Resource)))
			return ((Resource)val).name;
		return (string?)val;
	}

	public Guid get_component_reference(Guid component_id, uint32 property, Guid deffault = GUID_ZERO)
	{
		return (Guid)get_component_property(component_id, property, deffault);
	}

	public void set_component_bool(Guid component_id, uint32 property, bool val)
	{
		assert(component_exists_internal(_db, _id, component_id, true) == ObjectExists.EXISTS);

		// Search in components
		Value? components = _db.get_property(_id, _db.property_index(_id, STRING_ID_64("components", 0xe71d1687374e5a54)));
		if (components != null && ((GLib.GenericSet<Guid?>)components).contains(component_id)) {
			_db.set_bool(component_id, property, val);
			return;
		}

		_db.set_bool(_id, override_property_index("modified_components.#" + component_id.to_string() + "." + _db.property_name(component_id, property)), val);
	}

	public void set_component_double(Guid component_id, uint32 property, double val)
	{
		assert(component_exists_internal(_db, _id, component_id, true) == ObjectExists.EXISTS);

		// Search in components
		Value? components = _db.get_property(_id, _db.property_index(_id, STRING_ID_64("components", 0xe71d1687374e5a54)));
		if (components != null && ((GLib.GenericSet<Guid?>)components).contains(component_id)) {
			_db.set_double(component_id, property, val);
			return;
		}

		_db.set_double(_id, override_property_index("modified_components.#" + component_id.to_string() + "." + _db.property_name(component_id, property)), val);
	}

	public void set_component_string(Guid component_id, uint32 property, string val)
	{
		assert(component_exists_internal(_db, _id, component_id, true) == ObjectExists.EXISTS);

		// Search in components
		Value? components = _db.get_property(_id, _db.property_index(_id, STRING_ID_64("components", 0xe71d1687374e5a54)));
		if (components != null && ((GLib.GenericSet<Guid?>)components).contains(component_id)) {
			_db.set_string(component_id, property, val);
			return;
		}

		_db.set_string(_id, override_property_index("modified_components.#" + component_id.to_string() + "." + _db.property_name(component_id, property)), val);
	}

	public void set_component_vector3(Guid component_id, uint32 property, Vector3 val)
	{
		assert(component_exists_internal(_db, _id, component_id, true) == ObjectExists.EXISTS);

		// Search in components
		Value? components = _db.get_property(_id, _db.property_index(_id, STRING_ID_64("components", 0xe71d1687374e5a54)));
		if (components != null && ((GLib.GenericSet<Guid?>)components).contains(component_id)) {
			_db.set_vector3(component_id, property, val);
			return;
		}

		_db.set_vector3(_id, override_property_index("modified_components.#" + component_id.to_string() + "." + _db.property_name(component_id, property)), val);
	}

	public void set_component_quaternion(Guid component_id, uint32 property, Quaternion val)
	{
		assert(component_exists_internal(_db, _id, component_id, true) == ObjectExists.EXISTS);

		// Search in components
		Value? components = _db.get_property(_id, _db.property_index(_id, STRING_ID_64("components", 0xe71d1687374e5a54)));
		if (components != null && ((GLib.GenericSet<Guid?>)components).contains(component_id)) {
			_db.set_quaternion(component_id, property, val);
			return;
		}

		_db.set_quaternion(_id, override_property_index("modified_components.#" + component_id.to_string() + "." + _db.property_name(component_id, property)), val);
	}

	public void set_component_resource(Guid component_id, uint32 property, string? val)
	{
		assert(component_exists_internal(_db, _id, component_id, true) == ObjectExists.EXISTS);

		// Search in components
		Value? components = _db.get_property(_id, _db.property_index(_id, STRING_ID_64("components", 0xe71d1687374e5a54)));
		if (components != null && ((GLib.GenericSet<Guid?>)components).contains(component_id)) {
			_db.set_resource(component_id, property, val);
			return;
		}

		_db.set_resource(_id, override_property_index("modified_components.#" + component_id.to_string() + "." + _db.property_name(component_id, property)), val);
	}

	public void set_component_reference(Guid component_id, uint32 property, Guid val)
	{
		assert(component_exists_internal(_db, _id, component_id, true) == ObjectExists.EXISTS);

		// Search in components
		Value? components = _db.get_property(_id, _db.property_index(_id, STRING_ID_64("components", 0xe71d1687374e5a54)));
		if (components != null && ((GLib.GenericSet<Guid?>)components).contains(component_id)) {
			_db.set_reference(component_id, property, val);
			return;
		}

		_db.set_reference(_id, override_property_index("modified_components.#" + component_id.to_string() + "." + _db.property_name(component_id, property)), val);
	}

	/// Returns whether the @a unit_id has a component of type @a component_type.
	public static bool has_component_static(out Guid component_id, StringId64 component_type, Database db, Guid unit_id)
	{
		Value? val;
		component_id = GUID_ZERO;
		bool prefab_has_component = false;

		// If the component type is found inside the "components" array, the unit has the component
		// and it owns it.
		val = db.get_property(unit_id, db.property_index(unit_id, STRING_ID_64("components", 0xe71d1687374e5a54)));
		if (val != null) {
			foreach (Guid? id in (GLib.GenericSet<Guid?>)val) {
				if (db.object_type(id) == component_type) {
					component_id = id;
					return true;
				}
			}
		}

		// Otherwise, search if any prefab has the component.
		string? prefab = db.get_resource(unit_id, db.property_index(unit_id, STRING_ID_64("prefab", 0xab2f78e885f513c6)));
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
			return db.get_property(unit_id, db.property_index(unit_id, StringId64("deleted_components.#" + component_id.to_string()))) == null;

		component_id = GUID_ZERO;
		return false;
	}

	/// Returns whether the unit has the component_type.
	public bool has_component(out Guid component_id, StringId64 component_type)
	{
		return Unit.has_component_static(out component_id, component_type, _db, _id);
	}

	public Vector3 local_position()
	{
		Vector3 position;

		Guid component_id;
		if (has_component(out component_id, STRING_ID_64(OBJECT_TYPE_TRANSFORM, 0x69e14b13ad9b5315)))
			position = get_component_vector3(component_id, _db.property_index(component_id, STRING_ID_64("data.position", 0xbf17708e8bd2a30b)));
		else
			position = _db.get_vector3(_id, _db.property_index(_id, STRING_ID_64("position", 0x8bbeb160190f613a)));

		return position;
	}

	public Quaternion local_rotation()
	{
		Quaternion rotation;

		Guid component_id;
		if (has_component(out component_id, STRING_ID_64(OBJECT_TYPE_TRANSFORM, 0x69e14b13ad9b5315)))
			rotation = get_component_quaternion(component_id, _db.property_index(component_id, STRING_ID_64("data.rotation", 0x3c8974411eeaf63c)));
		else
			rotation = _db.get_quaternion(_id, _db.property_index(_id, STRING_ID_64("rotation", 0x2060566242789baa)));

		return rotation;
	}

	public Vector3 local_scale()
	{
		Vector3 scale;

		Guid component_id;
		if (has_component(out component_id, STRING_ID_64(OBJECT_TYPE_TRANSFORM, 0x69e14b13ad9b5315)))
			scale = get_component_vector3(component_id, _db.property_index(component_id, STRING_ID_64("data.scale", 0x16021af8bd4fea50)));
		else
			scale = _db.get_vector3(_id, _db.property_index(_id, STRING_ID_64("scale", 0xeec8c5fba3c8bc0b)), VECTOR3_ONE);

		return scale;
	}

	public void set_local_position(Vector3 position)
	{
		Guid component_id;
		if (has_component(out component_id, STRING_ID_64(OBJECT_TYPE_TRANSFORM, 0x69e14b13ad9b5315)))
			set_component_vector3(component_id, _db.property_index(component_id, STRING_ID_64("data.position", 0xbf17708e8bd2a30b)), position);
		else
			_db.set_vector3(_id, _db.property_index(_id, STRING_ID_64("position", 0x8bbeb160190f613a)), position);
	}

	public void set_local_rotation(Quaternion rotation)
	{
		Guid component_id;
		if (has_component(out component_id, STRING_ID_64(OBJECT_TYPE_TRANSFORM, 0x69e14b13ad9b5315)))
			set_component_quaternion(component_id, _db.property_index(component_id, STRING_ID_64("data.rotation", 0x3c8974411eeaf63c)), rotation);
		else
			_db.set_quaternion(_id, _db.property_index(_id, STRING_ID_64("rotation", 0x2060566242789baa)), rotation);
	}

	public void set_local_scale(Vector3 scale)
	{
		Guid component_id;
		if (has_component(out component_id, STRING_ID_64(OBJECT_TYPE_TRANSFORM, 0x69e14b13ad9b5315)))
			set_component_vector3(component_id, _db.property_index(component_id, STRING_ID_64("data.scale", 0x16021af8bd4fea50)), scale);
		else
			_db.set_vector3(_id, _db.property_index(_id, STRING_ID_64("scale", 0xeec8c5fba3c8bc0b)), scale);
	}

	// Adds the @a component_type to the unit and returns its ID.
	public Guid add_component_type(StringId64 component_type)
	{
		// Create a new component.
		Guid component_id = Guid.new_guid();
		_db.create(component_id, component_type);
		_db.add_to_set(_id, _db.property_index(_id, STRING_ID_64("components", 0xe71d1687374e5a54)), component_id);
		if (component_type == STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893))
			MeshResource.set_material_slot(_db, component_id, "default", "core/components/noop");
		return component_id;
	}

	/// Removes the @a component_type from the unit.
	public void remove_component_type(StringId64 component_type)
	{
		Guid component_id;
		if (has_component(out component_id, component_type)) {
			if (_id == _db.owner(component_id)) {
				_db.remove_from_set(_id, _db.property_index(_id, STRING_ID_64("components", 0xe71d1687374e5a54)), component_id);
				_db.destroy(component_id);
				_db.add_restore_point((int)ActionType.CHANGE_OBJECTS, { _id });
			} else {
				_db.set_bool(_id, override_property_index("deleted_components.#" + component_id.to_string()), false);

				// Clean all modified_components keys that matches the deleted component ID.
				string[] unit_keys = _db.get_keys(_id);
				for (int ii = 0; ii < unit_keys.length; ++ii) {
					if (unit_keys[ii].has_prefix("modified_components.#" + component_id.to_string()))
						_db.set_null(_id, _db.property_index(_id, StringId64(unit_keys[ii])));
				}
				_db.add_restore_point((int)ActionType.CHANGE_OBJECTS, { _id });
			}
		} else {
			logw("The unit has no such component type `%s`".printf(_db.type_name(component_type)));
		}
	}

	public static void register_component_type(StringId64 type, string depends_on)
	{
		if (_component_registry == null)
			_component_registry = new GLib.HashTable<StringId64?, GLib.GenericArray<StringId64?>>(StringId64.hash_func, StringId64.equal_func);
		GLib.GenericArray<StringId64?> dependencies = new GLib.GenericArray<StringId64?>();
		if (depends_on != "") {
			foreach (unowned string dependency in depends_on.split(", "))
				dependencies.add(StringId64(dependency));
		}
		_component_registry[type] = dependencies;
	}

	public string? prefab()
	{
		return _db.get_resource(_id, _db.property_index(_id, STRING_ID_64("prefab", 0xab2f78e885f513c6)));
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

			string? prefab = validation_db.get_resource(prefab_id, validation_db.property_index(prefab_id, STRING_ID_64("prefab", 0xab2f78e885f513c6)));
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

	public int set_prefab(string? prefab_name, uint32 flags = UnitFlags.NONE)
	{
		if (prefab() == prefab_name)
			return -1;

		if ((flags& UnitFlags.CHECK_PREFAB) != 0) {
			if (can_set_prefab(prefab_name) != 0)
				return -1;
		}

		Guid transform_id;
		bool has_transform = has_component(out transform_id, STRING_ID_64(OBJECT_TYPE_TRANSFORM, 0x69e14b13ad9b5315));
		bool restore_position = has_transform || _db.has_property(_id, _db.property_index(_id, STRING_ID_64("position", 0x8bbeb160190f613a)));
		bool restore_rotation = has_transform || _db.has_property(_id, _db.property_index(_id, STRING_ID_64("rotation", 0x2060566242789baa)));
		bool restore_scale = has_transform || _db.has_property(_id, _db.property_index(_id, STRING_ID_64("scale", 0xeec8c5fba3c8bc0b)));

		// Keep authored transform channels stable without turning defaults into overrides.
		Vector3 unit_pos = restore_position ? local_position() : VECTOR3_ZERO;
		Quaternion unit_rot = restore_rotation ? local_rotation() : QUATERNION_IDENTITY;
		Vector3 unit_scl = restore_scale ? local_scale() : VECTOR3_ONE;

		_db.set_resource(_id, _db.property_index(_id, STRING_ID_64("prefab", 0xab2f78e885f513c6)), prefab_name);

		// Drop overrides that belonged to the previous prefab hierarchy.
		foreach (unowned string key in _db.get_keys(_id)) {
			if (key.has_prefix("modified_components.")
				|| key.has_prefix("deleted_components.")
				|| key.has_prefix("modified_children.")
				|| key.has_prefix("deleted_children.")
				)
				_db.set_null(_id, _db.property_index(_id, StringId64(key)));
		}

		// Remove owned components already provided by the new prefab.
		if (prefab_name != null) {
			Guid prefab_id = GUID_ZERO;
			if (Unit.load_unit(out prefab_id, _db, prefab_name) == LoadError.SUCCESS) {
				foreach (unowned Guid? component_id in _db.get_set(_id, _db.property_index(_id, STRING_ID_64("components", 0xe71d1687374e5a54)))) {
					Guid prefab_component_id;
					if (!Unit.has_component_static(out prefab_component_id, _db.object_type(component_id), _db, prefab_id))
						continue;

					_db.remove_from_set(_id, _db.property_index(_id, STRING_ID_64("components", 0xe71d1687374e5a54)), component_id);
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
			&& _db.get_resource(_id, _db.property_index(_id, STRING_ID_64("prefab", 0xab2f78e885f513c6))) == "core/units/light";
	}

	/// Returns whether the unit is a camera unit.
	public bool is_camera()
	{
		return has_prefab()
			&& _db.get_resource(_id, _db.property_index(_id, STRING_ID_64("prefab", 0xab2f78e885f513c6))) == "core/units/camera";
	}

	public static void generate_add_component_commands(StringBuilder sb, Guid unit_id, Guid component_id, Database db)
	{
		Unit unit = Unit(db, unit_id);
		StringId64 component_type = db.object_type(component_id);

		if (component_type == STRING_ID_64(OBJECT_TYPE_TRANSFORM, 0x69e14b13ad9b5315)) {
			string s = LevelEditorApi.add_tranform_component(unit_id
				, component_id
				, unit.get_component_vector3   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.position", 0xbf17708e8bd2a30b)))
				, unit.get_component_quaternion(component_id, unit._db.property_index(component_id, STRING_ID_64("data.rotation", 0x3c8974411eeaf63c)))
				, unit.get_component_vector3   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.scale", 0x16021af8bd4fea50)))
				);
			sb.append(s);
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_CAMERA, 0x60ed8c3931822dc7)) {
			string s = LevelEditorApi.add_camera_component(unit_id
				, component_id
				, unit.get_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.projection", 0x6e0676d6db8d3734)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.fov", 0xae3b0c9413994b89)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.far_range", 0x3283caec3b9511b0)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.near_range", 0xbaf158a47f2c7242)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.orthographic_size", 0x9c48de9cb4cdde6f)))
				);
			sb.append(s);
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893)) {
			string s = LevelEditorApi.add_mesh_renderer_component(unit_id
				, component_id
				, unit.get_component_resource(component_id, unit._db.property_index(component_id, STRING_ID_64("data.mesh_resource", 0xd08afb39c15cc158)))
				, unit.get_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.geometry_name", 0x040a59886a3e4a2d)))
				, unit.get_component_bool  (component_id, unit._db.property_index(component_id, STRING_ID_64("data.visible", 0xad0490cc1fffda48)))
				, unit.get_component_bool  (component_id, unit._db.property_index(component_id, STRING_ID_64("data.cast_shadows", 0xc2b3140260446694)), true)
				);
			sb.append(s);
			generate_mesh_material_commands(sb, unit_id, component_id, db);
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_SPRITE_RENDERER, 0x3d7229ea6a1c2a3b)) {
			string s = LevelEditorApi.add_sprite_renderer_component(unit_id
				, component_id
				, unit.get_component_resource(component_id, unit._db.property_index(component_id, STRING_ID_64("data.sprite_resource", 0x223dd1c4e8beb8b0)))
				, unit.get_component_resource(component_id, unit._db.property_index(component_id, STRING_ID_64("data.material", 0xf014ddbddc53c116)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.layer", 0x90a95ce10ee6d6ed)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.depth", 0x0ccfd59731b887a6)))
				, unit.get_component_bool  (component_id, unit._db.property_index(component_id, STRING_ID_64("data.visible", 0xad0490cc1fffda48)))
				, unit.get_component_bool  (component_id, unit._db.property_index(component_id, STRING_ID_64("data.flip_x", 0x5b0c3f73c8bf9120)))
				, unit.get_component_bool  (component_id, unit._db.property_index(component_id, STRING_ID_64("data.flip_y", 0x85822f09e9cb4e87)))
				);
			sb.append(s);
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_LIGHT, 0x7dd7224fbb9f08c2)) {
			string s = LevelEditorApi.add_light_component(unit_id
				, component_id
				, unit.get_component_string (component_id, unit._db.property_index(component_id, STRING_ID_64("data.type", 0x3d4dcce26f0f13fd)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.range", 0x969effdaa778c82b)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.intensity", 0xd1e406b3aeae4d67)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.spot_angle", 0xad796e4ae667dead)))
				, unit.get_component_vector3(component_id, unit._db.property_index(component_id, STRING_ID_64("data.color", 0xac9624a15a2891c0)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.shadow_bias", 0xa8290b921c961845)), 0.0004)
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.shadow_bias_normal", 0x27c8d9bf59a6dff8)), 1.0)
				, unit.get_component_bool   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.cast_shadows", 0xc2b3140260446694)), true)
				, unit.get_component_resource(component_id, unit._db.property_index(component_id, STRING_ID_64("data.cookie", 0xe9199868645b3060)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.cookie_scale", 0xcb67dc4b1d86ba44)), 1.0)
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.cookie_x", 0x256474d6c2196cc5)), 0.0)
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.cookie_y", 0xf3edfef5cf63ab5a)), 0.0)
				);
			sb.append(s);
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_ANIMATION_STATE_MACHINE, 0x0d694773e87992ac)) {
			string s = LevelEditorApi.add_animation_state_machine_component(unit_id
				, component_id
				, unit.get_component_resource(component_id, unit._db.property_index(component_id, STRING_ID_64("data.state_machine_resource", 0xd98f8b4d76314483)))
				);
			sb.append(s);
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_MOVER, 0x273ef5a1ac07d371)) {
			sb.append(LevelEditorApi.add_mover_component(unit_id
				, component_id
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.height", 0x4be7bd93a635d8b2)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.radius", 0x380298023aeac247)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.max_slope_angle", 0x19f6752be9bb9fba)))
				, unit.get_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.collision_filter", 0x493f7c3cd3e78169)))
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_COLLIDER, 0x9a9fe4362129d74e)) {
			Guid actor_id;
			if (unit.has_component(out actor_id, STRING_ID_64(OBJECT_TYPE_ACTOR, 0xac2b738a374cf583))) {
				string shape = unit.get_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.shape", 0xd4ffa681b8480051)), "box");
				bool is_mesh = shape == "convex_hull" || shape == "mesh";
				sb.append(LevelEditorApi.add_actor_component(unit_id
					, actor_id
					, shape
					, unit.get_component_vector3   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.collider_data.position", 0x88957275bc90e7ca)))
					, unit.get_component_quaternion(component_id, unit._db.property_index(component_id, STRING_ID_64("data.collider_data.rotation", 0xb69c28e242fd898a)))
					, unit.get_component_vector3   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.collider_data.half_extents", 0xa4df041618e7072c)), Vector3(0.5, 0.5, 0.5))
					, unit.get_component_double    (component_id, unit._db.property_index(component_id, STRING_ID_64("data.collider_data.radius", 0x09bb508b35944b45)), 0.5)
					, unit.get_component_double    (component_id, unit._db.property_index(component_id, STRING_ID_64("data.collider_data.height", 0xb7ea14ce927d094d)), 1.0)
					, is_mesh ? unit.get_component_resource(component_id, unit._db.property_index(component_id, STRING_ID_64("data.scene", 0x8f8eae68a99b51cc))) : ""
					, is_mesh ? unit.get_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.name", 0x2b855b7e7675517f))) : ""
					));
			}
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_ACTOR, 0xac2b738a374cf583)) {
			Guid collider_id;
			if (unit.has_component(out collider_id, STRING_ID_64(OBJECT_TYPE_COLLIDER, 0x9a9fe4362129d74e)))
				generate_add_component_commands(sb, unit_id, collider_id, db);
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_FIXED_JOINT, 0x69617f389ada03c1)
			|| component_type == STRING_ID_64(OBJECT_TYPE_HINGE_JOINT, 0xd8064886bb32c4e8)
			|| component_type == STRING_ID_64(OBJECT_TYPE_SPHERICAL_JOINT, 0xf69ceb04066c63e3)
			|| component_type == STRING_ID_64(OBJECT_TYPE_SPRING_JOINT, 0xcdf8657d6ccbbba7)
			|| component_type == STRING_ID_64(OBJECT_TYPE_LIMB_JOINT, 0x96f166437068fff7)
			|| component_type == STRING_ID_64(OBJECT_TYPE_D6_JOINT, 0x4f780f631099cc71)) {
			Guid other_actor_unit_id = unit.get_component_reference(component_id, unit._db.property_index(component_id, STRING_ID_64("data.other_actor", 0x2a9f5ce42e295147)));
			if (other_actor_unit_id != GUID_ZERO && !db.is_alive(other_actor_unit_id))
				other_actor_unit_id = GUID_ZERO;
			sb.append(LevelEditorApi.add_joint_component(unit_id
				, component_id
				, db.type_name(component_type)
				, unit.get_component_vector3(component_id, unit._db.property_index(component_id, STRING_ID_64("data.position", 0xbf17708e8bd2a30b)), VECTOR3_ZERO)
				, unit.get_component_quaternion(component_id, unit._db.property_index(component_id, STRING_ID_64("data.rotation", 0x3c8974411eeaf63c)), QUATERNION_IDENTITY)
				, other_actor_unit_id
				, unit.get_component_vector3(component_id, unit._db.property_index(component_id, STRING_ID_64("data.other_position", 0x6868f507484de351)), VECTOR3_ZERO)
				, unit.get_component_quaternion(component_id, unit._db.property_index(component_id, STRING_ID_64("data.other_rotation", 0x1a729b41fc11d2c1)), QUATERNION_IDENTITY)
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_LOD_GROUP, 0x97993ef522a1a9f0)) {
			GLib.GenericSet<Guid?> levels = (GLib.GenericSet<Guid?>)unit.get_component_property(component_id, unit._db.property_index(component_id, STRING_ID_64("data.lod_levels", 0xf8b66d06ee19128f)), guid_set_new());
			GLib.GenericArray<Guid?> live_levels = new GLib.GenericArray<Guid?>();
			foreach (unowned Guid? level_id in levels) {
				if (db.is_alive(level_id))
					live_levels.add(level_id);
			}
			Guid?[] lod_levels = live_levels.steal();
			GLib.qsort_with_data<Guid?>(lod_levels, sizeof(Guid?), (a, b) => {
					double screen_size_a = db.get_double(a, db.property_index(a, STRING_ID_64("data.screen_size", 0xb0affbb45037b116)));
					double screen_size_b = db.get_double(b, db.property_index(b, STRING_ID_64("data.screen_size", 0xb0affbb45037b116)));
					return screen_size_a > screen_size_b ? -1 : (screen_size_a < screen_size_b ? 1 : 0);
				});

			Guid[] mesh_renderer_ids = new Guid[lod_levels.length];
			double[] screen_sizes = new double[lod_levels.length];
			for (int i = 0; i < lod_levels.length; ++i) {
				mesh_renderer_ids[i] = db.get_reference(lod_levels[i], db.property_index(lod_levels[i], STRING_ID_64("data.mesh_renderer", 0xba4d878d19255e65)));
				screen_sizes[i] = db.get_double(lod_levels[i], db.property_index(lod_levels[i], STRING_ID_64("data.screen_size", 0xb0affbb45037b116)));
			}

			sb.append(LevelEditorApi.add_lod_group_component(unit_id
				, component_id
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.level", 0xd70ddfec12f72a7b)), -1.0)
				, unit.get_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.fade_mode", 0xd1e695426829933b)), "none")
				, mesh_renderer_ids
				, screen_sizes
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_FOG, 0x063924dbf007ef0d)) {
			sb.append(LevelEditorApi.add_fog_component(unit_id, component_id));
			sb.append(LevelEditorApi.set_fog(unit_id
				, unit.get_component_vector3(component_id, unit._db.property_index(component_id, STRING_ID_64("data.color", 0xac9624a15a2891c0)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.density", 0xc54946c276726fb6)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.range_min", 0xbf6be34b866c5366)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.range_max", 0x792b1cb32fc03704)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.sun_blend", 0x5816ee4020741cf7)))
				, unit.get_component_bool   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.enabled", 0xe94d35860f381a41)))
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_GLOBAL_LIGHTING, 0xa002d9c1718af7fe)) {
			sb.append(LevelEditorApi.add_global_lighting_component(unit_id, component_id));
			sb.append(LevelEditorApi.set_global_lighting(unit_id
				, unit.get_component_resource (component_id, unit._db.property_index(component_id, STRING_ID_64("data.skydome_map", 0x21390a5a0186c3d9)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.skydome_intensity", 0xac668fe1d8a8ffee)))
				, unit.get_component_vector3(component_id, unit._db.property_index(component_id, STRING_ID_64("data.ambient_color", 0x7bf3621acdc9d34b)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.shadow_distance", 0xf7011f881f6cf44c)), 100.0)
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_BLOOM, 0x28e4a7cb995dd31c)) {
			sb.append(LevelEditorApi.add_bloom_component(unit_id, component_id));
			sb.append(LevelEditorApi.set_bloom(unit_id
				, unit.get_component_bool  (component_id, unit._db.property_index(component_id, STRING_ID_64("data.enabled", 0xe94d35860f381a41)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.threshold", 0xf196e4784078b3ed)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.weight", 0x45a7acbf247fe2bf)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.intensity", 0xd1e406b3aeae4d67)))
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_COLOR_GRADING, 0x18f07f17486057e5)) {
			sb.append(LevelEditorApi.add_color_grading_component(unit_id, component_id));
			sb.append(LevelEditorApi.set_color_grading(unit_id
				, unit.get_component_bool   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.enabled", 0xe94d35860f381a41)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.exposure_bias", 0x789f6aca1fbf35b5)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.contrast", 0x14cb1aa3781d5890)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.saturation", 0xf02ececf948a7d75)))
				, unit.get_component_vector3(component_id, unit._db.property_index(component_id, STRING_ID_64("data.color_filter", 0x65913769f14bc153)))
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_TONEMAP, 0xeda056ab7089b06b)) {
			sb.append(LevelEditorApi.add_tonemap_component(unit_id, component_id));
			sb.append(LevelEditorApi.set_tonemap(unit_id
				, unit.get_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.type", 0x3d4dcce26f0f13fd)))
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_VIGNETTE, 0xae5dfa90b77c3567)) {
			sb.append(LevelEditorApi.add_vignette_component(unit_id, component_id));
			sb.append(LevelEditorApi.set_vignette(unit_id
				, unit.get_component_bool   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.enabled", 0xe94d35860f381a41)))
				, unit.get_component_vector3(component_id, unit._db.property_index(component_id, STRING_ID_64("data.color", 0xac9624a15a2891c0)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.strength", 0x160efa2da4cdb898)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.radius", 0x380298023aeac247)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.softness", 0xc978a6136471ea40)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.roundness", 0x52b1d03886e89bc7)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.center_x", 0x6a91eb6d1e54a28c)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.center_y", 0x4cd541a241263696)))
				));
		}
	}

	public static void collect_unit_tree(GLib.GenericArray<Guid?> unit_ids, Guid unit_id, Database db)
	{
		unit_ids.add(unit_id);

		if (db.has_property(unit_id, db.property_index(unit_id, STRING_ID_64("children", 0x6fbb13de0e1dce0d)))) {
			Guid?[] children = db.get_set(unit_id, db.property_index(unit_id, STRING_ID_64("children", 0x6fbb13de0e1dce0d)));
			foreach (unowned Guid? child_id in children)
				collect_unit_tree(unit_ids, child_id, db);
		}
	}

	public static int compare_component_spawn_order(Database db, Guid? component_a, Guid? component_b)
	{
		double order_a = db.get_double(component_a, db.property_index(component_a, STRING_ID_64("spawn_order", 0x2f8a65a7213a6ef5)));
		double order_b = db.get_double(component_b, db.property_index(component_b, STRING_ID_64("spawn_order", 0x2f8a65a7213a6ef5)));

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

		for (int i = 0; i < unit_ids.length; ++i) {
			Guid id = unit_ids[i];
			Unit unit = Unit(db, id);
			if (unit.prefab() != null) {
				spawn_unit(sb, id, db);
			} else {
				sb.append(LevelEditorApi.spawn_empty_unit(id));
				Guid?[] unit_components = db.get_set(id, db.property_index(id, STRING_ID_64("components", 0xe71d1687374e5a54)));
				foreach (unowned Guid? component_id in unit_components)
					components.add(component_id);
			}
		}

		components.sort_with_data((a, b) => {
				return compare_component_spawn_order(db, a, b);
			});

		for (int i = 0; i < components.length; ++i) {
			Guid component_id = components[i];
			if (db.object_type(component_id) == STRING_ID_64(OBJECT_TYPE_TRANSFORM, 0x69e14b13ad9b5315))
				generate_add_component_commands(sb, db.owner(component_id), component_id, db);
		}

		for (int i = 0; i < unit_ids.length; ++i) {
			Guid id = unit_ids[i];
			Guid owner_id = db.owner(id);
			if (owner_id != GUID_ZERO && db.object_type(owner_id) == STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f)) {
				sb.append(LevelEditorApi.unit_set_parent(owner_id, id));
			}
		}

		for (int i = 0; i < components.length; ++i) {
			Guid component_id = components[i];
			if (db.object_type(component_id) != STRING_ID_64(OBJECT_TYPE_TRANSFORM, 0x69e14b13ad9b5315))
				generate_add_component_commands(sb, db.owner(component_id), component_id, db);
		}

		for (int i = 0; i < unit_ids.length; ++i) {
			Guid id = unit_ids[i];
			Unit unit = Unit(db, id);
			if (unit.prefab() == null) {
				sb.append(LevelEditorApi.object_set_hidden(id, db.get_bool(id, db.property_index(id, STRING_ID_64(Level.OBJECT_HIDDEN_KEY, 0xd378c492cdcff388)), false)));
				sb.append(LevelEditorApi.object_set_selectable(id, !db.get_bool(id, db.property_index(id, STRING_ID_64(Level.OBJECT_LOCKED_KEY, 0x3b9b1b9d1ccaf2e8)), false)));
			}
		}
	}

	public static void spawn_unit(StringBuilder sb, Guid unit_id, Database db)
	{
		Unit unit = Unit(db, unit_id);
		string? prefab = unit.prefab();

		if (prefab != null) {
			sb.append(LevelEditorApi.spawn_unit(unit_id
				, prefab
				, unit.local_position()
				));
			generate_change_commands(sb, { unit_id }, db);
		} else {
			sb.append(LevelEditorApi.spawn_empty_unit(unit_id));

			Guid?[] components = db.get_set(unit_id, db.property_index(unit_id, STRING_ID_64("components", 0xe71d1687374e5a54)));
			GLib.qsort_with_data<Guid?>(components, sizeof(Guid?), (a, b) => {
					double order_a = db.get_double(a, db.property_index(a, STRING_ID_64("spawn_order", 0x2f8a65a7213a6ef5)));
					double order_b = db.get_double(b, db.property_index(b, STRING_ID_64("spawn_order", 0x2f8a65a7213a6ef5)));
					return (int)(order_a - order_b);
				});

			foreach (unowned Guid? component_id in components)
				generate_add_component_commands(sb, unit_id, component_id, db);

			sb.append(LevelEditorApi.object_set_hidden(unit_id, db.get_bool(unit_id, db.property_index(unit_id, STRING_ID_64(Level.OBJECT_HIDDEN_KEY, 0xd378c492cdcff388)), false)));
			sb.append(LevelEditorApi.object_set_selectable(unit_id, !db.get_bool(unit_id, db.property_index(unit_id, STRING_ID_64(Level.OBJECT_LOCKED_KEY, 0x3b9b1b9d1ccaf2e8)), false)));
		}
	}

	public static void generate_mesh_material_commands(StringBuilder sb, Guid unit_id, Guid component_id, Database db)
	{
		Unit unit = Unit(db, unit_id);
		GLib.GenericSet<Guid?> bindings = (GLib.GenericSet<Guid?>)unit.get_component_property(component_id, unit._db.property_index(component_id, STRING_ID_64("data.materials", 0xb4c01840c957402d)), guid_set_new());
		foreach (Guid? binding_id in bindings) {
			if (!db.is_alive(binding_id))
				continue;
			string slot = db.get_string(binding_id, db.property_index(binding_id, STRING_ID_64("data.slot", 0x0de060e1cd2f27fe)));
			if (slot == "default")
				sb.append(LevelEditorApi.set_mesh_material(unit_id, slot, db.get_resource(binding_id, db.property_index(binding_id, STRING_ID_64("data.material", 0xf014ddbddc53c116)))));
		}
		foreach (Guid? binding_id in bindings) {
			if (!db.is_alive(binding_id))
				continue;
			string slot = db.get_string(binding_id, db.property_index(binding_id, STRING_ID_64("data.slot", 0x0de060e1cd2f27fe)));
			if (slot != "" && slot != "default")
				sb.append(LevelEditorApi.set_mesh_material(unit_id, slot, db.get_resource(binding_id, db.property_index(binding_id, STRING_ID_64("data.material", 0xf014ddbddc53c116)))));
		}
	}

	public static bool generate_lod_group_subobject_commands(StringBuilder sb, Guid object_id, Database db)
	{
		Guid component_id = db.owner(object_id);
		if (component_id == GUID_ZERO || db.object_type(component_id) != STRING_ID_64(OBJECT_TYPE_LOD_GROUP, 0x97993ef522a1a9f0))
			return false;

		generate_add_component_commands(sb, db.owner(component_id), component_id, db);
		return true;
	}

	public static bool generate_mesh_material_subobject_commands(StringBuilder sb, Guid object_id, Database db)
	{
		Guid component_id = db.owner(object_id);
		if (component_id == GUID_ZERO || db.object_type(component_id) != STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893))
			return false;

		generate_set_component_commands(sb, db.owner(component_id), component_id, db);
		return true;
	}

	public static int generate_spawn_unit_commands(StringBuilder sb, Guid?[] object_ids, Database db, bool respawn = false)
	{
		int i;

		for (i = 0; i < object_ids.length; ++i) {
			if (db.object_type(object_ids[i]) == STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f)) {
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
			} else if (!generate_lod_group_subobject_commands(sb, object_ids[i], db)
				&& !generate_mesh_material_subobject_commands(sb, object_ids[i], db)) {
				break;
			}
		}

		return i;
	}

	public static int generate_destroy_commands(StringBuilder sb, Guid?[] object_ids, Database db)
	{
		int i;

		for (i = 0; i < object_ids.length; ++i) {
			if (db.object_type(object_ids[i]) == STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f)) {
				sb.append(LevelEditorApi.destroy(object_ids[i]));
			} else if (is_component(object_ids[i], db)) {
				Guid component_id = object_ids[i];
				sb.append(LevelEditorApi.unit_destroy_component_type(db.owner(component_id), db.type_name(db.object_type(component_id))));
			} else if (!generate_lod_group_subobject_commands(sb, object_ids[i], db)
				&& !generate_mesh_material_subobject_commands(sb, object_ids[i], db)) {
				break;
			}
		}

		return i;
	}

	public static void generate_set_component_commands(StringBuilder sb, Guid unit_id, Guid component_id, Database db)
	{
		Unit unit = Unit(db, unit_id);
		StringId64 component_type = db.object_type(component_id);

		if (component_type == STRING_ID_64(OBJECT_TYPE_TRANSFORM, 0x69e14b13ad9b5315)) {
			sb.append(LevelEditorApi.move_object(unit_id
				, unit.get_component_vector3   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.position", 0xbf17708e8bd2a30b)))
				, unit.get_component_quaternion(component_id, unit._db.property_index(component_id, STRING_ID_64("data.rotation", 0x3c8974411eeaf63c)))
				, unit.get_component_vector3   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.scale", 0x16021af8bd4fea50)))
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_CAMERA, 0x60ed8c3931822dc7)) {
			sb.append(LevelEditorApi.set_camera(unit_id
				, unit.get_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.projection", 0x6e0676d6db8d3734)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.fov", 0xae3b0c9413994b89)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.far_range", 0x3283caec3b9511b0)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.near_range", 0xbaf158a47f2c7242)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.orthographic_size", 0x9c48de9cb4cdde6f)))
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893)) {
			sb.append(LevelEditorApi.set_mesh(unit_id
				, unit.get_component_resource(component_id, unit._db.property_index(component_id, STRING_ID_64("data.mesh_resource", 0xd08afb39c15cc158)))
				, unit.get_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.geometry_name", 0x040a59886a3e4a2d)))
				, unit.get_component_bool  (component_id, unit._db.property_index(component_id, STRING_ID_64("data.visible", 0xad0490cc1fffda48)))
				, unit.get_component_bool  (component_id, unit._db.property_index(component_id, STRING_ID_64("data.cast_shadows", 0xc2b3140260446694)), true)
				));
			generate_mesh_material_commands(sb, unit_id, component_id, db);
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_SPRITE_RENDERER, 0x3d7229ea6a1c2a3b)) {
			sb.append(LevelEditorApi.set_sprite(unit_id
				, unit.get_component_resource(component_id, unit._db.property_index(component_id, STRING_ID_64("data.sprite_resource", 0x223dd1c4e8beb8b0)))
				, unit.get_component_resource(component_id, unit._db.property_index(component_id, STRING_ID_64("data.material", 0xf014ddbddc53c116)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.layer", 0x90a95ce10ee6d6ed)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.depth", 0x0ccfd59731b887a6)))
				, unit.get_component_bool  (component_id, unit._db.property_index(component_id, STRING_ID_64("data.visible", 0xad0490cc1fffda48)))
				, unit.get_component_bool  (component_id, unit._db.property_index(component_id, STRING_ID_64("data.flip_x", 0x5b0c3f73c8bf9120)))
				, unit.get_component_bool  (component_id, unit._db.property_index(component_id, STRING_ID_64("data.flip_y", 0x85822f09e9cb4e87)))
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_LIGHT, 0x7dd7224fbb9f08c2)) {
			sb.append(LevelEditorApi.set_light(unit_id
				, unit.get_component_string (component_id, unit._db.property_index(component_id, STRING_ID_64("data.type", 0x3d4dcce26f0f13fd)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.range", 0x969effdaa778c82b)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.intensity", 0xd1e406b3aeae4d67)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.spot_angle", 0xad796e4ae667dead)))
				, unit.get_component_vector3(component_id, unit._db.property_index(component_id, STRING_ID_64("data.color", 0xac9624a15a2891c0)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.shadow_bias", 0xa8290b921c961845)), 0.0004)
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.shadow_bias_normal", 0x27c8d9bf59a6dff8)), 1.0)
				, unit.get_component_bool   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.cast_shadows", 0xc2b3140260446694)), true)
				, unit.get_component_resource(component_id, unit._db.property_index(component_id, STRING_ID_64("data.cookie", 0xe9199868645b3060)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.cookie_scale", 0xcb67dc4b1d86ba44)), 1.0)
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.cookie_x", 0x256474d6c2196cc5)), 0.0)
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.cookie_y", 0xf3edfef5cf63ab5a)), 0.0)
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_ANIMATION_STATE_MACHINE, 0x0d694773e87992ac)) {
			sb.append(LevelEditorApi.set_animation_state_machine(unit_id
				, unit.get_component_resource(component_id, unit._db.property_index(component_id, STRING_ID_64("data.state_machine_resource", 0xd98f8b4d76314483)))
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_COLLIDER, 0x9a9fe4362129d74e)) {
			generate_add_component_commands(sb, unit_id, component_id, db);
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_FOG, 0x063924dbf007ef0d)) {
			sb.append(LevelEditorApi.set_fog(unit_id
				, unit.get_component_vector3(component_id, unit._db.property_index(component_id, STRING_ID_64("data.color", 0xac9624a15a2891c0)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.density", 0xc54946c276726fb6)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.range_min", 0xbf6be34b866c5366)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.range_max", 0x792b1cb32fc03704)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.sun_blend", 0x5816ee4020741cf7)))
				, unit.get_component_bool   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.enabled", 0xe94d35860f381a41)))
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_GLOBAL_LIGHTING, 0xa002d9c1718af7fe)) {
			sb.append(LevelEditorApi.set_global_lighting(unit_id
				, unit.get_component_resource (component_id, unit._db.property_index(component_id, STRING_ID_64("data.skydome_map", 0x21390a5a0186c3d9)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.skydome_intensity", 0xac668fe1d8a8ffee)))
				, unit.get_component_vector3(component_id, unit._db.property_index(component_id, STRING_ID_64("data.ambient_color", 0x7bf3621acdc9d34b)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.shadow_distance", 0xf7011f881f6cf44c)), 100.0)
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_BLOOM, 0x28e4a7cb995dd31c)) {
			sb.append(LevelEditorApi.set_bloom(unit_id
				, unit.get_component_bool  (component_id, unit._db.property_index(component_id, STRING_ID_64("data.enabled", 0xe94d35860f381a41)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.threshold", 0xf196e4784078b3ed)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.weight", 0x45a7acbf247fe2bf)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.intensity", 0xd1e406b3aeae4d67)))
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_COLOR_GRADING, 0x18f07f17486057e5)) {
			sb.append(LevelEditorApi.set_color_grading(unit_id
				, unit.get_component_bool   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.enabled", 0xe94d35860f381a41)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.exposure_bias", 0x789f6aca1fbf35b5)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.contrast", 0x14cb1aa3781d5890)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.saturation", 0xf02ececf948a7d75)))
				, unit.get_component_vector3(component_id, unit._db.property_index(component_id, STRING_ID_64("data.color_filter", 0x65913769f14bc153)))
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_TONEMAP, 0xeda056ab7089b06b)) {
			sb.append(LevelEditorApi.set_tonemap(unit_id
				, unit.get_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.type", 0x3d4dcce26f0f13fd)))
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_VIGNETTE, 0xae5dfa90b77c3567)) {
			sb.append(LevelEditorApi.set_vignette(unit_id
				, unit.get_component_bool   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.enabled", 0xe94d35860f381a41)))
				, unit.get_component_vector3(component_id, unit._db.property_index(component_id, STRING_ID_64("data.color", 0xac9624a15a2891c0)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.strength", 0x160efa2da4cdb898)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.radius", 0x380298023aeac247)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.softness", 0xc978a6136471ea40)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.roundness", 0x52b1d03886e89bc7)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.center_x", 0x6a91eb6d1e54a28c)))
				, unit.get_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.center_y", 0x4cd541a241263696)))
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_SCRIPT, 0x297611d5d18f8ad6)) {
			/* No sync. */
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_ACTOR, 0xac2b738a374cf583)) {
			/* No sync. */
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_FIXED_JOINT, 0x69617f389ada03c1)
			|| component_type == STRING_ID_64(OBJECT_TYPE_HINGE_JOINT, 0xd8064886bb32c4e8)
			|| component_type == STRING_ID_64(OBJECT_TYPE_SPHERICAL_JOINT, 0xf69ceb04066c63e3)
			|| component_type == STRING_ID_64(OBJECT_TYPE_SPRING_JOINT, 0xcdf8657d6ccbbba7)
			|| component_type == STRING_ID_64(OBJECT_TYPE_LIMB_JOINT, 0x96f166437068fff7)
			|| component_type == STRING_ID_64(OBJECT_TYPE_D6_JOINT, 0x4f780f631099cc71)) {
			/* No sync. */
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_MOVER, 0x273ef5a1ac07d371)) {
			sb.append(LevelEditorApi.set_mover(unit_id
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.height", 0x4be7bd93a635d8b2)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.radius", 0x380298023aeac247)))
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.max_slope_angle", 0x19f6752be9bb9fba)))
				, unit.get_component_vector3(component_id, unit._db.property_index(component_id, STRING_ID_64("data.center", 0xfb5c19cf0dfd5c6c)))
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_LOD_GROUP, 0x97993ef522a1a9f0)) {
			sb.append(LevelEditorApi.set_lod_group(unit_id
				, unit.get_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.level", 0xd70ddfec12f72a7b)), -1.0)
				, unit.get_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.fade_mode", 0xd1e695426829933b)), "none")
				));
		} else if (component_type == STRING_ID_64(OBJECT_TYPE_ANIMATION_STATE_MACHINE, 0x0d694773e87992ac)) {
			/* No sync. */
		} else {
			logw("Unregistered component type `%s`".printf(db.type_name(component_type)));
		}
	}

	public static int generate_change_commands(StringBuilder sb, Guid?[] object_ids, Database db)
	{
		int i;

		for (i = 0; i < object_ids.length; ++i) {
			if (db.object_type(object_ids[i]) == STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f)) {
				Guid unit_id = object_ids[i];
				Unit unit = Unit(db, unit_id);

				sb.append(LevelEditorApi.move_object(unit_id
					, unit.local_position()
					, unit.local_rotation()
					, unit.local_scale()
					));

				_component_registry.foreach((component_type, value) => {
						Guid component_id;

						if (!unit.has_component(out component_id, component_type)) {
							sb.append(LevelEditorApi.unit_destroy_component_type(unit_id, db.type_name(component_type)));
							return;
						}

						generate_add_component_commands(sb, unit_id, component_id, db);
						generate_set_component_commands(sb, unit_id, component_id, db);
					});

				sb.append(LevelEditorApi.object_set_hidden(unit_id, db.get_bool(unit_id, db.property_index(unit_id, STRING_ID_64(Level.OBJECT_HIDDEN_KEY, 0xd378c492cdcff388)), false)));
				sb.append(LevelEditorApi.object_set_selectable(unit_id, !db.get_bool(unit_id, db.property_index(unit_id, STRING_ID_64(Level.OBJECT_LOCKED_KEY, 0x3b9b1b9d1ccaf2e8)), false)));
			} else if (Unit.is_component(object_ids[i], db)) {
				Guid component_id = object_ids[i];
				Guid unit_id = db.owner(component_id);
				generate_set_component_commands(sb, unit_id, component_id, db);
			} else if (!generate_lod_group_subobject_commands(sb, object_ids[i], db)
				&& !generate_mesh_material_subobject_commands(sb, object_ids[i], db)) {
				break;
			}
		}

		return i;
	}

	public static bool is_component(Guid id, Database db)
	{
		return (db.type_flags(db.object_type(id)) & ObjectTypeFlags.UNIT_COMPONENT) != 0;
	}

	public bool add_component_type_dependencies(StringId64 component_type)
	{
		Guid dummy;
		if (has_component(out dummy, component_type))
			return false;

		GLib.GenericArray<StringId64?> dependencies = Unit._component_registry[component_type];
		foreach (unowned StringId64? dependency in dependencies) {
			Guid dependency_component_id;
			if (!has_component(out dependency_component_id, dependency))
				add_component_type_dependencies(dependency);
		}

		add_component_type(component_type);
		return true;
	}
}

} /* namespace Crown */
