/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public enum SceneImportFlags
{
	NONE               = 0,
	LIGHTS_AND_CAMERAS = 1 << 0,
	ANIMATIONS         = 1 << 1,
}

public delegate void SceneImporter(Import import_result
	, SceneImportOptions options
	, Project project
	, string destination_dir
	, GLib.GenericArray<string> filenames
	, string? options_path
	);

[Compact]
public class SceneImportOptions
{
	public SceneImportFlags flags;

	public bool import_units;
	public bool import_lights;
	public bool import_cameras;
	public bool import_textures;
	public bool create_textures_folder;
	public bool import_materials;
	public bool create_materials_folder;
	public bool create_colliders;
	public bool import_lods;
	public string tangents;

	public bool import_animation;
	public bool new_skeleton;
	public string target_skeleton;
	public bool import_clips;
	public bool create_animations_folder;

	public SceneImportOptions(SceneImportFlags flags)
	{
		this.flags = flags;

		import_units = true;
		import_lights = has_feature(SceneImportFlags.LIGHTS_AND_CAMERAS);
		import_cameras = has_feature(SceneImportFlags.LIGHTS_AND_CAMERAS);
		import_textures = true;
		create_textures_folder = true;
		import_materials = true;
		create_materials_folder = true;
		create_colliders = false;
		import_lods = true;
		tangents = "calculate";

		import_animation = has_feature(SceneImportFlags.ANIMATIONS);
		new_skeleton = has_feature(SceneImportFlags.ANIMATIONS);
		target_skeleton = "";
		import_clips = has_feature(SceneImportFlags.ANIMATIONS);
		create_animations_folder = has_feature(SceneImportFlags.ANIMATIONS);
	}

	public bool has_feature(SceneImportFlags feature)
	{
		return (flags & feature) != 0;
	}

	void decode_option(string key, Value? value)
	{
		if (key == "import_lights" && has_feature(SceneImportFlags.LIGHTS_AND_CAMERAS))
			import_lights = (bool)value;
		else if (key == "import_cameras" && has_feature(SceneImportFlags.LIGHTS_AND_CAMERAS))
			import_cameras = (bool)value;
		else if (key == "import_textures")
			import_textures = (bool)value;
		else if (key == "create_textures_folder")
			create_textures_folder = (bool)value;
		else if (key == "import_materials")
			import_materials = (bool)value;
		else if (key == "create_materials_folder")
			create_materials_folder = (bool)value;
		else if (key == "create_colliders")
			create_colliders = (bool)value;
		else if (key == "import_lods")
			import_lods = (bool)value;
		else if (key == "tangents")
			tangents = (string)value;
		else if (key == "new_skeleton" && has_feature(SceneImportFlags.ANIMATIONS))
			new_skeleton = (bool)value;
		else if (key == "target_skeleton" && has_feature(SceneImportFlags.ANIMATIONS))
			target_skeleton = (string)value;
		else if (key == "import_clips" && has_feature(SceneImportFlags.ANIMATIONS))
			import_clips = (bool)value;
		else if (key == "create_animations_folder" && has_feature(SceneImportFlags.ANIMATIONS))
			create_animations_folder = (bool)value;
		else
			logw("Unknown option '%s'".printf(key));
	}

	public void decode(GLib.HashTable<string, Value?> json)
	{
		json.foreach(decode_option);

		if (has_feature(SceneImportFlags.ANIMATIONS)) {
			import_units = import_lights
				|| import_cameras
				|| import_textures
				|| import_materials
				|| import_lods
				;

			import_animation = new_skeleton
				|| import_clips
				;
		}
	}

	public GLib.HashTable<string, Value?> encode()
	{
		bool skip_units = !import_units;
		bool skip_anims = !import_animation;

		GLib.HashTable<string, Value?> obj = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);

