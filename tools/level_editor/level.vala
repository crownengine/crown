/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gee;

namespace Crown
{
/// Manages objects in a level.
public class Level
{
	public Project _project;

	// Engine connections
	public RuntimeInstance _runtime;

	// Data
	public Database _db;
	public Gee.ArrayList<Guid?> _selection;

	public uint _num_units;
	public uint _num_sounds;

	public string _name;
	public string _path;
	public Guid _id;

	// Signals
	public signal void selection_changed(Gee.ArrayList<Guid?> selection);
	public signal void object_editor_name_changed(Guid object_id, string name);

	public Level(Database db, RuntimeInstance runtime, Project project)
	{
		_project = project;

		// Engine connections
		_runtime = runtime;

		// Data
		_db = db;
		_db.undo_redo.connect(undo_redo_action);

		_selection = new Gee.ArrayList<Guid?>();

		reset();
	}

	/// Resets the level
	public void reset()
	{
		_db.reset();

		_selection.clear();
		selection_changed(_selection);

		_num_units = 0;
		_num_sounds = 0;

		_name = null;
		_path = null;
		_id = GUID_ZERO;
	}

	public int load_from_path(string name)
	{
		string resource_path = name + ".level";
		string path = _project.absolute_path(resource_path);

		FileStream fs = FileStream.open(path, "rb");
		if (fs == null)
			return 1;

		reset();
		int ret = _db.load_from_file(out _id, fs, resource_path);
		if (ret != 0)
			return ret;

		_name = name;
		_path = path;

		// Level files loaded from outside the source directory can be visualized and
		// modified in-memory, but never overwritten on disk, because they might be
		// shared with other projects (e.g. toolchain). Ensure that _path is null to
		// force save functions to choose a different path (inside the source
		// directory).
		if (!_project.path_is_within_source_dir(path))
			_path = null;

		// FIXME: hack to keep the LevelTreeView working.
		_db.key_changed(_id, "units");

		return 0;
	}

	public void save(string name)
	{
		string path = Path.build_filename(_project.source_dir(), name + ".level");

		_db.save(path, _id);
		_path = path;
		_name = name;
	}

	public void spawn_empty_unit()
	{
		StringBuilder sb = new StringBuilder();
		Guid id = Guid.new_guid();
		on_unit_spawned(id, null, VECTOR3_ZERO, QUATERNION_IDENTITY, VECTOR3_ONE);
		generate_spawn_unit_commands(new Guid?[] { id }, sb);
		_runtime.send_script(sb.str);
		selection_set(new Guid?[] { id });
	}

	public void destroy_objects(Guid?[] ids)
	{
		Guid?[] units = {};
		Guid?[] sounds = {};

		foreach (Guid id in ids) {
			if (_db.object_type(id) == OBJECT_TYPE_UNIT)
				units += id;
			else if (_db.object_type(id) == OBJECT_TYPE_SOUND_SOURCE)
				sounds += id;
		}

		if (units.length > 0) {
			foreach (Guid id in units) {
				_db.remove_from_set(_id, "units", id);
				_db.destroy(id);
			}
			_db.add_restore_point((int)ActionType.DESTROY_UNIT, units);
		}

		if (sounds.length > 0) {
			foreach (Guid id in sounds) {
				_db.remove_from_set(_id, "sounds", id);
				_db.destroy(id);
			}
			_db.add_restore_point((int)ActionType.DESTROY_SOUND, sounds);
		}

		send_destroy_objects(ids);
	}

	public void move_selected_objects(Vector3 pos, Quaternion rot, Vector3 scl)
	{
		if (_selection.size == 0)
			return;

		Guid id = _selection.last();
		on_move_objects(new Guid?[] { id }, new Vector3[] { pos }, new Quaternion[] { rot }, new Vector3[] { scl });
		send_move_objects(new Guid?[] { id }, new Vector3[] { pos }, new Quaternion[] { rot }, new Vector3[] { scl });
		_runtime.send(DeviceApi.frame());
	}

