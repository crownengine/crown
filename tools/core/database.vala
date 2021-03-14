/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gee;

namespace Crown
{
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
		REMOVE_FROM_SET,
		RESTORE_POINT
	}

	private struct RestorePoint
	{
		public int id;
		public uint32 size;
		public Guid[] data;
	}

	private class Stack
	{
		private uint8[] _data;
		private uint32 _read;

		public Stack()
		{
			_data = new uint8[1024*1024];
			_read = 0;
		}

		public void clear()
		{
			_read = 0;
		}

		public uint32 size()
		{
			return _read;
		}

		public void write_data(void* data, ulong len)
		{
			uint8* buf = (uint8*)data;
			for (ulong i = 0; i < len; ++i, ++_read)
				_data[_read] = buf[i];
		}

		public void write_bool(bool a)
		{
			write_data(&a, sizeof(bool));
		}

		public void write_int(int a)
		{
			write_data(&a, sizeof(int));
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

		public void write_action(Action t)
		{
			write_uint32((uint32)t);
		}

		public Action read_action()
		{
			_read -= (uint32)sizeof(uint32);
			uint32 a = *(uint32*)(&_data[_read]);
			return (Action)a;
		}

		public bool read_bool()
		{
			_read -= (uint32)sizeof(bool);
			bool a = *(bool*)(&_data[_read]);
			return a;
		}

		public int read_int()
		{
			_read -= (uint32)sizeof(int);
			int a = *(int*)(&_data[_read]);
			return a;
		}

		public uint32 read_uint32()
		{
			_read -= (uint32)sizeof(uint32);
			uint32 a = *(uint32*)(&_data[_read]);
			return a;
		}

		public double read_double()
		{
			_read -= (uint32)sizeof(double);
			double a = *(double*)(&_data[_read]);
			return a;
		}

		public Guid read_guid()
		{
			_read -= (uint32)sizeof(Guid);
			Guid a = *(Guid*)(&_data[_read]);
			return a;
		}

		public Vector3 read_vector3()
		{
			_read -= (uint32)sizeof(Vector3);
			Vector3 a = *(Vector3*)(&_data[_read]);
			return a;
		}

		public Quaternion read_quaternion()
		{
			_read -= (uint32)sizeof(Quaternion);
			Quaternion a = *(Quaternion*)(&_data[_read]);
			return a;
		}

		public string read_string()
		{
			_read -= (uint32)sizeof(uint32);
			uint32 len = *(uint32*)(&_data[_read]);
			_read -= len;
			uint8[] str = new uint8[len + 1];
			for (uint32 i = 0; i < len; ++i)
				str[i] = *(uint8*)(&_data[_read + i]);
			str[len] = '\0';
			return (string)str;
		}

		public void write_create_action(Guid id)
		{
			write_guid(id);
			write_action(Action.CREATE);
		}

		public void write_destroy_action(Guid id)
		{
			write_guid(id);
			write_action(Action.DESTROY);
		}

		public void write_set_property_null_action(Guid id, string key)
		{
			// No value to push
			write_string(key);
			write_guid(id);
			write_action(Action.SET_PROPERTY_NULL);
		}

		public void write_set_property_bool_action(Guid id, string key, bool val)
		{
			write_bool(val);
			write_string(key);
			write_guid(id);
			write_action(Action.SET_PROPERTY_BOOL);
		}

		public void write_set_property_double_action(Guid id, string key, double val)
		{
			write_double(val);
			write_string(key);
			write_guid(id);
			write_action(Action.SET_PROPERTY_DOUBLE);
		}

		public void write_set_property_string_action(Guid id, string key, string val)
		{
			write_string(val);
			write_string(key);
			write_guid(id);
			write_action(Action.SET_PROPERTY_STRING);
		}

		public void write_set_property_guid_action(Guid id, string key, Guid val)
		{
			write_guid(val);
			write_string(key);
			write_guid(id);
			write_action(Action.SET_PROPERTY_GUID);
		}

		public void write_set_property_vector3_action(Guid id, string key, Vector3 val)
		{
			write_vector3(val);
			write_string(key);
			write_guid(id);
			write_action(Action.SET_PROPERTY_VECTOR3);
		}

		public void write_set_property_quaternion_action(Guid id, string key, Quaternion val)
		{
			write_quaternion(val);
			write_string(key);
			write_guid(id);
			write_action(Action.SET_PROPERTY_QUATERNION);
		}

		public void write_add_to_set_action(Guid id, string key, Guid item_id)
		{
			write_guid(item_id);
			write_string(key);
			write_guid(id);
			write_action(Action.ADD_TO_SET);
		}

		public void write_remove_from_set_action(Guid id, string key, Guid item_id)
		{
			write_guid(item_id);
			write_string(key);
			write_guid(id);
			write_action(Action.REMOVE_FROM_SET);
		}

		public void write_restore_point(int id, uint32 size, Guid[] data)
		{
			uint32 num_guids = data.length;
			for (uint32 i = 0; i < num_guids; ++i)
				write_guid(data[i]);
			write_uint32(num_guids);
			write_uint32(size);
			write_int(id);
			write_action(Action.RESTORE_POINT);
		}

		public uint32 peek_type()
		{
			return *(uint32*)(&_data[_read - (uint32)sizeof(uint32)]);
		}

		public RestorePoint read_restore_point()
		{
			Action t = read_action();
			assert(t == Action.RESTORE_POINT);
			int id = read_int();
			uint32 size = read_uint32();
			uint32 num_guids = read_uint32();
			Guid[] ids = new Guid[num_guids];
			for (uint32 i = 0; i < num_guids; ++i)
				ids[i] = read_guid();
			return { id, size, ids };
		}
	}

	// Data
	private HashMap<string, Value?> _data;
	private Stack _undo;
	private Stack _redo;
	private Stack _undo_points;
	private Stack _redo_points;
	// The number of changes to the database since the last successful state
	// synchronization (load(), save() etc.). If it is less than 0, the changes
	// came from undo(), otherwise they came from redo() or from regular calls to
	// create(), destroy(), set_*() etc. A value of 0 means there were no changes.
	public int _distance_from_last_sync;

	// Signals
	public signal void key_changed(Guid id, string key);
	public signal void undo_redo(bool undo, int id, Guid[] data);

	public Database()
	{
		_data = new HashMap<string, Value?>();
		_undo = new Stack();
		_redo = new Stack();
		_undo_points = new Stack();
		_redo_points = new Stack();

		reset();
	}

	/// Resets database to clean state.
	public void reset()
	{
		_data.clear();
		_undo.clear();
		_redo.clear();
		_undo_points.clear();
		_redo_points.clear();

		_distance_from_last_sync = 0;

		// This is a special field which stores all objects
		_data.set("_objects", new HashMap<string, Value?>());
	}

	/// Returns whether the database has been changed since last call to Save().
	public bool changed()
	{
		return _distance_from_last_sync != 0;
	}

	/// Saves database to path without marking it as not changed.
	public void dump(string path, Guid id)
	{
		Hashtable json = encode(id);
		SJSON.save(json, path);
	}

	/// Saves database to path.
	public void save(string path, Guid id)
	{
		dump(path, id);
		_distance_from_last_sync = 0;
	}

	// See: load_more_from_path().
	public int load_more_from_file(ref Guid object_id, FileStream? fs, string resource_path)
	{
		Hashtable json = SJSON.load_from_file(fs);
		object_id = decode(json);

		// Create a mapping between the path and the object it has been loaded into.
		set_property_internal(0, GUID_ZERO, resource_path, object_id);

		return 0;
	}

	// Loads the database with the object stored at @a path, without resetting the
	// database. This makes it possible to load multiple objects from distinct
	// paths in the same database. @a resource_path is used as a key in the
	// database to refer to the object that has been loaded. This is useful when
	// you do not have the object ID but only its path, as it is often the case
	// since resources use paths and not IDs to reference each other.
	public int load_more_from_path(ref Guid object_id, string path, string resource_path)
	{
		FileStream fs = FileStream.open(path, "rb");
		if (fs == null)
			return 1;

		return load_more_from_file(ref object_id, fs, resource_path);
	}

	/// Loads the database with the object stored at @a path.
	public int load_from_file(ref Guid object_id, FileStream fs, string resource_path)
	{
		reset();
		return load_more_from_file(ref object_id, fs, resource_path);
	}

	/// Loads the database with the object stored at @a path.
	public int load_from_path(ref Guid object_id, string path, string resource_path)
	{
		reset();
		return load_more_from_path(ref object_id, path, resource_path);
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
			&& key != "_objects"
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

	// Decodes the @a json data inside the database object @a id.
	public Guid decode(Hashtable json)
	{
		Guid id;
		if (json.has_key("id"))
			id = Guid.parse((string)json["id"]);
		else
			id = Guid.new_guid();

		create_internal(0, id);
		decode_object(id, "", json);
		return id;
	}

	private void decode_object(Guid id, string db_key, Hashtable json)
	{
		string old_db = db_key;
		string k = db_key;

		string[] keys = json.keys.to_array();
		foreach (string key in keys)
		{
			assert(key != "_objects");
			if (key == "id")
				continue;

			Value? val = json[key];

			k += k == "" ? key : ("." + key);

			if (val.holds(typeof(Hashtable)))
			{
				Hashtable ht = (Hashtable)val;
				decode_object(id, k, ht);
			}
			else if (val.holds(typeof(ArrayList)))
			{
				ArrayList<Value?> arr = (ArrayList<Value?>)val;
				if (arr.size > 0 && arr[0].holds(typeof(double)))
					set_property_internal(0, id, k, decode_value(val));
				else
					decode_set(id, key, arr);
			}
			else
			{
				set_property_internal(0, id, k, decode_value(val));
			}

			k = old_db;
		}
	}

	private void decode_set(Guid id, string key, ArrayList<Value?> json)
	{
		// Set should be created even if it is empty.
		create_empty_set(0, id, key);

		for (int i = 0; i < json.size; ++i)
		{
			Hashtable obj = (Hashtable)json[i];
			Guid item_id = Guid.parse((string)obj["id"]);
			create_internal(0, item_id);
			decode_object(item_id, "", obj);
			add_to_set_internal(0, id, key, item_id);
		}
	}

	private Value? decode_value(Value? value)
	{
		if (value.holds(typeof(ArrayList)))
		{
			ArrayList<Value?> al = (ArrayList<Value?>)value;
			if (al.size == 3)
				return Vector3((double)al[0], (double)al[1], (double)al[2]);
			else if (al.size == 4)
				return Quaternion((double)al[0], (double)al[1], (double)al[2], (double)al[3]);
			else
				assert(false);
		}
		else if (value.holds(typeof(string)))
		{
			Guid id;
			if (Guid.try_parse((string)value, out id))
				return id;
			return value;
		}
		else if (value == null || value.holds(typeof(bool)) || value.holds(typeof(double)))
		{
			return value;
		}
		else
		{
			assert(false);
		}

		return null;
	}

	private Hashtable encode_object(Guid id, HashMap<string, Value?> db)
	{
		Hashtable obj = new Hashtable();
		if (id != GUID_ZERO)
			obj["id"] = id.to_string();

		string[] keys = db.keys.to_array();
		foreach (string key in keys)
		{
			if (key == "_objects")
				continue;

			// Since null-key is equivalent to non-existent key, skip serialization.
			if (db[key] == null)
				continue;

			string[] foo = key.split(".");
			Hashtable x = obj;
			if (foo.length > 1)
			{
				for (int i = 0; i < foo.length - 1; ++i)
				{
					string f = foo[i];

					if (x.has_key(f))
					{
						x = (Hashtable)x[f];
						continue;
					}

					Hashtable y = new Hashtable();
					x.set(f, y);
					x = y;
				}
			}
			x.set(foo[foo.length-1], encode_value(db[key]));
		}

		return obj;
	}

	private Value? encode_value(Value? value)
	{
		assert(is_valid_value(value) || value.holds(typeof(HashSet)));

		if (value.holds(typeof(Vector3)))
		{
			Vector3 v = (Vector3)value;
			ArrayList<Value?> arr = new Gee.ArrayList<Value?>();
			arr.add(v.x);
			arr.add(v.y);
			arr.add(v.z);
			return arr;
		}
		else if (value.holds(typeof(Quaternion)))
		{
			Quaternion q = (Quaternion)value;
			ArrayList<Value?> arr = new Gee.ArrayList<Value?>();
			arr.add(q.x);
			arr.add(q.y);
			arr.add(q.z);
			arr.add(q.w);
			return arr;
		}
		else if (value.holds(typeof(Guid)))
		{
			Guid id = (Guid)value;
			return id.to_string();
		}
		else if (value.holds(typeof(HashSet)))
		{
			HashSet<Guid?> hs = (HashSet<Guid?>)value;
			ArrayList<Value?> arr = new Gee.ArrayList<Value?>();
			foreach (Guid id in hs)
			{
				arr.add(encode_object(id, get_data(id)));
			}
			return arr;
		}
		else
		{
			return value;
		}
	}

	private HashMap<string, Value?> get_data(Guid id)
	{
		assert(has_object(id));

		HashMap<string, Value?> objects = (HashMap<string, Value?>)_data["_objects"];
		return (HashMap<string, Value?>)(id == GUID_ZERO ? _data : objects[id.to_string()]);
	}

	private void create_internal(int dir, Guid id)
	{
		assert(id != GUID_ZERO);

		if (_debug)
			logi("create %s".printf(id.to_string()));

		((HashMap<string, Value?>)_data["_objects"]).set(id.to_string(), new HashMap<string, Value?>());

		_distance_from_last_sync += dir;
		key_changed(id, "_objects");
	}

	private void destroy_internal(int dir, Guid id)
	{
		assert(id != GUID_ZERO);
		assert(has_object(id));

		if (_debug)
			logi("destroy %s".printf(id.to_string()));

		((HashMap<string, Value?>)_data["_objects"]).unset(id.to_string());

		_distance_from_last_sync += dir;
		key_changed(id, "_objects");
	}

	private void set_property_internal(int dir, Guid id, string key, Value? value)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(value));

		if (_debug)
			logi("set_property %s %s %s".printf(id.to_string(), key, (value == null) ? "null" : value_to_string(value)));

		HashMap<string, Value?> ob = get_data(id);
		ob[key] = value;

		_distance_from_last_sync += dir;
		key_changed(id, key);
	}

	private void create_empty_set(int dir, Guid id, string key)
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

		if (!ob.has_key(key))
		{
			HashSet<Guid?> hs = new HashSet<Guid?>(Guid.hash_func, Guid.equal_func);
			hs.add(item_id);
			ob[key] = hs;
		}
		else
		{
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

	public void create(Guid id)
	{
		assert(id != GUID_ZERO);
		assert(!has_object(id));

		_undo.write_destroy_action(id);
		_redo.clear();
		_redo_points.clear();

		create_internal(1, id);
	}

	public void destroy(Guid id)
	{
		assert(id != GUID_ZERO);
		assert(has_object(id));


		HashMap<string, Value?> o = get_data(id);
		string[] keys = o.keys.to_array();

		foreach (string key in keys)
		{
			Value? value = o[key];
			if (value.holds(typeof(HashSet)))
			{
				HashSet<Guid?> hs = (HashSet<Guid?>)value;
				Guid?[] ids = hs.to_array();
				foreach (Guid item_id in ids)
				{
					remove_from_set(id, key, item_id);
					destroy(item_id);
				}
			}
			else
			{
				set_property_null(id, key);
			}
		}

		_undo.write_create_action(id);
		_redo.clear();
		_redo_points.clear();

		destroy_internal(1, id);
	}

	public void set_property_null(Guid id, string key)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(null));

		HashMap<string, Value?> ob = get_data(id);
		if (ob.has_key(key) && ob[key] != null)
		{
			if (ob[key].holds(typeof(bool)))
				_undo.write_set_property_bool_action(id, key, (bool)ob[key]);
			if (ob[key].holds(typeof(double)))
				_undo.write_set_property_double_action(id, key, (double)ob[key]);
			if (ob[key].holds(typeof(string)))
				_undo.write_set_property_string_action(id, key, (string)ob[key]);
			if (ob[key].holds(typeof(Guid)))
				_undo.write_set_property_guid_action(id, key, (Guid)ob[key]);
			if (ob[key].holds(typeof(Vector3)))
				_undo.write_set_property_vector3_action(id, key, (Vector3)ob[key]);
			if (ob[key].holds(typeof(Quaternion)))
				_undo.write_set_property_quaternion_action(id, key, (Quaternion)ob[key]);
		}
		else
		{
			_undo.write_set_property_null_action(id, key);
		}

		_redo.clear();
		_redo_points.clear();

		set_property_internal(1, id, key, null);
	}

	public void set_property_bool(Guid id, string key, bool val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		HashMap<string, Value?> ob = get_data(id);
		if (ob.has_key(key) && ob[key] != null)
			_undo.write_set_property_bool_action(id, key, (bool)ob[key]);
		else
			_undo.write_set_property_null_action(id, key);

		_redo.clear();
		_redo_points.clear();

		set_property_internal(1, id, key, val);
	}

	public void set_property_double(Guid id, string key, double val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		HashMap<string, Value?> ob = get_data(id);
		if (ob.has_key(key) && ob[key] != null)
			_undo.write_set_property_double_action(id, key, (double)ob[key]);
		else
			_undo.write_set_property_null_action(id, key);

		_redo.clear();
		_redo_points.clear();

		set_property_internal(1, id, key, val);
	}

	public void set_property_string(Guid id, string key, string val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		HashMap<string, Value?> ob = get_data(id);
		if (ob.has_key(key) && ob[key] != null)
			_undo.write_set_property_string_action(id, key, (string)ob[key]);
		else
			_undo.write_set_property_null_action(id, key);

		_redo.clear();
		_redo_points.clear();

		set_property_internal(1, id, key, val);
	}

	public void set_property_guid(Guid id, string key, Guid val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		HashMap<string, Value?> ob = get_data(id);
		if (ob.has_key(key) && ob[key] != null)
			_undo.write_set_property_guid_action(id, key, (Guid)ob[key]);
		else
			_undo.write_set_property_null_action(id, key);

		_redo.clear();
		_redo_points.clear();

		set_property_internal(1, id, key, val);
	}

	public void set_property_vector3(Guid id, string key, Vector3 val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		HashMap<string, Value?> ob = get_data(id);
		if (ob.has_key(key) && ob[key] != null)
			_undo.write_set_property_vector3_action(id, key, (Vector3)ob[key]);
		else
			_undo.write_set_property_null_action(id, key);

		_redo.clear();
		_redo_points.clear();

		set_property_internal(1, id, key, val);
	}

	public void set_property_quaternion(Guid id, string key, Quaternion val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		HashMap<string, Value?> ob = get_data(id);
		if (ob.has_key(key) && ob[key] != null)
			_undo.write_set_property_quaternion_action(id, key, (Quaternion)ob[key]);
		else
			_undo.write_set_property_null_action(id, key);

		_redo.clear();
		_redo_points.clear();

		set_property_internal(1, id, key, val);
	}

	public void add_to_set(Guid id, string key, Guid item_id)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(item_id != GUID_ZERO);
		assert(has_object(item_id));

		_undo.write_remove_from_set_action(id, key, item_id);
		_redo.clear();
		_redo_points.clear();

		add_to_set_internal(1, id, key, item_id);
	}

	public void remove_from_set(Guid id, string key, Guid item_id)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(item_id != GUID_ZERO);

		_undo.write_add_to_set_action(id, key, item_id);
		_redo.clear();
		_redo_points.clear();

		remove_from_set_internal(1, id, key, item_id);
	}

	public bool has_object(Guid id)
	{
		return id == GUID_ZERO || ((HashMap<string, Value?>)_data["_objects"]).has_key(id.to_string());
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
			logi("get_property %s %s %s".printf(id.to_string(), key, (value == null) ? "null" : value_to_string(value)));

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
			logi("get_property %s %s %s".printf(id.to_string(), key, (value == null) ? "null" : value_to_string(value)));

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

	public void add_restore_point(int id, Guid[] data)
	{
		if (_debug)
			logi("add_restore_point %d, undo size = %u".printf(id, _undo.size()));

		_undo_points.write_restore_point(id, _undo.size(), data);

		_redo.clear();
		_redo_points.clear();
	}

	/// Duplicates the object specified by id and assign new_id to the duplicated object.
	public void duplicate(Guid id, Guid new_id)
	{
		assert(id != GUID_ZERO);
		assert(new_id != GUID_ZERO);
		assert(id != new_id);
		assert(has_object(id));

		create(new_id);

		HashMap<string, Value?> ob = get_data(id);
		string[] keys = ob.keys.to_array();
		foreach (string key in keys)
		{
			Value? val = ob[key];
			if (val.holds(typeof(HashSet)))
			{
				HashSet<Guid?> hs = (HashSet<Guid?>)val;
				foreach (Guid j in hs)
				{
					Guid x = Guid.new_guid();
					duplicate(j, x);
					add_to_set(new_id, key, x);
				}
			}
			else
			{
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
		foreach (string key in keys)
		{
			if (key == "_objects")
				continue;

			Value? value = ob[key];
			if (value.holds(typeof(HashSet)))
			{
				HashSet<Guid?> hs = (HashSet<Guid?>)value;
				foreach (Guid j in hs)
				{
					db.create(j);
					copy_deep(db, j, "");
					db.add_to_set(id, new_key + (new_key == "" ? "" : ".") + key, j);
				}
			}
			else
			{
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

	// Un-does the last action and returns its ID, or -1 if there is no
	// action to undo.
	public int undo()
	{
		if (_undo_points.size() == 0)
			return -1;

		RestorePoint rp = _undo_points.read_restore_point();
		_redo_points.write_restore_point(rp.id, _redo.size(), rp.data);
		undo_until(rp.size);
		undo_redo(true, rp.id, rp.data);
		return rp.id;
	}

	// Re-does the last action and returns its ID, or -1 if there is no
	// action to redo.
	public int redo()
	{
		if (_redo_points.size() == 0)
			return -1;

		RestorePoint rp = _redo_points.read_restore_point();
		_undo_points.write_restore_point(rp.id, _undo.size(), rp.data);
		redo_until(rp.size);
		undo_redo(false, rp.id, rp.data);
		return rp.id;
	}

	private void undo_until(uint32 size)
	{
		undo_redo_until(size, _undo, _redo);
	}

	private void redo_until(uint32 size)
	{
		undo_redo_until(size, _redo, _undo);
	}

	private void undo_redo_until(uint32 size, Stack undo, Stack redo)
	{
		int dir = undo == _undo ? -1 : 1;

		while (undo.size() != size)
		{
			uint32 type = undo.peek_type();
			if (type == Action.CREATE)
			{
				Action t = undo.read_action();
				assert(t == Action.CREATE);

				Guid id = undo.read_guid();

				redo.write_destroy_action(id);
				create_internal(dir, id);
			}
			else if (type == Action.DESTROY)
			{
				Action t = undo.read_action();
				assert(t == Action.DESTROY);

				Guid id = undo.read_guid();

				redo.write_create_action(id);
				destroy_internal(dir, id);
			}
			else if (type == Action.SET_PROPERTY_NULL)
			{
				Action t = undo.read_action();
				assert(t == Action.SET_PROPERTY_NULL);

				Guid id = undo.read_guid();
				string key = undo.read_string();

				if (has_property(id, key))
				{
					if (get_data(id)[key].holds(typeof(bool)))
						redo.write_set_property_bool_action(id, key, get_property_bool(id, key));
					if (get_data(id)[key].holds(typeof(double)))
						redo.write_set_property_double_action(id, key, get_property_double(id, key));
					if (get_data(id)[key].holds(typeof(string)))
						redo.write_set_property_string_action(id, key, get_property_string(id, key));
					if (get_data(id)[key].holds(typeof(Guid)))
						redo.write_set_property_guid_action(id, key, get_property_guid(id, key));
					if (get_data(id)[key].holds(typeof(Vector3)))
						redo.write_set_property_vector3_action(id, key, get_property_vector3(id, key));
					if (get_data(id)[key].holds(typeof(Quaternion)))
						redo.write_set_property_quaternion_action(id, key, get_property_quaternion(id, key));
				}
				else
				{
					redo.write_set_property_null_action(id, key);
				}
				set_property_internal(dir, id, key, null);
			}
			else if (type == Action.SET_PROPERTY_BOOL)
			{
				Action t = undo.read_action();
				assert(t == Action.SET_PROPERTY_BOOL);

				Guid id = undo.read_guid();
				string key = undo.read_string();
				bool val = undo.read_bool();

				if (has_property(id, key))
					redo.write_set_property_bool_action(id, key, get_property_bool(id, key));
				else
					redo.write_set_property_null_action(id, key);
				set_property_internal(dir, id, key, val);
			}
			else if (type == Action.SET_PROPERTY_DOUBLE)
			{
				Action t = undo.read_action();
				assert(t == Action.SET_PROPERTY_DOUBLE);

				Guid id = undo.read_guid();
				string key = undo.read_string();
				double val = undo.read_double();

				if (has_property(id, key))
					redo.write_set_property_double_action(id, key, get_property_double(id, key));
				else
					redo.write_set_property_null_action(id, key);
				set_property_internal(dir, id, key, val);
			}
			else if (type == Action.SET_PROPERTY_STRING)
			{
				Action t = undo.read_action();
				assert(t == Action.SET_PROPERTY_STRING);

				Guid id = undo.read_guid();
				string key = undo.read_string();
				string val = undo.read_string();

				if (has_property(id, key))
					redo.write_set_property_string_action(id, key, get_property_string(id, key));
				else
					redo.write_set_property_null_action(id, key);
				set_property_internal(dir, id, key, val);
			}
			else if (type == Action.SET_PROPERTY_GUID)
			{
				Action t = undo.read_action();
				assert(t == Action.SET_PROPERTY_GUID);

				Guid id = undo.read_guid();
				string key = undo.read_string();
				Guid val = undo.read_guid();

				if (has_property(id, key))
					redo.write_set_property_guid_action(id, key, get_property_guid(id, key));
				else
					redo.write_set_property_null_action(id, key);
				set_property_internal(dir, id, key, val);
			}
			else if (type == Action.SET_PROPERTY_VECTOR3)
			{
				Action t = undo.read_action();
				assert(t == Action.SET_PROPERTY_VECTOR3);

				Guid id = undo.read_guid();
				string key = undo.read_string();
				Vector3 val = undo.read_vector3();

				if (has_property(id, key))
					redo.write_set_property_vector3_action(id, key, get_property_vector3(id, key));
				else
					redo.write_set_property_null_action(id, key);
				set_property_internal(dir, id, key, val);
			}
			else if (type == Action.SET_PROPERTY_QUATERNION)
			{
				Action t = undo.read_action();
				assert(t == Action.SET_PROPERTY_QUATERNION);

				Guid id = undo.read_guid();
				string key = undo.read_string();
				Quaternion val = undo.read_quaternion();

				if (has_property(id, key))
					redo.write_set_property_quaternion_action(id, key, get_property_quaternion(id, key));
				else
					redo.write_set_property_null_action(id, key);
				set_property_internal(dir, id, key, val);
			}
			else if (type == Action.ADD_TO_SET)
			{
				Action t = undo.read_action();
				assert(t == Action.ADD_TO_SET);

				Guid id = undo.read_guid();
				string key = undo.read_string();
				Guid item_id = undo.read_guid();

				redo.write_remove_from_set_action(id, key, item_id);
				add_to_set_internal(dir, id, key, item_id);
			}
			else if (type == Action.REMOVE_FROM_SET)
			{
				Action t = undo.read_action();
				assert(t == Action.REMOVE_FROM_SET);

				Guid id = undo.read_guid();
				string key = undo.read_string();
				Guid item_id = undo.read_guid();

				redo.write_add_to_set_action(id, key, item_id);
				remove_from_set_internal(dir, id, key, item_id);
			}
		}
	}
}

}