		if (has_feature(SceneImportFlags.LIGHTS_AND_CAMERAS)) {
			obj.set("import_lights", skip_units ? false : import_lights);
			obj.set("import_cameras", skip_units ? false : import_cameras);
		}
		obj.set("import_textures", skip_units ? false : import_textures);
		obj.set("create_textures_folder", skip_units ? false : create_textures_folder);
		obj.set("import_materials", skip_units ? false : import_materials);
		obj.set("create_materials_folder", skip_units ? false : create_materials_folder);
		obj.set("create_colliders", skip_units ? false : create_colliders);
		obj.set("import_lods", skip_units ? false : import_lods);
		obj.set("tangents", tangents);
		if (has_feature(SceneImportFlags.ANIMATIONS)) {
			obj.set("new_skeleton", skip_anims ? false : new_skeleton);
			obj.set("target_skeleton", skip_anims ? "" : target_skeleton);
			obj.set("import_clips", skip_anims ? false : import_clips);
			obj.set("create_animations_folder", skip_anims ? false : create_animations_folder);
		}

		return obj;
	}
}

public class SceneImportDialog : Gtk.Window
{
	public Project _project;
	public string _destination_dir;
	public GLib.GenericArray<string> _filenames;
	public unowned Import _import_result;
	public unowned SceneImporter _importer;

	public string _options_path;
	public SceneImportOptions _options;

	public InputBool _import_units;
	public InputBool _import_lights;
	public InputBool _import_cameras;
	public InputBool _import_textures;
	public InputBool _create_textures_folder;
	public InputBool _import_materials;
	public InputBool _create_materials_folder;
	public InputBool _create_colliders;
	public InputBool _import_lods;
	public InputEnum _tangents;

	public InputBool _import_animation;
	public InputBool _new_skeleton;
	public InputResource _target_skeleton;
	public InputBool _import_clips;
	public InputBool _create_animations_folder;

	public PropertyGridSet _general_set;
	public Gtk.Box _box;

	public Gtk.Button _import;
	public Gtk.Button _cancel;
	public Gtk.HeaderBar _header_bar;

