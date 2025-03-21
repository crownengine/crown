/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gee;

namespace Crown
{
public enum UndoRedoAction
{
	RESTORE_POINT = int.MAX;
}

public struct RestorePointHeader
{
	public uint32 id;
	public uint32 flags;
	public uint32 size;
	public uint32 num_guids;
}

public struct RestorePoint
{
	public RestorePointHeader header;
	public Guid?[] data;
}

public class Stack
{
	public uint32 _capacity;

	public uint8[] _data;
	public uint32 _read; // Position of the read/write head.
	public uint32 _size; // Size of the data written in the stack.
	public uint32 _last_write_restore_point_size; // Size when write_restore_point() was last called.

	///
	public Stack(uint32 capacity)
	{
		assert(capacity > 0);

		_capacity = capacity;
		_data = new uint8[_capacity];

		clear();
	}

	///
	public uint32 size()
	{
		return _size;
	}

	///
	public void clear()
	{
		_read = 0;
		_size = 0;
		_last_write_restore_point_size = 0;
	}

	// Copies @a data into @a destination.
	public void copy_data(uint8* destination, void* data, ulong len)
	{
		uint8* source = (uint8*)data;
		for (ulong ii = 0; ii < len; ++ii)
			destination[ii] = source[ii];
	}

	// Writes @a data into the current page.
	public void write_data_internal(uint8* data, uint32 len)
	{
		assert(data != null);

		uint32 bytes_left = len;
		uint32 bytes_avail;

		// Write the data that wraps around.
		while (bytes_left > (bytes_avail = _capacity - _read)) {
			copy_data(&_data[_read]
				, ((uint8*)data) + (len - bytes_left)
				, bytes_avail
				);
			_read = (_read + bytes_avail) % _capacity;
			_size = uint32.min(_capacity, _size + bytes_avail);

			bytes_left -= bytes_avail;
		}

		// Write the remaining data.
		copy_data(&_data[_read]
			, ((uint8*)data) + (len - bytes_left)
			, bytes_left
			);
		_read += bytes_left;
		_size = uint32.min(_capacity, _size + bytes_left);

		_last_write_restore_point_size += len;
	}

	// Wrapper to avoid casting sizeof() manually.
	public void write_data(void* data, ulong len)
	{
		write_data_internal((uint8*)data, (uint32)len);
	}

	public void read_data_internal(uint8* data, uint32 len)
	{
		assert(data != null);

		uint32 bytes_left = len;

		// Read the data that wraps around.
		while (bytes_left > _read) {
			copy_data(data + bytes_left - _read
				, &_data[0]
				, _read
				);
			bytes_left -= _read;
			_size -= _read;
			assert(_size <= _capacity);

			_read = _capacity;
		}

		// Read the remaining data.
		copy_data(data
			, &_data[_read - bytes_left]
			, bytes_left
			);
		_read -= bytes_left;
		_size -= bytes_left;
		assert(_size <= _capacity);
	}

	// Wrapper to avoid casting sizeof() manually.
	public void read_data(void* data, ulong size)
	{
		read_data_internal((uint8*)data, (uint32)size);
	}

	public void write_bool(bool a)
	{
		write_data(&a, sizeof(bool));
	}

	public void write_uint32(uint32 a)
	{
		write_data(&a, sizeof(uint32));
	}

	public void write_double(double a)
	{
		write_data(&a, sizeof(double));
	}

	public void write_string(string str)
	{
		uint32 len = str.length;
		write_data(&str.data[0], len);
		write_data(&len, sizeof(uint32));
	}

	public void write_guid(Guid a)
	{
		write_data(&a, sizeof(Guid));
	}

	public void write_vector3(Vector3 a)
	{
		write_data(&a, sizeof(Vector3));
	}

	public void write_quaternion(Quaternion a)
	{
		write_data(&a, sizeof(Quaternion));
	}

	public bool read_bool()
	{
		bool a = false;
		read_data(&a, sizeof(bool));
		return a;
	}

	public int read_int()
	{
		int a = 0;
		read_data(&a, sizeof(int));
		return a;
	}

	public uint32 read_uint32()
	{
		uint32 a = 0;
		read_data(&a, sizeof(uint32));
		return a;
	}

	public double read_double()
	{
		double a = 0;
		read_data(&a, sizeof(double));
		return a;
	}

	public Guid read_guid()
	{
		Guid a = GUID_ZERO;
		read_data(&a, sizeof(Guid));
		return a;
	}

	public Vector3 read_vector3()
	{
		Vector3 a = VECTOR3_ZERO;
		read_data(&a, sizeof(Vector3));
		return a;
	}

	public Quaternion read_quaternion()
	{
		Quaternion a = QUATERNION_IDENTITY;
		read_data(&a, sizeof(Quaternion));
		return a;
	}

	public string read_string()
	{
		uint32 len = 0;
		read_data(&len, sizeof(uint32));
		uint8[] str = new uint8[len + 1];
		read_data(str, len);
		str[len] = '\0';
		return (string)str;
	}

	public void write_create_action(uint32 action, Guid id, string type)
	{
		write_string(type);
		write_guid(id);
		write_uint32(action);
	}

	public void write_destroy_action(uint32 action, Guid id, string type)
	{
		write_string(type);
		write_guid(id);
		write_uint32(action);
	}

	public void write_set_property_null_action(uint32 action, Guid id, string key)
	{
		// No value to push
		write_string(key);
		write_guid(id);
		write_uint32(action);
	}

