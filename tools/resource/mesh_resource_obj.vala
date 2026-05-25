/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
namespace OBJImport
{
	public static int get_destination_file(out GLib.File destination_file
		, string destination_dir
		, GLib.File source_file
		)
	{
		string path = Path.build_filename(destination_dir, source_file.get_basename());
		destination_file = File.new_for_path(path);
		return 0;
	}

	public static int get_resource_path(out string resource_path
		, GLib.File destination_file
		, Project project
		)
	{
		string resource_filename = project.resource_filename(destination_file.get_path());
		resource_path = ResourceId.normalize(resource_filename);
		return 0;
	}

	public static Vector3 vector3(ufbx.Vec3 v)
	{
		return Vector3(v.x, v.y, v.z);
	}

	public static Quaternion quaternion(ufbx.Quat q)
	{
		return Quaternion(q.x, q.y, q.z, q.w);
	}

	public static string texture_filename(ufbx.Texture texture)
	{
		if (texture.filename.data.length > 0)
			return ((string)texture.filename.data).replace("\\", Path.DIR_SEPARATOR_S);
		else if (texture.relative_filename.data.length > 0)
			return ((string)texture.relative_filename.data).replace("\\", Path.DIR_SEPARATOR_S);
		else if (texture.absolute_filename.data.length > 0)
			return ((string)texture.absolute_filename.data).replace("\\", Path.DIR_SEPARATOR_S);
		else
			return "";
	}

	public static string texture_display_name(ufbx.Texture texture)
	{
		string filename = texture_filename(texture);
		if (filename.length > 0)
			return filename;
		else if (texture.name.data.length > 0)
			return (string)texture.name.data;
		else
			return "<unnamed>";
	}

	public static GLib.File? texture_source_file(ufbx.Texture texture, GLib.File obj_file)
	{
		if (texture.relative_filename.data.length > 0) {
			string filename = ((string)texture.relative_filename.data).replace("\\", Path.DIR_SEPARATOR_S);
			if (Path.is_absolute(filename))
				return File.new_for_path(filename);

			GLib.File? parent = obj_file.get_parent();
			return parent != null
				? parent.resolve_relative_path(filename)
				: File.new_for_path(filename)
				;
		}

		if (texture.filename.data.length > 0)
			return File.new_for_path(((string)texture.filename.data).replace("\\", Path.DIR_SEPARATOR_S));

		if (texture.absolute_filename.data.length > 0)
			return File.new_for_path(((string)texture.absolute_filename.data).replace("\\", Path.DIR_SEPARATOR_S));

		return null;
	}