	public void duplicate_selected_objects()
	{
		if (_selection.size > 0) {
			Guid?[] ids = _selection.to_array();

			Guid?[] new_ids = new Guid?[ids.length];
			for (int i = 0; i < new_ids.length; ++i)
				new_ids[i] = Guid.new_guid();

			duplicate_objects(ids, new_ids);
		}
	}

	public void destroy_selected_objects()
	{
		destroy_objects(_selection.to_array());
		_selection.clear();
	}

	public void duplicate_objects(Guid?[] ids, Guid?[] new_ids)
	{
		for (int i = 0; i < ids.length; ++i) {
			_db.duplicate(ids[i], new_ids[i]);

			if (_db.object_type(ids[i]) == OBJECT_TYPE_UNIT) {
				_db.add_to_set(_id, "units", new_ids[i]);
			} else if (_db.object_type(ids[i]) == OBJECT_TYPE_SOUND_SOURCE) {
				_db.add_to_set(_id, "sounds", new_ids[i]);
			}
		}
		_db.add_restore_point((int)ActionType.DUPLICATE_OBJECTS, new_ids);

		send_spawn_objects(new_ids);
		selection_set(ids);
	}

	public void on_unit_spawned(Guid id, string? name, Vector3 pos, Quaternion rot, Vector3 scl)
	{
		_db.create(id, OBJECT_TYPE_UNIT);
		_db.set_property_string(id, "editor.name", "unit_%04u".printf(_num_units++));

		if (name != null) {
			_db.set_property_string(id, "prefab", name);
		}

		Unit unit = new Unit(_db, id);
		Guid component_id;
		if (unit.has_component(out component_id, OBJECT_TYPE_TRANSFORM)) {
			unit.set_component_property_vector3   (component_id, "data.position", pos);
			unit.set_component_property_quaternion(component_id, "data.rotation", rot);
			unit.set_component_property_vector3   (component_id, "data.scale", scl);
			unit.set_component_property_string    (component_id, "type", OBJECT_TYPE_TRANSFORM);
		} else {
			_db.set_property_vector3   (id, "position", pos);
			_db.set_property_quaternion(id, "rotation", rot);
			_db.set_property_vector3   (id, "scale", scl);
		}
		_db.add_to_set(_id, "units", id);
		_db.add_restore_point((int)ActionType.SPAWN_UNIT, new Guid?[] { id });
	}

	public void on_sound_spawned(Guid id, string name, Vector3 pos, Quaternion rot, Vector3 scl, double range, double volume, bool loop)
	{
		_db.create(id, OBJECT_TYPE_SOUND_SOURCE);
		_db.set_property_string    (id, "editor.name", "sound_%04u".printf(_num_sounds++));
		_db.set_property_vector3   (id, "position", pos);
		_db.set_property_quaternion(id, "rotation", rot);
		_db.set_property_string    (id, "name", name);
		_db.set_property_double    (id, "range", range);
		_db.set_property_double    (id, "volume", volume);
		_db.set_property_bool      (id, "loop", loop);
		_db.add_to_set(_id, "sounds", id);
		_db.add_restore_point((int)ActionType.SPAWN_SOUND, new Guid?[] { id });
	}

