/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gee;

namespace Crown
{
	public class Database
	{
		private enum ActionType
		{
			CREATE,
			DESTROY,
			SET_PROPERTY,
			ADD_TO_SET,
			REMOVE_FROM_SET,
			RESTORE_POINT
		}

		private enum ValueType
		{
			NULL,
			BOOL,
			DOUBLE,
			STRING,
			GUID,
			VECTOR3,
			QUATERNION
		}

		private struct CreateAction
		{
			public Guid id;
		}

		private struct DestroyAction
		{
			public Guid id;
		}

		private struct SetPropertyAction
		{
			public Guid id;
			public string key;
			public Value? val;
		}

		private struct AddToSetAction
		{
			public Guid id;
			public string key;
			public Guid item_id;
		}

		private struct RemoveFromSetAction
		{
			public Guid id;
			public string key;
			public Guid item_id;
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

			public void write_action_type(ActionType t)
			{
				write_uint32((uint32)t);
			}

			public void write_value_type(ValueType t)
			{
				write_uint32((uint32)t);
			}

			public ActionType read_action_type()
			{
				_read -= (uint32)sizeof(uint32);
				uint32 a = *(uint32*)(&_data[_read]);
				return (ActionType)a;
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

			public ValueType read_value_type()
			{
				_read -= (uint32)sizeof(uint32);
				uint32 a = *(uint32*)(&_data[_read]);
				return (ValueType)a;
			}

			public void write_create_action(Guid id)
			{
				write_guid(id);
				write_action_type(ActionType.CREATE);
			}

			public void write_destroy_action(Guid id)
			{
				write_guid(id);
				write_action_type(ActionType.DESTROY);
			}

			public void write_set_property_action(Guid id, string key, Value? val)
			{
				if (val == null)
				{
					// Push nothing
					write_value_type(ValueType.NULL);
				}
				else if (val.holds(typeof(bool)))
				{
					write_bool((bool)val);
					write_value_type(ValueType.BOOL);
				}
				else if (val.holds(typeof(double)))
				{
					write_double((double)val);
					write_value_type(ValueType.DOUBLE);
				}
				else if (val.holds(typeof(string)))
				{
					write_string((string)val);
					write_value_type(ValueType.STRING);
				}
				else if (val.holds(typeof(Guid)))
				{
					write_guid((Guid)val);
					write_value_type(ValueType.GUID);
				}
				else if (val.holds(typeof(Vector3)))
				{
					write_vector3((Vector3)val);
					write_value_type(ValueType.VECTOR3);
				}
				else if (val.holds(typeof(Quaternion)))
				{
					write_quaternion((Quaternion)val);
					write_value_type(ValueType.QUATERNION);
				}

				write_string(key);
				write_guid(id);
				write_action_type(ActionType.SET_PROPERTY);
			}

			public void write_add_to_set_action(Guid id, string key, Guid item_id)
			{
				write_guid(item_id);
				write_string(key);
				write_guid(id);
				write_action_type(ActionType.ADD_TO_SET);
			}

			public void write_remove_from_set_action(Guid id, string key, Guid item_id)
			{
				write_guid(item_id);
				write_string(key);
				write_guid(id);
				write_action_type(ActionType.REMOVE_FROM_SET);
			}

			public void write_restore_point(int id, uint32 size, Guid[] data)
			{
				uint32 num_guids = data.length;
				for (uint32 i = 0; i < num_guids; ++i)
					write_guid(data[i]);
				write_uint32(num_guids);
				write_uint32(size);
				write_int(id);
				write_action_type(ActionType.RESTORE_POINT);
			}

			public uint32 peek_type()
			{
				return *(uint32*)(&_data[_read - (uint32)sizeof(uint32)]);
			}

			public CreateAction read_create_action()
			{
				ActionType t = read_action_type();
				assert(t == ActionType.CREATE);
				return { read_guid() };
			}

			public DestroyAction read_destroy_action()
			{
				ActionType t = read_action_type();
				assert(t == ActionType.DESTROY);
				return { read_guid() };
			}

			public SetPropertyAction read_set_property_action()
			{
				ActionType t = read_action_type();
				assert(t == ActionType.SET_PROPERTY);

				Guid id = read_guid();
				string key = read_string();
				ValueType type = read_value_type();

				Value? val = null;

				if (type == ValueType.NULL)
				{
					// Pop nothing
				}
				else if (type == ValueType.BOOL)
					val = read_bool();
				else if (type == ValueType.DOUBLE)
					val = read_double();
				else if (type == ValueType.STRING)
					val = read_string();
				else if (type == ValueType.GUID)
					val = read_guid();
				else if (type == ValueType.VECTOR3)
					val = read_vector3();
				else if (type == ValueType.QUATERNION)
					val = read_quaternion();
				else
					assert(false);

				return { id, key, val };
			}

			public AddToSetAction read_add_to_set_action()
			{
				ActionType t = read_action_type();
				assert(t == ActionType.ADD_TO_SET);
				return { read_guid(), read_string(), read_guid() };
			}

			public RemoveFromSetAction read_remove_from_set_action()
			{
				ActionType t = read_action_type();
				assert(t == ActionType.REMOVE_FROM_SET);
				return { read_guid(), read_string(), read_guid() };
			}

			public RestorePoint read_restore_point()
			{
				ActionType t = read_action_type();
				assert(t == ActionType.RESTORE_POINT);
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
		private bool _changed;

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

		/// <summary>
		/// Resets database to clean state.
		/// </summary>
		public void reset()
		{
			_data.clear();
			_undo.clear();
			_redo.clear();
			_undo_points.clear();
			_redo_points.clear();

			_changed = false;

			// This is a special field which stores all objects
			_data.set("_objects", new HashMap<string, Value?>());
		}

		/// <summary>
		/// Returns whether the database has been changed since last call to Save().
		/// </summary>
		public bool changed()
		{
			return _changed;
		}

		/// <summary>
		/// Saves database to path.
		/// </summary>
		public void save(string path)
		{
			Hashtable json = encode();
			SJSON.save(json, path);
			_changed = false;
		}

		/// <summary>
		/// Loads database from path.
		/// </summary>
		public void load(string path)
		{
			Hashtable json = SJSON.load(path);
			decode(json);
			_changed = false;
		}

		private Hashtable encode()
		{
			return encode_object(_data);
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
			if (value.holds(typeof(HashSet<Guid?>)))
				return "Set<Guid>";

			return "<invalid>";
		}

		private void decode(Hashtable json)
		{
			reset();
			decode_root_object(json);
		}

		private void decode_root_object(Hashtable json)
		{
			decode_object(GUID_ZERO, "", json);
		}

		private void decode_object(Guid id, string db_key, Hashtable json)
		{
			string old_db = db_key;
			string k = db_key;

			string[] keys = json.keys.to_array();
			foreach (string key in keys)
			{
				assert(key != "_objects");

				Value? val = json[key];

				k += k == "" ? key : ("." + key);

				if (val.holds(typeof(Hashtable)))
				{
					Hashtable ht = (Hashtable)val;
					if (is_set(ht))
						decode_set(id, key, ht);
					else
						decode_object(id, k, ht);
				}
				else
				{
					set_property_internal(id, k, decode_value(val));
				}

				k = old_db;
			}
		}

		private bool is_set(Hashtable json)
		{
			string[] keys = json.keys.to_array();
			foreach (string k in keys)
			{
				Guid guid;
				if (!Guid.try_parse(k, out guid))
					return false;
			}

			return true;
		}

		private void decode_set(Guid id, string key, Hashtable json)
		{
			create_empty_set(id, key);

			string[] keys = json.keys.to_array();
			foreach (string k in keys)
			{
				Guid item_id = Guid.parse(k);
				create_internal(item_id);
				decode_object(item_id, "", (Hashtable)json[k]);
				add_to_set_internal(id, key, item_id);
			}
		}

		private Value? decode_value(Value? value)
		{
			if (value.holds(typeof(ArrayList<Value?>)))
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

		private Hashtable encode_object(HashMap<string, Value?> db)
		{
			Hashtable obj = new Hashtable();

			string[] keys = db.keys.to_array();
			foreach (string key in keys)
			{
				if (key == "_objects")
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
			assert(is_valid_value(value) || value.holds(typeof(HashSet<Guid?>)));

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
				return "\"%s\"".printf(id.to_string());
			}
			else if (value.holds(typeof(HashSet<Guid?>)))
			{
				HashSet<Guid?> hs = (HashSet<Guid?>)value;
				Hashtable ht = new Hashtable();
				foreach (Guid id in hs)
				{
					HashMap<string, Value?> objs = (HashMap<string, Value?>)_data["_objects"];
					ht.set(id.to_string(), encode_object((HashMap<string, Value?>)objs[id.to_string()]));
				}
				return ht;
			}
			else
			{
				return value;
			}
		}

		private HashMap<string, Value?> get_data(Guid id)
		{
			assert(has_object(id));

			return (HashMap<string, Value?>)(id == GUID_ZERO ? _data : (_data["_objects"] as HashMap<string, Value?>)[id.to_string()]);
		}

		private void create_internal(Guid id)
		{
			assert(id != GUID_ZERO);
#if CROWN_DEBUG
			stdout.printf("create %s\n", id.to_string());
#endif // CROWN_DEBUG
			(_data["_objects"] as HashMap<string, Value?>).set(id.to_string(), new HashMap<string, Value?>());

			_changed = true;
			key_changed(id, "_objects");
		}

		private void destroy_internal(Guid id)
		{
			assert(id != GUID_ZERO);
			assert(has_object(id));
#if CROWN_DEBUG
			stdout.printf("destroy %s\n", id.to_string());
#endif // CROWN_DEBUG
			(_data["_objects"] as HashMap<string, Value?>).unset(id.to_string());

			_changed = true;
			key_changed(id, "_objects");
		}

		private void set_property_internal(Guid id, string key, Value? value)
		{
			assert(has_object(id));
			assert(is_valid_key(key));
			assert(is_valid_value(value));
#if CROWN_DEBUG
			stdout.printf("set_property %s %s %s\n"
				, id.to_string()
				, key
				, (value == null) ? "null" : value_to_string(value)
				);
#endif // CROWN_DEBUG
			HashMap<string, Value?> ob = get_data(id);
			ob[key] = value;

			_changed = true;
			key_changed(id, key);
		}

		private void create_empty_set(Guid id, string key)
		{
			assert(has_object(id));
			assert(is_valid_key(key));

			HashMap<string, Value?> ob = get_data(id);
			assert(!ob.has_key(key));

			ob[key] = new HashSet<Guid?>(Guid.hash_func, Guid.equal_func);
		}

		private void add_to_set_internal(Guid id, string key, Guid item_id)
		{
			assert(has_object(id));
			assert(is_valid_key(key));
			assert(item_id != GUID_ZERO);
			assert(has_object(item_id));
#if CROWN_DEBUG
			stdout.printf("add_to_set %s %s %s\n"
				, id.to_string()
				, key
				, item_id.to_string()
				);
#endif // CROWN_DEBUG
			HashMap<string, Value?> ob = get_data(id);

			if (!ob.has_key(key))
			{
				HashSet<Guid?> hs = new HashSet<Guid?>(Guid.hash_func, Guid.equal_func);
				hs.add(item_id);
				ob[key] = hs;
			}
			else
			{
				(ob[key] as HashSet<Guid?>).add(item_id);
			}

			_changed = true;
			key_changed(id, key);
		}

		private void remove_from_set_internal(Guid id, string key, Guid item_id)
		{
			assert(has_object(id));
			assert(is_valid_key(key));
			assert(item_id != GUID_ZERO);
#if CROWN_DEBUG
			stdout.printf("remove_from_set %s %s %s\n"
				, id.to_string()
				, key
				, item_id.to_string()
				);
#endif // CROWN_DEBUG
			HashMap<string, Value?> ob = get_data(id);
			(ob[key] as HashSet<Guid?>).remove(item_id);

			_changed = true;
			key_changed(id, key);
		}

		public void create(Guid id)
		{
			assert(id != GUID_ZERO);
			assert(!has_object(id));

			_undo.write_destroy_action(id);
			_redo.clear();

			create_internal(id);
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
				if (value.holds(typeof(HashSet<Guid?>)))
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
					set_property(id, key, null);
				}
			}

			_undo.write_create_action(id);
			_redo.clear();

			destroy_internal(id);
		}

		public void set_property(Guid id, string key, Value? value)
		{
			assert(has_object(id));
			assert(is_valid_key(key));
			assert(is_valid_value(value));

			HashMap<string, Value?> ob = get_data(id);
			_undo.write_set_property_action(id, key, ob.has_key(key) ? ob[key] : null);
			_redo.clear();

			set_property_internal(id, key, value);
		}

		public void add_to_set(Guid id, string key, Guid item_id)
		{
			assert(has_object(id));
			assert(is_valid_key(key));
			assert(item_id != GUID_ZERO);
			assert(has_object(item_id));

			_undo.write_remove_from_set_action(id, key, item_id);
			_redo.clear();

			add_to_set_internal(id, key, item_id);
		}

		public void remove_from_set(Guid id, string key, Guid item_id)
		{
			assert(has_object(id));
			assert(is_valid_key(key));
			assert(item_id != GUID_ZERO);

			_undo.write_add_to_set_action(id, key, item_id);
			_redo.clear();

			remove_from_set_internal(id, key, item_id);
		}

		public bool has_object(Guid id)
		{
			bool contains = (_data["_objects"] as HashMap<string, Value?>).has_key(id.to_string());
			return id == GUID_ZERO || contains;
		}

		public bool has_property(Guid id, string key)
		{
			assert(has_object(id));
			assert(is_valid_key(key));

			return get_data(id).has_key(key);
		}

		public Value? get_property(Guid id, string key)
		{
			assert(has_object(id));
			assert(is_valid_key(key));

			HashMap<string, Value?> ob = get_data(id);
			Value? value = (ob.has_key(key) ? ob[key] : null);
#if CROWN_DEBUG
			stdout.printf("get_property %s %s %s\n"
				, id.to_string()
				, key
				, (value == null) ? "null" : value_to_string(value)
				);
#endif // CROWN_DEBUG
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
#if CROWN_DEBUG
			stdout.printf("add_restore_point %d\n", id);
#endif // CROWN_DEBUG
			_undo_points.write_restore_point(id, _undo.size(), data);
		}

		/// <summary>
		/// Duplicates the object specified by id and assign new_id to the duplicated object.
		/// </summary>
		public void duplicate(Guid id, Guid new_id)
		{
			assert(id != GUID_ZERO);
			assert(new_id != GUID_ZERO);
			assert(id != new_id);
			assert(has_object(id));

			create(new_id);

			HashMap<string, Value?> o = get_data(id);
			string[] keys = o.keys.to_array();
			foreach (string key in keys)
			{
				Value? val = o[key];
				if (val.holds(typeof(HashSet<Guid?>)))
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
					set_property(new_id, key, o[key]);
				}
			}
		}