	public void write_set_property_bool_action(uint32 action, Guid id, string key, bool val)
	{
		write_bool(val);
		write_string(key);
		write_guid(id);
		write_uint32(action);
	}

	public void write_set_property_double_action(uint32 action, Guid id, string key, double val)
	{
		write_double(val);
		write_string(key);
		write_guid(id);
		write_uint32(action);
	}

	public void write_set_property_string_action(uint32 action, Guid id, string key, string val)
	{
		write_string(val);
		write_string(key);
		write_guid(id);
		write_uint32(action);
	}

	public void write_set_property_guid_action(uint32 action, Guid id, string key, Guid val)
	{
		write_guid(val);
		write_string(key);
		write_guid(id);
		write_uint32(action);
	}

	public void write_set_property_vector3_action(uint32 action, Guid id, string key, Vector3 val)
	{
		write_vector3(val);
		write_string(key);
		write_guid(id);
		write_uint32(action);
	}

	public void write_set_property_quaternion_action(uint32 action, Guid id, string key, Quaternion val)
	{
		write_quaternion(val);
		write_string(key);
		write_guid(id);
		write_uint32(action);
	}

	public void write_add_to_set_action(uint32 action, Guid id, string key, Guid item_id)
	{
		write_guid(item_id);
		write_string(key);
		write_guid(id);
		write_uint32(action);
	}

	public void write_remove_from_set_action(uint32 action, Guid id, string key, Guid item_id)
	{
		write_guid(item_id);
		write_string(key);
		write_guid(id);
		write_uint32(action);
	}

	public void write_restore_point(uint32 id, uint32 flags, Guid?[] data)
	{
		uint32 size = _last_write_restore_point_size;

		uint32 num_guids = data.length;
		for (uint32 i = 0; i < num_guids; ++i)
			write_guid(data[num_guids - 1 - i]);
		write_uint32(num_guids);
		write_uint32(size);
		write_uint32(flags);
		write_uint32(id);
		write_uint32(UndoRedoAction.RESTORE_POINT);

		_last_write_restore_point_size = 0;
	}

	public RestorePoint read_restore_point()
	{
		uint32 t = read_uint32();
		assert(t == UndoRedoAction.RESTORE_POINT);

		uint32 id = read_uint32();
		uint32 flags = read_uint32();
		uint32 size = read_uint32();
		uint32 num_guids = read_uint32();
		Guid?[] ids = new Guid?[num_guids];
		for (uint32 i = 0; i < num_guids; ++i)
			ids[i] = read_guid();

		RestorePointHeader rph = { id, flags, size, num_guids };
		return { rph, ids };
	}
}

public class UndoRedo
{
	public Stack _undo;
	public Stack _redo;

	///
	public UndoRedo(uint32 undo_redo_size = 0)
	{
		uint32 size = uint32.max(1024, undo_redo_size);
		_undo = new Stack(size);
		_redo = new Stack(size);
	}

	public void reset()
	{
		_undo.clear();
		_redo.clear();
	}
}

public class Database
{
	private static bool _debug = false;
	private static bool _debug_getters = false;

	private enum Action
	{
		CREATE,
		DESTROY,
		SET_PROPERTY_NULL,
		SET_PROPERTY_BOOL,
		SET_PROPERTY_DOUBLE,
		SET_PROPERTY_STRING,
		SET_PROPERTY_GUID,
		SET_PROPERTY_VECTOR3,
		SET_PROPERTY_QUATERNION,
		ADD_TO_SET,
		REMOVE_FROM_SET
	}

	// Data
	private HashMap<Guid?, HashMap<string, Value?>> _data;
	private UndoRedo? _undo_redo;
	private Project _project;
	// The number of changes to the database since the last successful state
	// synchronization (load(), save() etc.). If it is less than 0, the changes
	// came from undo(), otherwise they came from redo() or from regular calls to
	// create(), destroy(), set_*() etc. A value of 0 means there were no changes.
	public int _distance_from_last_sync;

	// Signals
	public signal void key_changed(Guid id, string key);
	public signal void object_created(Guid id);
	public signal void object_destroyed(Guid id);
	public signal void undo_redo(bool undo, uint32 id, Guid?[] data);
	public signal void restore_point_added(int id, Guid?[] data, uint32 flags);

	public Database(Project project, UndoRedo? undo_redo = null)
	{
		_data = new HashMap<Guid?, HashMap<string, Value?>>(Guid.hash_func, Guid.equal_func);
		_project = project;
		_undo_redo = undo_redo;

		reset();
	}

	/// Resets database to clean state.
	public void reset()
	{
		_data.clear();

		if (_undo_redo != null)
			_undo_redo.reset();

		_distance_from_last_sync = 0;

		// This is a special field which stores all objects
		_data[GUID_ZERO] = new HashMap<string, Value?>();
	}

	/// Returns whether the database has been changed since last call to Save().
	public bool changed()
	{
		return _distance_from_last_sync != 0;
	}

	/// Saves database to path without marking it as not changed.
	public int dump(string path, Guid id)
	{
		try {
			Hashtable json = encode(id);
			SJSON.save(json, path);
			return 0;
		} catch (JsonWriteError e) {
			return -1;
		}
	}

	/// Saves database to path.
	public void save(string path, Guid id)
	{
		dump(path, id);
		_distance_from_last_sync = 0;
	}