	public void on_move_objects(Guid?[] ids, Vector3[] positions, Quaternion[] rotations, Vector3[] scales)
	{
		for (int i = 0; i < ids.length; ++i) {
			Guid id = ids[i];
			Vector3 pos = positions[i];
			Quaternion rot = rotations[i];
			Vector3 scl = scales[i];

			if (_db.object_type(id) == OBJECT_TYPE_UNIT) {
				Unit unit = new Unit(_db, id);
				Guid component_id;
				if (unit.has_component(out component_id, OBJECT_TYPE_TRANSFORM)) {
					unit.set_component_property_vector3   (component_id, "data.position", pos);
					unit.set_component_property_quaternion(component_id, "data.rotation", rot);
					unit.set_component_property_vector3   (component_id, "data.scale", scl);
				} else {
					_db.set_property_vector3   (id, "position", pos);
					_db.set_property_quaternion(id, "rotation", rot);
					_db.set_property_vector3   (id, "scale", scl);
				}
			} else if (_db.object_type(id) == OBJECT_TYPE_SOUND_SOURCE) {
				_db.set_property_vector3   (id, "position", pos);
				_db.set_property_quaternion(id, "rotation", rot);
			}
		}
		_db.add_restore_point((int)ActionType.MOVE_OBJECTS, ids);

		// FIXME: Hack to force update the properties view
		selection_changed(_selection);
	}

	public void on_selection(Guid[] ids)
	{
		_selection.clear();
		foreach (Guid id in ids)
			_selection.add(id);

		selection_changed(_selection);
	}

	public void selection_set(Guid?[] ids)
	{
		_selection.clear();
		for (int i = 0; i < ids.length; ++i)
			_selection.add(ids[i]);

		send_selection();
		_runtime.send(DeviceApi.frame());

		selection_changed(_selection);
	}

	public void send_selection()
	{
		_runtime.send_script(LevelEditorApi.selection_set(_selection.to_array()));
	}

	public void set_light(Guid unit_id, Guid component_id, string type, double range, double intensity, double spot_angle, Vector3 color)
	{
		Unit unit = new Unit(_db, unit_id);
		unit.set_component_property_string (component_id, "data.type",       type);
		unit.set_component_property_double (component_id, "data.range",      range);
		unit.set_component_property_double (component_id, "data.intensity",  intensity);
		unit.set_component_property_double (component_id, "data.spot_angle", spot_angle);
		unit.set_component_property_vector3(component_id, "data.color",      color);
		unit.set_component_property_string (component_id, "type", OBJECT_TYPE_LIGHT);
		_db.add_restore_point((int)ActionType.SET_LIGHT, new Guid?[] { unit_id });

		_runtime.send_script(LevelEditorApi.set_light(unit_id, type, range, intensity, spot_angle, color));
		_runtime.send(DeviceApi.frame());
	}

	public void set_mesh(Guid unit_id, Guid component_id, string mesh_resource, string geometry, string material, bool visible)
	{
		Unit unit = new Unit(_db, unit_id);
		unit.set_component_property_string(component_id, "data.mesh_resource", mesh_resource);
		unit.set_component_property_string(component_id, "data.geometry_name", geometry);
		unit.set_component_property_string(component_id, "data.material", material);
		unit.set_component_property_bool  (component_id, "data.visible", visible);
		unit.set_component_property_string(component_id, "type", OBJECT_TYPE_MESH_RENDERER);
		_db.add_restore_point((int)ActionType.SET_MESH, new Guid?[] { unit_id });

		_runtime.send_script(LevelEditorApi.set_mesh(unit_id, material, visible));
		_runtime.send(DeviceApi.frame());
	}

	public void set_sprite(Guid unit_id, Guid component_id, double layer, double depth, string material, string sprite_resource, bool visible)
	{
		Unit unit = new Unit(_db, unit_id);
		unit.set_component_property_double(component_id, "data.layer", layer);
		unit.set_component_property_double(component_id, "data.depth", depth);
		unit.set_component_property_string(component_id, "data.material", material);
		unit.set_component_property_string(component_id, "data.sprite_resource", sprite_resource);
		unit.set_component_property_bool  (component_id, "data.visible", visible);
		unit.set_component_property_string(component_id, "type", OBJECT_TYPE_SPRITE_RENDERER);
		_db.add_restore_point((int)ActionType.SET_SPRITE, new Guid?[] { unit_id });

		_runtime.send_script(LevelEditorApi.set_sprite(unit_id, sprite_resource, material, layer, depth, visible));
		_runtime.send(DeviceApi.frame());
	}

