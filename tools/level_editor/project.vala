/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public enum ImportResult
{
	SUCCESS, ///< Data imported successfully.
	ERROR,   ///< Error during import or elsewhere.
	CANCEL,  ///< User cancelled the import.
	CALLBACK ///< The actual importing will happen in the provided callback.
}

public delegate void Import(ImportResult result);

public class Project
{
	public const string LEVEL_EDITOR_TEST_NAME = "_level_editor_test";

	public delegate ImportResult ImporterDelegate(ProjectStore project_store
		, string destination_dir
		, SList<string> filenames
		, Import import_result
		, Gtk.Window? parent_window
		);

	[Compact]
	public struct ImporterData
	{
		public unowned ImporterDelegate delegate;
		public Gee.ArrayList<string> extensions;
		public double order;
		public Gtk.FileFilter _filter;
		public unowned Import import_result;

		ImporterData()
		{
			delegate = null;
			extensions = new Gee.ArrayList<string>();
			order = 0.0;
			_filter = new Gtk.FileFilter();
			import_result = null;
		}

		public bool can_import_extension(string extension)
		{
			string e = extension.down();

			foreach (var ext in extensions) {
				if (e == ext)
					return true;
			}

			return false;
		}

		public bool can_import_filenames(GLib.SList<string> filenames)
		{
			foreach (var filename in filenames) {
				if (!can_import_extension(path_extension(filename)))
					return false;
			}

			return true;
		}
	}

	// Data
	public File? _source_dir;
	public File _toolchain_dir;
	public File _data_dir;
	public File _user_dir;
	public File _level_editor_test_level;
	public File _level_editor_test_package;
	public string _platform;
	public Database _files;
	public Gee.HashMap<string, Guid?> _map;
	public ImporterData _all_extensions_importer_data;
	public Gee.ArrayList<ImporterData?> _importers;
	public bool _data_compiled;
	public Hashtable _data_index;

	public signal void file_added(string type, string name, uint64 size, uint64 mtime);
	public signal void file_changed(string type, string name, uint64 size, uint64 mtime);
	public signal void file_removed(string type, string name);
	public signal void tree_added(string name);
	public signal void tree_removed(string name);
	public signal void project_reset();
	public signal void project_loaded();

	public Project()
	{
#if CROWN_PLATFORM_WINDOWS
		_platform = "windows";
#else
		_platform = "linux";
#endif
		_files = new Database(this);
		_map = new Gee.HashMap<string, Guid?>();
		_all_extensions_importer_data = ImporterData();
		_all_extensions_importer_data.delegate = import_all_extensions;
		_importers = new Gee.ArrayList<ImporterData?>();
		_data_compiled = false;
		_data_index = new Hashtable();
	}

	public void data_compiled()
	{
		_data_compiled = true;

		try {
			string index_path = Path.build_filename(_data_dir.get_path(), "data_index.sjson");
			_data_index = SJSON.load_from_path(index_path);
		} catch (JsonSyntaxError e) {
			loge(e.message);
		}
	}

	public uint64 mtime(string type, string name)
	{
		var path = ResourceId.path(type, name);
		Guid id = _map[path];
		string mtime = _files.get_property_string(id, "mtime");
		return uint64.parse(mtime);
	}

	public void reset()
	{
		project_reset();

		_source_dir = null;

		_files.reset();
		_map.clear();
	}

	public bool is_loaded()
	{
		return _source_dir != null;
	}

	public void load(string source_dir)
	{
		reset();

		_source_dir = File.new_for_path(source_dir);
		_data_dir   = File.new_for_path(_source_dir.get_path() + "_" + _platform);

		_level_editor_test_level = File.new_for_path(Path.build_filename(_source_dir.get_path(), LEVEL_EDITOR_TEST_NAME + ".level"));
		_level_editor_test_package = File.new_for_path(Path.build_filename(_source_dir.get_path(), LEVEL_EDITOR_TEST_NAME + ".package"));

		// Cleanup source directory from previous runs' garbage
		delete_garbage();

		_user_dir = GLib.File.new_for_path(GLib.Path.build_filename(Crown._data_dir.get_path(), "projects", StringId64(source_dir).to_string()));
		try {
			_user_dir.make_directory_with_parents();
		} catch (Error e) {
			/* Nobody cares */
		}

		project_loaded();
	}

	public void set_toolchain_dir(string toolchain_dir)
	{
		_toolchain_dir = File.new_for_path(toolchain_dir);
	}