	public static int get_or_import_texture_resource_name(out string? resource_name
		, Database db
		, Project project
		, string filename
		, GLib.File obj_file
		, string destination_dir
		, bool create_textures_folder
		, ufbx.MaterialMap map
		, TextureUsage usage
		, bool preserve_alpha
		, Gee.HashMap<unowned ufbx.Texture, string> imported_textures
		)
	{
		resource_name = null;
		if (!map.texture_enabled || map.texture == null)
			return 0;

		unowned ufbx.Texture? texture = map.texture;
		if (imported_textures.has_key(texture) && !preserve_alpha) {
			resource_name = imported_textures[texture];
			return 0;
		}

		string textures_path = destination_dir;
		if (create_textures_folder) {
			GLib.File textures_file = File.new_for_path(Path.build_filename(destination_dir, "textures"));
			try {
				textures_file.make_directory();
			} catch (GLib.IOError.EXISTS e) {
				// Ignore.
			} catch (GLib.Error e) {
				loge(e.message);
				return 1;
			}

			textures_path = textures_file.get_path();
		}

		string texture_filename = OBJImport.texture_filename(texture);
		if (texture_filename.length == 0) {
			logw("'%s' references non-existing texture '%s'".printf(filename, OBJImport.texture_display_name(texture)));
			return 0;
		}

		string texture_basename = GLib.File.new_for_path(texture_filename).get_basename();
		string source_image_filename = Path.build_filename(textures_path, texture_basename);
		GLib.File source_image_file  = GLib.File.new_for_path(source_image_filename);
		string source_image_path     = source_image_file.get_path();

		bool source_image_exists = false;
		GLib.File? texture_file = OBJImport.texture_source_file(texture, obj_file);
		if (texture_file != null) {
			try {
				if (!texture_file.equal(source_image_file))
					texture_file.copy(source_image_file, FileCopyFlags.OVERWRITE);

				source_image_exists = source_image_file.query_exists();
			} catch (Error e) {
				logw(e.message);
			}
		}

		// Only create .texture resource if source image exists.
		if (!source_image_exists) {
			logw("'%s' references non-existing texture '%s'".printf(filename, OBJImport.texture_display_name(texture)));
			return 0;
		}

		string texture_resource_filename = project.resource_filename(source_image_path);
		string texture_resource_path     = ResourceId.normalize(texture_resource_filename);
		string texture_resource_name     = ResourceId.name(texture_resource_path);
		string? texture_resource_type    = ResourceId.type(texture_resource_path);
		string source_image              = texture_resource_name + "." + (texture_resource_type != null ? texture_resource_type : "png");

		// Create .texture resource.
		Guid texture_id = Guid.new_guid();
		TextureResource texture_resource;
		if ((usage & TextureUsage.NORMAL) != 0)
			texture_resource = TextureResource.normal_map(db, texture_id, source_image);
		else if ((usage & TextureUsage.DATA) != 0)
			texture_resource = TextureResource.data_map(db, texture_id, source_image);
		else if (preserve_alpha)
			texture_resource = TextureResource(db, texture_id, source_image, TextureFormat.BC3, true, false);
		else
			texture_resource = TextureResource.color_map(db, texture_id, source_image);
		if (texture_resource.save(project, texture_resource_name) != 0)
			return 1;

		imported_textures.set(texture, texture_resource_name);
		resource_name = texture_resource_name;
		return 0;
	}

} /* namespace OBJImport */

[Compact]
public class OBJImportOptions
{
	public InputBool import_textures;
	public InputBool create_textures_folder;
	public InputBool import_materials;
	public InputBool create_materials_folder;
	public InputBool create_colliders;
	public InputBool import_lods;
	public InputEnum tangents;

	public OBJImportOptions()
	{
		import_textures = new InputBool();
		import_textures.value = true;
		import_textures.value_changed.connect(on_import_textures_changed);
		create_textures_folder = new InputBool();
		create_textures_folder.value = true;
		import_materials = new InputBool();
		import_materials.value = true;
		import_materials.value_changed.connect(on_import_materials_changed);
		create_materials_folder = new InputBool();
		create_materials_folder.value = true;
		create_colliders = new InputBool();
		create_colliders.value = false;
		import_lods = new InputBool();
		import_lods.value = true;
		tangents = new InputEnum("calculate"
			, new string[] { "Calculate", "Import" }
			, new string[] { "calculate", "import" }
			);
	}

	public void on_import_textures_changed()
	{
		create_textures_folder.set_sensitive(import_textures.value);
	}

	public void on_import_materials_changed()
	{
		create_materials_folder.set_sensitive(import_materials.value);
	}

	public void decode(Hashtable json)
	{
		json.foreach((g) => {
				if (g.key == "import_textures")
					import_textures.value = (bool)g.value;
				else if (g.key == "create_textures_folder")
					create_textures_folder.value = (bool)g.value;
				else if (g.key == "import_materials")
					import_materials.value = (bool)g.value;
				else if (g.key == "create_materials_folder")
					create_materials_folder.value = (bool)g.value;
				else if (g.key == "create_colliders")
					create_colliders.value = (bool)g.value;
				else if (g.key == "import_lods")
					import_lods.value = (bool)g.value;
				else if (g.key == "tangents")
					tangents.value = (string)g.value;
				else
					logw("Unknown option '%s'".printf(g.key));

				return true;
			});

		import_textures.value_changed(import_textures);
		import_materials.value_changed(import_materials);
	}

	public Hashtable encode()
	{
		Hashtable obj = new Hashtable();

		obj.set("import_textures", import_textures.value);
		obj.set("create_textures_folder", import_textures.value ? create_textures_folder.value : false);
		obj.set("import_materials", import_materials.value);
		obj.set("create_materials_folder", import_materials.value ? create_materials_folder.value : false);
		obj.set("create_colliders", create_colliders.value ? create_colliders.value : false);
		obj.set("import_lods", import_lods.value);
		obj.set("tangents", tangents.value);

		return obj;
	}
}

