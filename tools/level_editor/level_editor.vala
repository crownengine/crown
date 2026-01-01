/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#if CROWN_PLATFORM_WINDOWS
extern uint GetCurrentProcessId();
extern uintptr OpenProcess(uint dwDesiredAccess, bool bInheritHandle, uint dwProcessId);
#elif CROWN_PLATFORM_LINUX
extern Posix.pid_t getpid();
#endif

namespace Crown
{
const int WINDOW_DEFAULT_WIDTH = 1280;
const int WINDOW_DEFAULT_HEIGHT = 720;
const string CROWN_EDITOR_NAME = "Crown Editor";
const string CROWN_EDITOR_ICON_NAME = "org.crownengine.Crown";
const string CROWN_SUBPROCESS_LAUNCHER = "org.crownengine.SubprocessLauncher";

public const string PANEL_WAITING = "panel-waiting";
public const string PANEL_EDITOR = "panel-editor";
public const string PANEL_PROJECTS_LIST = "panel-projects-list";
public const string PANEL_NEW_PROJECT = "panel-new-project";

public enum Theme
{
	DARK,
	LIGHT,

	COUNT
}

public enum ToolType
{
	PLACE,
	MOVE,
	ROTATE,
	SCALE,

	COUNT
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

public enum CameraViewType
{
	PERSPECTIVE,
	FRONT,
	BACK,
	RIGHT,
	LEFT,
	TOP,
	BOTTOM,

	COUNT;

	public string to_label()
	{
		switch (this) {
		case PERSPECTIVE:
			return "Perspective";
		case FRONT:
			return "View Front";
		case BACK:
			return "View Back";
		case RIGHT:
			return "View Right";
		case LEFT:
			return "View Left";
		case TOP:
			return "View Top";
		case BOTTOM:
			return "View Bottom";
		default:
			return "View Unknown";
		}
	}
}

public enum TargetConfig
{
	RELEASE,
	DEVELOPMENT,
	DEBUG,

	COUNT;

	public string to_key()
	{
		switch (this) {
		case RELEASE:
			return "release";
		case DEVELOPMENT:
			return "development";
		case DEBUG:
			return "debug";
		default:
			return "unknown";
		}
	}

	public string to_label()
	{
		switch (this) {
		case RELEASE:
			return "Release";
		case DEVELOPMENT:
			return "Development";
		case DEBUG:
			return "Debug";
		default:
			return "unknown";
		}
	}
}

public enum TargetPlatform
{
	ANDROID,
	HTML5,
	LINUX,
	WINDOWS,

	COUNT;

	public string to_key()
	{
		switch (this) {
		case ANDROID:
			return "android";
		case HTML5:
			return "html5";
		case LINUX:
			return "linux";
		case WINDOWS:
			return "windows";
		default:
			return "unknown";
		}
	}

	public string to_label()
	{
		switch (this) {
		case ANDROID:
			return "Android";
		case HTML5:
			return "HTML5";
		case LINUX:
			return "Linux";
		case WINDOWS:
			return "Windows";
		default:
			return "Unknown";
		}
	}
}

public enum TargetArch
{
	X86,
	X64,
	ARM,
	ARM64,
	WASM
}

public class RuntimeInstance
{
	public string _name;
	public uint32 _process_id;
	public uint _revision;
	public GLib.SourceFunc _stop_callback;
	public GLib.SourceFunc _refresh_callback;
	public bool _refresh_success;
	public ConsoleClient _client;

	public signal void connected(RuntimeInstance ri, string address, int port);
	public signal void disconnected(RuntimeInstance ri);
	public signal void disconnected_unexpected(RuntimeInstance ri);
	public signal void message_received(RuntimeInstance ri, ConsoleClient client, uint8[] json);

	public RuntimeInstance(string name)
	{
		_name = name;
		_process_id = uint32.MAX;
		_revision = 0;
		_stop_callback = null;
		_refresh_callback = null;
		_refresh_success = false;
		_client = new ConsoleClient();
		_client.connected.connect(on_client_connected);
		_client.message_received.connect(on_client_message_received);
	}

	public void on_client_connected(string address, int port)
	{
		connected(this, address, port);
	}

	public void on_client_disconnected()
	{
		disconnected(this);

		if (_stop_callback != null)
			_stop_callback();
	}

	public void on_client_disconnected_unexpected()
	{
		disconnected_unexpected(this);

		try {
			if (_process_id != uint32.MAX) {
				_subprocess_launcher.wait(_process_id);
				_process_id = uint32.MAX;
			}
		} catch (GLib.Error e) {
			loge(e.message);
		}
	}

	public void on_client_message_received(ConsoleClient client, uint8[] json)
	{
		message_received(this, client, json);
	}

	// Tries to connect to the @a client. Return the number of tries after
	// it succeeded or @a num_tries if failed.
	public async int connect_async(string address, int port, int num_tries, int interval)
	{
		// It is an error if the client disconnects after here.
		_client.disconnected.disconnect(on_client_disconnected);
		_client.disconnected.connect(on_client_disconnected_unexpected);

		// Try to connect to the client.
		int tries;
		for (tries = 0; tries < num_tries; ++tries) {
			_client.connect(address, port);
			if (_client.is_connected())
				break;

			GLib.Thread.usleep(interval*1000);
		}
		return tries;
	}

	public async void stop()
	{
		if (_client != null) {
			// Reset "disconnected" signal.
			_client.disconnected.disconnect(on_client_disconnected);
			_client.disconnected.disconnect(on_client_disconnected_unexpected);

			// Explicit call to this function should not produce error messages.
			_client.disconnected.connect(on_client_disconnected);

			if (_client.is_connected()) {
				_stop_callback = stop.callback;
				_client.send(RuntimeApi.quit());
				yield; // Wait for _client to disconnect.
				_stop_callback = null;
			}
		}

		try {
			if (_process_id != uint32.MAX)
				_subprocess_launcher.wait(_process_id);
			_process_id = uint32.MAX;
		} catch (GLib.Error e) {
			loge(e.message);
		}
	}

	public void send(string json)
	{
		_client.send(json);
	}

	public void send_script(string lua)
	{
		_client.send_script(lua);
	}

	public bool is_connected()
	{
		return _client.is_connected();
	}

	public async bool refresh(DataCompiler dc)
	{
		if (_refresh_callback != null)
			return false;

		if (!is_connected())
			return false;

		var compiler_revision = dc._revision;
		var refresh_list = yield dc.refresh_list(_revision);
		_client.send(DeviceApi.refresh(refresh_list));
		_client.send(DeviceApi.frame());
		_refresh_callback = refresh.callback;
		yield; // Wait for client to refresh the resources.

		if (_refresh_success)
			_revision = compiler_revision;

		return _refresh_success;
	}

	public void refresh_finished(bool success)
	{
		_refresh_success = success;
		if (_refresh_callback != null)
			_refresh_callback();
		_refresh_callback = null;
	}
}

const string BACKEND_STOPPING = "backend-stopping";
const string COMPILER_CRASHED = "compiler-crashed";
const string COMPILER_COMPILING_DATA = "compiler-compiling-data";
const string COMPILER_FAILED_COMPILATION = "compiler-failed-compilation";
const string COMPILER_CONNECTING = "compiler-connecting";

Gtk.Stack make_compiler_stack(Gtk.Widget child)
{
	Gtk.Stack stack = new Gtk.Stack();
	stack.add_named(stopping_backend(), BACKEND_STOPPING);
	stack.add_named(compiling_data(), COMPILER_COMPILING_DATA);
	stack.add_named(connecting_to_data_compiler(), COMPILER_CONNECTING);
	stack.add_named(compiler_crashed(), COMPILER_CRASHED);
	stack.add_named(compiler_failed_compilation(), COMPILER_FAILED_COMPILATION);
	stack.add_named(child, "child");
	return stack;
}

public class LevelEditorWindow : Gtk.ApplicationWindow
{
	public const GLib.ActionEntry[] action_entries =
	{
		{ "fullscreen", on_fullscreen, null, null }
	};

	public bool _fullscreen;

	public LevelEditorWindow(Gtk.Application app, Gtk.HeaderBar header_bar)
	{
		Object(application: app);

		this.add_action_entries(action_entries, this);
		this.set_titlebar(header_bar);

		this.title = CROWN_EDITOR_NAME;
		this.window_state_event.connect(this.on_window_state_event);
		this.delete_event.connect(this.on_delete_event);

		_fullscreen = false;

		this.set_default_size(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);
	}

	public void on_fullscreen(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_fullscreen)
			unfullscreen();
		else
			fullscreen();
	}

	public bool on_window_state_event(Gdk.EventWindowState ev)
	{
		_fullscreen = (ev.new_window_state & Gdk.WindowState.FULLSCREEN) != 0;
		return Gdk.EVENT_PROPAGATE;
	}

	public bool on_delete_event()
	{
		GLib.Application.get_default().activate_action("quit", null);
		return Gdk.EVENT_STOP; // Keep window alive.
	}

	public Hashtable encode()
	{
		Hashtable json_obj = new Hashtable();

		// This is the appropriate size to save, see:
		// https://valadoc.org/gtk+-3.0/Gtk.Window.set_default_size.html
		int width;
		int height;
		this.get_size(out width, out height);
		json_obj["width"] = width;
		json_obj["height"] = height;

		json_obj["maximized"] = this.is_maximized;
		json_obj["fullscreen"] = this._fullscreen;
		return json_obj;
	}

	public void decode(Hashtable json_obj)
	{
		if (json_obj.has_key("width"))
			this.default_width = (int)(double)json_obj["width"];
		else
			this.default_width = WINDOW_DEFAULT_WIDTH;

		if (json_obj.has_key("height"))
			this.default_height = (int)(double)json_obj["height"];
		else
			this.default_height = WINDOW_DEFAULT_HEIGHT;

		if (json_obj.has_key("maximized")) {
			if ((bool)json_obj["maximized"])
				this.maximize();
			else
				this.unmaximize();
		}

		if (json_obj.has_key("fullscreen")) {
			if ((bool)json_obj["fullscreen"])
				this.fullscreen();
			else
				this.unfullscreen();
		}
	}
}

public enum StartGame
{
	NORMAL,
	TEST
}

public class LevelEditorApplication : Gtk.Application
{
	public const GLib.ActionEntry[] action_entries_file =
	{
		//                                        parameter type
		// name                activate()         |        state
		// |                   |                  |        |
		{ "menu-file",          null,                  null,    null },
		{ "new-level",          on_new_level,          null,    null },
		{ "open-level",         on_open_level,         "s",     null },
		{ "new-project",        on_new_project,        null,    null },
		{ "add-project",        on_add_project,        null,    null },
		{ "remove-project",     on_remove_project,     "s",     null },
		{ "open-project",       on_open_project,       "(ss)",  null },
		{ "open-project-null",  on_open_project,       null,    null },
		{ "open-projects-list", on_open_projects_list, null,    null },
		{ "save",               on_save,               null,    null },
		{ "save-as",            on_save_as,            null,    null },
		{ "import",             on_import,             "(sas)", null },
		{ "import-null",        on_import,             null,    null },
		{ "preferences",        on_preferences,        null,    null },
		{ "deploy",             on_deploy,             null,    null },
		{ "close-project",      on_close_project,      null,    null },
		{ "quit",               on_quit,               null,    null },
		{ "open-resource",      on_open_resource,      "s",     null },
		{ "copy-path",          on_copy_path,          "s",     null },
		{ "copy-name",          on_copy_name,          "s",     null },
	};

	public const GLib.ActionEntry[] action_entries_edit =
	{
		{ "menu-edit",          null,                  null,   null    },
		{ "rename",             on_rename,             "(ss)", null    },
		{ "tool",               on_tool,               "i",    "1"     }, // See: Crown.ToolType
		{ "set-placeable",      on_set_placeable,      "(ss)", null    },
		{ "cancel-place",       on_cancel_place,       null,   null    },
		{ "snap",               on_snap,               "i",    "0"     }, // See: Crown.SnapMode
		{ "reference-system",   on_reference_system,   "i",    "0"     }, // See: Crown.ReferenceSystem
		{ "snap-to-grid",       on_snap_to_grid,       null,   "false" },
		{ "menu-grid",          null,                  null,   null    },
		{ "grid-show",          on_show_grid,          null,   "true"  },
		{ "grid-size",          on_grid_size,          "i",    "10"    }, // 10*meters.
		{ "menu-rotation-snap", null,                  null,   null    },
		{ "rotation-snap-size", on_rotation_snap_size, "i",    "15"    }
	};

	public const GLib.ActionEntry[] action_entries_create =
	{
		{ "menu-create",        null,               null, null },
		{ "menu-primitives",    null,               null, null },
		{ "primitive-cube",     on_spawn_primitive, null, null },
		{ "primitive-sphere",   on_spawn_primitive, null, null },
		{ "primitive-cone",     on_spawn_primitive, null, null },
		{ "primitive-cylinder", on_spawn_primitive, null, null },
		{ "primitive-plane",    on_spawn_primitive, null, null },
		{ "camera",             on_spawn_primitive, null, null },
		{ "light",              on_spawn_primitive, null, null },
		{ "sound-source",       on_spawn_primitive, null, null },
		{ "unit-empty",         on_spawn_unit,      null, null },
		{ "shading-environment",on_spawn_unit,      null, null },
	};

	public const GLib.ActionEntry[] action_entries_camera =
	{
		{ "menu-camera",           null,                     null, null },
		{ "camera-frame-all",      on_camera_frame_all,      null, null }
	};

	public const GLib.ActionEntry[] action_entries_view =
	{
		{ "menu-view",           null,                   null, null    },
		{ "project-browser",     on_project_browser,     null, null    },
		{ "console",             on_console,             null, null    },
		{ "statusbar",           on_statusbar,           null, null    },
		{ "inspector",           on_inspector,           null, null    },
		{ "debug-render-world",  on_debug_render_world,  null, "false" },
		{ "debug-physics-world", on_debug_physics_world, null, "false" }
	};

	public const GLib.ActionEntry[] action_entries_debug =
	{
		{ "menu-debug",          null,                   null, null },
		{ "test-level",          on_run_game,            null, null },
		{ "run-game",            on_run_game,            null, null },
		{ "build-data",          on_build_data,          null, null },
		{ "reload-all",          on_reload_all,          null, null },
		{ "restart-backend",     on_restart_backend,     null, null },
		{ "restart-editor-view", on_restart_editor_view, null, null }
	};

	public const GLib.ActionEntry[] action_entries_help =
	{
		{ "menu-help",    null,            null, null },
		{ "manual",       on_manual,       null, null },
		{ "report-issue", on_report_issue, null, null },
		{ "browse-logs",  on_browse_logs,  null, null },
		{ "changelog",    on_changelog,    null, null },
		{ "donate",       on_donate,       null, null },
		{ "credits",      on_credits,      null, null }
	};

	public const GLib.ActionEntry[] action_entries_project =
	{
		{ "delete-file",          on_delete_file,          "s",     null },
		{ "delete-directory",     on_delete_directory,     "s",     null },
		{ "create-directory",     on_create_directory,     "(ss)",  null },
		{ "create-script",        on_create_script,        "(ssb)", null },
		{ "create-unit",          on_create_unit,          "(ss)",  null },
		{ "create-state-machine", on_create_state_machine, "(sss)", null },
		{ "create-material",      on_create_material,      "(ss)",  null },
		{ "open-containing",      on_open_containing,      "s",     null },
		{ "texture-settings",     on_texture_settings,     "s",     null },
		{ "state-machine-editor", on_state_machine_editor, "s",     null },
		{ "open-object",          on_open_object,          "(ss)",  null },
		{ "reveal-resource",      on_reveal,               "(ss)",  null },
	};

	public const GLib.ActionEntry[] action_entries_package =
	{
		{ "create-package-android", on_create_package_android, "(sississsssi)", null },
		{ "create-package-html5",   on_create_package_html5,   "(sis)",         null },
		{ "create-package-linux",   on_create_package_linux,   "(sis)",         null },
		{ "create-package-windows", on_create_package_windows, "(sis)",         null }
	};

	public const GLib.ActionEntry[] action_entries_unit =
	{
		{ "unit-save-as-prefab", on_unit_save_as_prefab, "(ss)", null },
		{ "open-unit",           on_open_unit,           "s",    null },
	};

	// Command line options
	public uint _launcher_watch_id;
	public string? _source_dir = null;
	public string _level_resource = "";
	public User _user;
	public Hashtable _settings;
	public Hashtable _window_state;

	// Editor state
	public double _grid_size;
	public double _rotation_snap;
	public bool _show_grid;
	public bool _snap_to_grid;
	public bool _debug_render_world;
	public bool _debug_physics_world;
	public ToolType _tool_type;
	public ToolType _tool_type_prev;
	public SnapMode _snap_mode;
	public ReferenceSystem _reference_system;

	// Project state
	public string _placeable_type;
	public string _placeable_name;

	// Accelerators
	public string[] _tool_place_accels;
	public string[] _tool_move_accels;
	public string[] _tool_rotate_accels;
	public string[] _tool_scale_accels;
	public string[] _delete_accels;
	public string[] _camera_view_perspective_accels;
	public string[] _camera_view_front_accels;
	public string[] _camera_view_back_accels;
	public string[] _camera_view_right_accels;
	public string[] _camera_view_left_accels;
	public string[] _camera_view_top_accels;
	public string[] _camera_view_bottom_accels;
	public string[] _camera_frame_selected_accels;
	public string[] _camera_frame_all_accels;

	// Engine connections
	public Gee.ArrayList<RuntimeInstance> _runtimes;
	public RuntimeInstance _compiler;
	public RuntimeInstance _editor;
	public RuntimeInstance _game;
	public RuntimeInstance _thumbnail;

	// Level data
	public DatabaseEditor _database_editor;
	public Database _database;
	public Project _project;
	public ProjectStore _project_store;
	public Level _level;
	public DataCompiler _data_compiler;

	// Widgets
	public Gtk.CssProvider _css_provider;
	public ProjectBrowser _project_browser;
	public EditorViewport _editor_viewport;
	public LevelTreeView _level_treeview;
	public LevelLayersTreeView _level_layers_treeview;
	public PropertiesView _properties_view;
	public PreferencesDialog _preferences_dialog;
	public DeployDialog _deploy_dialog;
	public TextureSettingsDialog _texture_settings_dialog;
	public UnitEditor _unit_editor_dialog;
	public StateMachineEditor _state_machine_editor;
	public ObjectEditor _object_editor;
	public ThumbnailCache _thumbnail_cache;

