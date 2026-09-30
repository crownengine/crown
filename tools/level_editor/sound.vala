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

	public void create(Vector3 pos, Quaternion rot, Vector3 scl, double range, double volume, bool loop)
	{
		_db.create(_id, OBJECT_TYPE_SOUND_SOURCE);

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
		return _db.get_vector3(_id, _db.property_index(_id, STRING_ID_64("position", 0x8bbeb160190f613a)));
	}

	public Quaternion local_rotation()
	{
		return _db.get_quaternion(_id, _db.property_index(_id, STRING_ID_64("rotation", 0x2060566242789baa)));
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
		return _db.get_double(_id, _db.property_index(_id, STRING_ID_64("range", 0xc8beeb904f5e61ed)));
	}

	public void set_range(double range)
	{
		_db.set_double(_id, "range", range);
	}

	public double volume()
	{
		return _db.get_double(_id, _db.property_index(_id, STRING_ID_64("volume", 0x6ad9817aa72d9533)));
	}

	public void set_volume(double volume)
	{
		_db.set_double(_id, "volume", volume);
	}

	public bool loop()
	{
		return _db.get_bool(_id, _db.property_index(_id, STRING_ID_64("loop", 0x8c9c410e83f48bce)));
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

			string s = LevelEditorApi.spawn_sound(id
				, db.get_resource  (id, db.property_index(id, STRING_ID_64("name", 0xd4c943cba60c270b)))
				, db.get_vector3   (id, db.property_index(id, STRING_ID_64("position", 0x8bbeb160190f613a)))
				, db.get_quaternion(id, db.property_index(id, STRING_ID_64("rotation", 0x2060566242789baa)))
				, db.get_double    (id, db.property_index(id, STRING_ID_64("range", 0xc8beeb904f5e61ed)))
				, db.get_double    (id, db.property_index(id, STRING_ID_64("volume", 0x6ad9817aa72d9533)))
				, db.get_bool      (id, db.property_index(id, STRING_ID_64("loop", 0x8c9c410e83f48bce)))
				);
			sb.append(s);
			sb.append(LevelEditorApi.object_set_hidden(id, db.get_bool(id, db.property_index(id, STRING_ID_64(Level.OBJECT_HIDDEN_KEY, 0xd378c492cdcff388)), false)));
			sb.append(LevelEditorApi.object_set_selectable(id, !db.get_bool(id, db.property_index(id, STRING_ID_64(Level.OBJECT_LOCKED_KEY, 0x3b9b1b9d1ccaf2e8)), false)));
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

			sb.append(LevelEditorApi.move_object(id
				, sound.local_position()
				, sound.local_rotation()
				, sound.local_scale()
				));
			sb.append(LevelEditorApi.set_sound_range(id, sound.range()));
			sb.append(LevelEditorApi.object_set_hidden(id, db.get_bool(id, db.property_index(id, STRING_ID_64(Level.OBJECT_HIDDEN_KEY, 0xd378c492cdcff388)), false)));
			sb.append(LevelEditorApi.object_set_selectable(id, !db.get_bool(id, db.property_index(id, STRING_ID_64(Level.OBJECT_LOCKED_KEY, 0x3b9b1b9d1ccaf2e8)), false)));
		}

		return i;
	}
}

} /* namespace Crown */
