/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
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
		Guid id = Guid.new_guid();
		on_unit_spawned(id, null, VECTOR3_ZERO, QUATERNION_IDENTITY, VECTOR3_ONE);
		_db.add_restore_point((int)ActionType.SPAWN_UNIT, new Guid?[] { id });

		selection_set(new Guid?[] { id });
	}

	public void destroy_objects(Guid?[] ids)
	{
		foreach (Guid id in ids) {
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

		selection_set(ids);
	}

	public void on_unit_spawned(Guid id, string? name, Vector3 pos, Quaternion rot, Vector3 scl)
	{
		Unit unit = new Unit(_db, id);
		unit.create(name, pos, rot, scl);

		_db.set_property_string(id, "editor.name", "unit_%04u".printf(_num_units++));
		_db.add_to_set(_id, "units", id);
	}

	public void on_sound_spawned(Guid id, string name, Vector3 pos, Quaternion rot, Vector3 scl, double range, double volume, bool loop)
	{
		Sound sound = new Sound(_db, id);
		sound.create(name, pos, rot, scl, range, volume, loop);

		_db.set_property_string    (id, "editor.name", "sound_%04u".printf(_num_sounds++));
		_db.add_to_set(_id, "sounds", id);
	}

	public void on_move_objects(Guid?[] ids, Vector3[] positions, Quaternion[] rotations, Vector3[] scales)
	{
		for (int i = 0; i < ids.length; ++i) {
			if (_db.object_type(ids[i]) == OBJECT_TYPE_UNIT) {
				Unit unit = new Unit(_db, ids[i]);
				unit.set_local_position(positions[i]);
				unit.set_local_rotation(rotations[i]);
				unit.set_local_scale(scales[i]);
			} else if (_db.object_type(ids[i]) == OBJECT_TYPE_SOUND_SOURCE) {
				Sound sound = new Sound(_db, ids[i]);
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
		if (_db.has_property(object_id, "editor.name"))
			return _db.get_property_string(object_id, "editor.name");
		else
			return "<unnamed>";
	}

	public void object_set_editor_name(Guid object_id, string name)
	{
		_db.set_property_string(object_id, "editor.name", name);
		_db.add_restore_point((int)ActionType.OBJECT_SET_EDITOR_NAME, new Guid?[] { object_id });
	}

	public void send_spawn_objects(Guid?[] ids)
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

	public void send_destroy_objects(Guid?[] ids)
	{
		StringBuilder sb = new StringBuilder();
		foreach (Guid id in ids)
			sb.append(LevelEditorApi.destroy(id));

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
}

} /* namespace Crown */