	public void create_initial_files(string source_dir)
	{
		// Write boot.config
		{
			string text = "// Lua script to launch on boot"
				+ "\nboot_script = \"core/game/boot\""
				+ "\n"
				+ "\n// Package to load on boot"
				+ "\nboot_package = \"boot\""
				+ "\n"
				+ "\nwindow_title = \"New Project\""
				+ "\n"
				+ "\n// Linux-only configs"
				+ "\nlinux = {"
				+ "\n	renderer = {"
				+ "\n		resolution = [ 1280 720 ]"
				+ "\n	}"
				+ "\n}"
				+ "\n"
				+ "\n// Windows-only configs"
				+ "\nwindows = {"
				+ "\n	renderer = {"
				+ "\n		resolution = [ 1280 720 ]"
				+ "\n	}"
				+ "\n}"
				+ "\n"
				;

			string path = Path.build_filename(source_dir, "boot.config");
			FileStream fs = FileStream.open(path, "wb");
			if (fs != null)
				fs.write(text.data);
		}

		// Write boot.package
		{
			string text = "lua = ["
				+ "\n	\"core/game/boot\""
				+ "\n]"
				+ "\nshader = ["
				+ "\n	\"core/shaders/default\""
				+ "\n]"
				+ "\nphysics_config = ["
				+ "\n	\"global\""
				+ "\n]"
				+ "\nunit = ["
				+ "\n	\"core/units/camera\""
				+ "\n]"
				+ "\n"
				;

			string path = Path.build_filename(source_dir, "boot.package");
			FileStream fs = FileStream.open(path, "wb");
			if (fs != null)
				fs.write(text.data);
		}

		// Write global.physics_config
		{
			string text = "materials = {"
				+ "\n	default = { friction = 0.8 rolling_friction = 0.5 restitution = 0.81 }"
				+ "\n}"
				+ "\n"
				+ "\ncollision_filters = {"
				+ "\n	no_collision = { collides_with = [] }"
				+ "\n	default = { collides_with = [ \"default\" ] }"
				+ "\n}"
				+ "\n"
				+ "\nactors = {"
				+ "\n	static = { dynamic = false }"
				+ "\n	dynamic = { dynamic = true }"
				+ "\n	keyframed = { dynamic = true kinematic = true disable_gravity = true }"
				+ "\n}"
				+ "\n"
				;

			string path = Path.build_filename(source_dir, "global.physics_config");
			FileStream fs = FileStream.open(path, "wb");
			if (fs != null)
				fs.write(text.data);
		}

		// Write main.lua
		{
			string text = "require \"core/game/camera\""
				+ "\n"
				+ "\nGame = Game or {"
				+ "\n	sg = nil,"
				+ "\n	pw = nil,"
				+ "\n	rw = nil,"
				+ "\n	camera = nil,"
				+ "\n}"
				+ "\n"
				+ "\nGameBase.game = Game"
				+ "\nGameBase.game_level = nil"
				+ "\n"
				+ "\nfunction Game.level_loaded()"
				+ "\n	Device.enable_resource_autoload(true)"
				+ "\n"
				+ "\n	Game.sg = World.scene_graph(GameBase.world)"
				+ "\n	Game.pw = World.physics_world(GameBase.world)"
				+ "\n	Game.rw = World.render_world(GameBase.world)"
				+ "\n	Game.camera = FPSCamera(GameBase.world, GameBase.camera_unit)"
				+ "\nend"
				+ "\n"
				+ "\nfunction Game.update(dt)"
				+ "\n	-- Stop the engine when the 'ESC' key is released"
				+ "\n	if Keyboard.released(Keyboard.button_id(\"escape\")) then"
				+ "\n		Device.quit()"
				+ "\n	end"
				+ "\n"
				+ "\n	-- Update camera"
				+ "\n	local delta = Vector3.zero()"
				+ "\n	if Mouse.pressed(Mouse.button_id(\"right\")) then move = true end"
				+ "\n	if Mouse.released(Mouse.button_id(\"right\")) then move = false end"
				+ "\n	if move then delta = Mouse.axis(Mouse.axis_id(\"cursor_delta\")) end"
				+ "\n	Game.camera:update(dt, delta.x, delta.y)"
				+ "\nend"
				+ "\n"
				+ "\nfunction Game.render(dt)"
				+ "\nend"
				+ "\n"
				+ "\nfunction Game.shutdown()"
				+ "\nend"
				+ "\n"
				;

			string path = Path.build_filename(source_dir, "main.lua");
			FileStream fs = FileStream.open(path, "wb");
			if (fs != null)
				fs.write(text.data);
		}
	}