	public void set_camera(Guid unit_id, Guid component_id, string projection, double fov, double near_range, double far_range)
	{
		Unit unit = new Unit(_db, unit_id);
		unit.set_component_property_string(component_id, "data.projection", projection);
		unit.set_component_property_double(component_id, "data.fov", fov);
		unit.set_component_property_double(component_id, "data.near_range", near_range);
		unit.set_component_property_double(component_id, "data.far_range", far_range);
		unit.set_component_property_string(component_id, "type", OBJECT_TYPE_CAMERA);
		_db.add_restore_point((int)ActionType.SET_CAMERA, new Guid?[] { unit_id });

		_runtime.send_script(LevelEditorApi.set_camera(unit_id, projection, fov, near_range, far_range));
		_runtime.send(DeviceApi.frame());
	}

	public void set_collider(Guid unit_id, Guid component_id, string shape, string scene, string name)
	{
		Unit unit = new Unit(_db, unit_id);
		unit.set_component_property_string(component_id, "data.shape", shape);
		unit.set_component_property_string(component_id, "data.scene", scene);
		unit.set_component_property_string(component_id, "data.name", name);
		unit.set_component_property_string(component_id, "type", OBJECT_TYPE_COLLIDER);
		_db.add_restore_point((int)ActionType.SET_COLLIDER, new Guid?[] { unit_id });

		// No synchronization.
	}

	public void set_actor(Guid unit_id, Guid component_id, string class, string collision_filter, string material, double mass)
	{
		Unit unit = new Unit(_db, unit_id);
		unit.set_component_property_string(component_id, "data.class", class);
		unit.set_component_property_string(component_id, "data.collision_filter", collision_filter);
		unit.set_component_property_string(component_id, "data.material", material);
		unit.set_component_property_double(component_id, "data.mass", mass);
		unit.set_component_property_bool  (component_id, "data.lock_rotation_x", (bool)unit.get_component_property_bool(component_id, "data.lock_rotation_x"));
		unit.set_component_property_bool  (component_id, "data.lock_rotation_y", (bool)unit.get_component_property_bool(component_id, "data.lock_rotation_y"));
		unit.set_component_property_bool  (component_id, "data.lock_rotation_z", (bool)unit.get_component_property_bool(component_id, "data.lock_rotation_z"));
		unit.set_component_property_bool  (component_id, "data.lock_translation_x", (bool)unit.get_component_property_bool(component_id, "data.lock_translation_x"));
		unit.set_component_property_bool  (component_id, "data.lock_translation_y", (bool)unit.get_component_property_bool(component_id, "data.lock_translation_y"));
		unit.set_component_property_bool  (component_id, "data.lock_translation_z", (bool)unit.get_component_property_bool(component_id, "data.lock_translation_z"));
		unit.set_component_property_string(component_id, "type", OBJECT_TYPE_ACTOR);
		_db.add_restore_point((int)ActionType.SET_ACTOR, new Guid?[] { unit_id });

		// No synchronization.
	}

	public void set_script(Guid unit_id, Guid component_id, string script_resource)
	{
		Unit unit = new Unit(_db, unit_id);
		unit.set_component_property_string(component_id, "data.script_resource", script_resource);
		unit.set_component_property_string(component_id, "type", OBJECT_TYPE_SCRIPT);
		_db.add_restore_point((int)ActionType.SET_SCRIPT, new Guid?[] { unit_id });

		// No synchronization.
	}

	public void set_animation_state_machine(Guid unit_id, Guid component_id, string state_machine_resource)
	{
		Unit unit = new Unit(_db, unit_id);
		unit.set_component_property_string(component_id, "data.state_machine_resource", state_machine_resource);
		unit.set_component_property_string(component_id, "type", OBJECT_TYPE_ANIMATION_STATE_MACHINE);
		_db.add_restore_point((int)ActionType.SET_ANIMATION_STATE_MACHINE, new Guid?[] { unit_id });

		// No synchronization.
	}

