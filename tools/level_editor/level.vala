/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
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

	public Gee.HashMap<string, uint> _unit_names;
	public Gee.HashMap<string, uint> _sound_names;

	public string _name;
	public string _path;
	public Guid _id;

	public Level(Database db, RuntimeInstance runtime)
	{
		_project = db._project;

		// Engine connections
		_runtime = runtime;

		// Data
		_db = db;

		_unit_names = new Gee.HashMap<string, uint>();
		_sound_names = new Gee.HashMap<string, uint>();

		reset();
	}

	/// Resets the level
	public void reset()
	{
		_db.reset();

		_unit_names.clear();
		_sound_names.clear();

		_name = null;
		_path = null;
		_id = GUID_ZERO;
	}

	/// Loads the level @a level_name.
	public int load(string level_name)
	{
		string resource_path = level_name + ".level";
		string path = _project.absolute_path(resource_path);

		reset();

		int ret = _db.load_from_path(out _id, path, resource_path);
		if (ret != 0)
			return ret;

		// Level files loaded from outside the source directory can be visualized and
		// modified in-memory, but never overwritten on disk, because they might be
		// shared with other projects (e.g. toolchain). Ensure that _path is null to
		// force save functions to choose a different path (inside the source
		// directory).
		if (!_project.path_is_within_source_dir(path))
			_path = null;
		else
			_path = path;

		_name = level_name;
		return 0;
	}

	// Creates a new level based on a @a template_level_name.
	public int create(string template_level_name)
	{
		string resource_path = template_level_name + ".level";
		string path = _project.absolute_path(resource_path);

		reset();

		Guid template_id;
		int ret = _db.load_from_path(out template_id, path, resource_path);
		if (ret != 0)
			return ret;

		_id = Guid.new_guid();
		UndoRedo undo_redo = _db.disable_undo();
		_db.duplicate(template_id, _id);
		_db.restore_undo(undo_redo);

		_path = null;
		_name = template_level_name;
		return 0;
	}

	public int save(string name)
	{
		string path = Path.build_filename(_project.source_dir(), name + ".level");

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
	}

	public void replace_unit(Guid unit_id, string prefab_name)
	{
		string unit_editor_name = _db.name(unit_id);
		Unit unit = Unit(_db, unit_id);
		Vector3 unit_pos = unit.local_position();
		Quaternion unit_rot = unit.local_rotation();
		Vector3 unit_scl = unit.local_scale();

		_db.destroy(unit_id);
		_db.add_restore_point((int)ActionType.DESTROY_OBJECTS, { unit_id });

		Guid new_id = Guid.new_guid();
		Unit new_unit = Unit(_db, new_id);
		new_unit.create(prefab_name);
		new_unit.set_local_position(unit_pos);
		new_unit.set_local_rotation(unit_rot);
		new_unit.set_local_scale(unit_scl);

		_db.set_name(new_id, unit_editor_name);
		_db.add_to_set(_id, "units", new_id);
		_db.add_restore_point((int)ActionType.CREATE_OBJECTS, { new_id });
	}

	public string add_object_name(Gee.HashMap<string, uint> names, string resource_name)
	{
		string basename = GLib.Path.get_basename(resource_name);
		uint num = 0;

		if (!names.has_key(basename)) {
			names[basename] = 1;
		} else {
			num = names[basename];
			names.set(basename, num + 1);
		}

		return num > 0 ? "%s%u".printf(basename, num + 1) : basename;
	}

	public void on_unit_spawned(Guid id, string? name, Vector3 pos, Quaternion rot, Vector3 scl)
	{
		Unit unit = Unit(_db, id);
		unit.create(name);
		unit.set_local_position(pos);
		unit.set_local_rotation(rot);
		unit.set_local_scale(scl);

		_db.set_name(id, add_object_name(_unit_names, name != null ? name : "unit"));
		_db.add_to_set(_id, "units", id);
	}

	public void on_sound_spawned(Guid id, string name, Vector3 pos, Quaternion rot, Vector3 scl, double range, double volume, bool loop)
	{
		Sound sound = Sound(_db, id);
		sound.create(name, pos, rot, scl, range, volume, loop);

		_db.set_name(id, add_object_name(_sound_names, name));
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

	public void generate_spawn_objects(StringBuilder sb, Guid?[] object_ids)
	{
		int n;
		int i = 0;
		while (i < object_ids.length) {
			n = 0;
			n += Unit.generate_spawn_unit_commands(sb, object_ids[i:object_ids.length], _db);
			n += Sound.generate_spawn_sound_commands(sb, object_ids[i:object_ids.length], _db);
			i += n == 0 ? 1 : n;
		}
	}

	public void generate_destroy_objects(StringBuilder sb, Guid?[] object_ids)
	{
		int n;
		int i = 0;
		while (i < object_ids.length) {
			n = 0;
			n += Unit.generate_destroy_commands(sb, object_ids[i:object_ids.length], _db);
			n += Sound.generate_destroy_commands(sb, object_ids[i:object_ids.length], _db);
			i += n == 0 ? 1 : n;
		}
	}

	public void generate_change_objects(StringBuilder sb, Guid?[] object_ids)
	{
		int n;
		int i = 0;
		while (i < object_ids.length) {
			n = 0;
			n += Unit.generate_change_commands(sb, object_ids[i:object_ids.length], _db);
			n += Sound.generate_change_sound_commands(sb, object_ids[i:object_ids.length], _db);
			i += n == 0 ? 1 : n;
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
		sb.append(LevelEditorApi.spawn_skydome(_db.get_resource(_id, "skydome_unit", "core/units/skydome/skydome")));
		_runtime.send_script(sb.str);

		send_camera();
	}

	public void units(ref Gee.ArrayList<Guid?> ids)
	{
		Gee.HashSet<Guid?> units = _db.get_set(_id, "units", new Gee.HashSet<Guid?>());
		ids.add_all(units);
	}

	public void sounds(ref Gee.ArrayList<Guid?> ids)
	{
		Gee.HashSet<Guid?> sounds = _db.get_set(_id, "sounds", new Gee.HashSet<Guid?>());
		ids.add_all(sounds);
	}

	public void objects(ref Gee.ArrayList<Guid?> ids)
	{
		units(ref ids);
		sounds(ref ids);
	}

	public void send_camera()
	{
		_runtime.send_script(LevelEditorApi.camera_restore(_db.get_vector3(_id, "editor.camera.position")
			, _db.get_quaternion(_id, "editor.camera.rotation")
			, _db.get_double(_id, "editor.camera.orthographic_size")
			, _db.get_double(_id, "editor.camera.target_distance")
			, (CameraViewType)_db.get_double(_id, "editor.camera.view_type")
			));
	}

	public void on_camera(Hashtable msg)
	{
		_db.set(0, _id, "editor.camera.position", Vector3.from_array((Gee.ArrayList<Value?>)msg["position"]));
		_db.set(0, _id, "editor.camera.rotation", Quaternion.from_array((Gee.ArrayList<Value?>)msg["rotation"]));
		_db.set(0, _id, "editor.camera.orthographic_size", (double)msg["orthographic_size"]);
		_db.set(0, _id, "editor.camera.target_distance", (double)msg["target_distance"]);
	}
}

} /* namespace Crown */
