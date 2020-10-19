/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;
using Gee;

namespace Crown
{
public class Project
{
	public const string LEVEL_EDITOR_TEST_NAME = "_level_editor_test";

	// Data
	public File _source_dir;
	public File _toolchain_dir;
	public File _data_dir;
	public File _level_editor_test_level;
	public File _level_editor_test_package;
	public string _platform;
	public Database _files;
	public HashMap<string, Guid?> _map;
	public DataCompiler _data_compiler;

	public signal void file_added(string type, string name);
	public signal void file_removed(string type, string name);
	public signal void tree_added(string name);
	public signal void tree_removed(string name);
	public signal void project_reset();
	public signal void project_loaded();

	public Project(DataCompiler dc)
	{
#if CROWN_PLATFORM_LINUX
		_platform = "linux";
#elif CROWN_PLATFORM_WINDOWS
		_platform = "windows";
#endif // CROWN_PLATFORM_LINUX
		_files = new Database();
		_map = new HashMap<string, Guid?>();
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
				+ "\n"
				+ "\n	-- Spawn camera"
				+ "\n	local camera_unit = World.spawn_unit(GameBase.world, \"core/units/camera\")"
				+ "\n	SceneGraph.set_local_position(Game.sg, camera_unit, Vector3(0, 6.5, -30))"
				+ "\n	GameBase.game_camera = camera_unit"
				+ "\n	Game.camera = FPSCamera(GameBase.world, camera_unit)"
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

	public string source_dir()
	{
		return _source_dir.get_path();
	}

	public string toolchain_dir()
	{
		return _toolchain_dir.get_path();
	}

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

	public bool path_is_within_dir(string path, string dir)
	{
		GLib.File file = GLib.File.new_for_path(path);
		return file.has_prefix(_source_dir);
	}

	public void dump_test_level(Database db)
	{
		// Save test level to file
		db.dump(_level_editor_test_level.get_path());

		// Save temporary package to reference test level
		ArrayList<Value?> level = new ArrayList<Value?>();
		level.add("_level_editor_test");
		Hashtable package = new Hashtable();
		package["level"] = level;
		SJSON.save(package, _level_editor_test_package.get_path());
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

	public string id_to_name(string id)
	{
		Hashtable index = SJSON.load(Path.build_filename(_data_dir.get_path(), "data_index.sjson"));
		Value? name = index[id];
		return name != null ? (string)name : id;
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
		assert(path.has_prefix(_source_dir.get_path()));
		return _source_dir.get_relative_path(File.new_for_path(path));
	}

	public int import_sprites(SList<string> filenames, string destination_dir)
	{
		Hashtable importer_settings = null;
		string importer_settings_path = null;
		{
			GLib.File file_src = File.new_for_path(filenames.nth_data(0));
			GLib.File file_dst = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));

			string resource_filename = _source_dir.get_relative_path(file_dst);
			string resource_path     = resource_filename.substring(0, resource_filename.last_index_of_char('.'));

			importer_settings_path = Path.build_filename(_source_dir.get_path(), resource_path) + ".importer_settings";
		}

		SpriteImportDialog sid = new SpriteImportDialog(filenames.nth_data(0));
		sid.show_all();

		if (File.new_for_path(importer_settings_path).query_exists())
		{
			importer_settings = SJSON.load(importer_settings_path);
			sid.load(importer_settings);
		}
		else
		{
			importer_settings = new Hashtable();
		}

		if (sid.run() != Gtk.ResponseType.OK)
		{
			sid.destroy();
			return 1;
		}

		sid.save(importer_settings);

		int width     = (int)sid._pixbuf.width;
		int height    = (int)sid._pixbuf.height;
		int num_h     = (int)sid.cells_hv.value.x;
		int num_v     = (int)sid.cells_hv.value.y;
		int cell_w    = (int)sid.cell_wh.value.x;
		int cell_h    = (int)sid.cell_wh.value.y;
		int offset_x  = (int)sid.offset_xy.value.x;
		int offset_y  = (int)sid.offset_xy.value.y;
		int spacing_x = (int)sid.spacing_xy.value.x;
		int spacing_y = (int)sid.spacing_xy.value.y;
		double layer  = sid.layer.value;
		double depth  = sid.depth.value;

		Vector2 pivot_xy = sprite_cell_pivot_xy(cell_w, cell_h, sid.pivot.active);

		bool collision_enabled         = sid.collision_enabled.active;
		string shape_active_name       = (string)sid.shape.visible_child_name;
		int circle_collision_center_x  = (int)sid.circle_collision_center_xy.value.x;
		int circle_collision_center_y  = (int)sid.circle_collision_center_xy.value.y;
		int circle_collision_radius    = (int)sid.circle_collision_radius.value;
		int capsule_collision_center_x = (int)sid.capsule_collision_center_xy.value.x;
		int capsule_collision_center_y = (int)sid.capsule_collision_center_xy.value.y;
		int capsule_collision_radius   = (int)sid.capsule_collision_radius.value;
		int capsule_collision_height   = (int)sid.capsule_collision_height.value;
		int collision_x                = (int)sid.collision_xy.value.x;
		int collision_y                = (int)sid.collision_xy.value.y;
		int collision_w                = (int)sid.collision_wh.value.x;
		int collision_h                = (int)sid.collision_wh.value.y;
		string actor_class             = (string)sid.actor_class.value;
		bool lock_rotation_y           = sid.lock_rotation_y.active;
		double mass                    = (double)sid.mass.value;

		sid.destroy();

		foreach (unowned string filename_i in filenames)
		{
			if (!filename_i.has_suffix(".png"))
				continue;

			GLib.File file_src = File.new_for_path(filename_i);
			GLib.File file_dst = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));

