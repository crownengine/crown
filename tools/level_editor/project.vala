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

		public Project(DataCompiler dc)
		{
			_source_dir = null;
			_toolchain_dir = null;
			_data_dir = null;
			_level_editor_test_level = null;
			_level_editor_test_package = null;
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

		public void load(string source_dir, string toolchain_dir)
		{
			reset();

			_source_dir    = File.new_for_path(source_dir);
			_toolchain_dir = File.new_for_path(toolchain_dir);
			_data_dir      = File.new_for_path(_source_dir.get_path() + "_" + _platform);

			_level_editor_test_level = File.new_for_path(_source_dir.get_path() + "/" + "_level_editor_test.level");
			_level_editor_test_package = File.new_for_path(_source_dir.get_path() + "/" + "_level_editor_test.package");

			// Cleanup source directory from previous runs' garbage
			delete_garbage();
		}

		public void create_initial_files()
		{
			// Write boot.config
			{
				string text = """// Lua script to launch on boot
boot_script = "core/game/boot"

// Package to load on boot
boot_package = "boot"

window_title = "New Project"

// Linux-only configs
linux = {
	renderer = {
		resolution = [ 1280 720 ]
	}
}

// Windows-only configs
windows = {
	renderer = {
		resolution = [ 1280 720 ]
	}
}
""";
				string path = Path.build_filename(_source_dir.get_path(), "boot.config");
				FileStream fs = FileStream.open(path, "wb");
				if (fs != null)
					fs.write(text.data);
			}

			// Write boot.package
			{
				string text = """lua = [
	"core/game/boot"
	"core/game/camera"
	"core/game/game"
	"core/lua/class"
	"main"
]
shader = [
	"core/shaders/common"
	"core/shaders/default"
]
physics_config = [
	"global"
]
unit = [
	"core/units/camera"
]
""";
				string path = Path.build_filename(_source_dir.get_path(), "boot.package");
				FileStream fs = FileStream.open(path, "wb");
				if (fs != null)
					fs.write(text.data);
			}

			// Write global.physics_config
			{
				string text = """materials = {
	default = { friction = 0.8 rolling_friction = 0.5 restitution = 0.81 }
}

collision_filters = {
	no_collision = { collides_with = [] }
	default = { collides_with = [ "default" ] }
}

actors = {
	static = { dynamic = false }
	dynamic = { dynamic = true }
	keyframed = { dynamic = true kinematic = true disable_gravity = true }
}
""";
				string path = Path.build_filename(_source_dir.get_path(), "global.physics_config");
				FileStream fs = FileStream.open(path, "wb");
				if (fs != null)
					fs.write(text.data);
			}

			// Write main.lua
			{
				string text = """require "core/game/camera"

Game = Game or {
	sg = nil,
	pw = nil,
	rw = nil,
	camera = nil,
}

GameBase.game = Game
GameBase.game_level = nil

function Game.level_loaded()
	Device.enable_resource_autoload(true)

	Game.sg = World.scene_graph(GameBase.world)
	Game.pw = World.physics_world(GameBase.world)
	Game.rw = World.render_world(GameBase.world)

	-- Spawn camera
	local camera_unit = World.spawn_unit(GameBase.world, "core/units/camera")
	SceneGraph.set_local_position(Game.sg, camera_unit, Vector3(0, 6.5, -30))
	GameBase.game_camera = camera_unit
	Game.camera = FPSCamera(GameBase.world, camera_unit)
end

function Game.update(dt)
	-- Stop the engine when the 'ESC' key is released
	if Keyboard.released(Keyboard.button_id("escape")) then
		Device.quit()
	end

	-- Update camera
	local delta = Vector3.zero()
	if Mouse.pressed(Mouse.button_id("right")) then move = true end
	if Mouse.released(Mouse.button_id("right")) then move = false end
	if move then delta = Mouse.axis(Mouse.axis_id("cursor_delta")) end
	Game.camera:update(dt, delta.x, delta.y)
end

function Game.render(dt)
end

function Game.shutdown()
end
""";
				string path = Path.build_filename(_source_dir.get_path(), "main.lua");
				FileStream fs = FileStream.open(path, "wb");
				if (fs != null)
					fs.write(text.data);
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

		public void add_file(string path)
		{
			string name = path.substring(0, path.last_index_of("."));
			string type = path.substring(path.last_index_of(".") + 1);

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

		public void import_sprites(SList<string> filenames, string destination_dir)
		{
			Hashtable importer_settings = null;
			string importer_settings_path = null;
			{
				GLib.File file_src = File.new_for_path(filenames.nth_data(0));
				GLib.File file_dst = File.new_for_path(destination_dir + "/" + file_src.get_basename());

				string resource_filename = _source_dir.get_relative_path(file_dst);
				string resource_name     = resource_filename.substring(0, resource_filename.last_index_of_char('.'));

				importer_settings_path = Path.build_filename(_source_dir.get_path(), resource_name) + ".importer_settings";
			}

			SpriteImportDialog sid = new SpriteImportDialog(filenames.nth_data(0));

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
				return;
			}

			sid.save(importer_settings);

			int width     = (int)sid._pixbuf.width;
			int height    = (int)sid._pixbuf.height;
			int num_h     = (int)sid.cells_h.value;
			int num_v     = (int)sid.cells_v.value;
			int cell_w    = (int)sid.cell_w.value;
			int cell_h    = (int)sid.cell_h.value;
			int offset_x  = (int)sid.offset_x.value;
			int offset_y  = (int)sid.offset_y.value;
			int spacing_x = (int)sid.spacing_x.value;
			int spacing_y = (int)sid.spacing_y.value;
			double layer  = sid.layer.value;
			double depth  = sid.depth.value;

			Vector2 pivot_xy = sprite_cell_pivot_xy(cell_w, cell_h, sid.pivot.active);

			bool collision_enabled         = sid.collision_enabled.active;
			string shape_active_name       = (string)sid.shape.visible_child_name;
			int circle_collision_center_x  = (int)sid.circle_collision_center_x.value;
			int circle_collision_center_y  = (int)sid.circle_collision_center_y.value;
			int circle_collision_radius    = (int)sid.circle_collision_radius.value;
			int capsule_collision_center_x = (int)sid.capsule_collision_center_x.value;
			int capsule_collision_center_y = (int)sid.capsule_collision_center_y.value;
			int capsule_collision_radius   = (int)sid.capsule_collision_radius.value;
			int capsule_collision_height   = (int)sid.capsule_collision_height.value;
			int collision_x                = (int)sid.collision_x.value;
			int collision_y                = (int)sid.collision_y.value;
			int collision_w                = (int)sid.collision_w.value;
			int collision_h                = (int)sid.collision_h.value;
			string actor_class             = (string)sid.actor_class.value;
			bool lock_rotation_y           = sid.lock_rotation_y.active;
			double mass                    = (double)sid.mass.value;

			sid.destroy();

			foreach (unowned string filename_i in filenames)
			{
				if (!filename_i.has_suffix(".png"))
					continue;

				GLib.File file_src = File.new_for_path(filename_i);
				GLib.File file_dst = File.new_for_path(destination_dir + "/" + file_src.get_basename());

				string resource_filename = _source_dir.get_relative_path(file_dst);
				string resource_name     = resource_filename.substring(0, resource_filename.last_index_of_char('.'));

				SJSON.save(importer_settings, Path.build_filename(_source_dir.get_path(), resource_name) + ".importer_settings");

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
				SJSON.save(material, Path.build_filename(_source_dir.get_path(), resource_name) + ".material");

				try
				{
					file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
				}
				catch (Error e)
				{
					stderr.printf("Error: %s\n", e.message);
				}

				Hashtable texture = new Hashtable();
				texture["source"]        = resource_filename;
				texture["generate_mips"] = false;
				texture["normal_map"]    = false;
				SJSON.save(texture, Path.build_filename(_source_dir.get_path(), resource_name) + ".texture");

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

				SJSON.save(sprite, Path.build_filename(_source_dir.get_path(), resource_name) + ".sprite");


				// Generate .unit
				Database db = new Database();

				// Do not overwrite existing .unit
				string unit_name = Path.build_filename(_source_dir.get_path(), resource_name) + ".unit";
				if (File.new_for_path(unit_name).query_exists())
					db.load(unit_name);

				Unit unit = new Unit(db, GUID_ZERO, null);

				// Create transform
				{
					Guid id = Guid.new_guid();

					if (!unit.has_component("transform", ref id))
					{
						db.create(id);
						db.set_property_vector3   (id, "data.position", VECTOR3_ZERO);
						db.set_property_quaternion(id, "data.rotation", QUATERNION_IDENTITY);
						db.set_property_vector3   (id, "data.scale", VECTOR3_ONE);
						db.set_property_string    (id, "type", "transform");

						db.add_to_set(GUID_ZERO, "components", id);
					}
					else
					{
						unit.set_component_property_vector3   (id, "data.position", VECTOR3_ZERO);
						unit.set_component_property_quaternion(id, "data.rotation", QUATERNION_IDENTITY);
						unit.set_component_property_vector3   (id, "data.scale", VECTOR3_ONE);
						unit.set_component_property_string    (id, "type", "transform");
					}
				}

				// Create sprite_renderer
				{
					Guid id = Guid.new_guid();

					if (!unit.has_component("sprite_renderer", ref id))
					{
						db.create(id);
						db.set_property_string(id, "data.material", resource_name);
						db.set_property_string(id, "data.sprite_resource", resource_name);
						db.set_property_double(id, "data.layer", layer);
						db.set_property_double(id, "data.depth", depth);
						db.set_property_bool  (id, "data.visible", true);
						db.set_property_string(id, "type", "sprite_renderer");

						db.add_to_set(GUID_ZERO, "components", id);
					}
					else
					{
						unit.set_component_property_string(id, "data.material", resource_name);
						unit.set_component_property_string(id, "data.sprite_resource", resource_name);
						unit.set_component_property_double(id, "data.layer", layer);
						unit.set_component_property_double(id, "data.depth", depth);
						unit.set_component_property_bool  (id, "data.visible", true);
						unit.set_component_property_string(id, "type", "sprite_renderer");
					}
				}

				if (collision_enabled)
				{
					// Create collider
					double PIXELS_PER_METER = 32.0;
					{
						Guid id = Guid.new_guid();
						Quaternion rotation = QUATERNION_IDENTITY;

						if (!unit.has_component("collider", ref id))
						{
							db.create(id);
							db.set_property_string(id, "data.source", "inline");
							if (shape_active_name == "square_collider")
							{
								double pos_x =  (collision_x + collision_w/2.0 - pivot_xy.x) / PIXELS_PER_METER;
								double pos_y = -(collision_y + collision_h/2.0 - pivot_xy.y) / PIXELS_PER_METER;
								Vector3 position = Vector3(pos_x, 0, pos_y);
								Vector3 half_extents = Vector3(collision_w/2/PIXELS_PER_METER, 0.5/PIXELS_PER_METER, collision_h/2/PIXELS_PER_METER);
								db.set_property_vector3   (id, "data.collider_data.position", position);
								db.set_property_string    (id, "data.shape", "box");
								db.set_property_quaternion(id, "data.collider_data.rotation", rotation);
								db.set_property_vector3   (id, "data.collider_data.half_extents", half_extents);
							}
							else if (shape_active_name == "circle_collider")
							{
								double pos_x =  (circle_collision_center_x - pivot_xy.x) / PIXELS_PER_METER;
								double pos_y = -(circle_collision_center_y - pivot_xy.y) / PIXELS_PER_METER;
								Vector3 position = Vector3(pos_x, 0, pos_y);
								double radius = circle_collision_radius / PIXELS_PER_METER;
								db.set_property_vector3   (id, "data.collider_data.position", position);
								db.set_property_string    (id, "data.shape", "sphere");
								db.set_property_quaternion(id, "data.collider_data.rotation", rotation);
								db.set_property_double    (id, "data.collider_data.radius", radius);
							}
							else if (shape_active_name == "capsule_collider")
							{
								double pos_x =  (capsule_collision_center_x - pivot_xy.x) / PIXELS_PER_METER;
								double pos_y = -(capsule_collision_center_y - pivot_xy.y) / PIXELS_PER_METER;
								Vector3 position = Vector3(pos_x, 0, pos_y);
								double radius = capsule_collision_radius / PIXELS_PER_METER;
								double capsule_height = (capsule_collision_height - 2*capsule_collision_radius) / PIXELS_PER_METER;
								db.set_property_vector3   (id, "data.collider_data.position", position);
								db.set_property_string    (id, "data.shape", "capsule");
								db.set_property_quaternion(id, "data.collider_data.rotation", Quaternion.from_axis_angle(Vector3(0, 0, 1), (float)Math.PI/2));
								db.set_property_double    (id, "data.collider_data.radius", radius);
								db.set_property_double    (id, "data.collider_data.height", capsule_height);
							}
							db.set_property_string(id, "type", "collider");

							db.add_to_set(GUID_ZERO, "components", id);
						}
						else
						{
							unit.set_component_property_string(id, "data.source", "inline");
							if (shape_active_name == "square_collider")
							{
								double pos_x =  (collision_x + collision_w/2.0 - pivot_xy.x) / PIXELS_PER_METER;
								double pos_y = -(collision_y + collision_h/2.0 - pivot_xy.y) / PIXELS_PER_METER;
								Vector3 position = Vector3(pos_x, 0, pos_y);
								Vector3 half_extents = Vector3(collision_w/2/PIXELS_PER_METER, 0.5/PIXELS_PER_METER, collision_h/2/PIXELS_PER_METER);
								unit.set_component_property_vector3   (id, "data.collider_data.position", position);
								unit.set_component_property_quaternion(id, "data.collider_data.rotation", rotation);
								unit.set_component_property_string    (id, "data.shape", "box");
								unit.set_component_property_vector3   (id, "data.collider_data.half_extents", half_extents);
							}
							else if (shape_active_name == "circle_collider")
							{
								double pos_x =  (circle_collision_center_x - pivot_xy.x) / PIXELS_PER_METER;
								double pos_y = -(circle_collision_center_y - pivot_xy.y) / PIXELS_PER_METER;
								Vector3 position = Vector3(pos_x, 0, pos_y);
								double radius = circle_collision_radius / PIXELS_PER_METER;
								unit.set_component_property_vector3   (id, "data.collider_data.position", position);
								unit.set_component_property_quaternion(id, "data.collider_data.rotation", rotation);
								unit.set_component_property_string    (id, "data.shape", "sphere");
								unit.set_component_property_double    (id, "data.collider_data.radius", radius);
							}
							else if (shape_active_name == "capsule_collider")
							{
								double pos_x =  (capsule_collision_center_x - pivot_xy.x) / PIXELS_PER_METER;
								double pos_y = -(capsule_collision_center_y - pivot_xy.y) / PIXELS_PER_METER;
								Vector3 position = Vector3(pos_x, 0, pos_y);
								double radius = capsule_collision_radius / PIXELS_PER_METER;
								double capsule_height = (capsule_collision_height - 2*capsule_collision_radius) / PIXELS_PER_METER;
								unit.set_component_property_vector3   (id, "data.collider_data.position", position);
								unit.set_component_property_quaternion(id, "data.collider_data.rotation", Quaternion.from_axis_angle(Vector3(0, 0, 1), (float)Math.PI/2));
								unit.set_component_property_string    (id, "data.shape", "capsule");
								unit.set_component_property_double    (id, "data.collider_data.radius", radius);
								unit.set_component_property_double    (id, "data.collider_data.height", capsule_height);
							}
							unit.set_component_property_string(id, "type", "collider");
						}
					}

					// Create actor
					{
						Guid id = Guid.new_guid();

						if (!unit.has_component("actor", ref id))
						{
							db.create(id);
							db.set_property_string(id, "data.class", actor_class);
							db.set_property_string(id, "data.collision_filter", "default");
							db.set_property_bool  (id, "data.lock_rotation_x", true);
							db.set_property_bool  (id, "data.lock_rotation_y", lock_rotation_y);
							db.set_property_bool  (id, "data.lock_rotation_z", true);
							db.set_property_bool  (id, "data.lock_translation_x", false);
							db.set_property_bool  (id, "data.lock_translation_y", true);
							db.set_property_bool  (id, "data.lock_translation_z", false);
							db.set_property_double(id, "data.mass", mass);
							db.set_property_string(id, "data.material", "default");
							db.set_property_string(id, "type", "actor");

							db.add_to_set(GUID_ZERO, "components", id);
						}
						else
						{
							unit.set_component_property_string(id, "data.class", actor_class);
							unit.set_component_property_string(id, "data.collision_filter", "default");
							unit.set_component_property_bool  (id, "data.lock_rotation_x", true);
							unit.set_component_property_bool  (id, "data.lock_rotation_y", lock_rotation_y);
							unit.set_component_property_bool  (id, "data.lock_rotation_z", true);
							unit.set_component_property_bool  (id, "data.lock_translation_x", false);
							unit.set_component_property_bool  (id, "data.lock_translation_y", true);
							unit.set_component_property_bool  (id, "data.lock_translation_z", false);
							unit.set_component_property_double(id, "data.mass", mass);
							unit.set_component_property_string(id, "data.material", "default");
							unit.set_component_property_string(id, "type", "actor");
						}
					}
				}

				db.save(unit_name);
			}
		}

		public void import_meshes(SList<string> filenames, string destination_dir)
		{
			foreach (unowned string filename_i in filenames)
			{
				if (!filename_i.has_suffix(".mesh"))
					continue;

				GLib.File file_src = File.new_for_path(filename_i);
				GLib.File file_dst = File.new_for_path(destination_dir + "/" + file_src.get_basename());

				string resource_filename = _source_dir.get_relative_path(file_dst);
				string resource_name     = resource_filename.substring(0, resource_filename.last_index_of_char('.'));

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

				string material_name = resource_name;
				if (mtl.run() == (int)ResponseType.ACCEPT)
				{
					material_name = _source_dir.get_relative_path(File.new_for_path(mtl.get_filename()));
					material_name = material_name.substring(0, material_name.last_index_of_char('.'));
				}
				else
				{
					Hashtable material = new Hashtable();
					material["shader"]   = "mesh+DIFFUSE_MAP";
					material["textures"] = new Hashtable();
					material["uniforms"] = new Hashtable();
					SJSON.save(material, Path.build_filename(_source_dir.get_path(), resource_name) + ".material");
				}
				mtl.destroy();

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
					Guid id = Guid.new_guid();

					if (!unit.has_component("transform", ref id))
					{
						db.create(id);
						db.set_property_vector3   (id, "data.position", VECTOR3_ZERO);
						db.set_property_quaternion(id, "data.rotation", QUATERNION_IDENTITY);
						db.set_property_vector3   (id, "data.scale", VECTOR3_ONE);
						db.set_property_string    (id, "type", "transform");

						db.add_to_set(GUID_ZERO, "components", id);
					}
					else
					{
						unit.set_component_property_vector3   (id, "data.position", VECTOR3_ZERO);
						unit.set_component_property_quaternion(id, "data.rotation", QUATERNION_IDENTITY);
						unit.set_component_property_vector3   (id, "data.scale", VECTOR3_ONE);
						unit.set_component_property_string    (id, "type", "transform");
					}
				}

				// Remove all existing mesh_renderer components
				{
					Guid id = GUID_ZERO;
					while (unit.has_component("mesh_renderer", ref id))
						unit.remove_component(id);
				}

				Hashtable mesh = SJSON.load(filename_i);
				Hashtable mesh_nodes = (Hashtable)mesh["nodes"];
				// Create mesh_renderer
				{
					foreach (var entry in mesh_nodes.entries)
					{
						string node_name = (string)entry.key;

						Guid id = Guid.new_guid();
						db.create(id);
						db.set_property_string(id, "data.geometry_name", node_name);
						db.set_property_string(id, "data.material", material_name);
						db.set_property_string(id, "data.mesh_resource", resource_name);
						db.set_property_bool  (id, "data.visible", true);
						db.set_property_string(id, "type", "mesh_renderer");

						db.add_to_set(GUID_ZERO, "components", id);
					}
				}

				// Create collider
				{
					Guid id = Guid.new_guid();

					if (!unit.has_component("collider", ref id))
					{
						db.create(id);
						db.set_property_string(id, "data.shape", "mesh");
						db.set_property_string(id, "data.scene", resource_name);
						db.set_property_string(id, "data.name", mesh_nodes.entries.to_array()[0].key);
						db.set_property_string(id, "type", "collider");

						db.add_to_set(GUID_ZERO, "components", id);
					}
					else
					{
						unit.set_component_property_string(id, "data.shape", "mesh");
						unit.set_component_property_string(id, "data.scene", resource_name);
						unit.set_component_property_string(id, "data.name", mesh_nodes.entries.to_array()[0].key);
						unit.set_component_property_string(id, "type", "collider");
					}
				}

				// Create actor
				{
					Guid id = Guid.new_guid();

					if (!unit.has_component("actor", ref id))
					{
						db.create(id);
						db.set_property_string(id, "data.class", "static");
						db.set_property_string(id, "data.collision_filter", "default");
						db.set_property_double(id, "data.mass", 10);
						db.set_property_string(id, "data.material", "default");
						db.set_property_string(id, "type", "actor");

						db.add_to_set(GUID_ZERO, "components", id);
					}
					else
					{
						unit.set_component_property_string(id, "data.class", "static");
						unit.set_component_property_string(id, "data.collision_filter", "default");
						unit.set_component_property_double(id, "data.mass", 10);
						unit.set_component_property_string(id, "data.material", "default");
						unit.set_component_property_string(id, "type", "actor");
					}
				}

				db.save(unit_name);
			}
		}

		public void import_sounds(SList<string> filenames, string destination_dir)
		{
			foreach (unowned string filename_i in filenames)
			{
				if (!filename_i.has_suffix(".wav"))
					continue;

				GLib.File file_src = File.new_for_path(filename_i);
				GLib.File file_dst = File.new_for_path(destination_dir + "/" + file_src.get_basename());

				string resource_filename = _source_dir.get_relative_path(file_dst);
				string resource_name     = resource_filename.substring(0, resource_filename.last_index_of_char('.'));

				try
				{
					file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
				}
				catch (Error e)
				{
					stderr.printf("Error: %s\n", e.message);
				}

				Hashtable sound = new Hashtable();
				sound["source"] = resource_filename;

				SJSON.save(sound, Path.build_filename(_source_dir.get_path(), resource_name) + ".sound");
			}
		}

		public void import_textures(SList<string> filenames, string destination_dir)
		{
			foreach (unowned string filename_i in filenames)
			{
				if (!filename_i.has_suffix(".png"))
					continue;

				GLib.File file_src = File.new_for_path(filename_i);
				GLib.File file_dst = File.new_for_path(destination_dir + "/" + file_src.get_basename());

				string resource_filename = _source_dir.get_relative_path(file_dst);
				string resource_name     = resource_filename.substring(0, resource_filename.last_index_of_char('.'));

				try
				{
					file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
				}
				catch (Error e)
				{
					stderr.printf("Error: %s\n", e.message);
				}

				Hashtable texture = new Hashtable();
				texture["source"]        = resource_filename;
				texture["generate_mips"] = true;
				texture["normal_map"]    = false;

				SJSON.save(texture, Path.build_filename(_source_dir.get_path(), resource_name) + ".texture");
			}
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

			if (current_filter != null)
			{
				if (current_filter == sprite_filter)
					this.import_sprites(filenames, out_dir);
				else if (current_filter == mesh_filter)
					this.import_meshes(filenames, out_dir);
				else if (current_filter == sound_filter)
					this.import_sounds(filenames, out_dir);
				else if (current_filter == texture_filter)
					this.import_textures(filenames, out_dir);
			}

			_data_compiler.compile.begin(this.data_dir(), this.platform(), (obj, res) => {
				_data_compiler.compile.end(res);
			});

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