	public Gtk.Stack _project_stack;
	public Gtk.Stack _editor_stack;
	public Gtk.Stack _inspector_stack;

	public Toolbar _toolbar;
	public Gtk.Image _game_run_stop_image;
	public Gtk.Button _game_run;
	public Gtk.Notebook _level_tree_view_notebook;
	public Gtk.Notebook _console_notebook;
	public Gtk.Notebook _project_notebook;
	public Gtk.Notebook _inspector_notebook;
	public Gtk.Paned _editor_pane;
	public Gtk.Paned _content_pane;
	public Gtk.Paned _inspector_pane;
	public Gtk.Paned _main_pane;
	public Statusbar _statusbar;
	public Gtk.Box _main_vbox;
	public Gtk.FileFilter _file_filter;
	public Gtk.ComboBoxText _combo;
	public NewProject _new_project;
	public ProjectsList _projects_list;
	public Gtk.Stack _main_stack;
	public Gtk.HeaderBar _header_bar;
	public LevelEditorWindow _level_editor_window;

	public uint _save_timer_id;

	public signal void ui_read_selection(Guid?[] selection);

	public LevelEditorApplication()
	{
		Object(application_id: "org.crownengine.Crown"
			, flags: GLib.ApplicationFlags.FLAGS_NONE
			);

		GLib.Environment.set_prgname(this.application_id); // FIXME: Drop after GTK4 port.
	}

	public Theme theme_name_to_enum(string theme)
	{
		if (theme == "dark")
			return Theme.DARK;
		else if (theme == "light")
			return Theme.LIGHT;
		else
			return Theme.COUNT;
	}

	public void set_theme_from_name(string theme_name)
	{
		Theme theme = theme_name_to_enum(theme_name);
		set_theme(theme);
	}

	public void set_theme(Theme theme)
	{
		if (theme == Theme.COUNT)
			return;

		string css = "/org/crownengine/Crown/ui/style-%s.css".printf(theme == Theme.DARK ? "dark" : "light");
		_css_provider.load_from_resource(css);
	}

	public override void startup()
	{
		base.startup();

		Intl.setlocale(LocaleCategory.ALL, "C");

		_css_provider = new Gtk.CssProvider();
		var default_screen = Gdk.Display.get_default().get_default_screen();
		Gtk.StyleContext.add_provider_for_screen(default_screen
			, _css_provider
			, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
			);

		try {
			_settings = SJSON.load_from_path(_settings_file.get_path());
		} catch (JsonSyntaxError e) {
			loge(e.message);
		}
		try {
			_window_state = SJSON.load_from_path(_window_state_file.get_path());
		} catch (JsonSyntaxError e) {
			loge(e.message);
		}

		this.add_action_entries(action_entries_file, this);
		this.add_action_entries(action_entries_edit, this);
		this.add_action_entries(action_entries_create, this);
		this.add_action_entries(action_entries_camera, this);
		this.add_action_entries(action_entries_view, this);
		this.add_action_entries(action_entries_debug, this);
		this.add_action_entries(action_entries_help, this);
		this.add_action_entries(action_entries_project, this);
		this.add_action_entries(action_entries_package, this);
		this.add_action_entries(action_entries_unit, this);

		this.set_accels_for_action("app.new-level", { "<Primary>N" });
		this.set_accels_for_action("app.open-level", new string[] { "<Primary>O" });
		this.set_accels_for_action("app.save", new string[] { "<Primary>S" });
		this.set_accels_for_action("app.save-as", new string[] { "<Shift><Primary>S" });
		this.set_accels_for_action("app.import-null", new string[] { "<Primary>I" });
		this.set_accels_for_action("app.close-project", new string[] { "<Primary>W" });
		this.set_accels_for_action("app.quit", new string[] { "<Primary>Q" });

		this.set_accels_for_action("database.undo", new string[] { "<Primary>Z" });
		this.set_accels_for_action("database.redo", new string[] { "<Shift><Primary>Z" });
		this.set_accels_for_action("database.duplicate", new string[] { "<Primary>D" });
		this.set_accels_for_action("database.delete", new string[] { "Delete" });

		this.set_accels_for_action("app.tool(0)", new string[] { "Q" });
		this.set_accels_for_action("app.tool(1)", new string[] { "W" });
		this.set_accels_for_action("app.tool(2)", new string[] { "E" });
		this.set_accels_for_action("app.tool(3)", new string[] { "R" });

		this.set_accels_for_action("app.grid-size(0)", new string[] { "<Primary>G" });
		this.set_accels_for_action("app.rotation-snap-size(0)", new string[] { "<Primary>H" });

		this.set_accels_for_action("viewport.camera-view(0)", new string[] { "KP_5" });
		this.set_accels_for_action("viewport.camera-view(1)", new string[] { "KP_1" });
		this.set_accels_for_action("viewport.camera-view(2)", new string[] { "<Primary>KP_1" });
		this.set_accels_for_action("viewport.camera-view(3)", new string[] { "KP_3" });
		this.set_accels_for_action("viewport.camera-view(4)", new string[] { "<Primary>KP_3" });
		this.set_accels_for_action("viewport.camera-view(5)", new string[] { "KP_7" });
		this.set_accels_for_action("viewport.camera-view(6)", new string[] { "<Primary>KP_7" });
		this.set_accels_for_action("viewport.camera-frame-selected", new string[] { "F" });

		this.set_accels_for_action("app.camera-frame-all", new string[] { "A" });

		this.set_accels_for_action("app.console", new string[] { "<Primary>grave" });
		this.set_accels_for_action("win.fullscreen", new string[] { "F11" });

		this.set_accels_for_action("app.test-level", new string[] { "F5" });
		this.set_accels_for_action("app.build-data", new string[] { "<Primary>B" });
		this.set_accels_for_action("app.reload-all", new string[] { "F7" });

		this.set_accels_for_action("app.manual", new string[] { "F1" });

		_tool_place_accels = this.get_accels_for_action("app.tool(0)");
		_tool_move_accels = this.get_accels_for_action("app.tool(1)");
		_tool_rotate_accels = this.get_accels_for_action("app.tool(2)");
		_tool_scale_accels = this.get_accels_for_action("app.tool(3)");
		_delete_accels = this.get_accels_for_action("app.delete");
		_camera_view_perspective_accels = this.get_accels_for_action("viewport.camera-view(0)");
		_camera_view_front_accels = this.get_accels_for_action("viewport.camera-view(1)");
		_camera_view_back_accels = this.get_accels_for_action("viewport.camera-view(2)");
		_camera_view_right_accels = this.get_accels_for_action("viewport.camera-view(3)");
		_camera_view_left_accels = this.get_accels_for_action("viewport.camera-view(4)");
		_camera_view_top_accels = this.get_accels_for_action("viewport.camera-view(5)");
		_camera_view_bottom_accels = this.get_accels_for_action("viewport.camera-view(6)");
		_camera_frame_selected_accels = this.get_accels_for_action("viewport.camera-frame-selected");
		_camera_frame_all_accels = this.get_accels_for_action("app.camera-frame-all");

		_runtimes = new Gee.ArrayList<RuntimeInstance>();

		_compiler = new RuntimeInstance("data_compiler");
		_compiler.message_received.connect(on_message_received);
		_compiler.connected.connect(on_runtime_connected);
		_compiler.disconnected.connect(on_runtime_disconnected);
		_compiler.disconnected_unexpected.connect(on_data_compiler_disconnected_unexpected);

		_data_compiler = new DataCompiler(_compiler);
		_data_compiler.start.connect(on_data_compiler_start);
		_data_compiler.finished.connect(on_data_compiler_finished);

		_project = new Project();
		_project.set_toolchain_dir(_toolchain_dir.get_path());
		_project.register_importer("Sprite", { "png" }, SpriteResource.import, on_import_result, 0.0);
		_project.register_importer("Mesh", { "mesh", "fbx" }, MeshResource.import, on_import_result, 1.0);
		_project.register_importer("Sound", { "wav", "ogg" }, SoundResource.import, on_import_result, 2.0);
		_project.register_importer("Texture", { "dds", "exr", "jpg", "ktx", "png", "pvr", "tga", }, TextureResource.import, on_import_result, 2.0);
		_project.register_importer("Font", { "ttf", "otf" }, FontResource.import, on_import_result, 3.0);
		_project.project_reset.connect(on_project_reset);
		_project.project_loaded.connect(on_project_loaded);

		_preferences_dialog = new PreferencesDialog();
		_preferences_dialog.delete_event.connect(_preferences_dialog.hide_on_delete);
		_preferences_dialog.decode(_settings);

		_database_editor = new DatabaseEditor(_project, (uint)_preferences_dialog._undo_redo_max_size.value * 1024 * 1024);
		_database_editor.undo.connect((id) => { _statusbar.set_temporary_message("Undo: " + ActionNames[id]); });
		_database_editor.redo.connect((id) => { _statusbar.set_temporary_message("Redo: " + ActionNames[id]); });
		_database_editor.selection_changed.connect(on_selection_changed);

		_editor_viewport = new EditorViewport("editor"
			, _database_editor
			, _project
			, LEVEL_EDITOR_BOOT_DIR
			, EDITOR_ADDRESS
			, EDITOR_TCP_PORT
			);
		_editor = _editor_viewport._runtime;
		_editor.message_received.connect(on_message_received);
		_editor.connected.connect(on_editor_connected);
		_editor.disconnected.connect(on_runtime_disconnected);
		_editor.disconnected_unexpected.connect(on_editor_disconnected_unexpected);
		_runtimes.add(_editor);

		_preferences_dialog.set_runtime(_editor);

		set_theme_from_name(_preferences_dialog._theme_combo.value);

		_game = new RuntimeInstance("game");
		_game.message_received.connect(on_message_received);
		_game.connected.connect(on_game_connected);
		_game.disconnected.connect(on_game_disconnected);
		_game.disconnected_unexpected.connect(on_game_disconnected);
		_runtimes.add(_game);

		_thumbnail = new RuntimeInstance("thumbnail");
		_thumbnail.message_received.connect(on_message_received);
		_thumbnail.connected.connect(on_runtime_connected);
		_thumbnail.disconnected.connect(on_runtime_disconnected);
		_thumbnail.disconnected_unexpected.connect(on_runtime_disconnected_unexpected);
		_runtimes.add(_thumbnail);

		_database = _database_editor._database;
		_database.objects_created.connect(on_objects_created);
		_database.objects_destroyed.connect(on_objects_destroyed);
		_database.objects_changed.connect(on_objects_changed);
		_database.object_type_added.connect(on_object_type_added);

		_properties_view = new PropertiesView(_database_editor);

		create_object_types(_database);

		_properties_view.register_object_type(OBJECT_TYPE_UNIT, new UnitView(_database));

		_level = new Level(_database, _editor);

		// Editor state
		_grid_size = 1.0;
		_rotation_snap = 15.0;
		_show_grid = true;
		_snap_to_grid = false;
		_debug_render_world = false;
		_debug_physics_world = false;
		_tool_type = ToolType.MOVE;
		_tool_type_prev = _tool_type;
		_snap_mode = SnapMode.RELATIVE;
		_reference_system = ReferenceSystem.LOCAL;

		// Project state
		_placeable_type = "";
		_placeable_name = "";

		_project_store = new ProjectStore(_project);

		// Widgets
		_combo = new Gtk.ComboBoxText();
		_combo.append("editor", "Editor");
		_combo.append("game", "Game");
		_combo.set_active_id("editor");
		_combo.set_size_request(50, -1);

		_console_view = new ConsoleView(_project, _combo, _preferences_dialog);
		_thumbnail_cache = new ThumbnailCache(_project, _thumbnail, (uint)_preferences_dialog._thumbnail_cache_max_size.value * 1024 * 1024);
		_project_browser = new ProjectBrowser(_project_store, _thumbnail_cache);

		_level_treeview = new LevelTreeView(_database, _level);
		_level_treeview.selection_changed.connect(on_level_treeview_selection_changed);
		this.ui_read_selection.connect(_level_treeview.read_selection);
		this.ui_read_selection.connect(_properties_view.set_objects);

		_database.objects_created.connect(_level_treeview.on_objects_created);
		_database.objects_destroyed.connect(_level_treeview.on_objects_destroyed);
		_database.objects_changed.connect(_level_treeview.on_objects_changed);

		_level_layers_treeview = new LevelLayersTreeView(_database, _level);

		_level_tree_view_notebook = new Gtk.Notebook();
		_level_tree_view_notebook.show_border = false;
		_level_tree_view_notebook.append_page(_level_treeview, new Gtk.Image.from_icon_name("level-tree", Gtk.IconSize.SMALL_TOOLBAR));
		_level_tree_view_notebook.append_page(_level_layers_treeview, new Gtk.Image.from_icon_name("level-layers", Gtk.IconSize.SMALL_TOOLBAR));

		_inspector_notebook = new Gtk.Notebook();
		_inspector_notebook.show_border = false;
		_inspector_notebook.append_page(_properties_view, new Gtk.Label.with_mnemonic("Properties"));

		_inspector_pane = new Gtk.Paned(Gtk.Orientation.VERTICAL);
		_inspector_pane.pack1(_level_tree_view_notebook, true, false);
		_inspector_pane.pack2(_inspector_notebook, false, false);

		_project_stack = make_compiler_stack(_project_browser);
		_editor_stack = make_compiler_stack(_editor_viewport);
		_inspector_stack = make_compiler_stack(_inspector_pane);

		// Game run/stop button.
		_game_run_stop_image = new Gtk.Image.from_icon_name("game-run", Gtk.IconSize.MENU);
		_game_run_stop_image.margin_bottom
			= _game_run_stop_image.margin_end
			= _game_run_stop_image.margin_start
			= _game_run_stop_image.margin_top
			= 8
			;
		_game_run = new Gtk.Button();
		_game_run.add(_game_run_stop_image);
		_game_run.get_style_context().add_class("suggested-action");
		_game_run.get_style_context().add_class("image-button");
		_game_run.action_name = "app.test-level";
		_game_run.can_focus = false;

		_toolbar = new Toolbar();
		_editor_viewport._overlay.add_overlay(_toolbar);

		_console_notebook = new Gtk.Notebook();
		_console_notebook.show_border = false;
		_console_notebook.append_page(_console_view, new Gtk.Label.with_mnemonic("Console"));

		_project_notebook = new Gtk.Notebook();
		_project_notebook.show_border = false;
		_project_notebook.append_page(_project_stack, new Gtk.Label.with_mnemonic("Project"));

		_content_pane = new Gtk.Paned(Gtk.Orientation.VERTICAL);
		_content_pane.pack1(_editor_stack, true, false);
		_content_pane.pack2(_console_notebook, false, false);

		_editor_pane = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
		_editor_pane.pack1(_project_notebook, false, false);
		_editor_pane.pack2(_content_pane, true, false);

		_main_pane = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
		_main_pane.pack1(_editor_pane, true, false);
		_main_pane.pack2(_inspector_stack, false, false);

		_statusbar = new Statusbar();

		_main_vbox = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_main_vbox.pack_start(_main_pane, true, true, 0);
		_main_vbox.pack_start(_statusbar, false, false, 0);
		_main_vbox.set_visible(true);

		_file_filter = new Gtk.FileFilter();
		_file_filter.set_filter_name("Level (*.level)");
		_file_filter.add_pattern("*.level");

		_user = new User();
		_new_project = new NewProject(_user, _project);
		_new_project.fill_templates_list(_templates_dir.get_path());

		_projects_list = new ProjectsList(_user);
		_projects_list.set_visible(true); // To make Gtk.Stack work...

		_main_stack = new Gtk.Stack();
		_main_stack.add_named(new Gtk.Label("Waiting for %s...".printf(CROWN_SUBPROCESS_LAUNCHER)), PANEL_WAITING);
		_main_stack.add_named(_projects_list, PANEL_PROJECTS_LIST);
		_main_stack.add_named(_new_project, PANEL_NEW_PROJECT);
		_main_stack.add_named(_main_vbox, PANEL_EDITOR);

		_header_bar = new Gtk.HeaderBar();
		_header_bar.show_close_button = true;
		_header_bar.has_subtitle = false;
		_header_bar.pack_start(_game_run);

		// Delete expired logs
		if (_preferences_dialog._log_delete_after_days.value != 0) {
			try {
				FileEnumerator enumerator = _logs_dir.enumerate_children("standard::*"
					, FileQueryInfoFlags.NOFOLLOW_SYMLINKS
					);
				GLib.FileInfo info = null;
				while ((info = enumerator.next_file()) != null) {
					if (info.get_file_type() != GLib.FileType.REGULAR)
						continue; // Skip anything but regular files

					// Parse DateTime from log filename
					int year = 1970;
					int month = 1;
					int day = 1;
					if (info.get_name().scanf("%d-%d-%d.log", &year, &month, &day) != 3)
						continue; // Skip malformed filenames

					GLib.DateTime time_log = new GLib.DateTime.local(year, month, day, 0, 0, 0.0);
					if (time_log == null)
						continue; // Skip invalid dates

					GLib.DateTime time_now = new GLib.DateTime.now_local();
					if (time_now.difference(time_log) <= GLib.TimeSpan.DAY*_preferences_dialog._log_delete_after_days.value)
						continue; // Skip if date is within range

					// Delete
					GLib.File log_file = _logs_dir.resolve_relative_path(info.get_name());
					log_file.delete();
				}
			} catch (GLib.Error e) {
				loge(e.message);
			}
		}

		_user.load(_user_file.get_path());
		_console_view._entry_history.load(_console_history_file.get_path());

		show_panel(PANEL_WAITING);

		this.set_menubar(make_menubar());
	}