			string resource_filename = _source_dir.get_relative_path(file_dst);
			string resource_path     = resource_filename.substring(0, resource_filename.last_index_of_char('.'));
			string resource_name     = resource_path_to_resource_name(resource_path);

			SJSON.save(importer_settings, Path.build_filename(_source_dir.get_path(), resource_path) + ".importer_settings");

			Hashtable textures = new Hashtable();
			textures["u_albedo"] = resource_name;

			Hashtable uniform = new Hashtable();
			uniform["type"]  = "vector4";
			uniform["value"] = Vector4(1.0, 1.0, 1.0, 1.0).to_array();

			Hashtable uniforms = new Hashtable();
			uniforms["u_color"] = uniform;

			Hashtable material = new Hashtable();
			material["shader"]   = "sprite";
			material["textures"] = textures;
			material["uniforms"] = uniforms;
			SJSON.save(material, Path.build_filename(_source_dir.get_path(), resource_path) + ".material");

			try
			{
				file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
			}
			catch (Error e)
			{
				stderr.printf("Error: %s\n", e.message);
			}

			Hashtable texture = new Hashtable();
			texture["source"]        = resource_path_to_resource_name(resource_filename);
			texture["generate_mips"] = false;
			texture["normal_map"]    = false;
			SJSON.save(texture, Path.build_filename(_source_dir.get_path(), resource_path) + ".texture");

			Hashtable sprite = new Hashtable();
			sprite["width"]  = width;
			sprite["height"] = height;

			ArrayList<Value?> frames = new ArrayList<Value?>();
			for (int r = 0; r < num_v; ++r)
			{
				for (int c = 0; c < num_h; ++c)
				{
					Vector2 cell_xy = sprite_cell_xy(r
						, c
						, offset_x
						, offset_y
						, cell_w
						, cell_h
						, spacing_x
						, spacing_y
						);

					// Pivot is relative to the top-left corner of the cell
					int x = (int)cell_xy.x;
					int y = (int)cell_xy.y;

					Hashtable data = new Hashtable();
					data["name"]   = "sprite_%d".printf(c+num_h*r);
					data["region"] = Vector4(x, y, cell_w, cell_h).to_array();
					data["pivot"]  = Vector2(x+pivot_xy.x, y+pivot_xy.y).to_array();
					frames.add(data);
				}
			}
			sprite["frames"] = frames;

			SJSON.save(sprite, Path.build_filename(_source_dir.get_path(), resource_path) + ".sprite");