		/// <summary>
		/// Copies the database to db under the given new_key.
		/// </summary>
		public void copy_to(Database db, string new_key)
		{
			assert(db != null);
			assert(is_valid_key(new_key));

			copy_deep(db, GUID_ZERO, new_key);
		}

		public void copy_deep(Database db, Guid id, string new_key)
		{
			HashMap<string, Value?> o = get_data(id);
			string[] keys = o.keys.to_array();
			foreach (string key in keys)
			{
				if (key == "_objects")
					continue;

				Value? value = o[key];
				if (value.holds(typeof(HashSet<Guid?>)))
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
					db.set_property(id, new_key + (new_key == "" ? "" : ".") + key, o[key]);
				}
			}
		}

		public void undo()
		{
			if (_undo_points.size() == 0)
				return;

			RestorePoint rp = _undo_points.read_restore_point();
			_redo_points.write_restore_point(rp.id, _redo.size(), rp.data);
			undo_until(rp.size);
			undo_redo(true, rp.id, rp.data);
		}

		public void redo()
		{
			if (_redo_points.size() == 0)
				return;

			RestorePoint rp = _redo_points.read_restore_point();
			_undo_points.write_restore_point(rp.id, _undo.size(), rp.data);
			redo_until(rp.size);
			undo_redo(false, rp.id, rp.data);
		}

