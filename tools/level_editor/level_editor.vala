/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gdk; // Pixbuf
using Gee;
using Gtk;

namespace Crown
{
	public enum ToolType
	{
		PLACE,
		MOVE,
		ROTATE,
		SCALE
	}

	public enum SnapMode
	{
		RELATIVE,
		ABSOLUTE
	}

	public enum ReferenceSystem
	{
		LOCAL,
		WORLD
	}

	public enum PlaceableType
	{
		UNIT,
		SOUND
	}

	public class LevelEditor : Gtk.Window
	{
		// Project paths
		private string _source_dir;
		private string _toolchain_dir;
		private string _data_dir;
		private string _platform;

		// Editor state
		private double _grid_size;
		private double _rotation_snap;
		private bool _show_grid;
		private bool _snap_to_grid;
		private bool _debug_render_world;
		private bool _debug_physics_world;
		private ToolType _tool_type;
		private SnapMode _snap_mode;
		private ReferenceSystem _reference_system;

		// Engine connections
		private GLib.Subprocess _compiler_process;
		private GLib.Subprocess _engine_process;
		private GLib.Subprocess _game_process;
		private ConsoleClient _compiler;
		private ConsoleClient _engine;

		// Level data
		private Database _db;
		private Level _level;
		private string _level_filename;
		private ResourceCompiler _resource_compiler;

		// Widgets
		private StartingCompiler _starting_compiler;
		private ConsoleView _console_view;
		private EngineView _engine_view;
		private LevelTreeView _level_treeview;
		private LevelLayersTreeView _level_layers_treeview;
		private PropertiesView _properties_view;
/*
		private GraphStore _graph_store;
		private GraphView _graph_view;
*/
		private Gtk.Alignment _alignment_engine;
		private Gtk.Alignment _alignment_level_tree_view;
		private Gtk.Alignment _alignment_properties_view;

		private Gtk.ActionGroup _action_group;
		private Gtk.UIManager _ui_manager;
		private Gtk.Paned _pane_left;
		private Gtk.Paned _pane_right;
		private Gtk.Notebook _notebook_left;
		private Gtk.Notebook _notebook_right;
		private Gtk.Box _vbox;
		private Gtk.FileFilter _file_filter;

		private ResourceBrowser _resource_browser;

		const Gtk.ActionEntry[] action_entries =
		{
			{ "menu-file",            null,  "_File",            null,             null, null                   },
			{ "new",                  null,  "New",              "<ctrl>N",        null, on_new                 },
			{ "open",                 null,  "Open",             "<ctrl>O",        null, on_open                },
			{ "save",                 null,  "Save",             "<ctrl>S",        null, on_save                },
			{ "save-as",              null,  "Save As...",       null,             null, on_save_as             },
			{ "preferences",          null,  "Preferences",      null,             null, on_preferences         },
			{ "quit",                 null,  "Quit",             "<ctrl>Q",        null, on_quit                },
			{ "menu-edit",            null,  "_Edit",            null,             null, null                   },
			{ "undo",                 null,  "Undo",             "<ctrl>Z",        null, on_undo                },
			{ "redo",                 null,  "Redo",             "<shift><ctrl>Z", null, on_redo                },
			{ "duplicate",            null,  "Duplicate",        "<ctrl>D",        null, on_duplicate           },
			{ "delete",               null,  "Delete",           "<ctrl>K",        null, on_delete              },
			{ "menu-grid",            null,  "Grid",             null,             null, null                   },
			{ "grid-custom",          null,  "Custom",           "G",              null, on_custom_grid         },
			{ "menu-rotation-snap",   null,  "Rotation Snap",    null,             null, null                   },
			{ "rotation-snap-custom", null,  "Custom",           "H",              null, on_rotation_snap       },
			{ "menu-create",          null,  "Create",           null,             null, null                   },
			{ "menu-primitives",      null,  "Primitives",       null,             null, null                   },
			{ "primitive-cube",       null,  "Cube",             null,             null, on_create_cube         },
			{ "primitive-sphere",     null,  "Sphere",           null,             null, on_create_sphere       },
			{ "primitive-cone",       null,  "Cone",             null,             null, on_create_cone         },
			{ "primitive-cylinder",   null,  "Cylinder",         null,             null, on_create_cylinder     },
			{ "primitive-plane",      null,  "Plane",            null,             null, on_create_plane        },
			{ "camera",               null,  "Camera",           null,             null, on_create_camera       },
			{ "light",                null,  "Light",            null,             null, on_create_light        },
			{ "sound-source",         null,  "Sound Source",     null,             null, on_create_sound_source },
			{ "menu-engine",          null,  "En_gine",          null,             null, null                   },
			{ "menu-view",            null,  "View",             null,             null, null                   },
			{ "resource-browser",     null,  "Resource Browser", "<ctrl>P",        null, on_resource_browser    },
			{ "restart",              null,  "_Restart",         null,             null, on_engine_restart      },
			{ "reload-lua",           null,  "Reload Lua",       "F7",             null, on_reload_lua          },
			{ "menu-run",             null,  "_Run",             null,             null, null                   },
			{ "game-run",             "run", "Run Game",         "F5",             null, on_run_game            },
			{ "menu-help",            null,  "Help",             null,             null, null                   },
			{ "manual",               null,  "Manual",           "F1",             null, on_manual              },
			{ "report-issue",         null,  "Report an Issue",  null,             null, on_report_issue        },
			{ "open-last-log",        null,  "Open last.log",    null,             null, on_open_last_log       },
			{ "about",                null,  "About",            null,             null, on_about               }
		};

