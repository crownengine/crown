/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gee;

namespace Crown
{
	public class Stack
	{
		private ArrayList<Value?> _data;

		public Stack()
		{
			_data = new ArrayList<Value?>();
		}

		public void push(Value? value)
		{
			_data.add(value);
		}

		public Value? pop()
		{
			return _data.remove_at(_data.size - 1);
		}

		public Value? peek()
		{
			return _data.last();
		}

		public int size()
		{
			return _data.size;
		}

		public void clear()
		{
			_data.clear();
		}
	}

	public class Database
	{
		private abstract class ChangeAction
		{
		}

		private class CreateAction : ChangeAction
		{
			public Guid _id;

			public CreateAction(Guid id)
			{
				_id = id;
			}
		}

		private class DestroyAction : ChangeAction
		{
			public Guid _id;

			public DestroyAction(Guid id)
			{
				_id = id;
			}
		}

		private class SetPropertyAction : ChangeAction
		{
			public Guid _id;
			public string _key;
			public Value? _value;

			public SetPropertyAction(Guid id, string key, Value? value)
			{
				_id = id;
				_key = key;
				_value = value;
			}
		}

		private class AddToSetAction : ChangeAction
		{
			public Guid _id;
			public string _key;
			public Guid _item_id;

			public AddToSetAction(Guid id, string key, Guid item_id)
			{
				_id = id;
				_key = key;
				_item_id = item_id;
			}
		}

		private class RemoveFromSetAction : ChangeAction
		{
			public Guid _id;
			public string _key;
			public Guid _item_id;

			public RemoveFromSetAction(Guid id, string key, Guid item_id)
			{
				_id = id;
				_key = key;
				_item_id = item_id;
			}
		}

		public delegate void ActionCallback(bool undo, int id, Value? data);

		private class RestorePoint : ChangeAction
		{
			public ActionCallback _undo_redo;
			public int _id;
			public Value? _data;

			public RestorePoint(ActionCallback undo_redo, int id, Value? data)
			{
				_undo_redo = undo_redo;
				_id = id;
				_data = data;
			}

			public void Redo()
			{
				if (_undo_redo != null)
					_undo_redo(false, _id, _data);
			}

			public void Undo()
			{
				if (_undo_redo != null)
					_undo_redo(true, _id, _data);
			}
		}

		// Data
		private HashMap<string, Value?> _data;
		private Stack _undo;
		private Stack _redo;
		private bool _changed;

		// Signals
		public signal void key_changed(Guid id, string key);

		public Database()
		{
			_data = new HashMap<string, Value?>();
			_undo = new Stack();
			_redo = new Stack();

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

			_undo.push(new DestroyAction(id));
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

			_undo.push(new CreateAction(id));
			_redo.clear();

			destroy_internal(id);
		}

		public void set_property(Guid id, string key, Value? value)
		{
			assert(has_object(id));
			assert(is_valid_key(key));
			assert(is_valid_value(value));

			HashMap<string, Value?> ob = get_data(id);
			_undo.push(new SetPropertyAction(id, key, ob.has_key(key) ? ob[key] : null));
			_redo.clear();

			set_property_internal(id, key, value);
		}

		public void add_to_set(Guid id, string key, Guid item_id)
		{
			assert(has_object(id));
			assert(is_valid_key(key));
			assert(item_id != GUID_ZERO);
			assert(has_object(item_id));

			_undo.push(new RemoveFromSetAction(id, key, item_id));
			_redo.clear();

			add_to_set_internal(id, key, item_id);
		}

