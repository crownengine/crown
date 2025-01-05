/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class Sound
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

		_db.set_property_string(_id, "name", name);
		set_local_position(pos);
		set_local_rotation(rot);
		set_local_scale(scl);
		set_range(range);
		set_volume(volume);
		set_loop(loop);
	}

	public Vector3 local_position()
	{
		return _db.get_property_vector3(_id, "position");
	}

	public Quaternion local_rotation()
	{
		return _db.get_property_quaternion(_id, "rotation");
	}

	public Vector3 local_scale()
	{
		return Vector3(1.0, 1.0, 1.0);
	}

	public void set_local_position(Vector3 position)
	{
		_db.set_property_vector3(_id, "position", position);
	}

	public void set_local_rotation(Quaternion rotation)
	{
		_db.set_property_quaternion(_id, "rotation", rotation);
	}

	public void set_local_scale(Vector3 scale)
	{
		// Do nothing.
	}

	public double range()
	{
		return _db.get_property_double(_id, "range");
	}

	public void set_range(double range)
	{
		_db.set_property_double(_id, "range", range);
	}

	public double volume()
	{
		return _db.get_property_double(_id, "volume");
	}

	public void set_volume(double volume)
	{
		_db.set_property_double(_id, "volume", volume);
	}

	public bool loop()
	{
		return _db.get_property_bool(_id, "loop");
	}

	public void set_loop(bool loop)
	{
		_db.set_property_bool(_id, "loop", loop);
	}

	public static int generate_spawn_sound_commands(StringBuilder sb, Guid?[] object_ids, Database db)
	{
		int i = 0;
		for (; i < object_ids.length; ++i) {
			if (db.object_type(object_ids[i]) != OBJECT_TYPE_SOUND_SOURCE)
				break;

			Guid id = object_ids[i];

			string s = LevelEditorApi.spawn_sound(id
				, db.get_property_string    (id, "name")
				, db.get_property_vector3   (id, "position")
				, db.get_property_quaternion(id, "rotation")
				, db.get_property_double    (id, "range")
				, db.get_property_double    (id, "volume")
				, db.get_property_bool      (id, "loop")
				);
			sb.append(s);
		}

		return i;
	}

	public void send(RuntimeInstance runtime)
	{
		runtime.send_script(LevelEditorApi.move_object(_id
			, local_position()
			, local_rotation()
			, local_scale()
			));
		runtime.send_script(LevelEditorApi.set_sound_range(_id, range()));
	}
}

} /* namespace Crown */
