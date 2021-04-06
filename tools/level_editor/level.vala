/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gee;

namespace Crown
{
/// Manages objects in a level.
public class Level
{
	public Project _project;

	// Engine connections
	public ConsoleClient _client;

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

	public Level(Database db, ConsoleClient client, Project project)
	{
		_project = project;

		// Engine connections
		_client = client;

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
		string path = _project.resource_path_to_absolute_path(resource_path);

		FileStream fs = FileStream.open(path, "rb");
		if (fs == null)
			return 1;

		reset();
		int ret = _db.load_from_file(ref _id, fs, resource_path);
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

	public void spawn_unit(Guid id, string name, Vector3 pos, Quaternion rot, Vector3 scl)
	{
		on_unit_spawned(id, name, pos, rot, scl);
		send_spawn_units(new Guid[] { id });
	}

	public void destroy_objects(Guid[] ids)
	{
		Guid[] units = {};
		Guid[] sounds = {};

		foreach (Guid id in ids)
		{
			if (is_unit(id))
				units += id;
			else if (is_sound(id))
				sounds += id;
		}

		if (units.length > 0)
		{
			_db.add_restore_point((int)ActionType.DESTROY_UNIT, units);
			foreach (Guid id in units)
			{
				_db.remove_from_set(_id, "units", id);
				_db.destroy(id);
			}
		}

		if (sounds.length > 0)
		{
			_db.add_restore_point((int)ActionType.DESTROY_SOUND, sounds);
			foreach (Guid id in sounds)
			{
				_db.remove_from_set(_id, "sounds", id);
				_db.destroy(id);
			}
		}

		send_destroy_objects(ids);
	}

	public void move_selected_objects(Vector3 pos, Quaternion rot, Vector3 scl)
	{
		if (_selection.size == 0)
			return;

		Guid id = _selection.last();
		on_move_objects(new Guid[] { id }, new Vector3[] { pos }, new Quaternion[] { rot }, new Vector3[] { scl });
		send_move_objects(new Guid[] { id }, new Vector3[] { pos }, new Quaternion[] { rot }, new Vector3[] { scl });
	}

	public void duplicate_selected_objects()
	{
		if (_selection.size > 0)
		{
			Guid[] ids = new Guid[_selection.size];
			// FIXME
			{
				Guid?[] tmp = _selection.to_array();
				for (int i = 0; i < tmp.length; ++i)
					ids[i] = tmp[i];
			}
			Guid[] new_ids = new Guid[ids.length];

			for (int i = 0; i < new_ids.length; ++i)
				new_ids[i] = Guid.new_guid();

			duplicate_objects(ids, new_ids);
		}
	}

	public void destroy_selected_objects()
	{
		Guid[] ids = new Guid[_selection.size];
		// FIXME
		{
			Guid?[] tmp = _selection.to_array();
			for (int i = 0; i < tmp.length; ++i)
				ids[i] = tmp[i];
		}
		_selection.clear();

		destroy_objects(ids);
	}

	public void duplicate_objects(Guid[] ids, Guid[] new_ids)
	{
		_db.add_restore_point((int)ActionType.DUPLICATE_OBJECTS, new_ids);
		for (int i = 0; i < ids.length; ++i)
		{
			_db.duplicate(ids[i], new_ids[i]);

			if (is_unit(ids[i]))
			{
				_db.add_to_set(_id, "units", new_ids[i]);
			}
			else if (is_sound(ids[i]))
			{
				_db.add_to_set(_id, "sounds", new_ids[i]);
			}
		}
		send_spawn_objects(new_ids);
		selection_set(ids);
	}

	public void on_unit_spawned(Guid id, string name, Vector3 pos, Quaternion rot, Vector3 scl)
	{
		_project.load_unit(name);

		_db.add_restore_point((int)ActionType.SPAWN_UNIT, new Guid[] { id });
		_db.create(id);
		_db.set_property_string(id, "editor.name", "unit_%04u".printf(_num_units++));
		_db.set_property_string(id, "prefab", name);

		Unit unit = new Unit(_db, id);
		Guid component_id;
		if (unit.has_component(out component_id, "transform"))
		{
			unit.set_component_property_vector3   (component_id, "data.position", pos);
			unit.set_component_property_quaternion(component_id, "data.rotation", rot);
			unit.set_component_property_vector3   (component_id, "data.scale", scl);
			unit.set_component_property_string    (component_id, "type", "transform");
		}
		else
		{
			_db.set_property_vector3   (id, "position", pos);
			_db.set_property_quaternion(id, "rotation", rot);
			_db.set_property_vector3   (id, "scale", scl);
		}
		_db.add_to_set(_id, "units", id);
	}

	public void on_sound_spawned(Guid id, string name, Vector3 pos, Quaternion rot, Vector3 scl, double range, double volume, bool loop)
	{
		_db.add_restore_point((int)ActionType.SPAWN_SOUND, new Guid[] { id });
		_db.create(id);
		_db.set_property_string    (id, "editor.name", "sound_%04u".printf(_num_sounds++));
		_db.set_property_vector3   (id, "position", pos);
		_db.set_property_quaternion(id, "rotation", rot);
		_db.set_property_string    (id, "name", name);
		_db.set_property_double    (id, "range", range);
		_db.set_property_double    (id, "volume", volume);
		_db.set_property_bool      (id, "loop", loop);
		_db.add_to_set(_id, "sounds", id);
	}

	public void on_move_objects(Guid[] ids, Vector3[] positions, Quaternion[] rotations, Vector3[] scales)
	{
		_db.add_restore_point((int)ActionType.MOVE_OBJECTS, ids);

		for (int i = 0; i < ids.length; ++i)
		{
			Guid id = ids[i];
			Vector3 pos = positions[i];
			Quaternion rot = rotations[i];
			Vector3 scl = scales[i];

			if (is_unit(id))
			{
				Unit unit = new Unit(_db, id);
				Guid component_id;
				if (unit.has_component(out component_id, "transform"))
				{
					unit.set_component_property_vector3   (component_id, "data.position", pos);
					unit.set_component_property_quaternion(component_id, "data.rotation", rot);
					unit.set_component_property_vector3   (component_id, "data.scale", scl);
				}
				else
				{
					_db.set_property_vector3   (id, "position", pos);
					_db.set_property_quaternion(id, "rotation", rot);
					_db.set_property_vector3   (id, "scale", scl);
				}
			}
			else if (is_sound(id))
			{
				_db.set_property_vector3   (id, "position", pos);
				_db.set_property_quaternion(id, "rotation", rot);
			}
		}
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

	public void selection_set(Guid[] ids)
	{
		_selection.clear();
		for (int i = 0; i < ids.length; ++i)
			_selection.add(ids[i]);

		send_selection();

		selection_changed(_selection);
	}

	public void send_selection()
	{
		_client.send_script(LevelEditorApi.selection_set(_selection.to_array()));
		_client.send(DeviceApi.frame());
	}

	public void set_light(Guid unit_id, Guid component_id, string type, double range, double intensity, double spot_angle, Vector3 color)
	{
		_db.add_restore_point((int)ActionType.SET_LIGHT, new Guid[] { unit_id });

		Unit unit = new Unit(_db, unit_id);
		unit.set_component_property_string (component_id, "data.type",       type);
		unit.set_component_property_double (component_id, "data.range",      range);
		unit.set_component_property_double (component_id, "data.intensity",  intensity);
		unit.set_component_property_double (component_id, "data.spot_angle", spot_angle);
		unit.set_component_property_vector3(component_id, "data.color",      color);
		unit.set_component_property_string (component_id, "type", "light");

		_client.send_script(LevelEditorApi.set_light(unit_id, type, range, intensity, spot_angle, color));
		_client.send(DeviceApi.frame());
	}

	public void set_mesh(Guid unit_id, Guid component_id, string mesh_resource, string geometry, string material, bool visible)
	{
		_db.add_restore_point((int)ActionType.SET_MESH, new Guid[] { unit_id });

		Unit unit = new Unit(_db, unit_id);
		unit.set_component_property_string(component_id, "data.mesh_resource", mesh_resource);
		unit.set_component_property_string(component_id, "data.geometry_name", geometry);
		unit.set_component_property_string(component_id, "data.material", material);
		unit.set_component_property_bool  (component_id, "data.visible", visible);
		unit.set_component_property_string(component_id, "type", "mesh_renderer");

		_client.send_script(LevelEditorApi.set_mesh(unit_id, 0 /*instance_id*/, material, visible));
		_client.send(DeviceApi.frame());
	}

	public void set_sprite(Guid unit_id, Guid component_id, double layer, double depth, string material, string sprite_resource, bool visible)
	{
		_db.add_restore_point((int)ActionType.SET_SPRITE, new Guid[] { unit_id });

		Unit unit = new Unit(_db, unit_id);
		unit.set_component_property_double(component_id, "data.layer", layer);
		unit.set_component_property_double(component_id, "data.depth", depth);
		unit.set_component_property_string(component_id, "data.material", material);
		unit.set_component_property_string(component_id, "data.sprite_resource", sprite_resource);
		unit.set_component_property_bool  (component_id, "data.visible", visible);
		unit.set_component_property_string(component_id, "type", "sprite_renderer");

		_client.send_script(LevelEditorApi.set_sprite(unit_id, material, layer, depth, visible));
		_client.send(DeviceApi.frame());
	}

	public void set_camera(Guid unit_id, Guid component_id, string projection, double fov, double near_range, double far_range)
	{
		_db.add_restore_point((int)ActionType.SET_CAMERA, new Guid[] { unit_id });

		Unit unit = new Unit(_db, unit_id);
		unit.set_component_property_string(component_id, "data.projection", projection);
		unit.set_component_property_double(component_id, "data.fov", fov);
		unit.set_component_property_double(component_id, "data.near_range", near_range);
		unit.set_component_property_double(component_id, "data.far_range", far_range);
		unit.set_component_property_string(component_id, "type", "camera");

		_client.send_script(LevelEditorApi.set_camera(unit_id, projection, fov, near_range, far_range));
		_client.send(DeviceApi.frame());
	}

	public void set_collider(Guid unit_id, Guid component_id, string shape, string scene, string name)
	{
		_db.add_restore_point((int)ActionType.SET_COLLIDER, new Guid[] { unit_id });

		Unit unit = new Unit(_db, unit_id);
		unit.set_component_property_string(component_id, "data.shape", shape);
		unit.set_component_property_string(component_id, "data.scene", scene);
		unit.set_component_property_string(component_id, "data.name", name);
		unit.set_component_property_string(component_id, "type", "collider");

		// No synchronization.
	}

	public void set_actor(Guid unit_id, Guid component_id, string class, string collision_filter, string material, double mass)
	{
		_db.add_restore_point((int)ActionType.SET_ACTOR, new Guid[] { unit_id });

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
		unit.set_component_property_string(component_id, "type", "actor");

		// No synchronization.
	}

	public void set_script(Guid unit_id, Guid component_id, string script_resource)
	{
		_db.add_restore_point((int)ActionType.SET_SCRIPT, new Guid[] { unit_id });

		Unit unit = new Unit(_db, unit_id);
		unit.set_component_property_string(component_id, "data.script_resource", script_resource);
		unit.set_component_property_string(component_id, "type", "script");

		// No synchronization.
	}

	public void set_animation_state_machine(Guid unit_id, Guid component_id, string state_machine_resource)
	{
		_db.add_restore_point((int)ActionType.SET_ANIMATION_STATE_MACHINE, new Guid[] { unit_id });

		Unit unit = new Unit(_db, unit_id);
		unit.set_component_property_string(component_id, "data.state_machine_resource", state_machine_resource);
		unit.set_component_property_string(component_id, "type", "animation_state_machine");

		// No synchronization.
	}

	public void set_sound(Guid sound_id, string name, double range, double volume, bool loop)
	{
		_db.add_restore_point((int)ActionType.SET_SOUND, new Guid[] { sound_id });

		_db.set_property_string(sound_id, "name", name);
		_db.set_property_double(sound_id, "range", range);
		_db.set_property_double(sound_id, "volume", volume);
		_db.set_property_bool  (sound_id, "loop", loop);

		_client.send_script(LevelEditorApi.set_sound_range(sound_id, range));
		_client.send(DeviceApi.frame());
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
		_db.add_restore_point((int)ActionType.OBJECT_SET_EDITOR_NAME, new Guid[] { object_id });
		_db.set_property_string(object_id, "editor.name", name);

		object_editor_name_changed(object_id, name);
	}

	private void send_spawn_units(Guid[] ids)
	{
		StringBuilder sb = new StringBuilder();
		generate_spawn_unit_commands(ids, sb);
		_client.send_script(sb.str);
		_client.send(DeviceApi.frame());
	}

	private void send_spawn_sounds(Guid[] ids)
	{
		StringBuilder sb = new StringBuilder();
		generate_spawn_sound_commands(ids, sb);
		_client.send_script(sb.str);
		_client.send(DeviceApi.frame());
	}

	private void send_spawn_objects(Guid[] ids)
	{
		StringBuilder sb = new StringBuilder();
		for (int i = 0; i < ids.length; ++i)
		{
			if (is_unit(ids[i]))
			{
				generate_spawn_unit_commands(new Guid[] { ids[i] }, sb);
			}
			else if (is_sound(ids[i]))
			{
				generate_spawn_sound_commands(new Guid[] { ids[i] }, sb);
			}
		}
		_client.send_script(sb.str);
		_client.send(DeviceApi.frame());
	}

	private void send_destroy_objects(Guid[] ids)
	{
		StringBuilder sb = new StringBuilder();
		foreach (Guid id in ids)
			sb.append(LevelEditorApi.destroy(id));

		_client.send_script(sb.str);
		_client.send(DeviceApi.frame());
	}

	private void send_move_objects(Guid[] ids, Vector3[] positions, Quaternion[] rotations, Vector3[] scales)
	{
		StringBuilder sb = new StringBuilder();
		for (int i = 0; i < ids.length; ++i)
			sb.append(LevelEditorApi.move_object(ids[i], positions[i], rotations[i], scales[i]));

		_client.send_script(sb.str);
		_client.send(DeviceApi.frame());
	}

	public void send_level()
	{
		HashSet<Guid?> units  = _db.get_property_set(_id, "units", new HashSet<Guid?>());
		HashSet<Guid?> sounds = _db.get_property_set(_id, "sounds", new HashSet<Guid?>());

		Guid[] unit_ids = new Guid[units.size];
		Guid[] sound_ids = new Guid[sounds.size];

		// FIXME
		{
			Guid?[] tmp = units.to_array();
			for (int i = 0; i < tmp.length; ++i)
				unit_ids[i] = tmp[i];
		}
		// FIXME
		{
			Guid?[] tmp = sounds.to_array();
			for (int i = 0; i < tmp.length; ++i)
				sound_ids[i] = tmp[i];
		}

		StringBuilder sb = new StringBuilder();
		sb.append(LevelEditorApi.reset());
		generate_spawn_unit_commands(unit_ids, sb);
		generate_spawn_sound_commands(sound_ids, sb);
		_client.send_script(sb.str);

		send_selection();
		_client.send(DeviceApi.frame());
	}

	private void generate_spawn_unit_commands(Guid[] unit_ids, StringBuilder sb)
	{
		foreach (Guid unit_id in unit_ids)
		{
			Unit unit = new Unit(_db, unit_id);

			if (unit.has_prefab())
				_project.load_unit(_db.get_property_string(unit_id, "prefab"));

			sb.append(LevelEditorApi.spawn_empty_unit(unit_id));

			Guid component_id;
			if (unit.has_component(out component_id, "transform"))
			{
				string s = LevelEditorApi.add_tranform_component(unit_id
					, component_id
					, unit.get_component_property_vector3   (component_id, "data.position")
					, unit.get_component_property_quaternion(component_id, "data.rotation")
					, unit.get_component_property_vector3   (component_id, "data.scale")
					);
				sb.append(s);
			}
			if (unit.has_component(out component_id, "mesh_renderer"))
			{
				string s = LevelEditorApi.add_mesh_component(unit_id
					, component_id
					, unit.get_component_property_string(component_id, "data.mesh_resource")
					, unit.get_component_property_string(component_id, "data.geometry_name")
					, unit.get_component_property_string(component_id, "data.material")
					, unit.get_component_property_bool  (component_id, "data.visible")
					);
				sb.append(s);
			}
			if (unit.has_component(out component_id, "sprite_renderer"))
			{
				string s = LevelEditorApi.add_sprite_component(unit_id
					, component_id
					, unit.get_component_property_string(component_id, "data.sprite_resource")
					, unit.get_component_property_string(component_id, "data.material")
					, unit.get_component_property_double(component_id, "data.layer")
					, unit.get_component_property_double(component_id, "data.depth")
					, unit.get_component_property_bool  (component_id, "data.visible")
					);
				sb.append(s);
			}
			if (unit.has_component(out component_id, "light"))
			{
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
			if (unit.has_component(out component_id, "camera"))
			{
				string s = LevelEditorApi.add_camera_component(unit_id
					, component_id
					, unit.get_component_property_string(component_id, "data.projection")
					, unit.get_component_property_double(component_id, "data.fov")
					, unit.get_component_property_double(component_id, "data.far_range")
					, unit.get_component_property_double(component_id, "data.near_range")
					);
				sb.append(s);
			}
		}
	}

	private void generate_spawn_sound_commands(Guid[] sound_ids, StringBuilder sb)
	{
		foreach (Guid sound_id in sound_ids)
		{
			string s = LevelEditorApi.spawn_sound(sound_id
				, _db.get_property_string    (sound_id, "name")
				, _db.get_property_vector3   (sound_id, "position")
				, _db.get_property_quaternion(sound_id, "rotation")
				, _db.get_property_double    (sound_id, "range")
				, _db.get_property_double    (sound_id, "volume")
				, _db.get_property_bool      (sound_id, "loop")
				);
			sb.append(s);
		}
	}

	private void undo_redo_action(bool undo, int id, Guid[] data)
	{
		switch (id)
		{
		case (int)ActionType.SPAWN_UNIT:
			{
				if (undo)
					send_destroy_objects(data);
				else
					send_spawn_units(data);
			}
			break;

		case (int)ActionType.DESTROY_UNIT:
			{
				if (undo)
					send_spawn_units(data);
				else
					send_destroy_objects(data);
			}
			break;

		case (int)ActionType.SPAWN_SOUND:
			{
				if (undo)
					send_destroy_objects(data);
				else
					send_spawn_sounds(data);
			}
			break;

		case (int)ActionType.DESTROY_SOUND:
			{
				if (undo)
					send_spawn_sounds(data);
				else
					send_destroy_objects(data);
			}
			break;

		case (int)ActionType.MOVE_OBJECTS:
			{
				Guid[] ids = data;

				Vector3[] positions = new Vector3[ids.length];
				Quaternion[] rotations = new Quaternion[ids.length];
				Vector3[] scales = new Vector3[ids.length];

				for (int i = 0; i < ids.length; ++i)
				{
					if (is_unit(ids[i]))
					{
						Guid unit_id = ids[i];

						Unit unit = new Unit(_db, unit_id);
						Guid component_id;
						if (unit.has_component(out component_id, "transform"))
						{
							positions[i] = unit.get_component_property_vector3   (component_id, "data.position");
							rotations[i] = unit.get_component_property_quaternion(component_id, "data.rotation");
							scales[i]    = unit.get_component_property_vector3   (component_id, "data.scale");
						}
						else
						{
							positions[i] = _db.get_property_vector3   (unit_id, "position");
							rotations[i] = _db.get_property_quaternion(unit_id, "rotation");
							scales[i]    = _db.get_property_vector3   (unit_id, "scale");
						}
					}
					else if (is_sound(ids[i]))
					{
						Guid sound_id = ids[i];
						positions[i] = _db.get_property_vector3   (sound_id, "position");
						rotations[i] = _db.get_property_quaternion(sound_id, "rotation");
						scales[i]    = Vector3(1.0, 1.0, 1.0);
					}
					else
					{
						assert(false);
					}
				}

				send_move_objects(ids, positions, rotations, scales);
				// FIXME: Hack to force update the properties view
				selection_changed(_selection);
			}
			break;

		case (int)ActionType.DUPLICATE_OBJECTS:
			{
				Guid[] new_ids = data;
				if (undo)
					send_destroy_objects(new_ids);
				else
					send_spawn_objects(new_ids);
			}
			break;

		case (int)ActionType.OBJECT_SET_EDITOR_NAME:
			object_editor_name_changed(data[0], object_editor_name(data[0]));
			break;

		case (int)ActionType.SET_LIGHT:
			{
				Guid unit_id = data[0];

				Unit unit = new Unit(_db, unit_id);
				Guid component_id;
				unit.has_component(out component_id, "light");

				_client.send_script(LevelEditorApi.set_light(unit_id
					, unit.get_component_property_string (component_id, "data.type")
					, unit.get_component_property_double (component_id, "data.range")
					, unit.get_component_property_double (component_id, "data.intensity")
					, unit.get_component_property_double (component_id, "data.spot_angle")
					, unit.get_component_property_vector3(component_id, "data.color")
					));
				// FIXME: Hack to force update the properties view
				selection_changed(_selection);
			}
			break;

		case (int)ActionType.SET_MESH:
			{
				Guid unit_id = data[0];

				Unit unit = new Unit(_db, unit_id);
				Guid component_id;
				unit.has_component(out component_id, "mesh_renderer");

				_client.send_script(LevelEditorApi.set_mesh(unit_id
					, 0/*instance_id*/
					, unit.get_component_property_string(component_id, "data.material")
					, unit.get_component_property_bool  (component_id, "data.visible")
					));
				// FIXME: Hack to force update the properties view
				selection_changed(_selection);
			}
			break;

		case (int)ActionType.SET_SPRITE:
			{
				Guid unit_id = data[0];

				Unit unit = new Unit(_db, unit_id);
				Guid component_id;
				unit.has_component(out component_id, "sprite_renderer");

				_client.send_script(LevelEditorApi.set_sprite(unit_id
					, unit.get_component_property_string(component_id, "data.material")
					, unit.get_component_property_double(component_id, "data.layer")
					, unit.get_component_property_double(component_id, "data.depth")
					, unit.get_component_property_bool  (component_id, "data.visible")
					));
				// FIXME: Hack to force update the properties view
				selection_changed(_selection);
			}
			break;

		case (int)ActionType.SET_CAMERA:
			{
				Guid unit_id = data[0];

				Unit unit = new Unit(_db, unit_id);
				Guid component_id;
				unit.has_component(out component_id, "camera");

				_client.send_script(LevelEditorApi.set_camera(unit_id
					, unit.get_component_property_string(component_id, "data.projection")
					, unit.get_component_property_double(component_id, "data.fov")
					, unit.get_component_property_double(component_id, "data.near_range")
					, unit.get_component_property_double(component_id, "data.far_range")
					));
				// FIXME: Hack to force update the properties view
				selection_changed(_selection);
			}
			break;

		case (int)ActionType.SET_COLLIDER:
		case (int)ActionType.SET_ACTOR:
		case (int)ActionType.SET_SCRIPT:
		case (int)ActionType.SET_ANIMATION_STATE_MACHINE:
			// FIXME: Hack to force update the properties view
			selection_changed(_selection);
			break;

		case (int)ActionType.SET_SOUND:
			{
				Guid sound_id = data[0];

				_client.send_script(LevelEditorApi.set_sound_range(sound_id
					, _db.get_property_double(sound_id, "range")
					));
				// FIXME: Hack to force update the properties view
				selection_changed(_selection);
			}
			break;

		default:
			loge("Unknown undo/redo action: %d".printf(id));
			break;
		}
	}

	public bool is_unit(Guid id)
	{
		return _db.get_property_set(_id, "units", new HashSet<Guid?>()).contains(id);
	}

	public bool is_sound(Guid id)
	{
		return _db.get_property_set(_id, "sounds", new HashSet<Guid?>()).contains(id);
	}
}

}
