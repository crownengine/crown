/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public struct MaterialResource
{
	public Database _db;
	public Guid _id;

	public MaterialResource(Database db, Guid id, string shader)
	{
		_db = db;
		_id = id;

		_db.create(_id, STRING_ID_64(OBJECT_TYPE_MATERIAL, 0xeac0b497876adedf));
		_db.set_string(_id, _db.property_index(_id, STRING_ID_64("shader", 0xcce8d5b5f5ae333f)), shader);
	}

	public MaterialResource.mesh(Database db
		, Guid material_id
		, string? albedo_map = null
		, string? normal_map = null
		, string? metallic_map = null
		, string? roughness_map = null
		, string? ao_map = null
		, string? emission_map = null
		, Vector3 albedo = VECTOR3_ONE
		, double metallic = 0.0
		, double roughness = 1.0
		, Vector3 emission_color = VECTOR3_ZERO
		, double emission_intensity = 1.0
		, string? shader = null
		, bool masking = false
		)
	{
		string mesh_shader = shader != null ? shader : "mesh";
		if (masking)
			mesh_shader += "+MASKED";

		this(db, material_id, mesh_shader);

		set_vector3("u_albedo", albedo);
		set_float("u_metallic", metallic);
		set_float("u_roughness", roughness);
		set_vector3("u_emission_color", emission_color);
		set_float("u_emission_intensity", emission_intensity);

		if (albedo_map != null) set_texture("u_albedo_map", albedo_map);
		if (normal_map != null) set_texture("u_normal_map", normal_map);
		if (metallic_map != null) set_texture("u_metallic_map", metallic_map);
		if (roughness_map != null) set_texture("u_roughness_map", roughness_map);
		if (ao_map != null) set_texture("u_ao_map", ao_map);
		if (emission_map != null) set_texture("u_emission_map", emission_map);

		set_float("u_use_albedo_map", (double)(albedo_map != null));
		set_float("u_use_normal_map", (double)(normal_map != null));
		set_float("u_use_metallic_map", (double)(metallic_map != null));
		set_float("u_use_roughness_map", (double)(roughness_map != null));
		set_float("u_use_ao_map", (double)(ao_map != null));
		set_float("u_use_emission_map", (double)(emission_map != null));
		set_float("u_use_opacity_map", (double)masking);
	}

	public MaterialResource.sprite(Database db
		, Guid material_id
		, string albedo_map
		, Quaternion color = Quaternion(1.0, 1.0, 1.0, 1.0)
		)
	{
		this(db, material_id, "sprite");

		set_texture("u_albedo_map", albedo_map);
		set_vector4("u_color", color);
	}

	public MaterialResource.gui(Database db, Guid material_id, string? albedo_map)
	{
		// FIXME: add defines list to ctor?
		string shader = "gui";
		if (albedo_map != null)
			shader += "+DIFFUSE_MAP";

		this(db, material_id, shader);

		if (albedo_map != null)
			set_texture("u_albedo_map", albedo_map);
	}

	private void set_uniform_vector4(string uniform_name, double x, double y, double z, double w)
	{
		Guid uniform_id = GUID_ZERO;
		Guid?[] uniforms = _db.get_set(_id, _db.property_index(_id, STRING_ID_64("uniforms", 0x3ba38dc4d483d37e)));
		foreach (unowned Guid? id in uniforms) {
			if (_db.get_string(id, _db.property_index(id, STRING_ID_64("name", 0xd4c943cba60c270b))) == uniform_name) {
				uniform_id = id;
				break;
			}
		}

		if (uniform_id == GUID_ZERO) {
			uniform_id = Guid.new_guid();
			_db.create(uniform_id, STRING_ID_64(OBJECT_TYPE_UNIFORM_VECTOR4, 0x0727ab62adb92bf0));
			_db.set_string(uniform_id, _db.property_index(uniform_id, STRING_ID_64("name", 0xd4c943cba60c270b)), uniform_name);
			_db.add_to_set(_id, _db.property_index(_id, STRING_ID_64("uniforms", 0x3ba38dc4d483d37e)), uniform_id);
		}

		_db.set_double(uniform_id, _db.property_index(uniform_id, STRING_ID_64("value.x", 0xfa204d422e6d6299)), x);
		_db.set_double(uniform_id, _db.property_index(uniform_id, STRING_ID_64("value.y", 0xe5776873c594fbf5)), y);
		_db.set_double(uniform_id, _db.property_index(uniform_id, STRING_ID_64("value.z", 0x150e7a91a060d411)), z);
		_db.set_double(uniform_id, _db.property_index(uniform_id, STRING_ID_64("value.w", 0x7e696a767c2af865)), w);
	}

	public void set_float(string uniform_name, double value)
	{
		set_uniform_vector4(uniform_name, value, 0.0, 0.0, 0.0);
	}

	public void set_vector2(string uniform_name, Vector2 value)
	{
		set_uniform_vector4(uniform_name, value.x, value.y, 0.0, 0.0);
	}

	public void set_vector3(string uniform_name, Vector3 value)
	{
		set_uniform_vector4(uniform_name, value.x, value.y, value.z, 0.0);
	}

	public void set_vector4(string uniform_name, Quaternion value)
	{
		set_uniform_vector4(uniform_name, value.x, value.y, value.z, value.w);
	}

	public void set_texture(string sampler_name, string texture_name)
	{
		Guid texture_id = GUID_ZERO;
		Guid?[] textures = _db.get_set(_id, _db.property_index(_id, STRING_ID_64("textures", 0x64e318ce6361e6ac)));
		foreach (unowned Guid? id in textures) {
			if (_db.get_string(id, _db.property_index(id, STRING_ID_64("name", 0xd4c943cba60c270b))) == sampler_name) {
				texture_id = id;
				break;
			}
		}

		if (texture_id == GUID_ZERO) {
			texture_id = Guid.new_guid();
			_db.create(texture_id, STRING_ID_64(OBJECT_TYPE_TEXTURE_SAMPLER, 0x703635aedd770877));
			_db.set_string(texture_id, _db.property_index(texture_id, STRING_ID_64("name", 0xd4c943cba60c270b)), sampler_name);
			_db.add_to_set(_id, _db.property_index(_id, STRING_ID_64("textures", 0x64e318ce6361e6ac)), texture_id);
		}

		_db.set_resource(texture_id, _db.property_index(texture_id, STRING_ID_64("texture", 0xcd4238c6a0c69e32)), texture_name);
	}

	public int save(Project project, string resource_name)
	{
		return _db.save(project.absolute_path(resource_name) + "." + OBJECT_TYPE_MATERIAL, _id);
	}
}