	// See: add_from_path().
	public int add_from_file(out Guid object_id, FileStream? fs, string resource_path)
	{
		try {
			Hashtable json = SJSON.load_from_file(fs);

			// Parse the object's ID or generate a new one if none is found.
			if (json.has_key("id"))
				object_id = Guid.parse((string)json["id"]);
			else if (json.has_key("_guid"))
				object_id = Guid.parse((string)json["_guid"]);
			else
				object_id = Guid.new_guid();

			create_internal(0, object_id);
			set_object_type(object_id, ResourceId.type(resource_path));

			decode_object(object_id, GUID_ZERO, "", json);

			// Create a mapping between the path and the object it has been loaded into.
			set_property_internal(0, GUID_ZERO, resource_path, object_id);
			return 0;
		} catch (JsonSyntaxError e) {
			object_id = GUID_ZERO;
			return -1;
		}
	}

	// Adds the object stored at @a path to the database.
	// This makes it possible to load multiple objects from distinct
	// paths in the same database. @a resource_path is used as a key in the
	// database to refer to the object that has been loaded. This is useful when
	// you do not have the object ID but only its path, as it is often the case
	// since resources use paths and not IDs to reference each other.
	public int add_from_path(out Guid object_id, string path, string resource_path)
	{
		object_id = GUID_ZERO;

		FileStream fs = FileStream.open(path, "rb");
		if (fs == null)
			return 1;

		return add_from_file(out object_id, fs, resource_path);
	}

	public int add_from_resource_path(out Guid object_id, string resource_path)
	{
		// If the resource is already loaded.
		if (has_property(GUID_ZERO, resource_path)) {
			object_id = get_property_guid(GUID_ZERO, resource_path);
			return 0;
		}

		string path = _project.absolute_path(resource_path);
		return add_from_path(out object_id, path, resource_path);
	}

	/// Loads the database with the object stored at @a path.
	public int load_from_file(out Guid object_id, FileStream fs, string resource_path)
	{
		reset();
		return add_from_file(out object_id, fs, resource_path);
	}

	/// Loads the database with the object stored at @a path.
	public int load_from_path(out Guid object_id, string path, string resource_path)
	{
		reset();
		return add_from_path(out object_id, path, resource_path);
	}

	/// Encodes the object @a id into SJSON object.
	private Hashtable encode(Guid id)
	{
		return encode_object(id, get_data(id));
	}

	private static bool is_valid_value(Value? value)
	{
		return value == null
			|| value.holds(typeof(bool))
			|| value.holds(typeof(double))
			|| value.holds(typeof(string))
			|| value.holds(typeof(Guid))
			|| value.holds(typeof(Vector3))
			|| value.holds(typeof(Quaternion))
			;
	}

	private static bool is_valid_key(string key)
	{
		return key.length > 0
			&& !key.has_prefix(".")
			&& !key.has_suffix(".")
			;
	}

	private static string value_to_string(Value? value)
	{
		if (value == null)
			return "null";
		if (value.holds(typeof(bool)))
			return ((bool)value).to_string();
		if (value.holds(typeof(double)))
			return ((double)value).to_string();
		if (value.holds(typeof(string)))
			return ((string)value).to_string();
		if (value.holds(typeof(Guid)))
			return ((Guid)value).to_string();
		if (value.holds(typeof(Vector3)))
			return ((Vector3)value).to_string();
		if (value.holds(typeof(Quaternion)))
			return ((Quaternion)value).to_string();
		if (value.holds(typeof(HashSet)))
			return "Set<Guid>";

		return "<invalid>";
	}

	private void decode_object(Guid id, Guid owner_id, string db_key, Hashtable json)
	{
		string old_db = db_key;
		string k = db_key;

		string[] keys = json.keys.to_array();
		foreach (string key in keys) {
			// ID is filled by decode_set().
			if (key == "id" || key == "_guid")
				continue;

			// The "type" key defines object type only if it appears
			// in the root of a JSON object (k == "").
			if (k == "") {
				if (key == "type" || key == "_type")
					set_object_type(id, (string)json[key]);
			}

			Value? val = json[key];

			k += k == "" ? key : ("." + key);

			if (val.holds(typeof(Hashtable))) {
				Hashtable ht = (Hashtable)val;
				decode_object(id, owner_id, k, ht);
			} else if (val.holds(typeof(ArrayList))) {
				ArrayList<Value?> arr = (ArrayList<Value?>)val;
				if (arr.size > 0
					&& arr[0].holds(typeof(double))
					&& k != "frames" // sprite_animation
					)
					set_property_internal(0, id, k, decode_value(val));
				else
					decode_set(id, key, arr);
			} else {
				set_property_internal(0, id, k, decode_value(val));
			}

			k = old_db;
		}
	}

