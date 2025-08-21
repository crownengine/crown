/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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

	public Vector3 _camera_position;
	public Quaternion _camera_rotation;
	public double _camera_orthographic_size;
	public double _camera_target_distance;
	public CameraViewType _camera_view_type;

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
		_selection = new Gee.ArrayList<Guid?>(Guid.equal_func);

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

		_camera_position = Vector3(20, -20, 20);
		_camera_rotation = Quaternion.look(Vector3(-20, 20, -20).normalize(), Vector3(0.0, 0.0, 1.0));
		_camera_target_distance = _camera_position.length();
		_camera_orthographic_size = _camera_target_distance;
		_camera_view_type = CameraViewType.PERSPECTIVE;
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

		camera_decode();
		GLib.Application.get_default().activate_action("camera-view", new GLib.Variant.int32(_camera_view_type));

		_name = name;
		_path = path;

		// Level files loaded from outside the source directory can be visualized and
		// modified in-memory, but never overwritten on disk, because they might be
		// shared with other projects (e.g. toolchain). Ensure that _path is null to
		// force save functions to choose a different path (inside the source
		// directory).
		if (!_project.path_is_within_source_dir(path))
			_path = null;

		return 0;
	}

	public int save(string name)
	{
		string path = Path.build_filename(_project.source_dir(), name + ".level");

		camera_encode();
		int err = _db.save(path, _id);
		_path = path;
		_name = name;
		return err;
	}

	public void spawn_unit(string? name)
	{
		Guid id = Guid.new_guid();
		on_unit_spawned(id, name, VECTOR3_ZERO, QUATERNION_IDENTITY, VECTOR3_ONE);
		_db.add_restore_point((int)ActionType.CREATE_OBJECTS, new Guid?[] { id });

		selection_set(new Guid?[] { id });
	}

	public void destroy_objects(Guid?[] ids)
	{
		foreach (Guid id in ids) {
			_selection.remove(id);

			if (_db.object_type(id) == OBJECT_TYPE_UNIT) {
				_db.remove_from_set(_id, "units", id);
				_db.destroy(id);
			} else if (_db.object_type(id) == OBJECT_TYPE_SOUND_SOURCE) {
				_db.remove_from_set(_id, "sounds", id);
				_db.destroy(id);
			}
		}

		_db.add_restore_point((int)ActionType.DESTROY_OBJECTS, ids);
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
		_db.add_restore_point((int)ActionType.CREATE_OBJECTS, new_ids);

		selection_set(ids);
	}

	public void on_unit_spawned(Guid id, string? name, Vector3 pos, Quaternion rot, Vector3 scl)
	{
		Unit unit = Unit(_db, id);
		unit.create(name, pos, rot, scl);

		_db.set_object_name(id, "unit_%04u".printf(_num_units++));
		_db.add_to_set(_id, "units", id);
	}

	public void on_sound_spawned(Guid id, string name, Vector3 pos, Quaternion rot, Vector3 scl, double range, double volume, bool loop)
	{
		Sound sound = Sound(_db, id);
		sound.create(name, pos, rot, scl, range, volume, loop);

		_db.set_object_name(id, "sound_%04u".printf(_num_sounds++));
		_db.add_to_set(_id, "sounds", id);
	}

	public void on_move_objects(Guid?[] ids, Vector3[] positions, Quaternion[] rotations, Vector3[] scales)
	{
		for (int i = 0; i < ids.length; ++i) {
			if (_db.object_type(ids[i]) == OBJECT_TYPE_UNIT) {
				Unit unit = Unit(_db, ids[i]);
				unit.set_local_position(positions[i]);
				unit.set_local_rotation(rotations[i]);
				unit.set_local_scale(scales[i]);
			} else if (_db.object_type(ids[i]) == OBJECT_TYPE_SOUND_SOURCE) {
				Sound sound = Sound(_db, ids[i]);
				sound.set_local_position(positions[i]);
				sound.set_local_rotation(rotations[i]);
				sound.set_local_scale(scales[i]);
			}
		}
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

	public string object_editor_name(Guid object_id)
	{
		return _db.object_name(object_id);
	}

	public void object_set_editor_name(Guid object_id, string name)
	{
		_db.set_object_name(object_id, name);
		_db.add_restore_point((int)ActionType.OBJECT_SET_EDITOR_NAME, new Guid?[] { object_id });
	}

	public void generate_spawn_objects(StringBuilder sb, Guid?[] object_ids)
	{
		int i = 0;
		while (i < object_ids.length) {
			if (_db.object_type(object_ids[i]) == OBJECT_TYPE_UNIT) {
				i += Unit.generate_spawn_unit_commands(sb, object_ids[i:object_ids.length], _db);
			} else if (_db.object_type(object_ids[i]) == OBJECT_TYPE_SOUND_SOURCE) {
				i += Sound.generate_spawn_sound_commands(sb, object_ids[i:object_ids.length], _db);
			} else {
				++i; // Skip object.
			}
		}
	}

	public void generate_destroy_objects(StringBuilder sb, Guid?[] object_ids)
	{
		int i = 0;
		while (i < object_ids.length) {
			if (_db.object_type(object_ids[i]) == OBJECT_TYPE_UNIT) {
				i += Unit.generate_destroy_commands(sb, object_ids[i:object_ids.length], _db);
			} else if (_db.object_type(object_ids[i]) == OBJECT_TYPE_SOUND_SOURCE) {
				sb.append(LevelEditorApi.destroy(object_ids[i]));
				++i;
			} else {
				++i; // Skip object.
			}
		}
	}

	public void generate_change_objects(StringBuilder sb, Guid?[] object_ids)
	{
		int i = 0;
		while (i < object_ids.length) {
			if (_db.object_type(object_ids[i]) == OBJECT_TYPE_UNIT) {
				i += Unit.generate_change_commands(sb, object_ids[i:object_ids.length], _db);
			} else if (_db.object_type(object_ids[i]) == OBJECT_TYPE_SOUND_SOURCE) {
				i += Sound.generate_change_sound_commands(sb, object_ids[i:object_ids.length], _db);
			} else {
				++i; // Skip object.
			}
		}
	}

	public void send_level()
	{
		Gee.ArrayList<Guid?> unit_ids = new Gee.ArrayList<Guid?>();
		Gee.ArrayList<Guid?> sound_ids = new Gee.ArrayList<Guid?>();
		units(ref unit_ids);
		sounds(ref sound_ids);

		StringBuilder sb = new StringBuilder();
		sb.append(LevelEditorApi.reset());
		Unit.generate_spawn_unit_commands(sb, unit_ids.to_array(), _db);
		Sound.generate_spawn_sound_commands(sb, sound_ids.to_array(), _db);
		sb.append(LevelEditorApi.spawn_skydome(_db.get_property_string(_id, "skydome_unit", "core/units/skydome/skydome")));
		_runtime.send_script(sb.str);

		send_selection();
		send_camera();
	}

	public void units(ref Gee.ArrayList<Guid?> ids)
	{
		Gee.HashSet<Guid?> units = _db.get_property_set(_id, "units", new Gee.HashSet<Guid?>());
		ids.add_all(units);
	}

	public void sounds(ref Gee.ArrayList<Guid?> ids)
	{
		Gee.HashSet<Guid?> sounds = _db.get_property_set(_id, "sounds", new Gee.HashSet<Guid?>());
		ids.add_all(sounds);
	}

	public void objects(ref Gee.ArrayList<Guid?> ids)
	{
		units(ref ids);
		sounds(ref ids);
	}

	public void camera_decode()
	{
		string properties[] =
		{
			"editor.camera.position",
			"editor.camera.rotation",
			"editor.camera.target_distance",
			"editor.camera.orthographic_size",
			"editor.camera.view_type"
		};

		foreach (var p in properties) {
			if (!_db.has_property(_id, p))
				return;
		}

		_camera_position = _db.get_property_vector3(_id, "editor.camera.position");
		_camera_rotation = _db.get_property_quaternion(_id, "editor.camera.rotation");
		_camera_target_distance = _db.get_property_double(_id, "editor.camera.target_distance");
		_camera_orthographic_size = _db.get_property_double(_id, "editor.camera.orthographic_size");
		_camera_view_type = (CameraViewType)_db.get_property_double(_id, "editor.camera.view_type");
	}

	public void camera_encode()
	{
		_db.set_property_internal(0, _id, "editor.camera.position", _camera_position);
		_db.set_property_internal(0, _id, "editor.camera.rotation", _camera_rotation);
		_db.set_property_internal(0, _id, "editor.camera.target_distance", _camera_target_distance);
		_db.set_property_internal(0, _id, "editor.camera.orthographic_size", _camera_orthographic_size);
		_db.set_property_internal(0, _id, "editor.camera.view_type", (double)_camera_view_type);
	}

	public void send_camera()
	{
		_runtime.send_script(LevelEditorApi.camera_restore(_camera_position
			, _camera_rotation
			, _camera_orthographic_size
			, _camera_target_distance
			, _camera_view_type
			));
	}

	public void on_camera(Hashtable msg)
	{
		_camera_position = Vector3.from_array((Gee.ArrayList<Value?>)msg["position"]);
		_camera_rotation = Quaternion.from_array((Gee.ArrayList<Value?>)msg["rotation"]);
		_camera_orthographic_size = (double)msg["orthographic_size"];
		_camera_target_distance = (double)msg["target_distance"];
	}
}

} /* namespace Crown */