public class ShaderVariantMetadata
{
	public string _shader;
	public string _library;
	public PropertyDefinition[] _properties = {};

	public static string shader_name(string variant)
	{
		int separator = variant.index_of_char('+');
		return separator == -1 ? variant : variant.substring(0, separator);
	}

	private static int compare_defines(string a, string b)
	{
		return GLib.strcmp(a, b);
	}

	public static string variant_key(string variant)
	{
		string[] parts = variant.split("+");
		if (parts.length <= 1)
			return variant;
		string[] defines = parts[1 : parts.length];
		GLib.qsort_with_data<string>(defines, sizeof(string), compare_defines);
		return parts[0] + "+" + string.joinv("+", defines);
	}
}

public class ShaderMetadataLibrary
{
	public string _name;
	public GLib.HashTable<string, GLib.GenericArray<PropertyDefinition?>> _shaders = new GLib.HashTable<string, GLib.GenericArray<PropertyDefinition?>>(GLib.str_hash, GLib.str_equal);
	public GLib.HashTable<string, ShaderVariantMetadata> _variants = new GLib.HashTable<string, ShaderVariantMetadata>(GLib.str_hash, GLib.str_equal);
}

public class ShaderMetadataCatalog
{
	public GLib.HashTable<string, ShaderVariantMetadata> _materials = new GLib.HashTable<string, ShaderVariantMetadata>(GLib.str_hash, GLib.str_equal);
	public GLib.HashTable<string, ShaderMetadataLibrary> _libraries = new GLib.HashTable<string, ShaderMetadataLibrary>(GLib.str_hash, GLib.str_equal);
	public GLib.HashTable<string, string> _shader_libraries = new GLib.HashTable<string, string>(GLib.str_hash, GLib.str_equal);