public class OBJImportDialog : Gtk.Window
{
	public Project _project;
	public string _destination_dir;
	public Gee.ArrayList<string> _filenames;
	public unowned Import _import_result;

	public string _options_path;
	public OBJImportOptions _options;

	public PropertyGridSet _general_set;
	public Gtk.Box _box;

	public Gtk.Button _import;
	public Gtk.Button _cancel;
	public Gtk.HeaderBar _header_bar;

	public OBJImportDialog(Database database, string destination_dir, GLib.SList<string> filenames, Import import_result)
	{
		_project = database._project;
		_destination_dir = destination_dir;
		_filenames = new Gee.ArrayList<string>();
		foreach (var f in filenames)
			_filenames.add(f);
		_import_result = import_result;

		_general_set = new PropertyGridSet();

		_options = new OBJImportOptions();
		GLib.File file_dst;
		string resource_path;
		OBJImport.get_destination_file(out file_dst, destination_dir, File.new_for_path(_filenames[0]));
		OBJImport.get_resource_path(out resource_path, file_dst, _project);
		string resource_name = ResourceId.name(resource_path);
		_options_path = _project.absolute_path(resource_name) + ".importer_settings";
		try {
			_options.decode(SJSON.load_from_path(_options_path));
		} catch (JsonSyntaxError e) {
			// No-op.
		}

		PropertyGrid cv;
		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Import Textures", _options.import_textures, "Import all textures.");
		cv.add_row("Create Textures Folder", _options.create_textures_folder, "Put imported textures in a sub-folder.");
		cv.add_row("Import Materials", _options.import_materials, "Import all materials.");
		cv.add_row("Create Materials Folder", _options.create_materials_folder, "Put imported materials in a sub-folder.");
		cv.add_row("Create Colliders", _options.create_colliders, "Create colliders and actors for each imported unit.");
		cv.add_row("Import LODs", _options.import_lods, "Create LOD Group component in the root unit if any LOD exists.");
		cv.add_row("Tangents", _options.tangents, "Import tangents from source or calculate them with MikkTSpace.");
		_general_set.add_property_grid(cv, "Units");

		_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_box.pack_start(_general_set, false, false);

		_cancel = new Gtk.Button.with_label("Cancel");
		_cancel.clicked.connect(() => {
				close();
			});
		_import = new Gtk.Button.with_label("Import");
		_import.get_style_context().add_class("suggested-action");
		_import.clicked.connect(import);

		_header_bar = new Gtk.HeaderBar();
		_header_bar.title = "Import OBJ...";
		_header_bar.show_close_button = true;
		_header_bar.pack_start(_cancel);
		_header_bar.pack_end(_import);

		this.set_titlebar(_header_bar);
		this.add(_box);
	}

	void import()
	{
		ImportResult res = OBJImporter.do_import(_options, _project, _destination_dir, _filenames);
		if (res == ImportResult.SUCCESS) {
			try {
				SJSON.save(_options.encode(), _options_path);
			} catch (JsonWriteError e) {
				res = ImportResult.ERROR;
			}
		}

		string? primary_path = null; // Track primary_path.
		if (res == ImportResult.SUCCESS && _filenames.size > 0) {
			GLib.File file_dst;
			string resource_path;

			OBJImport.get_destination_file(out file_dst, _destination_dir, File.new_for_path(_filenames[0]));
			OBJImport.get_resource_path(out resource_path, file_dst, _project);
			string resource_name = ResourceId.name(resource_path);

			primary_path = ResourceId.path(OBJECT_TYPE_UNIT, resource_name);
		}

		_import_result(res, primary_path);
		close();
	}
}