	public SceneImportDialog(Database database
		, string destination_dir
		, GLib.SList<string> filenames
		, Import import_result
		, owned SceneImportOptions options
		, string options_path
		, string title
		, SceneImporter importer
		)
	{
		_project = database._project;
		_destination_dir = destination_dir;
		_filenames = new GLib.GenericArray<string>();
		foreach (var f in filenames)
			_filenames.add(f);
		_import_result = import_result;
		_options = (owned)options;
		_options_path = options_path;
		_importer = importer;

		_general_set = new PropertyGridSet();

		_import_units = new InputBool();
		_import_units.value = _options.import_units;
		_import_lights = new InputBool();
		_import_lights.value = _options.import_lights;
		_import_cameras = new InputBool();
		_import_cameras.value = _options.import_cameras;
		_import_textures = new InputBool();
		_import_textures.value = _options.import_textures;
		_create_textures_folder = new InputBool();
		_create_textures_folder.value = _options.create_textures_folder;
		_import_materials = new InputBool();
		_import_materials.value = _options.import_materials;
		_create_materials_folder = new InputBool();
		_create_materials_folder.value = _options.create_materials_folder;
		_create_colliders = new InputBool();
		_create_colliders.value = _options.create_colliders;
		_import_lods = new InputBool();
		_import_lods.value = _options.import_lods;
		_tangents = new InputEnum(_options.tangents
			, new string[] { _("Calculate"), _("Import") }
			, new string[] { "calculate", "import" }
			);

		_import_animation = new InputBool();
		_import_animation.value = _options.import_animation;
		_new_skeleton = new InputBool();
		_new_skeleton.value = _options.new_skeleton;
		_target_skeleton = new InputResource(OBJECT_TYPE_MESH_SKELETON, database);
		_target_skeleton.value = _options.target_skeleton != "" ? _options.target_skeleton : null;
		_import_clips = new InputBool();
		_import_clips.value = _options.import_clips;
		_create_animations_folder = new InputBool();
		_create_animations_folder.value = _options.create_animations_folder;

		PropertyGrid cv;
		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		if (_options.has_feature(SceneImportFlags.LIGHTS_AND_CAMERAS)) {
			cv.add_row(_("Import Lights"), _import_lights, _("Import all light nodes."));
			cv.add_row(_("Import Cameras"), _import_cameras, _("Import all camera nodes."));
		}
		cv.add_row(_("Import Textures"), _import_textures, _("Import all textures."));
		cv.add_row(_("Create Textures Folder"), _create_textures_folder, _("Put imported textures in a sub-folder."));
		cv.add_row(_("Import Materials"), _import_materials, _("Import all materials."));
		cv.add_row(_("Create Materials Folder"), _create_materials_folder, _("Put imported materials in a sub-folder."));
		cv.add_row(_("Create Colliders"), _create_colliders, _("Create colliders and actors for each imported unit."));
		cv.add_row(_("Import LODs"), _import_lods, _("Create LOD Group component in the root unit if any LOD exists."));
		cv.add_row(_("Tangents"), _tangents, _("Import tangents from source or calculate them with MikkTSpace."));
		if (_options.has_feature(SceneImportFlags.ANIMATIONS))
			_general_set.add_property_grid_optional(cv, _("Units"), _import_units, _("Import nodes as units, materials and textures."));
		else
			_general_set.add_property_grid(cv, _("Units"));

		if (_options.has_feature(SceneImportFlags.ANIMATIONS)) {
			cv = new PropertyGrid();
			cv.column_homogeneous = true;
			cv.add_row(_("New Skeleton"), _new_skeleton, _("Create a new skeleton."));
			cv.add_row(_("Target Skeleton"), _target_skeleton, _("Skeleton to use."));
			cv.add_row(_("Import Animations"), _import_clips, _("Import all animation clips."));
			cv.add_row(_("Create Animations Folder"), _create_animations_folder, _("Put imported animations in a sub-folder."));
			_general_set.add_property_grid_optional(cv, _("Animation"), _import_animation, _("Import animations and skeleton."));
		}

		_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
#if CROWN_GTK3
		_box.pack_start(_general_set, false, false);
#else
		_box.append(_general_set);
#endif

		_cancel = new Gtk.Button.with_label(_("Cancel"));
		_cancel.clicked.connect(cancel);
		_import = new Gtk.Button.with_label(_("Import"));
#if CROWN_GTK3
		_import.get_style_context().add_class("suggested-action");
#else
		_import.add_css_class("suggested-action");
#endif
		_import.clicked.connect(import);

		_header_bar = new Gtk.HeaderBar();
#if CROWN_GTK3
		_header_bar.title = title;
		_header_bar.show_close_button = true;
#else
		_header_bar.set_title_widget(new Gtk.Label(title));
		_header_bar.show_title_buttons = true;
#endif
		_header_bar.pack_start(_cancel);
		_header_bar.pack_end(_import);
		this.title = title;

		if (_options.has_feature(SceneImportFlags.ANIMATIONS)) {
			_import_units.value_changed.connect(on_import_units_changed);
			_import_units.value_changed.connect(on_import_options_changed);
			_import_animation.value_changed.connect(on_import_animation_changed);
			_import_animation.value_changed.connect(on_import_options_changed);
			_new_skeleton.value_changed.connect(on_new_skeleton_changed);
			_new_skeleton.value_changed.connect(on_import_options_changed);
			_target_skeleton.value_changed.connect(on_import_options_changed);
			_import_clips.value_changed.connect(on_import_animations_changed);
			_import_clips.value_changed.connect(on_import_options_changed);
		}
		_import_textures.value_changed.connect(on_import_textures_changed);
		_import_materials.value_changed.connect(on_import_materials_changed);

		on_import_units_changed();
		on_import_textures_changed();
		on_import_materials_changed();
		on_import_animation_changed();
		on_import_animations_changed();
		on_new_skeleton_changed();
		on_import_options_changed();

		this.set_titlebar(_header_bar);
		this.set_default_size(_options.has_feature(SceneImportFlags.ANIMATIONS) ? 456 : 391
			, _options.has_feature(SceneImportFlags.ANIMATIONS) ? 430 : 258
			);
#if CROWN_GTK3
		this.add(_box);
#else
		this.set_child(_box);
#endif
	}