		const Gtk.RadioActionEntry[] grid_entries =
		{
			{ "grid-0.1", null, "0.1m", null, null, 10  },
			{ "grid-0.2", null, "0.2m", null, null, 20  },
			{ "grid-0.5", null, "0.5m", null, null, 50  },
			{ "grid-1",   null, "1m",   null, null, 100 },
			{ "grid-2",   null, "2m",   null, null, 200 },
			{ "grid-5",   null, "5m",   null, null, 500 }
		};

		const RadioActionEntry[] rotation_snap_entries =
		{
			{ "rotation-snap-1",   null, "1°",   null, null,   1 },
			{ "rotation-snap-15",  null, "15°",  null, null,  15 },
			{ "rotation-snap-30",  null, "30°",  null, null,  30 },
			{ "rotation-snap-45",  null, "45°",  null, null,  45 },
			{ "rotation-snap-90",  null, "90°",  null, null,  90 },
			{ "rotation-snap-180", null, "180°", null, null, 180 }
		};

		const RadioActionEntry[] tool_entries =
		{
			{ "place",  "tool-place",  "Place",  "Q", "Place",  (int)ToolType.PLACE  },
			{ "move",   "tool-move",   "Move",   "W", "Move",   (int)ToolType.MOVE   },
			{ "rotate", "tool-rotate", "Rotate", "E", "Rotate", (int)ToolType.ROTATE },
			{ "scale",  "tool-scale",  "Scale",  "R", "Scale",  (int)ToolType.SCALE  }
		};

		const RadioActionEntry[] snap_mode_entries =
		{
			{ "snap-relative", "reference-local", "Relative Snap", null, "Relative Snap", (int)SnapMode.RELATIVE },
			{ "snap-absolute", "reference-world", "Absolute Snap", null, "Absolute Snap", (int)SnapMode.ABSOLUTE }
		};

		const RadioActionEntry[] reference_system_entries =
		{
			{ "reference-system-local", "axis-local", "Local Axis", null, "Local Axis", (int)ReferenceSystem.LOCAL },
			{ "reference-system-world", "axis-world", "World Axis", null, "World Axis", (int)ReferenceSystem.WORLD }
		};

		const ToggleActionEntry[] snap_to_entries =
		{
			{ "snap-to-grid", "snap-to-grid", "Snap To Grid", "<ctrl>U", "Snap To Grid", on_snap_to_grid, true },
			{ "grid-show",     null,          "Show Grid",    null,      "Show Grid",    on_show_grid,    true }
		};

		const ToggleActionEntry[] view_entries =
		{
			{ "debug-render-world",  null, "Debug Render World",  null, null, on_debug_render_world,  false },
			{ "debug-physics-world", null, "Debug Physics World", null, null, on_debug_physics_world, false }
		};