		public void remove_from_set(Guid id, string key, Guid item_id)
		{
			assert(has_object(id));
			assert(is_valid_key(key));
			assert(item_id != GUID_ZERO);

			_undo.push(new AddToSetAction(id, key, item_id));
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

		public void add_restore_point(ActionCallback? undo_redo = null, int id = -1, Value? data = null)
		{
#if CROWN_DEBUG
			stdout.printf("add_restore_point %d\n", id);
#endif // CROWN_DEBUG
			_undo.push(new RestorePoint(undo_redo, id, data));
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

		public void undo_single_action()
		{
			RestorePoint begin_action = null;

			while (true)
			{
				if (_undo.size() == 0)
				{
					if (begin_action != null)
					{
						_redo.push(begin_action);
						begin_action.Undo();
					}
					return;
				}

				Value? action = _undo.peek();
				if (action.holds(typeof(RestorePoint)))
				{
					if (begin_action == null)
					{
						begin_action = (RestorePoint)action;
						_undo.pop();
						continue;
					}
					else
					{
						_redo.push(begin_action);
						begin_action.Undo();
						return;
					}
				}
				else if (action.holds(typeof(CreateAction)))
				{
					CreateAction a = (action as CreateAction);
					_redo.push(new DestroyAction(a._id));
					create_internal(a._id);
				}
				else if (action.holds(typeof(DestroyAction)))
				{
					DestroyAction a = (action as DestroyAction);
					_redo.push(new CreateAction(a._id));
					destroy_internal(a._id);
				}
				else if (action.holds(typeof(SetPropertyAction)))
				{
					SetPropertyAction a = (action as SetPropertyAction);
					_redo.push(new SetPropertyAction(a._id, a._key, get_data(a._id).has_key(a._key) ? get_data(a._id)[a._key] : null));
					set_property_internal(a._id, a._key, a._value);
				}
				else if (action.holds(typeof(AddToSetAction)))
				{
					AddToSetAction a = (action as AddToSetAction);
					_redo.push(new RemoveFromSetAction(a._id, a._key, a._item_id));
					add_to_set_internal(a._id, a._key, a._item_id);
				}
				else if (action.holds(typeof(RemoveFromSetAction)))
				{
					RemoveFromSetAction a = (action as RemoveFromSetAction);
					_redo.push(new AddToSetAction(a._id, a._key, a._item_id));
					remove_from_set_internal(a._id, a._key, a._item_id);
				}

				_undo.pop();
			}
		}

		public void redo_single_action()
		{
			RestorePoint begin_action = null;

			while (true)
			{
				if (_redo.size() == 0)
				{
					if (begin_action != null)
					{
						_undo.push(begin_action);
						begin_action.Redo();
					}
					return;
				}

				Value? action = _redo.peek();
				if (action.holds(typeof(RestorePoint)))
				{
					if (begin_action == null)
					{
						begin_action = (RestorePoint)action;
						_redo.pop();
						continue;
					}
					else
					{
						_undo.push(begin_action);
						begin_action.Redo();
						return;
					}
				}
				else if (action.holds(typeof(CreateAction)))
				{
					CreateAction a = (action as CreateAction);
					_undo.push(new DestroyAction(a._id));
					create_internal(a._id);
				}
				else if (action.holds(typeof(DestroyAction)))
				{
					DestroyAction a = (action as DestroyAction);
					_undo.push(new CreateAction(a._id));
					destroy_internal(a._id);
				}
				else if (action.holds(typeof(SetPropertyAction)))
				{
					SetPropertyAction a = (action as SetPropertyAction);
					_undo.push(new SetPropertyAction(a._id, a._key, get_data(a._id).has_key(a._key) ? get_data(a._id)[a._key] : null));
					set_property_internal(a._id, a._key, a._value);
				}
				else if (action.holds(typeof(AddToSetAction)))
				{
					AddToSetAction a = (action as AddToSetAction);
					_undo.push(new RemoveFromSetAction(a._id, a._key, a._item_id));
					add_to_set_internal(a._id, a._key, a._item_id);
				}
				else if (action.holds(typeof(RemoveFromSetAction)))
				{
					RemoveFromSetAction a = (action as RemoveFromSetAction);
					_undo.push(new AddToSetAction(a._id, a._key, a._item_id));
					remove_from_set_internal(a._id, a._key, a._item_id);
				}

				_redo.pop();
			}
		}
	}
}
