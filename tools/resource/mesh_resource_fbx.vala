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
				else if (g.key == "new_skeleton")
					new_skeleton.value = (bool)g.value;
				else if (g.key == "target_skeleton")
					target_skeleton.value = (string)g.value;
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
		obj.set("new_skeleton", skip_anims ? false : new_skeleton.value);
		obj.set("target_skeleton", skip_anims ? "" : target_skeleton.value);
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
		cv.add_row("Import Lights", _options.import_lights);
		cv.add_row("Import Cameras", _options.import_cameras);
		cv.add_row("Import Textures", _options.import_textures);
		cv.add_row("Create Textures Folder", _options.create_textures_folder);
		cv.add_row("Import Materials", _options.import_materials);
		cv.add_row("Create Materials Folder", _options.create_materials_folder);
		_general_set.add_property_grid_optional(cv, "Units", _options.import_units);

		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("New Skeleton", _options.new_skeleton);
		cv.add_row("Target Skeleton", _options.target_skeleton);
		cv.add_row("Import Animations", _options.import_clips);
		cv.add_row("Create Animations Folder", _options.create_animations_folder);
		_general_set.add_property_grid_optional(cv, "Animation", _options.import_animation);

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
		_header_bar.title = "Import FBX...";
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
		_import_result(res);
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
	public static void unit_create_components(FBXImportOptions options
		, Database db
		, Guid parent_unit_id
		, Guid unit_id
		, string resource_name
		, ufbx.Node node
		, Gee.HashMap<unowned ufbx.Material, string> imported_materials
		)
	{
		Vector3 pos = vector3(node.local_transform.translation);
		Quaternion rot = quaternion(node.local_transform.rotation);
		Vector3 scl = vector3(node.local_transform.scale);
		string editor_name = node.name.data.length == 0 ? OBJECT_NAME_UNNAMED : (string)node.name.data;

		// Create mesh_renderer.
		if (node.mesh != null) {
			Unit unit = Unit(db, unit_id);
			db.create(unit_id, OBJECT_TYPE_UNIT);
			db.set_name(unit_id, editor_name);

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
		} else if (node.light != null) {
			if (!options.import_lights.value)
				return;

			Unit unit = Unit(db, unit_id);
			unit.create("core/units/light");
			db.set_name(unit_id, editor_name);
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

			Unit unit = Unit(db, unit_id);
			unit.create("core/units/camera");
			db.set_name(unit_id, editor_name);
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
			Unit unit = Unit(db, unit_id);
			db.create(unit_id, OBJECT_TYPE_UNIT);
			db.set_name(unit_id, editor_name);

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
		}

		if (parent_unit_id != GUID_ZERO)
			db.add_to_set(parent_unit_id, "children", unit_id);

		for (size_t i = 0; i < node.children.data.length; ++i) {
			unit_create_components(options
				, db
				, unit_id
				, Guid.new_guid()
				, resource_name
				, node.children.data[i]
				, imported_materials
				);
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
			Gee.HashMap<unowned ufbx.Texture, string> imported_textures = new Gee.HashMap<unowned ufbx.Texture, string>();
			Gee.HashMap<unowned ufbx.Material, string> imported_materials = new Gee.HashMap<unowned ufbx.Material, string>();

			// Import textures.
			if (options.import_units.value && options.import_textures.value) {
				// Create 'textures' folder.
				string directory_name = "textures";
				string textures_path = destination_dir;
				if (options.create_textures_folder.value && scene.textures.data.length != 0) {
					GLib.File textures_file = File.new_for_path(Path.build_filename(destination_dir, directory_name));
					try {
						textures_file.make_directory();
					} catch (GLib.IOError.EXISTS e) {
						// Ignore.
					} catch (GLib.Error e) {
						loge(e.message);
						return ImportResult.ERROR;
					}

					textures_path = textures_file.get_path();
				}

				// Import textures.
				for (size_t i = 0; i < scene.textures.data.length; ++i) {
					unowned ufbx.Texture texture = scene.textures.data[i];

					string source_image_filename = Path.build_filename(textures_path, (string)texture.name.data + ".png");
					GLib.File source_image_file  = GLib.File.new_for_path(source_image_filename);
					string source_image_path     = source_image_file.get_path();

					FileStream fs;
					// Extract embedded texture data or copy external
					// texture files into textures_path.
					if (texture.content.data.length > 0) {
						// Extract embedded PNG data.
						fs = FileStream.open(source_image_path, "wb");
						if (fs == null) {
							loge("Failed to open texture '%s'".printf(source_image_path));
							return ImportResult.ERROR;
						}

						size_t num_written = fs.write((uint8[])texture.content.data);
						if (num_written != texture.content.data.length) {
							loge("Failed to write texture '%s'".printf(source_image_path));
							return ImportResult.ERROR;
						}
					} else {
						// TODO: Copy external texture file.
						fs = null;
					}

					// Only create .texture resource if source image exists.
					if (fs == null) {
						logw("'%s' references non-existing texture '%s'".printf(filename_i, (string)texture.name.data));
					} else {
						string texture_resource_filename = project.resource_filename(source_image_path);
						string texture_resource_path     = ResourceId.normalize(texture_resource_filename);
						string texture_resource_name     = ResourceId.name(texture_resource_path);

						// Create .texture resource.
						Guid texture_id = Guid.new_guid();
						// FIXME: detect texture type.
						TextureResource texture_resource = TextureResource.color_map(db
							, texture_id
							, texture_resource_name + ".png"
							);
						if (texture_resource.save(project, texture_resource_name) != 0)
							return ImportResult.ERROR;

						imported_textures.set(texture, texture_resource_name);
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

					for (int mm = 0; mm < ufbx.MaterialPbrMap.MAP_COUNT; ++mm) {
						unowned ufbx.MaterialMap map = material.pbr.maps[mm];

						switch (mm) {
						case ufbx.MaterialPbrMap.BASE_COLOR: {
							albedo = vector3(map.value_vec3);

							// Lookup matching imported texture, if any.
							if (map.texture_enabled
								&& map.texture != null
								&& imported_textures.has_key(map.texture)
								) {
								albedo_map = imported_textures[map.texture];
							}
							break;
						}

						case ufbx.MaterialPbrMap.NORMAL_MAP: {
							// Lookup matching imported texture, if any.
							if (map.texture_enabled
								&& map.texture != null
								&& imported_textures.has_key(map.texture)
								) {
								normal_map = imported_textures[map.texture];
							}
							break;
						}

						case ufbx.MaterialPbrMap.METALNESS: {
							metallic = map.value_real;

							// Lookup matching imported texture, if any.
							if (map.texture_enabled
								&& map.texture != null
								&& imported_textures.has_key(map.texture)
								) {
								metallic_map = imported_textures[map.texture];
							}
							break;
						}

						case ufbx.MaterialPbrMap.ROUGHNESS: {
							roughness = map.value_real;

							// Lookup matching imported texture, if any.
							if (map.texture_enabled
								&& map.texture != null
								&& imported_textures.has_key(map.texture)
								) {
								roughness_map = imported_textures[map.texture];
							}
							break;
						}

						case ufbx.MaterialPbrMap.AMBIENT_OCCLUSION: {
							// Lookup matching imported texture, if any.
							if (map.texture_enabled
								&& map.texture != null
								&& imported_textures.has_key(map.texture)
								) {
								ao_map = imported_textures[map.texture];
							}
							break;
						}

						case ufbx.MaterialPbrMap.EMISSION_COLOR: {
							emission_color = vector3(map.value_vec3);

							// Lookup matching imported texture, if any.
							if (map.texture_enabled
								&& map.texture != null
								&& imported_textures.has_key(map.texture)
								) {
								emission_map = imported_textures[map.texture];
							}
							break;
						}

						case ufbx.MaterialPbrMap.EMISSION_FACTOR:
							emission_intensity = (double)map.value_real;
							break;

						default:
							break;
						}
					}

					if (options.import_animation.value)
						shader += "+SKINNING";

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
						);
					if (material_resource.save(project, material_resource_name) != 0)
						return ImportResult.ERROR;

					imported_materials.set(material, material_resource_name);
				}
			}

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

			if (options.import_units.value) {
				// Generate or modify existing .unit.
				Guid unit_id = Guid.new_guid();
				unit_create_components(options
					, db
					, GUID_ZERO
					, unit_id
					, resource_name
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