	public void set_sound(Guid sound_id, string name, double range, double volume, bool loop)
	{
		_db.set_property_string(sound_id, "name", name);
		_db.set_property_double(sound_id, "range", range);
		_db.set_property_double(sound_id, "volume", volume);
		_db.set_property_bool  (sound_id, "loop", loop);
		_db.add_restore_point((int)ActionType.SET_SOUND, new Guid?[] { sound_id });

		_runtime.send_script(LevelEditorApi.set_sound_range(sound_id, range));
		_runtime.send(DeviceApi.frame());
	}

	public string object_editor_name(Guid object_id)
	{
		if (_db.has_property(object_id, "editor.name"))
			return _db.get_property_string(object_id, "editor.name");
		else
			return "<unnamed>";
	}

	public void object_set_editor_name(Guid object_id, string name)
	{
		_db.set_property_string(object_id, "editor.name", name);
		_db.add_restore_point((int)ActionType.OBJECT_SET_EDITOR_NAME, new Guid?[] { object_id });

		object_editor_name_changed(object_id, name);
	}

	private void send_spawn_units(Guid?[] ids)
	{
		StringBuilder sb = new StringBuilder();
		generate_spawn_unit_commands(ids, sb);
		_runtime.send_script(sb.str);
	}

	private void send_spawn_sounds(Guid?[] ids)
	{
		StringBuilder sb = new StringBuilder();
		generate_spawn_sound_commands(ids, sb);
		_runtime.send_script(sb.str);
	}

	private void send_spawn_objects(Guid?[] ids)
	{
		StringBuilder sb = new StringBuilder();
		for (int i = 0; i < ids.length; ++i) {
			if (_db.object_type(ids[i]) == OBJECT_TYPE_UNIT) {
				generate_spawn_unit_commands(new Guid?[] { ids[i] }, sb);
			} else if (_db.object_type(ids[i]) == OBJECT_TYPE_SOUND_SOURCE) {
				generate_spawn_sound_commands(new Guid?[] { ids[i] }, sb);
			}
		}
		_runtime.send_script(sb.str);
	}

	private void send_destroy_objects(Guid?[] ids)
	{
		StringBuilder sb = new StringBuilder();
		foreach (Guid id in ids)
			sb.append(LevelEditorApi.destroy(id));

		_runtime.send_script(sb.str);
	}

	private void send_move_objects(Guid?[] ids, Vector3[] positions, Quaternion[] rotations, Vector3[] scales)
	{
		StringBuilder sb = new StringBuilder();
		for (int i = 0; i < ids.length; ++i)
			sb.append(LevelEditorApi.move_object(ids[i], positions[i], rotations[i], scales[i]));

		_runtime.send_script(sb.str);
	}

	public void send_level()
	{
		Gee.ArrayList<Guid?> unit_ids = new Gee.ArrayList<Guid?>();
		Gee.ArrayList<Guid?> sound_ids = new Gee.ArrayList<Guid?>();
		units(ref unit_ids);
		sounds(ref sound_ids);

		StringBuilder sb = new StringBuilder();
		sb.append(LevelEditorApi.reset());
		generate_spawn_unit_commands(unit_ids.to_array(), sb);
		generate_spawn_sound_commands(sound_ids.to_array(), sb);
		_runtime.send_script(sb.str);

		send_selection();
	}

	public void units(ref Gee.ArrayList<Guid?> ids)
	{
		HashSet<Guid?> units = _db.get_property_set(_id, "units", new HashSet<Guid?>());
		ids.add_all(units);
	}

	public void sounds(ref Gee.ArrayList<Guid?> ids)
	{
		HashSet<Guid?> sounds = _db.get_property_set(_id, "sounds", new HashSet<Guid?>());
		ids.add_all(sounds);
	}