		public LevelEditor(string source_dir, string toolchain_dir, string data_dir)
		{
			this.title = "Level Editor";

			// Project paths
			_source_dir = source_dir;
			_toolchain_dir = toolchain_dir;
			_data_dir = data_dir;
			_platform   = "linux";

			// Editor state
			_grid_size = 1.0;
			_rotation_snap = 45.0;
			_show_grid = true;
			_snap_to_grid = true;
			_debug_render_world = false;
			_debug_physics_world = false;
			_tool_type = ToolType.MOVE;
			_snap_mode = SnapMode.RELATIVE;
			_reference_system = ReferenceSystem.LOCAL;

			// Engine connections
			_compiler_process = null;
			_engine_process = null;
			_game_process = null;
			_compiler = new ConsoleClient();
			_compiler.connected.connect(on_compiler_connected);
			_compiler.disconnected.connect(on_compiler_disconnected);
			_compiler.message_received.connect(on_message_received);
			_engine = new ConsoleClient();
			_engine.connected.connect(on_engine_connected);
			_engine.disconnected.connect(on_engine_disconnected);
			_engine.message_received.connect(on_message_received);

			// Level data
			_db = new Database();
			_level = new Level(_db, _engine, _source_dir, _toolchain_dir);
			_level_filename = null;
			_resource_compiler = new ResourceCompiler(_compiler);

			// Widgets
			_console_view = new ConsoleView(_engine);
			_level_treeview = new LevelTreeView(_db, _level);
			_level_layers_treeview = new LevelLayersTreeView(_db, _level);
			_properties_view = new PropertiesView(_level);
/*
			_graph_store = new GraphStore();
			_graph_view = new GraphView(_graph_store);
*/
			_starting_compiler = new StartingCompiler();
			_alignment_engine = new Gtk.Alignment(0, 0, 1, 1);
			_alignment_level_tree_view = new Gtk.Alignment(0, 0, 1, 1);
			_alignment_properties_view = new Gtk.Alignment(0, 0, 1, 1);
			_alignment_engine.add(_starting_compiler);
			_alignment_level_tree_view.add(_starting_compiler);
			_alignment_properties_view.add(_starting_compiler);

			start_compiler();
			new_level();

			try
			{
				Gtk.IconTheme.add_builtin_icon("tool-place",      16, new Pixbuf.from_file("ui/icons/theme/tool-place.png"));
				Gtk.IconTheme.add_builtin_icon("tool-move",       16, new Pixbuf.from_file("ui/icons/theme/tool-move.png"));
				Gtk.IconTheme.add_builtin_icon("tool-rotate",     16, new Pixbuf.from_file("ui/icons/theme/tool-rotate.png"));
				Gtk.IconTheme.add_builtin_icon("tool-scale",      16, new Pixbuf.from_file("ui/icons/theme/tool-scale.png"));
				Gtk.IconTheme.add_builtin_icon("axis-local",      16, new Pixbuf.from_file("ui/icons/theme/axis-local.png"));
				Gtk.IconTheme.add_builtin_icon("axis-world",      16, new Pixbuf.from_file("ui/icons/theme/axis-world.png"));
				Gtk.IconTheme.add_builtin_icon("snap-to-grid",    16, new Pixbuf.from_file("ui/icons/theme/snap-to-grid.png"));
				Gtk.IconTheme.add_builtin_icon("reference-local", 16, new Pixbuf.from_file("ui/icons/theme/reference-local.png"));
				Gtk.IconTheme.add_builtin_icon("reference-world", 16, new Pixbuf.from_file("ui/icons/theme/reference-world.png"));
				Gtk.IconTheme.add_builtin_icon("run",             16, new Pixbuf.from_file("ui/icons/theme/run.png"));
				Gtk.IconTheme.add_builtin_icon("level-tree",      16, new Pixbuf.from_file("ui/icons/theme/level-tree.png"));
				Gtk.IconTheme.add_builtin_icon("level-layers",    16, new Pixbuf.from_file("ui/icons/theme/level-layers.png"));
				Gtk.IconTheme.add_builtin_icon("layer-visible",   16, new Pixbuf.from_file("ui/icons/theme/layer-visible.png"));
				Gtk.IconTheme.add_builtin_icon("layer-locked",    16, new Pixbuf.from_file("ui/icons/theme/layer-locked.png"));
			}
			catch (Error e)
			{
				stderr.printf(e.message);
			}

			_action_group = new Gtk.ActionGroup("group");
			_action_group.add_actions(action_entries, this);
			_action_group.add_radio_actions(grid_entries, (int)(_grid_size*100.0), this.on_grid_changed);
			_action_group.add_radio_actions(rotation_snap_entries, (int)_rotation_snap, this.on_rotation_snap_changed);
			_action_group.add_radio_actions(tool_entries, (int)_tool_type, on_tool_changed);
			_action_group.add_radio_actions(snap_mode_entries, (int)_snap_mode, on_snap_mode_changed);
			_action_group.add_radio_actions(reference_system_entries, (int)_reference_system, on_reference_system_changed);
			_action_group.add_toggle_actions(snap_to_entries, this);
			_action_group.add_toggle_actions(view_entries, this);

			_ui_manager = new UIManager();
			try
			{
				_ui_manager.add_ui_from_file("ui/level_editor_menu.xml");
				_ui_manager.insert_action_group(_action_group, 0);
				add_accel_group(_ui_manager.get_accel_group());
			}
			catch (Error e)
			{
				error(e.message);
			}

			_pane_left = new Gtk.Paned(Gtk.Orientation.VERTICAL);
			_pane_left.pack1(_alignment_engine, true, true);
			_pane_left.pack2(_console_view, true, true);

			Toolbar toolbar = _ui_manager.get_widget("/toolbar") as Toolbar;
			toolbar.set_icon_size(Gtk.IconSize.SMALL_TOOLBAR);
			toolbar.set_style(Gtk.ToolbarStyle.ICONS);
			Gtk.Box vb = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
			vb.pack_start(toolbar, false, false, 0);
			vb.pack_start(_pane_left, true, true, 0);

			_notebook_right = new Notebook();
			_notebook_right.show_border = false;
			_notebook_right.append_page(_level_treeview, new Gtk.Image.from_icon_name("level-tree", IconSize.SMALL_TOOLBAR));
			_notebook_right.append_page(_level_layers_treeview, new Gtk.Image.from_icon_name("level-layers", IconSize.SMALL_TOOLBAR));

			Gtk.Paned rb = new Gtk.Paned(Gtk.Orientation.VERTICAL);
			rb.pack1(_notebook_right, true, true);
			rb.pack2(_alignment_properties_view, true, true);

			_pane_right = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
			_pane_right.pack1(vb, true, false);
			_pane_right.pack2(rb, true, false);

			_notebook_left = new Notebook();
			_notebook_left.show_border = false;
			_notebook_left.append_page(_pane_right, new Gtk.Label("Level"));
/*
			_notebook_left.append_page(_graph_view, new Gtk.Label("Pepper"));
*/

			MenuBar menu = (MenuBar)_ui_manager.get_widget("/menubar");
			_vbox = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
			_vbox.pack_start(menu, false, false, 0);
			_vbox.pack_start(_notebook_left, true, true, 0);

			_file_filter = new FileFilter();
			_file_filter.set_filter_name("Level (*.level)");
			_file_filter.add_pattern("*.level");

			_resource_browser = new ResourceBrowser(toolbar, _source_dir, _data_dir);
			_resource_browser.resource_selected.connect(on_resource_browser_resource_selected);
			_resource_browser.delete_event.connect(() => { _resource_browser.hide(); return true; });
			_resource_browser.modal = true;

			this.destroy.connect(this.on_destroy);
			this.delete_event.connect(this.on_delete_event);

			this.add(_vbox);
			this.set_size_request(1280, 720);
			this.maximize();
			this.show_all();
		}

