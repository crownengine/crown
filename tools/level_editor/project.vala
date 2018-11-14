/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;
using Gee;

namespace Crown
{
	public class Project
	{
		// Data
		private File _source_dir;
		private File _toolchain_dir;
		private File _data_dir;
		private File _level_editor_test;
		private string _platform;

		private Database _files;
		private HashMap<string, Guid?> _map;

		public signal void changed();

		public Project()
		{
			_source_dir = null;
			_toolchain_dir = null;
			_data_dir = null;
			_level_editor_test = null;
#if CROWN_PLATFORM_LINUX
			_platform = "linux";
#elif CROWN_PLATFORM_WINDOWS
			_platform = "windows";
#endif // CROWN_PLATFORM_LINUX
			_files = new Database();
			_map = new HashMap<string, Guid?>();
		}

		public void reset()
		{
			_files.reset();
			_map.clear();
		}

		public void load(string source_dir, string toolchain_dir)
		{
			reset();
			_source_dir        = File.new_for_path(source_dir);
			_toolchain_dir     = File.new_for_path(toolchain_dir);
			_data_dir          = File.new_for_path(_source_dir.get_path() + "_" + _platform);
			_level_editor_test = File.new_for_path(_source_dir.get_path() + "/" + "_level_editor_test.level");
		}