public class OBJImporter
{
	public static void unit_create_components(OBJImportOptions options
		, Database db
		, Guid parent_unit_id
		, Guid unit_id
		, string resource_name
		, string import_path
		, ufbx.Node node
		, Gee.HashMap<unowned ufbx.Material, string> imported_materials
		)
	{
		Vector3 pos = OBJImport.vector3(node.local_transform.translation);
		Quaternion rot = OBJImport.quaternion(node.local_transform.rotation);
		Vector3 scl = OBJImport.vector3(node.local_transform.scale);
		string editor_name = node.name.data.length == 0 ? OBJECT_NAME_UNNAMED : (string)node.name.data;

		Unit unit = Unit(db, unit_id);
		if (!db.has_object(unit_id))
			db.create(unit_id, OBJECT_TYPE_UNIT);
		db.set_name(unit_id, editor_name);
		// editor.import_path is importer-owned metadata, not a filesystem path.
		// It identifies the source node in the imported hierarchy so reimport
		// can reuse the same unit and preserve component GUIDs.
		db.set_string(unit_id, "editor.import_path", import_path);

		if (node.mesh != null) {
			// Create transform.
			{
				Guid component_id;
				if (!unit.has_component(out component_id, OBJECT_TYPE_TRANSFORM)) {
					component_id = Guid.new_guid();
					db.create(component_id, OBJECT_TYPE_TRANSFORM);
					db.add_to_set(unit_id, "components", component_id);
				}

				unit.set_component_vector3   (component_id, "data.position", pos);
				unit.set_component_quaternion(component_id, "data.rotation", rot);
				unit.set_component_vector3   (component_id, "data.scale", scl);
				unit.set_component_string    (component_id, "data.name", editor_name);
			}

			if (node.mesh.num_faces > 0) {
				// Create mesh_renderer.
				{
					Guid component_id;
					if (!unit.has_component(out component_id, OBJECT_TYPE_MESH_RENDERER)) {
						component_id = Guid.new_guid();
						db.create(component_id, OBJECT_TYPE_MESH_RENDERER);
						db.add_to_set(unit_id, "components", component_id);
					}

					string material_name = "core/fallback/fallback";
					if (node.materials.data.length > 0) {
						unowned ufbx.Material mesh_instance_material = node.materials.data[0];
						if (imported_materials.has_key(mesh_instance_material))
							material_name = imported_materials[mesh_instance_material];
					}

					unit.set_component_string(component_id, "data.geometry_name", editor_name);
					unit.set_component_string(component_id, "data.material", material_name);
					unit.set_component_string(component_id, "data.mesh_resource", resource_name);
					unit.set_component_bool  (component_id, "data.visible", true);
				}

				if (options.create_colliders.value) {
					// Create collider.
					{
						Guid component_id;
						if (!unit.has_component(out component_id, OBJECT_TYPE_COLLIDER)) {
							component_id = Guid.new_guid();
							db.create(component_id, OBJECT_TYPE_COLLIDER);
							db.add_to_set(unit_id, "components", component_id);
						}

						unit.set_component_string(component_id, "data.shape", "mesh");
						unit.set_component_string(component_id, "data.scene", resource_name);
						unit.set_component_string(component_id, "data.name", editor_name);
					}

					// Create actor.
					{
						Guid component_id;
						if (!unit.has_component(out component_id, OBJECT_TYPE_ACTOR)) {
							component_id = Guid.new_guid();
							db.create(component_id, OBJECT_TYPE_ACTOR);
							db.add_to_set(unit_id, "components", component_id);
						}

						unit.set_component_string(component_id, "data.class", "static");
						unit.set_component_string(component_id, "data.collision_filter", "default");
						unit.set_component_double(component_id, "data.mass", 1.0);
						unit.set_component_string(component_id, "data.material", "default");
					}
				}
			}
		} else {
			// Create transform.
			Guid component_id;
			if (!unit.has_component(out component_id, OBJECT_TYPE_TRANSFORM)) {
				component_id = Guid.new_guid();
				db.create(component_id, OBJECT_TYPE_TRANSFORM);
				db.add_to_set(unit_id, "components", component_id);
			}

			unit.set_component_vector3   (component_id, "data.position", pos);
			unit.set_component_quaternion(component_id, "data.rotation", rot);
			unit.set_component_vector3   (component_id, "data.scale", scl);
			unit.set_component_string    (component_id, "data.name", editor_name);
		}

		if (parent_unit_id != GUID_ZERO)
			db.add_to_set(parent_unit_id, "children", unit_id);

		// Reuse only children that existed before this import, and assign each
		// existing child to at most one imported node.
		Gee.HashSet<Guid?> matched_children = new Gee.HashSet<Guid?>(Guid.hash_func, Guid.equal_func);
		Gee.HashSet<Guid?> old_children = db.has_property(unit_id, "children")
			? db.get_set(unit_id, "children")
			: new Gee.HashSet<Guid?>(Guid.hash_func, Guid.equal_func)
			;
		Gee.ArrayList<Guid?> child_unit_ids = new Gee.ArrayList<Guid?>();

		for (size_t i = 0; i < node.children.data.length; ++i) {
			unowned ufbx.Node child_node = node.children.data[i];
			string child_editor_name = child_node.name.data.length == 0 ? OBJECT_NAME_UNNAMED : (string)child_node.name.data;
			string child_import_path = import_path
				+ "/"
				+ ((uint)i).to_string()
				+ ":"
				+ child_editor_name
				;
			Guid child_unit_id = GUID_ZERO;

			foreach (Guid child_id in old_children) {
				if (matched_children.contains(child_id) || !db.is_alive(child_id))
					continue;
				if (db.get_string(child_id, "editor.import_path", "") == child_import_path) {
					child_unit_id = child_id;
					break;
				}
			}

			if (child_unit_id == GUID_ZERO) {
				foreach (Guid child_id in old_children) {
					if (matched_children.contains(child_id)
						|| !db.is_alive(child_id)
						|| db.name(child_id) != child_editor_name
						)
						continue;

					child_unit_id = child_id;
					break;
				}
			}

			if (child_unit_id == GUID_ZERO)
				child_unit_id = Guid.new_guid();
			matched_children.add(child_unit_id);
			child_unit_ids.add(child_unit_id);

			unit_create_components(options
				, db
				, unit_id
				, child_unit_id
				, resource_name
				, child_import_path
				, child_node
				, imported_materials
				);
		}

		if (options.import_lods.value) {
			// Build one LOD group from sibling meshes named *_LOD0, *_LOD1, ...
			for (int i = 0; i < child_unit_ids.size; ++i) {
				unowned ufbx.Node child_node = node.children.data[i];
				if (child_node.name.data.length == 0)
					continue;

				string name = (string)child_node.name.data;
				if (!name.down().has_suffix("_lod0"))
					continue;

				Guid component_id;
				if (!unit.has_component(out component_id, OBJECT_TYPE_LOD_GROUP)) {
					component_id = Guid.new_guid();
					db.create(component_id, OBJECT_TYPE_LOD_GROUP);
					db.add_to_set(unit_id, "components", component_id);
				}

				unit.set_component_string(component_id, "data.fade_mode", "none");
				unit.set_component_double(component_id, "data.level", -1.0);
				db.create_empty_set(component_id, "data.lod_levels");

				double screen_size = 1.0;
				string base_name = name.substring(0, name.length - 5);
				// Add levels by suffix until the next LOD mesh is missing.
				for (int lod_i = 0; ; ++lod_i) {
					Guid lod_unit_id = GUID_ZERO;
					string lod_name = (base_name + "_lod" + lod_i.to_string()).down();
					for (int ci = 0; ci < child_unit_ids.size; ++ci) {
						unowned ufbx.Node n = node.children.data[ci];
						if (n.name.data.length == 0)
							continue;

						string child_name = (string)n.name.data;
						if (child_name.down() != lod_name)
							continue;

						lod_unit_id = child_unit_ids[ci];
						break;
					}
					if (lod_unit_id == GUID_ZERO)
						break;

					Guid level_id = Guid.new_guid();
					db.create(level_id, OBJECT_TYPE_LOD_LEVEL);
					db.set_reference(level_id, "data.mesh_renderer", lod_unit_id);
					db.set_double(level_id, "data.screen_size", screen_size);
					db.add_to_set(component_id, "data.lod_levels", level_id);
					screen_size *= 0.5;
				}
				return;
			}
		}
	}