	public int create_script(string directory, string name, bool empty)
	{
		string script_path = Path.build_filename(directory, name + ".lua");
		string path = this.absolute_path(script_path);

		FileStream fs = FileStream.open(path, "wb");
		if (fs != null) {
			if (empty) {
				return fs.puts("\n");
			} else {
				string text = "local Behavior = Behavior or {}"
					+ "\nlocal Data = Data or {}"
					+ "\n"
					+ "\nfunction Behavior.spawned(world, units)"
					+ "\n	if Data[world] == nil then"
					+ "\n		Data[world] = {}"
					+ "\n	end"
					+ "\n"
					+ "\n	for uu = 1, #units do"
					+ "\n		local unit = units[uu]"
					+ "\n"
					+ "\n		-- Store instance-specific data"
					+ "\n		if Data[world][unit] == nil then"
					+ "\n			-- Data[world][unit] = {}"
					+ "\n		end"
					+ "\n"
					+ "\n		-- Do something with the unit"
					+ "\n	end"
					+ "\nend"
					+ "\n"
					+ "\nfunction Behavior.update(world, dt)"
					+ "\n	-- Update all units"
					+ "\nend"
					+ "\n"
					+ "\nfunction Behavior.unspawned(world, units)"
					+ "\n	-- Cleanup"
					+ "\n	for uu = 1, #units do"
					+ "\n		if Data[world][units] then"
					+ "\n			Data[world][units] = nil"
					+ "\n		end"
					+ "\n	end"
					+ "\nend"
					+ "\n"
					+ "\nreturn Behavior"
					+ "\n"
					;
				return fs.puts(text);
			}
		}

		return -1;
	}

	public int create_unit(string directory, string name)
	{
		string unit_path = Path.build_filename(directory, name + ".unit");
		string path = this.absolute_path(unit_path);

		Database db = new Database(this, null);
		Guid unit_id = Guid.new_guid();
		Unit unit = Unit(db, unit_id);
		unit.create_empty();

		db.dump(path, unit_id);
		return 0;
	}

	public int create_state_machine(string directory, string name)
	{
		string resource_name = Path.build_filename(directory, name);

		Database db = new Database(this, null);
		Guid node_id = Guid.new_guid();
		StateMachineNode sm_node = StateMachineNode(db, node_id);
		Guid machine_id = Guid.new_guid();
		StateMachineResource machine = StateMachineResource(db, machine_id, sm_node);
		machine.add_node(sm_node);

		machine.save(this, resource_name);
		return 0;
	}

	// Returns the absolute path to the source directory.
	public string source_dir()
	{
		if (_source_dir == null)
			return "";
		else
			return _source_dir.get_path();
	}

	// Returns the absolute path to the toolchain directory.
	public string toolchain_dir()
	{
		return _toolchain_dir.get_path();
	}

	// Returns the absolute path to the data directory.
	public string data_dir()
	{
		return _data_dir.get_path();
	}

	// Returns the absolute path to the user-specific data for this project.
	public string user_dir()
	{
		return _user_dir.get_path();
	}

	public string platform()
	{
		return _platform;
	}

	public string name()
	{
		string sd = source_dir();
		return sd.substring(sd.last_index_of_char(GLib.Path.DIR_SEPARATOR) + 1);
	}

	public bool path_is_within_source_dir(string path)
	{
		GLib.File file = GLib.File.new_for_path(path);
		return file.has_prefix(_source_dir);
	}

	public void delete_garbage()
	{
		try {
			_level_editor_test_level.delete();
			_level_editor_test_package.delete();
		} catch (GLib.Error e) {
			// Ignored
		}
	}

	/// Converts the @a resource_id to its corresponding human-readable @a
	/// resource_name. It returns true if the conversion is successful, otherwise
	/// it returns false and sets @a resource_name to the value of @a resource_id.
	public bool resource_id_to_name(out string resource_name, string resource_id)
	{
		Value? name = _data_index[resource_id];
		if (name != null) {
			resource_name = (string)name;
			return true;
		}

		resource_name = resource_id;
		return false;
	}

	public Database files()
	{
		return _files;
	}