	public static ShaderMetadataLibrary? load_file(Project project, string resource) throws JsonSyntaxError
	{
		string path = GLib.Path.build_filename(project.data_dir(), "data", ResourceId.id(resource).to_string()) + ".meta";
		if (!GLib.FileUtils.test(path, GLib.FileTest.EXISTS))
			return null;
		return parse(JSON.load(path));
	}

	public static ShaderMetadataLibrary parse(GLib.HashTable<string, Value?> data)
	{
		ShaderMetadataLibrary library = new ShaderMetadataLibrary();
		library._name = (string)data["library"];
		GLib.GenericArray<Value?> shaders = (GLib.GenericArray<Value?>)data["shaders"];
		foreach (Value? shader in shaders) {
			GLib.HashTable<string, Value?> declaration = (GLib.HashTable<string, Value?>)shader;
			string name = (string)declaration["name"];
			GLib.GenericArray<PropertyDefinition?> definitions = new GLib.GenericArray<PropertyDefinition?>();
			GLib.GenericArray<Value?> defines = (GLib.GenericArray<Value?>)declaration["defines"];
			foreach (Value? define in defines) {
				GLib.HashTable<string, Value?> fields = (GLib.HashTable<string, Value?>)define;
				string define_name = (string)fields["name"];
				definitions.add(PropertyDefinition()
					{
						type = PropertyType.BOOL,
						name = define_name,
						label = (string)fields["label"],
						tooltip = (string?)fields["tooltip"],
						deffault = false,
					});
			}
			library._shaders[name] = definitions;
		}
		GLib.GenericArray<Value?> variants = (GLib.GenericArray<Value?>)data["variants"];
		foreach (Value? variant in variants) {
			GLib.HashTable<string, Value?> record = (GLib.HashTable<string, Value?>)variant;
			ShaderVariantMetadata variant_metadata = new ShaderVariantMetadata();
			variant_metadata._shader = (string)record["shader"];
			variant_metadata._library = library._name;
			PropertyDefinition[] properties = {};
			GLib.GenericArray<Value?> defines = (GLib.GenericArray<Value?>)record["defines"];
			foreach (Value? define in defines)
				variant_metadata._shader += "+" + (string)define;
			variant_metadata._shader = ShaderVariantMetadata.variant_key(variant_metadata._shader);
			GLib.GenericArray<Value?> uniforms = (GLib.GenericArray<Value?>)record["uniforms"];
			foreach (Value? uniform in uniforms) {
				GLib.HashTable<string, Value?> fields = (GLib.HashTable<string, Value?>)uniform;
				string name = (string)fields["name"];
				PropertyType type;
				int num_properties = 1;
				switch ((string)fields["type"]) {
				case "float":
					type = PropertyType.DOUBLE;
					break;
				case "vector2":
					type = PropertyType.DOUBLE;
					num_properties = 2;
					break;
				case "vector3":
					type = PropertyType.VECTOR3;
					break;
				case "vector4":
					type = PropertyType.QUATERNION;
					break;
					default:
					assert_not_reached();
				}
				PropertyEditorType editor = type == PropertyType.QUATERNION ? PropertyEditorType.VECTOR : PropertyEditorType.DEFAULT;
				switch ((string?)fields["editor"]) {
				case "bool":
					type = PropertyType.BOOL;
					break;
				case "color":
					editor = PropertyEditorType.COLOR;
					break;
				}
				Vector4 value = Vector4.from_array((GLib.GenericArray<Value?>)fields["default"]);
				Vector4 min = Vector4.from_array((GLib.GenericArray<Value?>)fields["min"]);
				Vector4 max = Vector4.from_array((GLib.GenericArray<Value?>)fields["max"]);
				string label = (string)fields["label"];
				PropertyDefinition def = PropertyDefinition()
				{
					type = type,
					name = MaterialDocument.uniform_key(name),
					label = label,
					tooltip = (string?)fields["tooltip"],
					editor = editor,
				};
				for (int i = 0; i < num_properties; ++i) {
					if (num_properties == 2) {
						def.name = MaterialDocument.uniform_key(name) + (i == 0 ? ".x" : ".y");
						def.label = label + (i == 0 ? " X" : " Y");
					}
					def.deffault = MaterialDocument.property_value(def, value);
					if (type != PropertyType.BOOL) {
						def.min = MaterialDocument.property_value(def, min);
						def.max = MaterialDocument.property_value(def, max);
					}
					properties += def;
				}
			}
			GLib.GenericArray<Value?> samplers = (GLib.GenericArray<Value?>)record["samplers"];
			foreach (Value? sampler in samplers) {
				GLib.HashTable<string, Value?> fields = (GLib.HashTable<string, Value?>)sampler;
				string name = (string)fields["name"];
				properties += PropertyDefinition()
				{
					type = PropertyType.RESOURCE,
					name = MaterialDocument.texture_key(name),
					label = (string)fields["label"],
					tooltip = (string?)fields["tooltip"],
					resource_type = OBJECT_TYPE_TEXTURE,
				};
			}
			variant_metadata._properties = properties;
			if (!library._variants.contains(variant_metadata._shader))
				library._variants[variant_metadata._shader] = variant_metadata;
		}
		return library;
	}

