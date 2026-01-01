/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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
	public static int load_unit(out Guid prefab_id, Database db, string name)
	{
		return db.add_from_resource_path(out prefab_id, name + ".unit");
	}

	public void create_empty()
	{
		_db.create(_id, OBJECT_TYPE_UNIT);
	}

	public void create(string? prefab)
	{
		create_empty();
		_db.set_resource(_id, "prefab", prefab);
	}

	public Value? get_component_property(Guid component_id, string key, Value? deffault = null)
	{
		Value? val;

		// Search in components
		val = _db.get_property(_id, "components");
		if (val != null) {
			if (((Gee.HashSet<Guid?>)val).contains(component_id))
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
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((Gee.HashSet<Guid?>)components).contains(component_id)) {
			_db.set_bool(component_id, key, val);
			return;
		}

		_db.set_bool(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_double(Guid component_id, string key, double val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((Gee.HashSet<Guid?>)components).contains(component_id)) {
			_db.set_double(component_id, key, val);
			return;
		}

		_db.set_double(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_string(Guid component_id, string key, string val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((Gee.HashSet<Guid?>)components).contains(component_id)) {
			_db.set_string(component_id, key, val);
			return;
		}

		_db.set_string(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_vector3(Guid component_id, string key, Vector3 val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((Gee.HashSet<Guid?>)components).contains(component_id)) {
			_db.set_vector3(component_id, key, val);
			return;
		}

		_db.set_vector3(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_quaternion(Guid component_id, string key, Quaternion val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((Gee.HashSet<Guid?>)components).contains(component_id)) {
			_db.set_quaternion(component_id, key, val);
			return;
		}

		_db.set_quaternion(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_resource(Guid component_id, string key, string? val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((Gee.HashSet<Guid?>)components).contains(component_id)) {
			_db.set_resource(component_id, key, val);
			return;
		}

		_db.set_resource(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_reference(Guid component_id, string key, Guid val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((Gee.HashSet<Guid?>)components).contains(component_id)) {
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
			foreach (Guid id in (Gee.HashSet<Guid?>)val) {
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
			scale = _db.get_vector3(_id, "scale");

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
				_db.add_restore_point((int)ActionType.DESTROY_OBJECTS, { component_id });
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
			_component_registry = new Hashtable();
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

			Guid?[] components = db.get_set(unit_id, "components", new Gee.HashSet<Guid?>()).to_array();

			Gee.ArrayList<int> spawn_order = new Gee.ArrayList<int>();
			for (int i = 0; i < components.length; ++i)
				spawn_order.add(i);

			spawn_order.sort((a, b) => {
					double order_a = db.get_double(components[a], "spawn_order");
					double order_b = db.get_double(components[b], "spawn_order");
					return (int)(order_a - order_b);
				});

			for (int i = 0; i < spawn_order.size; ++i) {
				Guid component_id = components[spawn_order[i]];
				generate_add_component_commands(sb, unit_id, component_id, db);
			}
		}

		sb.append("Device.set_temp_count(editor_nv, editor_nq, editor_nm)");
	}

	public static int generate_spawn_unit_commands(StringBuilder sb, Guid?[] object_ids, Database db)
	{
		int i;

		for (i = 0; i < object_ids.length; ++i) {
			if (db.object_type(object_ids[i]) == OBJECT_TYPE_UNIT) {
				if (!db.is_alive(object_ids[i]))
					continue;

				spawn_unit(sb, object_ids[i], db);

				Unit unit = Unit(db, object_ids[i]);
				if (db.has_property(unit._id, "children")) {
					Gee.HashSet<Guid?> children = db.get_set(unit._id, "children", new Gee.HashSet<Guid?>());
					generate_spawn_unit_commands(sb, children.to_array(), db);
				}

				Guid owner_id = db.owner(object_ids[i]);
				if (owner_id != GUID_ZERO && db.object_type(owner_id) == OBJECT_TYPE_UNIT)
					sb.append(LevelEditorApi.unit_set_parent(owner_id, object_ids[i]));
			} else if (Unit.is_component(object_ids[i], db)) {
				if (!db.is_alive(object_ids[i]))
					continue;

				Guid component_id = object_ids[i];
				Guid unit_id = db.owner(component_id);
				generate_add_component_commands(sb, unit_id, component_id, db);
			} else {
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
			} else {
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
		} else if (component_type == OBJECT_TYPE_MOVER) {
			sb.append(LevelEditorApi.set_mover(unit_id
				, unit.get_component_double(component_id, "data.height")
				, unit.get_component_double(component_id, "data.radius")
				, unit.get_component_double(component_id, "data.max_slope_angle")
				, unit.get_component_vector3(component_id, "data.center")
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

				_component_registry.foreach((g) => {
						string component_type = g.key;
						Guid component_id;

						if (!unit.has_component(out component_id, component_type)) {
							sb.append(LevelEditorApi.unit_destroy_component_type(unit_id, component_type));
							return true;
						}

						generate_add_component_commands(sb, unit_id, component_id, db);
						generate_set_component_commands(sb, unit_id, component_id, db);
						return true;
					});
			} else if (Unit.is_component(object_ids[i], db)) {
				Guid component_id = object_ids[i];
				Guid unit_id = db.owner(component_id);
				generate_set_component_commands(sb, unit_id, component_id, db);
			} else {
				break;
			}
		}

		return i;
	}

	public static bool is_component(Guid id, Database db)
	{
		return (db.type_flags(StringId64(db.object_type(id))) & ObjectTypeFlags.UNIT_COMPONENT) != 0;
	}

	public void add_component_type_dependencies(ref Gee.ArrayList<Guid?> components_added, string component_type)
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