		private bool on_focus_in(EventFocus ev)
		{
			add_accel_group(_ui_manager.get_accel_group());
			return true;
		}

		private bool on_focus_out(EventFocus ev)
		{
			remove_accel_group(_ui_manager.get_accel_group());
			return true;
		}

		private void on_resource_browser_resource_selected(PlaceableType placeable_type, string name)
		{
			_engine.send_script(LevelEditorApi.set_placeable(placeable_type, name));
		}

		private void on_compiler_connected()
		{
			_console_view.log("Compiler connected.", "info");
			_compiler.receive_async();
		}

		private void on_compiler_disconnected()
		{
			_console_view.log("Compiler disconnected.", "info");
		}

		private void on_engine_connected()
		{
			_console_view.log("Engine connected.", "info");
			_engine.receive_async();
		}

		private void on_engine_disconnected()
		{
			_console_view.log("Engine disconnected.", "info");
		}

		private static int stringcmp(ref string a, ref string b)
		{
			return Posix.strcmp(a, b);
		}

		private void on_message_received(ConsoleClient client, uint8[] json)
		{
			try
			{
				Hashtable msg = JSON.decode(json) as Hashtable;
				string msg_type = msg["type"] as string;

				if (msg_type == "message")
				{
					_console_view.log((string)msg["message"], (string)msg["severity"]);
				}
				else if (msg_type == "compile")
				{
					Guid id = Guid.parse((string)msg["id"]);

					if (msg.has_key("start"))
					{
						// FIXME
					}
					else if (msg.has_key("success"))
					{
						_resource_compiler.finished((bool)msg["success"]);
					}
				}
				else if (msg_type == "unit_spawned")
				{
					string id             = (string)           msg["id"];
					string name           = (string)           msg["name"];
					ArrayList<Value?> pos = (ArrayList<Value?>)msg["position"];
					ArrayList<Value?> rot = (ArrayList<Value?>)msg["rotation"];
					ArrayList<Value?> scl = (ArrayList<Value?>)msg["scale"];

					_level.on_unit_spawned(Guid.parse(id)
						, name
						, Vector3.from_array(pos)
						, Quaternion.from_array(rot)
						, Vector3.from_array(scl)
						);
				}
				else if (msg_type == "sound_spawned")
				{
					string id             = (string)           msg["id"];
					string name           = (string)           msg["name"];
					ArrayList<Value?> pos = (ArrayList<Value?>)msg["position"];
					ArrayList<Value?> rot = (ArrayList<Value?>)msg["rotation"];
					ArrayList<Value?> scl = (ArrayList<Value?>)msg["scale"];
					double range          = (double)           msg["range"];
					double volume         = (double)           msg["volume"];
					bool loop             = (bool)             msg["loop"];

					_level.on_sound_spawned(Guid.parse(id)
						, name
						, Vector3.from_array(pos)
						, Quaternion.from_array(rot)
						, Vector3.from_array(scl)
						, range
						, volume
						, loop
						);
				}
				else if (msg_type == "move_objects")
				{
					Hashtable ids           = (Hashtable)msg["ids"];
					Hashtable new_positions = (Hashtable)msg["new_positions"];
					Hashtable new_rotations = (Hashtable)msg["new_rotations"];
					Hashtable new_scales    = (Hashtable)msg["new_scales"];

					string[] keys = ids.keys.to_array();
					Posix.qsort(keys, keys.length, sizeof(string), (Posix.compar_fn_t)stringcmp);

					Guid[] n_ids             = new Guid[keys.length];
					Vector3[] n_positions    = new Vector3[keys.length];
					Quaternion[] n_rotations = new Quaternion[keys.length];
					Vector3[] n_scales       = new Vector3[keys.length];

					for (int i = 0; i < keys.length; ++i)
					{
						string k = keys[i];

						n_ids[i]       = Guid.parse((string)ids[k]);
						n_positions[i] = Vector3.from_array((ArrayList<Value?>)(new_positions[k]));
						n_rotations[i] = Quaternion.from_array((ArrayList<Value?>)new_rotations[k]);
						n_scales[i]    = Vector3.from_array((ArrayList<Value?>)new_scales[k]);
					}

					_level.move_objects(n_ids, n_positions, n_rotations, n_scales);
				}
				else if (msg_type == "selection")
				{
					Hashtable objects = (Hashtable)msg["objects"];

					string[] keys = objects.keys.to_array();
					Posix.qsort(keys, keys.length, sizeof(string), (Posix.compar_fn_t)stringcmp);

					Guid[] ids = new Guid[keys.length];

					for (int i = 0; i < keys.length; ++i)
					{
						string k = keys[i];
						ids[i] = Guid.parse((string)objects[k]);
					}

					_level.on_selection(ids);
				}
			}
			catch (Error e)
			{
				_console_view.log(e.message, "error");
			}

			// Receive next message
			client.receive_async();
		}