			// Generate .unit
			Database db = new Database();

			// Do not overwrite existing .unit
			string unit_name = Path.build_filename(_source_dir.get_path(), resource_path) + ".unit";
			if (File.new_for_path(unit_name).query_exists())
				db.load(unit_name);

			Unit unit = new Unit(db, GUID_ZERO, null);

			// Create transform
			{
				Guid component_id;
				if (unit.has_component(out component_id, "transform"))
				{
					unit.set_component_property_vector3   (component_id, "data.position", VECTOR3_ZERO);
					unit.set_component_property_quaternion(component_id, "data.rotation", QUATERNION_IDENTITY);
					unit.set_component_property_vector3   (component_id, "data.scale", VECTOR3_ONE);
					unit.set_component_property_string    (component_id, "type", "transform");
				}
				else
				{
					component_id = Guid.new_guid();
					db.create(component_id);
					db.set_property_vector3   (component_id, "data.position", VECTOR3_ZERO);
					db.set_property_quaternion(component_id, "data.rotation", QUATERNION_IDENTITY);
					db.set_property_vector3   (component_id, "data.scale", VECTOR3_ONE);
					db.set_property_string    (component_id, "type", "transform");

					db.add_to_set(GUID_ZERO, "components", component_id);
				}
			}

			// Create sprite_renderer
			{
				Guid component_id;
				if (unit.has_component(out component_id, "sprite_renderer"))
				{
					unit.set_component_property_string(component_id, "data.material", resource_name);
					unit.set_component_property_string(component_id, "data.sprite_resource", resource_name);
					unit.set_component_property_double(component_id, "data.layer", layer);
					unit.set_component_property_double(component_id, "data.depth", depth);
					unit.set_component_property_bool  (component_id, "data.visible", true);
					unit.set_component_property_string(component_id, "type", "sprite_renderer");
				}
				else
				{
					component_id = Guid.new_guid();
					db.create(component_id);
					db.set_property_string(component_id, "data.material", resource_name);
					db.set_property_string(component_id, "data.sprite_resource", resource_name);
					db.set_property_double(component_id, "data.layer", layer);
					db.set_property_double(component_id, "data.depth", depth);
					db.set_property_bool  (component_id, "data.visible", true);
					db.set_property_string(component_id, "type", "sprite_renderer");

					db.add_to_set(GUID_ZERO, "components", component_id);
				}
			}

