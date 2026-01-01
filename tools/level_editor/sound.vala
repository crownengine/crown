/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public struct Sound
{
	public Database _db;
	public Guid _id;

	public Sound(Database db, Guid id)
	{
		_db = db;
		_id = id;
	}

	public void create(string name, Vector3 pos, Quaternion rot, Vector3 scl, double range, double volume, bool loop)
	{
		_db.create(_id, OBJECT_TYPE_SOUND_SOURCE);

		_db.set_resource(_id, "name", name);
		set_local_position(pos);
		set_local_rotation(rot);
		set_local_scale(scl);
		set_range(range);
		set_volume(volume);
		set_loop(loop);
		set_group("music");
	}

	public Vector3 local_position()
	{
		return _db.get_vector3(_id, "position");
	}

	public Quaternion local_rotation()
	{
		return _db.get_quaternion(_id, "rotation");
	}

	public Vector3 local_scale()
	{
		return Vector3(1.0, 1.0, 1.0);
	}

	public void set_local_position(Vector3 position)
	{
		_db.set_vector3(_id, "position", position);
	}

	public void set_local_rotation(Quaternion rotation)
	{
		_db.set_quaternion(_id, "rotation", rotation);
	}

	public void set_local_scale(Vector3 scale)
	{
		// Do nothing.
	}

	public double range()
	{
		return _db.get_double(_id, "range");
	}

	public void set_range(double range)
	{
		_db.set_double(_id, "range", range);
	}

	public double volume()
	{
		return _db.get_double(_id, "volume");
	}

	public void set_volume(double volume)
	{
		_db.set_double(_id, "volume", volume);
	}

	public bool loop()
	{
		return _db.get_bool(_id, "loop");
	}

	public void set_loop(bool loop)
	{
		_db.set_bool(_id, "loop", loop);
	}

	public void set_group(string group)
	{
		_db.set_string(_id, "group", group);
	}

	public static int generate_spawn_sound_commands(StringBuilder sb, Guid?[] object_ids, Database db)
	{
		int i = 0;
		for (; i < object_ids.length; ++i) {
			if (db.object_type(object_ids[i]) != OBJECT_TYPE_SOUND_SOURCE)
				break;

			Guid id = object_ids[i];

			sb.append("editor_nv, editor_nq, editor_nm = Device.temp_count()");
			string s = LevelEditorApi.spawn_sound(id
				, db.get_resource  (id, "name")
				, db.get_vector3   (id, "position")
				, db.get_quaternion(id, "rotation")
				, db.get_double    (id, "range")
				, db.get_double    (id, "volume")
				, db.get_bool      (id, "loop")
				);
			sb.append(s);
			sb.append("Device.set_temp_count(editor_nv, editor_nq, editor_nm)");
		}

		return i;
	}

	public static int generate_destroy_commands(StringBuilder sb, Guid?[] object_ids, Database db)
	{
		int i = 0;
		for (; i < object_ids.length; ++i) {
			if (db.object_type(object_ids[i]) != OBJECT_TYPE_SOUND_SOURCE)
				break;

			sb.append(LevelEditorApi.destroy(object_ids[i]));
		}

		return i;
	}

	public static int generate_change_sound_commands(StringBuilder sb, Guid?[] object_ids, Database db)
	{
		int i = 0;
		for (; i < object_ids.length; ++i) {
			if (db.object_type(object_ids[i]) != OBJECT_TYPE_SOUND_SOURCE)
				break;

			Guid id = object_ids[i];
			Sound sound = Sound(db, id);

			sb.append("editor_nv, editor_nq, editor_nm = Device.temp_count()");
			sb.append(LevelEditorApi.move_object(id
				, sound.local_position()
				, sound.local_rotation()
				, sound.local_scale()
				));
			sb.append(LevelEditorApi.set_sound_range(id, sound.range()));
			sb.append("Device.set_temp_count(editor_nv, editor_nq, editor_nm)");
		}

		return i;
	}
}

} /* namespace Crown */