	public void clear()
	{
		_materials.remove_all();
		_libraries.remove_all();
		_shader_libraries.remove_all();
	}

	public void add(string resource, ShaderMetadataLibrary metadata)
	{
		if (ResourceId.type(resource) == OBJECT_TYPE_MATERIAL)
			_materials[resource] = metadata._variants.get_values().data;
		ShaderMetadataLibrary? library = _libraries[metadata._name];
		if (library == null) {
			library = new ShaderMetadataLibrary();
			library._name = metadata._name;
			_libraries[metadata._name] = library;
		}
		if (ResourceId.type(resource) == "shader") {
			foreach (string shader in metadata._shaders.get_keys()) {
				library._shaders[shader] = metadata._shaders[shader];
				if (!_shader_libraries.contains(shader))
					_shader_libraries[shader] = metadata._name;
			}
		}
		foreach (ShaderVariantMetadata variant in metadata._variants.get_values()) {
			if (!library._variants.contains(variant._shader))
				library._variants[variant._shader] = variant;
		}
	}

	public string? resolve_library(string variant)
	{
		return _shader_libraries[ShaderVariantMetadata.shader_name(variant)];
	}

	public ShaderVariantMetadata? variant(string? library_name, string shader)
	{
		ShaderMetadataLibrary? library = library_name == null ? null : _libraries[library_name];
		return library == null ? null : library._variants[ShaderVariantMetadata.variant_key(shader)];
	}

	public void load_project_file(Project project, string resource)
	{
		try {
			ShaderMetadataLibrary? metadata = load_file(project, resource);
			if (metadata != null)
				add(resource, metadata);
		} catch (JsonSyntaxError e) {
			logw("%s: %s".printf(resource, e.message));
		}
	}

	public void load(Project project)
	{
		clear();
		if (!project._data_compiled)
			return;
		foreach (string resource in project._map.get_keys()) {
			if (ResourceId.type(resource) == "shader")
				load_project_file(project, resource);
		}
		foreach (Value? resource in project._data_index.get_values()) {
			string path = (string)resource;
			if (project._map.contains(path) && ResourceId.type(path) == OBJECT_TYPE_MATERIAL)
				load_project_file(project, path);
		}
	}
}