			if (collision_enabled)
			{
				// Create collider
				double PIXELS_PER_METER = 32.0;
				{
					Quaternion rotation = QUATERNION_IDENTITY;

					Guid component_id;
					if (unit.has_component(out component_id, "collider"))
					{
						unit.set_component_property_string(component_id, "data.source", "inline");
						if (shape_active_name == "square_collider")
						{
							double pos_x =  (collision_x + collision_w/2.0 - pivot_xy.x) / PIXELS_PER_METER;
							double pos_y = -(collision_y + collision_h/2.0 - pivot_xy.y) / PIXELS_PER_METER;
							Vector3 position = Vector3(pos_x, 0, pos_y);
							Vector3 half_extents = Vector3(collision_w/2/PIXELS_PER_METER, 0.5/PIXELS_PER_METER, collision_h/2/PIXELS_PER_METER);
							unit.set_component_property_vector3   (component_id, "data.collider_data.position", position);
							unit.set_component_property_quaternion(component_id, "data.collider_data.rotation", rotation);
							unit.set_component_property_string    (component_id, "data.shape", "box");
							unit.set_component_property_vector3   (component_id, "data.collider_data.half_extents", half_extents);
						}
						else if (shape_active_name == "circle_collider")
						{
							double pos_x =  (circle_collision_center_x - pivot_xy.x) / PIXELS_PER_METER;
							double pos_y = -(circle_collision_center_y - pivot_xy.y) / PIXELS_PER_METER;
							Vector3 position = Vector3(pos_x, 0, pos_y);
							double radius = circle_collision_radius / PIXELS_PER_METER;
							unit.set_component_property_vector3   (component_id, "data.collider_data.position", position);
							unit.set_component_property_quaternion(component_id, "data.collider_data.rotation", rotation);
							unit.set_component_property_string    (component_id, "data.shape", "sphere");
							unit.set_component_property_double    (component_id, "data.collider_data.radius", radius);
						}
						else if (shape_active_name == "capsule_collider")
						{
							double pos_x =  (capsule_collision_center_x - pivot_xy.x) / PIXELS_PER_METER;
							double pos_y = -(capsule_collision_center_y - pivot_xy.y) / PIXELS_PER_METER;
							Vector3 position = Vector3(pos_x, 0, pos_y);
							double radius = capsule_collision_radius / PIXELS_PER_METER;
							double capsule_height = (capsule_collision_height - 2*capsule_collision_radius) / PIXELS_PER_METER;
							unit.set_component_property_vector3   (component_id, "data.collider_data.position", position);
							unit.set_component_property_quaternion(component_id, "data.collider_data.rotation", Quaternion.from_axis_angle(Vector3(0, 0, 1), (float)Math.PI/2));
							unit.set_component_property_string    (component_id, "data.shape", "capsule");
							unit.set_component_property_double    (component_id, "data.collider_data.radius", radius);
							unit.set_component_property_double    (component_id, "data.collider_data.height", capsule_height);
						}
						unit.set_component_property_string(component_id, "type", "collider");
					}
					else
					{
						component_id = Guid.new_guid();
						db.create(component_id);
						db.set_property_string(component_id, "data.source", "inline");
						if (shape_active_name == "square_collider")
						{
							double pos_x =  (collision_x + collision_w/2.0 - pivot_xy.x) / PIXELS_PER_METER;
							double pos_y = -(collision_y + collision_h/2.0 - pivot_xy.y) / PIXELS_PER_METER;
							Vector3 position = Vector3(pos_x, 0, pos_y);
							Vector3 half_extents = Vector3(collision_w/2/PIXELS_PER_METER, 0.5/PIXELS_PER_METER, collision_h/2/PIXELS_PER_METER);
							db.set_property_vector3   (component_id, "data.collider_data.position", position);
							db.set_property_string    (component_id, "data.shape", "box");
							db.set_property_quaternion(component_id, "data.collider_data.rotation", rotation);
							db.set_property_vector3   (component_id, "data.collider_data.half_extents", half_extents);
						}
						else if (shape_active_name == "circle_collider")
						{
							double pos_x =  (circle_collision_center_x - pivot_xy.x) / PIXELS_PER_METER;
							double pos_y = -(circle_collision_center_y - pivot_xy.y) / PIXELS_PER_METER;
							Vector3 position = Vector3(pos_x, 0, pos_y);
							double radius = circle_collision_radius / PIXELS_PER_METER;
							db.set_property_vector3   (component_id, "data.collider_data.position", position);
							db.set_property_string    (component_id, "data.shape", "sphere");
							db.set_property_quaternion(component_id, "data.collider_data.rotation", rotation);
							db.set_property_double    (component_id, "data.collider_data.radius", radius);
						}
						else if (shape_active_name == "capsule_collider")
						{
							double pos_x =  (capsule_collision_center_x - pivot_xy.x) / PIXELS_PER_METER;
							double pos_y = -(capsule_collision_center_y - pivot_xy.y) / PIXELS_PER_METER;
							Vector3 position = Vector3(pos_x, 0, pos_y);
							double radius = capsule_collision_radius / PIXELS_PER_METER;
							double capsule_height = (capsule_collision_height - 2*capsule_collision_radius) / PIXELS_PER_METER;
							db.set_property_vector3   (component_id, "data.collider_data.position", position);
							db.set_property_string    (component_id, "data.shape", "capsule");
							db.set_property_quaternion(component_id, "data.collider_data.rotation", Quaternion.from_axis_angle(Vector3(0, 0, 1), (float)Math.PI/2));
							db.set_property_double    (component_id, "data.collider_data.radius", radius);
							db.set_property_double    (component_id, "data.collider_data.height", capsule_height);
						}
						db.set_property_string(component_id, "type", "collider");

						db.add_to_set(GUID_ZERO, "components", component_id);
					}
				}

				// Create actor
				{
					Guid component_id;
					if (unit.has_component(out component_id, "actor"))
					{
						unit.set_component_property_string(component_id, "data.class", actor_class);
						unit.set_component_property_string(component_id, "data.collision_filter", "default");
						unit.set_component_property_bool  (component_id, "data.lock_rotation_x", true);
						unit.set_component_property_bool  (component_id, "data.lock_rotation_y", lock_rotation_y);
						unit.set_component_property_bool  (component_id, "data.lock_rotation_z", true);
						unit.set_component_property_bool  (component_id, "data.lock_translation_x", false);
						unit.set_component_property_bool  (component_id, "data.lock_translation_y", true);
						unit.set_component_property_bool  (component_id, "data.lock_translation_z", false);
						unit.set_component_property_double(component_id, "data.mass", mass);
						unit.set_component_property_string(component_id, "data.material", "default");
						unit.set_component_property_string(component_id, "type", "actor");
					}
					else
					{
						component_id = Guid.new_guid();
						db.create(component_id);
						db.set_property_string(component_id, "data.class", actor_class);
						db.set_property_string(component_id, "data.collision_filter", "default");
						db.set_property_bool  (component_id, "data.lock_rotation_x", true);
						db.set_property_bool  (component_id, "data.lock_rotation_y", lock_rotation_y);
						db.set_property_bool  (component_id, "data.lock_rotation_z", true);
						db.set_property_bool  (component_id, "data.lock_translation_x", false);
						db.set_property_bool  (component_id, "data.lock_translation_y", true);
						db.set_property_bool  (component_id, "data.lock_translation_z", false);
						db.set_property_double(component_id, "data.mass", mass);
						db.set_property_string(component_id, "data.material", "default");
						db.set_property_string(component_id, "type", "actor");

						db.add_to_set(GUID_ZERO, "components", component_id);
					}
				}
			}
			else /* if (collision_enabled) */
			{
				// Destroy collider and actor if any
				Guid component_id;
				if (unit.has_component(out component_id, "collider"))
				{
					db.remove_from_set(GUID_ZERO, "components", component_id);
					db.destroy(component_id);
				}
				if (unit.has_component(out component_id, "actor"))
				{
					db.remove_from_set(GUID_ZERO, "components", component_id);
					db.destroy(component_id);
				}
			}