	public override void activate()
	{
		if (_level_editor_window == null) {
			_level_editor_window = new LevelEditorWindow(this, _header_bar);
			if (_window_state.has_key("level_editor_window"))
				_level_editor_window.decode((Hashtable)_window_state["level_editor_window"]);
			_level_editor_window.add(_main_stack);
			_level_editor_window.insert_action_group("viewport", _editor_viewport._action_group);
			_level_editor_window.insert_action_group("database", _database_editor._action_group);

			try {
				_level_editor_window.icon = Gtk.IconTheme.get_default().load_icon(CROWN_EDITOR_ICON_NAME, 256, 0);
			} catch (Error e) {
				loge(e.message);
			}
		}

		this.active_window.show_all();

		// Register a callback to be called when SubprocessLauncher service 'appears'.
		_launcher_watch_id = GLib.Bus.watch_name(GLib.BusType.SESSION
			, CROWN_SUBPROCESS_LAUNCHER
			, GLib.BusNameWatcherFlags.NONE
			, on_subprocess_launcher_appeared
			);
	}

	public void on_subprocess_launcher_appeared(GLib.DBusConnection connection, string name, string name_owner)
	{
		try {
			GLib.Bus.unwatch_name(_launcher_watch_id);

			// Connect to SubprocessLauncher service.
			_subprocess_launcher = GLib.Bus.get_proxy_sync(GLib.BusType.SESSION
				, CROWN_SUBPROCESS_LAUNCHER
				, "/org/crownengine/subprocess_launcher"
				);

			if (_source_dir == null) {
				show_panel(PANEL_PROJECTS_LIST);
			} else {
				show_panel(PANEL_EDITOR);
				restart_backend.begin();
			}
		} catch (IOError e) {
			loge(e.message);
		}
	}

	public override bool local_command_line(ref unowned string[] args, out int exit_status)
	{
		if (args.length > 1) {
			if (!GLib.FileUtils.test(args[1], FileTest.EXISTS) || !GLib.FileUtils.test(args[1], FileTest.IS_DIR)) {
				loge("Source directory does not exist or it is not a directory");
				exit_status = 1;
				return true;
			}

			_source_dir = args[1];
		}

		if (args.length > 2) {
			// Validation is done below after the Project object instantiation
			_level_resource = args[2];
		}

		exit_status = 0;
		return false;
	}

	public override int command_line(ApplicationCommandLine command_line)
	{
		this.activate();
		return 0;
	}

	public RuntimeInstance? current_selected_runtime()
	{
		if (_combo.get_active_id() == "editor")
			return _editor;
		else if (_combo.get_active_id() == "game")
			return _game;
		else
			return null;
	}

	public void on_runtime_connected(RuntimeInstance ri, string address, int port)
	{
		ri._revision = _data_compiler._revision;

		logi("Connected to %s@%s:%d".printf(ri._name, address, port));
	}

	public void on_runtime_disconnected(RuntimeInstance ri)
	{
		logi("Disconnected from %s".printf(ri._name));
	}

	public void on_runtime_disconnected_unexpected(RuntimeInstance ri)
	{
		logw("Disconnected from %s unexpectedly".printf(ri._name));
	}

	public async void on_data_compiler_disconnected_unexpected(RuntimeInstance ri)
	{
		on_runtime_disconnected_unexpected(ri);

		yield stop_heads();

		// Reset the callback
		_data_compiler.compile_finished(false, 0);

		_project_stack.set_visible_child_name(COMPILER_CRASHED);
		_editor_stack.set_visible_child_name(COMPILER_CRASHED);
		_inspector_stack.set_visible_child_name(COMPILER_CRASHED);
	}

	public void on_data_compiler_start()
	{
		_statusbar.set_status("Compiling data...");
	}

	public void on_data_compiler_finished(bool success)
	{
		_statusbar.clear_status();

		if (!success) {
			_statusbar.set_temporary_message("Failed to compile data");
			return;
		}

		_project.data_compiled();
	}

	public void on_editor_connected(RuntimeInstance ri, string address, int port)
	{
		on_runtime_connected(ri, address, port);

		// Update editor view with current editor state.
		_level.send_level();
		_database_editor.send_selection(_editor);
		send_state();
		_preferences_dialog.apply();
		_editor.send(DeviceApi.frame());
	}

	public void on_editor_disconnected_unexpected(RuntimeInstance ri)
	{
		on_runtime_disconnected_unexpected(ri);
	}

	public void on_game_connected(RuntimeInstance ri, string address, int port)
	{
		on_runtime_connected(ri, address, port);

		_combo.set_active_id("game");
	}

	public void on_game_disconnected(RuntimeInstance ri)
	{
		on_runtime_disconnected(ri);

		_combo.set_active_id("editor");
		_game_run_stop_image.set_from_icon_name("game-run", Gtk.IconSize.MENU);
	}

	public void on_message_received(RuntimeInstance ri, ConsoleClient client, uint8[] json)
	{
		try {
			Hashtable msg = JSON.decode(json) as Hashtable;
			handle_message(ri, client, (string)msg["type"], msg);
		} catch (JsonSyntaxError e) {
			loge(e.message);
		}
	}

	public void handle_message(RuntimeInstance ri, ConsoleClient client, string msg_type, Hashtable msg)
	{
		if (msg_type == "message") {
			string system = ri._name + ": " + (string)msg["system"];
			log(system, (string)msg["severity"], (string)msg["message"]);
		} else if (msg_type == "add_file") {
			string path = (string)msg["path"];
			uint64 size = uint64.parse((string)msg["size"]);
			uint64 mtime = uint64.parse((string)msg["mtime"]);

			_project.add_file(path, size, mtime);
		} else if (msg_type == "remove_file") {
			string path = (string)msg["path"];

			_database.set(0, GUID_ZERO, path, null);
			_project.remove_file(path);
		} else if (msg_type == "add_tree") {
			string path = (string)msg["path"];

			_project.add_tree(path);
		} else if (msg_type == "remove_tree") {
			string path = (string)msg["path"];

			_project.remove_tree(path);
		} else if (msg_type == "change_file") {
			string path = (string)msg["path"];
			uint64 size = uint64.parse((string)msg["size"]);
			uint64 mtime = uint64.parse((string)msg["mtime"]);

			_database.set(0, GUID_ZERO, path, null);
			_project.change_file(path, size, mtime);
		} else if (msg_type == "compile") {
			_data_compiler.message(msg);
		} else if (msg_type == "refresh") {
			ri.refresh_finished((bool)msg["success"]);
		} else if (msg_type == "refresh_list") {
			_data_compiler.refresh_list_finished((Gee.ArrayList<Value?>)msg["list"]);
		} else if (msg_type == "unit_spawned") {
			Guid id = Guid.parse((string)msg["id"]);
			string name = (string)msg["name"];
			Gee.ArrayList<Value?> pos = (Gee.ArrayList<Value?>)msg["position"];
			Gee.ArrayList<Value?> rot = (Gee.ArrayList<Value?>)msg["rotation"];
			Gee.ArrayList<Value?> scl = (Gee.ArrayList<Value?>)msg["scale"];

			_level.on_unit_spawned(id
				, name
				, Vector3.from_array(pos)
				, Quaternion.from_array(rot)
				, Vector3.from_array(scl)
				);

			_database_editor.selection_set({ id });
			_database.add_restore_point((int)ActionType.CREATE_OBJECTS, new Guid?[] { id }, ActionTypeFlags.FROM_SERVER);
		} else if (msg_type == "sound_spawned") {
			Guid id = Guid.parse((string)msg["id"]);
			string name = (string)msg["name"];
			Gee.ArrayList<Value?> pos = (Gee.ArrayList<Value?>)msg["position"];
			Gee.ArrayList<Value?> rot = (Gee.ArrayList<Value?>)msg["rotation"];
			Gee.ArrayList<Value?> scl = (Gee.ArrayList<Value?>)msg["scale"];
			double range = (double)msg["range"];
			double volume = (double)msg["volume"];
			bool loop = (bool)msg["loop"];

			_level.on_sound_spawned(id
				, name
				, Vector3.from_array(pos)
				, Quaternion.from_array(rot)
				, Vector3.from_array(scl)
				, range
				, volume
				, loop
				);
			_database.add_restore_point((int)ActionType.CREATE_OBJECTS, new Guid?[] { id }, ActionTypeFlags.FROM_SERVER);
		} else if (msg_type == "move_objects") {
			Hashtable ids           = (Hashtable)msg["ids"];
			Hashtable new_positions = (Hashtable)msg["new_positions"];
			Hashtable new_rotations = (Hashtable)msg["new_rotations"];
			Hashtable new_scales    = (Hashtable)msg["new_scales"];

			Gee.ArrayList<string> keys = new Gee.ArrayList<string>.wrap(ids.keys.to_array());
			keys.sort(Gee.Functions.get_compare_func_for(typeof(string)));

			Guid?[] n_ids            = new Guid?[keys.size];
			Vector3[] n_positions    = new Vector3[keys.size];
			Quaternion[] n_rotations = new Quaternion[keys.size];
			Vector3[] n_scales       = new Vector3[keys.size];

			for (int i = 0; i < keys.size; ++i) {
				string k = keys[i];

				n_ids[i]       = Guid.parse((string)ids[k]);
				n_positions[i] = Vector3.from_array((Gee.ArrayList<Value?>)(new_positions[k]));
				n_rotations[i] = Quaternion.from_array((Gee.ArrayList<Value?>)new_rotations[k]);
				n_scales[i]    = Vector3.from_array((Gee.ArrayList<Value?>)new_scales[k]);
			}

			_level.on_move_objects(n_ids, n_positions, n_rotations, n_scales);
			_database.add_restore_point((int)ActionType.CHANGE_OBJECTS, n_ids, ActionTypeFlags.FROM_SERVER);
		} else if (msg_type == "selection") {
			Hashtable objects = (Hashtable)msg["objects"];

			Gee.ArrayList<string> keys = new Gee.ArrayList<string>.wrap(objects.keys.to_array());
			keys.sort(Gee.Functions.get_compare_func_for(typeof(string)));

			Guid?[] ids = new Guid?[keys.size];

			for (int i = 0; i < keys.size; ++i) {
				string k = keys[i];
				ids[i] = Guid.parse((string)objects[k]);
			}

			_database_editor.selection_read(ids);
			ui_read_selection(_database_editor._selection.to_array());
		} else if (msg_type == "camera") {
			if (ri == _editor)
				_level.on_camera(msg);
		} else if (msg_type == "error") {
			loge((string)msg["message"]);
		} else if (msg_type == "thumbnail") {
			string resource_type = (string)msg["resource_type"];
			string resource_name = (string)msg["resource_name"];
			string path = (string)msg["path"];
			_thumbnail_cache.thumbnail_ready(resource_type, resource_name, path);
		} else {
			loge("Unknown message type: " + msg_type);
		}
	}

	public void append_editor_state(StringBuilder sb)
	{
		// This state is common to any project.
		sb.append(LevelEditorApi.set_grid_size(_grid_size));
		sb.append(LevelEditorApi.set_rotation_snap(_rotation_snap));
		sb.append(LevelEditorApi.enable_show_grid(_show_grid));
		sb.append(LevelEditorApi.enable_snap_to_grid(_snap_to_grid));
		sb.append(LevelEditorApi.enable_debug_render_world(_debug_render_world));
		sb.append(LevelEditorApi.enable_debug_physics_world(_debug_physics_world));
		sb.append(LevelEditorApi.set_tool_type(_tool_type));
		sb.append(LevelEditorApi.set_snap_mode(_snap_mode));
		sb.append(LevelEditorApi.set_reference_system(_reference_system));
	}

	public void append_project_state(StringBuilder sb)
	{
		// This state is not guaranteed to be applicable to any project.
		if (_placeable_type != "")
			sb.append(LevelEditorApi.set_placeable(_placeable_type, _placeable_name));
	}

	public void send_state()
	{
		StringBuilder sb = new StringBuilder();
		append_editor_state(sb);
		append_project_state(sb);
		_editor.send_script(sb.str);
	}

	public void on_objects_created(Guid?[] object_ids, uint32 flags)
	{
		if ((flags & ActionTypeFlags.FROM_SERVER) == 0) {
			StringBuilder sb = new StringBuilder();
			_level.generate_spawn_objects(sb, object_ids);
			if (sb.len > 0) {
				_editor.send_script(sb.str);
				_editor_viewport.frame();
			}
		}

		ui_read_selection(_database_editor._selection.to_array());
		update_active_window_title();
	}

	public void on_objects_destroyed(Guid?[] object_ids, uint32 flags = 0)
	{
		if ((flags & ActionTypeFlags.FROM_SERVER) == 0) {
			StringBuilder sb = new StringBuilder();
			_level.generate_destroy_objects(sb, object_ids);
			if (sb.len > 0) {
				_editor.send_script(sb.str);
				_editor_viewport.frame();
			}
		}

		ui_read_selection(_database_editor._selection.to_array());
		update_active_window_title();
	}

	public void on_objects_changed(Guid?[] object_ids, uint32 flags = 0)
	{
		if ((flags & ActionTypeFlags.FROM_SERVER) == 0) {
			StringBuilder sb = new StringBuilder();
			_level.generate_change_objects(sb, object_ids);
			if (sb.len > 0) {
				_editor.send_script(sb.str);
				_editor_viewport.frame();
			}
		}

		ui_read_selection(_database_editor._selection.to_array());
		update_active_window_title();
	}

	public void on_object_type_added(ObjectTypeInfo info)
	{
		if ((info.flags & ObjectTypeFlags.UNIT_COMPONENT) != 0) {
			Unit.register_component_type(info.name, info.user_data != null ? info.user_data : "");
			_properties_view.register_object_type(info.name, null);
		} else if (info.name != OBJECT_TYPE_UNIT) { // FIXME
			_properties_view.register_object_type(info.name, null);
		}
	}

	public async bool restart_backend()
	{
		yield stop_backend();

		// Spawn the data compiler.
		string args[] =
		{
			ENGINE_EXE,
			"--source-dir",
			_project.source_dir(),
			"--data-dir",
			_project.data_dir(),
			"--map-source-dir",
			"core",
			_project.toolchain_dir(),
			"--server",
			"--wait-console"
		};

		try {
			_compiler._process_id = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
		} catch (Error e) {
			loge(e.message);
		}

		_project_stack.set_visible_child_name(COMPILER_CONNECTING);
		_editor_stack.set_visible_child_name(COMPILER_CONNECTING);
		_inspector_stack.set_visible_child_name(COMPILER_CONNECTING);

		int tries = yield _compiler.connect_async(DATA_COMPILER_ADDRESS
			, DATA_COMPILER_TCP_PORT
			, DATA_COMPILER_CONNECTION_TRIES
			, DATA_COMPILER_CONNECTION_INTERVAL
			);
		if (tries == DATA_COMPILER_CONNECTION_TRIES) {
			loge("Cannot connect to data_compiler");
			return false;
		}

		_project_stack.set_visible_child_name(COMPILER_COMPILING_DATA);
		_editor_stack.set_visible_child_name(COMPILER_COMPILING_DATA);
		_inspector_stack.set_visible_child_name(COMPILER_COMPILING_DATA);

		// Compile data.
		bool success = yield _data_compiler.compile(_project.data_dir(), _project.platform());

		if (!success) {
			_project_stack.set_visible_child_name(COMPILER_FAILED_COMPILATION);
			_editor_stack.set_visible_child_name(COMPILER_FAILED_COMPILATION);
			_inspector_stack.set_visible_child_name(COMPILER_FAILED_COMPILATION);
			return success;
		}

		// If successful, start the level editor.
		_project_stack.set_visible_child_name("child");
		_editor_stack.set_visible_child_name("child");
		_inspector_stack.set_visible_child_name("child");

		yield _editor_viewport.restart_runtime();
		yield start_thumbnail();

		_project_stack.set_visible_child(_project_browser);
		_inspector_stack.set_visible_child(_inspector_pane);
		_project_browser.select_project_root();
		return success;
	}

	public async void stop_heads()
	{
		yield _game.stop();
		yield _thumbnail.stop();
		yield _editor_viewport.stop_runtime();
	}

	public async void stop_backend()
	{
		_project_stack.set_visible_child_name(BACKEND_STOPPING);
		_editor_stack.set_visible_child_name(BACKEND_STOPPING);
		_inspector_stack.set_visible_child_name(BACKEND_STOPPING);

		yield stop_heads();
		yield stop_data_compiler();
	}

	public async void stop_data_compiler()
	{
		if (_compiler.is_connected())
			_project.reset_files();

		yield _compiler.stop();
	}

	public int dump_test_level()
	{
		try {
			// Save temporary package to reference test level.
			Gee.ArrayList<Value?> level = new Gee.ArrayList<Value?>();
			level.add("_level_editor_test");
			Hashtable package = new Hashtable();
			package["level"] = level;
			SJSON.save(package, _project._level_editor_test_package.get_path());
		} catch (JsonWriteError e) {
			return -1;
		}

		// Save test level to file.
		return _database.dump(_project._level_editor_test_level.get_path(), _level._id);
	}

	public async void start_game(StartGame sg)
	{
		if (sg == StartGame.TEST && dump_test_level() != 0)
			return;

		bool success = yield _data_compiler.compile(_project.data_dir(), _project.platform());
		_project.delete_garbage();

		if (!success) {
			_game_run_stop_image.set_from_icon_name("game-run", Gtk.IconSize.MENU);
			return;
		}

		// Spawn the game.
		string args[] =
		{
			ENGINE_EXE,
			"--data-dir",
			_project.data_dir(),
			"--console-port",
			GAME_TCP_PORT.to_string(),
			"--wait-console",
			"--lua-string",
			sg == StartGame.TEST ? "TEST=true" : ""
		};

		try {
			_game._process_id = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
		} catch (Error e) {
			loge(e.message);
		}

		// Try to connect to the game.
		int tries = yield _game.connect_async(GAME_ADDRESS
			, GAME_TCP_PORT
			, GAME_CONNECTION_TRIES
			, GAME_CONNECTION_INTERVAL
			);
		if (tries == GAME_CONNECTION_TRIES) {
			loge("Cannot connect to game");
			return;
		}
	}

	public async void start_thumbnail()
	{
		string args[] =
		{
			ENGINE_EXE,
			"--data-dir",
			_project.data_dir(),
			"--boot-dir",
			THUMBNAIL_BOOT_DIR,
			"--console-port",
			THUMBNAIL_TCP_PORT.to_string(),
			"--wait-console",
			"--pumped",
			"--hidden"
		};

		try {
			_thumbnail._process_id = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
		} catch (Error e) {
			loge(e.message);
		}

		// Try to connect to the game.
		int tries = yield _thumbnail.connect_async(THUMBNAIL_ADDRESS
			, THUMBNAIL_TCP_PORT
			, THUMBNAIL_CONNECTION_TRIES
			, THUMBNAIL_CONNECTION_INTERVAL
			);
		if (tries == THUMBNAIL_CONNECTION_TRIES) {
			loge("Cannot connect to thumbnail");
			return;
		}
	}

