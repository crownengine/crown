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

public enum InputDoubleFlags
{
	NONE     = 0,
	INFINITY = 1 << 0,
}

public enum LoadError
{
	SUCCESS   = 0,
	CORRUPTED = -1,
	NOT_FOUND = -2,
}

public delegate void EnumCallback(InputField enum_property, InputEnum property, Project project);
public delegate void ResourceCallback(InputField enum_property, InputResource property, Project project);

public struct PropertyDefinition
{
	public PropertyType type;
	public string name;
	public string? label;

	public PropertyEditorType editor;
	public InputDoubleFlags input_double_flags;
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
	public bool fixed_set; ///< Whether objects cannot be added to or removed from the set.

	public bool hidden;
	public bool not_serialized;
	public string? tooltip;
	public int declaration_order; ///< -1 for undeclared keys; otherwise original declaration order.
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

	public void write_string_id64(StringId64 a)
	{
		write_data(&a._id, sizeof(uint64));
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

	public StringId64 read_string_id64()
	{
		uint64 id = 0;
		read_data(&id, sizeof(uint64));
		return StringId64.from_uint64(id);
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

	public void write_create_action(uint32 action, Guid id, StringId64 type)
	{
		write_string_id64(type);
		write_guid(id);
		write_uint32(action);
	}

	public void write_destroy_action(uint32 action, Guid id, StringId64 type)
	{
		write_string_id64(type);
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

	public void write_set_type_action(uint32 action, Guid id, StringId64 type)
	{
		write_string_id64(type);
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
const string OBJECT_TYPE_DATABASE = "database";
const string OBJECT_TYPE_FILE = "file";
public const StringId64 OBJECT_TYPE_ANY = { 0u };

public enum ObjectTypeFlags
{
	NONE           = 0,
	UNIT_COMPONENT = 1 << 0,
	RESOURCE       = 1 << 1,
}

public delegate void Aspect(out string name, Database database, Guid id);

[Compact]
public struct AspectData
{
	public unowned Aspect callback;
}

public struct ObjectTypeInfo
{
	string[] property_names;
	StringId64[] property_name_ids;
	PropertyDefinition[] property_definitions;
	int num_declared;
	string name;
	string ui_name;
	string? ui_category;
	double ui_order;
	ObjectTypeFlags flags;
	string? user_data;
	GLib.HashTable<StringId64?, AspectData?> aspects;
}

public class Database
{
	public static bool _debug = false;
	public static bool _debug_getters = false;
	private const uint32 PROPERTY_TYPE = 0u;
	private const uint32 PROPERTY_OWNER = 1u;
	private const uint32 PROPERTY_ALIVE = 2u;
	private const uint32 PROPERTY_PREFAB = 3u;
	private const uint32 PROPERTY_FIRST = 4u;

	public enum Action
	{
		CREATE,
		DESTROY,
		SET_NULL,
		SET_BOOL,
		SET_DOUBLE,
		SET_STRING,
		SET_TYPE,
		SET_VECTOR3,
		SET_QUATERNION,
		SET_RESOURCE,
		SET_REFERENCE,
		ADD_TO_SET,
		REMOVE_FROM_SET
	}

	// Data
	public GLib.HashTable<StringId64?, ObjectTypeInfo?> _object_definitions;
	public GLib.HashTable<Guid?, GLib.GenericArray<Value?>> _data;
	public UndoRedo? _undo_redo;
	public Project _project;
	// The number of changes to the database since the last successful state
	// synchronization (load(), save() etc.). If it is less than 0, the changes
	// came from undo(), otherwise they came from redo() or from regular calls to
	// create(), destroy(), set_*() etc. A value of 0 means there were no changes.

	// Signals
	public signal void object_type_added(StringId64 type, ObjectTypeInfo info);
	public signal void objects_created(Guid?[] object_ids, uint32 flags);
	public signal void objects_destroyed(Guid?[] object_ids, uint32 flags);
	public signal void objects_changed(Guid?[] object_ids, uint32 flags);

	public Database(Project project, UndoRedo? undo_redo = null)
	{
		_object_definitions = new GLib.HashTable<StringId64?, ObjectTypeInfo?>(StringId64.hash_func, StringId64.equal_func);
		_data = new GLib.HashTable<Guid?, GLib.GenericArray<Value?>>(Guid.hash_func, Guid.equal_func);
		_project = project;
		_undo_redo = undo_redo;

		PropertyDefinition[] properties;

		properties = {};
		create_object_type(OBJECT_TYPE_DATABASE, properties);

		properties =
		{
			PropertyDefinition()
			{
				type = PropertyType.STRING,
				name = "path",
			},
			PropertyDefinition()
			{
				type = PropertyType.STRING,
				name = "type",
			},
			PropertyDefinition()
			{
				type = PropertyType.STRING,
				name = "name",
			},
			PropertyDefinition()
			{
				type = PropertyType.STRING,
				name = "size",
			},
			PropertyDefinition()
			{
				type = PropertyType.STRING,
				name = "mtime",
			},
		};
		create_object_type(OBJECT_TYPE_FILE, properties);
		reset();
	}

	/// Resets database to clean state.
	public void reset()
	{
		_data.remove_all();

		if (_undo_redo != null)
			_undo_redo.reset();

		// This is a special field which stores all objects
		_data[GUID_ZERO] = new GLib.GenericArray<Value?>(PROPERTY_FIRST);
		_data[GUID_ZERO].length = (int)PROPERTY_FIRST;
	}

	/// Returns whether the database has been changed since last call to Save().
	public bool changed()
	{
		return _undo_redo != null
			? _undo_redo._distance_from_last_sync != 0
			: false
			;
	}

	private void prune_stale_overrides(Guid id)
	{
		StringId64 type = object_type(id);

		if (type == STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f)) {
			prune_stale_unit_overrides(id);
		} else if (type == STRING_ID_64(OBJECT_TYPE_LEVEL, 0x2a690fd348fe9ac5)) {
			Guid?[] units = get_set(id, property_index(id, STRING_ID_64("units", 0x11d87c01b90297db)));
			foreach (unowned Guid? unit_id in units)
				prune_stale_unit_overrides(unit_id);
		}
	}

	private void prune_stale_unit_overrides(Guid unit_id)
	{
		GLib.GenericArray<Guid?> unit_ids = new GLib.GenericArray<Guid?>();
		Unit.collect_unit_tree(unit_ids, unit_id, this);

		for (int i = 0; i < unit_ids.length; ++i)
			Unit(this, unit_ids[i]).prune_stale_overrides();
	}

	private void convert_material(GLib.HashTable<string, Value?> json)
	{
		if (json.contains("textures") && json["textures"].holds(typeof(GLib.HashTable))) {
			GLib.HashTable<string, Value?> old_textures = (GLib.HashTable<string, Value?>)json["textures"];
			GLib.GenericArray<Value?> textures = new GLib.GenericArray<Value?>();

			old_textures.foreach((name, texture_value) => {
					GLib.HashTable<string, Value?> texture = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
					texture["_type"] = OBJECT_TYPE_TEXTURE_SAMPLER;
					texture["name"] = name;
					texture["texture"] = texture_value;
					textures.add(texture);
				});

			json["textures"] = textures;
		}

		if (json.contains("uniforms") && json["uniforms"].holds(typeof(GLib.HashTable))) {
			GLib.HashTable<string, Value?> old_uniforms = (GLib.HashTable<string, Value?>)json["uniforms"];
			GLib.GenericArray<Value?> uniforms = new GLib.GenericArray<Value?>();

			old_uniforms.foreach((name, uniform_value) => {
					GLib.HashTable<string, Value?> old_uniform = (GLib.HashTable<string, Value?>)uniform_value;
					string type = (string)old_uniform["type"];
					Value? value = old_uniform["value"];

					GLib.HashTable<string, Value?> uniform = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
					uniform["name"] = name;

					if (type == "matrix4x4") {
						uniform["_type"] = OBJECT_TYPE_UNIFORM_MATRIX4X4;

						GLib.GenericArray<Value?> matrix = (GLib.GenericArray<Value?>)value;
						string[] rows = { "x", "y", "z", "t" };
						for (int row = 0; row < 4; ++row) {
							int offset = row*4;

							GLib.HashTable<string, Value?> row_value = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
							row_value["x"] = (double)matrix[offset + 0];
							row_value["y"] = (double)matrix[offset + 1];
							row_value["z"] = (double)matrix[offset + 2];
							row_value["w"] = (double)matrix[offset + 3];
							uniform[rows[row]] = row_value;
						}
					} else {
						uniform["_type"] = OBJECT_TYPE_UNIFORM_VECTOR4;

						double[] v = { 0.0, 0.0, 0.0, 0.0 };
						if (value.holds(typeof(double))) {
							v[0] = (double)value;
						} else {
							GLib.GenericArray<Value?> arr = (GLib.GenericArray<Value?>)value;
							for (int i = 0; i < arr.length && i < 4; ++i) {
								v[i] = (double)arr[i];
							}
						}

						GLib.HashTable<string, Value?> vector = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
						vector["x"] = v[0];
						vector["y"] = v[1];
						vector["z"] = v[2];
						vector["w"] = v[3];
						uniform["value"] = vector;
					}

					uniforms.add(uniform);
				});

			json["uniforms"] = uniforms;
		}
	}

	private Guid legacy_mesh_material_guid(Guid component_id)
	{
		Guid id = component_id;
		id.data1 ^= 0x8000000000000000u;
		return id;
	}

	private void convert_mesh_renderer(Guid id, GLib.HashTable<string, Value?> json)
	{
		// Copy the legacy whole-mesh material into the normal material slot set.
		if (!json.contains("data") || !json["data"].holds(typeof(GLib.HashTable)))
			return;

		GLib.HashTable<string, Value?> data = (GLib.HashTable<string, Value?>)json["data"];
		if (!data.contains("material") || !data["material"].holds(typeof(string)))
			return;

		if (data.contains("materials"))
			return;

		GLib.GenericArray<Value?> materials = new GLib.GenericArray<Value?>();
		data["materials"] = materials;

		GLib.HashTable<string, Value?> binding_data = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
		binding_data["slot"] = "default";
		binding_data["material"] = data["material"];

		GLib.HashTable<string, Value?> binding = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
		binding["_guid"] = legacy_mesh_material_guid(id).to_string();
		binding["_type"] = OBJECT_TYPE_MESH_MATERIAL;
		binding["data"] = binding_data;

		materials.add(binding);
	}

	/// Saves database to path without marking it as not changed.
	public int dump(string path, Guid id)
	{
		UndoRedo? undo_redo = disable_undo();
		try {
			prune_stale_overrides(id);
			SJSON.save(encode(id), path);
			return 0;
		} catch (JsonWriteError e) {
			return -1;
		} finally {
			restore_undo(undo_redo);
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

	public UndoRedo? disable_undo()
	{
		var undo = _undo_redo;
		_undo_redo = null;
		return undo;
	}

	public void restore_undo(UndoRedo? undo_redo)
	{
		_undo_redo = undo_redo;
	}

	// See: add_from_path().
	public LoadError add_from_file(out Guid object_id, GLib.File file, string resource_path)
	{
		object_id = GUID_ZERO;

		try {
			uint8[] bytes;
			string? etag;
			file.load_contents(null, out bytes, out etag);
			GLib.HashTable<string, Value?> json = SJSON.decode(bytes);

			UndoRedo? undo_redo = disable_undo();
			try {
				// Parse the object's ID or generate a new one if none is found.
				if (json.contains("id"))
					object_id = Guid.parse((string)json["id"]);
				else if (json.contains("_guid"))
					object_id = Guid.parse((string)json["_guid"]);
				else
					object_id = Guid.new_guid();

				string type = ResourceId.type(resource_path);
				StringId64 type_hash = StringId64(type);
				assert(has_type(type_hash));

				_data[object_id] = new GLib.GenericArray<Value?>(PROPERTY_FIRST);
				set_type(object_id, type_hash);
				set_owner(object_id, GUID_ZERO);
				set_alive(object_id, true);
				_data[object_id].length = (int)PROPERTY_FIRST;

				if (has_type(type_hash) && !json.contains("_prefab"))
					_init_object(object_id, object_definition(type_hash));

				decode_object(object_id, GUID_ZERO, "", json);

				// Create a mapping between the path and the object it has been loaded into.
				set(0, GUID_ZERO, resource_path, object_id);

				prune_stale_overrides(object_id);

				return LoadError.SUCCESS;
			} finally {
				restore_undo(undo_redo);
			}
		} catch (GLib.IOError.NOT_FOUND e) {
			return LoadError.NOT_FOUND;
		} catch (GLib.IOError.NOT_DIRECTORY e) {
			return LoadError.NOT_FOUND;
		} catch (JsonSyntaxError e) {
			object_id = GUID_ZERO;
			return LoadError.CORRUPTED;
		} catch (GLib.Error e) {
			return LoadError.CORRUPTED;
		}
	}

	// Adds the object stored at @a path to the database.
	// This makes it possible to load multiple objects from distinct
	// paths in the same database. @a resource_path is used as a key in the
	// database to refer to the object that has been loaded. This is useful when
	// you do not have the object ID but only its path, as it is often the case
	// since resources use paths and not IDs to reference each other.
	public LoadError add_from_path(out Guid object_id, string path, string resource_path)
	{
		return add_from_file(out object_id, GLib.File.new_for_path(path), resource_path);
	}

	public LoadError add_from_resource_path(out Guid object_id, string resource_path)
	{
		// If the resource is already loaded.
		if (has_property(GUID_ZERO, resource_path)) {
			object_id = get_reference(GUID_ZERO, property_index(GUID_ZERO, StringId64(resource_path)));
			return LoadError.SUCCESS;
		}

		string path = _project.absolute_path(resource_path);
		return add_from_path(out object_id, path, resource_path);
	}

	/// Loads the database with the object stored at @a file.
	public LoadError load_from_file(out Guid object_id, GLib.File file, string resource_path)
	{
		reset();
		return add_from_file(out object_id, file, resource_path);
	}

	/// Loads the database with the object stored at @a path.
	public LoadError load_from_path(out Guid object_id, string path, string resource_path)
	{
		reset();
		return add_from_path(out object_id, path, resource_path);
	}

	/// Encodes the object @a id into SJSON object.
	public GLib.HashTable<string, Value?> encode(Guid id)
	{
		return encode_object(id);
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

	public static bool is_valid_key(Guid object_id, string key)
	{
		return object_id == GUID_ZERO
			? key.length > 0
			: key.length > 0
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
		if (value.holds(typeof(GLib.GenericSet)))
			return "Set<Guid>";

		return "<invalid>";
	}

	public void decode_object_compat(Guid id, Guid owner_id, string db_key, GLib.HashTable<string, Value?> json)
	{
		string old_db = db_key;
		string k = db_key;

		GLib.HashTableIter<string, Value?> iter = GLib.HashTableIter<string, Value?>(json);
		unowned string key;
		unowned Value? val;
		while (iter.next(out key, out val)) {
			// ID is filled by decode_set().
			if (key == "id"
				|| key == "_guid"
				|| key == "_alive"
				|| key == "prefab"
				|| key == "children"
				)
				continue;

			k += k == "" ? key : ("." + key);

			if (val.holds(typeof(GLib.HashTable))) {
				GLib.HashTable<string, Value?> ht = (GLib.HashTable<string, Value?>)val;
				decode_object(id, owner_id, k, ht);
			} else if (val.holds(typeof(GLib.GenericArray))) {
				GLib.GenericArray<Value?> arr = (GLib.GenericArray<Value?>)val;
				if (arr.length > 0
					&& arr[0].holds(typeof(double))
					&& k != "frames" // sprite_animation
					)
					set(0, id, k, decode_value(val));
				else
					decode_set(id, k, arr);
			} else {
				set(0, id, k, decode_value(val));
			}

			k = old_db;
		}
	}

	public void decode_object_from_properties(Guid id, Guid owner_id, ObjectTypeInfo? info, GLib.HashTable<string, Value?> json)
	{
		// Nested objects can append dynamic properties and replace this schema's array.
		for (int property_i = 0; property_i < info.num_declared; ++property_i) {
			unowned PropertyDefinition def = info.property_definitions[property_i];
			// Find table and key to read from.
			string[] keys = def.name.split(".");
			string key = keys[keys.length - 1];
			GLib.HashTable<string, Value?> input = json;

			if (keys.length > 1) {
				for (int i = 0; i < keys.length - 1; ++i) {
					string f = keys[i];

					if (input.contains(f)) {
						input = (GLib.HashTable<string, Value?>)input[f];
						continue;
					}
				}
			}

			if (!input.contains(key))
				continue;

			// Read property.
			if (def.type == PropertyType.OBJECTS_SET) {
				decode_set(id, def.name, (GLib.GenericArray<Value?>)input[key]);
			} else if (def.type == PropertyType.RESOURCE) {
				Resource res = { null };
				if (input.contains(key)) {
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

	public void decode_object(Guid id, Guid owner_id, string db_key, GLib.HashTable<string, Value?> json)
	{
		StringId64 type;

		// The "type" key defines object type only if it appears
		// in the root of a JSON object (k == "").
		if (db_key == "" && !has_property(id, "_type")) {
			string? type_name = null;
			if (json.contains("_type"))
				type_name = (string)json["_type"];
			else if (json.contains("type"))
				type_name = (string)json["type"];

			assert(type_name != null);
			type = StringId64(type_name);
			assert(has_type(type));
			set_type(id, type);

			if (!json.contains("_prefab"))
				_init_object(id, object_definition(type));
		} else {
			type = object_type(id);
		}

		if (db_key == "" && json.contains("_prefab"))
			set(0, id, "_prefab", Guid.parse((string)json["_prefab"]));
		if (type == STRING_ID_64(OBJECT_TYPE_MATERIAL, 0xeac0b497876adedf))
			convert_material(json);
		else if (type == STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893))
			convert_mesh_renderer(id, json);

		unowned ObjectTypeInfo? info = type_info(type);
		if (info != null)
			decode_object_from_properties(id, owner_id, info, json);

		if (type == STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f) || (type == STRING_ID_64(OBJECT_TYPE_MATERIAL, 0xeac0b497876adedf) && info == null))
			decode_object_compat(id, owner_id, db_key, json);
	}

	public void decode_set(Guid owner_id, string key, GLib.GenericArray<Value?> json)
	{
		// Set should be created even if it is empty.
		create_empty_set(owner_id, key);
		StringId64 owner_type = object_type(owner_id);

		for (int i = 0; i < json.length; ++i) {
			GLib.HashTable<string, Value?> obj;
			obj = (GLib.HashTable<string, Value?>)json[i];

			// Decode object ID.
			Guid obj_id;
			if (obj.contains("id") && owner_type != STRING_ID_64(OBJECT_TYPE_FONT, 0x9efe0a916aae7880))
				obj_id = Guid.parse((string)obj["id"]);
			else if (obj.contains("_guid"))
				obj_id = Guid.parse((string)obj["_guid"]);
			else
				obj_id = Guid.new_guid();

			_data[obj_id] = new GLib.GenericArray<Value?>(PROPERTY_FIRST);

			set_owner(obj_id, owner_id);
			set_alive(obj_id, true);
			_data[obj_id].length = (int)PROPERTY_FIRST;
			decode_object(obj_id, owner_id, "", obj);
			assert(has_property(obj_id, "_type"));

			add_to_set_internal(0, owner_id, key, obj_id);
		}
	}

	public Value? decode_value(Value? value)
	{
		if (value.holds(typeof(GLib.GenericArray))) {
			GLib.GenericArray<Value?> al = (GLib.GenericArray<Value?>)value;
			if (al.length == 1)
				return Vector3((double)al[0], 0.0, 0.0);
			else if (al.length == 2)
				return Vector3((double)al[0], (double)al[1], 0.0);
			else if (al.length == 3)
				return Vector3((double)al[0], (double)al[1], (double)al[2]);
			else if (al.length == 4)
				return Quaternion((double)al[0], (double)al[1], (double)al[2], (double)al[3]);
			else
				return Vector3(0.0, 0.0, 0.0);
		} else if (value.holds(typeof(string))) {
			Guid id;
			if (Guid.try_parse(out id, (string)value))
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

	public GLib.HashTable<string, Value?> encode_object_compat(Guid id, GLib.GenericArray<Value?> db)
	{
		GLib.HashTable<string, Value?> obj = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
		if (id != GUID_ZERO)
			obj["_guid"] = id.to_string();

		for (uint32 property = 0; property < db.length; ++property) {
			Value? value = db[property];
			if (value == null || property == PROPERTY_OWNER || property == PROPERTY_ALIVE)
				continue;
			if (property == PROPERTY_TYPE) {
				obj["_type"] = type_name(object_type(id));
				continue;
			}

			string key = property_name(id, property);
			string[] foo = key.split(".");
			GLib.HashTable<string, Value?> x = obj;
			if (foo.length > 1) {
				for (int i = 0; i < foo.length - 1; ++i) {
					string f = foo[i];

					if (x.contains(f)) {
						x = (GLib.HashTable<string, Value?>)x[f];
						continue;
					}

					GLib.HashTable<string, Value?> y = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
					x.set(f, y);
					x = y;
				}
			}
			x.set(foo[foo.length - 1], encode_value(value));
		}

		return obj;
	}

	public GLib.HashTable<string, Value?> encode_object(Guid id)
	{
		assert(is_alive(id));

		StringId64 type = object_type(id);
		unowned PropertyDefinition[]? properties = object_definition(type);

		if (type == STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f) || type == STRING_ID_64(OBJECT_TYPE_MATERIAL, 0xeac0b497876adedf) || properties == null)
			return encode_object_compat(id, get_data(id));

		GLib.HashTable<string, Value?> obj = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
		if (id != GUID_ZERO) {
			obj["_guid"] = id.to_string();
			obj["_type"] = type_name(type);
			unowned Value? prefab = get_local(id, PROPERTY_PREFAB);
			if (prefab != null)
				obj["_prefab"] = ((Guid)prefab).to_string();
		}

		unowned GLib.GenericArray<Value?> values = _data[id];
		foreach (PropertyDefinition def in properties) {
			if (def.not_serialized)
				continue;

			// Since null-key is equivalent to non-existent key, skip serialization.
			uint32 property = property_index(id, StringId64(def.name));
			if (property >= values.length)
				continue;
			unowned Value? value = get_local(id, property);
			if (value == null)
				continue;

			string[] foo = def.name.split(".");
			GLib.HashTable<string, Value?> x = obj;
			if (foo.length > 1) {
				for (int i = 0; i < foo.length - 1; ++i) {
					string f = foo[i];

					if (x.contains(f)) {
						x = (GLib.HashTable<string, Value?>)x[f];
						continue;
					}

					GLib.HashTable<string, Value?> y = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
					x.set(f, y);
					x = y;
				}
			}
			x.set(foo[foo.length - 1], encode_value(value));
		}

		return obj;
	}

	public Value? encode_value(Value? value)
	{
		assert(is_valid_value(value) || value.holds(typeof(GLib.GenericSet)));

		if (value.holds(typeof(Vector3))) {
			Vector3 v = (Vector3)value;
			GLib.GenericArray<Value?> arr = new GLib.GenericArray<Value?>();
			arr.add(v.x);
			arr.add(v.y);
			arr.add(v.z);
			return arr;
		} else if (value.holds(typeof(Quaternion))) {
			Quaternion q = (Quaternion)value;
			GLib.GenericArray<Value?> arr = new GLib.GenericArray<Value?>();
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
		} else if (value.holds(typeof(GLib.GenericSet))) {
			GLib.GenericSet<Guid?> hs = (GLib.GenericSet<Guid?>)value;
			GLib.GenericArray<Value?> arr = new GLib.GenericArray<Value?>();
			foreach (Guid? id in hs) {
				if (!is_alive(id))
					continue;
				arr.add(encode_object(id));
			}
			return arr;
		} else {
			return value;
		}
	}

	private uint32 find_property_index(Guid id, StringId64 key)
	{
		if (key == STRING_ID_64("_type", 0xbca6069732c8be59))
			return PROPERTY_TYPE;
		if (key == STRING_ID_64("_owner", 0x47565d780b69e979))
			return PROPERTY_OWNER;
		if (key == STRING_ID_64("_alive", 0x98efc3855fa92703))
			return PROPERTY_ALIVE;
		if (key == STRING_ID_64("_prefab", 0xeb91306c1265f913))
			return PROPERTY_PREFAB;

		unowned ObjectTypeInfo? info;
		if (id == GUID_ZERO) {
			info = _object_definitions[STRING_ID_64(OBJECT_TYPE_DATABASE, 0x6d90dd26dff90856)];
		} else {
			unowned Value? type_value = _data[id][PROPERTY_TYPE];
			unowned StringId64? type = (StringId64?)type_value.get_boxed();
			info = _object_definitions[type];
		}
		assert(info != null);
		for (uint32 i = 0; i < info.property_name_ids.length; ++i) {
			if (info.property_name_ids[i] == key)
				return PROPERTY_FIRST + i;
		}
		return uint32.MAX;
	}

	public uint32 property_index(Guid id, StringId64 key)
	{
		assert(has_object(id));
		return find_property_index(id, key);
	}

	// Register undeclared keys when they are written.
	private uint32 ensure_property_index(Guid id, string key)
	{
		StringId64 key_id = StringId64(key);
		uint32 property = find_property_index(id, key_id);
		if (property != uint32.MAX)
			return property;

		StringId64 type_hash = object_type(id);
		unowned ObjectTypeInfo? info = _object_definitions[type_hash];
		assert(info != null);
		uint32 index = PROPERTY_FIRST + (uint32)info.property_name_ids.length;
		// Move the arrays to avoid copying them when appending a new property.
		string[] names = (owned)info.property_names;
		names += key;
		info.property_names = (owned)names;
		StringId64[] name_ids = (owned)info.property_name_ids;
		name_ids += key_id;
		info.property_name_ids = (owned)name_ids;
		PropertyDefinition[] definitions = (owned)info.property_definitions;
		definitions += PropertyDefinition()
		{
			name = key, declaration_order = -1
		};
		info.property_definitions = (owned)definitions;
		return index;
	}

	private string property_name(Guid id, uint32 property)
	{
		if (property == PROPERTY_TYPE)
			return "_type";
		if (property == PROPERTY_OWNER)
			return "_owner";
		if (property == PROPERTY_ALIVE)
			return "_alive";
		if (property == PROPERTY_PREFAB)
			return "_prefab";
		unowned ObjectTypeInfo? info = _object_definitions[object_type(id)];
		assert(property >= PROPERTY_FIRST && property - PROPERTY_FIRST < info.property_names.length);
		return info.property_names[property - PROPERTY_FIRST];
	}

	public GLib.GenericArray<Value?> get_data(Guid id)
	{
		assert(has_object(id));

		return _data[id];
	}

	private unowned Value? get_local(Guid id, uint32 property)
	{
		unowned GLib.GenericArray<Value?> values = _data[id];
		assert(property < values.length);
		return values[property];
	}

	private void set_local(Guid id, uint32 property, owned Value? value)
	{
		unowned GLib.GenericArray<Value?> values = _data[id];
		while (values.length < property)
			values.add(null);
		if (values.length == property)
			values.add((owned)value);
		else
			values[property] = (owned)value;
	}

	public void set(int dir, Guid id, string key, Value? value)
	{
		assert(has_object(id));
		assert(is_valid_key(id, key));
		assert(is_valid_value(value));

		if (_debug)
			logi("set_property %s %s %s".printf(debug_string(id), key, debug_string(value)));

		if (key == "_type" && value != null)
			set_type(id, StringId64((string)value));
		else
			set_local(id, ensure_property_index(id, key), value);

		if (_undo_redo != null)
			_undo_redo._distance_from_last_sync += dir;
	}

	public void create_empty_set(Guid id, string key)
	{
		assert(has_object(id));
		assert(is_valid_key(id, key));

		set_local(id, ensure_property_index(id, key), guid_set_new());
	}

	public void add_to_set_internal(int dir, Guid id, string key, Guid item_id)
	{
		assert(has_object(id));
		assert(is_valid_key(id, key));
		assert(item_id != GUID_ZERO);
		assert(has_object(item_id));

		if (_debug)
			logi("add_to_set %s %s %s".printf(debug_string(id), key, debug_string(item_id)));

		uint32 property = ensure_property_index(id, key);
		unowned Value? value = property < _data[id].length ? get_local(id, property) : null;

		if (value == null) {
			GLib.GenericSet<Guid?> hs = guid_set_new();
			hs.add(item_id);
			set_local(id, property, hs);
		} else {
			((GLib.GenericSet<Guid?>)value).add(item_id);
		}

		set_local(item_id, PROPERTY_OWNER, id);

		if (_undo_redo != null)
			_undo_redo._distance_from_last_sync += dir;
	}

	public void remove_from_set_internal(int dir, Guid id, string key, Guid item_id)
	{
		assert(has_object(id));
		assert(is_valid_key(id, key));
		assert(item_id != GUID_ZERO);

		if (_debug)
			logi("remove_from_set %s %s %s".printf(debug_string(id), key, debug_string(item_id)));

		((GLib.GenericSet<Guid?>)get_local(id, property_index(id, StringId64(key)))).remove(item_id);

		set_owner(item_id, GUID_ZERO);

		if (_undo_redo != null)
			_undo_redo._distance_from_last_sync += dir;
	}

	// Returns the schema ID of the object @a id.
	public StringId64 object_type(Guid id)
	{
		assert(has_object(id));

		if (id == GUID_ZERO)
			return STRING_ID_64(OBJECT_TYPE_DATABASE, 0x6d90dd26dff90856);
		unowned Value? type = get_local(id, PROPERTY_TYPE);
		return (StringId64)type;
	}

	// Returns the owner of @a id.
	public Guid owner(Guid id)
	{
		assert(has_object(id));
		return (Guid)get_local(id, PROPERTY_OWNER);
	}

	// Sets the @a type of the object @a id.
	// This is called automatically when loading data or when new objects are created via create().
	// It can occasionally be called manually after loading legacy data with no type information
	// stored inside objects.
	public void set_type(Guid id, StringId64 type)
	{
		assert(has_object(id));
		assert(has_type(type));
		set_local(id, PROPERTY_TYPE, type);
	}

	public void set_owner(Guid id, Guid owner_id)
	{
		assert(has_object(id));
		assert(has_object(owner_id));
		set_local(id, PROPERTY_OWNER, owner_id);
	}

	public void set_alive(Guid id, bool alive)
	{
		assert(has_object(id));
		set_local(id, PROPERTY_ALIVE, alive);
	}

	public bool is_alive(Guid id)
	{
		return id == GUID_ZERO
			|| has_object(id) && (bool)get_local(id, PROPERTY_ALIVE)
			;
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

	public void create_empty(Guid id, StringId64 type)
	{
		assert(id != GUID_ZERO);
		assert(!has_object(id));
		assert(has_type(type));

		if (_debug)
			logi("create %s".printf(debug_string(id)));

		if (_undo_redo != null) {
			_undo_redo._distance_from_last_sync += 1;
			_undo_redo._undo.write_destroy_action(Action.DESTROY, id, type);
			_undo_redo._redo.clear();
		}

		_data[id] = new GLib.GenericArray<Value?>(PROPERTY_FIRST);
		set_type(id, type);
		set_owner(id, GUID_ZERO);
		set_alive(id, true);
		_data[id].length = (int)PROPERTY_FIRST;
	}

	public void create(Guid id, StringId64 type)
	{
		create_empty(id, type);
		_init_object(id, object_definition(type));
	}

	public void create_from_prefab(Guid id, Guid prefab_id)
	{
		assert(is_alive(prefab_id));
		// Instances store only their own properties; missing values come from _prefab.
		create_empty(id, object_type(prefab_id));
		set_reference(id, "_prefab", prefab_id);
	}

	public void destroy(Guid id)
	{
		assert(id != GUID_ZERO);
		assert(has_object(id));

		StringId64 obj_type = object_type(id);

		GLib.GenericArray<Value?> values = get_data(id);
		foreach (unowned Value? value in values) {
			if (value != null && value.holds(typeof(GLib.GenericSet))) {
				GLib.GenericSet<Guid?> hs = (GLib.GenericSet<Guid?>)value;
				foreach (Guid? item_id in hs) {
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
		assert(is_valid_key(id, key));
		assert(is_valid_value(null));

		if (_undo_redo != null) {
			GLib.GenericArray<Value?> ob = get_data(id);
			uint32 property = property_index(id, StringId64(key));
			if (property < ob.length && ob[property] != null) {
				if (property == PROPERTY_TYPE)
					_undo_redo._undo.write_set_type_action(Action.SET_TYPE, id, object_type(id));
				if (ob[property].holds(typeof(bool)))
					_undo_redo._undo.write_set_bool_action(Action.SET_BOOL, id, key, (bool)ob[property]);
				if (ob[property].holds(typeof(double)))
					_undo_redo._undo.write_set_double_action(Action.SET_DOUBLE, id, key, (double)ob[property]);
				if (ob[property].holds(typeof(string)))
					_undo_redo._undo.write_set_string_action(Action.SET_STRING, id, key, (string)ob[property]);
				if (ob[property].holds(typeof(Vector3)))
					_undo_redo._undo.write_set_vector3_action(Action.SET_VECTOR3, id, key, (Vector3)ob[property]);
				if (ob[property].holds(typeof(Quaternion)))
					_undo_redo._undo.write_set_quaternion_action(Action.SET_QUATERNION, id, key, (Quaternion)ob[property]);
				if (ob[property].holds(typeof(Resource)))
					_undo_redo._undo.write_set_resource_action(Action.SET_RESOURCE, id, key, (Resource)ob[property]);
				if (ob[property].holds(typeof(Guid)))
					_undo_redo._undo.write_set_reference_action(Action.SET_REFERENCE, id, key, (Guid)ob[property]);
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
		assert(is_valid_key(id, key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			GLib.GenericArray<Value?> ob = get_data(id);
			uint32 property = property_index(id, StringId64(key));
			if (property < ob.length && ob[property] != null)
				_undo_redo._undo.write_set_bool_action(Action.SET_BOOL, id, key, (bool)ob[property]);
			else
				_undo_redo._undo.write_set_null_action(Action.SET_NULL, id, key);

			_undo_redo._redo.clear();
		}

		set(1, id, key, val);
	}

	public void set_double(Guid id, string key, double val)
	{
		assert(has_object(id));
		assert(is_valid_key(id, key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			GLib.GenericArray<Value?> ob = get_data(id);
			uint32 property = property_index(id, StringId64(key));
			if (property < ob.length && ob[property] != null)
				_undo_redo._undo.write_set_double_action(Action.SET_DOUBLE, id, key, (double)ob[property]);
			else
				_undo_redo._undo.write_set_null_action(Action.SET_NULL, id, key);

			_undo_redo._redo.clear();
		}

		set(1, id, key, val);
	}

	public void set_string(Guid id, string key, string val)
	{
		assert(has_object(id));
		assert(is_valid_key(id, key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			GLib.GenericArray<Value?> ob = get_data(id);
			uint32 property = property_index(id, StringId64(key));
			if (property < ob.length && ob[property] != null) {
				if (property == PROPERTY_TYPE)
					_undo_redo._undo.write_set_type_action(Action.SET_TYPE, id, object_type(id));
				else
					_undo_redo._undo.write_set_string_action(Action.SET_STRING, id, key, (string)ob[property]);
			} else {
				_undo_redo._undo.write_set_null_action(Action.SET_NULL, id, key);
			}

			_undo_redo._redo.clear();
		}

		set(1, id, key, val);
	}

	public void set_vector3(Guid id, string key, Vector3 val)
	{
		assert(has_object(id));
		assert(is_valid_key(id, key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			GLib.GenericArray<Value?> ob = get_data(id);
			uint32 property = property_index(id, StringId64(key));
			if (property < ob.length && ob[property] != null)
				_undo_redo._undo.write_set_vector3_action(Action.SET_VECTOR3, id, key, (Vector3)ob[property]);
			else
				_undo_redo._undo.write_set_null_action(Action.SET_NULL, id, key);

			_undo_redo._redo.clear();
		}

		set(1, id, key, val);
	}

	public void set_quaternion(Guid id, string key, Quaternion val)
	{
		assert(has_object(id));
		assert(is_valid_key(id, key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			GLib.GenericArray<Value?> ob = get_data(id);
			uint32 property = property_index(id, StringId64(key));
			if (property < ob.length && ob[property] != null)
				_undo_redo._undo.write_set_quaternion_action(Action.SET_QUATERNION, id, key, (Quaternion)ob[property]);
			else
				_undo_redo._undo.write_set_null_action(Action.SET_NULL, id, key);

			_undo_redo._redo.clear();
		}

		set(1, id, key, val);
	}

	public void set_resource(Guid id, string key, string? val)
	{
		assert(has_object(id));
		assert(is_valid_key(id, key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			GLib.GenericArray<Value?> ob = get_data(id);
			uint32 property = property_index(id, StringId64(key));
			if (property >= ob.length || ob[property] == null) {
				_undo_redo._undo.write_set_null_action(Action.SET_NULL, id, key);
			} else {
				Value? old_value = ob[property];
				Resource old_resource = { null };
				// Unit component overrides loaded by decode_object_compat() are strings.
				if (old_value.holds(typeof(string))) {
					old_resource.name = (string)old_value;
				} else {
					assert(old_value.holds(typeof(Resource)));
					old_resource = (Resource)old_value;
				}
				_undo_redo._undo.write_set_resource_action(Action.SET_RESOURCE, id, key, old_resource);
			}

			_undo_redo._redo.clear();
		}

		Resource res = { val };
		set(1, id, key, res);
	}

	public void set_reference(Guid id, string key, Guid val)
	{
		assert(has_object(id));
		assert(is_valid_key(id, key));
		assert(is_valid_value(val));

		if (_undo_redo != null) {
			GLib.GenericArray<Value?> ob = get_data(id);
			uint32 property = property_index(id, StringId64(key));
			if (property < ob.length && ob[property] != null)
				_undo_redo._undo.write_set_reference_action(Action.SET_REFERENCE, id, key, (Guid)ob[property]);
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
		assert(is_valid_key(id, key));
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
		assert(is_valid_key(id, key));
		assert(item_id != GUID_ZERO);

		if (_undo_redo != null) {
			_undo_redo._undo.write_add_to_set_action(Action.ADD_TO_SET, id, key, item_id);
			_undo_redo._redo.clear();
		}

		remove_from_set_internal(1, id, key, item_id);
	}

	public bool has_object(Guid id)
	{
		return id == GUID_ZERO || _data.contains(id);
	}

	public bool has_property(Guid id, string key)
	{
		return get_property(id, key) != null;
	}

	private bool has_local_property(Guid id, string key)
	{
		uint32 property = find_property_index(id, StringId64(key));
		return property != uint32.MAX && property < _data[id].length && get_local(id, property) != null;
	}

	private Guid legacy_mesh_material_source(GLib.GenericSet<Guid?> objects)
	{
		foreach (unowned Guid? id in objects) {
			if (!is_alive(id) || object_type(id) != STRING_ID_64(OBJECT_TYPE_MESH_MATERIAL, 0x6701353384dd782d))
				continue;
			Guid component_id = owner(id);
			if (component_id != GUID_ZERO
				&& is_alive(component_id)
				&& object_type(component_id) == STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893)
				&& Guid.equal_func(id, legacy_mesh_material_guid(component_id))
				)
				return id;
		}

		return GUID_ZERO;
	}

	private bool mesh_material_overrides_inherited_slot(Guid local_id, GLib.GenericSet<Guid?> objects)
	{
		if (object_type(local_id) != STRING_ID_64(OBJECT_TYPE_MESH_MATERIAL, 0x6701353384dd782d)
			|| !has_local_property(local_id, "data.slot")
			)
			return false;

		unowned string slot = (string)get_local(local_id, property_index(local_id, STRING_ID_64("data.slot", 0x0de060e1cd2f27fe)));
		if (slot == "")
			return false;

		foreach (unowned Guid? id in objects) {
			if (is_alive(id)
				&& object_type(id) == STRING_ID_64(OBJECT_TYPE_MESH_MATERIAL, 0x6701353384dd782d)
				&& get_string(id, property_index(id, STRING_ID_64("data.slot", 0x0de060e1cd2f27fe))) == slot
				)
				return true;
		}

		return false;
	}

	public Value? inherit_value(Value? local, Value? inherited)
	{
		if (local == null)
			return inherited;
		if (!local.holds(typeof(GLib.GenericSet)))
			return local;
		if (inherited != null && !inherited.holds(typeof(GLib.GenericSet)))
			return local;

		GLib.GenericSet<Guid?> merged = guid_set_new();
		if (inherited != null) {
			foreach (unowned Guid? id in (GLib.GenericSet<Guid?>)inherited) {
				if (is_alive(id))
					merged.add(id);
			}
		}
		foreach (unowned Guid? id in (GLib.GenericSet<Guid?>)local) {
			if (!is_alive(id))
				continue;
			Guid prefab_id = get_reference(id, PROPERTY_PREFAB);
			// Ignore unsupported full material overrides produced before inherited
			// object-set instances had a serialized _prefab link.
			if (prefab_id == GUID_ZERO
				&& inherited != null
				&& mesh_material_overrides_inherited_slot(id, (GLib.GenericSet<Guid?>)inherited)
				)
				continue;
			if (prefab_id != GUID_ZERO) {
				// A local instance replaces its source only while the source is in the set.
				if (!merged.contains(prefab_id)) {
					Guid legacy_source = object_type(id) == STRING_ID_64(OBJECT_TYPE_MESH_MATERIAL, 0x6701353384dd782d)
					&& !is_alive(prefab_id)
					&& inherited != null
						? legacy_mesh_material_source((GLib.GenericSet<Guid?>)inherited)
						: GUID_ZERO
						;
					if (legacy_source == GUID_ZERO)
						continue;
					prefab_id = legacy_source;
					set(0, id, "_prefab", prefab_id);
					if (!merged.contains(prefab_id))
						continue;
				}
				merged.remove(prefab_id);
			}
			merged.add(id);
		}

		return merged;
	}

	private Value? get_inherited_property(Guid id, uint32 property, GLib.GenericSet<Guid?> visited)
	{
		if (!visited.add(id))
			return null;

		unowned Value? local = property < _data[id].length ? get_local(id, property) : null;
		unowned Value? prefab = get_local(id, PROPERTY_PREFAB);
		if (prefab == null || (local != null && !local.holds(typeof(GLib.GenericSet))))
			return local;

		Guid prefab_id = (Guid)prefab;
		if (prefab_id == GUID_ZERO || !is_alive(prefab_id) || object_type(prefab_id) != object_type(id))
			return local;

		Value? inherited = get_inherited_property(prefab_id, property, visited);
		return inherit_value(local, inherited);
	}

	private Value? get_property_at(Guid id, uint32 property, Value? val = null)
	{
		assert(has_object(id));
		if (property == uint32.MAX)
			return val;
		if (property == PROPERTY_TYPE) {
			unowned Value? type_hash = get_local(id, PROPERTY_TYPE);
			if (type_hash == null)
				return val;
			return type_hash;
		}

		string key = property_name(id, property);
		unowned Value? local = property < _data[id].length ? get_local(id, property) : null;
		Value? value = local;
		if (!key.has_prefix("_")
			&& (local == null || local.holds(typeof(GLib.GenericSet)))
			&& get_local(id, PROPERTY_PREFAB) != null
			)
			value = get_inherited_property(id, property, new GLib.GenericSet<Guid?>(Guid.hash_func, Guid.equal_func));
		if (value == null)
			value = val;

		if (_debug_getters)
			logi("get_property %s %s %s".printf(debug_string(id), key, debug_string(value)));

		return value;
	}

	public Value? get_property(Guid id, string key, Value? val = null)
	{
		assert(has_object(id));
		assert(is_valid_key(id, key));
		uint32 property = find_property_index(id, StringId64(key));
		if (property == uint32.MAX) {
			if (_debug_getters)
				logi("get_property %s %s %s".printf(debug_string(id), key, debug_string(val)));
			return val;
		}
		return get_property_at(id, property, val);
	}

	public bool get_bool(Guid id, uint32 property, bool deffault = false)
	{
		return (bool)get_property_at(id, property, deffault);
	}

	public double get_double(Guid id, uint32 property, double deffault = 0.0)
	{
		return (double)get_property_at(id, property, deffault);
	}

	public string get_string(Guid id, uint32 property, string deffault = "")
	{
		return (string)get_property_at(id, property, deffault);
	}

	public Vector3 get_vector3(Guid id, uint32 property, Vector3 deffault = VECTOR3_ZERO)
	{
		return (Vector3)get_property_at(id, property, deffault);
	}

	public Quaternion get_quaternion(Guid id, uint32 property, Quaternion deffault = QUATERNION_IDENTITY)
	{
		return (Quaternion)get_property_at(id, property, deffault);
	}

	public string? get_resource(Guid id, uint32 property, string? deffault = null)
	{
		Resource deffault_res = { deffault };
		Value? val = get_property_at(id, property, deffault_res);
		assert(val == null || val.holds(typeof(Crown.Resource)));
		return ((Resource)val).name;
	}

	public Guid get_reference(Guid id, uint32 property, Guid deffault = GUID_ZERO)
	{
		return (Guid)get_property_at(id, property, deffault);
	}

	public Guid?[] get_set(Guid id, uint32 property)
	{
		assert(has_object(id));

		GLib.GenericArray<Guid?> value = new GLib.GenericArray<Guid?>();
		Value? set_value = get_property_at(id, property);
		if (set_value != null) {
			GLib.GenericSet<Guid?> objects = (GLib.GenericSet<Guid?>)set_value;
			foreach (unowned Guid? obj in objects) {
				if (is_alive(obj))
					value.add(obj);
			}
		}

		if (_debug_getters && property != uint32.MAX)
			logi("get_property %s %s Set<Guid>".printf(debug_string(id), property_name(id, property)));

		return value.steal();
	}

	public string[] get_keys(Guid id)
	{
		GLib.GenericArray<string> keys = new GLib.GenericArray<string>();
		GLib.GenericArray<Value?> values = get_data(id);
		for (uint32 property = 0; property < values.length; ++property) {
			if (values[property] != null)
				keys.add(property_name(id, property));
		}
		return keys.steal();
	}

	public void add_restore_point(int id, Guid?[] data, uint32 flags = 0u)
	{
		if (_debug)
			logi("add_restore_point %d, undo size = %u".printf(id, _undo_redo != null ? _undo_redo._undo.size() : 0));

		if (_undo_redo != null) {
			_undo_redo._undo.write_restore_point(id, flags, data);
			_undo_redo._redo.clear();
		}

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

	/// Duplicates the objects specified by @a ids and assigns @a new_ids to the duplicated objects.
	public void duplicate(Guid?[] ids, Guid?[] new_ids, Database? dest = null)
	{
		assert(ids.length == new_ids.length);

		if (dest == null)
			dest = this;

		GLib.HashTable<Guid?, Guid?> duplicates = new GLib.HashTable<Guid?, Guid?>(Guid.hash_func, Guid.equal_func);
		GLib.GenericArray<Guid?> objects = new GLib.GenericArray<Guid?>();
		for (int i = 0; i < ids.length; ++i) {
			assert(ids[i] != GUID_ZERO);
			assert(new_ids[i] != GUID_ZERO);
			assert(ids[i] != new_ids[i]);
			assert(has_object(ids[i]));
			assert(!duplicates.contains(ids[i]));

			duplicates[ids[i]] = new_ids[i];
			objects.add(ids[i]);
			StringId64 type = object_type(ids[i]);
			dest.create_empty(new_ids[i], type);
		}

		for (uint i = 0; i < objects.length; ++i) {
			GLib.GenericArray<Value?> values = get_data(objects[i]);
			foreach (unowned Value? value in values) {
				if (value == null || !value.holds(typeof(GLib.GenericSet)))
					continue;

				GLib.GenericSet<Guid?> hs = (GLib.GenericSet<Guid?>)value;
				foreach (Guid? j in hs) {
					if (!is_alive(j) || duplicates.contains(j))
						continue;

					Guid x = Guid.new_guid();
					duplicates[j] = x;
					objects.add(j);
					StringId64 type = object_type(j);
					dest.create_empty(x, type);
				}
			}
		}

		for (uint i = 0; i < objects.length; ++i) {
			Guid source_id = objects[i];
			Guid duplicate_id = duplicates[source_id];
			GLib.GenericArray<Value?> values = get_data(source_id);
			for (uint32 property = PROPERTY_OWNER; property < values.length; ++property) {
				Value? value = values[property];
				if (value == null)
					continue;
				string key = property_name(source_id, property);
				if (value.holds(typeof(GLib.GenericSet))) {
					dest.create_empty_set(duplicate_id, key);
					GLib.GenericSet<Guid?> hs = (GLib.GenericSet<Guid?>)value;
					foreach (Guid? j in hs) {
						if (!is_alive(j))
							continue;

						dest.add_to_set(duplicate_id, key, duplicates[j]);
					}
				} else {
					if (value.holds(typeof(Guid))) {
						Guid reference = (Guid)value;
						if (duplicates.contains(reference))
							reference = duplicates[reference];
						dest.set_reference(duplicate_id, key, reference);
					} else {
						dest.set_property(duplicate_id, key, value);
					}
				}
			}
		}
	}

	/// Duplicates one object only. Use Database.duplicate() to duplicate multiple objects together.
	public void duplicate_one(Guid id, Guid new_id, Database? dest = null)
	{
		duplicate({ id }, { new_id }, dest);
	}

	public void duplicate_and_add_to_set(Guid?[] ids, Guid?[] new_ids)
	{
		duplicate(ids, new_ids);

		for (int i = 0; i < ids.length; ++i) {
			Guid id = ids[i];
			Guid new_id = new_ids[i];

			Guid owner_id = owner(id);
			if (owner_id == GUID_ZERO)
				continue;

			// Already attached to its duplicated owner by duplicate().
			if (owner(new_id) != owner_id)
				continue;

			unowned PropertyDefinition[]? properties = object_definition(object_type(owner_id));
			bool added = false;

			foreach (PropertyDefinition def in properties) {
				if (def.type != PropertyType.OBJECTS_SET)
					continue;

				Guid?[] objects = get_set(owner_id, property_index(owner_id, StringId64(def.name)));
				foreach (unowned Guid? object_id in objects) {
					if (Guid.equal_func(object_id, id)) {
						add_to_set(owner_id, def.name, new_id);
						added = true;
						break;
					}
				}

				if (added)
					break;
			}
		}
	}

	/// Copies the database to db under the specified new_key.
	public void copy_to(Database db, string new_key)
	{
		assert(db != null);
		assert(is_valid_key(GUID_ZERO, new_key));

		copy_deep(db, GUID_ZERO, new_key);
	}

	public void copy_deep(Database db, Guid id, string new_key)
	{
		if (!db.has_object(id)) {
			StringId64 type = object_type(id);
			db.create_empty(id, type);
		}

		GLib.GenericArray<Value?> ob = get_data(id);
		for (uint32 property = 0; property < ob.length; ++property) {
			Value? value = ob[property];
			if (value == null)
				continue;
			string key = property_name(id, property);
			if (property == PROPERTY_TYPE) {
				db.set_type(id, object_type(id));
				continue;
			}
			if (value.holds(typeof(GLib.GenericSet))) {
				string set_key = new_key + (new_key == "" ? "" : ".") + key;
				db.create_empty_set(id, set_key);
				GLib.GenericSet<Guid?> hs = (GLib.GenericSet<Guid?>)value;
				foreach (Guid? j in hs) {
					StringId64 type = object_type(j);
					db.create_empty(j, type);
					copy_deep(db, j, "");
					db.add_to_set(id, set_key, j);
				}
			} else {
				string kk = new_key + (new_key == "" ? "" : ".") + key;

				if (value.holds(typeof(bool)))
					db.set_bool(id, kk, (bool)value);
				if (value.holds(typeof(double)))
					db.set_double(id, kk, (double)value);
				if (value.holds(typeof(string)))
					db.set_string(id, kk, (string)value);
				if (value.holds(typeof(Vector3)))
					db.set_vector3(id, kk, (Vector3)value);
				if (value.holds(typeof(Quaternion)))
					db.set_quaternion(id, kk, (Quaternion)value);
				if (value.holds(typeof(Resource)))
					db.set_resource(id, kk, ((Resource)value).name);
				if (value.holds(typeof(Guid)))
					db.set_reference(id, kk, (Guid)value);
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

		uint32 flags = rp.header.flags & ~((uint32)ActionTypeFlags.FROM_SERVER);

		switch (rp.header.id) {
		case ActionType.CREATE_OBJECTS:
			objects_destroyed(rp.data, flags);
			break;
		case ActionType.DESTROY_OBJECTS:
			objects_created(rp.data, flags);
			break;
		case ActionType.CHANGE_OBJECTS:
			objects_changed(rp.data, flags);
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

		uint32 flags = rp.header.flags & ~((uint32)ActionTypeFlags.FROM_SERVER);

		switch (rp.header.id) {
		case ActionType.CREATE_OBJECTS:
			objects_created(rp.data, flags);
			break;
		case ActionType.DESTROY_OBJECTS:
			objects_destroyed(rp.data, flags);
			break;
		case ActionType.CHANGE_OBJECTS:
			objects_changed(rp.data, flags);
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
				StringId64 obj_type = undo.read_string_id64();
				set_alive(id, true);
				_undo_redo._distance_from_last_sync += dir;
				redo.write_destroy_action(Action.DESTROY, id, obj_type);
			} else if (action == Action.DESTROY) {
				Guid id = undo.read_guid();
				StringId64 obj_type = undo.read_string_id64();
				set_alive(id, false);
				_undo_redo._distance_from_last_sync += dir;
				redo.write_create_action(Action.CREATE, id, obj_type);
			} else if (action == Action.SET_NULL) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				uint32 property = property_index(id, StringId64(key));
				unowned Value? value = property < _data[id].length ? get_local(id, property) : null;

				if (value != null) {
					if (key == "_type")
						redo.write_set_type_action(Action.SET_TYPE, id, object_type(id));
					if (value.holds(typeof(bool)))
						redo.write_set_bool_action(Action.SET_BOOL, id, key, (bool)value);
					if (value.holds(typeof(double)))
						redo.write_set_double_action(Action.SET_DOUBLE, id, key, (double)value);
					if (value.holds(typeof(string)))
						redo.write_set_string_action(Action.SET_STRING, id, key, (string)value);
					if (value.holds(typeof(Vector3)))
						redo.write_set_vector3_action(Action.SET_VECTOR3, id, key, (Vector3)value);
					if (value.holds(typeof(Quaternion)))
						redo.write_set_quaternion_action(Action.SET_QUATERNION, id, key, (Quaternion)value);
					if (value.holds(typeof(Resource)))
						redo.write_set_resource_action(Action.SET_RESOURCE, id, key, (Resource)value);
					if (value.holds(typeof(Guid)))
						redo.write_set_reference_action(Action.SET_REFERENCE, id, key, (Guid)value);
				} else {
					redo.write_set_null_action(Action.SET_NULL, id, key);
				}
				set(dir, id, key, null);
			} else if (action == Action.SET_BOOL) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				bool val = undo.read_bool();

				if (has_local_property(id, key))
					redo.write_set_bool_action(Action.SET_BOOL, id, key, get_bool(id, property_index(id, StringId64(key))));
				else
					redo.write_set_null_action(Action.SET_NULL, id, key);
				set(dir, id, key, val);
			} else if (action == Action.SET_DOUBLE) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				double val = undo.read_double();

				if (has_local_property(id, key))
					redo.write_set_double_action(Action.SET_DOUBLE, id, key, get_double(id, property_index(id, StringId64(key))));
				else
					redo.write_set_null_action(Action.SET_NULL, id, key);
				set(dir, id, key, val);
			} else if (action == Action.SET_STRING) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				string val = undo.read_string();

				if (has_local_property(id, key))
					redo.write_set_string_action(Action.SET_STRING, id, key, get_string(id, property_index(id, StringId64(key))));
				else
					redo.write_set_null_action(Action.SET_NULL, id, key);
				set(dir, id, key, val);
			} else if (action == Action.SET_TYPE) {
				Guid id = undo.read_guid();
				StringId64 type = undo.read_string_id64();
				if (has_local_property(id, "_type"))
					redo.write_set_type_action(Action.SET_TYPE, id, object_type(id));
				else
					redo.write_set_null_action(Action.SET_NULL, id, "_type");
				set_type(id, type);
				_undo_redo._distance_from_last_sync += dir;
			} else if (action == Action.SET_VECTOR3) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				Vector3 val = undo.read_vector3();

				if (has_local_property(id, key))
					redo.write_set_vector3_action(Action.SET_VECTOR3, id, key, get_vector3(id, property_index(id, StringId64(key))));
				else
					redo.write_set_null_action(Action.SET_NULL, id, key);
				set(dir, id, key, val);
			} else if (action == Action.SET_QUATERNION) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				Quaternion val = undo.read_quaternion();

				if (has_local_property(id, key))
					redo.write_set_quaternion_action(Action.SET_QUATERNION, id, key, get_quaternion(id, property_index(id, StringId64(key))));
				else
					redo.write_set_null_action(Action.SET_NULL, id, key);
				set(dir, id, key, val);
			} else if (action == Action.SET_RESOURCE) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				Resource val = undo.read_resource();

				if (has_local_property(id, key))
					redo.write_set_resource_action(Action.SET_RESOURCE, id, key, { get_resource(id, property_index(id, StringId64(key))) });
				else
					redo.write_set_null_action(Action.SET_NULL, id, key);
				set(dir, id, key, val);
			} else if (action == Action.SET_REFERENCE) {
				Guid id = undo.read_guid();
				string key = undo.read_string();
				Guid val = undo.read_guid();

				if (has_local_property(id, key))
					redo.write_set_reference_action(Action.SET_REFERENCE, id, key, get_reference(id, property_index(id, StringId64(key))));
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

	private PropertyDefinition[] validate_properties(PropertyDefinition[] properties)
	{
		PropertyDefinition[] validated = {};
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

			validated += def;
		}
		return validated;
	}

	// Creates a new object @a type with the specified @a properties and returns its ID.
	public StringId64 create_object_type(string type
		, PropertyDefinition[] properties
		, double ui_order = 0.0
		, string? ui_category = null
		, ObjectTypeFlags flags = ObjectTypeFlags.NONE
		, string? user_data = null
		)
	{
		StringId64 type_hash = StringId64(type);
		assert(!_object_definitions.contains(type_hash));

		PropertyDefinition[] validated = validate_properties(properties);
		ObjectTypeInfo new_info = {};
		_object_definitions[type_hash] = new_info;
		unowned ObjectTypeInfo? info = _object_definitions[type_hash];
		info.property_names = new string[validated.length];
		info.property_name_ids = new StringId64[validated.length];
		info.num_declared = validated.length;
		for (int i = 0; i < validated.length; ++i) {
			validated[i].declaration_order = i;
			info.property_names[i] = validated[i].name;
			info.property_name_ids[i] = StringId64(validated[i].name);
		}
		info.property_definitions = (owned)validated;
		info.name = type;
		info.ui_name = camel_case(type);
		info.ui_order = ui_order;
		info.ui_category = ui_category;
		info.flags = flags;
		info.user_data = user_data;
		info.aspects = new GLib.HashTable<StringId64?, AspectData?>(StringId64.hash_func, StringId64.equal_func);
		object_type_added(type_hash, info);
		return type_hash;
	}

	// Returns the declared properties of @a type. The view is invalidated when a new key is registered for this type.
	public unowned PropertyDefinition[]? object_definition(StringId64 type)
	{
		unowned ObjectTypeInfo? info = _object_definitions[type];
		if (info == null)
			return null;

		return info.property_definitions[0 : info.num_declared];
	}

	// Returns the name of the object @id. If the object has no name set, it returns
	// OBJECT_NAME_UNNAMED.
	public string name(Guid id)
	{
		string name = get_string(id, property_index(id, STRING_ID_64("editor.name", 0xf03c7393491c46db)), OBJECT_NAME_UNNAMED);

		if (name == OBJECT_NAME_UNNAMED)
			return get_string(id, property_index(id, STRING_ID_64("name", 0xd4c943cba60c270b)), OBJECT_NAME_UNNAMED);

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
		return _object_definitions.contains(type);
	}

	public unowned string type_name(StringId64 type)
	{
		return _object_definitions[type].name;
	}

	public uint type_flags(StringId64 type)
	{
		return _object_definitions[type].flags;
	}

	public unowned ObjectTypeInfo? type_info(StringId64 type)
	{
		return _object_definitions[type];
	}

	public Guid?[] all_objects_of_type(StringId64 type)
	{
		GLib.GenericArray<Guid?> all = new GLib.GenericArray<Guid?>();
		GLib.HashTableIter<Guid?, GLib.GenericArray<Value?>> iter = GLib.HashTableIter<Guid?, GLib.GenericArray<Value?>>(_data);
		unowned Guid? id;
		unowned GLib.GenericArray<Value?> data;

		while (iter.next(out id, out data)) {
			if (id != GUID_ZERO
				&& (type == OBJECT_TYPE_ANY || object_type(id) == type)
				&& is_alive(id)) {
				all.add(id);
			}
		}

		return all.steal();
	}

	public bool is_subobject_of(Guid subobject_id, Guid object_id, string set_name)
	{
		assert(has_object(subobject_id));
		assert(has_object(object_id));

		if (!has_property(object_id, set_name))
			return false;

		foreach (unowned Guid? object in get_set(object_id, property_index(object_id, StringId64(set_name)))) {
			if (Guid.equal_func(object, subobject_id))
				return true;
		}

		return false;
	}

	public bool find_property(ref uint32 property_index, StringId64 object_type, PropertyType type, string name)
	{
		if (!has_type(object_type))
			return false;

		unowned ObjectTypeInfo? info = _object_definitions[object_type];
		StringId64 name_id = StringId64(name);
		for (uint32 i = 0; i < info.property_name_ids.length; ++i) {
			if (info.property_name_ids[i] != name_id)
				continue;
			PropertyDefinition def = info.property_definitions[i];
			if (def.declaration_order >= 0 && def.type == type) {
				property_index = PROPERTY_FIRST + i;
				return true;
			}
		}

		return false;
	}

	public void set_aspect(StringId64 object_type, StringId64 aspect, Aspect callback)
	{
		unowned ObjectTypeInfo? info = type_info(object_type);

		AspectData data = AspectData();
		data.callback = callback;

		info.aspects[aspect] = data;
		assert(info.aspects.contains(aspect));
		assert(get_aspect(object_type, aspect) == callback);
	}

	public unowned Aspect? get_aspect(StringId64 object_type, StringId64 aspect)
	{
		unowned ObjectTypeInfo? info = type_info(object_type);

		if (info.aspects.contains(aspect))
			return info.aspects[aspect].callback;

		return null;
	}
}

public void default_name_aspect(out string name, Database database, Guid id)
{
	name = database.name(id);

	StringId64 object_type = database.object_type(id);

	uint32 name_index = 0;
	if (database.find_property(ref name_index, object_type, PropertyType.STRING, "name"))
		name = database.get_string(id, name_index);
	else if (database.find_property(ref name_index, object_type, PropertyType.STRING, "editor.name"))
		name = database.get_string(id, name_index);
	else
		name = "(%s)".printf(database.type_info(object_type).ui_name);
}

} /* namespace Crown */