			db.save(unit_name);
		}

		return 0;
	}

	public int import_meshes(SList<string> filenames, string destination_dir)
	{
		foreach (unowned string filename_i in filenames)
		{
			if (!filename_i.has_suffix(".mesh"))
				continue;

			GLib.File file_src = File.new_for_path(filename_i);
			GLib.File file_dst = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));

			string resource_filename = _source_dir.get_relative_path(file_dst);
			string resource_path     = resource_filename.substring(0, resource_filename.last_index_of_char('.'));
			string resource_name     = resource_path_to_resource_name(resource_path);

			// Choose material or create new one
			FileChooserDialog mtl = new FileChooserDialog("Select material... (Cancel to create a new one)"
				, null
				, FileChooserAction.OPEN
				, "Cancel"
				, ResponseType.CANCEL
				, "Select"
				, ResponseType.ACCEPT
				);
			mtl.set_current_folder(_source_dir.get_path());

			FileFilter fltr = new FileFilter();
			fltr.set_filter_name("Material (*.material)");
			fltr.add_pattern("*.material");
			mtl.add_filter(fltr);

			string material_path = resource_path;
			if (mtl.run() == (int)ResponseType.ACCEPT)
			{
				material_path = _source_dir.get_relative_path(File.new_for_path(mtl.get_filename()));
				material_path = material_path.substring(0, material_path.last_index_of_char('.'));
			}
			else
			{
				Hashtable material = new Hashtable();
				material["shader"]   = "mesh+DIFFUSE_MAP";
				material["textures"] = new Hashtable();
				material["uniforms"] = new Hashtable();
				SJSON.save(material, Path.build_filename(_source_dir.get_path(), material_path) + ".material");
			}
			mtl.destroy();
			string material_name = resource_path_to_resource_name(material_path);

			try
			{
				file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
			}
			catch (Error e)
			{
				stderr.printf("Error: %s\n", e.message);
			}

			// Generate .unit
			Database db = new Database();

			// Do not overwrite existing .unit
			string unit_name = Path.build_filename(_source_dir.get_path(), resource_name) + ".unit";
			if (File.new_for_path(unit_name).query_exists())
				db.load(unit_name);

			Unit unit = new Unit(db, GUID_ZERO, null);

			// Create transform
			{
				Guid component_id;
				if (unit.has_component(out component_id, "transform"))
				{
					unit.set_component_property_vector3   (component_id, "data.position", VECTOR3_ZERO);
					unit.set_component_property_quaternion(component_id, "data.rotation", QUATERNION_IDENTITY);
					unit.set_component_property_vector3   (component_id, "data.scale", VECTOR3_ONE);
					unit.set_component_property_string    (component_id, "type", "transform");
				}
				else
				{
					component_id = Guid.new_guid();
					db.create(component_id);
					db.set_property_vector3   (component_id, "data.position", VECTOR3_ZERO);
					db.set_property_quaternion(component_id, "data.rotation", QUATERNION_IDENTITY);
					db.set_property_vector3   (component_id, "data.scale", VECTOR3_ONE);
					db.set_property_string    (component_id, "type", "transform");

					db.add_to_set(GUID_ZERO, "components", component_id);
				}
			}

			// Remove all existing mesh_renderer components
			{
				Guid component_id;
				while (unit.has_component(out component_id, "mesh_renderer"))
					unit.remove_component(component_id);
			}

			Hashtable mesh = SJSON.load(filename_i);
			Hashtable mesh_nodes = (Hashtable)mesh["nodes"];
			// Create mesh_renderer
			{
				foreach (var entry in mesh_nodes.entries)
				{
					Guid component_id = Guid.new_guid();
					db.create(component_id);
					db.set_property_string(component_id, "data.geometry_name", (string)entry.key);
					db.set_property_string(component_id, "data.material", material_name);
					db.set_property_string(component_id, "data.mesh_resource", resource_name);
					db.set_property_bool  (component_id, "data.visible", true);
					db.set_property_string(component_id, "type", "mesh_renderer");

					db.add_to_set(GUID_ZERO, "components", component_id);
				}
			}

			// Create collider
			{
				Guid component_id;
				if (unit.has_component(out component_id, "collider"))
				{
					unit.set_component_property_string(component_id, "data.shape", "mesh");
					unit.set_component_property_string(component_id, "data.scene", resource_name);
					unit.set_component_property_string(component_id, "data.name", mesh_nodes.entries.to_array()[0].key);
					unit.set_component_property_string(component_id, "type", "collider");
				}
				else
				{
					component_id = Guid.new_guid();
					db.create(component_id);
					db.set_property_string(component_id, "data.shape", "mesh");
					db.set_property_string(component_id, "data.scene", resource_name);
					db.set_property_string(component_id, "data.name", mesh_nodes.entries.to_array()[0].key);
					db.set_property_string(component_id, "type", "collider");

					db.add_to_set(GUID_ZERO, "components", component_id);
				}
			}

			// Create actor
			{
				Guid component_id;
				if (unit.has_component(out component_id, "actor"))
				{
					unit.set_component_property_string(component_id, "data.class", "static");
					unit.set_component_property_string(component_id, "data.collision_filter", "default");
					unit.set_component_property_double(component_id, "data.mass", 10);
					unit.set_component_property_string(component_id, "data.material", "default");
					unit.set_component_property_string(component_id, "type", "actor");
				}
				else
				{
					component_id = Guid.new_guid();
					db.create(component_id);
					db.set_property_string(component_id, "data.class", "static");
					db.set_property_string(component_id, "data.collision_filter", "default");
					db.set_property_double(component_id, "data.mass", 10);
					db.set_property_string(component_id, "data.material", "default");
					db.set_property_string(component_id, "type", "actor");

					db.add_to_set(GUID_ZERO, "components", component_id);
				}
			}

			db.save(unit_name);
		}

		return 0;
	}

	public int import_sounds(SList<string> filenames, string destination_dir)
	{
		foreach (unowned string filename_i in filenames)
		{
			if (!filename_i.has_suffix(".wav"))
				continue;

			GLib.File file_src = File.new_for_path(filename_i);
			GLib.File file_dst = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));

			string resource_filename = _source_dir.get_relative_path(file_dst);
			string resource_path     = resource_filename.substring(0, resource_filename.last_index_of_char('.'));

			try
			{
				file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
			}
			catch (Error e)
			{
				stderr.printf("Error: %s\n", e.message);
			}

			Hashtable sound = new Hashtable();
			sound["source"] = resource_path_to_resource_name(resource_filename);

			SJSON.save(sound, Path.build_filename(_source_dir.get_path(), resource_path) + ".sound");
		}

		return 0;
	}

	public int import_textures(SList<string> filenames, string destination_dir)
	{
		foreach (unowned string filename_i in filenames)
		{
			if (!filename_i.has_suffix(".png"))
				continue;

			GLib.File file_src = File.new_for_path(filename_i);
			GLib.File file_dst = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));

			string resource_filename = _source_dir.get_relative_path(file_dst);
			string resource_path     = resource_filename.substring(0, resource_filename.last_index_of_char('.'));

			try
			{
				file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
			}
			catch (Error e)
			{
				stderr.printf("Error: %s\n", e.message);
			}

			Hashtable texture = new Hashtable();
			texture["source"]        = resource_path_to_resource_name(resource_filename);
			texture["generate_mips"] = true;
			texture["normal_map"]    = false;

			SJSON.save(texture, Path.build_filename(_source_dir.get_path(), resource_path) + ".texture");
		}

		return 0;
	}

	public void import(string? destination_dir, Gtk.Window? parent_window = null)
	{
		Gtk.FileFilter sprite_filter = new Gtk.FileFilter();
		sprite_filter.set_filter_name("Sprite (*.png)");
		sprite_filter.add_pattern("*.png");

		Gtk.FileFilter mesh_filter = new Gtk.FileFilter();
		mesh_filter.set_filter_name("Mesh (*.mesh)");
		mesh_filter.add_pattern("*.mesh");

		Gtk.FileFilter sound_filter = new Gtk.FileFilter();
		sound_filter.set_filter_name("Sound (*.wav)");
		sound_filter.add_pattern("*.wav");

		Gtk.FileFilter texture_filter = new Gtk.FileFilter();
		texture_filter.set_filter_name("Texture (*.png, *.tga, *.dds, *.ktx, *.pvr)");
		texture_filter.add_pattern("*.png");
		texture_filter.add_pattern("*.tga");
		texture_filter.add_pattern("*.dds");
		texture_filter.add_pattern("*.ktx");
		texture_filter.add_pattern("*.pvr");

		Gtk.FileChooserDialog src = new Gtk.FileChooserDialog("Import..."
			, parent_window
			, FileChooserAction.OPEN
			, "Cancel"
			, ResponseType.CANCEL
			, "Open"
			, ResponseType.ACCEPT
			);
		src.select_multiple = true;
		src.add_filter(sprite_filter);
		src.add_filter(mesh_filter);
		src.add_filter(sound_filter);
		src.add_filter(texture_filter);

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
			out_dir = destination_dir;
		}

		Gtk.FileFilter? current_filter = src.get_filter();
		GLib.SList<string> filenames = src.get_filenames();

		int success = 1;
		if (current_filter != null)
		{
			if (current_filter == sprite_filter)
				success = this.import_sprites(filenames, out_dir);
			else if (current_filter == mesh_filter)
				success = this.import_meshes(filenames, out_dir);
			else if (current_filter == sound_filter)
				success = this.import_sounds(filenames, out_dir);
			else if (current_filter == texture_filter)
				success = this.import_textures(filenames, out_dir);
		}

		if (success == 0)
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
