/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;
using Gee;

namespace Crown
{
public class Project
{
	public const string LEVEL_EDITOR_TEST_NAME = "_level_editor_test";

	public delegate int ImporterDelegate(Project project, string destination_dir, SList<string> filenames);

	[Compact]
	public struct ImporterData
	{
		public unowned ImporterDelegate delegate;
		public Gee.ArrayList<string> extensions;
		public double order;
		public Gtk.FileFilter _filter;

		ImporterData()
		{
			delegate = null;
			extensions = new Gee.ArrayList<string>();
			order = 0.0;
			_filter = new Gtk.FileFilter();
		}
	}

	// Data
	public File _source_dir;
	public File _toolchain_dir;
	public File _data_dir;
	public File _level_editor_test_level;
	public File _level_editor_test_package;
	public string _platform;
	public Database _database;
	public Database _files;
	public HashMap<string, Guid?> _map;
	public ImporterData _all_extensions_importer_data;
	public Gee.ArrayList<ImporterData?> _importers;
	public DataCompiler _data_compiler;

	public signal void file_added(string type, string name);
	public signal void file_removed(string type, string name);
	public signal void tree_added(string name);
	public signal void tree_removed(string name);
	public signal void project_reset();
	public signal void project_loaded();

	public Project(Database db, DataCompiler dc)
	{
#if CROWN_PLATFORM_LINUX
		_platform = "linux";
#elif CROWN_PLATFORM_WINDOWS
		_platform = "windows";
#endif // CROWN_PLATFORM_LINUX
		_database = db;
		_files = new Database();
		_map = new HashMap<string, Guid?>();
		_all_extensions_importer_data = ImporterData();
		_all_extensions_importer_data.delegate = import_all_extensions;
		_importers = new Gee.ArrayList<ImporterData?>();
		_data_compiler = dc;
	}

	public void reset()
	{
		_files.reset();
		_map.clear();

		project_reset();
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

		project_loaded();
	}