// PropertyGrid edits complete values in the working database. Source entries
// are retained for round trips; conversion to the resource format happens on export.
public class MaterialDocument
{
	public Database _db;
	public Guid _id;
	public Guid _values_id;
	public GLib.HashTable<string, Quaternion?> _uniforms = new GLib.HashTable<string, Quaternion?>(GLib.str_hash, GLib.str_equal);
	public GLib.HashTable<string, string?> _textures = new GLib.HashTable<string, string?>(GLib.str_hash, GLib.str_equal);
	public GLib.HashTable<string, Guid?> _entry_ids = new GLib.HashTable<string, Guid?>(GLib.str_hash, GLib.str_equal);

	public MaterialDocument(Database db, Guid id)
	{
		_db = db;
		_id = id;
		foreach (Guid? entry in _db.get_set(_id, _db.property_index(_id, STRING_ID_64("uniforms", 0x3ba38dc4d483d37e)))) {
			if (_db.object_type(entry) != STRING_ID_64(OBJECT_TYPE_UNIFORM_VECTOR4, 0x0727ab62adb92bf0))
				continue;
			string name = _db.get_string(entry, _db.property_index(entry, STRING_ID_64("name", 0xd4c943cba60c270b)));
			if (!_uniforms.contains(name))
				_uniforms[name] = Quaternion(_db.get_double(entry, _db.property_index(entry, STRING_ID_64("value.x", 0xfa204d422e6d6299)))
					, _db.get_double(entry, _db.property_index(entry, STRING_ID_64("value.y", 0xe5776873c594fbf5)))
					, _db.get_double(entry, _db.property_index(entry, STRING_ID_64("value.z", 0x150e7a91a060d411)))
					, _db.get_double(entry, _db.property_index(entry, STRING_ID_64("value.w", 0x7e696a767c2af865)))
					);
		}
		foreach (Guid? entry in _db.get_set(_id, _db.property_index(_id, STRING_ID_64("textures", 0x64e318ce6361e6ac)))) {
			string name = _db.get_string(entry, _db.property_index(entry, STRING_ID_64("name", 0xd4c943cba60c270b)));
			if (!_textures.contains(name))
				_textures[name] = _db.get_resource(entry, _db.property_index(entry, STRING_ID_64("texture", 0xcd4238c6a0c69e32)));
		}
		UndoRedo? undo = _db.disable_undo();
		create_values({});
		_db.restore_undo(undo);
	}

	public void create_values(PropertyDefinition[] properties)
	{
		Guid previous = _values_id;
		_values_id = Guid.new_guid();
		StringId64 type = _db.create_object_type("material_values_" + _values_id.to_string(), properties);
		_db.create_empty(_values_id, type);
		foreach (PropertyDefinition def in properties) {
			uint32 property = _db.property_index(_values_id, StringId64(def.name));
			if (def.type == PropertyType.RESOURCE) {
				_db.set_resource(_values_id, property, _textures[entry_name(def)]);
			} else {
				Quaternion value = _uniforms[entry_name(def)];
				_db.set_property(_values_id, property, property_value(def, Vector4(value.x, value.y, value.z, value.w)));
			}
		}
		if (previous != GUID_ZERO)
			_db.destroy(previous);
	}

	public string shader()
	{
		return _db.get_string(_id, _db.property_index(_id, STRING_ID_64("shader", 0xcce8d5b5f5ae333f)), "");
	}

	public string? shader_library()
	{
		return _db.get_resource(_id, _db.property_index(_id, STRING_ID_64("shader_library", 0xe5ee32a477239a93)));
	}