		private void undo_until(uint32 size)
		{
			while (_undo.size() != size)
			{
				uint32 type = _undo.peek_type();
				if (type == ActionType.CREATE)
				{
					CreateAction a = _undo.read_create_action();
					_redo.write_destroy_action(a.id);
					create_internal(a.id);
				}
				else if (type == ActionType.DESTROY)
				{
					DestroyAction a = _undo.read_destroy_action();
					_redo.write_create_action(a.id);
					destroy_internal(a.id);
				}
				else if (type == ActionType.SET_PROPERTY)
				{
					SetPropertyAction a = _undo.read_set_property_action();
					_redo.write_set_property_action(a.id, a.key, get_data(a.id).has_key(a.key) ? get_data(a.id)[a.key] : null);
					set_property_internal(a.id, a.key, a.val);
				}
				else if (type == ActionType.ADD_TO_SET)
				{
					AddToSetAction a = _undo.read_add_to_set_action();
					_redo.write_remove_from_set_action(a.id, a.key, a.item_id);
					add_to_set_internal(a.id, a.key, a.item_id);
				}
				else if (type == ActionType.REMOVE_FROM_SET)
				{
					RemoveFromSetAction a = _undo.read_remove_from_set_action();
					_redo.write_add_to_set_action(a.id, a.key, a.item_id);
					remove_from_set_internal(a.id, a.key, a.item_id);
				}
			}
		}