		private void send_state()
		{
			StringBuilder sb = new StringBuilder();
			sb.append(LevelEditorApi.set_grid_size(_grid_size));
			sb.append(LevelEditorApi.set_rotation_snap(_rotation_snap));
			sb.append(LevelEditorApi.enable_show_grid(_show_grid));
			sb.append(LevelEditorApi.enable_snap_to_grid(_snap_to_grid));
			sb.append(LevelEditorApi.enable_debug_render_world(_debug_render_world));
			sb.append(LevelEditorApi.enable_debug_physics_world(_debug_physics_world));
			sb.append(LevelEditorApi.set_tool_type(_tool_type));
			sb.append(LevelEditorApi.set_snap_mode(_snap_mode));
			sb.append(LevelEditorApi.set_reference_system(_reference_system));
			_engine.send_script(sb.str);
		}

		private bool on_button_press(EventButton ev)
		{
			// Prevent accelerators to step on camera's toes
			remove_accel_group(_ui_manager.get_accel_group());
			return true;
		}

		private bool on_button_release(EventButton ev)
		{
			add_accel_group(_ui_manager.get_accel_group());
			return true;
		}

		private void start_compiler()
		{
			string args[] =
			{
				ENGINE_EXE,
				"--source-dir", _source_dir,
				"--map-source-dir", "core", _toolchain_dir,
				"--server",
				null
			};

			GLib.SubprocessLauncher sl = new GLib.SubprocessLauncher(SubprocessFlags.STDOUT_SILENCE | SubprocessFlags.STDERR_SILENCE);
			sl.set_cwd(ENGINE_DIR);
			try
			{
				_compiler_process = sl.spawnv(args);
			}
			catch (Error e)
			{
				_console_view.log(e.message, "error");
			}

			while (!_compiler.is_connected())
				_compiler.connect("127.0.0.1", CROWN_DEFAULT_SERVER_PORT);

			_resource_compiler.compile.begin(_data_dir, _platform, (obj, res) => {
				if (_resource_compiler.compile.end(res))
				{
					if (_engine_view != null)
						return;

					_engine_view = new EngineView(_engine);
					_engine_view.realized.connect(on_engine_view_realized);
					_engine_view.button_press_event.connect(on_button_press);
					_engine_view.button_release_event.connect(on_button_release);
					_engine_view._event_box.focus_in_event.connect(on_focus_in);
					_engine_view._event_box.focus_out_event.connect(on_focus_out);

					_alignment_engine.remove(_alignment_engine.get_child());
					_alignment_level_tree_view.remove(_alignment_level_tree_view.get_child());
					_alignment_properties_view.remove(_alignment_properties_view.get_child());

					_alignment_engine.add(_engine_view);
					_alignment_level_tree_view.add(_level_treeview);
					_alignment_properties_view.add(_properties_view);

					_alignment_engine.show_all();
					_alignment_level_tree_view.show_all();
					_alignment_properties_view.show_all();
				}
			});
		}