	public void add_file(string path, uint64 size, uint64 mtime)
	{
		string type = path_extension(path);
		string name = type == "" ? path : path.substring(0, path.last_index_of("."));

		Guid id = Guid.new_guid();
		_files.create(id, OBJECT_TYPE_FILE);
		_files.set_property_string(id, "path", path);
		_files.set_property_string(id, "type", type);
		_files.set_property_string(id, "name", name);
		_files.set_property_string(id, "size", size.to_string());
		_files.set_property_string(id, "mtime", mtime.to_string());
		_files.add_to_set(GUID_ZERO, "data", id);

		_map[path] = id;

		file_added(type, name, size, mtime);
	}

	public void change_file(string path, uint64 size, uint64 mtime)
	{
		string type = path_extension(path);
		string name = type == "" ? path : path.substring(0, path.last_index_of("."));

		Guid id = _map[path];
		_files.set_property_string(id, "size", size.to_string());
		_files.set_property_string(id, "mtime", mtime.to_string());

		_data_compiled = false;

		file_changed(type, name, size, mtime);
	}

	public void remove_file(string path)
	{
		if (!_map.has_key(path)) {
			logw("remove_file: map does not contain path: %s".printf(path));
			return;
		}

		Guid id = _map[path];
		file_removed(_files.get_property_string(id, "type"), _files.get_property_string(id, "name"));

		_files.remove_from_set(GUID_ZERO, "data", id);
		_files.destroy(id);

		_map.unset(path);
	}

	public void add_tree(string path)
	{
		tree_added(path);
	}

	public void remove_tree(string path)
	{
		tree_removed(path);
	}

	public string resource_filename(string absolute_path)
	{
		string prefix = _source_dir.get_path();

		if (absolute_path.has_prefix(_toolchain_dir.get_path() + "/core"))
			prefix = _toolchain_dir.get_path();

		return File.new_for_path(prefix).get_relative_path(File.new_for_path(absolute_path));
	}

	public string absolute_path(string resource_path)
	{
		string prefix = _source_dir.get_path();

		if (resource_path.has_prefix("core/") || resource_path == "core")
			prefix = _toolchain_dir.get_path();

		return Path.build_filename(prefix, resource_path);
	}

	public static ImportResult import_all_extensions(ProjectStore project_store, string destination_dir, SList<string> filenames, Import import_result, Gtk.Window? parent_window)
	{
		Project project = project_store._project;

		Gee.ArrayList<string> paths = new Gee.ArrayList<string>();
		foreach (var item in filenames)
			paths.add(item);

		paths.sort((a, b) => {
				int ext_a = a.last_index_of_char('.');
				int ext_b = b.last_index_of_char('.');
				return strcmp(a[ext_a : a.length], b[ext_b : b.length]);
			});

		int result = 0;
		while (paths.size != 0 && result == ImportResult.SUCCESS) {
			// Find importer for the first file in the list of selected filenames.
			ImporterData? importer = project.find_importer_for_path(paths[0]);
			if (importer == null)
				return ImportResult.ERROR;

			// Create the list of all filenames importable by importer.
			Gee.ArrayList<string> importables = new Gee.ArrayList<string>();
			var cur = paths.list_iterator();
			for (var has_next = cur.next(); has_next; has_next = cur.next()) {
				string path = paths[cur.index()];

				if (importer.can_import_extension(path_extension(path))) {
					importables.add(path);
					cur.remove();
				}
			}

			// If importables is empty, filenames must have been filled with
			// un-importable filenames...
			if (importables.size == 0)
				return ImportResult.ERROR;

			// Convert importables to SList<string> to be used as delegate param.
			SList<string> importables_list = new SList<string>();
			foreach (var item in importables)
				importables_list.append(item);

			result = importer.delegate(project_store, destination_dir, importables_list, import_result, parent_window);
		}

		return result;
	}

	public class FileFilterFuncData
	{
		public string extension;

		public FileFilterFuncData(string ext)
		{
			extension = ext;
		}

		public bool handler(Gtk.FileFilterInfo info)
		{
			return info.filename.down().has_suffix("." + extension);
		}
	}

	// Returns a Gtk.FileFilter based on file @a extensions list.
	public Gtk.FileFilter create_gtk_file_filter(string name, Gee.ArrayList<string> extensions)
	{
		Gtk.FileFilter filter = new Gtk.FileFilter();

		string extensions_comma_separated = "";
		foreach (var ext in extensions) {
			extensions_comma_separated += "*.%s, ".printf(ext);
			FileFilterFuncData data = new FileFilterFuncData(ext);
			filter.add_custom(Gtk.FileFilterFlags.FILENAME, data.handler);
		}
		filter.set_filter_name(name + " (%s)".printf(extensions_comma_separated[0 : -2]));

		return filter;
	}