	private void decode_set(Guid owner_id, string key, ArrayList<Value?> json)
	{
		// Set should be created even if it is empty.
		create_empty_set(0, owner_id, key);

		for (int i = 0; i < json.size; ++i) {
			Hashtable obj;
			string owner_type = object_type(owner_id);
			if (owner_type == "sprite_animation")
				obj = new Hashtable();
			else
				obj = (Hashtable)json[i];

			// Decode object ID.
			Guid obj_id;
			if (obj.has_key("id") && owner_type != "font")
				obj_id = Guid.parse((string)obj["id"]);
			else if (obj.has_key("_guid"))
				obj_id = Guid.parse((string)obj["_guid"]);
			else
				obj_id = Guid.new_guid();

			create_internal(0, obj_id);

			// Determine the object's type based on the type of its
			// parent and other heuristics.
			if (owner_type == OBJECT_TYPE_LEVEL) {
				if (key == "units")
					set_object_type(obj_id, OBJECT_TYPE_UNIT);
				else if (key == "sounds")
					set_object_type(obj_id, OBJECT_TYPE_SOUND_SOURCE);
				else
					set_object_type(obj_id, "undefined");
			} else if (owner_type == OBJECT_TYPE_STATE_MACHINE) {
				if (key == "states")
					set_object_type(obj_id, "state_machine_node");
				else if (key == "variables")
					set_object_type(obj_id, "state_machine_variable");
				else
					set_object_type(obj_id, "undefined");
			} else if (owner_type == "state_machine_node") {
				if (key == "animations")
					set_object_type(obj_id, "node_animation");
				else if (key == "transitions")
					set_object_type(obj_id, "node_transition");
			} else if (owner_type == OBJECT_TYPE_SPRITE) {
				if (key == "frames") {
					set_object_type(obj_id, "sprite_frame");
					set_property_internal(0, obj_id, "index", (double)i);
				}
			} else if (owner_type == "sprite_animation") {
				if (key == "frames") {
					set_object_type(obj_id, "animation_frame");
					set_property_internal(0, obj_id, "index", (double)json[i]);
				}
			} else if (owner_type == "font") {
				if (key == "glyphs") {
					set_object_type(obj_id, "font_glyph");
					set_property_internal(0, obj_id, "cp", (double)obj["id"]);
				}
			}

			decode_object(obj_id, owner_id, "", obj);
			assert(has_property(obj_id, "_type"));

			add_to_set_internal(0, owner_id, key, obj_id);
		}
	}

	private Value? decode_value(Value? value)
	{
		if (value.holds(typeof(ArrayList))) {
			ArrayList<Value?> al = (ArrayList<Value?>)value;
			if (al.size == 1)
				return Vector3((double)al[0], 0.0, 0.0);
			else if (al.size == 2)
				return Vector3((double)al[0], (double)al[1], 0.0);
			else if (al.size == 3)
				return Vector3((double)al[0], (double)al[1], (double)al[2]);
			else if (al.size == 4)
				return Quaternion((double)al[0], (double)al[1], (double)al[2], (double)al[3]);
			else
				return Vector3(0.0, 0.0, 0.0);
		} else if (value.holds(typeof(string))) {
			Guid id;
			if (Guid.try_parse((string)value, out id))
				return id;
			return value;
		} else if (value == null
			|| value.holds(typeof(bool))
			|| value.holds(typeof(double))) {
			return value;
		} else {
			return null;
		}
	}

	private Hashtable encode_object(Guid id, HashMap<string, Value?> db)
	{
		Hashtable obj = new Hashtable();
		if (id != GUID_ZERO)
			obj["_guid"] = id.to_string();

		string[] keys = db.keys.to_array();
		foreach (string key in keys) {
			// Since null-key is equivalent to non-existent key, skip serialization.
			if (db[key] == null)
				continue;

			string[] foo = key.split(".");
			Hashtable x = obj;
			if (foo.length > 1) {
				for (int i = 0; i < foo.length - 1; ++i) {
					string f = foo[i];

					if (x.has_key(f)) {
						x = (Hashtable)x[f];
						continue;
					}

					Hashtable y = new Hashtable();
					x.set(f, y);
					x = y;
				}
			}
			x.set(foo[foo.length - 1], encode_value(db[key]));
		}

		return obj;
	}

	private Value? encode_value(Value? value)
	{
		assert(is_valid_value(value) || value.holds(typeof(HashSet)));

		if (value.holds(typeof(Vector3))) {
			Vector3 v = (Vector3)value;
			ArrayList<Value?> arr = new Gee.ArrayList<Value?>();
			arr.add(v.x);
			arr.add(v.y);
			arr.add(v.z);
			return arr;
		} else if (value.holds(typeof(Quaternion))) {
			Quaternion q = (Quaternion)value;
			ArrayList<Value?> arr = new Gee.ArrayList<Value?>();
			arr.add(q.x);
			arr.add(q.y);
			arr.add(q.z);
			arr.add(q.w);
			return arr;
		} else if (value.holds(typeof(Guid))) {
			Guid id = (Guid)value;
			return id.to_string();
		} else if (value.holds(typeof(HashSet))) {
			HashSet<Guid?> hs = (HashSet<Guid?>)value;
			ArrayList<Value?> arr = new Gee.ArrayList<Value?>();
			foreach (Guid id in hs) {
				arr.add(encode_object(id, get_data(id)));
			}
			return arr;
		} else {
			return value;
		}
	}

	private HashMap<string, Value?> get_data(Guid id)
	{
		assert(has_object(id));

		return _data[id];
	}

	private void create_internal(int dir, Guid id)
	{
		assert(id != GUID_ZERO);

		if (_debug)
			logi("create %s".printf(id.to_string()));

		_data[id] = new HashMap<string, Value?>();

		_distance_from_last_sync += dir;
		object_created(id);
	}

	private void destroy_internal(int dir, Guid id)
	{
		assert(id != GUID_ZERO);
		assert(has_object(id));

		if (_debug)
			logi("destroy %s".printf(id.to_string()));

		object_destroyed(id);
		_distance_from_last_sync += dir;
	}

	public void set_property_internal(int dir, Guid id, string key, Value? value)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(value));

		if (_debug)
			logi("set_property %s %s %s".printf(id.to_string(), key, value_to_string(value)));

		HashMap<string, Value?> ob = get_data(id);
		ob[key] = value;

