/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
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

public static string light_type(ufbx.LightType ufbx_type)
{
	switch (ufbx_type) {
	case ufbx.LightType.DIRECTIONAL:
		return "directional";
	case ufbx.LightType.SPOT:
		return "spot";
	case ufbx.LightType.AREA:
		return "area";
	case ufbx.LightType.VOLUME:
		return "volume";
	case ufbx.LightType.POINT:
	default:
		return "omni";
	}
}

public static string projection_type(ufbx.ProjectionMode ufbx_mode)
{
	switch (ufbx_mode) {
	case ufbx.ProjectionMode.ORTHOGRAPHIC:
		return "orthographic";
	case ufbx.ProjectionMode.PERSPECTIVE:
	default:
		return "perspective";
	}
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

public static GLib.File? texture_source_file(ufbx.Texture texture, GLib.File fbx_file)
{
	if (texture.relative_filename.data.length > 0) {
		string filename = ((string)texture.relative_filename.data).replace("\\", Path.DIR_SEPARATOR_S);
		if (Path.is_absolute(filename))
			return File.new_for_path(filename);

		GLib.File? parent = fbx_file.get_parent();
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

public enum TextureUsage
{
	NONE   = 0,
	COLOR  = 1 << 0,
	NORMAL = 1 << 1,
	DATA   = 1 << 2,
}

[Compact]
public class FBXImportOptions
{
	public InputBool import_units;
	public InputBool import_lights;
	public InputBool import_cameras;
	public InputBool import_textures;
	public InputBool create_textures_folder;
	public InputBool import_materials;
	public InputBool create_materials_folder;
	public InputBool create_colliders;
	public InputBool import_lods;
	public InputEnum tangents;

	public InputBool import_animation;
	public InputBool new_skeleton;
	public InputResource target_skeleton;
	public InputBool import_clips;
	public InputBool create_animations_folder;

	public FBXImportOptions(Database db)
	{
		import_units = new InputBool();
		import_units.value = true;
		import_units.value_changed.connect(on_import_units_changed);
		import_lights = new InputBool();
		import_lights.value = true;
		import_cameras = new InputBool();
		import_cameras.value = true;
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
			, new string[] { _("Calculate"), _("Import") }
			, new string[] { "calculate", "import" }
			);
		import_animation = new InputBool();
		import_animation.value = true;
		import_animation.value_changed.connect(on_import_animation_changed);
		new_skeleton = new InputBool();
		new_skeleton.value = true;
		new_skeleton.value_changed.connect(on_new_skeleton_changed);
		target_skeleton = new InputResource(OBJECT_TYPE_MESH_SKELETON, db);
		target_skeleton.sensitive = false;
		import_clips = new InputBool();
		import_clips.value = true;
		import_clips.value_changed.connect(on_import_animations_changed);
		create_animations_folder = new InputBool();
		create_animations_folder.value = true;
	}

	public void on_import_units_changed()
	{
		import_lights.sensitive = import_units.value;
		import_cameras.sensitive = import_units.value;
		import_textures.sensitive = import_units.value;
		create_textures_folder.sensitive = import_units.value;
		import_materials.sensitive = import_units.value;
		create_materials_folder.sensitive = import_units.value;
		create_colliders.sensitive = import_units.value;
		import_lods.sensitive = import_units.value;
	}

	public void on_import_textures_changed()
	{
		create_textures_folder.set_sensitive(import_textures.value);
	}

	public void on_import_materials_changed()
	{
		create_materials_folder.set_sensitive(import_materials.value);
	}

	public void on_import_animations_changed()
	{
		create_animations_folder.set_sensitive(import_clips.value);
	}

	public void on_import_animation_changed()
	{
		new_skeleton.sensitive = import_animation.value;
		import_clips.sensitive = import_animation.value;
		create_animations_folder.sensitive = import_clips.value;
	}

	public void on_new_skeleton_changed()
	{
		target_skeleton.sensitive = !new_skeleton.value;
	}

	public void decode(Hashtable json)
	{
		json.foreach((g) => {
				if (g.key == "import_lights")
					import_lights.value = (bool)g.value;
				else if (g.key == "import_cameras")
					import_cameras.value = (bool)g.value;
				else if (g.key == "import_textures")
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
				else if (g.key == "new_skeleton")
					new_skeleton.value = (bool)g.value;
				else if (g.key == "target_skeleton")
					target_skeleton.value = (string)g.value != "" ? (string)g.value : null;
				else if (g.key == "import_clips")
					import_clips.value = (bool)g.value;
				else if (g.key == "create_animations_folder")
					create_animations_folder.value = (bool)g.value;
				else
					logw("Unknown option '%s'".printf(g.key));

				return true;
			});

		import_units.value = import_lights.value
			|| import_cameras.value
			|| import_textures.value
			|| import_materials.value
			|| import_lods.value
			;
		import_units.value_changed(import_animation);

		import_animation.value = new_skeleton.value
			|| import_clips.value
			;
		import_animation.value_changed(import_animation);
	}

	public Hashtable encode()
	{
		bool skip_units = !import_units.value;
		bool skip_anims = !import_animation.value;

		Hashtable obj = new Hashtable();

		obj.set("import_lights", skip_units ? false : import_lights.value);
		obj.set("import_cameras", skip_units ? false : import_cameras.value);
		obj.set("import_textures", skip_units ? false : import_textures.value);
		obj.set("create_textures_folder", skip_units ? false : create_textures_folder.value);
		obj.set("import_materials", skip_units ? false : import_materials.value);
		obj.set("create_materials_folder", skip_units ? false : create_materials_folder.value);
		obj.set("create_colliders", skip_units ? false : create_colliders.value);
		obj.set("import_lods", skip_units ? false : import_lods.value);
		obj.set("tangents", tangents.value);
		obj.set("new_skeleton", skip_anims ? false : new_skeleton.value);
		obj.set("target_skeleton", (skip_anims || target_skeleton.value == null) ? "" : target_skeleton.value);
		obj.set("import_clips", skip_anims ? false : import_clips.value);
		obj.set("create_animations_folder", skip_anims ? false : create_animations_folder.value);

		return obj;
	}
}

public class FBXImportDialog : Gtk.Window
{
	public Project _project;
	public string _destination_dir;
	public Gee.ArrayList<string> _filenames;
	public unowned Import _import_result;

	public string _options_path;
	public FBXImportOptions _options;

	public PropertyGridSet _general_set;
	public Gtk.Box _box;

	public Gtk.Button _import;
	public Gtk.Button _cancel;
	public Gtk.HeaderBar _header_bar;

	public FBXImportDialog(Database database, string destination_dir, GLib.SList<string> filenames, Import import_result)
	{
		_project = database._project;
		_destination_dir = destination_dir;
		_filenames = new Gee.ArrayList<string>();
		foreach (var f in filenames)
			_filenames.add(f);
		_import_result = import_result;

		_general_set = new PropertyGridSet();

		_options = new FBXImportOptions(database);
		GLib.File file_dst;
		string resource_path;
		get_destination_file(out file_dst, destination_dir, File.new_for_path(_filenames[0]));
		get_resource_path(out resource_path, file_dst, _project);
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
		cv.add_row(_("Import Lights"), _options.import_lights, _("Import all light nodes."));
		cv.add_row(_("Import Cameras"), _options.import_cameras, _("Import all camera nodes."));
		cv.add_row(_("Import Textures"), _options.import_textures, _("Import all textures."));
		cv.add_row(_("Create Textures Folder"), _options.create_textures_folder, _("Put imported textures in a sub-folder."));
		cv.add_row(_("Import Materials"), _options.import_materials, _("Import all materials."));
		cv.add_row(_("Create Materials Folder"), _options.create_materials_folder, _("Put imported materials in a sub-folder."));
		cv.add_row(_("Create Colliders"), _options.create_colliders, _("Create colliders and actors for each imported unit."));
		cv.add_row(_("Import LODs"), _options.import_lods, _("Create LOD Group component in the root unit if any LOD exists."));
		cv.add_row(_("Tangents"), _options.tangents, _("Import tangents from source or calculate them with MikkTSpace."));
		_general_set.add_property_grid_optional(cv, _("Units"), _options.import_units, _("Import nodes as units, materials and textures."));

		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row(_("New Skeleton"), _options.new_skeleton, _("Create a new skeleton."));
		cv.add_row(_("Target Skeleton"), _options.target_skeleton, _("Skeleton to use."));
		cv.add_row(_("Import Animations"), _options.import_clips, _("Import all animation clips."));
		cv.add_row(_("Create Animations Folder"), _options.create_animations_folder, _("Put imported animations in a sub-folder."));
		_general_set.add_property_grid_optional(cv, _("Animation"), _options.import_animation, _("Import animations and skeleton."));

		_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_box.pack_start(_general_set, false, false);

		_cancel = new Gtk.Button.with_label(_("Cancel"));
		_cancel.clicked.connect(() => {
				close();
			});
		_import = new Gtk.Button.with_label(_("Import"));
		_import.get_style_context().add_class("suggested-action");
		_import.clicked.connect(import);

		_header_bar = new Gtk.HeaderBar();
		_header_bar.title = _("Import FBX...");
		_header_bar.show_close_button = true;
		_header_bar.pack_start(_cancel);
		_header_bar.pack_end(_import);

		_options.import_units.value_changed.connect(on_import_options_changed);
		_options.import_animation.value_changed.connect(on_import_options_changed);
		_options.new_skeleton.value_changed.connect(on_import_options_changed);
		_options.target_skeleton.value_changed.connect(on_import_options_changed);

		this.set_titlebar(_header_bar);
		this.add(_box);
	}

	void import()
	{
		ImportResult res = FBXImporter.do_import(_options, _project, _destination_dir, _filenames);
		if (res == ImportResult.SUCCESS) {
			try {
				SJSON.save(_options.encode(), _options_path);
			} catch (JsonWriteError e) {
				res = ImportResult.ERROR;
			}
		}

		string? primary_path = null;		// Track primary_path
		if (res == ImportResult.SUCCESS && _filenames.size > 0) {
			GLib.File file_dst;
			string resource_path;

			get_destination_file(out file_dst, _destination_dir, File.new_for_path(_filenames[0]));
			get_resource_path(out resource_path, file_dst, _project);
			string resource_name = ResourceId.name(resource_path);

			primary_path = ResourceId.path(OBJECT_TYPE_UNIT, resource_name);
		}

		_import_result(res, primary_path);
		close();
	}

	void on_import_options_changed()
	{
		bool target_skeleton_is_valid = _options.new_skeleton.value
			|| _options.target_skeleton.value != ""
			;

		bool enable_import_button = (_options.import_units.value
			|| _options.import_animation.value)
			&& target_skeleton_is_valid
			;
		_import.set_sensitive(enable_import_button);
	}
}

public class FBXImporter
{
	public static int get_or_import_texture_resource_name(out string? resource_name
		, Database db
		, Project project
		, string filename
		, GLib.File fbx_file
		, string destination_dir
		, bool create_textures_folder
		, ufbx.MaterialMap map
		, string semantic_suffix
		, TextureUsage usage
		, bool preserve_alpha
		, Gee.HashMap<string, string> imported_textures
		)
	{
		resource_name = null;
		if (!map.texture_enabled || map.texture == null)
			return 0;

		unowned ufbx.Texture? texture = map.texture;
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

		string texture_filename = Crown.texture_filename(texture);
		string texture_basename = texture_filename.length > 0
			? GLib.File.new_for_path(texture_filename).get_basename()
			: (string)texture.name.data + ".png"
			;
		string source_image_filename = Path.build_filename(textures_path, texture_basename);
		GLib.File source_image_file  = GLib.File.new_for_path(source_image_filename);
		string source_image_path     = source_image_file.get_path();

		string texture_resource_filename = project.resource_filename(source_image_path);
		string texture_resource_path     = ResourceId.normalize(texture_resource_filename);
		string texture_source_name       = ResourceId.name(texture_resource_path);
		string texture_resource_name     = texture_source_name + semantic_suffix;
		string? texture_resource_type    = ResourceId.type(texture_resource_path);
		string source_image              = texture_source_name + "." + (texture_resource_type != null ? texture_resource_type : "png");

		if (imported_textures.has_key(texture_resource_name) && !preserve_alpha) {
			resource_name = imported_textures[texture_resource_name];
			return 0;
		}

		bool source_image_exists = false;
		// Extract embedded texture data or copy external texture files into textures_path.
		if (texture.content.data.length > 0) {
			// Extract embedded PNG data.
			FileStream fs = FileStream.open(source_image_path, "wb");
			if (fs == null) {
				loge("Failed to open texture '%s'".printf(source_image_path));
				return 1;
			}

			size_t num_written = fs.write((uint8[])texture.content.data);
			if (num_written != texture.content.data.length) {
				loge("Failed to write texture '%s'".printf(source_image_path));
				return 1;
			}

			source_image_exists = true;
		} else {
			GLib.File? texture_file = Crown.texture_source_file(texture, fbx_file);
			if (texture_file != null) {
				try {
					if (!texture_file.equal(source_image_file))
						texture_file.copy(source_image_file, FileCopyFlags.OVERWRITE);

					source_image_exists = source_image_file.query_exists();
				} catch (Error e) {
					logw(e.message);
				}
			}
		}

		// Only create .texture resource if source image exists.
		if (!source_image_exists) {
			logw("'%s' references non-existing texture '%s'".printf(filename, texture_basename));
			return 0;
		}

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

		imported_textures.set(texture_resource_name, texture_resource_name);
		resource_name = texture_resource_name;
		return 0;
	}

	public static void unit_create_components(FBXImportOptions options
		, Database db
		, Guid parent_unit_id
		, Guid unit_id
		, string resource_name
		, string import_path
		, ufbx.Scene scene
		, ufbx.Node node
		, Gee.HashMap<unowned ufbx.Material, string> imported_materials
		)
	{
		Vector3 pos = vector3(node.local_transform.translation);
		Quaternion rot = quaternion(node.local_transform.rotation);
		Vector3 scl = vector3(node.local_transform.scale);
		string editor_name = node.name.data.length == 0 ? OBJECT_NAME_UNNAMED : (string)node.name.data;
		Unit unit = Unit(db, unit_id);

		if ((node.light != null && !options.import_lights.value)
			|| (node.camera != null && !options.import_cameras.value)
			) {
			if (db.has_object(unit_id) && parent_unit_id != GUID_ZERO) {
				Value? children = db.get_property(parent_unit_id, "children");
				if (children != null)
					((Gee.HashSet<Guid?>)children).remove(unit_id);
				db.destroy(unit_id);
			}
			return;
		}

		// Create mesh_renderer.
		if (node.mesh != null) {
			if (!db.has_object(unit_id))
				db.create(unit_id, OBJECT_TYPE_UNIT);

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

					unowned ufbx.Material mesh_instance_material = node.materials.data[0];
					string material_name = "core/fallback/fallback";
					if (imported_materials.has_key(mesh_instance_material))
						material_name = imported_materials[mesh_instance_material];

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
		} else if (node.light != null) {
			if (!options.import_lights.value)
				return;

			if (!db.has_object(unit_id))
				unit.create("core/units/light");
			else
				db.set_resource(unit_id, "prefab", "core/units/light");
			unit.set_local_position(pos);
			unit.set_local_rotation(rot);
			unit.set_local_scale(scl);

			Guid component_id;
			if (unit.has_component(out component_id, OBJECT_TYPE_LIGHT)) {
				unit.set_component_string (component_id, "data.type", light_type(node.light.type));
				unit.set_component_double (component_id, "data.range", 10.0);
				unit.set_component_double (component_id, "data.intensity", (double)node.light.intensity);
				unit.set_component_double (component_id, "data.spot_angle", 0.5 * MathUtils.rad((double)node.light.outer_angle));
				unit.set_component_vector3(component_id, "data.color", vector3(node.light.color));
				unit.set_component_double (component_id, "data.shadow_bias", 0.0001);
				unit.set_component_bool   (component_id, "data.cast_shadows", node.light.cast_shadows);
			}
		} else if (node.camera != null) {
			if (!options.import_cameras.value)
				return;

			if (!db.has_object(unit_id))
				unit.create("core/units/camera");
			else
				db.set_resource(unit_id, "prefab", "core/units/camera");
			unit.set_local_position(pos);
			unit.set_local_rotation(rot);
			unit.set_local_scale(scl);

			Guid component_id;
			if (unit.has_component(out component_id, OBJECT_TYPE_CAMERA)) {
				unit.set_component_string(component_id, "data.projection", projection_type(node.camera.projection_mode));
				unit.set_component_double(component_id, "data.fov", MathUtils.rad((double)node.camera.field_of_view_deg.y));
				unit.set_component_double(component_id, "data.far_range", (double)node.camera.far_plane);
				unit.set_component_double(component_id, "data.near_range", (double)node.camera.near_plane);
			}
		} else if (node.bone != null) {
			return;
		} else {
			if (!db.has_object(unit_id))
				db.create(unit_id, OBJECT_TYPE_UNIT);

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

		if (!options.create_colliders.value) {
			Guid component_id;
			if (unit.has_component(out component_id, OBJECT_TYPE_COLLIDER) && db.owner(component_id) == unit_id) {
				Value? components = db.get_property(unit_id, "components");
				if (components != null)
					((Gee.HashSet<Guid?>)components).remove(component_id);
				db.destroy(component_id);
			}

			if (unit.has_component(out component_id, OBJECT_TYPE_ACTOR) && db.owner(component_id) == unit_id) {
				Value? components = db.get_property(unit_id, "components");
				if (components != null)
					((Gee.HashSet<Guid?>)components).remove(component_id);
				db.destroy(component_id);
			}
		}

		db.set_name(unit_id, editor_name);
		// editor.import_path is importer-owned metadata, not a filesystem path.
		// It identifies the source node in the imported hierarchy so reimport
		// can reuse the same unit and preserve component GUIDs.
		db.set_string(unit_id, "editor.import_path", import_path);

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
				, scene
				, child_node
				, imported_materials
				);
		}

		if (options.import_lods.value) {
			// Use LOD groups authored in the FBX file.
			for (size_t gi = 0; gi < scene.lod_groups.data.length; ++gi) {
				unowned ufbx.LodGroup lod_group = scene.lod_groups.data[gi];
				for (size_t ii = 0; ii < lod_group.instances.data.length; ++ii) {
					if (lod_group.instances.data[ii] != node)
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

					// Add levels in source order, using FBX distances when available.
					for (size_t li = 0; li < lod_group.lod_levels.data.length && (int)li < child_unit_ids.size; ++li) {
						double screen_size = lod_group.relative_distances
							? (double)lod_group.lod_levels.data[li].distance / 100.0
							: 1.0 / (1 << (int)li)
							;

						Guid level_id = Guid.new_guid();
						db.create(level_id, OBJECT_TYPE_LOD_LEVEL);
						db.set_reference(level_id, "data.mesh_renderer", child_unit_ids[(int)li]);
						db.set_double(level_id, "data.screen_size", screen_size);
						db.add_to_set(component_id, "data.lod_levels", level_id);
					}
					return;
				}
			}

			if (scene.lod_groups.data.length > 0)
				return;

			// Fall back to sibling meshes named *_LOD0, *_LOD1, ...
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
		} else {
			Guid component_id;
			if (unit.has_component(out component_id, OBJECT_TYPE_LOD_GROUP) && db.owner(component_id) == unit_id) {
				Value? components = db.get_property(unit_id, "components");
				if (components != null)
					((Gee.HashSet<Guid?>)components).remove(component_id);
				db.destroy(component_id);
			}
		}
	}

	public static unowned ufbx.Node? find_first_non_bone_parent(ufbx.Node? bone_node)
	{
		assert(bone_node != null);

		while (bone_node.bone != null)
			bone_node = bone_node.parent;

		return bone_node;
	}

	public static unowned ufbx.Node? find_skeleton_root(ufbx.Node? node)
	{
		if (node.bone != null)
			return node;

		for (size_t i = 0; i < node.children.data.length; ++i) {
			unowned ufbx.Node? n = find_skeleton_root(node.children.data[i]);
			if (n != null)
				return find_first_non_bone_parent(n);
		}

		return null;
	}

	public static void import_skeleton(FBXImportOptions options
		, Database db
		, Guid parent_bone_id
		, Guid bone_id
		, ufbx.Node node
		)
	{
		db.create(bone_id, OBJECT_TYPE_MESH_BONE);
		db.set_string(bone_id, "name", (string)node.name.data);
		if (parent_bone_id != GUID_ZERO)
			db.add_to_set(parent_bone_id, "children", bone_id);

		for (size_t i = 0; i < node.children.data.length; ++i) {
			unowned ufbx.Node child_bone = node.children.data[i];

			if (child_bone.bone == null)
				continue; // Skip non-bone children.

			import_skeleton(options
				, db
				, bone_id
				, Guid.new_guid()
				, child_bone
				);
		}
	}

	public static bool material_uses_skinning(ufbx.Scene scene, ufbx.Material material)
	{
		for (size_t i = 0; i < scene.nodes.data.length; ++i) {
			unowned ufbx.Node node = scene.nodes.data[i];
			if (node.mesh == null || node.mesh.skin_deformers.data.length != 1 || node.materials.data.length == 0)
				continue;

			if (node.materials.data[0] == material)
				return true;
		}

		return false;
	}

	public static ImportResult do_import(FBXImportOptions options, Project project, string destination_dir, Gee.ArrayList<string> filenames)
	{
		foreach (string filename_i in filenames) {
			string resource_path;
			GLib.File file_dst;
			GLib.File file_src = File.new_for_path(filename_i);
			if (get_destination_file(out file_dst, destination_dir, file_src) != 0)
				return ImportResult.ERROR;
			if (get_resource_path(out resource_path, file_dst, project) != 0)
				return ImportResult.ERROR;
			string resource_name = ResourceId.name(resource_path);
			string resource_basename = GLib.File.new_for_path(resource_name).get_basename();

			// Copy FBX file.
			try {
				file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
			} catch (Error e) {
				loge(e.message);
				return ImportResult.ERROR;
			}

			// Keep in sync with mesh_fbx.cpp!
			ufbx.LoadOpts load_opts = {};
			load_opts.target_camera_axes =
			{
				ufbx.CoordinateAxis.POSITIVE_X,
				ufbx.CoordinateAxis.POSITIVE_Z,
				ufbx.CoordinateAxis.NEGATIVE_Y
			};
			load_opts.target_light_axes =
			{
				ufbx.CoordinateAxis.POSITIVE_X,
				ufbx.CoordinateAxis.POSITIVE_Y,
				ufbx.CoordinateAxis.POSITIVE_Z
			};
			load_opts.target_axes = ufbx.CoordinateAxes.RIGHT_HANDED_Z_UP;
			load_opts.target_unit_meters = 1.0f;
			load_opts.space_conversion = ufbx.SpaceConversion.TRANSFORM_ROOT;

			// Load FBX file.
			ufbx.Error error = {};
			ufbx.Scene? scene = ufbx.Scene.load_file(filename_i, load_opts, ref error);

			Database db = new Database(project);
			Gee.HashMap<string, string> imported_textures = new Gee.HashMap<string, string>();
			Gee.HashMap<unowned ufbx.Material, string> imported_materials = new Gee.HashMap<unowned ufbx.Material, string>();

			// Import animations.
			StateMachineResource? smr = null;

			if (options.import_animation.value) {
				string target_skeleton = options.target_skeleton.value;

				// Import animation skeleton.
				if (options.new_skeleton.value) {
					// Create .animation_skeleton resource.
					unowned ufbx.Node? skeleton_root_node = find_skeleton_root(scene.root_node);
					if (skeleton_root_node != null) {
						Guid skeleton_hierarchy_id = Guid.new_guid();
						import_skeleton(options
							, db
							, GUID_ZERO
							, skeleton_hierarchy_id
							, skeleton_root_node
							);

						Guid animation_skeleton_id = Guid.new_guid();
						db.create(animation_skeleton_id, OBJECT_TYPE_MESH_SKELETON);
						db.set_string(animation_skeleton_id, "source", resource_path);
						db.add_to_set(animation_skeleton_id, "skeleton", skeleton_hierarchy_id);
						if (db.save(project.absolute_path(resource_name) + "." + OBJECT_TYPE_MESH_SKELETON, animation_skeleton_id) != 0)
							return ImportResult.ERROR;

						target_skeleton = resource_name;

						// Create .state_machine resource to drive the skeleton.
						Guid state_machine_id = Guid.new_guid();
						smr = StateMachineResource.mesh(db
							, state_machine_id
							, target_skeleton
							);
						if (smr.save(project, resource_name) != 0)
							return ImportResult.ERROR;
					}
				}

				// Import animation clip.
				if (options.import_clips.value) {
					if (target_skeleton == "") {
						logw("Animation must have a target skeleton. Animation clips won't be imported.");
					} else {
						// Create 'animations' folder.
						string directory_name = "animations";
						string animations_path = destination_dir;
						if (options.create_animations_folder.value && scene.anim_stacks.data.length != 0) {
							GLib.File animations_file = File.new_for_path(Path.build_filename(destination_dir, directory_name));
							try {
								animations_file.make_directory();
							} catch (GLib.IOError.EXISTS e) {
								// Ignore.
							} catch (GLib.Error e) {
								loge(e.message);
								return ImportResult.ERROR;
							}

							animations_path = animations_file.get_path();
						}

						// Extract clips.
						if (scene.anim_stacks.data.length > 0) {
							unowned ufbx.AnimStack anim_stack = scene.anim_stacks.data[0];

							string anim_filename = Path.build_filename(animations_path, resource_basename + "." + OBJECT_TYPE_MESH_ANIMATION);
							GLib.File anim_file  = GLib.File.new_for_path(anim_filename);
							string anim_path     = anim_file.get_path();

							string anim_resource_filename = project.resource_filename(anim_path);
							string anim_resource_path     = ResourceId.normalize(anim_resource_filename);
							string anim_resource_name     = ResourceId.name(anim_resource_path);

							// Create .mesh_animation resource.
							Guid anim_id = Guid.new_guid();
							db.create(anim_id, OBJECT_TYPE_MESH_ANIMATION);
							db.set_string(anim_id, "source", resource_path);
							db.set_string(anim_id, "target_skeleton", target_skeleton);
							db.set_string(anim_id, "stack_name", (string)anim_stack.name.data);
							if (db.save(project.absolute_path(anim_resource_name) + "." + OBJECT_TYPE_MESH_ANIMATION, anim_id) != 0)
								return ImportResult.ERROR;
						}
					}
				}
			}

			// Import materials.
			if (options.import_units.value && options.import_materials.value) {
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
								albedo = vector3(map.value_vec3);

							unowned ufbx.MaterialMap opacity = material.pbr.opacity;
							masking = map.texture_enabled
								&& opacity.texture_enabled
								&& map.texture != null
								&& opacity.texture != null
								;
							if (masking) {
								string color_texture_filename = texture_filename(map.texture);
								string opacity_texture_filename = texture_filename(opacity.texture);
								masking = map.texture == opacity.texture
									|| (color_texture_filename.length > 0 && color_texture_filename == opacity_texture_filename)
									;
							}

							if (!masking) {
								unowned ufbx.MaterialMap transparency = material.fbx.transparency_factor;
								masking = map.texture_enabled
									&& transparency.texture_enabled
									&& map.texture != null
									&& transparency.texture != null
									;
								if (masking) {
									string color_texture_filename = texture_filename(map.texture);
									string transparency_texture_filename = texture_filename(transparency.texture);
									masking = map.texture == transparency.texture
										|| (color_texture_filename.length > 0 && color_texture_filename == transparency_texture_filename)
										;
								}
							}

							if (!masking) {
								unowned ufbx.MaterialMap transparency = material.fbx.transparency_color;
								masking = map.texture_enabled
									&& transparency.texture_enabled
									&& map.texture != null
									&& transparency.texture != null
									;
								if (masking) {
									string color_texture_filename = texture_filename(map.texture);
									string transparency_texture_filename = texture_filename(transparency.texture);
									masking = map.texture == transparency.texture
										|| (color_texture_filename.length > 0 && color_texture_filename == transparency_texture_filename)
										;
								}
							}

							if (options.import_textures.value
								&& get_or_import_texture_resource_name(out albedo_map
									, db
									, project
									, filename_i
									, file_src
									, destination_dir
									, options.create_textures_folder.value
									, map
									, "_df"
									, TextureUsage.COLOR
									, masking
									, imported_textures
									) != 0)
								return ImportResult.ERROR;
							break;
						}

						case ufbx.MaterialPbrMap.NORMAL_MAP: {
							if (options.import_textures.value
								&& get_or_import_texture_resource_name(out normal_map
									, db
									, project
									, filename_i
									, file_src
									, destination_dir
									, options.create_textures_folder.value
									, map
									, "_nr"
									, TextureUsage.NORMAL
									, false
									, imported_textures
									) != 0)
								return ImportResult.ERROR;
							break;
						}

						case ufbx.MaterialPbrMap.METALNESS: {
							if (map.has_value)
								metallic = map.value_real;

							if (options.import_textures.value
								&& get_or_import_texture_resource_name(out metallic_map
									, db
									, project
									, filename_i
									, file_src
									, destination_dir
									, options.create_textures_folder.value
									, map
									, "_mt"
									, TextureUsage.DATA
									, false
									, imported_textures
									) != 0)
								return ImportResult.ERROR;
							break;
						}

						case ufbx.MaterialPbrMap.ROUGHNESS: {
							if (map.has_value)
								roughness = map.value_real;

							if (options.import_textures.value
								&& get_or_import_texture_resource_name(out roughness_map
									, db
									, project
									, filename_i
									, file_src
									, destination_dir
									, options.create_textures_folder.value
									, map
									, "_rg"
									, TextureUsage.DATA
									, false
									, imported_textures
									) != 0)
								return ImportResult.ERROR;
							break;
						}

						case ufbx.MaterialPbrMap.AMBIENT_OCCLUSION: {
							if (options.import_textures.value
								&& get_or_import_texture_resource_name(out ao_map
									, db
									, project
									, filename_i
									, file_src
									, destination_dir
									, options.create_textures_folder.value
									, map
									, "_ao"
									, TextureUsage.DATA
									, false
									, imported_textures
									) != 0)
								return ImportResult.ERROR;
							break;
						}

						case ufbx.MaterialPbrMap.EMISSION_COLOR: {
							if (map.has_value)
								emission_color = vector3(map.value_vec3);

							if (options.import_textures.value
								&& get_or_import_texture_resource_name(out emission_map
									, db
									, project
									, filename_i
									, file_src
									, destination_dir
									, options.create_textures_folder.value
									, map
									, "_em"
									, TextureUsage.COLOR
									, false
									, imported_textures
									) != 0)
								return ImportResult.ERROR;
							break;
						}

						case ufbx.MaterialPbrMap.EMISSION_FACTOR:
							if (map.has_value)
								emission_intensity = (double)map.value_real;
							break;

						default:
							break;
						}
					}

					if (smr != null && material_uses_skinning(scene, material))
						shader += "+SKINNING";

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

			if (options.import_units.value) {
				// Generate or modify existing .unit.
				create_object_types(db);
				Guid unit_id;
				if (db.add_from_resource_path(out unit_id, resource_name + ".unit") != 0)
					unit_id = Guid.new_guid();
				unit_create_components(options
					, db
					, GUID_ZERO
					, unit_id
					, resource_name
					, "root"
					, scene
					, scene.root_node
					, imported_materials
					);

				if (options.import_animation.value && options.new_skeleton.value && smr != null) {
					// Create animation_state_machine component.
					Unit unit = Unit(db, unit_id);

					Guid component_id;
					if (!unit.has_component(out component_id, OBJECT_TYPE_ANIMATION_STATE_MACHINE)) {
						component_id = Guid.new_guid();
						db.create(component_id, OBJECT_TYPE_ANIMATION_STATE_MACHINE);
						db.add_to_set(unit_id, "components", component_id);
					}

					unit.set_component_string(component_id, "data.state_machine_resource", resource_name);
				}

				if (db.save(project.absolute_path(resource_name) + ".unit", unit_id) != 0)
					return ImportResult.ERROR;

				Guid mesh_id = Guid.new_guid();
				db.create(mesh_id, OBJECT_TYPE_MESH);
				db.set_string(mesh_id, "source", resource_path);
				if (db.save(project.absolute_path(resource_name) + ".mesh", mesh_id) != 0)
					return ImportResult.ERROR;
			}
		}

		return ImportResult.SUCCESS;
	}

	public static void import(Import import_result, Database database, string destination_dir, GLib.SList<string> filenames, Gtk.Window? parent_window)
	{
		FBXImportDialog dialog = new FBXImportDialog(database, destination_dir, filenames, import_result);
		dialog.set_transient_for(parent_window);
		dialog.set_modal(true);
		dialog.show_all();
		dialog.present();
	}
}

} /* namespace Crown */