	public void on_tool(GLib.SimpleAction action, GLib.Variant? param)
	{
		ToolType type = (ToolType)param.get_int32();

		if (type == ToolType.PLACE) {
			// Store previous tool for it to be restored later.
			if (_tool_type != ToolType.PLACE)
				_tool_type_prev = _tool_type;
		}

		_tool_type = type;

		_editor_viewport.grab_focus();
		send_state();
		_editor_viewport.frame();
		action.set_state(param);
	}

	public void on_cancel_place(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_tool_type != ToolType.PLACE)
			return;

		activate_action("tool", new GLib.Variant.int32(_tool_type_prev));
	}

	public void on_snap(GLib.SimpleAction action, GLib.Variant? param)
	{
		_snap_mode = (SnapMode)param.get_int32();

		send_state();
		_editor_viewport.frame();
		action.set_state(param);
	}

	public void on_reference_system(GLib.SimpleAction action, GLib.Variant? param)
	{
		_reference_system = (ReferenceSystem)param.get_int32();

		send_state();
		_editor_viewport.frame();
		action.set_state(param);
	}

	public void on_grid_size(GLib.SimpleAction action, GLib.Variant? param)
	{
		int32 new_size = param.get_int32();

		if (new_size != 0) {
			_grid_size = (double)new_size / 10.0;
			send_state();
			_editor_viewport.frame();
			action.set_state(param);
			return;
		}

		// Custom grid size.
		Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Grid size"
			, this.active_window
			, Gtk.DialogFlags.MODAL
			, "Cancel"
			, Gtk.ResponseType.CANCEL
			, "Ok"
			, Gtk.ResponseType.OK
			, null
			);

		InputDouble sb = new InputDouble(_grid_size, 0.1, 1000);
		sb.activate.connect(() => { dg.response(Gtk.ResponseType.OK); });
		dg.get_content_area().add(sb);

		dg.response.connect((response_id) => {
			if (response_id == Gtk.ResponseType.OK) {
				_grid_size = sb.value;
				send_state();
				_editor_viewport.frame();
				action.set_state(param);
			}
			dg.destroy();
		});