		private void redo_until(uint32 size)
		{
			while (_redo.size() != size)
			{
				uint32 type = _redo.peek_type();
				if (type == ActionType.CREATE)
				{
					CreateAction a = _redo.read_create_action();
					_undo.write_destroy_action(a.id);
					create_internal(a.id);
				}
				else if (type == ActionType.DESTROY)
				{
					DestroyAction a = _redo.read_destroy_action();
					_undo.write_create_action(a.id);
					destroy_internal(a.id);
				}
				else if (type == ActionType.SET_PROPERTY)
				{
					SetPropertyAction a = _redo.read_set_property_action();
					_undo.write_set_property_action(a.id, a.key, get_data(a.id).has_key(a.key) ? get_data(a.id)[a.key] : null);
					set_property_internal(a.id, a.key, a.val);
				}
				else if (type == ActionType.ADD_TO_SET)
				{
					AddToSetAction a = _redo.read_add_to_set_action();
					_undo.write_remove_from_set_action(a.id, a.key, a.item_id);
					add_to_set_internal(a.id, a.key, a.item_id);
				}
				else if (type == ActionType.REMOVE_FROM_SET)
				{
					RemoveFromSetAction a = _redo.read_remove_from_set_action();
					_undo.write_add_to_set_action(a.id, a.key, a.item_id);
					remove_from_set_internal(a.id, a.key, a.item_id);
				}
			}
		}
	}
}