		_distance_from_last_sync += dir;
		key_changed(id, key);
	}

	public void create_empty_set(int dir, Guid id, string key)
	{
		assert(has_object(id));
		assert(is_valid_key(key));

		HashMap<string, Value?> ob = get_data(id);
		assert(!ob.has_key(key));

		ob[key] = new HashSet<Guid?>(Guid.hash_func, Guid.equal_func);
	}

	private void add_to_set_internal(int dir, Guid id, string key, Guid item_id)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(item_id != GUID_ZERO);
		assert(has_object(item_id));

		if (_debug)
			logi("add_to_set %s %s %s".printf(id.to_string(), key, item_id.to_string()));

		HashMap<string, Value?> ob = get_data(id);

		if (!ob.has_key(key)) {
			HashSet<Guid?> hs = new HashSet<Guid?>(Guid.hash_func, Guid.equal_func);
			hs.add(item_id);
			ob[key] = hs;
		} else {
			((HashSet<Guid?>)ob[key]).add(item_id);
		}

		_distance_from_last_sync += dir;
		key_changed(id, key);
	}

	private void remove_from_set_internal(int dir, Guid id, string key, Guid item_id)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(item_id != GUID_ZERO);

		if (_debug)
			logi("remove_from_set %s %s %s".printf(id.to_string(), key, item_id.to_string()));

		HashMap<string, Value?> ob = get_data(id);
		((HashSet<Guid?>)ob[key]).remove(item_id);

		_distance_from_last_sync += dir;
		key_changed(id, key);
	}

	// Returns the type of the object @a id.
	public string object_type(Guid id)
	{
		assert(has_object(id));

		if (id == GUID_ZERO)
			return "database";
		else
			return (string)get_data(id)["_type"];
	}

	// Sets the @a type of the object @a id.
	// This is called automatically when loading data or when new objects are created via create().
	// It can occasionally be called manually after loading legacy data with no type information
	// stored inside objects.
	public void set_object_type(Guid id, string type)
	{
		assert(has_object(id));
		get_data(id)["_type"] = type;
	}

	public void create(Guid id, string type)
	{
		assert(id != GUID_ZERO);
		assert(!has_object(id));

		if (_undo_redo != null) {
			_undo_redo._undo.write_destroy_action(Action.DESTROY, id, type);
			_undo_redo._redo.clear();
		}

		create_internal(1, id);
		set_object_type(id, type);
		object_created(id);
	}

	public void destroy(Guid id)
	{
		assert(id != GUID_ZERO);
		assert(has_object(id));

		string obj_type = object_type(id);

		HashMap<string, Value?> o = get_data(id);
		string[] keys = o.keys.to_array();

		foreach (string key in keys) {
			Value? value = o[key];
			if (value.holds(typeof(HashSet))) {
				HashSet<Guid?> hs = (HashSet<Guid?>)value;
				Guid?[] ids = hs.to_array();
				foreach (Guid item_id in ids) {
					remove_from_set(id, key, item_id);
					destroy(item_id);
				}
			} else {
				if (key != "type" && key != "_type")
					set_property_null(id, key);
			}
		}

		if (_undo_redo != null) {
			_undo_redo._undo.write_create_action(Action.CREATE, id, obj_type);
			_undo_redo._redo.clear();
		}

		destroy_internal(1, id);
	}

	public void set_property_null(Guid id, string key)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(null));

		if (_undo_redo != null) {
			HashMap<string, Value?> ob = get_data(id);
			if (ob.has_key(key) && ob[key] != null) {
				if (ob[key].holds(typeof(bool)))
					_undo_redo._undo.write_set_property_bool_action(Action.SET_PROPERTY_BOOL, id, key, (bool)ob[key]);
				if (ob[key].holds(typeof(double)))
					_undo_redo._undo.write_set_property_double_action(Action.SET_PROPERTY_DOUBLE, id, key, (double)ob[key]);
				if (ob[key].holds(typeof(string)))
					_undo_redo._undo.write_set_property_string_action(Action.SET_PROPERTY_STRING, id, key, (string)ob[key]);
				if (ob[key].holds(typeof(Guid)))
					_undo_redo._undo.write_set_property_guid_action(Action.SET_PROPERTY_GUID, id, key, (Guid)ob[key]);
				if (ob[key].holds(typeof(Vector3)))
					_undo_redo._undo.write_set_property_vector3_action(Action.SET_PROPERTY_VECTOR3, id, key, (Vector3)ob[key]);
				if (ob[key].holds(typeof(Quaternion)))
					_undo_redo._undo.write_set_property_quaternion_action(Action.SET_PROPERTY_QUATERNION, id, key, (Quaternion)ob[key]);
			} else {
				_undo_redo._undo.write_set_property_null_action(Action.SET_PROPERTY_NULL, id, key);
			}

			_undo_redo._redo.clear();
		}

		set_property_internal(1, id, key, null);
	}

	public void set_property_bool(Guid id, string key, bool val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			HashMap<string, Value?> ob = get_data(id);
			if (ob.has_key(key) && ob[key] != null)
				_undo_redo._undo.write_set_property_bool_action(Action.SET_PROPERTY_BOOL, id, key, (bool)ob[key]);
			else
				_undo_redo._undo.write_set_property_null_action(Action.SET_PROPERTY_NULL, id, key);

			_undo_redo._redo.clear();
		}

		set_property_internal(1, id, key, val);
	}

	public void set_property_double(Guid id, string key, double val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			HashMap<string, Value?> ob = get_data(id);
			if (ob.has_key(key) && ob[key] != null)
				_undo_redo._undo.write_set_property_double_action(Action.SET_PROPERTY_DOUBLE, id, key, (double)ob[key]);
			else
				_undo_redo._undo.write_set_property_null_action(Action.SET_PROPERTY_NULL, id, key);

			_undo_redo._redo.clear();
		}

		set_property_internal(1, id, key, val);
	}

	public void set_property_string(Guid id, string key, string val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			HashMap<string, Value?> ob = get_data(id);
			if (ob.has_key(key) && ob[key] != null)
				_undo_redo._undo.write_set_property_string_action(Action.SET_PROPERTY_STRING, id, key, (string)ob[key]);
			else
				_undo_redo._undo.write_set_property_null_action(Action.SET_PROPERTY_NULL, id, key);

			_undo_redo._redo.clear();
		}

		set_property_internal(1, id, key, val);
	}

	public void set_property_guid(Guid id, string key, Guid val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			HashMap<string, Value?> ob = get_data(id);
			if (ob.has_key(key) && ob[key] != null)
				_undo_redo._undo.write_set_property_guid_action(Action.SET_PROPERTY_GUID, id, key, (Guid)ob[key]);
			else
				_undo_redo._undo.write_set_property_null_action(Action.SET_PROPERTY_NULL, id, key);

			_undo_redo._redo.clear();
		}

		set_property_internal(1, id, key, val);
	}

	public void set_property_vector3(Guid id, string key, Vector3 val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			HashMap<string, Value?> ob = get_data(id);
			if (ob.has_key(key) && ob[key] != null)
				_undo_redo._undo.write_set_property_vector3_action(Action.SET_PROPERTY_VECTOR3, id, key, (Vector3)ob[key]);
			else
				_undo_redo._undo.write_set_property_null_action(Action.SET_PROPERTY_NULL, id, key);

			_undo_redo._redo.clear();
		}

		set_property_internal(1, id, key, val);
	}

	public void set_property_quaternion(Guid id, string key, Quaternion val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			HashMap<string, Value?> ob = get_data(id);
			if (ob.has_key(key) && ob[key] != null)
				_undo_redo._undo.write_set_property_quaternion_action(Action.SET_PROPERTY_QUATERNION, id, key, (Quaternion)ob[key]);
			else
				_undo_redo._undo.write_set_property_null_action(Action.SET_PROPERTY_NULL, id, key);

			_undo_redo._redo.clear();
		}

		set_property_internal(1, id, key, val);
	}

	public void set_property(Guid id, string key, Value? val)
	{
		if (val == null)
			set_property_null(id, key);
		if (val.holds(typeof(bool)))
			set_property_bool(id, key, (bool)val);
		else if (val.holds(typeof(double)))
			set_property_double(id, key, (double)val);
		else if (val.holds(typeof(string)))
			set_property_string(id, key, (string)val);
		else if (val.holds(typeof(Guid)))
			set_property_guid(id, key, (Guid)val);
		else if (val.holds(typeof(Vector3)))
			set_property_vector3(id, key, (Vector3)val);
		else if (val.holds(typeof(Quaternion)))
			set_property_quaternion(id, key, (Quaternion)val);
		else
			assert(false);
	}

	public void add_to_set(Guid id, string key, Guid item_id)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(item_id != GUID_ZERO);
		assert(has_object(item_id));

		if (_undo_redo != null) {
			_undo_redo._undo.write_remove_from_set_action(Action.REMOVE_FROM_SET, id, key, item_id);
			_undo_redo._redo.clear();
		}

		add_to_set_internal(1, id, key, item_id);
	}

	public void remove_from_set(Guid id, string key, Guid item_id)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(item_id != GUID_ZERO);

		if (_undo_redo != null) {
			_undo_redo._undo.write_add_to_set_action(Action.ADD_TO_SET, id, key, item_id);
			_undo_redo._redo.clear();
		}

		remove_from_set_internal(1, id, key, item_id);
	}

	public bool has_object(Guid id)
	{
		return id == GUID_ZERO || _data.has_key(id);
	}

	public bool has_property(Guid id, string key)
	{
		return get_property(id, key) != null;
	}

	public Value? get_property(Guid id, string key)
	{
		assert(has_object(id));
		assert(is_valid_key(key));

		HashMap<string, Value?> ob = get_data(id);
		Value? value = (ob.has_key(key) ? ob[key] : null);

		if (_debug_getters)
			logi("get_property %s %s %s".printf(id.to_string(), key, value_to_string(value)));

		return value;
	}

	public bool get_property_bool(Guid id, string key)
	{
		return (bool)get_property(id, key);
	}

	public double get_property_double(Guid id, string key)
	{
		return (double)get_property(id, key);
	}

	public string get_property_string(Guid id, string key)
	{
		return (string)get_property(id, key);
	}

	public Guid get_property_guid(Guid id, string key)
	{
		return (Guid)get_property(id, key);
	}

	public Vector3 get_property_vector3(Guid id, string key)
	{
		return (Vector3)get_property(id, key);
	}

	public Quaternion get_property_quaternion(Guid id, string key)
	{
		return (Quaternion)get_property(id, key);
	}

	public HashSet<Guid?> get_property_set(Guid id, string key, HashSet<Guid?> deffault)
	{
		assert(has_object(id));
		assert(is_valid_key(key));

		HashMap<string, Value?> ob = get_data(id);
		HashSet<Guid?> value;
		if (ob.has_key(key))
			value = ob[key] as HashSet<Guid?>;
		else
			value = deffault;

		if (_debug_getters)
			logi("get_property %s %s %s".printf(id.to_string(), key, value_to_string(value)));

		return value;
	}

	public HashMap<string, Value?> get_object(Guid id)
	{
		return (HashMap<string, Value?>)get_data(GUID_ZERO)[id.to_string()];
	}

	public string[] get_keys(Guid id)
	{
		HashMap<string, Value?> data = get_data(id);
		return data.keys.to_array();
	}

	public void add_restore_point(int id, Guid?[] data, uint32 flags = 0u)
	{
		if (_debug)
			logi("add_restore_point %d, undo size = %u".printf(id, _undo_redo._undo.size()));

		if (_undo_redo != null) {
			_undo_redo._undo.write_restore_point(id, flags, data);
			_undo_redo._redo.clear();
			restore_point_added(id, data, flags);
		}
	}

	/// Duplicates the object specified by id and assign new_id to the duplicated object.
	public void duplicate(Guid id, Guid new_id)
	{
		assert(id != GUID_ZERO);
		assert(new_id != GUID_ZERO);
		assert(id != new_id);
		assert(has_object(id));

		create(new_id, object_type(id));

		HashMap<string, Value?> ob = get_data(id);
		string[] keys = ob.keys.to_array();
		foreach (string key in keys) {
			Value? val = ob[key];
			if (val.holds(typeof(HashSet))) {
				HashSet<Guid?> hs = (HashSet<Guid?>)val;
				foreach (Guid j in hs) {
					Guid x = Guid.new_guid();
					duplicate(j, x);
					add_to_set(new_id, key, x);
				}
			} else {
				if (ob[key] == null)
					set_property_null(new_id, key);
				if (ob[key].holds(typeof(bool)))
					set_property_bool(new_id, key, (bool)ob[key]);
				if (ob[key].holds(typeof(double)))
					set_property_double(new_id, key, (double)ob[key]);
				if (ob[key].holds(typeof(string)))
					set_property_string(new_id, key, (string)ob[key]);
				if (ob[key].holds(typeof(Guid)))
					set_property_guid(new_id, key, (Guid)ob[key]);
				if (ob[key].holds(typeof(Vector3)))
					set_property_vector3(new_id, key, (Vector3)ob[key]);
				if (ob[key].holds(typeof(Quaternion)))
					set_property_quaternion(new_id, key, (Quaternion)ob[key]);
			}
		}
	}

	/// Copies the database to db under the given new_key.
	public void copy_to(Database db, string new_key)
	{
		assert(db != null);
		assert(is_valid_key(new_key));

		copy_deep(db, GUID_ZERO, new_key);
	}

	public void copy_deep(Database db, Guid id, string new_key)
	{
		HashMap<string, Value?> ob = get_data(id);
		string[] keys = ob.keys.to_array();
		foreach (string key in keys) {
			Value? value = ob[key];
			if (value.holds(typeof(HashSet))) {
				HashSet<Guid?> hs = (HashSet<Guid?>)value;
				foreach (Guid j in hs) {
					db.create(j, object_type(j));
					copy_deep(db, j, "");
					db.add_to_set(id, new_key + (new_key == "" ? "" : ".") + key, j);
				}
			} else {
				if (!db.has_object(id))
					db.create(id, object_type(id));

				string kk = new_key + (new_key == "" ? "" : ".") + key;

				if (ob[key] == null)
					db.set_property_null(id, kk);
				if (ob[key].holds(typeof(bool)))
					db.set_property_bool(id, kk, (bool)ob[key]);
				if (ob[key].holds(typeof(double)))
					db.set_property_double(id, kk, (double)ob[key]);
				if (ob[key].holds(typeof(string)))
					db.set_property_string(id, kk, (string)ob[key]);
				if (ob[key].holds(typeof(Guid)))
					db.set_property_guid(id, kk, (Guid)ob[key]);
				if (ob[key].holds(typeof(Vector3)))
					db.set_property_vector3(id, kk, (Vector3)ob[key]);
				if (ob[key].holds(typeof(Quaternion)))
					db.set_property_quaternion(id, kk, (Quaternion)ob[key]);
			}
		}
	}

	// Tries to read a restore point @a rp from the @a stack and returns
	// 0 if successful.
	private int try_read_restore_point(ref RestorePoint rp, Stack stack)
	{
		if (stack.size() < sizeof(Action) + sizeof(RestorePointHeader))
			return -1;

		rp = stack.read_restore_point();

		if (stack.size() < rp.header.size) {
			// The restore point has been overwritten.
			stack.clear();
			return -1;
		}

		return 0;
	}

	// Un-does the last action and returns its ID, or -1 if there is no
	// action to undo.
	public int undo()
	{
		if (_undo_redo == null)
			return -1;

		RestorePoint rp = {};
		if (try_read_restore_point(ref rp, _undo_redo._undo) != 0)
			return -1;

		undo_or_redo(_undo_redo._undo, _undo_redo._redo, rp.header.size);

		undo_redo(true, rp.header.id, rp.data);
		_undo_redo._redo.write_restore_point(rp.header.id, rp.header.flags, rp.data);

		return (int)rp.header.id;
	}

	// Re-does the last action and returns its ID, or -1 if there is no
	// action to redo.
	public int redo()
	{
		if (_undo_redo == null)
			return -1;

		RestorePoint rp = {};
		if (try_read_restore_point(ref rp, _undo_redo._redo) != 0)
			return -1;

		undo_or_redo(_undo_redo._redo, _undo_redo._undo, rp.header.size);

		undo_redo(false, rp.header.id, rp.data);
		_undo_redo._undo.write_restore_point(rp.header.id, rp.header.flags, rp.data);

		return (int)rp.header.id;
	}

	private void undo_or_redo(Stack undo, Stack redo, uint32 restore_point_size)
	{
		assert(undo.size() >= restore_point_size);

		int dir = undo == _undo_redo._undo ? -1 : 1;

		// Read up to restore_point_size bytes.
		uint32 undo_size_start = undo.size();
		while (undo_size_start - undo.size() < restore_point_size) {
			Action action = (Action)undo.read_uint32();
			if (action == Action.CREATE) {
				Guid id = undo.read_guid();
				string obj_type = undo.read_string();

				redo.write_destroy_action(Action.DESTROY, id, obj_type);
				create_internal(dir, id);
				set_object_type(id, obj_type);
			} else if (action == Action.DESTROY) {
				Guid id = undo.read_guid();
				string obj_type = undo.read_string();

				redo.write_create_action(Action.CREATE, id, obj_type);
				destroy_internal(dir, id);
			} else if (action == Action.SET_PROPERTY_NULL) {
				Guid id = undo.read_guid();
				string key = undo.read_string();

				if (has_property(id, key)) {
					if (get_data(id)[key].holds(typeof(bool)))
						redo.write_set_property_bool_action(Action.SET_PROPERTY_BOOL, id, key, get_property_bool(id, key));
					if (get_data(id)[key].holds(typeof(double)))
						redo.write_set_property_double_action(Action.SET_PROPERTY_DOUBLE, id, key, get_property_double(id, key));
					if (get_data(id)[key].holds(typeof(string)))
						redo.write_set_property_string_action(Action.SET_PROPERTY_STRING, id, key, get_property_string(id, key));
					if (get_data(id)[key].holds(typeof(Guid)))
						redo.write_set_property_guid_action(Action.SET_PROPERTY_GUID, id, key, get_property_guid(id, key));
					if (get_data(id)[key].holds(typeof(Vector3)))
						redo.write_set_property_vector3_action(Action.SET_PROPERTY_VECTOR3, id, key, get_property_vector3(id, key));
					if (get_data(id)[key].holds(typeof(Quaternion)))
						redo.write_set_property_quaternion_action(Action.SET_PROPERTY_QUATERNION, id, key, get_property_quaternion(id, key));
				} else {
					redo.write_set_property_null_action(Action.SET_PROPERTY_NULL, id, key);
				}
				set_property_internal(dir, id, key, null);
			} else if (action == Action.SET_PROPERTY_BOOL) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				bool val = undo.read_bool();

				if (has_property(id, key))
					redo.write_set_property_bool_action(Action.SET_PROPERTY_BOOL, id, key, get_property_bool(id, key));
				else
					redo.write_set_property_null_action(Action.SET_PROPERTY_NULL, id, key);
				set_property_internal(dir, id, key, val);
			} else if (action == Action.SET_PROPERTY_DOUBLE) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				double val = undo.read_double();

				if (has_property(id, key))
					redo.write_set_property_double_action(Action.SET_PROPERTY_DOUBLE, id, key, get_property_double(id, key));
				else
					redo.write_set_property_null_action(Action.SET_PROPERTY_NULL, id, key);
				set_property_internal(dir, id, key, val);
			} else if (action == Action.SET_PROPERTY_STRING) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				string val = undo.read_string();

				if (has_property(id, key))
					redo.write_set_property_string_action(Action.SET_PROPERTY_STRING, id, key, get_property_string(id, key));
				else
					redo.write_set_property_null_action(Action.SET_PROPERTY_NULL, id, key);
				set_property_internal(dir, id, key, val);
			} else if (action == Action.SET_PROPERTY_GUID) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				Guid val = undo.read_guid();

				if (has_property(id, key))
					redo.write_set_property_guid_action(Action.SET_PROPERTY_GUID, id, key, get_property_guid(id, key));
				else
					redo.write_set_property_null_action(Action.SET_PROPERTY_NULL, id, key);
				set_property_internal(dir, id, key, val);
			} else if (action == Action.SET_PROPERTY_VECTOR3) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				Vector3 val = undo.read_vector3();

				if (has_property(id, key))
					redo.write_set_property_vector3_action(Action.SET_PROPERTY_VECTOR3, id, key, get_property_vector3(id, key));
				else
					redo.write_set_property_null_action(Action.SET_PROPERTY_NULL, id, key);
				set_property_internal(dir, id, key, val);
			} else if (action == Action.SET_PROPERTY_QUATERNION) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				Quaternion val = undo.read_quaternion();

				if (has_property(id, key))
					redo.write_set_property_quaternion_action(Action.SET_PROPERTY_QUATERNION, id, key, get_property_quaternion(id, key));
				else
					redo.write_set_property_null_action(Action.SET_PROPERTY_NULL, id, key);
				set_property_internal(dir, id, key, val);
			} else if (action == Action.ADD_TO_SET) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				Guid item_id = undo.read_guid();

				redo.write_remove_from_set_action(Action.REMOVE_FROM_SET, id, key, item_id);
				add_to_set_internal(dir, id, key, item_id);
			} else if (action == Action.REMOVE_FROM_SET) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				Guid item_id = undo.read_guid();

				redo.write_add_to_set_action(Action.ADD_TO_SET, id, key, item_id);
				remove_from_set_internal(dir, id, key, item_id);
			}
		}
	}
}

} /* namespace Crown */