		dg.show_all();
	}

	public void update_active_window_title()
	{
		string title = "";

		if (_level._name != null) {
			if (_database.changed())
				title += " • ";

			title += (_level._name == LEVEL_EMPTY) ? "untitled" : _level._name;
			title += " - ";
		}

		title += CROWN_EDITOR_NAME;

		if (this.active_window.title != title)
			this.active_window.title = title;
	}

	public void load_level(string name)
	{
		if (name == _level._name)
			return;

		if (_level.load(name) != 0) {
			loge("Unable to load level %s".printf(name));
			return;
		}

		if (_editor.is_connected()) {
			_level.send_level();
			send_state();
			_editor_viewport.frame();
		}

		update_active_window_title();
		_level_treeview.set_level(_level);
	}

	public bool do_save(string path)
	{
		string resource_filename = _project.resource_filename(path);
		string resource_path     = ResourceId.normalize(resource_filename);
		string resource_name     = ResourceId.name(resource_path);

		if (_level.save(resource_name) != 0) {
			Gtk.MessageDialog md = new Gtk.MessageDialog(this.active_window
				, Gtk.DialogFlags.MODAL
				, Gtk.MessageType.ERROR
				, Gtk.ButtonsType.NONE
				, "Unable to save level '%s'".printf(resource_name)
				);
			md.add_button("_Ok", Gtk.ResponseType.OK);
			md.set_default_response(Gtk.ResponseType.OK);
			md.response.connect(() => { md.destroy(); });
			md.show_all();
			return false;
		}

		_statusbar.set_temporary_message("Saved %s".printf(_level._path));
		update_active_window_title();
		return true;
	}

	public void save_as(string? filename, string? success_action = null, GLib.Variant? variant = null)
	{
		string path = filename;

		if (path != null) {
			if (do_save(path) && success_action != null)
				GLib.Application.get_default().activate_action(success_action, variant);
		} else {
			SaveResourceDialog srd = new SaveResourceDialog("Save As..."
				, this.active_window
				, OBJECT_TYPE_LEVEL
				, ""
				, _project
				);
			srd.safer_response.connect((response_id, path) => {
					if (response_id == Gtk.ResponseType.ACCEPT && path != null) {
						if (do_save(path) && success_action != null)
							GLib.Application.get_default().activate_action(success_action, variant);
					}
					srd.destroy();
				});
			srd.show_all();
		}
	}

	public void save(string? success_action = null, GLib.Variant? variant = null)
	{
		save_as(_level._path, success_action, variant);
	}

	public bool save_timeout()
	{
		if (_level._path != null)
			save();

		return GLib.Source.CONTINUE;
	}

	public Hashtable encode()
	{
		Hashtable json_obj = new Hashtable();
		json_obj["level_editor_window"] = ((LevelEditorWindow)this.active_window).encode();
		return json_obj;
	}

	public override void shutdown()
	{
		// Disable auto-save.
		if (_save_timer_id > 0)
			GLib.Source.remove(_save_timer_id);

		// Save editor settings.
		_user.save(_user_file.get_path());
		_preferences_dialog.encode(_settings);
		try {
			SJSON.save(_settings, _settings_file.get_path());
		} catch (JsonWriteError e) {
			loge(e.message);
		}
		try {
			SJSON.save(encode(), _window_state_file.get_path());
		} catch (JsonWriteError e) {
			loge(e.message);
		}
		_console_view._entry_history.save(_console_history_file.get_path());

		if (_preferences_dialog != null)
			_preferences_dialog.destroy();

		base.shutdown();
	}

	public void do_new_level()
	{
		if (_level.create(LEVEL_EMPTY) != 0) {
			loge("Unable to create a new level.");
			return;
		}

		if (_editor.is_connected()) {
			_level.send_level();
			send_state();
			_editor_viewport.frame();
		}

		update_active_window_title();
		_level_treeview.set_level(_level);
	}

	public void on_new_level(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (!_database.changed()) {
			do_new_level();
		} else {
			Gtk.Dialog dlg = new_level_changed_dialog(this.active_window);
			dlg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.NO)
						do_new_level();
					else if (response_id == Gtk.ResponseType.YES)
						save("new-level");
					dlg.destroy();
				});
			dlg.show_all();
		}
	}

	public void do_open_level(string path)
	{
		string resource_filename = _project.resource_filename(path);
		string resource_path     = ResourceId.normalize(resource_filename);
		string resource_name     = ResourceId.name(resource_path);

		load_level(resource_name);
	}

	public void on_open_level_from_menubar(GLib.SimpleAction action, GLib.Variant? param)
	{
		OpenResourceDialog dlg = new OpenResourceDialog("Open Level..."
			, this.active_window
			, OBJECT_TYPE_LEVEL
			, _project
			);
		dlg.safer_response.connect((response_id, path) => {
				if (response_id == Gtk.ResponseType.ACCEPT && path != null)
					do_open_level(path);
				dlg.destroy();
			});
		dlg.show_all();
	}

	public void on_open_level(GLib.SimpleAction action, GLib.Variant? param)
	{
		string level_name = param.get_string();
		if (level_name != "" && level_name == _level._name)
			return;

		if (!_database.changed()) {
			if (level_name != "")
				load_level(level_name);
			else // Action invoked from menubar File > Open Level...
				on_open_level_from_menubar(action, param);
		} else {
			Gtk.Dialog dlg = new_level_changed_dialog(this.active_window);
			dlg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.NO) {
						if (level_name != "")
							load_level(level_name);
						else // Action invoked from menubar File > Open Level...
							on_open_level_from_menubar(action, param);
					} else if (response_id == Gtk.ResponseType.YES) {
						save("open-level", param);
					}
					dlg.destroy();
				});
			dlg.show_all();
		}
	}

	public void do_open_project(string source_dir, string? level_name)
	{
		if (_project.source_dir() == source_dir) {
			logi("Project `%s` is open already.".printf(source_dir));
			return;
		}

		logi("Loading project: `%s`...".printf(source_dir));

		if (_project.load(source_dir) != 0) {
			Gtk.MessageDialog md = new Gtk.MessageDialog(this.active_window
				, Gtk.DialogFlags.MODAL
				, Gtk.MessageType.INFO
				, Gtk.ButtonsType.OK
				, "The folder `%s` does not appear to be a valid Crown project.".printf(source_dir)
				);
			md.set_default_response(Gtk.ResponseType.OK);
			md.response.connect(() => { md.destroy(); });
			md.show_all();
			return;
		}

		this.show_panel(PANEL_EDITOR, Gtk.StackTransitionType.NONE);
		_user.add_or_touch_recent_project(source_dir, source_dir);
		_console_view.reset();

		logi("Project `%s` loaded.".printf(source_dir));

		if (level_name == null)
			do_new_level();
		else
			load_level(level_name);

		restart_backend.begin();
	}

	public void open_project(string source_dir, string? level_name = null)
	{
		if (source_dir != "") {
			do_open_project(source_dir, level_name);
		} else {
			Gtk.FileChooserDialog dlg = new_open_project_dialog(this.active_window);
			dlg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.ACCEPT)
						do_open_project(dlg.get_file().get_path(), level_name);
					dlg.destroy();
				});
			dlg.show_all();
		}
	}

	public void on_open_project(GLib.SimpleAction action, GLib.Variant? param)
	{
		string source_dir = "";
		string level_name = LEVEL_EMPTY;

		if (param != null) {
			source_dir = (string)param.get_child_value(0);
			level_name = (string)param.get_child_value(1);
		}

		if (!_database.changed()) {
			open_project(source_dir);
		} else {
			Gtk.Dialog dlg = new_level_changed_dialog(this.active_window);
			dlg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.NO)
						open_project(source_dir, level_name);
					else if (response_id == Gtk.ResponseType.YES)
						save("open-project", new GLib.Variant.tuple({source_dir, level_name}));
					dlg.destroy();
				});
			dlg.show_all();
		}
	}

	public void on_open_projects_list(GLib.SimpleAction action, GLib.Variant? param)
	{
		show_panel(PANEL_PROJECTS_LIST);
	}

	public void do_new_project()
	{
		reset_project();

		stop_backend.begin((obj, res) => {
				stop_backend.end(res);
				show_panel(PANEL_NEW_PROJECT);
			});
	}

	public void on_new_project(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (!_database.changed()) {
			do_new_project();
		} else {
			Gtk.Dialog dlg = new_level_changed_dialog(this.active_window);
			dlg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.NO)
						do_new_project();
					else if (response_id == Gtk.ResponseType.YES)
						save("new-project");
					dlg.destroy();
				});
			dlg.show_all();
		}
	}

	public void on_add_project(GLib.SimpleAction action, GLib.Variant? param)
	{
		Gtk.FileChooserDialog dlg = new_open_project_dialog(this.active_window);
		dlg.response.connect((response_id) => {
				if (response_id == Gtk.ResponseType.ACCEPT) {
					string source_dir = dlg.get_file().get_path();
					_user.add_or_touch_recent_project(source_dir, source_dir);
				}
				dlg.destroy();
			});
		dlg.show_all();
	}

	public void on_remove_project(GLib.SimpleAction action, GLib.Variant? param)
	{
		string source_dir = param.get_string();

		Gtk.MessageDialog md = new Gtk.MessageDialog(this.active_window
			, Gtk.DialogFlags.MODAL
			, Gtk.MessageType.WARNING
			, Gtk.ButtonsType.NONE
			, "Remove \"%s\" from the list?\n\nThis action removes the project from the list only, files on disk will not be deleted.".printf(source_dir)
			);
		md.add_button("_Cancel", Gtk.ResponseType.CANCEL);
		md.add_button("_Remove", Gtk.ResponseType.YES);
		md.set_default_response(Gtk.ResponseType.CANCEL);
		md.response.connect((response_id) => {
				if (response_id == Gtk.ResponseType.YES) {
					_user.remove_recent_project(source_dir);
				}

				md.destroy();
			});
		md.show_all();
	}

	public void on_save(GLib.SimpleAction action, GLib.Variant? param)
	{
		save();
	}

	public void on_save_as(GLib.SimpleAction action, GLib.Variant? param)
	{
		save_as(null);
	}

	public void on_import_result(ImportResult result)
	{
		if (result == ImportResult.ERROR) {
			loge("Failed to import resource(s)");
			return;
		} else if (result == ImportResult.SUCCESS) {
			compile_and_reload.begin();
		}

		ui_read_selection(_database_editor._selection.to_array());
	}

	public void on_import(GLib.SimpleAction action, GLib.Variant? param)
	{
		string? destination_dir = null;
		string[] filenames = {};

		if (param != null) {
			destination_dir = (string)param.get_child_value(0);
			filenames = (string[])param.get_child_value(1);
		}

		_project.import(destination_dir
			, filenames
			, on_import_result
			, _database
			, this.active_window
			);
	}

	public void on_preferences(GLib.SimpleAction action, GLib.Variant? param)
	{
		_preferences_dialog.set_transient_for(_level_editor_window);
		_preferences_dialog.show_all();
		_preferences_dialog.present();
	}

	public void on_deploy(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_deploy_dialog == null) {
			_deploy_dialog = new DeployDialog(_project, _editor);
			_deploy_dialog.set_transient_for(_level_editor_window);
			_deploy_dialog.delete_event.connect(_deploy_dialog.hide_on_delete);
		}

		_deploy_dialog.show_all();
		_deploy_dialog.present();
	}

	public void on_texture_settings(GLib.SimpleAction action, GLib.Variant? param)
	{
		string texture_name = param.get_string();

		if (_texture_settings_dialog == null) {
			_texture_settings_dialog = new TextureSettingsDialog(_project, _database);
			_texture_settings_dialog.set_transient_for(_level_editor_window);
			_texture_settings_dialog.delete_event.connect(_texture_settings_dialog.hide_on_delete);
			_texture_settings_dialog.texture_saved.connect(() => {
						compile_and_reload.begin();
					});
		}

		_texture_settings_dialog.show_all();
		_texture_settings_dialog.present();
		_texture_settings_dialog.set_texture(texture_name);
	}

	public void on_state_machine_editor(GLib.SimpleAction action, GLib.Variant? param)
	{
		string state_machine_name = param.get_string();

		if (_state_machine_editor == null) {
			_state_machine_editor = new StateMachineEditor(this
				, _project
				, "core/editors/state_machine_editor"
				, "127.0.0.1"
				, 10844
				, (uint)_preferences_dialog._undo_redo_max_size.value * 1024 * 1024
				);
			_state_machine_editor.set_transient_for(_level_editor_window);
			_state_machine_editor.saved.connect(() => {
						compile_and_reload.begin();
					});
			_runtimes.add(_state_machine_editor._runtime);
			this.add_window(_state_machine_editor);
		}

		_state_machine_editor.show_all();
		_state_machine_editor.present();
		_state_machine_editor.set_state_machine(state_machine_name);
	}

	public void on_open_object(GLib.SimpleAction action, GLib.Variant? param)
	{
		string resource_type = (string)param.get_child_value(0);
		string resource_name = (string)param.get_child_value(1);

		if (_object_editor == null) {
			_object_editor = new ObjectEditor(this
				, _project
				, (uint)_preferences_dialog._undo_redo_max_size.value * 1024 * 1024
				);
			_object_editor.set_transient_for(_level_editor_window);
			_object_editor.saved.connect(() => {
						compile_and_reload.begin();
					});
			this.add_window(_object_editor);
		}

		_object_editor.show_all();
		_object_editor.present();
		_object_editor.set_object(resource_type, resource_name);
	}

	public void on_reveal(GLib.SimpleAction action, GLib.Variant? param)
	{
		string type = (string)param.get_child_value(0);
		string name = (string)param.get_child_value(1);

		if (!_project_notebook.is_visible())
			_project_notebook.show_all();

		_project_browser.reveal(type, name);
	}

	public Gtk.Dialog new_level_changed_dialog(Gtk.Window? parent)
	{
		return new_resource_changed_dialog(parent, "Level");
	}

	public Gtk.FileChooserDialog new_open_project_dialog(Gtk.Window? parent)
	{
		return new Gtk.FileChooserDialog("Open Project..."
			, parent
			, Gtk.FileChooserAction.SELECT_FOLDER
			, "Cancel"
			, Gtk.ResponseType.CANCEL
			, "Open"
			, Gtk.ResponseType.ACCEPT
			);
	}

	public void reset_project()
	{
		_placeable_type = OBJECT_TYPE_UNIT;
		_placeable_name = "core/units/primitives/cube";

		_database_editor.clear_selection();
		_level.reset();
		_project.reset();

		this.active_window.title = CROWN_EDITOR_NAME;
	}

	public void do_close_project()
	{
		reset_project();

		stop_backend.begin((obj, res) => {
				stop_backend.end(res);
				show_panel(PANEL_PROJECTS_LIST);
			});
	}

	public void on_close_project(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (!_database.changed()) {
			do_close_project();
		} else {
			Gtk.Dialog dlg = new_level_changed_dialog(this.active_window);
			dlg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.NO)
						do_close_project();
					else if (response_id == Gtk.ResponseType.YES)
						save("close-project");
					dlg.destroy();
				});
			dlg.show_all();
		}
	}

	public void stop_backend_and_quit()
	{
		reset_project();

		stop_backend.begin((obj, res) => {
				stop_backend.end(res);
				this.quit();
			});
	}

	public void on_quit(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (!_database.changed()) {
			stop_backend_and_quit();
		} else {
			Gtk.Dialog dlg = new_level_changed_dialog(this.active_window);
			dlg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.NO)
						stop_backend_and_quit();
					else if (response_id == Gtk.ResponseType.YES)
						save("quit");
					dlg.destroy();
				});
			dlg.show_all();
		}
	}

	public static bool is_image_file(string path)
	{
		return path.has_suffix(".png")
			|| path.has_suffix(".tga")
			;
	}

	public void on_open_resource(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (param == null)
			return;

		GLib.AppInfo? app = null;
		string resource_path  = param.get_string();
		string? resource_type = ResourceId.type(resource_path);
		string? resource_name = ResourceId.name(resource_path);
		if (resource_type == null || resource_name == null)
			return;

		if (resource_type == OBJECT_TYPE_LEVEL) {
			activate_action("open-level", resource_name);
			return;
		} else if (resource_type == OBJECT_TYPE_TEXTURE) {
			activate_action("texture-settings", resource_name);
			return;
		} else if (resource_type == OBJECT_TYPE_UNIT) {
			activate_action("open-unit", resource_name);
			return;
		} else if (resource_type == OBJECT_TYPE_STATE_MACHINE) {
			activate_action("state-machine-editor", resource_name);
			return;
		} else if (resource_type == "lua") {
			app = _preferences_dialog._lua_external_tool_button.get_app_info();
		} else if (is_image_file(resource_path)) {
			app = _preferences_dialog._image_external_tool_button.get_app_info();
		} else if (_database.has_type(StringId64(resource_type))) {
			activate_action("open-object", new GLib.Variant.tuple({ resource_type, resource_name }));
			return;
		}

		GLib.File file = GLib.File.new_for_path(_project.absolute_path(resource_path));
		try {
			if (app == null)
				app = file.query_default_handler();
		} catch (GLib.Error e) {
			// Ignore.
		}

		if (app == null)
			app = GLib.AppInfo.get_default_for_type("text/plain", false);

		if (app != null) {
			GLib.List<GLib.File> files = new GLib.List<GLib.File>();
			files.append(file);
			try {
				app.launch(files, null);
			} catch (GLib.Error e) {
				open_text_editor(file.get_path());
			}
		} else {
			open_text_editor(file.get_path());
		}
	}

	public void copy_string(string str)
	{
		var clip = Gtk.Clipboard.get_default(Gdk.Display.get_default());
		clip.set_text(str, str.length);
#if !CROWN_PLATFORM_WINDOWS
		clip.store();
#endif
	}

	public void on_copy_path(GLib.SimpleAction action, GLib.Variant? param)
	{
		string path  = param.get_string();
		copy_string(_project.absolute_path(path));
	}

	public void on_copy_name(GLib.SimpleAction action, GLib.Variant? param)
	{
		copy_string(param.get_string());
	}

	public void on_show_grid(GLib.SimpleAction action, GLib.Variant? param)
	{
		_show_grid = !action.get_state().get_boolean();
		send_state();
		_editor_viewport.frame();
		action.set_state(new GLib.Variant.boolean(_show_grid));
	}

	public void on_rotation_snap_size(GLib.SimpleAction action, GLib.Variant? param)
	{
		int32 new_size = param.get_int32();

		if (new_size != 0) {
			_rotation_snap = (double)new_size;
			send_state();
			_editor_viewport.frame();
			action.set_state(param);
			return;
		}

		// Custom rotation size.
		Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Rotation snap"
			, this.active_window
			, Gtk.DialogFlags.MODAL
			, "Cancel"
			, Gtk.ResponseType.CANCEL
			, "Ok"
			, Gtk.ResponseType.OK
			, null
			);

		InputDouble sb = new InputDouble(_rotation_snap, 1.0, 180.0);
		sb.activate.connect(() => { dg.response(Gtk.ResponseType.OK); });
		dg.get_content_area().add(sb);

		dg.response.connect((response_id) => {
				if (response_id == Gtk.ResponseType.OK) {
					_rotation_snap = sb.value;
					send_state();
					_editor_viewport.frame();
					action.set_state(param);
				}
				dg.destroy();
			});

		dg.show_all();
	}

	public void on_spawn_primitive(GLib.SimpleAction action, GLib.Variant? param)
	{
		GLib.Variant[] paramz;

		if (action.name == "primitive-cube")
			paramz = { OBJECT_TYPE_UNIT, "core/units/primitives/cube" };
		else if (action.name == "primitive-sphere")
			paramz = { OBJECT_TYPE_UNIT, "core/units/primitives/sphere" };
		else if (action.name == "primitive-cone")
			paramz = { OBJECT_TYPE_UNIT, "core/units/primitives/cone" };
		else if (action.name == "primitive-cylinder")
			paramz = { OBJECT_TYPE_UNIT, "core/units/primitives/cylinder" };
		else if (action.name == "primitive-plane")
			paramz = { OBJECT_TYPE_UNIT, "core/units/primitives/plane" };
		else if (action.name == "camera")
			paramz = { OBJECT_TYPE_UNIT, "core/units/camera" };
		else if (action.name == "light")
			paramz = { OBJECT_TYPE_UNIT, "core/units/light" };
		else if (action.name == "sound-source")
			paramz = { OBJECT_TYPE_SOUND, "sound" };
		else
			paramz = { OBJECT_TYPE_UNIT, "core/units/primitives/cube" };

		activate_action("set-placeable", new GLib.Variant.tuple(paramz));
	}

	public void on_spawn_unit(GLib.SimpleAction action, GLib.Variant? param)
	{
		string? unit_name;

		if (action.name == "empty-unit")
			unit_name = null;
		else if (action.name == "shading-environment")
			unit_name = "core/renderer/default_shading_environment";
		else
			unit_name = null;

		_level.spawn_unit(unit_name);
		_editor_viewport.frame();
	}

	public void on_camera_frame_all(GLib.SimpleAction action, GLib.Variant? param)
	{
		Gee.ArrayList<Guid?> all_objects = new Gee.ArrayList<Guid?>();
		_level.objects(ref all_objects);
		_editor.send_script(LevelEditorApi.frame_objects(all_objects.to_array()));
		_editor_viewport.frame();
	}

	public void on_project_browser(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_project_notebook.is_visible()) {
			_project_notebook.hide();
		} else {
			_project_notebook.show_all();
		}
	}

	public void on_console(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_console_notebook.is_visible()) {
			if (_console_view._entry.has_focus)
				_console_notebook.hide();
			else
				_console_view._entry.grab_focus_without_selecting();
		} else {
			_console_notebook.show_all();
			_console_view._entry.grab_focus_without_selecting();
		}
	}

	public void on_statusbar(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_statusbar.is_visible()) {
			_statusbar.hide();
		} else {
			_statusbar.show_all();
		}
	}

	public void on_inspector(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_inspector_stack.is_visible()) {
			_inspector_stack.hide();
		} else {
			_inspector_stack.show_all();
		}
	}

	public void on_restart_backend(GLib.SimpleAction action, GLib.Variant? param)
	{
		restart_backend.begin((obj, res) => {
				restart_backend.end(res);
			});
	}

	public void on_restart_editor_view(GLib.SimpleAction action, GLib.Variant? param)
	{
		_editor_viewport.restart_runtime.begin((obj, res) => {
				_editor_viewport.restart_runtime.end(res);
			});
	}

	public void on_build_data(GLib.SimpleAction action, GLib.Variant? param)
	{
		_data_compiler.compile.begin(_project.data_dir(), _project.platform(), (obj, res) => {
				_data_compiler.compile.end(res);
			});
	}

	public void on_reload_all(GLib.SimpleAction action, GLib.Variant? param)
	{
		compile_and_reload.begin();
	}

	public async bool refresh_all_clients()
	{
		bool success = true;

		foreach (var ri in _runtimes)
			if (ri.is_connected() && !yield ri.refresh(_data_compiler))
				success = false;

		if (success) {
			_project_browser.queue_draw();

			// Apply editor changes to reloaded units.
			var sb = new GLib.StringBuilder();
			Gee.ArrayList<Guid?> unit_ids = new Gee.ArrayList<Guid?>();
			_level.units(ref unit_ids);
			Unit.generate_change_commands(sb, unit_ids.to_array(), _database);
			foreach (var id in unit_ids)
				sb.append(LevelEditorApi.unit_freeze(id));

			_editor.send_script(sb.str);
			_editor_viewport.frame();
		}

		return success;
	}

	public void on_snap_to_grid(GLib.SimpleAction action, GLib.Variant? param)
	{
		_snap_to_grid = !action.get_state().get_boolean();
		send_state();
		_editor_viewport.frame();
		action.set_state(new GLib.Variant.boolean(_snap_to_grid));
	}

	public void on_debug_render_world(GLib.SimpleAction action, GLib.Variant? param)
	{
		_debug_render_world = !action.get_state().get_boolean();
		send_state();
		_editor_viewport.frame();
		action.set_state(new GLib.Variant.boolean(_debug_render_world));
	}

	public void on_debug_physics_world(GLib.SimpleAction action, GLib.Variant? param)
	{
		_debug_physics_world = !action.get_state().get_boolean();
		send_state();
		_editor_viewport.frame();
		action.set_state(new GLib.Variant.boolean(_debug_physics_world));
	}

	public void on_run_game(GLib.SimpleAction action, GLib.Variant? param)
	{
		// Trigger a 'focus_out_event' on the currently focused
		// widget within the active_window, if any. This will
		// force 'focus' to commit its pending changes to the
		// database so we do not miss any modifications before
		// launching the game.
		Gtk.Widget? focus = this.active_window.get_focus();
		_editor_viewport.grab_focus();
		if (focus != null)
			focus.grab_focus();

		var icon_displayed = _game_run_stop_image.icon_name;

		_game.stop.begin((obj, res) => {
				_game.stop.end(res);
				if (icon_displayed == "game-run") {
					// Always change icon state regardless of failures.
					_game_run_stop_image.set_from_icon_name("game-stop", Gtk.IconSize.MENU);

					start_game.begin(action.name == "test-level" ? StartGame.TEST : StartGame.NORMAL);
				}
			});
	}

	public void do_rename(Guid object_id, string new_name)
	{
		if (new_name != "" && _database.name(object_id) != new_name) {
			_database.set_name(object_id, new_name);
			_database.add_restore_point((int)ActionType.CHANGE_OBJECTS, new Guid?[] { object_id });
		}
	}

	public void on_rename(GLib.SimpleAction action, GLib.Variant? param)
	{
		Guid object_id = Guid.parse((string)param.get_child_value(0));
		string new_name = (string)param.get_child_value(1);

		if (new_name != "") {
			do_rename(object_id, new_name);
		} else {
			Gtk.Dialog dg = new Gtk.Dialog.with_buttons("New Name"
				, this.active_window
				, Gtk.DialogFlags.MODAL
				, "Cancel"
				, Gtk.ResponseType.CANCEL
				, "Ok"
				, Gtk.ResponseType.OK
				, null
				);

			InputString sb = new InputString();
			sb.activate.connect(() => { dg.response(Gtk.ResponseType.OK); });
			sb.value = _database.name(object_id);

			dg.get_content_area().add(sb);
			dg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.OK)
						do_rename(object_id, sb.text.strip());
					dg.destroy();
				});
			dg.show_all();
		}
	}

	public void on_manual(GLib.SimpleAction action, GLib.Variant? param)
	{
		try {
			AppInfo.launch_default_for_uri(CROWN_LATEST_DOCS_URL, null);
		} catch (Error e) {
			loge(e.message);
		}
	}

	public void on_report_issue(GLib.SimpleAction action, GLib.Variant? param)
	{
		try {
			AppInfo.launch_default_for_uri("https://github.com/crownengine/crown/issues", null);
		} catch (Error e) {
			loge(e.message);
		}
	}

	public void on_browse_logs(GLib.SimpleAction action, GLib.Variant? param)
	{
		open_directory(_logs_dir.get_path());
	}

	public void on_changelog(GLib.SimpleAction action, GLib.Variant? param)
	{
		try {
			AppInfo.launch_default_for_uri(CROWN_LATEST_CHANGELOG_URL, null);
		} catch (Error e) {
			loge(e.message);
		}
	}

	public void on_donate(GLib.SimpleAction action, GLib.Variant? param)
	{
		try {
			AppInfo.launch_default_for_uri(CROWN_FUND_URL, null);
		} catch (Error e) {
			loge(e.message);
		}
	}

	public void on_credits(GLib.SimpleAction action, GLib.Variant? param)
	{
		try {
			AppInfo.launch_default_for_uri(CROWN_CREDITS_URL, null);
		} catch (Error e) {
			loge(e.message);
		}
	}

	public void do_delete_file(string resource_path)
	{
		string path = _project.absolute_path(resource_path);

		try {
			GLib.File.new_for_path(path).delete();
		} catch (Error e) {
			loge(e.message);
		}
	}

	public void on_delete_file(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (param == null)
			return;

		string resource_path  = param.get_string();
		string? resource_type = ResourceId.type(resource_path);
		string? resource_name = ResourceId.name(resource_path);

		if (resource_type != null && resource_name != null) {
			if (resource_name == _level._name) {
				if (!_database.changed()) {
					do_delete_file(resource_path);
					GLib.Application.get_default().activate_action("new-level", null);
				} else {
					Gtk.Dialog dlg = new_level_changed_dialog(this.active_window);
					dlg.response.connect((response_id) => {
							if (response_id == Gtk.ResponseType.NO) {
								do_delete_file(resource_path);
								do_new_level();
							} else if (response_id == Gtk.ResponseType.YES) {
								save("new-level");
							}
							dlg.destroy();
						});
					dlg.show_all();
				}
			} else {
				do_delete_file(resource_path);
			}
		}
	}

	public void do_delete_directory(string dir_name)
	{
		if (dir_name == "")
			return;

		var path = _project.absolute_path(dir_name);
		try {
			_project.delete_tree(GLib.File.new_for_path(path));
		} catch (Error e) {
			loge(e.message);
		}
	}

	public void on_delete_directory(GLib.SimpleAction action, GLib.Variant? param)
	{
		string dir_name = param.get_string();

		Gtk.MessageDialog md = new Gtk.MessageDialog(this.active_window
			, Gtk.DialogFlags.MODAL
			, Gtk.MessageType.WARNING
			, Gtk.ButtonsType.NONE
			, "Delete Folder " + dir_name + "?"
			);

		Gtk.Widget btn;
		md.add_button("_Cancel", Gtk.ResponseType.CANCEL);
		btn = md.add_button("_Delete", Gtk.ResponseType.YES);
		btn.get_style_context().add_class("destructive-action");
		md.set_default_response(Gtk.ResponseType.CANCEL);

		md.response.connect((response_id) => {
			if (response_id == Gtk.ResponseType.YES)
				do_delete_directory(dir_name);
			md.destroy();
		});

		md.show_all();
	}

	public void do_create_directory(string parent_dir_name, string dir_name)
	{
		if (dir_name == "")
			return;

		var path = _project.absolute_path(GLib.Path.build_filename(parent_dir_name, dir_name));
		try {
			GLib.File.new_for_path(path).make_directory();
		} catch (Error e) {
			loge(e.message);
		}
	}

	public void on_create_directory(GLib.SimpleAction action, GLib.Variant? param)
	{
		string parent_dir_name = (string)param.get_child_value(0);
		string dir_name = (string)param.get_child_value(1);

		if (dir_name != "") {
			do_create_directory(parent_dir_name, dir_name);
		} else {
			Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Folder Name"
				, this.active_window
				, Gtk.DialogFlags.MODAL
				, "Cancel"
				, Gtk.ResponseType.CANCEL
				, "Ok"
				, Gtk.ResponseType.OK
				, null
				);

			InputString sb = new InputString();
			sb.activate.connect(() => { dg.response(Gtk.ResponseType.OK); });
			dg.get_content_area().add(sb);

			dg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.OK)
						do_create_directory(parent_dir_name, sb.text.strip());
					dg.destroy();
				});

			dg.show_all();
		}
	}

	public async bool compile_and_reload()
	{
		if (yield _data_compiler.compile(_project.data_dir(), _project.platform()))
			return yield refresh_all_clients();
		else
			return false;
	}

	public void do_create_script(string dir_name, string script_name, bool empty)
	{
		if (script_name == "")
			return;

		int ec = _project.create_script(dir_name, script_name, empty);
		if (ec < 0) {
			loge("Failed to create script %s".printf(script_name));
			return;
		}

		compile_and_reload.begin();
	}

	public void on_create_script(GLib.SimpleAction action, GLib.Variant? param)
	{
		string dir_name = (string)param.get_child_value(0);
		string script_name = (string)param.get_child_value(1);
		bool empty = (bool)param.get_child_value(2);

		if (script_name != "") {
			do_create_script(dir_name, script_name, empty);
		} else {
			Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Script Name"
				, this.active_window
				, Gtk.DialogFlags.MODAL
				, "Cancel"
				, Gtk.ResponseType.CANCEL
				, "Ok"
				, Gtk.ResponseType.OK
				, null
				);

			InputString sb = new InputString();
			sb.activate.connect(() => { dg.response(Gtk.ResponseType.OK); });
			dg.get_content_area().add(sb);

			dg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.OK)
						do_create_script(dir_name, sb.text.strip(), empty);
					dg.destroy();
				});

			dg.show_all();
		}
	}

	public void do_create_unit(string dir_name, string unit_name)
	{
		if (unit_name == "")
			return;

		int ec = _project.create_unit(dir_name, unit_name);
		if (ec < 0) {
			loge("Failed to create unit %s".printf(unit_name));
			return;
		}

		compile_and_reload.begin();
	}

	public void on_create_unit(GLib.SimpleAction action, GLib.Variant? param)
	{
		string dir_name = (string)param.get_child_value(0);
		string unit_name = (string)param.get_child_value(1);

		if (unit_name != "") {
			do_create_unit(dir_name, unit_name);
		} else {
			Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Unit Name"
				, this.active_window
				, Gtk.DialogFlags.MODAL
				, "Cancel"
				, Gtk.ResponseType.CANCEL
				, "Ok"
				, Gtk.ResponseType.OK
				, null
				);

			InputString sb = new InputString();
			sb.activate.connect(() => { dg.response(Gtk.ResponseType.OK); });
			dg.get_content_area().add(sb);

			dg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.OK)
						do_create_unit(dir_name, sb.text.strip());
					dg.destroy();
				});

			dg.show_all();
		}
	}

	public void do_create_state_machine(string dir_name, string state_machine_name, string skeleton_name)
	{
		if (state_machine_name == "")
			return;

		int ec = _project.create_state_machine(dir_name, state_machine_name, skeleton_name != "" ? skeleton_name : null);
		if (ec < 0) {
			loge("Failed to create state machine %s".printf(state_machine_name));
			return;
		}

		compile_and_reload.begin();
	}

	public void on_create_state_machine(GLib.SimpleAction action, GLib.Variant? param)
	{
		string dir_name = (string)param.get_child_value(0);
		string state_machine_name = (string)param.get_child_value(1);
		string skeleton_name = (string)param.get_child_value(2);

		if (state_machine_name != "") {
			do_create_state_machine(dir_name, state_machine_name, skeleton_name);
		} else {
			Gtk.Dialog dg = new Gtk.Dialog.with_buttons("State Machine Name"
				, this.active_window
				, Gtk.DialogFlags.MODAL
				, "Cancel"
				, Gtk.ResponseType.CANCEL
				, "Ok"
				, Gtk.ResponseType.OK
				, null
				);

			InputString sb = new InputString();
			sb.activate.connect(() => { dg.response(Gtk.ResponseType.OK); });
			dg.get_content_area().add(sb);

			dg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.OK)
						do_create_state_machine(dir_name, sb.text.strip(), skeleton_name);
					dg.destroy();
				});

			dg.show_all();
		}

	}

	public void do_create_material(string dir_name, string material_name)
	{
		if (material_name == "")
			return;

		int ec = _project.create_material(dir_name, material_name);
		if (ec < 0) {
			loge("Failed to create material %s".printf(material_name));
			return;
		}

		compile_and_reload.begin();
	}

	public void on_create_material(GLib.SimpleAction action, GLib.Variant? param)
	{
		string dir_name = (string)param.get_child_value(0);
		string material_name = (string)param.get_child_value(1);

		if (material_name != "") {
			do_create_material(dir_name, material_name);
		} else {
			Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Material Name"
				, this.active_window
				, Gtk.DialogFlags.MODAL
				, "Cancel"
				, Gtk.ResponseType.CANCEL
				, "Ok"
				, Gtk.ResponseType.OK
				, null
				);

			InputString sb = new InputString();
			sb.activate.connect(() => { dg.response(Gtk.ResponseType.OK); });
			dg.get_content_area().add(sb);

			dg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.OK)
						do_create_material(dir_name, sb.text.strip());
					dg.destroy();
				});

			dg.show_all();
		}
	}

	public void on_open_containing(GLib.SimpleAction action, GLib.Variant? param)
	{
		string path = param.get_string();
		GLib.File abs_path = GLib.File.new_for_path(_project.absolute_path(path));
		GLib.File? abs_parent = abs_path.get_parent();

		string abs_parent_path = abs_parent != null
			? abs_parent.get_path()
			: abs_path.get_path()
			;

		open_directory(abs_parent_path);
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

	public Gtk.Dialog new_package_dir_exists_dialog(string package_dir)
	{
		Gtk.MessageDialog md = new Gtk.MessageDialog(_deploy_dialog
			, Gtk.DialogFlags.MODAL
			, Gtk.MessageType.QUESTION
			, Gtk.ButtonsType.NONE
			, "A file named `%s` already exists.\nOverwrite?".printf(package_dir)
			);
		md.set_default_response(Gtk.ResponseType.NO);

		Gtk.Widget btn;
		md.add_button("_No", Gtk.ResponseType.NO);
		btn = md.add_button("_Yes", Gtk.ResponseType.YES);
		btn.get_style_context().add_class("destructive-action");
		return md;
	}

	public GLib.File deploy_package_dir(out string config_path, string output_path, string app_identifier, TargetPlatform platform, TargetArch arch, TargetConfig config)
	{
		string platform_name[] =
		{
			"android", // TargetArch.ANDROID
			"html5",   // TargetArch.HTML5
			"linux",   // TargetArch.LINUX
			"windows"  // TargetArch.WINDOWS
		};

		string arch_name[] =
		{
			"x86",   // TargetArch.X86
			"x64",   // TargetArch.X64
			"arm",   // TargetArch.ARM
			"arm64", // TargetArch.ARM64
			"wasm"   // TargetArch.WASM
		};

		string config_name[] =
		{
			"release",     // TargetConfig.RELEASE
			"development", // TargetConfig.DEVELOPMENT
#if CROWN_DEBUG
			"debug"        // TargetConfig.DEBUG
#endif
		};

		string platform_path = Path.build_path(Path.DIR_SEPARATOR_S, output_path, platform_name[platform], arch_name[arch]);
		config_path = Path.build_path(Path.DIR_SEPARATOR_S, platform_path, config_name[config]);
		return GLib.File.new_for_path(Path.build_path(Path.DIR_SEPARATOR_S, config_path, app_identifier));
	}

	public void do_create_package_android(GLib.File package_dir
		, string config_path
		, string output_path
		, int config
		, string app_title
		, string app_identifier
		, int app_version_code
		, string app_version_name
		, string keystore_path
		, string keystore_pass
		, string key_alias
		, string key_pass
		, int arch
		, string apk_name
		)
	{
		string config_name[] =
		{
			"release",
			"development",
#if CROWN_DEBUG
			"debug"
#endif
		};

		AndroidDeployer android = new AndroidDeployer();
		android.check_config();

		logi("Creating Android package (%s)...".printf(arch == TargetArch.ARM ? "ARMv7-A" : "ARMv8-A"));

		var activity_name = "MainActivity";
		var package_path = package_dir.get_path();

		// Architecture-agnostic paths.
		var manifests_path = Path.build_path(Path.DIR_SEPARATOR_S, package_path, "manifests");
		var java_sources_path = Path.build_path(Path.DIR_SEPARATOR_S, package_path, "java");
		var app_sources_path = Path.build_path(Path.DIR_SEPARATOR_S, java_sources_path, app_identifier.replace(".", "/"));
		var assets_path = Path.build_path(Path.DIR_SEPARATOR_S, package_path, "assets");
		var res_path = Path.build_path(Path.DIR_SEPARATOR_S, package_path, "res");
		var bin_path = Path.build_path(Path.DIR_SEPARATOR_S, package_path, "bin");
		var obj_path = Path.build_path(Path.DIR_SEPARATOR_S, package_path, "obj");
		var res_layout_path = Path.build_path(Path.DIR_SEPARATOR_S, res_path, "layout");
		var res_values_path = Path.build_path(Path.DIR_SEPARATOR_S, res_path, "values");
		var res_drawable_path = Path.build_path(Path.DIR_SEPARATOR_S, res_path, "drawable");
		var manifest_xml_path = Path.build_path(Path.DIR_SEPARATOR_S, manifests_path, "AndroidManifest.xml");
		var strings_xml_path = Path.build_path(Path.DIR_SEPARATOR_S, res_path, "values", "strings.xml");
		var activity_java_path = Path.build_path(Path.DIR_SEPARATOR_S, app_sources_path, "%s.java".printf(activity_name));
		var android_jar_path = Path.build_path(Path.DIR_SEPARATOR_S, android._sdk_path, "platforms", "android-" + android._sdk_api_level, "android.jar");
		var libcrown_src_name = "libcrown-" + config_name[config] + ".so";
		var libcpp_name = "libc++_shared.so";
		var signed_apk = Path.build_path(Path.DIR_SEPARATOR_S, bin_path, apk_name + ".signed.apk");
		var unaligned_apk = Path.build_path(Path.DIR_SEPARATOR_S, bin_path, apk_name + ".unaligned.apk");
		var final_apk = Path.build_path(Path.DIR_SEPARATOR_S, config_path, apk_name + ".apk");

#if CROWN_PLATFORM_LINUX
		string host_platform = "linux-x86_64";
#elif CROWN_PLATFORM_WINDOWS
		string host_platform = "windows-x86_64";
#endif

		// Architecture-specific paths.
		string dc_platform = null;
		string bin_folder  = null;
		string apk_arch    = null;
		string llvm_arch   = null;
		if (arch == TargetArch.ARM) {
			dc_platform = "android";
			bin_folder  = "android-arm";
			apk_arch    = "armeabi-v7a";
			llvm_arch   = "arm-linux-androideabi";
		} else if (arch == TargetArch.ARM64) {
			dc_platform = "android-arm64";
			bin_folder  = "android-arm64";
			apk_arch    = "arm64-v8a";
			llvm_arch   = "aarch64-linux-android";
		} else {
			loge("Invalid architecture");
			return;
		}

		var libcrown_src_path = Path.build_path(Path.DIR_SEPARATOR_S, "..", "..", bin_folder, "bin", libcrown_src_name);
		var libcpp_src_path   = Path.build_path(Path.DIR_SEPARATOR_S
			, android._ndk_root_path
			, "toolchains"
			, "llvm"
			, "prebuilt"
			, host_platform
			, "sysroot"
			, "usr"
			, "lib"
			, llvm_arch
			, libcpp_name
			);
		var lib_path_relative      = Path.build_path(Path.DIR_SEPARATOR_S, "lib", apk_arch);
		var lib_path               = Path.build_path(Path.DIR_SEPARATOR_S, package_path, lib_path_relative);
		var libcrown_path_relative = Path.build_path(Path.DIR_SEPARATOR_S, lib_path_relative, "libcrown.so");
		var libcpp_path_relative   = Path.build_path(Path.DIR_SEPARATOR_S, lib_path_relative, libcpp_name);
		var libcrown_dst_path      = Path.build_path(Path.DIR_SEPARATOR_S, lib_path, "libcrown.so");
		var libcpp_dst_path        = Path.build_path(Path.DIR_SEPARATOR_S, lib_path, "libc++_shared.so");

		// Create Android project skeleton.
		try {
			GLib.File.new_for_path(manifests_path).make_directory();
			GLib.File.new_for_path(app_sources_path).make_directory_with_parents();
			GLib.File.new_for_path(lib_path).make_directory_with_parents();
			GLib.File.new_for_path(assets_path).make_directory();
			GLib.File.new_for_path(bin_path).make_directory();
			GLib.File.new_for_path(obj_path).make_directory();
			GLib.File.new_for_path(res_layout_path).make_directory_with_parents();
			GLib.File.new_for_path(res_values_path).make_directory();
			GLib.File.new_for_path(res_drawable_path).make_directory();
		} catch (Error e) {
			loge(e.message);
		}

		// Compile game data.
		try {
			GLib.File.new_for_path(libcrown_src_path).copy(GLib.File.new_for_path(libcrown_dst_path), GLib.FileCopyFlags.NONE);
			GLib.File.new_for_path(libcpp_src_path).copy(GLib.File.new_for_path(libcpp_dst_path), GLib.FileCopyFlags.NONE);

			string[] args;

			// Populate Android assets folder with data.
			args = new string[]
			{
				ENGINE_EXE,
				"--source-dir",
				_project.source_dir(),
				"--map-source-dir",
				"core",
				_project.toolchain_dir(),
				"--bundle-dir",
				assets_path,
				"--compile",
				"--bundle",
				"--platform",
				dc_platform
			};

			uint32 pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			int exit_status = _subprocess_launcher.wait(pid);

			if (exit_status != 0) {
				loge("Failed to compile data. exit_status = %d".printf(exit_status));
				return;
			}
		} catch (Error e) {
			loge(e.message);
			return;
		}

		// Create Android manifest.
		string android_manifest = "";
		android_manifest += "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
		android_manifest += "\n<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"";
		android_manifest += "\n  package=\"%s\"".printf(app_identifier);
		android_manifest += "\n  android:versionCode=\"%u\"".printf(app_version_code);
		android_manifest += "\n  android:versionName=\"%s\">".printf(app_version_name);
		android_manifest += "\n";
		android_manifest += "\n  <uses-sdk android:minSdkVersion=\"23\" />";
		android_manifest += "\n";
		android_manifest += "\n  <!-- For ConsoleServer -->";
		android_manifest += "\n  <uses-permission android:name=\"android.permission.INTERNET\" />";
		android_manifest += "\n";
		android_manifest += "\n  <application";
		android_manifest += "\n    android:hasCode=\"true\"";
		android_manifest += "\n    android:label=\"%s\">".printf(app_title);
		android_manifest += "\n    <activity";
		android_manifest += "\n      android:name=\"%s.%s\"".printf(app_identifier, activity_name);
		android_manifest += "\n      android:configChanges=\"orientation|keyboardHidden\"";
		android_manifest += "\n      android:label=\"@string/activity_label\"";
		android_manifest += "\n      android:screenOrientation=\"landscape\"";
		android_manifest += "\n      android:theme=\"@android:style/Theme.NoTitleBar.Fullscreen\">";
		android_manifest += "\n";
		android_manifest += "\n      <!-- Tell NativeActivity the name of our .so -->";
		android_manifest += "\n      <meta-data";
		android_manifest += "\n        android:name=\"android.app.lib_name\"";
		android_manifest += "\n        android:value=\"crown\" />";
		android_manifest += "\n";
		android_manifest += "\n      <intent-filter>";
		android_manifest += "\n        <action android:name=\"android.intent.action.MAIN\" />";
		android_manifest += "\n        <action android:name=\"android.intent.action.VIEW\" />";
		android_manifest += "\n        <category android:name=\"android.intent.category.LAUNCHER\" />";
		android_manifest += "\n      </intent-filter>";
		android_manifest += "\n    </activity>";
		android_manifest += "\n  </application>";
		android_manifest += "\n</manifest>";
		android_manifest += "\n";

		GLib.FileStream? fs = FileStream.open(manifest_xml_path, "w");
		if (fs == null) {
			loge("Failed to open '%s'".printf(manifest_xml_path));
			return;
		}
		fs.write(android_manifest.data);

		// Create Android strings.xml.
		string android_strings = "";
		android_strings += "<resources>";
		android_strings += "\n<string name=\"activity_label\">%s</string>".printf(app_title);
		android_strings += "\n</resources>";
		android_strings += "\n";

		fs = FileStream.open(strings_xml_path, "w");
		if (fs == null) {
			loge("Failed to open '%s'".printf(strings_xml_path));
			return;
		}
		fs.write(android_strings.data);

		// Create Android activity.
		string android_activity = "";
		android_activity += "package %s;".printf(app_identifier);
		android_activity += "\n";
		android_activity += "\nimport android.app.NativeActivity;";
		android_activity += "\nimport android.os.Bundle;";
		android_activity += "\nimport android.view.View;";
		android_activity += "\n";
		android_activity += "\npublic class %s extends NativeActivity".printf(activity_name);
		android_activity += "\n{";
		android_activity += "\n    static";
		android_activity += "\n    {";
		android_activity += "\n        System.loadLibrary(\"crown\");";
		android_activity += "\n    }";
		android_activity += "\n";
		android_activity += "\n    @Override";
		android_activity += "\n    public void onCreate(Bundle savedInstanceState) {";
		android_activity += "\n        super.onCreate(savedInstanceState);";
		android_activity += "\n        // Init additional stuff here (Ads etc.)";
		android_activity += "\n    }";
		android_activity += "\n";
		android_activity += "\n    @Override";
		android_activity += "\n    public void onDestroy() {";
		android_activity += "\n        // Destroy additional stuff here (Ads etc)";
		android_activity += "\n        super.onDestroy();";
		android_activity += "\n    }";
		android_activity += "\n";
		android_activity += "\n    @Override";
		android_activity += "\n    public void onWindowFocusChanged(boolean hasFocus) {";
		android_activity += "\n        super.onWindowFocusChanged(hasFocus);";
		android_activity += "\n        if (hasFocus) {";
		android_activity += "\n            hideSystemUI();";
		android_activity += "\n        }";
		android_activity += "\n    }";
		android_activity += "\n";
		android_activity += "\n    public void hideSystemUI() {";
		android_activity += "\n        // Enables regular immersive mode.";
		android_activity += "\n        // For \"lean back\" mode, remove SYSTEM_UI_FLAG_IMMERSIVE.";
		android_activity += "\n        // Or for \"sticky immersive,\" replace it with SYSTEM_UI_FLAG_IMMERSIVE_STICKY";
		android_activity += "\n        View decorView = getWindow().getDecorView();";
		android_activity += "\n        decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_IMMERSIVE";
		android_activity += "\n            // Set the content to appear under the system bars so that the";
		android_activity += "\n            // content doesn't resize when the system bars hide and show.";
		android_activity += "\n            | View.SYSTEM_UI_FLAG_LAYOUT_STABLE";
		android_activity += "\n            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION";
		android_activity += "\n            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN";
		android_activity += "\n            // Hide the nav bar and status bar";
		android_activity += "\n            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION";
		android_activity += "\n            | View.SYSTEM_UI_FLAG_FULLSCREEN";
		android_activity += "\n            );";
		android_activity += "\n    }";
		android_activity += "\n";
		android_activity += "\n    public void showSystemUI() {";
		android_activity += "\n        // Shows the system bars by removing all the flags";
		android_activity += "\n        // except for the ones that make the content appear under the system bars.";
		android_activity += "\n        View decorView = getWindow().getDecorView();";
		android_activity += "\n        decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE";
		android_activity += "\n            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION";
		android_activity += "\n            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN";
		android_activity += "\n            );";
		android_activity += "\n    }";
		android_activity += "\n}";
		android_activity += "\n";

		fs = FileStream.open(activity_java_path, "w");
		if (fs == null) {
			loge("Failed to open '%s'".printf(activity_java_path));
			return;
		}
		fs.write(android_activity.data);

		// Compile java NativeActivity.
		Thread<int> javac = new Thread<int>("javac", () => {
				try {
					string[] args = new string[]
					{
						android._javac_path,
						"-verbose",
						"-source",
						"8", // https://docs.oracle.com/javase/1.5.0/docs/relnotes/version-5.0.html
						"-target",
						"8", // https://docs.oracle.com/javase/1.5.0/docs/relnotes/version-5.0.html
						"-d",
						obj_path,
						"-classpath",
						"java",
						"-bootclasspath",
						android_jar_path,
						activity_java_path
					};

					var sl = new GLib.SubprocessLauncher(subprocess_flags());
					sl.spawnv(args);
				} catch (Error e) {
					loge(e.message);
					return -1;
				}

				return 0;
			});
		javac.join();

		// Generate Android APK.
		new Thread<int>("post-javac", () => {
				// FIXME: just wait() for javac to terminate...
				var class_path = Path.build_path(Path.DIR_SEPARATOR_S
					, obj_path
					, app_identifier.replace(".", "/")
					, "%s.class".printf(activity_name)
					);
				var class_file = GLib.File.new_for_path(class_path);

				// Wait for javac to generate the .class file.
				GLib.Timer timer = new GLib.Timer();
				timer.start();

				while (!class_file.query_exists() && timer.elapsed() < 5) {
					GLib.Thread.usleep(500*1000);
				}

				if (!class_file.query_exists()) {
					loge("Failed to generate .class file");
					return -1;
				}

				// Generate remaining APK stuff.
				string[] args;
				uint32 pid;
				int exit_status;

				try {
					args = new string[]
					{
						android._d8_path,
						"--output",
						bin_path,
						class_path,
						"--no-desugaring"
					};

					pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
					exit_status = _subprocess_launcher.wait(pid);
					if (exit_status != 0) {
						loge("Failed to generate dex file. exit_status %d".printf(exit_status));
						return -1;
					}

					args = new string[]
					{
						android._aapt_path,
						"package",
						"-f",
						"-m",
						"-F",
						unaligned_apk,
						"-M",
						manifest_xml_path,
						"-S",
						res_path,
						"-A",
						assets_path,
						"-I",
						android_jar_path
					};

					pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
					exit_status = _subprocess_launcher.wait(pid);
					if (exit_status != 0) {
						loge("Failed to do something with the APK. exit_status %d".printf(exit_status));
						return -1;
					}

					args = new string[]
					{
						android._aapt_path,
						"add",
						unaligned_apk,
						"classes.dex"
					};

					pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, bin_path);
					exit_status = _subprocess_launcher.wait(pid);
					if (exit_status != 0) {
						loge("Failed to add classes.dex to APK. exit_status %d".printf(exit_status));
						return -1;
					}

					args = new string[]
					{
						android._aapt_path,
						"add",
						unaligned_apk,
						libcrown_path_relative.replace("\\", "/"),
						libcpp_path_relative.replace("\\", "/")
					};

					pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, package_path);
					exit_status = _subprocess_launcher.wait(pid);
					if (exit_status != 0) {
						loge("Failed to add libs to APK. exit_status %d".printf(exit_status));
						return -1;
					}

					args = new string[]
					{
						android._jarsigner_path,
						"-keystore",
						keystore_path,
						"-storepass",
						keystore_pass,
						"-keypass",
						key_pass,
						"-signedjar",
						signed_apk,
						unaligned_apk,
						key_alias
					};

					pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
					exit_status = _subprocess_launcher.wait(pid);
					if (exit_status != 0) {
						loge("Failed sign APK. exit_status %d".printf(exit_status));
						return -1;
					}

					args = new string[]
					{
						android._zipalign_path,
						"-f",
						"4",
						signed_apk,
						final_apk
					};

					pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
					exit_status = _subprocess_launcher.wait(pid);
					if (exit_status != 0) {
						loge("Failed align APK. exit_status %d".printf(exit_status));
						return -1;
					}
				} catch (Error e) {
					loge(e.message);
					loge("Failed to deploy '%s'".printf(app_title));
					return -1;
				}

				logi("Done: #FILE(%s)".printf(config_path));
				return 0;
			});
	}

	public void on_create_package_android(GLib.SimpleAction action, GLib.Variant? param)
	{
		var output_path = (string)param.get_child_value(0);
		var config = (int)param.get_child_value(1);
		var app_title = (string)param.get_child_value(2);
		var app_identifier = (string)param.get_child_value(3);
		var app_version_code = (int)param.get_child_value(4);
		var app_version_name = (string)param.get_child_value(5);
		var keystore_path = (string)param.get_child_value(6);
		var keystore_pass = (string)param.get_child_value(7);
		var key_alias = (string)param.get_child_value(8);
		var key_pass = (string)param.get_child_value(9);
		var arch = (int)param.get_child_value(10);

		var apk_name = app_identifier + "-" + app_version_name;

		string config_path;
		GLib.File package_dir = deploy_package_dir(out config_path
			, output_path
			, apk_name
			, TargetPlatform.ANDROID
			, arch
			, (TargetConfig)config
			);

		if (package_dir.query_exists()) {
			Gtk.Dialog dlg = new_package_dir_exists_dialog(package_dir.get_basename());
			dlg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.YES) {
						try {
							delete_tree(package_dir);
							package_dir.make_directory_with_parents();
							do_create_package_android(package_dir
								, config_path
								, output_path
								, config
								, app_title
								, app_identifier
								, app_version_code
								, app_version_name
								, keystore_path
								, keystore_pass
								, key_alias
								, key_pass
								, arch
								, apk_name
								);
						} catch (Error e) {
							loge(e.message);
						}
					}
					dlg.destroy();
				});
			dlg.show_all();
		} else {
			try {
				package_dir.make_directory_with_parents();
				do_create_package_android(package_dir
					, config_path
					, output_path
					, config
					, app_title
					, app_identifier
					, app_version_code
					, app_version_name
					, keystore_path
					, keystore_pass
					, key_alias
					, key_pass
					, arch
					, apk_name
					);
			} catch (Error e) {
				loge(e.message);
			}
		}
	}

	public void do_create_package_html5(GLib.File package_dir
		, string output_path
		, int config
		, string app_title
		, string exe_name
		)
	{
		string config_name[] =
		{
			"release",
			"development",
#if CROWN_DEBUG
			"debug"
#endif
		};

		HTML5Deployer html5 = new HTML5Deployer();
		html5.check_config();

		logi("Creating HTML5 package...");

		string package_path = package_dir.get_path();

		// Create data bundle.
		try {
			string[] args;
			string tmp_bundle_dir = GLib.DirUtils.make_tmp("XXXXXX");

			args = new string[]
			{
				ENGINE_EXE,
				"--source-dir",
				_project.source_dir(),
				"--map-source-dir",
				"core",
				_project.toolchain_dir(),
				"--bundle-dir",
				tmp_bundle_dir,
				"--compile",
				"--bundle",
				"--platform",
				"html5"
			};

			var pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			var exit_status = _subprocess_launcher.wait(pid);

			if (exit_status != 0) {
				loge("Failed to compile data. exit_status = %d".printf(exit_status));
				return;
			}

			// Copy runtime executables to package folder.
			var runtime_name_src = "crown-%s".printf(config_name[config]);
			var runtime_path_src = Path.build_path(Path.DIR_SEPARATOR_S, "..", "..", "wasm", "bin", runtime_name_src);
			var runtime_name_dst = Path.build_filename(package_path, runtime_name_src);

			var src = File.new_for_path(runtime_path_src + ".js");
			var dst = File.new_for_path(runtime_name_dst + ".js");
			src.copy(dst, FileCopyFlags.OVERWRITE);

			try {
				src = File.new_for_path(runtime_path_src + ".worker.js");
				dst = File.new_for_path(runtime_name_dst + ".worker.js");
				src.copy(dst, FileCopyFlags.OVERWRITE);
			} catch (GLib.Error e) {
				// NOOP: newer emscripten versions embed .worker.js into main .js.
			}

			src = File.new_for_path(runtime_path_src + ".wasm");
			dst = File.new_for_path(runtime_name_dst + ".wasm");
			src.copy(dst, FileCopyFlags.OVERWRITE);

			// Package bundle data with emscripten's file_packager.
			args = new string[]
			{
				Path.build_path(Path.DIR_SEPARATOR_S, html5._emscripten_sdk_path, "tools", "file_packager"),
				Path.build_path(Path.DIR_SEPARATOR_S, package_path, "data.bin"),
				"--preload",
				"./data",
				"--js-output=" + Path.build_path(Path.DIR_SEPARATOR_S, package_path, "data.js")
			};

			pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, tmp_bundle_dir);
			exit_status = _subprocess_launcher.wait(pid);
			delete_tree(GLib.File.new_for_path(tmp_bundle_dir));

			if (exit_status != 0) {
				loge("Failed to package data.js. exit_status %d".printf(exit_status));
				return;
			}

			// Generate index.html.
			var index_html_path = Path.build_path(Path.DIR_SEPARATOR_S, package_path, "index.html");

			string index_html = "";
			index_html += "\n<!doctype html>";
			index_html += "\n<html lang=\"en-us\">";
			index_html += "\n<head>";
			index_html += "\n<meta charset=\"utf-8\">";
			index_html += "\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
			index_html += "\n<style>";
			index_html += "\n  body {";
			index_html += "\n  	 margin: 0px;";
			index_html += "\n  	 background-color: black;";
			index_html += "\n  }";
			index_html += "\n  .app {";
			index_html += "\n    display: block;";
			index_html += "\n    border: 0px;";
			index_html += "\n    margin: 0px;";
			index_html += "\n    position: absolute;";
			index_html += "\n    top: 0;";
			index_html += "\n    left: 0;";
			index_html += "\n    width: 100%;";
			index_html += "\n    height: 100%;";
			index_html += "\n    background-color: black;";
			index_html += "\n  }";
			index_html += "\n</style>";
			index_html += "\n</head>";
			index_html += "\n<body>";
			index_html += "\n<canvas class=\"app\" id=\"canvas\" oncontextmenu=\"event.preventDefault()\" tabindex=-1></canvas>";
			index_html += "\n<script type='text/javascript'>";
			index_html += "\n  var Module = {";
			index_html += "\n    preRun: [],";
			index_html += "\n    postRun: [],";
			index_html += "\n    canvas: (() => {";
			index_html += "\n      var canvas = document.getElementById('canvas');";
			index_html += "\n      canvas.addEventListener(\"webglcontextlost\", (e) => {";
			index_html += "\n        alert('WebGL context lost. You will need to reload the page.');";
			index_html += "\n        e.preventDefault();";
			index_html += "\n      }, false);";
			index_html += "\n      return canvas;";
			index_html += "\n    })(),";
			index_html += "\n    setStatus: (text) => { },";
			index_html += "\n    monitorRunDependencies: (left) => { }";
			index_html += "\n  };";
			index_html += "\n  window.onerror = (event) => {";
			index_html += "\n    console.error('onerror: ' + event.message);";
			index_html += "\n  };";
			index_html += "\n</script>";
			index_html += "\n<script async type=\"text/javascript\" src=\"data.js\"></script>";
			index_html += "\n<script async type=\"text/javascript\" src=\"%s.js\"></script>".printf(runtime_name_src);
			index_html += "\n</body>";
			index_html += "\n</html>";
			index_html += "\n";

			GLib.FileStream? fs = FileStream.open(index_html_path, "w");
			if (fs == null) {
				loge("Failed to open '%s'".printf(index_html_path));
				return;
			}
			fs.write(index_html.data);
		} catch (Error e) {
			loge(e.message);
			loge("Failed to deploy '%s'".printf(app_title));
			return;
		}

		logi("Done: #FILE(%s)".printf(package_path));
	}

	public void on_create_package_html5(GLib.SimpleAction action, GLib.Variant? param)
	{
		var output_path = (string)param.get_child_value(0);
		var config = (int)param.get_child_value(1);
		var app_title = (string)param.get_child_value(2);

		var exe_name = app_title.replace(" ", "_").down();

		string config_path;
		GLib.File package_dir = deploy_package_dir(out config_path
			, output_path
			, exe_name
			, TargetPlatform.HTML5
			, TargetArch.WASM
			, (TargetConfig)config
			);

		if (package_dir.query_exists()) {
			Gtk.Dialog dlg = new_package_dir_exists_dialog(package_dir.get_basename());
			dlg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.YES) {
						try {
							delete_tree(package_dir);
							package_dir.make_directory_with_parents();
							do_create_package_html5(package_dir, output_path, config, app_title, exe_name);
						} catch (Error e) {
							loge(e.message);
						}
					}
					dlg.destroy();
				});
			dlg.show_all();
		} else {
			try {
				package_dir.make_directory_with_parents();
				do_create_package_html5(package_dir, output_path, config, app_title, exe_name);
			} catch (Error e) {
				loge(e.message);
			}
		}
	}

	public void do_create_package_linux(GLib.File package_dir
		, string output_path
		, int config
		, string app_title
		, string exe_name
		)
	{
		string config_name[] =
		{
			"release",
			"development",
#if CROWN_DEBUG
			"debug"
#endif
		};

		logi("Creating Linux package...");

		string package_path = package_dir.get_path();

		// Create data bundle.
		try {
			string args[] =
			{
				ENGINE_EXE,
				"--source-dir",
				_project.source_dir(),
				"--map-source-dir",
				"core",
				_project.toolchain_dir(),
				"--bundle-dir",
				package_path,
				"--compile",
				"--bundle",
				"--platform",
				"linux"
			};

			uint32 compiler = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			int exit_status = _subprocess_launcher.wait(compiler);

			if (exit_status != 0) {
				loge("Failed to compile data. exit_status = %d".printf(exit_status));
				return;
			}

			GLib.File engine_exe_src = File.new_for_path(EXE_PREFIX + "crown-%s".printf(config_name[config]) + EXE_SUFFIX);
			GLib.File engine_exe_dst = File.new_for_path(Path.build_filename(package_path, exe_name + EXE_SUFFIX));
			engine_exe_src.copy(engine_exe_dst, FileCopyFlags.OVERWRITE);
		} catch (Error e) {
			loge(e.message);
			loge("Failed to deploy '%s'".printf(app_title));
			return;
		}

		logi("Done: #FILE(%s)".printf(package_path));
	}

	public void on_create_package_linux(GLib.SimpleAction action, GLib.Variant? param)
	{
		var output_path = (string)param.get_child_value(0);
		var config = (int)param.get_child_value(1);
		var app_title = (string)param.get_child_value(2);

		var exe_name = app_title.replace(" ", "_").down();

		string config_path;
		GLib.File package_dir = deploy_package_dir(out config_path
			, output_path
			, exe_name
			, TargetPlatform.LINUX
			, TargetArch.X64
			, (TargetConfig)config
			);

		if (package_dir.query_exists()) {
			Gtk.Dialog dlg = new_package_dir_exists_dialog(package_dir.get_basename());
			dlg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.YES) {
						try {
							delete_tree(package_dir);
							package_dir.make_directory_with_parents();
							do_create_package_linux(package_dir, output_path, config, app_title, exe_name);
						} catch (Error e) {
							loge(e.message);
						}
					}
					dlg.destroy();
				});
			dlg.show_all();
		} else {
			try {
				package_dir.make_directory_with_parents();
				do_create_package_linux(package_dir, output_path, config, app_title, exe_name);
			} catch (Error e) {
				loge(e.message);
			}
		}
	}

	void do_create_package_windows(GLib.File package_dir
		, string output_path
		, int config
		, string app_title
		, string exe_name
		)
	{
		string config_name[] =
		{
			"release",
			"development",
#if CROWN_DEBUG
			"debug"
#endif
		};

		logi("Creating Windows package");

		string package_path = package_dir.get_path();

		// Create data bundle.
		try {
			string args[] =
			{
				ENGINE_EXE,
				"--source-dir",
				_project.source_dir(),
				"--map-source-dir",
				"core",
				_project.toolchain_dir(),
				"--bundle-dir",
				package_path,
				"--compile",
				"--bundle",
				"--platform",
				"windows"
			};

			uint32 compiler = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			int exit_status = _subprocess_launcher.wait(compiler);

			if (exit_status != 0) {
				loge("Failed to compile data. exit_status = %d".printf(exit_status));
				return;
			}

			GLib.File engine_exe_src = File.new_for_path(EXE_PREFIX + "crown-%s".printf(config_name[config]) + EXE_SUFFIX);
			GLib.File engine_exe_dst = File.new_for_path(Path.build_filename(package_path, exe_name + EXE_SUFFIX));
			engine_exe_src.copy(engine_exe_dst, FileCopyFlags.OVERWRITE);

			string openal_name = "openal-release.dll";
			GLib.File openal_dll_src = File.new_for_path(openal_name);
			GLib.File openal_dll_dst = File.new_for_path(Path.build_filename(package_path, openal_name));
			openal_dll_src.copy(openal_dll_dst, FileCopyFlags.OVERWRITE);

			try {
				string lua_name = "lua51.dll";
				GLib.File lua_dll_src = File.new_for_path(lua_name);
				GLib.File lua_dll_dst = File.new_for_path(Path.build_filename(package_path, lua_name));
				lua_dll_src.copy(lua_dll_dst, FileCopyFlags.OVERWRITE);
			} catch (Error e) {
				// Ignore: runtime's MinGW builds statically link luajit.
			}
		} catch (Error e) {
			loge("%s".printf(e.message));
			loge("Failed to deploy '%s'".printf(app_title));
			return;
		}

		logi("Done: #FILE(%s)".printf(package_path));
	}

	public void on_create_package_windows(GLib.SimpleAction action, GLib.Variant? param)
	{
		var output_path = (string)param.get_child_value(0);
		var config = (int)param.get_child_value(1);
		var app_title = (string)param.get_child_value(2);

		var exe_name = app_title.replace(" ", "_").down();

		string config_path;
		GLib.File package_dir = deploy_package_dir(out config_path
			, output_path
			, exe_name
			, TargetPlatform.WINDOWS
			, TargetArch.X64
			, (TargetConfig)config
			);

		if (package_dir.query_exists()) {
			Gtk.Dialog dlg = new_package_dir_exists_dialog(package_dir.get_basename());
			dlg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.YES) {
						try {
							delete_tree(package_dir);
							package_dir.make_directory_with_parents();
							do_create_package_windows(package_dir, output_path, config, app_title, exe_name);
						} catch (Error e) {
							loge(e.message);
						}
					}
					dlg.destroy();
				});
			dlg.show_all();
		} else {
			try {
				package_dir.make_directory_with_parents();
				do_create_package_windows(package_dir, output_path, config, app_title, exe_name);
			} catch (Error e) {
				loge(e.message);
			}
		}
	}

	public void on_unit_save_as_prefab(GLib.SimpleAction action, GLib.Variant? param)
	{
		string guid = (string)param.get_child_value(0);
		string name = (string)param.get_child_value(1);

		SaveResourceDialog srd = new SaveResourceDialog("Save Prefab As..."
			, this.active_window
			, OBJECT_TYPE_UNIT
			, name
			, _project
			);
		srd.safer_response.connect((response_id, path) => {
				if (response_id == Gtk.ResponseType.ACCEPT && path != null) {
					string prefab_filename = _project.resource_filename(path);
					string prefab_path     = ResourceId.normalize(prefab_filename);
					string prefab_name     = ResourceId.name(prefab_path);

					Guid unit_id = Guid.parse(guid);
					if (Unit(_database, unit_id).prefab() == prefab_name) {
						Gtk.MessageDialog md = new Gtk.MessageDialog(this.active_window
							, Gtk.DialogFlags.MODAL
							, Gtk.MessageType.ERROR
							, Gtk.ButtonsType.NONE
							, "Cannot save prefab '%s' over itself".printf(prefab_name)
							);
						md.add_button("_Ok", Gtk.ResponseType.OK);
						md.set_default_response(Gtk.ResponseType.OK);
						md.response.connect(() => { md.destroy(); });
						md.show_all();
					} else if (_database.has_object(unit_id)) {
						Guid prefab_id = Guid.new_guid();
						Database new_database = new Database(_project);
						_database.duplicate(unit_id, prefab_id, new_database);
						new_database.save(path, prefab_id);

						compile_and_reload.begin((obj, res) => {
								if (compile_and_reload.end(res)) {
									_level.replace_unit(unit_id, prefab_name);
								}
							});
					}
				}

				srd.destroy();
			});
		srd.show_all();
		srd.present();
	}

	public void on_open_unit(GLib.SimpleAction action, GLib.Variant? param)
	{
		string unit_name = param.get_string();

		if (_unit_editor_dialog == null) {
			_unit_editor_dialog = new UnitEditor(this
				, _project
				, "core/editors/level_editor"
				, "127.0.0.1"
				, 10444
				, (uint)_preferences_dialog._undo_redo_max_size.value * 1024 * 1024
				);
			_unit_editor_dialog.set_transient_for(_level_editor_window);
			_unit_editor_dialog.saved.connect(on_unit_editor_saved);
			_runtimes.add(_unit_editor_dialog._runtime);
		}

		_unit_editor_dialog.show_all();
		_unit_editor_dialog.present();
		_unit_editor_dialog.set_unit(unit_name);
	}

	public void on_unit_editor_saved()
	{
		compile_and_reload.begin((obj, res) => {
				if (compile_and_reload.end(res)) {
					ui_read_selection(_database_editor._selection.to_array());
				}
			});
	}

	public void set_autosave_timer(uint minutes)
	{
		if (_save_timer_id > 0)
			GLib.Source.remove(_save_timer_id);

		_save_timer_id = GLib.Timeout.add_seconds(minutes*60, save_timeout);
	}

	public void menu_set_enabled(bool enabled, GLib.ActionEntry[] entries, string[]? whitelist = null)
	{
		for (int ii = 0; ii < entries.length; ++ii) {
			string action_name = entries[ii].name;
			int jj = 0;
			if (whitelist != null) {
				for (; jj < whitelist.length; ++jj) {
					if (action_name == whitelist[jj])
						break;
				}
			}
			if (whitelist == null || whitelist != null && jj == whitelist.length) {
				GLib.SimpleAction sa = this.lookup_action(action_name) as GLib.SimpleAction;
				if (sa != null)
					sa.set_enabled(enabled);
			}
		}
	}

	public void set_conflicting_accels(bool on)
	{
		if (on) {
			this.set_accels_for_action("app.tool(0)", _tool_place_accels);
			this.set_accels_for_action("app.tool(1)", _tool_move_accels);
			this.set_accels_for_action("app.tool(2)", _tool_rotate_accels);
			this.set_accels_for_action("app.tool(3)", _tool_scale_accels);
			this.set_accels_for_action("app.delete", _delete_accels);
			this.set_accels_for_action("viewport.camera-view(0)", _camera_view_perspective_accels);
			this.set_accels_for_action("viewport.camera-view(1)", _camera_view_front_accels);
			this.set_accels_for_action("viewport.camera-view(2)", _camera_view_back_accels);
			this.set_accels_for_action("viewport.camera-view(3)", _camera_view_right_accels);
			this.set_accels_for_action("viewport.camera-view(4)", _camera_view_left_accels);
			this.set_accels_for_action("viewport.camera-view(5)", _camera_view_top_accels);
			this.set_accels_for_action("viewport.camera-view(6)", _camera_view_bottom_accels);
			this.set_accels_for_action("viewport.camera-frame-selected", _camera_frame_selected_accels);
			this.set_accels_for_action("app.camera-frame-all", _camera_frame_all_accels);
		} else {
			this.set_accels_for_action("app.tool(0)", {});
			this.set_accels_for_action("app.tool(1)", {});
			this.set_accels_for_action("app.tool(2)", {});
			this.set_accels_for_action("app.tool(3)", {});
			this.set_accels_for_action("app.delete", {});
			this.set_accels_for_action("viewport.camera-view(0)", {});
			this.set_accels_for_action("viewport.camera-view(1)", {});
			this.set_accels_for_action("viewport.camera-view(2)", {});
			this.set_accels_for_action("viewport.camera-view(3)", {});
			this.set_accels_for_action("viewport.camera-view(4)", {});
			this.set_accels_for_action("viewport.camera-view(5)", {});
			this.set_accels_for_action("viewport.camera-view(6)", {});
			this.set_accels_for_action("viewport.camera-frame-selected", {});
			this.set_accels_for_action("app.camera-frame-all", {});
		}
	}

	public void entry_any_focus_in(Gtk.Widget widget)
	{
		set_conflicting_accels(false);
	}

	public void entry_any_focus_out(Gtk.Widget widget)
	{
		set_conflicting_accels(true);
	}

	public void show_panel(string name, Gtk.StackTransitionType stt = Gtk.StackTransitionType.NONE)
	{
		_main_stack.set_visible_child_full(name, stt);

		if (name == PANEL_EDITOR) {
			// FIXME: save/restore last known window state
			int win_w;
			int win_h;
			this.active_window.get_size(out win_w, out win_h);
			_editor_pane.set_position(320);
			_content_pane.set_position(win_h - 250);
			_inspector_pane.set_position(win_h - 600);
			_main_pane.set_position(win_w - 375);

			menu_set_enabled(true, action_entries_file);
			menu_set_enabled(true, action_entries_edit);
			menu_set_enabled(true, action_entries_create);
			menu_set_enabled(true, action_entries_camera);
			menu_set_enabled(true, action_entries_view);
			menu_set_enabled(true, action_entries_debug);
			menu_set_enabled(true, action_entries_help);
		} else if (name == PANEL_PROJECTS_LIST
			|| name == PANEL_NEW_PROJECT
			) {
			menu_set_enabled(false, action_entries_file, {"new-project", "add-project", "open-project", "open-project-null", "open-projects-list", "remove-project", "quit"});
			menu_set_enabled(false, action_entries_edit);
			menu_set_enabled(false, action_entries_create);
			menu_set_enabled(false, action_entries_camera);
			menu_set_enabled(false, action_entries_view);
			menu_set_enabled(false, action_entries_debug);
			menu_set_enabled(true, action_entries_help);
		}
	}

	public void on_set_placeable(GLib.SimpleAction action, GLib.Variant? param)
	{
		_placeable_type = (string)param.get_child_value(0);
		_placeable_name = (string)param.get_child_value(1);

		_editor.send_script(LevelEditorApi.set_placeable(_placeable_type, _placeable_name));
		activate_action("tool", new GLib.Variant.int32(ToolType.PLACE));
	}

	public void on_project_reset()
	{
		if (!_project.is_loaded())
			return;

		// Save per-project data.
		try {
			string path = GLib.Path.build_filename(_project.user_dir(), "project_store.sjson");
			SJSON.save(_project_store.encode(), path);
		} catch (JsonWriteError e) {
			loge(e.message);
		}

		// Destroy dialogs.
		_texture_settings_dialog = null;
		_deploy_dialog = null;
	}

	public void on_project_loaded()
	{
		// Load per-project data.
		try {
			string path = GLib.Path.build_filename(_project.user_dir(), "project_store.sjson");
			_project_store.decode(SJSON.load_from_path(path));
		}
		catch (JsonSyntaxError e) {
			// No-op.
		}
	}

	public SelectResourceDialog new_select_resource_dialog(string resource_type)
	{
		return new SelectResourceDialog(resource_type, _project_store, this.active_window);
	}

	public void on_level_treeview_selection_changed(Gee.ArrayList<Guid?> selection)
	{
		_database_editor.selection_read(selection.to_array());

		ui_read_selection.disconnect(_level_treeview.read_selection);
		ui_read_selection(_database_editor._selection.to_array());
		ui_read_selection.connect(_level_treeview.read_selection);

		_database_editor.send_selection(_editor);
		_editor_viewport.frame();
	}

	public void on_selection_changed()
	{
		ui_read_selection(_database_editor._selection.to_array());

		_database_editor.send_selection(_editor);
		_editor_viewport.frame();
	}
}