	public void objects(ref Gee.ArrayList<Guid?> ids)
	{
		units(ref ids);
		sounds(ref ids);
	}

	private void generate_spawn_unit_commands(Guid?[] unit_ids, StringBuilder sb)
	{
		foreach (Guid unit_id in unit_ids) {
			Unit unit = new Unit(_db, unit_id);

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
	}

	private void generate_spawn_sound_commands(Guid?[] sound_ids, StringBuilder sb)
	{
		foreach (Guid id in sound_ids) {
			string s = LevelEditorApi.spawn_sound(id
				, _db.get_property_string    (id, "name")
				, _db.get_property_vector3   (id, "position")
				, _db.get_property_quaternion(id, "rotation")
				, _db.get_property_double    (id, "range")
				, _db.get_property_double    (id, "volume")
				, _db.get_property_bool      (id, "loop")
				);
			sb.append(s);
		}
	}

	private void undo_redo_action(bool undo, uint32 id, Guid?[] data)
	{
		switch (id) {
		case (int)ActionType.SPAWN_UNIT:
			if (undo)
				send_destroy_objects(data);
			else
				send_spawn_units(data);

			_runtime.send(DeviceApi.frame());
			break;

		case (int)ActionType.DESTROY_UNIT:
			if (undo)
				send_spawn_units(data);
			else
				send_destroy_objects(data);

			_runtime.send(DeviceApi.frame());
			break;

		case (int)ActionType.SPAWN_SOUND:
			if (undo)
				send_destroy_objects(data);
			else
				send_spawn_sounds(data);

			_runtime.send(DeviceApi.frame());
			break;

		case (int)ActionType.DESTROY_SOUND:
			if (undo)
				send_spawn_sounds(data);
			else
				send_destroy_objects(data);

			_runtime.send(DeviceApi.frame());
			break;

		case (int)ActionType.MOVE_OBJECTS: {
			Guid?[] ids = data;

			Vector3[] positions = new Vector3[ids.length];
			Quaternion[] rotations = new Quaternion[ids.length];
			Vector3[] scales = new Vector3[ids.length];

			for (int i = 0; i < ids.length; ++i) {
				if (_db.object_type(ids[i]) == OBJECT_TYPE_UNIT) {
					Guid unit_id = ids[i];

					Unit unit = new Unit(_db, unit_id);
					Guid component_id;
					if (unit.has_component(out component_id, OBJECT_TYPE_TRANSFORM)) {
						positions[i] = unit.get_component_property_vector3   (component_id, "data.position");
						rotations[i] = unit.get_component_property_quaternion(component_id, "data.rotation");
						scales[i]    = unit.get_component_property_vector3   (component_id, "data.scale");
					} else {
						positions[i] = _db.get_property_vector3   (unit_id, "position");
						rotations[i] = _db.get_property_quaternion(unit_id, "rotation");
						scales[i]    = _db.get_property_vector3   (unit_id, "scale");
					}
				} else if (_db.object_type(ids[i]) == OBJECT_TYPE_SOUND_SOURCE) {
					Guid sound_id = ids[i];
					positions[i] = _db.get_property_vector3   (sound_id, "position");
					rotations[i] = _db.get_property_quaternion(sound_id, "rotation");
					scales[i]    = Vector3(1.0, 1.0, 1.0);
				} else {
					assert(false);
				}
			}

			send_move_objects(ids, positions, rotations, scales);
			_runtime.send(DeviceApi.frame());
			// FIXME: Hack to force update the properties view
			selection_changed(_selection);
			break;
		}

		case (int)ActionType.DUPLICATE_OBJECTS: {
			Guid?[] new_ids = data;
			if (undo)
				send_destroy_objects(new_ids);
			else
				send_spawn_objects(new_ids);

			_runtime.send(DeviceApi.frame());
			break;
		}

		case (int)ActionType.OBJECT_SET_EDITOR_NAME:
			object_editor_name_changed(data[0], object_editor_name(data[0]));
			break;

		case (int)ActionType.SET_LIGHT: {
			Guid unit_id = data[0];

			Unit unit = new Unit(_db, unit_id);
			Guid component_id;
			unit.has_component(out component_id, OBJECT_TYPE_LIGHT);

			_runtime.send_script(LevelEditorApi.set_light(unit_id
				, unit.get_component_property_string (component_id, "data.type")
				, unit.get_component_property_double (component_id, "data.range")
				, unit.get_component_property_double (component_id, "data.intensity")
				, unit.get_component_property_double (component_id, "data.spot_angle")
				, unit.get_component_property_vector3(component_id, "data.color")
				));
			_runtime.send(DeviceApi.frame());
			// FIXME: Hack to force update the properties view
			selection_changed(_selection);
			break;
		}

		case (int)ActionType.SET_MESH: {
			Guid unit_id = data[0];

			Unit unit = new Unit(_db, unit_id);
			Guid component_id;
			unit.has_component(out component_id, OBJECT_TYPE_MESH_RENDERER);

			_runtime.send_script(LevelEditorApi.set_mesh(unit_id
				, unit.get_component_property_string(component_id, "data.material")
				, unit.get_component_property_bool  (component_id, "data.visible")
				));
			_runtime.send(DeviceApi.frame());
			// FIXME: Hack to force update the properties view
			selection_changed(_selection);
			break;
		}

		case (int)ActionType.SET_SPRITE: {
			Guid unit_id = data[0];

			Unit unit = new Unit(_db, unit_id);
			Guid component_id;
			unit.has_component(out component_id, OBJECT_TYPE_SPRITE_RENDERER);

			_runtime.send_script(LevelEditorApi.set_sprite(unit_id
				, unit.get_component_property_string(component_id, "data.sprite_resource")
				, unit.get_component_property_string(component_id, "data.material")
				, unit.get_component_property_double(component_id, "data.layer")
				, unit.get_component_property_double(component_id, "data.depth")
				, unit.get_component_property_bool  (component_id, "data.visible")
				));
			_runtime.send(DeviceApi.frame());
			// FIXME: Hack to force update the properties view
			selection_changed(_selection);
			break;
		}

		case (int)ActionType.SET_CAMERA: {
			Guid unit_id = data[0];

			Unit unit = new Unit(_db, unit_id);
			Guid component_id;
			unit.has_component(out component_id, OBJECT_TYPE_CAMERA);

			_runtime.send_script(LevelEditorApi.set_camera(unit_id
				, unit.get_component_property_string(component_id, "data.projection")
				, unit.get_component_property_double(component_id, "data.fov")
				, unit.get_component_property_double(component_id, "data.near_range")
				, unit.get_component_property_double(component_id, "data.far_range")
				));
			_runtime.send(DeviceApi.frame());
			// FIXME: Hack to force update the properties view
			selection_changed(_selection);
			break;
		}

		case (int)ActionType.SET_COLLIDER:
		case (int)ActionType.SET_ACTOR:
		case (int)ActionType.SET_SCRIPT:
		case (int)ActionType.SET_ANIMATION_STATE_MACHINE:
			_runtime.send(DeviceApi.frame());
			// FIXME: Hack to force update the properties view
			selection_changed(_selection);
			break;

		case (int)ActionType.SET_SOUND: {
			Guid sound_id = data[0];

			_runtime.send_script(LevelEditorApi.set_sound_range(sound_id
				, _db.get_property_double(sound_id, "range")
				));
			_runtime.send(DeviceApi.frame());
			// FIXME: Hack to force update the properties view
			selection_changed(_selection);
			break;
		}

		default:
			loge("Unknown undo/redo action: %u".printf(id));
			break;
		}
	}
}

} /* namespace Crown */