	public void register_importer_internal(string name, ref ImporterData data)
	{
		data._filter = create_gtk_file_filter(name, data.extensions);
		_importers.add(data);
		_importers.sort((a, b) => { return a.order < b.order ? -1 : 1; });

		// Skip duplicated extensions.
		foreach (string ext in data.extensions) {
			if (!_all_extensions_importer_data.extensions.contains(ext))
				_all_extensions_importer_data.extensions.add(ext);
		}

		_all_extensions_importer_data._filter = create_gtk_file_filter("All", _all_extensions_importer_data.extensions);
	}

	// Registers an @a importer for importing source data with the given @a
	// extensions. @a order is used to establish precedence when distinct importers
	// support similar extensions; lower values have higher precedence.
	public void register_importer(string name, string[] extensions, ImporterDelegate importer, Import import_result, double order)
	{
		ImporterData data = ImporterData();
		data.delegate = importer;
		data.extensions.add_all_array(extensions);
		data.order = order;
		data.import_result = import_result;

		register_importer_internal(name, ref data);
	}

	// Returns the preferable importer (lowest order values) which can import files
	// with the given @a extension.
	public ImporterData? find_importer_for_extension(string extension)
	{
		foreach (var imp in _importers) {
			if (imp.can_import_extension(extension))
				return imp;
		}

		return null;
	}

	public ImporterData? find_importer_for_path(string path)
	{
		return find_importer_for_extension(path_extension(path));
	}

	public bool is_type_importable(string type)
	{
		return find_importer_for_extension(type) != null;
	}

	public ImportResult import(string? destination_dir, Import import_result, ProjectStore project_store, Gtk.Window? parent_window = null)
	{
		Gtk.FileChooserDialog src = new Gtk.FileChooserDialog("Import..."
			, parent_window
			, Gtk.FileChooserAction.OPEN
			, "Cancel"
			, Gtk.ResponseType.CANCEL
			, "Open"
			, Gtk.ResponseType.ACCEPT
			);
		src.select_multiple = true;
		foreach (var importer in _importers)
			src.add_filter(importer._filter);
		src.add_filter(_all_extensions_importer_data._filter);
		src.set_filter(_all_extensions_importer_data._filter);

		if (src.run() != (int)Gtk.ResponseType.ACCEPT) {
			src.destroy();
			return ImportResult.CANCEL;
		}

		string out_dir = "";
		if (destination_dir == null) {
			Gtk.FileChooserDialog dst = new Gtk.FileChooserDialog("Select destination folder..."
				, parent_window
				, Gtk.FileChooserAction.SELECT_FOLDER
				, "Cancel"
				, Gtk.ResponseType.CANCEL
				, "Select"
				, Gtk.ResponseType.ACCEPT
				);
			dst.set_current_folder(this.source_dir());

			if (dst.run() != (int)Gtk.ResponseType.ACCEPT) {
				dst.destroy();
				src.destroy();
				return ImportResult.CANCEL;
			}

			out_dir = dst.get_filename();
			dst.destroy();
		} else {
			out_dir = this.absolute_path(destination_dir);
		}

		Gtk.FileFilter? current_filter = src.get_filter();
		GLib.SList<string> filenames = src.get_filenames();
		src.destroy();

		// Find importer callback.
		unowned ImporterDelegate? importer = null;
		foreach (var imp in _importers) {
			if (imp._filter == current_filter && imp.can_import_filenames(filenames)) {
				importer = imp.delegate;
				break;
			}
		}
		// Fallback if no importer found.
		if (importer == null)
			importer = _all_extensions_importer_data.delegate;

		return importer(project_store, out_dir, filenames, import_result, parent_window);
	}

	public void delete_tree(GLib.File file) throws Error
	{
		GLib.FileEnumerator fe = file.enumerate_children("standard::*"
			, GLib.FileQueryInfoFlags.NOFOLLOW_SYMLINKS
			);

		GLib.FileInfo info = null;
		while ((info = fe.next_file()) != null) {
			GLib.File subfile = file.resolve_relative_path(info.get_name());

			if (info.get_file_type() == GLib.FileType.DIRECTORY)
				delete_tree(subfile);
			else
				subfile.delete();
		}

		file.delete();
	}
}

} /* namespace Crown */