// Global paths
public static GLib.File _toolchain_dir;
public static GLib.File _templates_dir;
public static GLib.File _data_dir;
public static GLib.File _config_dir;
public static GLib.File _cache_dir;
public static GLib.File _logs_dir;
public static GLib.File _thumbnails_dir;
public static GLib.File _thumbnails_normal_dir;
public static GLib.File _documents_dir;
public static GLib.File _log_file;
public static GLib.File _settings_file;
public static GLib.File _user_file;
public static GLib.File _console_history_file;
public static GLib.File _window_state_file;

public static GLib.FileStream _log_stream;
public static ConsoleView _console_view;
public static bool _console_view_valid = false;
public static string _log_prefix;

public static SubprocessLauncher _subprocess_launcher;

public static void log(string system, string severity, string message)
{
	GLib.DateTime now = new GLib.DateTime.now_local();
	int now_us = now.get_microsecond();
	string now_str = now.format("%H:%M:%S");

	string plain_text_line = "%s.%06d  %.4s %s: %s\n".printf(now_str
		, now_us
		, severity.ascii_up()
		, system
		, message
		);

	if (_log_stream != null) {
		_log_stream.puts(plain_text_line);
		_log_stream.flush();
	}

	if (_console_view_valid) {
		string line = "%s: %s\n".printf(system, message);
		string time = "%s.%06d  ".printf(now_str, now_us);
		_console_view.log(time, severity, line);
	}
}

