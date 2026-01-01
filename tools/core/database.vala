/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public enum PropertyType
{
	BOOL,
	DOUBLE,
	STRING,
	VECTOR3,
	QUATERNION,
	RESOURCE,
	REFERENCE,
	OBJECTS_SET,
}

public enum PropertyEditorType
{
	DEFAULT,  ///< Default editor for the property type.
	ENUM,     ///< A string selected from a list.
	RESOURCE, ///< A resource name selected from a project.
	ANGLE,    ///< An angle value displayed in degrees.
	COLOR,    ///< An RGB color from a color picker.
}

public delegate void EnumCallback(InputField enum_property, InputEnum property, Project project);
public delegate void ResourceCallback(InputField enum_property, InputResource property, Project project);

public struct PropertyDefinition
{
	public PropertyType type;
	public string name;
	public string? label;

	public PropertyEditorType editor;
	public Value? min;
	public Value? max;
	public Value? deffault;
	public string[] enum_values;
	public string[] enum_labels;
	public string? enum_property;
	public unowned EnumCallback? enum_callback;
	public unowned ResourceCallback? resource_callback;
	public string? resource_type;
	public StringId64 object_type;

	public bool hidden;
	public bool not_serialized;
}

public struct Resource
{
	public string? name;
}

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

	public Resource read_resource()
	{
		string name = read_string();
		Resource resource = { name == "" ? null : name };
		return resource;
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

	public void write_set_null_action(uint32 action, Guid id, string key)
	{
		// No value to push
		write_string(key);
		write_guid(id);
		write_uint32(action);
	}

	public void write_set_bool_action(uint32 action, Guid id, string key, bool val)
	{
		write_bool(val);
		write_string(key);
		write_guid(id);
		write_uint32(action);
	}

	public void write_set_double_action(uint32 action, Guid id, string key, double val)
	{
		write_double(val);
		write_string(key);
		write_guid(id);
		write_uint32(action);
	}

	public void write_set_string_action(uint32 action, Guid id, string key, string val)
	{
		write_string(val);
		write_string(key);
		write_guid(id);
		write_uint32(action);
	}

	public void write_set_vector3_action(uint32 action, Guid id, string key, Vector3 val)
	{
		write_vector3(val);
		write_string(key);
		write_guid(id);
		write_uint32(action);
	}

	public void write_set_quaternion_action(uint32 action, Guid id, string key, Quaternion val)
	{
		write_quaternion(val);
		write_string(key);
		write_guid(id);
		write_uint32(action);
	}

	public void write_set_resource_action(uint32 action, Guid id, string key, Resource val)
	{
		write_string(val.name == null ? "" : val.name);
		write_string(key);
		write_guid(id);
		write_uint32(action);
	}

	public void write_set_reference_action(uint32 action, Guid id, string key, Guid val)
	{
		write_guid(val);
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
	public int _distance_from_last_sync;

	///
	public UndoRedo(uint32 undo_redo_size = 0)
	{
		uint32 size = uint32.max(1024, undo_redo_size);
		_undo = new Stack(size);
		_redo = new Stack(size);

		reset();
	}

	public void reset()
	{
		_undo.clear();
		_redo.clear();
		_distance_from_last_sync = 0;
	}
}

const string OBJECT_NAME_UNNAMED = "Unnamed";

public enum ObjectTypeFlags
{
	NONE           = 0,
	UNIT_COMPONENT = 1 << 0,
}

public struct PropertiesSlice
{
	int start; // Index of first property.
	int end;   // Index of last property + 1.
}

public delegate void Aspect(out string name, Database database, Guid id);

[Compact]
public struct AspectData
{
	public unowned Aspect callback;
}

public struct ObjectTypeInfo
{
	PropertiesSlice properties;
	string name;
	string ui_name;
	double ui_order;
	ObjectTypeFlags flags;
	string? user_data;
	Gee.HashMap<StringId64?, AspectData?> aspects;
}

public class Database
{
	public static bool _debug = false;
	public static bool _debug_getters = false;

	public enum Action
	{
		CREATE,
		DESTROY,
		SET_NULL,
		SET_BOOL,
		SET_DOUBLE,
		SET_STRING,
		SET_VECTOR3,
		SET_QUATERNION,
		SET_RESOURCE,
		SET_REFERENCE,
		ADD_TO_SET,
		REMOVE_FROM_SET
	}

	// Data
	private PropertyDefinition[] _property_definitions;
	public Gee.HashMap<StringId64?, ObjectTypeInfo?> _object_definitions;
	public Gee.HashMap<Guid?, Gee.HashMap<string, Value?>> _data;
	public UndoRedo? _undo_redo;
	public Project _project;
	// The number of changes to the database since the last successful state
	// synchronization (load(), save() etc.). If it is less than 0, the changes
	// came from undo(), otherwise they came from redo() or from regular calls to
	// create(), destroy(), set_*() etc. A value of 0 means there were no changes.

	// Signals
	public signal void object_type_added(ObjectTypeInfo info);
	public signal void objects_created(Guid?[] object_ids, uint32 flags);
	public signal void objects_destroyed(Guid?[] object_ids, uint32 flags);
	public signal void objects_changed(Guid?[] object_ids, uint32 flags);

	public Database(Project project, UndoRedo? undo_redo = null)
	{
		_property_definitions = new PropertyDefinition[0];
		_object_definitions = new Gee.HashMap<StringId64?, ObjectTypeInfo?>(StringId64.hash_func, StringId64.equal_func);
		_data = new Gee.HashMap<Guid?, Gee.HashMap<string, Value?>>(Guid.hash_func, Guid.equal_func);
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

		// This is a special field which stores all objects
		_data[GUID_ZERO] = new Gee.HashMap<string, Value?>();
	}

	/// Returns whether the database has been changed since last call to Save().
	public bool changed()
	{
		return _undo_redo != null
			? _undo_redo._distance_from_last_sync != 0
			: false
			;
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
	public int save(string path, Guid id)
	{
		int err = dump(path, id);
		if (err == 0) {
			if (_undo_redo != null)
				_undo_redo._distance_from_last_sync = 0;
		}

		return err;
	}

	public UndoRedo disable_undo()
	{
		var undo = _undo_redo;
		_undo_redo = null;
		return undo;
	}

	public void restore_undo(UndoRedo undo_redo)
	{
		_undo_redo = undo_redo;
	}

	// See: add_from_path().
	public int add_from_file(out Guid object_id, FileStream? fs, string resource_path)
	{
		UndoRedo undo_redo = disable_undo();

		try {
			Hashtable json = SJSON.load_from_file(fs);

			// Parse the object's ID or generate a new one if none is found.
			if (json.has_key("id"))
				object_id = Guid.parse((string)json["id"]);
			else if (json.has_key("_guid"))
				object_id = Guid.parse((string)json["_guid"]);
			else
				object_id = Guid.new_guid();

			string type = ResourceId.type(resource_path);
			StringId64 type_hash = StringId64(type);

			_data[object_id] = new Gee.HashMap<string, Value?>();
			set_type(object_id, type);
			set_owner(object_id, GUID_ZERO);
			set_alive(object_id, true);

			if (has_type(type_hash))
				_init_object(object_id, object_definition(type_hash));

			decode_object(object_id, GUID_ZERO, "", json);

			// Create a mapping between the path and the object it has been loaded into.
			set(0, GUID_ZERO, resource_path, object_id);

			restore_undo(undo_redo);
			return 0;
		} catch (JsonSyntaxError e) {
			object_id = GUID_ZERO;
			restore_undo(undo_redo);
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
			object_id = get_reference(GUID_ZERO, resource_path);
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
	public Hashtable encode(Guid id)
	{
		return encode_object(id, get_data(id));
	}

	public static bool is_valid_value(Value? value)
	{
		return value == null
			|| value.holds(typeof(bool))
			|| value.holds(typeof(double))
			|| value.holds(typeof(string))
			|| value.holds(typeof(Vector3))
			|| value.holds(typeof(Quaternion))
			|| value.holds(typeof(Resource))
			|| value.holds(typeof(Guid))
			;
	}

	public static bool is_valid_key(string key)
	{
		return key.length > 0
			&& !key.has_prefix(".")
			&& !key.has_suffix(".")
			;
	}

	public static string debug_string(Value? value)
	{
		if (value == null)
			return "null";
		if (value.holds(typeof(bool)))
			return ((bool)value).to_string();
		if (value.holds(typeof(double)))
			return ((double)value).to_string();
		if (value.holds(typeof(string)))
			return ((string)value).to_string();
		if (value.holds(typeof(Vector3)))
			return ((Vector3)value).to_string();
		if (value.holds(typeof(Quaternion)))
			return ((Quaternion)value).to_string();
		if (value.holds(typeof(Resource)))
			return ((Resource)value).name == null ? "(None)" : ((Resource)value).name;
		if (value.holds(typeof(Guid)))
			return ((Guid)value).to_debug_string();
		if (value.holds(typeof(Gee.HashSet)))
			return "Set<Guid>";

		return "<invalid>";
	}

	public void decode_object_compat(Guid id, Guid owner_id, string db_key, Hashtable json)
	{
		string old_db = db_key;
		string k = db_key;

		string[] keys = json.keys.to_array();
		foreach (string key in keys) {
			// ID is filled by decode_set().
			if (key == "id"
				|| key == "_guid"
				|| key == "_alive"
				|| key == "prefab"
				)
				continue;

			Value? val = json[key];

			k += k == "" ? key : ("." + key);

			if (val.holds(typeof(Hashtable))) {
				Hashtable ht = (Hashtable)val;
				decode_object(id, owner_id, k, ht);
			} else if (val.holds(typeof(Gee.ArrayList))) {
				Gee.ArrayList<Value?> arr = (Gee.ArrayList<Value?>)val;
				if (arr.size > 0
					&& arr[0].holds(typeof(double))
					&& k != "frames" // sprite_animation
					)
					set(0, id, k, decode_value(val));
				else
					decode_set(id, key, arr);
			} else {
				set(0, id, k, decode_value(val));
			}

			k = old_db;
		}
	}

	public void decode_object_from_properties(Guid id, Guid owner_id, PropertyDefinition[]? properties, Hashtable json)
	{
		foreach (PropertyDefinition def in properties) {
			// Find table and key to read from.
			string[] keys = def.name.split(".");
			string key = keys[keys.length - 1];
			Hashtable input = json;

			if (keys.length > 1) {
				for (int i = 0; i < keys.length - 1; ++i) {
					string f = keys[i];

					if (input.has_key(f)) {
						input = (Hashtable)input[f];
						continue;
					}
				}
			}

			if (!input.has_key(key))
				continue;

			// Read property.
			if (def.type == PropertyType.OBJECTS_SET) {
				decode_set(id, def.name, (Gee.ArrayList<Value?>)input[key]);
			} else if (def.type == PropertyType.RESOURCE) {
				Resource res = { null };
				if (input.has_key(key)) {
					Value? val = input[key];
					if (val.holds(typeof(string)))
						res.name = (string)input[key];
				}
				set(0, id, def.name, res);
			} else {
				set(0, id, def.name, decode_value(input[key]));
			}
		}
	}

	public void decode_object(Guid id, Guid owner_id, string db_key, Hashtable json)
	{
		string? type = null;

		// The "type" key defines object type only if it appears
		// in the root of a JSON object (k == "").
		if (db_key == "" && !has_property(id, "_type")) {
			if (json.has_key("_type"))
				type = (string)json["_type"];
			else if (json.has_key("type"))
				type = (string)json["type"];

			assert(type != null);
			set_type(id, type);

			StringId64 type_hash = StringId64(type);
			if (has_type(type_hash))
				_init_object(id, object_definition(type_hash));
		} else {
			type = object_type(id);
		}

		assert(type != null);
		PropertyDefinition[]? properties = object_definition(StringId64(type));
		decode_object_from_properties(id, owner_id, properties, json);

		if (type == OBJECT_TYPE_UNIT)
			decode_object_compat(id, owner_id, db_key, json);
	}

	public void decode_set(Guid owner_id, string key, Gee.ArrayList<Value?> json)
	{
		// Set should be created even if it is empty.
		create_empty_set(owner_id, key);

		for (int i = 0; i < json.size; ++i) {
			Hashtable obj;
			string owner_type = object_type(owner_id);
			obj = (Hashtable)json[i];

			// Decode object ID.
			Guid obj_id;
			if (obj.has_key("id") && owner_type != OBJECT_TYPE_FONT)
				obj_id = Guid.parse((string)obj["id"]);
			else if (obj.has_key("_guid"))
				obj_id = Guid.parse((string)obj["_guid"]);
			else
				obj_id = Guid.new_guid();

			_data[obj_id] = new Gee.HashMap<string, Value?>();

			set_owner(obj_id, owner_id);
			set_alive(obj_id, true);
			decode_object(obj_id, owner_id, "", obj);
			assert(has_property(obj_id, "_type"));

			add_to_set_internal(0, owner_id, key, obj_id);
		}
	}

	public Value? decode_value(Value? value)
	{
		if (value.holds(typeof(Gee.ArrayList))) {
			Gee.ArrayList<Value?> al = (Gee.ArrayList<Value?>)value;
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

	public Hashtable encode_object_compat(Guid id, Gee.HashMap<string, Value?> db)
	{
		Hashtable obj = new Hashtable();
		if (id != GUID_ZERO)
			obj["_guid"] = id.to_string();

		string[] keys = db.keys.to_array();
		foreach (string key in keys) {
			// Since null-key is equivalent to non-existent key, skip serialization.
			if (db[key] == null || key == "_owner" || key == "_alive")
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

	public Hashtable encode_object(Guid id, Gee.HashMap<string, Value?> db)
	{
		assert(is_alive(id));

		string type = object_type(id);
		PropertyDefinition[]? properties = object_definition(StringId64(type));

		if (type == OBJECT_TYPE_UNIT || properties == null)
			return encode_object_compat(id, db);

		Hashtable obj = new Hashtable();
		if (id != GUID_ZERO) {
			obj["_guid"] = id.to_string();
			obj["_type"] = type;
		}

		foreach (PropertyDefinition def in properties) {
			if (def.not_serialized)
				continue;

			// Since null-key is equivalent to non-existent key, skip serialization.
			if (db[def.name] == null)
				continue;

			string[] foo = def.name.split(".");
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
			x.set(foo[foo.length - 1], encode_value(db[def.name]));
		}

		return obj;
	}

	public Value? encode_value(Value? value)
	{
		assert(is_valid_value(value) || value.holds(typeof(Gee.HashSet)));

		if (value.holds(typeof(Vector3))) {
			Vector3 v = (Vector3)value;
			Gee.ArrayList<Value?> arr = new Gee.ArrayList<Value?>();
			arr.add(v.x);
			arr.add(v.y);
			arr.add(v.z);
			return arr;
		} else if (value.holds(typeof(Quaternion))) {
			Quaternion q = (Quaternion)value;
			Gee.ArrayList<Value?> arr = new Gee.ArrayList<Value?>();
			arr.add(q.x);
			arr.add(q.y);
			arr.add(q.z);
			arr.add(q.w);
			return arr;
		} else if (value.holds(typeof(Resource))) {
			Resource res = (Resource)value;
			if (res.name == null)
				return null;
			else
				return res.name;
		} else if (value.holds(typeof(Guid))) {
			Guid id = (Guid)value;
			return id.to_string();
		} else if (value.holds(typeof(Gee.HashSet))) {
			Gee.HashSet<Guid?> hs = (Gee.HashSet<Guid?>)value;
			Gee.ArrayList<Value?> arr = new Gee.ArrayList<Value?>();
			foreach (Guid id in hs) {
				if (!is_alive(id))
					continue;
				arr.add(encode_object(id, get_data(id)));
			}
			return arr;
		} else {
			return value;
		}
	}

	public Gee.HashMap<string, Value?> get_data(Guid id)
	{
		assert(has_object(id));

		return _data[id];
	}

	public void set(int dir, Guid id, string key, Value? value)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(value));

		if (_debug)
			logi("set_property %s %s %s".printf(debug_string(id), key, debug_string(value)));

		Gee.HashMap<string, Value?> ob = get_data(id);
		ob[key] = value;

		if (_undo_redo != null)
			_undo_redo._distance_from_last_sync += dir;
	}

	public void create_empty_set(Guid id, string key)
	{
		assert(has_object(id));
		assert(is_valid_key(key));

		Gee.HashMap<string, Value?> ob = get_data(id);
		ob[key] = new Gee.HashSet<Guid?>(Guid.hash_func, Guid.equal_func);
	}

	public void add_to_set_internal(int dir, Guid id, string key, Guid item_id)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(item_id != GUID_ZERO);
		assert(has_object(item_id));

		if (_debug)
			logi("add_to_set %s %s %s".printf(debug_string(id), key, debug_string(item_id)));

		Gee.HashMap<string, Value?> ob = get_data(id);

		if (!ob.has_key(key)) {
			Gee.HashSet<Guid?> hs = new Gee.HashSet<Guid?>(Guid.hash_func, Guid.equal_func);
			hs.add(item_id);
			ob[key] = hs;
		} else {
			((Gee.HashSet<Guid?>)ob[key]).add(item_id);
		}

		get_data(item_id)["_owner"] = id;

		if (_undo_redo != null)
			_undo_redo._distance_from_last_sync += dir;
	}

	public void remove_from_set_internal(int dir, Guid id, string key, Guid item_id)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(item_id != GUID_ZERO);

		if (_debug)
			logi("remove_from_set %s %s %s".printf(debug_string(id), key, debug_string(item_id)));

		Gee.HashMap<string, Value?> ob = get_data(id);
		((Gee.HashSet<Guid?>)ob[key]).remove(item_id);

		set_owner(id, GUID_ZERO);

		if (_undo_redo != null)
			_undo_redo._distance_from_last_sync += dir;
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

	// Returns the owner of @a id.
	public Guid owner(Guid id)
	{
		assert(has_object(id));
		return (Guid)get_data(id)["_owner"];
	}

	// Sets the @a type of the object @a id.
	// This is called automatically when loading data or when new objects are created via create().
	// It can occasionally be called manually after loading legacy data with no type information
	// stored inside objects.
	public void set_type(Guid id, string type)
	{
		assert(has_object(id));
		get_data(id)["_type"] = type;
	}

	public void set_owner(Guid id, Guid owner_id)
	{
		assert(has_object(id));
		assert(has_object(owner_id));
		get_data(id)["_owner"] = owner_id;
	}

	public void set_alive(Guid id, bool alive)
	{
		assert(has_object(id));
		get_data(id)["_alive"] = alive;
	}

	public bool is_alive(Guid id)
	{
		assert(has_object(id));
		return id == GUID_ZERO || (bool)get_data(id)["_alive"];
	}

	public void _init_object(Guid id, PropertyDefinition[] properties)
	{
		foreach (PropertyDefinition def in properties) {
			switch (def.type) {
			case PropertyType.BOOL:
				set_bool(id, def.name, (bool)def.deffault);
				break;
			case PropertyType.DOUBLE:
				set_double(id, def.name, (double)def.deffault);
				break;
			case PropertyType.STRING:
				set_string(id, def.name, (string)def.deffault);
				break;
			case PropertyType.VECTOR3:
				set_vector3(id, def.name, (Vector3)def.deffault);
				break;
			case PropertyType.QUATERNION:
				set_quaternion(id, def.name, (Quaternion)def.deffault);
				break;
			case PropertyType.RESOURCE:
				set_resource(id, def.name, (string?)def.deffault);
				break;
			case PropertyType.REFERENCE:
				set_reference(id, def.name, (Guid)def.deffault);
				break;
			case PropertyType.OBJECTS_SET:
				create_empty_set(id, def.name);
				break;
				default:
				assert(false);
				break;
			}
		}
	}

	public void create(Guid id, string type)
	{
		assert(id != GUID_ZERO);
		assert(!has_object(id));

		if (_debug)
			logi("create %s".printf(debug_string(id)));

		if (_undo_redo != null) {
			_undo_redo._distance_from_last_sync += 1;
			_undo_redo._undo.write_destroy_action(Action.DESTROY, id, type);
			_undo_redo._redo.clear();
		}

		_data[id] = new Gee.HashMap<string, Value?>();
		set_alive(id, true);
		set_type(id, type);

		StringId64 type_hash = StringId64(type);
		if (has_type(type_hash))
			_init_object(id, object_definition(type_hash));
	}

	public void destroy(Guid id)
	{
		assert(id != GUID_ZERO);
		assert(has_object(id));

		string obj_type = object_type(id);

		Gee.HashMap<string, Value?> o = get_data(id);
		string[] keys = o.keys.to_array();

		foreach (string key in keys) {
			Value? value = o[key];
			if (value.holds(typeof(Gee.HashSet))) {
				Gee.HashSet<Guid?> hs = (Gee.HashSet<Guid?>)value;
				Guid?[] ids = hs.to_array();
				foreach (Guid item_id in ids) {
					if (is_alive(item_id))
						destroy(item_id);
				}
			}
		}

		set_alive(id, false);

		if (_undo_redo != null) {
			_undo_redo._distance_from_last_sync += 1;
			_undo_redo._undo.write_create_action(Action.CREATE, id, obj_type);
			_undo_redo._redo.clear();
		}

		if (_debug)
			logi("destroy %s".printf(debug_string(id)));
	}

	public void set_null(Guid id, string key)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(null));

		if (_undo_redo != null) {
			Gee.HashMap<string, Value?> ob = get_data(id);
			if (ob.has_key(key) && ob[key] != null) {
				if (ob[key].holds(typeof(bool)))
					_undo_redo._undo.write_set_bool_action(Action.SET_BOOL, id, key, (bool)ob[key]);
				if (ob[key].holds(typeof(double)))
					_undo_redo._undo.write_set_double_action(Action.SET_DOUBLE, id, key, (double)ob[key]);
				if (ob[key].holds(typeof(string)))
					_undo_redo._undo.write_set_string_action(Action.SET_STRING, id, key, (string)ob[key]);
				if (ob[key].holds(typeof(Vector3)))
					_undo_redo._undo.write_set_vector3_action(Action.SET_VECTOR3, id, key, (Vector3)ob[key]);
				if (ob[key].holds(typeof(Quaternion)))
					_undo_redo._undo.write_set_quaternion_action(Action.SET_QUATERNION, id, key, (Quaternion)ob[key]);
				if (ob[key].holds(typeof(Resource)))
					_undo_redo._undo.write_set_resource_action(Action.SET_RESOURCE, id, key, (Resource)ob[key]);
				if (ob[key].holds(typeof(Guid)))
					_undo_redo._undo.write_set_reference_action(Action.SET_REFERENCE, id, key, (Guid)ob[key]);
			} else {
				_undo_redo._undo.write_set_null_action(Action.SET_NULL, id, key);
			}

			_undo_redo._redo.clear();
		}

		set(1, id, key, null);
	}

	public void set_bool(Guid id, string key, bool val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			Gee.HashMap<string, Value?> ob = get_data(id);
			if (ob.has_key(key) && ob[key] != null)
				_undo_redo._undo.write_set_bool_action(Action.SET_BOOL, id, key, (bool)ob[key]);
			else
				_undo_redo._undo.write_set_null_action(Action.SET_NULL, id, key);

			_undo_redo._redo.clear();
		}

		set(1, id, key, val);
	}

	public void set_double(Guid id, string key, double val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			Gee.HashMap<string, Value?> ob = get_data(id);
			if (ob.has_key(key) && ob[key] != null)
				_undo_redo._undo.write_set_double_action(Action.SET_DOUBLE, id, key, (double)ob[key]);
			else
				_undo_redo._undo.write_set_null_action(Action.SET_NULL, id, key);

			_undo_redo._redo.clear();
		}

		set(1, id, key, val);
	}

	public void set_string(Guid id, string key, string val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			Gee.HashMap<string, Value?> ob = get_data(id);
			if (ob.has_key(key) && ob[key] != null)
				_undo_redo._undo.write_set_string_action(Action.SET_STRING, id, key, (string)ob[key]);
			else
				_undo_redo._undo.write_set_null_action(Action.SET_NULL, id, key);

			_undo_redo._redo.clear();
		}

		set(1, id, key, val);
	}

	public void set_vector3(Guid id, string key, Vector3 val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			Gee.HashMap<string, Value?> ob = get_data(id);
			if (ob.has_key(key) && ob[key] != null)
				_undo_redo._undo.write_set_vector3_action(Action.SET_VECTOR3, id, key, (Vector3)ob[key]);
			else
				_undo_redo._undo.write_set_null_action(Action.SET_NULL, id, key);

			_undo_redo._redo.clear();
		}

		set(1, id, key, val);
	}

	public void set_quaternion(Guid id, string key, Quaternion val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			Gee.HashMap<string, Value?> ob = get_data(id);
			if (ob.has_key(key) && ob[key] != null)
				_undo_redo._undo.write_set_quaternion_action(Action.SET_QUATERNION, id, key, (Quaternion)ob[key]);
			else
				_undo_redo._undo.write_set_null_action(Action.SET_NULL, id, key);

			_undo_redo._redo.clear();
		}

		set(1, id, key, val);
	}

	public void set_resource(Guid id, string key, string? val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			Gee.HashMap<string, Value?> ob = get_data(id);
			if (ob.has_key(key) && ob[key] != null)
				_undo_redo._undo.write_set_resource_action(Action.SET_RESOURCE, id, key, { ((Resource)ob[key]).name });
			else
				_undo_redo._undo.write_set_null_action(Action.SET_NULL, id, key);

			_undo_redo._redo.clear();
		}

		Resource res = { val };
		set(1, id, key, res);
	}

	public void set_reference(Guid id, string key, Guid val)
	{
		assert(has_object(id));
		assert(is_valid_key(key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			Gee.HashMap<string, Value?> ob = get_data(id);
			if (ob.has_key(key) && ob[key] != null)
				_undo_redo._undo.write_set_reference_action(Action.SET_REFERENCE, id, key, (Guid)ob[key]);
			else
				_undo_redo._undo.write_set_null_action(Action.SET_NULL, id, key);

			_undo_redo._redo.clear();
		}

		set(1, id, key, val);
	}

	public void set_property(Guid id, string key, Value? val)
	{
		if (val == null)
			set_null(id, key);
		if (val.holds(typeof(bool)))
			set_bool(id, key, (bool)val);
		else if (val.holds(typeof(double)))
			set_double(id, key, (double)val);
		else if (val.holds(typeof(string)))
			set_string(id, key, (string)val);
		else if (val.holds(typeof(Vector3)))
			set_vector3(id, key, (Vector3)val);
		else if (val.holds(typeof(Quaternion)))
			set_quaternion(id, key, (Quaternion)val);
		else if (val.holds(typeof(Resource)))
			set_resource(id, key, ((Resource)val).name);
		else if (val.holds(typeof(Guid)))
			set_reference(id, key, (Guid)val);
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

	public Value? get_property(Guid id, string key, Value? val = null)
	{
		assert(has_object(id));
		assert(is_valid_key(key));

		Gee.HashMap<string, Value?> ob = get_data(id);
		Value? value = (ob.has_key(key) ? ob[key] : val);

		if (_debug_getters)
			logi("get_property %s %s %s".printf(debug_string(id), key, debug_string(value)));

		return value;
	}

	public bool get_bool(Guid id, string key, bool deffault = false)
	{
		return (bool)get_property(id, key, deffault);
	}

	public double get_double(Guid id, string key, double deffault = 0.0)
	{
		return (double)get_property(id, key, deffault);
	}

	public string get_string(Guid id, string key, string deffault = "")
	{
		return (string)get_property(id, key, deffault);
	}

	public Vector3 get_vector3(Guid id, string key, Vector3 deffault = VECTOR3_ZERO)
	{
		return (Vector3)get_property(id, key, deffault);
	}

	public Quaternion get_quaternion(Guid id, string key, Quaternion deffault = QUATERNION_IDENTITY)
	{
		return (Quaternion)get_property(id, key, deffault);
	}

	public string? get_resource(Guid id, string key, string? deffault = null)
	{
		Resource deffault_res = { deffault };
		Value? val = get_property(id, key, deffault_res);
		assert(val == null || val.holds(typeof(Crown.Resource)));
		return ((Resource)val).name;
	}

	public Guid get_reference(Guid id, string key, Guid deffault = GUID_ZERO)
	{
		return (Guid)get_property(id, key, deffault);
	}

	public Gee.HashSet<Guid?> get_set(Guid id, string key, Gee.HashSet<Guid?> deffault = new Gee.HashSet<Guid?>(Guid.hash_func, Guid.equal_func))
	{
		assert(has_object(id));
		assert(is_valid_key(key));

		Gee.HashMap<string, Value?> ob = get_data(id);
		Gee.HashSet<Guid?> value;
		if (ob.has_key(key)) {
			Gee.HashSet<Guid?> objects = (Gee.HashSet<Guid?>)ob[key];
			value = new Gee.HashSet<Guid?>(Guid.hash_func, Guid.equal_func);

			foreach (var obj in objects) {
				if (is_alive(obj))
					value.add(obj);
			}
		} else {
			value = deffault;
		}

		if (_debug_getters)
			logi("get_property %s %s %s".printf(debug_string(id), key, debug_string(value)));

		return value;
	}

	public Gee.HashMap<string, Value?> get_object(Guid id)
	{
		return (Gee.HashMap<string, Value?>)get_data(GUID_ZERO)[id.to_string()];
	}

	public string[] get_keys(Guid id)
	{
		Gee.HashMap<string, Value?> data = get_data(id);
		return data.keys.to_array();
	}

	public void add_restore_point(int id, Guid?[] data, uint32 flags = 0u)
	{
		if (_debug)
			logi("add_restore_point %d, undo size = %u".printf(id, _undo_redo._undo.size()));

		if (_undo_redo != null) {
			_undo_redo._undo.write_restore_point(id, flags, data);
			_undo_redo._redo.clear();

			switch (id) {
			case ActionType.CREATE_OBJECTS:
				objects_created(data, flags);
				break;
			case ActionType.DESTROY_OBJECTS:
				objects_destroyed(data, flags);
				break;
			case ActionType.CHANGE_OBJECTS:
				objects_changed(data, flags);
				break;
				default:
				logw("Unknown action type %d".printf(id));
				break;
			}
		}
	}

	/// Duplicates the object specified by id and assign new_id to the duplicated object.
	public void duplicate(Guid id, Guid new_id, Database? dest = null)
	{
		assert(id != GUID_ZERO);
		assert(new_id != GUID_ZERO);
		assert(id != new_id);
		assert(has_object(id));

		if (dest == null)
			dest = this;

		dest.create(new_id, object_type(id));

		Gee.HashMap<string, Value?> ob = get_data(id);
		string[] keys = ob.keys.to_array();
		foreach (string key in keys) {
			Value? val = ob[key];
			if (val.holds(typeof(Gee.HashSet))) {
				Gee.HashSet<Guid?> hs = (Gee.HashSet<Guid?>)val;
				foreach (Guid j in hs) {
					Guid x = Guid.new_guid();
					duplicate(j, x, dest);
					dest.add_to_set(new_id, key, x);
				}
			} else {
				if (ob[key] == null)
					dest.set_null(new_id, key);
				else if (ob[key].holds(typeof(bool)))
					dest.set_bool(new_id, key, (bool)ob[key]);
				else if (ob[key].holds(typeof(double)))
					dest.set_double(new_id, key, (double)ob[key]);
				else if (ob[key].holds(typeof(string)))
					dest.set_string(new_id, key, (string)ob[key]);
				else if (ob[key].holds(typeof(Vector3)))
					dest.set_vector3(new_id, key, (Vector3)ob[key]);
				else if (ob[key].holds(typeof(Quaternion)))
					dest.set_quaternion(new_id, key, (Quaternion)ob[key]);
				else if (ob[key].holds(typeof(Resource)))
					dest.set_resource(new_id, key, ((Resource)ob[key]).name);
				else if (ob[key].holds(typeof(Guid)))
					dest.set_reference(new_id, key, (Guid)ob[key]);
				else
					assert(false);
			}
		}
	}

	public void duplicate_and_add_to_set(Guid id, Guid new_id)
	{
		duplicate(id, new_id);

		Guid owner_id = owner(id);
		if (owner_id == GUID_ZERO)
			return;

		PropertyDefinition[]? properties = object_definition(StringId64(object_type(owner_id)));

		foreach (PropertyDefinition def in properties) {
			if (def.type != PropertyType.OBJECTS_SET)
				continue;

			Gee.HashSet<Guid?> objects = get_set(owner_id, def.name);

			if (objects.contains(id)) {
				add_to_set(owner_id, def.name, new_id);
				break;
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
		Gee.HashMap<string, Value?> ob = get_data(id);
		string[] keys = ob.keys.to_array();
		foreach (string key in keys) {
			Value? value = ob[key];
			if (value.holds(typeof(Gee.HashSet))) {
				Gee.HashSet<Guid?> hs = (Gee.HashSet<Guid?>)value;
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
					db.set_null(id, kk);
				if (ob[key].holds(typeof(bool)))
					db.set_bool(id, kk, (bool)ob[key]);
				if (ob[key].holds(typeof(double)))
					db.set_double(id, kk, (double)ob[key]);
				if (ob[key].holds(typeof(string)))
					db.set_string(id, kk, (string)ob[key]);
				if (ob[key].holds(typeof(Vector3)))
					db.set_vector3(id, kk, (Vector3)ob[key]);
				if (ob[key].holds(typeof(Quaternion)))
					db.set_quaternion(id, kk, (Quaternion)ob[key]);
				if (ob[key].holds(typeof(Resource)))
					db.set_resource(id, kk, ((Resource)ob[key]).name);
				if (ob[key].holds(typeof(Guid)))
					db.set_reference(id, kk, (Guid)ob[key]);
			}
		}
	}

	// Tries to read a restore point @a rp from the @a stack and returns
	// 0 if successful.
	public int try_read_restore_point(ref RestorePoint rp, Stack stack)
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

		switch (rp.header.id) {
		case ActionType.CREATE_OBJECTS:
			objects_destroyed(rp.data, 0u);
			break;
		case ActionType.DESTROY_OBJECTS:
			objects_created(rp.data, 0u);
			break;
		case ActionType.CHANGE_OBJECTS:
			objects_changed(rp.data, 0u);
			break;
			default:
			logw("Unknown action type %u".printf(rp.header.id));
			break;
		}

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

		switch (rp.header.id) {
		case ActionType.CREATE_OBJECTS:
			objects_created(rp.data, 0u);
			break;
		case ActionType.DESTROY_OBJECTS:
			objects_destroyed(rp.data, 0u);
			break;
		case ActionType.CHANGE_OBJECTS:
			objects_changed(rp.data, 0u);
			break;
			default:
			logw("Unknown action type %u".printf(rp.header.id));
			break;
		}

		_undo_redo._undo.write_restore_point(rp.header.id, rp.header.flags, rp.data);

		return (int)rp.header.id;
	}

	public void undo_or_redo(Stack undo, Stack redo, uint32 restore_point_size)
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
				set_alive(id, true);
				_undo_redo._distance_from_last_sync += dir;
				redo.write_destroy_action(Action.DESTROY, id, obj_type);
			} else if (action == Action.DESTROY) {
				Guid id = undo.read_guid();
				string obj_type = undo.read_string();
				set_alive(id, false);
				_undo_redo._distance_from_last_sync += dir;
				redo.write_create_action(Action.CREATE, id, obj_type);
			} else if (action == Action.SET_NULL) {
				Guid id = undo.read_guid();
				string key = undo.read_string();

				if (has_property(id, key)) {
					if (get_data(id)[key].holds(typeof(bool)))
						redo.write_set_bool_action(Action.SET_BOOL, id, key, get_bool(id, key));
					if (get_data(id)[key].holds(typeof(double)))
						redo.write_set_double_action(Action.SET_DOUBLE, id, key, get_double(id, key));
					if (get_data(id)[key].holds(typeof(string)))
						redo.write_set_string_action(Action.SET_STRING, id, key, get_string(id, key));
					if (get_data(id)[key].holds(typeof(Vector3)))
						redo.write_set_vector3_action(Action.SET_VECTOR3, id, key, get_vector3(id, key));
					if (get_data(id)[key].holds(typeof(Quaternion)))
						redo.write_set_quaternion_action(Action.SET_QUATERNION, id, key, get_quaternion(id, key));
					if (get_data(id)[key].holds(typeof(Resource)))
						redo.write_set_resource_action(Action.SET_RESOURCE, id, key, { get_resource(id, key) });
					if (get_data(id)[key].holds(typeof(Guid)))
						redo.write_set_reference_action(Action.SET_REFERENCE, id, key, get_reference(id, key));
				} else {
					redo.write_set_null_action(Action.SET_NULL, id, key);
				}
				set(dir, id, key, null);
			} else if (action == Action.SET_BOOL) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				bool val = undo.read_bool();

				if (has_property(id, key))
					redo.write_set_bool_action(Action.SET_BOOL, id, key, get_bool(id, key));
				else
					redo.write_set_null_action(Action.SET_NULL, id, key);
				set(dir, id, key, val);
			} else if (action == Action.SET_DOUBLE) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				double val = undo.read_double();

				if (has_property(id, key))
					redo.write_set_double_action(Action.SET_DOUBLE, id, key, get_double(id, key));
				else
					redo.write_set_null_action(Action.SET_NULL, id, key);
				set(dir, id, key, val);
			} else if (action == Action.SET_STRING) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				string val = undo.read_string();

				if (has_property(id, key))
					redo.write_set_string_action(Action.SET_STRING, id, key, get_string(id, key));
				else
					redo.write_set_null_action(Action.SET_NULL, id, key);
				set(dir, id, key, val);
			} else if (action == Action.SET_VECTOR3) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				Vector3 val = undo.read_vector3();

				if (has_property(id, key))
					redo.write_set_vector3_action(Action.SET_VECTOR3, id, key, get_vector3(id, key));
				else
					redo.write_set_null_action(Action.SET_NULL, id, key);
				set(dir, id, key, val);
			} else if (action == Action.SET_QUATERNION) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				Quaternion val = undo.read_quaternion();

				if (has_property(id, key))
					redo.write_set_quaternion_action(Action.SET_QUATERNION, id, key, get_quaternion(id, key));
				else
					redo.write_set_null_action(Action.SET_NULL, id, key);
				set(dir, id, key, val);
			} else if (action == Action.SET_RESOURCE) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				Resource val = undo.read_resource();

				if (has_property(id, key))
					redo.write_set_resource_action(Action.SET_RESOURCE, id, key, { get_resource(id, key) });
				else
					redo.write_set_null_action(Action.SET_NULL, id, key);
				set(dir, id, key, val);
			} else if (action == Action.SET_REFERENCE) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				Guid val = undo.read_guid();

				if (has_property(id, key))
					redo.write_set_reference_action(Action.SET_REFERENCE, id, key, get_reference(id, key));
				else
					redo.write_set_null_action(Action.SET_NULL, id, key);
				set(dir, id, key, val);
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

	public void add_properties(PropertyDefinition[] properties)
	{
		foreach (PropertyDefinition def in properties) {
			// Generate labels if missing.
			if (def.label == null) {
				int ld = def.name.last_index_of_char('.');
				string label = ld == -1 ? def.name : def.name.substring(ld + 1);
				def.label = camel_case(label);
			}
			if (def.enum_labels.length == 0) {
				string[] labels = new string[def.enum_values.length];
				for (int i = 0; i < def.enum_values.length; ++i)
					labels[i] = camel_case(def.enum_values[i]);
				def.enum_labels = labels;
			}

			// Assign default/min/max values.
			switch (def.type) {
			case PropertyType.BOOL:
				if (def.deffault == null)
					def.deffault = false;
				assert(def.deffault.holds(typeof(bool)));
				break;
			case PropertyType.DOUBLE:
				if (def.deffault == null)
					def.deffault = 0.0;
				if (def.min == null)
					def.min = -double.MAX;
				if (def.max == null)
					def.max = double.MAX;

				assert(def.deffault.holds(typeof(double)));
				assert(def.min.holds(typeof(double)));
				assert(def.max.holds(typeof(double)));
				break;
			case PropertyType.STRING:
				if (def.deffault == null) {
					if (def.enum_values.length > 0)
						def.deffault = def.enum_values[0];
					else
						def.deffault = "";
				}

				assert(def.enum_property == null && def.enum_callback == null || def.enum_property != null && def.enum_callback != null && def.enum_values.length == 0);
				assert(def.deffault.holds(typeof(string)));
				break;
			case PropertyType.VECTOR3:
				if (def.deffault == null)
					def.deffault = VECTOR3_ZERO;
				if (def.min == null)
					def.min = VECTOR3_MIN;
				if (def.max == null)
					def.max = VECTOR3_MAX;

				assert(def.deffault.holds(typeof(Vector3)));
				assert(def.min.holds(typeof(Vector3)));
				assert(def.max.holds(typeof(Vector3)));
				break;
			case PropertyType.QUATERNION:
				if (def.deffault == null)
					def.deffault = QUATERNION_IDENTITY;

				assert(def.deffault.holds(typeof(Quaternion)));
				break;
			case PropertyType.RESOURCE:
				if (def.deffault == null)
					def.deffault = (string?)null;
				assert(def.resource_type != null);
				assert(def.deffault.holds(typeof(string?)));
				break;
			case PropertyType.REFERENCE:
				def.deffault = GUID_ZERO;
				assert(def.deffault.holds(typeof(Guid)));
				assert(def.object_type._id != 0);
				break;
			case PropertyType.OBJECTS_SET:
				assert(def.object_type._id != 0);
				break;
				default:
				assert(false);
				break;
			}

			_property_definitions += def;
		}
	}

	// Creates a new object @a type with the specified @a properties and returns its ID.
	public StringId64 create_object_type(string type
		, PropertyDefinition[] properties
		, double ui_order = 0.0
		, ObjectTypeFlags flags = ObjectTypeFlags.NONE
		, string? user_data = null
		)
	{
		StringId64 type_hash = StringId64(type);
		assert(!_object_definitions.has_key(type_hash));
		assert(properties.length > 0);

		int first_property = _property_definitions.length;
		int num_properties = first_property + properties.length;

		add_properties(properties);

		ObjectTypeInfo info = {};
		info.properties = { first_property, num_properties };
		info.name = type;
		info.ui_name = camel_case(type);
		info.ui_order = ui_order;
		info.flags = flags;
		info.user_data = user_data;
		info.aspects = new Gee.HashMap<StringId64?, AspectData?>(StringId64.hash_func, StringId64.equal_func);
		_object_definitions[type_hash] = info;

		object_type_added(info);
		return type_hash;
	}

	// Returns the array of properties (i.e. its definition) of the object @a type.
	public unowned PropertyDefinition[]? object_definition(StringId64 type)
	{
		if (!_object_definitions.has_key(type))
			return null;

		PropertiesSlice ps = _object_definitions[type].properties;
		return _property_definitions[ps.start : ps.end];
	}

	// Returns the name of the object @id. If the object has no name set, it returns
	// OBJECT_NAME_UNNAMED.
	public string name(Guid id)
	{
		string name = get_string(id, "editor.name", OBJECT_NAME_UNNAMED);

		if (name == OBJECT_NAME_UNNAMED)
			return get_string(id, "name", OBJECT_NAME_UNNAMED);

		return name;
	}

	// Sets the @a name of the object @a id.
	public void set_name(Guid id, string name)
	{
		set_string(id, "editor.name", name);
	}

	// Returns whether the object @a type exists (i.e. has been created with create_object_type()).
	public bool has_type(StringId64 type)
	{
		return _object_definitions.has_key(type);
	}

	public string type_name(StringId64 type)
	{
		return _object_definitions[type].name;
	}

	public uint type_flags(StringId64 type)
	{
		return _object_definitions[type].flags;
	}

	public ObjectTypeInfo type_info(StringId64 type)
	{
		return _object_definitions[type];
	}

	public Guid?[] all_objects_of_type(StringId64 type)
	{
		Gee.ArrayList<Guid?> all = new Gee.ArrayList<Guid?>();

		foreach (var item in _data) {
			Guid id = item.key;

			if (id != GUID_ZERO
				&& StringId64(object_type(id)) == type
				&& is_alive(id)) {
				all.add(id);
			}
		}

		return all.to_array();
	}

	public bool is_subobject_of(Guid subobject_id, Guid object_id, string set_name)
	{
		assert(has_object(subobject_id));
		assert(has_object(object_id));

		if (!has_property(object_id, set_name))
			return false;

		return get_set(object_id, set_name).contains(subobject_id);
	}

	public bool find_property(ref uint32 property_index, StringId64 object_type, PropertyType type, string name)
	{
		PropertyDefinition[]? properties = object_definition(object_type);

		if (properties == null)
			return false;

		for (int i = 0; i < properties.length; ++i) {
			PropertyDefinition def = properties[i];

			if (def.type == type && def.name == name) {
				property_index = i;
				return true;
			}
		}

		return false;
	}

	public void set_aspect(StringId64 object_type, StringId64 aspect, Aspect callback)
	{
		ObjectTypeInfo info = type_info(object_type);

		AspectData data = AspectData();
		data.callback = callback;

		info.aspects[aspect] = data;
		assert(info.aspects.has_key(aspect));
		assert(get_aspect(object_type, aspect) == callback);
	}

	public unowned Aspect? get_aspect(StringId64 object_type, StringId64 aspect)
	{
		ObjectTypeInfo info = type_info(object_type);

		if (info.aspects.has_key(aspect))
			return info.aspects[aspect].callback;

		return null;
	}
}

public void default_name_aspect(out string name, Database database, Guid id)
{
	name = database.name(id);

	StringId64 object_type = StringId64(database.object_type(id));
	PropertyDefinition[]? properties = database.object_definition(object_type);

	uint32 name_index = 0;
	if (database.find_property(ref name_index, object_type, PropertyType.STRING, "name"))
		name = database.get_string(id, properties[name_index].name);
	else if (database.find_property(ref name_index, object_type, PropertyType.STRING, "editor.name"))
		name = database.get_string(id, properties[name_index].name);
	else
		name = OBJECT_NAME_UNNAMED;
}

} /* namespace Crown */