	public void select_shader(string? library, string name)
	{
		if (shader_library() == library && shader() == name)
			return;
		if (shader_library() != library || ShaderVariantMetadata.shader_name(shader()) != ShaderVariantMetadata.shader_name(name)) {
			foreach (string set in new string[] { "uniforms", "textures" }) {
				uint32 property = _db.property_index(_id, StringId64(set));
				foreach (Guid? entry in _db.get_set(_id, property)) {
					_db.remove_from_set(_id, property, entry);
					_db.destroy(entry);
				}
			}
			_uniforms.remove_all();
			_textures.remove_all();
			_entry_ids.remove_all();
			create_values({});
		}
		if (shader_library() != library)
			_db.set_resource(_id, _db.property_index(_id, STRING_ID_64("shader_library", 0xe5ee32a477239a93)), library);
		_db.set_string(_id, _db.property_index(_id, STRING_ID_64("shader", 0xcce8d5b5f5ae333f)), name);
		_db.add_restore_point(ActionType.CHANGE_OBJECTS, { _id }, ActionTypeFlags.FROM_OBJECTS_SET_EDITOR);
	}

	public static string uniform_key(string name)
	{
		return "uniforms." + name + ".value";
	}

	public static string texture_key(string name)
	{
		return "textures." + name + ".value";
	}

	public static Guid find_entry(Database source, Guid material, string set, string name)
	{
		foreach (Guid? entry in source.get_set(material, source.property_index(material, StringId64(set)))) {
			if (source.get_string(entry, source.property_index(entry, STRING_ID_64("name", 0xd4c943cba60c270b))) == name)
				return entry;
		}
		return GUID_ZERO;
	}

	public static string entry_name(PropertyDefinition def)
	{
		string prefix = def.type == PropertyType.RESOURCE ? "textures." : "uniforms.";
		return def.name.substring(prefix.length, def.name.last_index_of(".value") - prefix.length);
	}

	public static Value property_value(PropertyDefinition def, Vector4 value)
	{
		if (def.type == PropertyType.QUATERNION)
			return Quaternion(value.x, value.y, value.z, value.w);
		else if (def.type == PropertyType.VECTOR3)
			return value.to_vector3();
		else if (def.type == PropertyType.BOOL)
			return value.x != 0.0;
		else
			return def.name.has_suffix(".value.y") ? value.y : value.x;
	}

	public static void apply_value(ref Quaternion value, string key, Value stored)
	{
		if (stored.holds(typeof(Quaternion))) {
			value = (Quaternion)stored;
		} else if (stored.holds(typeof(Vector3))) {
			Vector3 vector = (Vector3)stored;
			value.x = vector.x;
			value.y = vector.y;
			value.z = vector.z;
		} else if (stored.holds(typeof(bool))) {
			value.x = (bool)stored ? 1.0 : 0.0;
		} else if (key.has_suffix(".value.y")) {
			value.y = (double)stored;
		} else {
			value.x = (double)stored;
		}
	}

	public void apply_metadata(ShaderVariantMetadata? variant)
	{
		foreach (string name in _uniforms.get_keys())
			_uniforms[name] = uniform_value(name);
		foreach (PropertyDefinition def in _db.object_definition(_db.object_type(_values_id))) {
			if (def.type == PropertyType.RESOURCE)
				_textures[entry_name(def)] = texture_value(entry_name(def));
		}

		PropertyDefinition[] properties = {};
		GLib.HashTable<string, Quaternion?> defaults = new GLib.HashTable<string, Quaternion?>(GLib.str_hash, GLib.str_equal);
		if (variant != null) {
			foreach (PropertyDefinition def in variant._properties) {
				string name = entry_name(def);
				if (def.type == PropertyType.RESOURCE) {
					if (!_textures.contains(name))
						_textures[name] = null;
				} else {
					Guid entry = find_entry(_db, _id, "uniforms", name);
					if (entry != GUID_ZERO && _db.object_type(entry) != STRING_ID_64(OBJECT_TYPE_UNIFORM_VECTOR4, 0x0727ab62adb92bf0))
						continue;
					Quaternion value = defaults.contains(name) ? defaults[name] : Quaternion(0, 0, 0, 0);
					apply_value(ref value, def.name, def.deffault);
					defaults[name] = value;
				}
				properties += def;
			}
		}
		foreach (string name in defaults.get_keys()) {
			if (!_uniforms.contains(name))
				_uniforms[name] = defaults[name];
		}

		bool changed = _db.changed();
		UndoRedo? undo = _db.disable_undo();
		create_values(properties);
		if (undo != null) {
			undo.reset();
			undo._distance_from_last_sync = changed ? 1 : 0;
		}
		_db.restore_undo(undo);
	}