public static void logi(string message)
{
	log(_log_prefix, "info", message);
}

public static void logw(string message)
{
	log(_log_prefix, "warning", message);
}

public static void loge(string message)
{
	log(_log_prefix, "error", message);
}

public void open_directory(string directory)
{
#if CROWN_PLATFORM_LINUX
	try {
		GLib.AppInfo.launch_default_for_uri("file://" + directory, null);
	} catch (Error e) {
		loge(e.message);
	}
#else
	GLib.SubprocessLauncher sl = new GLib.SubprocessLauncher(subprocess_flags());
	try {
		sl.spawnv({ "explorer.exe", directory, null });
	} catch (Error e) {
		loge(e.message);
	}
#endif
}

public void open_text_editor(string path)
{
#if CROWN_PLATFORM_WINDOWS
	GLib.SubprocessLauncher sl = new GLib.SubprocessLauncher(subprocess_flags());
	try {
		sl.spawnv({ "notepad.exe", path, null });
	} catch (Error e) {
		loge(e.message);
	}
#endif
}

public static GLib.SubprocessFlags subprocess_flags()
{
	GLib.SubprocessFlags flags = SubprocessFlags.NONE;
#if !CROWN_DEBUG
	flags |= SubprocessFlags.STDOUT_SILENCE | SubprocessFlags.STDERR_SILENCE;
#endif
	return flags;
}