		public void create_initial_files()
		{
			// Write boot.config
			{
				string text = """// Lua script to launch on boot
boot_script = "core/game/boot"

// Package to load on boot
boot_package = "boot"

window_title = "New Poject"

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
				FileStream fs = FileStream.open(path, "w");
				GLib.assert(fs != null);
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
""";
				string path = Path.build_filename(_source_dir.get_path(), "boot.package");
				FileStream fs = FileStream.open(path, "w");
				GLib.assert(fs != null);
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
				FileStream fs = FileStream.open(path, "w");
				GLib.assert(fs != null);
				fs.write(text.data);
			}

			// Write main.lua
			{
				string text = """require "core/game/camera"

Game = Game or {}

Game = {
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
				FileStream fs = FileStream.open(path, "w");
				GLib.assert(fs != null);
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

		public string level_editor_test_level()
		{
			return _level_editor_test.get_path();
		}

		public void delete_level_editor_test_level()
		{
			try
			{
				_level_editor_test.delete();
			}
			catch (GLib.Error e)
			{
				stderr.printf("%s\n", e.message);
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

			changed();
		}

		public void remove_file(string path)
		{
			Guid id = _map[path];
			_files.remove_from_set(GUID_ZERO, "data", id);
			_files.destroy(id);

			_map.unset(path);

			changed();
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

			bool collision_enabled = sid.collision_enabled.active;
			int collision_x = (int)sid.collision_x.value;
			int collision_y = (int)sid.collision_y.value;
			int collision_w = (int)sid.collision_w.value;
			int collision_h = (int)sid.collision_h.value;

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
					// Create collider (geometry)
					{
						// d-----a
						// |     |
						// |     |
						// c-----b
						Vector2 a = {};
						Vector2 b = {};
						Vector2 c = {};
						Vector2 d = {};

						double PIXELS_PER_METER = 32.0;

						a.x = (collision_w + collision_x - pivot_xy.x) / PIXELS_PER_METER;
						a.y = (              collision_y - pivot_xy.y) / PIXELS_PER_METER;

						b.x = (collision_w + collision_x - pivot_xy.x) / PIXELS_PER_METER;
						b.y = (collision_h + collision_y - pivot_xy.y) / PIXELS_PER_METER;

						c.x = (              collision_x - pivot_xy.x) / PIXELS_PER_METER;
						c.y = (collision_h + collision_y - pivot_xy.y) / PIXELS_PER_METER;

						d.x = (              collision_x - pivot_xy.x) / PIXELS_PER_METER;
						d.y = (              collision_y - pivot_xy.y) / PIXELS_PER_METER;

						// Invert Y axis
						a.y = a.y == 0.0f ? a.y : -a.y;
						b.y = b.y == 0.0f ? b.y : -b.y;
						c.y = c.y == 0.0f ? c.y : -c.y;
						d.y = d.y == 0.0f ? d.y : -d.y;

						ArrayList<Value?> position = new ArrayList<Value?>();
						// Up face
						position.add(a.x); position.add( 0.25); position.add(a.y);
						position.add(b.x); position.add( 0.25); position.add(b.y);
						position.add(c.x); position.add( 0.25); position.add(c.y);
						position.add(d.x); position.add( 0.25); position.add(d.y);
						// Bottom face
						position.add(a.x); position.add(-0.25); position.add(a.y);
						position.add(b.x); position.add(-0.25); position.add(b.y);
						position.add(c.x); position.add(-0.25); position.add(c.y);
						position.add(d.x); position.add(-0.25); position.add(d.y);

						ArrayList<Value?> indices_data = new ArrayList<Value?>();
						indices_data.add(0); indices_data.add(2); indices_data.add(3);
						indices_data.add(7); indices_data.add(5); indices_data.add(4);
						indices_data.add(4); indices_data.add(1); indices_data.add(0);
						indices_data.add(5); indices_data.add(2); indices_data.add(1);
						indices_data.add(6); indices_data.add(3); indices_data.add(2);
						indices_data.add(0); indices_data.add(7); indices_data.add(4);
						indices_data.add(0); indices_data.add(1); indices_data.add(2);
						indices_data.add(7); indices_data.add(6); indices_data.add(5);
						indices_data.add(4); indices_data.add(5); indices_data.add(1);
						indices_data.add(5); indices_data.add(6); indices_data.add(2);
						indices_data.add(6); indices_data.add(7); indices_data.add(3);
						indices_data.add(0); indices_data.add(3); indices_data.add(7);

						ArrayList<Value?> data = new ArrayList<Value?>();
						data.add(indices_data);

						Hashtable indices = new Hashtable();
						indices["size"] = indices_data.size;
						indices["data"] = data;

						Hashtable geometries_collider = new Hashtable();
						geometries_collider["position"] = position;
						geometries_collider["indices"] = indices;

						Hashtable geometries = new Hashtable();
						geometries["collider"] = geometries_collider;

						ArrayList<Value?> matrix_local = new ArrayList<Value?>();
						matrix_local.add(1.0); matrix_local.add(0.0); matrix_local.add(0.0); matrix_local.add(0.0);
						matrix_local.add(0.0); matrix_local.add(1.0); matrix_local.add(0.0); matrix_local.add(0.0);
						matrix_local.add(0.0); matrix_local.add(0.0); matrix_local.add(1.0); matrix_local.add(0.0);
						matrix_local.add(0.0); matrix_local.add(0.0); matrix_local.add(0.0); matrix_local.add(1.0);

						Hashtable nodes_collider = new Hashtable();
						nodes_collider["matrix_local"] = matrix_local;

						Hashtable nodes = new Hashtable();
						nodes["collider"] = nodes_collider;

						Hashtable mesh = new Hashtable();
						mesh["geometries"] = geometries;
						mesh["nodes"] = nodes;

						SJSON.save(mesh, Path.build_filename(_source_dir.get_path(), resource_name) + ".mesh");
					}

					// Create collider
					{
						Guid id = Guid.new_guid();

						if (!unit.has_component("collider", ref id))
						{
							db.create(id);
							db.set_property_string(id, "data.shape", "box");
							db.set_property_string(id, "data.scene", resource_name);
							db.set_property_string(id, "data.name", "collider");
							db.set_property_string(id, "data.material", "default");
							db.set_property_string(id, "type", "collider");

							db.add_to_set(GUID_ZERO, "components", id);
						}
						else
						{
							unit.set_component_property_string(id, "data.shape", "box");
							unit.set_component_property_string(id, "data.scene", resource_name);
							unit.set_component_property_string(id, "data.name", "collider");
							unit.set_component_property_string(id, "data.material", "default");
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
							db.set_property_bool  (id, "data.lock_rotation_x", true);
							db.set_property_bool  (id, "data.lock_rotation_y", true);
							db.set_property_bool  (id, "data.lock_rotation_z", true);
							db.set_property_bool  (id, "data.lock_translation_x", false);
							db.set_property_bool  (id, "data.lock_translation_y", true);
							db.set_property_bool  (id, "data.lock_translation_z", false);
							db.set_property_double(id, "data.mass", 10);
							db.set_property_string(id, "data.material", "default");
							db.set_property_string(id, "type", "actor");

							db.add_to_set(GUID_ZERO, "components", id);
						}
						else
						{
							unit.set_component_property_string(id, "data.class", "static");
							unit.set_component_property_string(id, "data.collision_filter", "default");
							unit.set_component_property_bool  (id, "data.lock_rotation_x", true);
							unit.set_component_property_bool  (id, "data.lock_rotation_y", true);
							unit.set_component_property_bool  (id, "data.lock_rotation_z", true);
							unit.set_component_property_bool  (id, "data.lock_translation_x", false);
							unit.set_component_property_bool  (id, "data.lock_translation_y", true);
							unit.set_component_property_bool  (id, "data.lock_translation_z", false);
							unit.set_component_property_double(id, "data.mass", 10);
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
						db.set_property_string(id, "data.material", "default");
						db.set_property_string(id, "type", "collider");

						db.add_to_set(GUID_ZERO, "components", id);
					}
					else
					{
						unit.set_component_property_string(id, "data.shape", "mesh");
						unit.set_component_property_string(id, "data.scene", resource_name);
						unit.set_component_property_string(id, "data.name", mesh_nodes.entries.to_array()[0].key);
						unit.set_component_property_string(id, "data.material", "default");
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
	}
}