		private void stop_compiler()
		{
			_compiler.close();

			if (_compiler_process != null)
				_compiler_process.force_exit();
		}

		private void start_engine(uint window_xid)
		{
			string args[] =
			{
				ENGINE_EXE,
				"--data-dir", _data_dir,
				"--boot-dir", LEVEL_EDITOR_BOOT_DIR,
				"--parent-window", window_xid.to_string(),
				"--wait-console",
				null
			};

			GLib.SubprocessLauncher sl = new GLib.SubprocessLauncher(SubprocessFlags.STDOUT_SILENCE | SubprocessFlags.STDERR_SILENCE);
			sl.set_cwd(ENGINE_DIR);
			try
			{
				_engine_process = sl.spawnv(args);
			}
			catch (Error e)
			{
				_console_view.log(e.message, "error");
			}

			while (!_engine.is_connected())
				_engine.connect("127.0.0.1", 10001);

			_level.send();
			send_state();
		}

		private void stop_engine()
		{
			_engine.close();

			if (_engine_process != null)
				_engine_process.force_exit();
		}

		private void restart_engine()
		{
			stop_engine();
			start_engine(_engine_view.window_id);
		}

		private void start_game()
		{
			_resource_compiler.compile.begin(_data_dir, _platform, (obj, res) => {
				if (_resource_compiler.compile.end(res))
				{
					string args[] =
					{
						ENGINE_EXE,
						"--data-dir", _data_dir,
						"--console-port", "12345",
						null
					};

					GLib.SubprocessLauncher sl = new GLib.SubprocessLauncher(SubprocessFlags.NONE);
					sl.set_cwd(ENGINE_DIR);

					try
					{
						_game_process = sl.spawnv(args);
					}
					catch (Error e)
					{
						_console_view.log(e.message, "error");
					}
				}
			});
		}

		private void stop_game()
		{
			if (_game_process != null)
				_game_process.force_exit();
		}

		private void on_engine_view_realized()
		{
			start_engine(_engine_view.window_id);
		}

		private void on_tool_changed(Gtk.Action action)
		{
			RadioAction ra = (RadioAction)action;
			_tool_type = (ToolType)ra.current_value;
			send_state();
		}

		private void on_snap_mode_changed(Gtk.Action action)
		{
			RadioAction ra = (RadioAction)action;
			_snap_mode = (SnapMode)ra.current_value;
			send_state();
		}

		private void on_reference_system_changed(Gtk.Action action)
		{
			RadioAction ra = (RadioAction)action;
			_reference_system = (ReferenceSystem)ra.current_value;
			send_state();
		}

		private void on_grid_changed(Gtk.Action action)
		{
			RadioAction ra = (RadioAction)action;
			_grid_size = (float)ra.current_value/100.0;
			send_state();
		}

		private void on_rotation_snap_changed(Gtk.Action action)
		{
			RadioAction ra = (RadioAction)action;
			_rotation_snap = (float)ra.current_value;
			send_state();
		}

		private void new_level()
		{
			_level_filename = null;
			_level.new_level();
		}

		private void load()
		{
			FileChooserDialog fcd = new FileChooserDialog("Open..."
				, this
				, FileChooserAction.OPEN
				, "Cancel"
				, ResponseType.CANCEL
				, "Open"
				, ResponseType.ACCEPT
				);
			fcd.add_filter(_file_filter);
			fcd.set_current_folder(_source_dir);

			if (fcd.run() == (int)ResponseType.ACCEPT)
			{
				_level_filename = fcd.get_filename();
				_level.load(_level_filename);
				_level.send();
				send_state();
			}

			fcd.destroy();
		}

		private bool save_as()
		{
			bool saved = false;

			FileChooserDialog fcd = new FileChooserDialog("Save As..."
				, this
				, FileChooserAction.SAVE
				, "Cancel"
				, ResponseType.CANCEL
				, "Save"
				, ResponseType.ACCEPT
				);
			fcd.add_filter(_file_filter);
			fcd.set_current_folder(_source_dir);

			if (fcd.run() == (int)ResponseType.ACCEPT)
			{
				_level_filename = fcd.get_filename();
				_level.save(_level_filename);
				saved = true;
			}

			fcd.destroy();
			return saved;
		}

		private bool save()
		{
			bool saved = false;

			if (_level_filename == null)
			{
				saved = save_as();
			}
			else
			{
				_level.save(_level_filename);
				saved = true;
			}

			return saved;
		}

		private void shutdown()
		{
			_resource_browser.destroy();
			stop_game();
			stop_engine();
			stop_compiler();
			Gtk.main_quit();
		}