public static bool is_directory_empty(string path)
{
	GLib.File file = GLib.File.new_for_path(path);
	try {
		FileEnumerator enumerator = file.enumerate_children("standard::*"
			, FileQueryInfoFlags.NOFOLLOW_SYMLINKS
			);
		return enumerator.next_file() == null;
	} catch (GLib.Error e) {
		loge(e.message);
	}

	return false;
}

public void device_frame_delayed(uint delay_ms, RuntimeInstance runtime)
{
	// FIXME: find a way to time exactly when it is effective to queue a redraw.
	// See: https://blogs.gnome.org/jnelson/2010/10/13/those-realize-map-widget-signals/
	GLib.Timeout.add_full(GLib.Priority.DEFAULT, delay_ms, () => {
			runtime.send(DeviceApi.frame());
			return GLib.Source.REMOVE;
		});
}

public static int main(string[] args)
{
	// If args does not contain --child, spawn the launcher.
	int ii;
	for (ii = 0; ii < args.length; ++ii) {
		if (args[ii] == "--launcher") {
			break;
		}
	}

	if (ii == args.length) {
		_log_prefix = "editor";
	} else {
		_log_prefix = "launcher";

		// Remove --child from args for backward compatibility.
		if (args.length > 1)
			args = args[0 : args.length - 1];
	}

	// Redirect GLib logs to internal log*().
	GLib.set_print_handler((msg) => { logi(msg); });
	GLib.set_printerr_handler((msg) => { loge(msg); });

	GLib.Log.set_writer_func((log_level, fields) => {
			foreach (var field in fields) {
				if (field.key == "MESSAGE") {
					switch (log_level) {
					case LEVEL_DEBUG:
#if CROWN_DEBUG
						logi((string)field.value);
#endif
						break;

					case LEVEL_INFO:
					case LEVEL_MESSAGE:
						logi((string)field.value);
						break;

					case LEVEL_CRITICAL:
					case LEVEL_WARNING:
						logw((string)field.value);
						break;

					case LEVEL_ERROR:
						loge((string)field.value);
						break;

					default:
						logw((string)field.value);
						break;
					}

					return GLib.LogWriterOutput.HANDLED;
				}
			}

			return GLib.LogWriterOutput.UNHANDLED;
		});

	// Global paths
	_data_dir = GLib.File.new_for_path(GLib.Path.build_filename(GLib.Environment.get_user_data_dir(), "crown"));
	try {
		_data_dir.make_directory();
	} catch (Error e) {
		/* Nobody cares */
	}
	_config_dir = GLib.File.new_for_path(GLib.Path.build_filename(GLib.Environment.get_user_config_dir(), "crown"));
	try {
		_config_dir.make_directory();
	} catch (Error e) {
		/* Nobody cares */
	}
	_cache_dir = GLib.File.new_for_path(GLib.Path.build_filename(GLib.Environment.get_user_cache_dir(), "crown"));
	try {
		_cache_dir.make_directory();
	} catch (Error e) {
		/* Nobody cares */
	}
	_logs_dir = GLib.File.new_for_path(GLib.Path.build_filename(_data_dir.get_path(), "logs"));
	try {
		_logs_dir.make_directory();
	} catch (Error e) {
		/* Nobody cares */
	}
	_documents_dir = GLib.File.new_for_path(GLib.Environment.get_user_special_dir(GLib.UserDirectory.DOCUMENTS));

	_thumbnails_dir = GLib.File.new_for_path(GLib.Path.build_filename(GLib.Environment.get_user_cache_dir(), "thumbnails"));
	try {
		_thumbnails_dir.make_directory();
	} catch (Error e) {
		/* Nobody cares */
	}

	_thumbnails_normal_dir = GLib.File.new_for_path(GLib.Path.build_filename(_thumbnails_dir.get_path(), "normal"));
	try {
		_thumbnails_normal_dir.make_directory();
	} catch (Error e) {
		/* Nobody cares */
	}

	_log_file = GLib.File.new_for_path(GLib.Path.build_filename(_logs_dir.get_path(), new GLib.DateTime.now_local().format("%Y-%m-%d") + ".log"));
	_log_stream = GLib.FileStream.open(_log_file.get_path(), "a");

	if (_log_prefix == "launcher")
		return launcher_main(args);

	// Spawn launcher process.
	try {
		string[] child_args = args;
		child_args += "--launcher";
#if CROWN_PLATFORM_WINDOWS
		child_args += ((uint64)OpenProcess(0x00100000 /* SYNCHRONIZE */, true, GetCurrentProcessId())).to_string();
#elif CROWN_PLATFORM_LINUX
		child_args += ((uint64)getpid()).to_string();
#endif
		GLib.Pid launcher_pid;

		GLib.Process.spawn_async(null
			, child_args
			, null
			, 0
			, null
			, out launcher_pid
			);
	} catch (GLib.SpawnError e) {
		loge("%s".printf(e.message));
		return 1;
	}

	_settings_file = GLib.File.new_for_path(GLib.Path.build_filename(_config_dir.get_path(), "settings.sjson"));
	_window_state_file = GLib.File.new_for_path(GLib.Path.build_filename(_data_dir.get_path(), "window.sjson"));
	_user_file = GLib.File.new_for_path(GLib.Path.build_filename(_data_dir.get_path(), "user.sjson"));
	_console_history_file = GLib.File.new_for_path(GLib.Path.build_filename(_data_dir.get_path(), "console_history.txt"));

	// Find toolchain path, more desirable paths come first.
	ii = 0;
	string toolchain_paths[] =
	{
		"../../..",         // Relative path in release package.
		"../../../samples", // Relative path in git worktree.
		".",
	};
	for (ii = 0; ii < toolchain_paths.length; ++ii) {
		string path = Path.build_filename(toolchain_paths[ii], "core");
		if (GLib.FileUtils.test(path, FileTest.EXISTS) && GLib.FileUtils.test(path, FileTest.IS_DIR)) {
			_toolchain_dir = File.new_for_path(path).get_parent();
			break;
		}
	}
	if (ii == toolchain_paths.length) {
		loge("Unable to find the toolchain directory");
		return 1;
	}

	// Find templates path, more desirable paths come first.
	string templates_path[] =
	{
		"../../..", // Relative path in release package or git worktree.
		".",
	};
	for (ii = 0; ii < templates_path.length; ++ii) {
		string path = Path.build_filename(templates_path[ii], "samples");
		if (GLib.FileUtils.test(path, FileTest.EXISTS) && GLib.FileUtils.test(path, FileTest.IS_DIR)) {
			_templates_dir = File.new_for_path(path);
			break;
		}
	}
	if (ii == templates_path.length) {
		loge("Unable to find the templates directory");
		return 1;
	}

#if CROWN_PLATFORM_LINUX
	Gdk.set_allowed_backends("x11");
#endif

	// Use fontconfig backend.
	Pango.FontMap fontmap = Pango.CairoFontMap.new_for_font_type(Cairo.FontType.FT);
	Pango.CairoFontMap.set_default((Pango.CairoFontMap)fontmap);

	LevelEditorApplication app = new LevelEditorApplication();
	return app.run(args);
}

} /* namespace Crown */