	void cancel()
	{
		close();
	}

	void import()
	{
		read_options();
		_importer(_import_result, _options, _project, _destination_dir, _filenames, _options_path);
		close();
	}

	void read_options()
	{
		bool has_animations = _options.has_feature(SceneImportFlags.ANIMATIONS);
		bool skip_units = has_animations && !_import_units.value;
		bool skip_anims = !has_animations || !_import_animation.value;
		string? target_skeleton = _target_skeleton.value;

		_options.import_units = !skip_units;
		_options.import_lights = skip_units || !_options.has_feature(SceneImportFlags.LIGHTS_AND_CAMERAS) ? false : _import_lights.value;
		_options.import_cameras = skip_units || !_options.has_feature(SceneImportFlags.LIGHTS_AND_CAMERAS) ? false : _import_cameras.value;
		_options.import_textures = skip_units ? false : _import_textures.value;
		_options.create_textures_folder = skip_units ? false : _create_textures_folder.value;
		_options.import_materials = skip_units ? false : _import_materials.value;
		_options.create_materials_folder = skip_units ? false : _create_materials_folder.value;
		_options.create_colliders = skip_units ? false : _create_colliders.value;
		_options.import_lods = skip_units ? false : _import_lods.value;
		_options.tangents = _tangents.value;

		_options.import_animation = has_animations && _import_animation.value;
		_options.new_skeleton = skip_anims ? false : _new_skeleton.value;
		_options.target_skeleton = (skip_anims || target_skeleton == null) ? "" : target_skeleton;
		_options.import_clips = skip_anims ? false : _import_clips.value;
		_options.create_animations_folder = skip_anims ? false : _create_animations_folder.value;
	}

	void on_import_units_changed()
	{
		_import_lights.sensitive = _import_units.value;
		_import_cameras.sensitive = _import_units.value;
		_import_textures.sensitive = _import_units.value;
		_create_textures_folder.sensitive = _import_units.value;
		_import_materials.sensitive = _import_units.value;
		_create_materials_folder.sensitive = _import_units.value;
		_create_colliders.sensitive = _import_units.value;
		_import_lods.sensitive = _import_units.value;
	}

	void on_import_textures_changed()
	{
		_create_textures_folder.set_sensitive(_import_units.value && _import_textures.value);
	}

	void on_import_materials_changed()
	{
		_create_materials_folder.set_sensitive(_import_units.value && _import_materials.value);
	}

	void on_import_animations_changed()
	{
		_create_animations_folder.set_sensitive(_import_animation.value && _import_clips.value);
		_target_skeleton.sensitive = _import_animation.value && !_new_skeleton.value && _import_clips.value;
	}

	void on_import_animation_changed()
	{
		_new_skeleton.sensitive = _import_animation.value;
		_import_clips.sensitive = _import_animation.value;
		_create_animations_folder.sensitive = _import_animation.value && _import_clips.value;
		_target_skeleton.sensitive = _import_animation.value && !_new_skeleton.value && _import_clips.value;
	}

	void on_new_skeleton_changed()
	{
		_target_skeleton.sensitive = _import_animation.value && !_new_skeleton.value && _import_clips.value;
	}

	void on_import_options_changed()
	{
		if (!_options.has_feature(SceneImportFlags.ANIMATIONS)) {
			_import.set_sensitive(true);
			return;
		}

		bool target_skeleton_is_valid = !_import_animation.value
			|| !_import_clips.value
			|| _new_skeleton.value
			|| (_target_skeleton.value != null && _target_skeleton.value != "")
			;

		bool has_options_to_import = _import_units.value
			|| (_import_animation.value
				&& (_new_skeleton.value || _import_clips.value))
			;

		_import.set_sensitive(has_options_to_import && target_skeleton_is_valid);
	}
}

} /* namespace Crown */