		private void quit()
		{
			if (!_db.changed())
			{
				shutdown();
				return;
			}

			Gtk.MessageDialog md = new Gtk.MessageDialog(this
				, Gtk.DialogFlags.MODAL
				, Gtk.MessageType.WARNING
				, Gtk.ButtonsType.NONE
				, "File changed, save?"
				);
			md.add_button("Quit without Saving", ResponseType.NO);
			md.add_button("Cancel", ResponseType.CANCEL);
			md.add_button("Save", ResponseType.YES);
			int rt = md.run();
			md.destroy();

			if (rt == (int)ResponseType.YES && save() || rt == (int)ResponseType.NO)
				shutdown();
		}

		private void on_new()
		{
			if (!_db.changed())
			{
				new_level();
				_level.send();
				send_state();
				return;
			}

			Gtk.MessageDialog md = new Gtk.MessageDialog(this
				, Gtk.DialogFlags.MODAL
				, Gtk.MessageType.WARNING
				, Gtk.ButtonsType.NONE
				, "File changed, save?"
				);
			md.add_button("New without Saving", ResponseType.NO);
			md.add_button("Cancel", ResponseType.CANCEL);
			md.add_button("Save", ResponseType.YES);
			int rt = md.run();
			md.destroy();

			if (rt == (int)ResponseType.YES && save() || rt == (int)ResponseType.NO)
			{
				new_level();
				_level.send();
				send_state();
			}
		}

		private void on_open(Gtk.Action action)
		{
			if (!_db.changed())
			{
				load();
				return;
			}

			Gtk.MessageDialog md = new Gtk.MessageDialog(this
				, Gtk.DialogFlags.MODAL
				, Gtk.MessageType.WARNING
				, Gtk.ButtonsType.NONE
				, "File changed, save?"
				);
			md.add_button("Open without Saving", ResponseType.NO);
			md.add_button("Cancel", ResponseType.CANCEL);
			md.add_button("Save", ResponseType.YES);
			int rt = md.run();
			md.destroy();

			if (rt == (int)ResponseType.YES && save() || rt == (int)ResponseType.NO)
				load();
		}

		private void on_save(Gtk.Action action)
		{
			save();
		}

		private void on_save_as(Gtk.Action action)
		{
			save_as();
		}

		private void on_preferences(Gtk.Action action)
		{
		}

		private void on_quit(Gtk.Action action)
		{
			quit();
		}

		private void on_show_grid(Gtk.Action action)
		{
			ToggleAction ta = (ToggleAction)action;
			_show_grid = ta.active;
			send_state();
		}

		private void on_custom_grid()
		{
			MessageDialog dg = new MessageDialog(this
				, DialogFlags.MODAL
				, MessageType.OTHER
				, ButtonsType.OK_CANCEL
				, "Grid size in meters:"
				);

			SpinButtonDouble sb = new SpinButtonDouble(_grid_size, 0.1, 1000);
			(dg.message_area as Gtk.Box).add(sb);
			dg.show_all();

			if (dg.run() == (int)ResponseType.OK)
			{
				_grid_size = sb.value;
				send_state();
			}

			dg.destroy();
		}

		private void on_rotation_snap(Gtk.Action action)
		{
			MessageDialog dg = new MessageDialog(this
				, DialogFlags.MODAL
				, MessageType.OTHER
				, ButtonsType.OK_CANCEL
				, "Rotation snap in degrees:"
				);

			SpinButtonDouble sb = new SpinButtonDouble(_rotation_snap, 1.0, 180.0);
			(dg.message_area as Gtk.Box).add(sb);
			dg.show_all();

			if (dg.run() == (int)ResponseType.OK)
			{
				_rotation_snap = sb.value;
				send_state();
			}

			dg.destroy();
		}

		private void on_create_cube(Gtk.Action action)
		{
			_engine.send_script(LevelEditorApi.set_placeable(PlaceableType.UNIT, "core/units/primitives/cube"));
		}

		private void on_create_sphere(Gtk.Action action)
		{
			_engine.send_script(LevelEditorApi.set_placeable(PlaceableType.UNIT, "core/units/primitives/sphere"));
		}

		private void on_create_cone(Gtk.Action action)
		{
			_engine.send_script(LevelEditorApi.set_placeable(PlaceableType.UNIT, "core/units/primitives/cone"));
		}

		private void on_create_cylinder(Gtk.Action action)
		{
			_engine.send_script(LevelEditorApi.set_placeable(PlaceableType.UNIT, "core/units/primitives/cylinder"));
		}

		private void on_create_plane(Gtk.Action action)
		{
			_engine.send_script(LevelEditorApi.set_placeable(PlaceableType.UNIT, "core/units/primitives/plane"));
		}

		private void on_create_camera(Gtk.Action action)
		{
			_engine.send_script(LevelEditorApi.set_placeable(PlaceableType.UNIT, "core/units/camera"));
		}

		private void on_create_light(Gtk.Action action)
		{
			_engine.send_script(LevelEditorApi.set_placeable(PlaceableType.UNIT, "core/units/light"));
		}