	public Quaternion uniform_value(string name)
	{
		Quaternion value = _uniforms.contains(name) ? _uniforms[name] : Quaternion(0, 0, 0, 0);
		string key = uniform_key(name);
		foreach (string field in new string[] { key, key + ".x", key + ".y" }) {
			Value? stored = _db.get_property(_values_id, _db.property_index(_values_id, StringId64(field)));
			if (stored != null)
				apply_value(ref value, field, stored);
		}
		return value;
	}

	public string? texture_value(string name)
	{
		uint32 property = _db.property_index(_values_id, StringId64(texture_key(name)));
		return property == uint32.MAX ? _textures[name] : _db.get_resource(_values_id, property);
	}

	public Guid ensure_entry(Database output, string set, string name, StringId64 type)
	{
		Guid entry = find_entry(output, _id, set, name);
		if (entry != GUID_ZERO)
			return entry;
		string key = set + "." + name;
		if (!_entry_ids.contains(key))
			_entry_ids[key] = Guid.new_guid();
		entry = _entry_ids[key];
		output.create(entry, type);
		output.set_string(entry, output.property_index(entry, STRING_ID_64("name", 0xd4c943cba60c270b)), name);
		output.add_to_set(_id, output.property_index(_id, StringId64(set)), entry);
		return entry;
	}

	public void export(out Database output)
	{
		output = new Database(_db._project);
		create_object_types(output);
		_db.copy_deep(output, _id, "");
		foreach (string name in _uniforms.get_keys()) {
			Quaternion value = uniform_value(name);
			Guid entry = ensure_entry(output, "uniforms", name, STRING_ID_64(OBJECT_TYPE_UNIFORM_VECTOR4, 0x0727ab62adb92bf0));
			output.set_double(entry, output.property_index(entry, STRING_ID_64("value.x", 0xfa204d422e6d6299)), value.x);
			output.set_double(entry, output.property_index(entry, STRING_ID_64("value.y", 0xe5776873c594fbf5)), value.y);
			output.set_double(entry, output.property_index(entry, STRING_ID_64("value.z", 0x150e7a91a060d411)), value.z);
			output.set_double(entry, output.property_index(entry, STRING_ID_64("value.w", 0x7e696a767c2af865)), value.w);
		}
		foreach (string name in _textures.get_keys()) {
			string? texture = texture_value(name);
			if (texture != null) {
				Guid entry = ensure_entry(output, "textures", name, STRING_ID_64(OBJECT_TYPE_TEXTURE_SAMPLER, 0x703635aedd770877));
				output.set_resource(entry, output.property_index(entry, STRING_ID_64("texture", 0xcd4238c6a0c69e32)), texture);
			} else {
				Guid entry = find_entry(output, _id, "textures", name);
				if (entry != GUID_ZERO && output.get_resource(entry, output.property_index(entry, STRING_ID_64("texture", 0xcd4238c6a0c69e32))) != null) {
					output.remove_from_set(_id, output.property_index(_id, STRING_ID_64("textures", 0x64e318ce6361e6ac)), entry);
					output.destroy(entry);
				}
			}
		}
	}

	public int save(string path)
	{
		Database output;
		export(out output);
		int result = output.save(path, _id);
		if (result == 0 && _db._undo_redo != null)
			_db._undo_redo._distance_from_last_sync = 0;
		return result;
	}
}

} /* namespace Crown */