	/// Loads the unit @a name and all its prefabs recursively into the database.
	public void load_unit(string name)
	{
		string resource_path = name + ".unit";

		// If the unit is already loaded.
		if (_database.has_property(GUID_ZERO, resource_path))
			return;

		// Load the unit.
		string path = resource_path_to_absolute_path(resource_path);

		Guid prefab_id = GUID_ZERO;
		if (_database.load_more_from_path(ref prefab_id, path, resource_path) != 0)
			return; // Caller can query the database to check for error.
		assert(prefab_id != GUID_ZERO);

		// Load all prefabs recursively, if any.
		Value? prefab = _database.get_property(prefab_id, "prefab");
		if (prefab != null)
			load_unit((string)prefab);
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
				+ "\n	\"core/game/camera\""
				+ "\n	\"core/game/game\""
				+ "\n	\"core/lua/class\""
				+ "\n	\"main\""
				+ "\n]"
				+ "\nshader = ["
				+ "\n	\"core/shaders/common\""
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

	public void create_script(string directory, string name, bool empty)
	{
		string path = Path.build_filename(_source_dir.get_path(), directory + "/" + name + ".lua");
		FileStream fs = FileStream.open(path, "wb");
		if (fs != null)
		{
			if (empty)
			{
				fs.puts("\n");
			}
			else
			{
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
				fs.puts(text);
			}

			_data_compiler.compile.begin(this.data_dir(), this.platform(), (obj, res) => {
				_data_compiler.compile.end(res);
			});
		}
	}

	public void create_unit(string directory, string name)
	{
		string path = Path.build_filename(_source_dir.get_path(), directory + "/" + name + ".unit");
		FileStream fs = FileStream.open(path, "wb");
		if (fs != null)
		{
			fs.puts("\ncomponents = [");
			fs.puts("\n]");
			fs.puts("\n");

			_data_compiler.compile.begin(this.data_dir(), this.platform(), (obj, res) => {
				_data_compiler.compile.end(res);
			});
		}
	}

	// Returns the absolute path to the source directory.
	public string source_dir()
	{
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
		try
		{
			_level_editor_test_level.delete();
			_level_editor_test_package.delete();
		}
		catch (GLib.Error e)
		{
			// Ignored
		}
	}

	/// Converts the @a resource_id to its corresponding human-readable @a
	/// resource_name. It returns true if the conversion is successful, otherwise
	/// it returns false and sets @a resource_name to the value of @a resource_id.
	public bool resource_id_to_name(out string resource_name, string resource_id)
	{
		string index_path = Path.build_filename(_data_dir.get_path(), "data_index.sjson");
		Hashtable index = SJSON.load_from_path(index_path);
		Value? name = index[resource_id];
		if (name != null)
		{
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

	public string basename(string path)
	{
		int ls = path.last_index_of("/");
		return ls == -1 ? path : path.substring(ls + 1);
	}

	public string extension(string path)
	{
		string bn = basename(path);
		int ld = bn.last_index_of(".");
		return (ld == -1 || ld == 0) ? "" : bn.substring(ld + 1);
	}

	public void add_file(string path)
	{
		string type = extension(path);
		string name = type == "" ? path : path.substring(0, path.last_index_of("."));

		Guid id = Guid.new_guid();
		_files.create(id);
		_files.set_property_string(id, "path", path);
		_files.set_property_string(id, "type", type);
		_files.set_property_string(id, "name", name);
		_files.add_to_set(GUID_ZERO, "data", id);

		_map[path] = id;

		file_added(type, name);
	}

	public void remove_file(string path)
	{
		if (!_map.has_key(path))
		{
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

	/// Converts @a path to the corresponding resource name.
	/// On Linux, no transformation is needed. On Windows,
	/// backslashes are converted to slashes.
	public string resource_path_to_resource_name(string path)
	{
		return path.replace("\\", "/");
	}

	public string resource_filename_to_resource_path(string filename)
	{
		return filename.substring(0, filename.last_index_of_char('.'));
	}

	public string absolute_path_to_resource_filename(string path)
	{
		string prefix = _source_dir.get_path();

		if (path.has_prefix(_toolchain_dir.get_path() + "/core"))
			prefix = _toolchain_dir.get_path() + "/core";

		return File.new_for_path(prefix).get_relative_path(File.new_for_path(path));
	}

	public string resource_path_to_absolute_path(string resource_path)
	{
		string prefix = _source_dir.get_path();

		if (resource_path.has_prefix("core/"))
			prefix = _toolchain_dir.get_path();

		return Path.build_filename(prefix, resource_path);
	}

	public static int import_all_extensions(Project project, string destination_dir, SList<string> filenames)
	{
		Gee.ArrayList<string> paths = new Gee.ArrayList<string>();
		foreach (var item in filenames)
			paths.add(item);

		paths.sort((a, b) => {
			int ext_a = a.last_index_of_char('.');
			int ext_b = b.last_index_of_char('.');
			return strcmp(a[ext_a:a.length], b[ext_b:b.length]);
		});

		int success = 0;
		while (paths.size != 0 && success == 0)
		{
			// Find importer for the first file in the list of selected filenames.
			ImporterData? importer = project.find_importer_for_path(paths[0]);
			if (importer == null)
				return -1;

			// Create the list of all filenames importable by importer.
			Gee.ArrayList<string> importables = new Gee.ArrayList<string>();
			var cur = paths.list_iterator();
			for (var has_next = cur.next(); has_next; has_next = cur.next())
			{
				string path = paths[cur.index()];

				foreach (var ext in importer.extensions)
				{
					if (path.has_suffix("." + ext))
					{
						importables.add(path);
						cur.remove();
					}
				}
			}

			// If importables is empty, filenames must have been filled with
			// un-importable filenames...
			if (importables.size == 0)
				return -1;

			// Convert importables to SList<string> to be used as delegate param.
			SList<string> importables_list = new SList<string>();
			foreach (var item in importables)
				importables_list.append(item);

			success = importer.delegate(project, destination_dir, importables_list);
		}

		return success;
	}

	// Returns a Gtk.FileFilter based on file @a extensions list.
	public Gtk.FileFilter create_gtk_file_filter(string name, ArrayList<string> extensions)
	{
		Gtk.FileFilter filter = new Gtk.FileFilter();

		string extensions_comma_separated = "";
		foreach (var ext in extensions)
		{
			extensions_comma_separated += "*.%s, ".printf(ext);
			filter.add_pattern("*.%s".printf(ext));
		}
		filter.set_filter_name(name + " (%s)".printf(extensions_comma_separated[0:-2]));

		return filter;
	}

	public void register_importer_internal(string name, ref ImporterData data)
	{
		data._filter = create_gtk_file_filter(name, data.extensions);
		_importers.add(data);
		_importers.sort((a, b) => { return (a.order < b.order ? -1 : 1); });

		_all_extensions_importer_data.extensions.add_all(data.extensions);
		_all_extensions_importer_data._filter = create_gtk_file_filter("All", _all_extensions_importer_data.extensions);
	}

	// Registers an @a importer for importing source data with the given @a
	// extensions. @a order is used to establish precedence when distinct importers
	// support similar extensions; lower values have higher precedence.
	public void register_importer(string name, string[] extensions, ImporterDelegate importer, double order)
	{
		ImporterData data = ImporterData();
		data.delegate = importer;
		data.extensions.add_all_array(extensions);
		data.order = order;

		register_importer_internal(name, ref data);
	}

	// Returns the preferable importer (lowest order values) which can import files
	// with the given @a extension.
	public ImporterData? find_importer_for_path(string path)
	{
		foreach (var imp in _importers)
		{
			foreach (var ext in imp.extensions)
			{
				if (path.has_suffix("." + ext))
					return imp;
			}
		}

		return null;
	}

	public void import(string? destination_dir, Gtk.Window? parent_window = null)
	{
		Gtk.FileChooserDialog src = new Gtk.FileChooserDialog("Import..."
			, parent_window
			, FileChooserAction.OPEN
			, "Cancel"
			, ResponseType.CANCEL
			, "Open"
			, ResponseType.ACCEPT
			);
		src.select_multiple = true;
		foreach (var importer in _importers)
			src.add_filter(importer._filter);
		src.add_filter(_all_extensions_importer_data._filter);
		src.set_filter(_all_extensions_importer_data._filter);

		if (src.run() != (int)ResponseType.ACCEPT)
		{
			src.destroy();
			return;
		}

		string out_dir = "";
		if (destination_dir == null)
		{
			Gtk.FileChooserDialog dst = new Gtk.FileChooserDialog("Select destination folder..."
				, parent_window
				, FileChooserAction.SELECT_FOLDER
				, "Cancel"
				, ResponseType.CANCEL
				, "Select"
				, ResponseType.ACCEPT
				);
			dst.set_current_folder(this.source_dir());

			if (dst.run() != (int)ResponseType.ACCEPT)
			{
				dst.destroy();
				src.destroy();
				return;
			}

			out_dir = dst.get_filename();
			dst.destroy();
		}
		else
		{
			out_dir = GLib.File.new_for_path(GLib.Path.build_filename(source_dir(), destination_dir)).get_path();
		}

		Gtk.FileFilter? current_filter = src.get_filter();
		GLib.SList<string> filenames = src.get_filenames();

		// Find importer callback
		unowned ImporterDelegate? importer = null;
		foreach (var imp in _importers)
		{
			if (imp._filter == current_filter)
			{
				importer = imp.delegate;
				break;
			}
		}
		// Fallback if no importer found
		if (importer == null)
			importer = _all_extensions_importer_data.delegate;

		// Import
		if (importer(this, out_dir, filenames) == 0)
		{
			_data_compiler.compile.begin(this.data_dir(), this.platform(), (obj, res) => {
				_data_compiler.compile.end(res);
			});
		}

		src.destroy();
	}

	public void delete_tree(GLib.File file) throws Error
	{
		GLib.FileEnumerator fe = file.enumerate_children("standard::*"
			, GLib.FileQueryInfoFlags.NOFOLLOW_SYMLINKS
			);

		GLib.FileInfo info = null;
		while ((info = fe.next_file()) != null)
		{
			GLib.File subfile = file.resolve_relative_path(info.get_name());

			if (info.get_file_type() == GLib.FileType.DIRECTORY)
				delete_tree(subfile);
			else
				subfile.delete();
		}

		file.delete();
	}
}

}