		private void on_create_sound_source(Gtk.Action action)
		{
			_engine.send_script(LevelEditorApi.set_placeable(PlaceableType.SOUND, ""));
		}

		private void on_resource_browser(Gtk.Action action)
		{
			_resource_browser.show_all();
		}

		private void on_engine_restart(Gtk.Action action)
		{
			restart_engine();
		}

		private void on_reload_lua(Gtk.Action action)
		{
			_resource_compiler.compile.begin(_data_dir, _platform, (obj, res) => {
				if (_resource_compiler.compile.end(res))
				{
					_engine.send(EngineApi.pause());
					_engine.send(EngineApi.reload("lua", "core/editors/level_editor/level_editor"));
					_engine.send(EngineApi.unpause());
				}
			});
		}

		public void on_snap_to_grid(Gtk.Action action)
		{
			ToggleAction ta = (ToggleAction)action;
			_snap_to_grid = ta.active;
			send_state();
		}

		private void on_debug_render_world(Gtk.Action action)
		{
			ToggleAction ta = (ToggleAction)action;
			_debug_render_world = ta.active;
			send_state();
		}

		private void on_debug_physics_world(Gtk.Action action)
		{
			ToggleAction ta = (ToggleAction)action;
			_debug_physics_world = ta.active;
			send_state();
		}

		private void on_run_game(Gtk.Action action)
		{
			start_game();
		}

		private void on_undo(Gtk.Action action)
		{
			_db.undo();
		}

		private void on_redo(Gtk.Action action)
		{
			_db.redo();
		}

		private void on_duplicate(Gtk.Action action)
		{
			_level.duplicate_selected_objects();
		}

		private void on_delete(Gtk.Action action)
		{
			_level.destroy_selected_objects();
		}

		private void on_manual(Gtk.Action action)
		{
			try
			{
				AppInfo.launch_default_for_uri("https://taylor001.github.io/crown/manual.html", null);
			}
			catch (Error e)
			{
				_console_view.log(e.message, "error");
			}
		}

		private void on_report_issue(Gtk.Action action)
		{
			try
			{
				AppInfo.launch_default_for_uri("https://github.com/taylor001/crown/issues", null);
			}
			catch (Error e)
			{
				_console_view.log(e.message, "error");
			}
		}

		private void on_open_last_log(Gtk.Action action)
		{
			File file = File.new_for_path(_data_dir + "/last.log");
			try
			{
				AppInfo.launch_default_for_uri(file.get_uri(), null);
			}
			catch (Error e)
			{
				_console_view.log(e.message, "error");
			}
		}

		private void on_about(Gtk.Action action)
		{
			Gtk.AboutDialog dlg = new Gtk.AboutDialog();
			dlg.set_destroy_with_parent(true);
			dlg.set_transient_for(this);
			dlg.set_modal(true);

			try
			{
				dlg.set_logo(new Pixbuf.from_file("ui/icons/128x128/pepper.png"));
			}
			catch (Error e)
			{
				stderr.printf("%s\n", e.message);
			}

			dlg.program_name = "Crown Game Engine";
			dlg.version = CROWN_VERSION;
			dlg.website = "https://github.com/taylor001/crown";
			dlg.copyright = "Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.";
			dlg.license = "Crown Game Engine.\n"
				+ "Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.\n"
				+ "\n"
				+ "This program is free software; you can redistribute it and/or\n"
				+ "modify it under the terms of the GNU General Public License\n"
				+ "as published by the Free Software Foundation; either version 2\n"
				+ "of the License, or (at your option) any later version.\n"
				+ "\n"
				+ "This program is distributed in the hope that it will be useful,\n"
				+ "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
				+ "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
				+ "GNU General Public License for more details.\n"
				+ "\n"
				+ "You should have received a copy of the GNU General Public License\n"
				+ "along with this program; if not, write to the Free Software\n"
				+ "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n"
				;
			dlg.run();
			dlg.destroy();
		}

		private void on_destroy()
		{
			Gtk.main_quit();
		}

		private bool on_delete_event()
		{
			quit();
			return true;
		}
	}

	public static int main (string[] args)
	{
		Gtk.init(ref args);
		Intl.setlocale(LocaleCategory.ALL, "C");
		Gtk.Settings.get_default().gtk_application_prefer_dark_theme = true;

		Gtk.CssProvider provider = new Gtk.CssProvider();
		Gdk.Screen screen = Gdk.Display.get_default().get_default_screen();
		Gtk.StyleContext.add_provider_for_screen(screen, provider, STYLE_PROVIDER_PRIORITY_APPLICATION);
		provider.load_from_path("ui/theme/style.css");

		var window = new LevelEditor(args[1], args[2], args[3]);
		window.show_all();

		Gtk.main();
		return 0;
	}
}