	public static ImportResult do_import(OBJImportOptions options, Project project, string destination_dir, Gee.ArrayList<string> filenames)
	{
		foreach (string filename_i in filenames) {
			string resource_path;
			GLib.File file_dst;
			GLib.File file_src = File.new_for_path(filename_i);
			if (OBJImport.get_destination_file(out file_dst, destination_dir, file_src) != 0)
				return ImportResult.ERROR;
			if (OBJImport.get_resource_path(out resource_path, file_dst, project) != 0)
				return ImportResult.ERROR;
			string resource_name = ResourceId.name(resource_path);

			// Copy OBJ file.
			try {
				file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
			} catch (Error e) {
				loge(e.message);
				return ImportResult.ERROR;
			}

			ufbx.LoadOpts load_opts = {};
			load_opts.file_format = ufbx.FileFormat.OBJ;
			load_opts.load_external_files = true;
			load_opts.ignore_missing_external_files = true;
			load_opts.obj_search_mtl_by_filename = true;
			load_opts.target_axes = ufbx.CoordinateAxes.RIGHT_HANDED_Z_UP;
			load_opts.target_unit_meters = 1.0f;
			load_opts.space_conversion = ufbx.SpaceConversion.TRANSFORM_ROOT;
			load_opts.obj_axes = ufbx.CoordinateAxes.RIGHT_HANDED_Z_UP;
			load_opts.obj_unit_meters = 1.0f;

			// Load OBJ file.
			ufbx.Error error = {};
			ufbx.Scene? scene = ufbx.Scene.load_file(filename_i, load_opts, ref error);
			if (scene == null) {
				loge("ufbx: %s".printf((string)error.description.data));
				return ImportResult.ERROR;
			}

			Database db = new Database(project);
			Gee.HashMap<unowned ufbx.Texture, string> imported_textures = new Gee.HashMap<unowned ufbx.Texture, string>();
			Gee.HashMap<unowned ufbx.Material, string> imported_materials = new Gee.HashMap<unowned ufbx.Material, string>();

			// Import materials.
			if (options.import_materials.value) {
				// Create 'materials' folder.
				string directory_name = "materials";
				string materials_path = destination_dir;
				if (options.create_materials_folder.value && scene.materials.data.length != 0) {
					GLib.File materials_file = File.new_for_path(Path.build_filename(destination_dir, directory_name));
					try {
						materials_file.make_directory();
					} catch (GLib.IOError.EXISTS e) {
						// Ignore.
					} catch (GLib.Error e) {
						loge(e.message);
						return ImportResult.ERROR;
					}

					materials_path = materials_file.get_path();
				}

				// Extract materials.
				for (size_t i = 0; i < scene.materials.data.length; ++i) {
					unowned ufbx.Material material = scene.materials.data[i];

					string material_filename = Path.build_filename(materials_path, (string)material.name.data + ".png");
					GLib.File material_file  = GLib.File.new_for_path(material_filename);
					string material_path     = material_file.get_path();

					string material_resource_filename = project.resource_filename(material_path);
					string material_resource_path     = ResourceId.normalize(material_resource_filename);
					string material_resource_name     = ResourceId.name(material_resource_path);

					string shader = "mesh";
					Vector3 albedo = Vector3(1, 1, 1);
					double metallic = 0.0;
					double roughness = 1.0;
					Vector3 emission_color = Vector3(0, 0, 0);
					double emission_intensity = 1.0;
					string? albedo_map = null;
					string? normal_map = null;
					string? metallic_map = null;
					string? roughness_map = null;
					string? ao_map = null;
					string? emission_map = null;
					bool masking = false;

					for (int mm = 0; mm < ufbx.MaterialPbrMap.MAP_COUNT; ++mm) {
						unowned ufbx.MaterialMap map = material.pbr.maps[mm];

						switch (mm) {
						case ufbx.MaterialPbrMap.BASE_COLOR: {
								if (map.has_value)
									albedo = OBJImport.vector3(map.value_vec3);

								unowned ufbx.MaterialMap opacity = material.pbr.opacity;
								masking = map.texture_enabled
									&& opacity.texture_enabled
									&& map.texture != null
									&& opacity.texture != null
									;
								if (masking) {
									string color_texture_filename = OBJImport.texture_filename(map.texture);
									string opacity_texture_filename = OBJImport.texture_filename(opacity.texture);
									masking = map.texture == opacity.texture
										|| (color_texture_filename.length > 0 && color_texture_filename == opacity_texture_filename)
										;
								}

								if (options.import_textures.value
									&& OBJImport.get_or_import_texture_resource_name(out albedo_map
									, db
									, project
									, filename_i
									, file_src
									, destination_dir
									, options.create_textures_folder.value
									, map
									, TextureUsage.COLOR
									, masking
									, imported_textures
									) != 0)
									return ImportResult.ERROR;
								break;
						}

						case ufbx.MaterialPbrMap.NORMAL_MAP:
							if (options.import_textures.value
								&& OBJImport.get_or_import_texture_resource_name(out normal_map
								, db
								, project
								, filename_i
								, file_src
								, destination_dir
								, options.create_textures_folder.value
								, map
								, TextureUsage.NORMAL
								, false
								, imported_textures
								) != 0)
								return ImportResult.ERROR;
							break;

						case ufbx.MaterialPbrMap.METALNESS:
							if (map.has_value)
								metallic = map.value_real;

							if (options.import_textures.value
								&& OBJImport.get_or_import_texture_resource_name(out metallic_map
								, db
								, project
								, filename_i
								, file_src
								, destination_dir
								, options.create_textures_folder.value
								, map
								, TextureUsage.DATA
								, false
								, imported_textures
								) != 0)
								return ImportResult.ERROR;
							break;

						case ufbx.MaterialPbrMap.ROUGHNESS:
							if (map.has_value)
								roughness = map.value_real;

							if (options.import_textures.value
								&& OBJImport.get_or_import_texture_resource_name(out roughness_map
								, db
								, project
								, filename_i
								, file_src
								, destination_dir
								, options.create_textures_folder.value
								, map
								, TextureUsage.DATA
								, false
								, imported_textures
								) != 0)
								return ImportResult.ERROR;
							break;

						case ufbx.MaterialPbrMap.AMBIENT_OCCLUSION:
							if (options.import_textures.value
								&& OBJImport.get_or_import_texture_resource_name(out ao_map
								, db
								, project
								, filename_i
								, file_src
								, destination_dir
								, options.create_textures_folder.value
								, map
								, TextureUsage.DATA
								, false
								, imported_textures
								) != 0)
								return ImportResult.ERROR;
							break;

						case ufbx.MaterialPbrMap.EMISSION_COLOR:
							if (map.has_value)
								emission_color = OBJImport.vector3(map.value_vec3);

							if (options.import_textures.value
								&& OBJImport.get_or_import_texture_resource_name(out emission_map
								, db
								, project
								, filename_i
								, file_src
								, destination_dir
								, options.create_textures_folder.value
								, map
								, TextureUsage.COLOR
								, false
								, imported_textures
								) != 0)
								return ImportResult.ERROR;
							break;

						case ufbx.MaterialPbrMap.EMISSION_FACTOR:
							if (map.has_value)
								emission_intensity = (double)map.value_real;
							break;

						default:
							break;
						}
					}

					masking = masking && albedo_map != null;

					// Create .material resource.
					MaterialResource material_resource = MaterialResource.mesh(db
						, Guid.new_guid()
						, albedo_map
						, normal_map
						, metallic_map
						, roughness_map
						, ao_map
						, emission_map
						, albedo
						, metallic
						, roughness
						, emission_color
						, emission_intensity
						, shader
						, masking
						);
					if (material_resource.save(project, material_resource_name) != 0)
						return ImportResult.ERROR;

					imported_materials.set(material, material_resource_name);
				}
			}

			// Generate or modify existing .unit.
			Guid unit_id;
			if (db.add_from_resource_path(out unit_id, resource_name + ".unit") != 0)
				unit_id = Guid.new_guid();
			unit_create_components(options
				, db
				, GUID_ZERO
				, unit_id
				, resource_name
				, "root"
				, scene.root_node
				, imported_materials
				);

			if (db.save(project.absolute_path(resource_name) + ".unit", unit_id) != 0)
				return ImportResult.ERROR;

			Guid mesh_id = Guid.new_guid();
			db.create(mesh_id, OBJECT_TYPE_MESH);
			db.set_string(mesh_id, "source", resource_path);
			if (db.save(project.absolute_path(resource_name) + ".mesh", mesh_id) != 0)
				return ImportResult.ERROR;
		}

		return ImportResult.SUCCESS;
	}

	public static void import(Import import_result, Database database, string destination_dir, GLib.SList<string> filenames, Gtk.Window? parent_window)
	{
		OBJImportDialog dialog = new OBJImportDialog(database, destination_dir, filenames, import_result);
		dialog.set_transient_for(parent_window);
		dialog.set_modal(true);
		dialog.show_all();
		dialog.present();
	}
}

} /* namespace Crown */
