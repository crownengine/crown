/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gdk; // Pixbuf
using Gee;
using Gtk;

namespace Crown
{
const int WINDOW_DEFAULT_WIDTH = 1280;
const int WINDOW_DEFAULT_HEIGHT = 720;
const string CROWN_EDITOR_NAME = "Crown Editor";
const string CROWN_EDITOR_MAIN_WINDOW_TITLE = CROWN_EDITOR_NAME + " " + CROWN_VERSION;
const string CROWN_EDITOR_ICON_NAME = "crown-black-socket";

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

	COUNT
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
	public SubprocessLauncher _subprocess_launcher;
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

	public RuntimeInstance(SubprocessLauncher sl, string name)
	{
		_subprocess_launcher = sl;
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

	private void on_client_connected(string address, int port)
	{
		connected(this, address, port);
	}

	private void on_client_disconnected()
	{
		disconnected(this);

		if (_stop_callback != null)
			_stop_callback();
	}

	private void on_client_disconnected_unexpected()
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

	private void on_client_message_received(ConsoleClient client, uint8[] json)
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

public class LevelEditorWindow : Gtk.ApplicationWindow
{
	private const GLib.ActionEntry[] action_entries =
	{
		{ "fullscreen", on_fullscreen, null, null }
	};

	public bool _fullscreen;

	public LevelEditorWindow(Gtk.Application app)
	{
		Object(application: app);

		this.add_action_entries(action_entries, this);

		this.title = CROWN_EDITOR_MAIN_WINDOW_TITLE;
		this.key_press_event.connect(this.on_key_press);
		this.key_release_event.connect(this.on_key_release);
		this.window_state_event.connect(this.on_window_state_event);
		this.delete_event.connect(this.on_delete_event);
		this.focus_out_event.connect(this.on_focus_out);

		_fullscreen = false;
		this.set_default_size(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);
	}

	private void on_fullscreen(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_fullscreen)
			unfullscreen();
		else
			fullscreen();
	}

	private bool on_key_press(Gdk.EventKey ev)
	{
		LevelEditorApplication app = (LevelEditorApplication)application;

		string str = "";

		if (ev.keyval == Gdk.Key.Control_L)
			str += LevelEditorApi.key_down("ctrl_left");
		else if (ev.keyval == Gdk.Key.Shift_L)
			str += LevelEditorApi.key_down("shift_left");
		else if (ev.keyval == Gdk.Key.Alt_L)
			str += LevelEditorApi.key_down("alt_left");

		if (str.length != 0) {
			app._editor.send_script(str);
			app._editor.send(DeviceApi.frame());
		}

		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_key_release(Gdk.EventKey ev)
	{
		LevelEditorApplication app = (LevelEditorApplication)application;

		string str = "";

		if (ev.keyval == Gdk.Key.Control_L)
			str += LevelEditorApi.key_up("ctrl_left");
		else if (ev.keyval == Gdk.Key.Shift_L)
			str += LevelEditorApi.key_up("shift_left");
		else if (ev.keyval == Gdk.Key.Alt_L)
			str += LevelEditorApi.key_up("alt_left");

		if (str.length != 0) {
			app._editor.send_script(str);
			app._editor.send(DeviceApi.frame());
		}

		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_window_state_event(Gdk.EventWindowState ev)
	{
		_fullscreen = (ev.new_window_state & Gdk.WindowState.FULLSCREEN) != 0;
		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_delete_event()
	{
		LevelEditorApplication app = (LevelEditorApplication)application;

		if (app.should_quit())
			app.stop_backend_and_quit();

		return Gdk.EVENT_STOP; // Keep window alive.
	}

	private bool on_focus_out(Gdk.EventFocus ev)
	{
		LevelEditorApplication app = (LevelEditorApplication)application;

		app._editor.send_script(LevelEditorApi.key_up("ctrl_left"));
		app._editor.send_script(LevelEditorApi.key_up("shift_left"));
		app._editor.send_script(LevelEditorApi.key_up("alt_left"));
		return Gdk.EVENT_PROPAGATE;
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
	// Constants
	private const GLib.ActionEntry[] action_entries_file =
	{
		//                                   parameter type
		// name            activate()        |     state
		// |               |                 |     |
		{ "menu-file",     null,             null, null },
		{ "new-level",     on_new_level,     null, null },
		{ "open-level",    on_open_level,    "s",  null },
		{ "new-project",   on_new_project,   null, null },
		{ "add-project",   on_add_project,   null, null },
		{ "open-project",  on_open_project,  "s",  null },
		{ "save",          on_save,          null, null },
		{ "save-as",       on_save_as,       null, null },
		{ "import",        on_import,        "s",  null },
		{ "import-null",   on_import,        null, null },
		{ "preferences",   on_preferences,   null, null },
		{ "deploy",        on_deploy,        null, null },
		{ "close-project", on_close_project, null, null },
		{ "quit",          on_quit,          null, null },
		{ "open-resource", on_open_resource, "s",  null },
		{ "copy-path",     on_copy_path,     "s",  null },
		{ "copy-name",     on_copy_name,     "s",  null }
	};

	private const GLib.ActionEntry[] action_entries_edit =
	{
		{ "menu-edit",          null,                  null,   null    },
		{ "undo",               on_undo,               null,   null    },
		{ "redo",               on_redo,               null,   null    },
		{ "duplicate",          on_duplicate,          null,   null    },
		{ "delete",             on_delete,             null,   null    },
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

	private const GLib.ActionEntry[] action_entries_create =
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
		{ "unit-empty",         on_spawn_unit,      null, null }
	};

	private const GLib.ActionEntry[] action_entries_camera =
	{
		{ "menu-camera",           null,                     null, null },
		{ "camera-view",           on_camera_view,           "i",  "0"  }, // See: Crown.CameraViewType
		{ "camera-frame-selected", on_camera_frame_selected, null, null },
		{ "camera-frame-all",      on_camera_frame_all,      null, null }
	};

	private const GLib.ActionEntry[] action_entries_view =
	{
		{ "menu-view",           null,                   null, null    },
		{ "resource-chooser",    on_resource_chooser,    null, null    },
		{ "project-browser",     on_project_browser,     null, null    },
		{ "console",             on_console,             null, null    },
		{ "statusbar",           on_statusbar,           null, null    },
		{ "inspector",           on_inspector,           null, null    },
		{ "debug-render-world",  on_debug_render_world,  null, "false" },
		{ "debug-physics-world", on_debug_physics_world, null, "false" }
	};

	private const GLib.ActionEntry[] action_entries_debug =
	{
		{ "menu-debug",          null,                   null, null },
		{ "test-level",          on_run_game,            null, null },
		{ "run-game",            on_run_game,            null, null },
		{ "build-data",          on_build_data,          null, null },
		{ "reload-lua",          on_refresh_lua,         null, null },
		{ "restart-editor-view", on_restart_editor_view, null, null }
	};

	private const GLib.ActionEntry[] action_entries_help =
	{
		{ "menu-help",    null,            null, null },
		{ "manual",       on_manual,       null, null },
		{ "report-issue", on_report_issue, null, null },
		{ "browse-logs",  on_browse_logs,  null, null },
		{ "changelog",    on_changelog,    null, null },
		{ "donate",       on_donate,       null, null },
		{ "about",        on_about,        null, null }
	};

	private const GLib.ActionEntry[] action_entries_project =
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
		{ "reveal-resource",      on_reveal,               "(ss)",  null }
	};

	private const GLib.ActionEntry[] action_entries_package =
	{
		{ "create-package-android", on_create_package_android, "(sississsssi)", null },
		{ "create-package-html5",   on_create_package_html5,   "(sis)",         null },
		{ "create-package-linux",   on_create_package_linux,   "(sis)",         null },
		{ "create-package-windows", on_create_package_windows, "(sis)",         null }
	};

	private const GLib.ActionEntry[] action_entries_unit =
	{
		{ "unit-add-component",    on_unit_add_component,    "s", null },
		{ "unit-remove-component", on_unit_remove_component, "s", null }
	};

	// Command line options
	private string? _source_dir = null;
	private string _level_resource = "";
	private User _user;
	private Hashtable _settings;
	private Hashtable _window_state;

	// Subprocess launcher service.
	private SubprocessLauncher _subprocess_launcher;

	// Editor state
	private double _grid_size;
	private double _rotation_snap;
	private bool _show_grid;
	private bool _snap_to_grid;
	private bool _debug_render_world;
	private bool _debug_physics_world;
	private ToolType _tool_type;
	private ToolType _tool_type_prev;
	private SnapMode _snap_mode;
	private ReferenceSystem _reference_system;

	// Project state
	private string _placeable_type;
	private string _placeable_name;

	// Accelerators
	private string[] _tool_place_accels;
	private string[] _tool_move_accels;
	private string[] _tool_rotate_accels;
	private string[] _tool_scale_accels;
	private string[] _delete_accels;
	private string[] _camera_view_perspective_accels;
	private string[] _camera_view_front_accels;
	private string[] _camera_view_back_accels;
	private string[] _camera_view_right_accels;
	private string[] _camera_view_left_accels;
	private string[] _camera_view_top_accels;
	private string[] _camera_view_bottom_accels;
	private string[] _camera_frame_selected_accels;
	private string[] _camera_frame_all_accels;

	// Engine connections
	private RuntimeInstance _compiler;
	public RuntimeInstance _editor;
	private RuntimeInstance _resource_preview;
	private RuntimeInstance _game;
	private RuntimeInstance _thumbnail;

	// Level data
	private UndoRedo _undo_redo;
	private Database _database;
	private Project _project;
	private ProjectStore _project_store;
	private Level _level;
	private DataCompiler _data_compiler;

	// Widgets
	private Gtk.CssProvider _css_provider;
	private ProjectBrowser _project_browser;
	private EditorView _editor_view;
	private EditorView _resource_preview_view;
	private LevelTreeView _level_treeview;
	private LevelLayersTreeView _level_layers_treeview;
	private PropertiesView _properties_view;
	private PreferencesDialog _preferences_dialog;
	private DeployDialog _deploy_dialog;
	private TextureSettingsDialog _texture_settings_dialog;
	private ResourceChooser _resource_chooser;
	private Gtk.Popover _resource_popover;
	private Gtk.Overlay _editor_view_overlay;
	private ThumbnailCache _thumbnail_cache;

	private Gtk.Stack _project_stack;
	private Gtk.Label _project_stack_compiling_data_label;
	private Gtk.Label _project_stack_connecting_to_data_compiler_label;
	private Gtk.Label _project_stack_compiler_crashed_label;
	private Gtk.Label _project_stack_compiler_failed_compilation_label;
	private Gtk.Label _project_stack_stopping_backend_label;

	private Gtk.Stack _editor_stack;
	private Gtk.Label _editor_stack_compiling_data_label;
	private Gtk.Label _editor_stack_connecting_to_data_compiler_label;
	private Gtk.Label _editor_stack_compiler_crashed_label;
	private Gtk.Label _editor_stack_compiler_failed_compilation_label;
	private Gtk.Label _editor_stack_disconnected_label;
	private Gtk.Label _editor_stack_oops_label;
	private Gtk.Label _editor_stack_stopping_backend_label;

	public Gtk.Stack _resource_preview_stack;
	public Gtk.Label _resource_preview_disconnected_label;
	public Gtk.Label _resource_preview_oops_label;
	public Gtk.Label _resource_preview_no_preview_label;

	private Gtk.Stack _inspector_stack;
	private Gtk.Label _inspector_stack_compiling_data_label;
	private Gtk.Label _inspector_stack_connecting_to_data_compiler_label;
	private Gtk.Label _inspector_stack_compiler_crashed_label;
	private Gtk.Label _inspector_stack_compiler_failed_compilation_label;
	private Gtk.Label _inspector_stack_stopping_backend_label;

	private Gtk.Toolbar _toolbar;
	private Gtk.ToolButton _toolbar_run;
	private Gtk.Notebook _level_tree_view_notebook;
	private Gtk.Notebook _console_notebook;
	private Gtk.Notebook _project_notebook;
	private Gtk.Notebook _inspector_notebook;
	private Gtk.Paned _editor_pane;
	private Gtk.Paned _content_pane;
	private Gtk.Paned _inspector_pane;
	private Gtk.Paned _main_pane;
	private Statusbar _statusbar;
	private Gtk.Box _main_vbox;
	private Gtk.FileFilter _file_filter;
	private Gtk.ComboBoxText _combo;
	private PanelNewProject _panel_new_project;
	private PanelProjectsList _panel_projects_list;
	private PanelWelcome _panel_welcome;
	private Gtk.Stack _main_stack;

	private uint _save_timer_id;

	public LevelEditorApplication(SubprocessLauncher subprocess_launcher)
	{
		Object(application_id: "org.crownengine.Crown"
			, flags: GLib.ApplicationFlags.FLAGS_NONE
			);

		_subprocess_launcher = subprocess_launcher;
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

	protected override void startup()
	{
		base.startup();

		Intl.setlocale(LocaleCategory.ALL, "C");

		_css_provider = new Gtk.CssProvider();
		var default_screen = Gdk.Display.get_default().get_default_screen();
		Gtk.StyleContext.add_provider_for_screen(default_screen
			, _css_provider
			, STYLE_PROVIDER_PRIORITY_APPLICATION
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

		// HACK: register CrownClamp type within GObject's type system to
		// make GtkBuilder able to find it when creating the widget from
		// .ui files.
		// https://stackoverflow.com/questions/24235937/custom-gtk-widget-with-template-ui
		new Clamp().get_type().ensure();

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

		_tool_place_accels = this.get_accels_for_action("app.tool(0)");
		_tool_move_accels = this.get_accels_for_action("app.tool(1)");
		_tool_rotate_accels = this.get_accels_for_action("app.tool(2)");
		_tool_scale_accels = this.get_accels_for_action("app.tool(3)");
		_delete_accels = this.get_accels_for_action("app.delete");
		_camera_view_perspective_accels = this.get_accels_for_action("app.camera-view(0)");
		_camera_view_front_accels = this.get_accels_for_action("app.camera-view(1)");
		_camera_view_back_accels = this.get_accels_for_action("app.camera-view(2)");
		_camera_view_right_accels = this.get_accels_for_action("app.camera-view(3)");
		_camera_view_left_accels = this.get_accels_for_action("app.camera-view(4)");
		_camera_view_top_accels = this.get_accels_for_action("app.camera-view(5)");
		_camera_view_bottom_accels = this.get_accels_for_action("app.camera-view(6)");
		_camera_frame_selected_accels = this.get_accels_for_action("app.camera-frame-selected");
		_camera_frame_all_accels = this.get_accels_for_action("app.camera-frame-all");

		_compiler = new RuntimeInstance(_subprocess_launcher, "data_compiler");
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
		_project.register_importer("Sound", { "wav" }, SoundResource.import, on_import_result, 2.0);
		_project.register_importer("Texture", { "png", "tga", "dds", "ktx", "pvr" }, TextureResource.import, on_import_result, 2.0);
		_project.register_importer("Font", { "ttf", "otf" }, FontResource.import, on_import_result, 3.0);
		_project.project_reset.connect(on_project_reset);
		_project.project_loaded.connect(on_project_loaded);

		_editor = new RuntimeInstance(_subprocess_launcher, "editor");
		_editor.message_received.connect(on_message_received);
		_editor.connected.connect(on_editor_connected);
		_editor.disconnected.connect(on_runtime_disconnected);
		_editor.disconnected_unexpected.connect(on_editor_disconnected_unexpected);

		_preferences_dialog = new PreferencesDialog(_editor);
		_preferences_dialog.delete_event.connect(_preferences_dialog.hide_on_delete);
		_preferences_dialog.decode(_settings);

		set_theme_from_name(_preferences_dialog._theme_combo.value);

		_resource_preview = new RuntimeInstance(_subprocess_launcher, "resource_preview");
		_resource_preview.message_received.connect(on_message_received);
		_resource_preview.connected.connect(on_runtime_connected);
		_resource_preview.disconnected.connect(on_runtime_disconnected);
		_resource_preview.disconnected_unexpected.connect(on_resource_preview_disconnected_unexpected);

		_game = new RuntimeInstance(_subprocess_launcher, "game");
		_game.message_received.connect(on_message_received);
		_game.connected.connect(on_game_connected);
		_game.disconnected.connect(on_game_disconnected);
		_game.disconnected_unexpected.connect(on_game_disconnected);

		_thumbnail = new RuntimeInstance(_subprocess_launcher, "thumbnail");
		_thumbnail.message_received.connect(on_message_received);
		_thumbnail.connected.connect(on_runtime_connected);
		_thumbnail.disconnected.connect(on_runtime_disconnected);
		_thumbnail.disconnected_unexpected.connect(on_runtime_disconnected_unexpected);

		_undo_redo = new UndoRedo((uint)_preferences_dialog._undo_redo_max_size.value * 1024 * 1024);
		_database = new Database(_project, _undo_redo);
		_database.key_changed.connect(() => { update_active_window_title(); });
		_database.restore_point_added.connect(on_restore_point_added);
		_database.undo_redo.connect(on_undo_redo);

		_level = new Level(_database, _editor, _project);

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

		// Register component types.
		Unit.register_component_type(OBJECT_TYPE_TRANSFORM,               "");
		Unit.register_component_type(OBJECT_TYPE_LIGHT,                   OBJECT_TYPE_TRANSFORM);
		Unit.register_component_type(OBJECT_TYPE_CAMERA,                  OBJECT_TYPE_TRANSFORM);
		Unit.register_component_type(OBJECT_TYPE_MESH_RENDERER,           OBJECT_TYPE_TRANSFORM);
		Unit.register_component_type(OBJECT_TYPE_SPRITE_RENDERER,         OBJECT_TYPE_TRANSFORM);
		Unit.register_component_type(OBJECT_TYPE_COLLIDER,                OBJECT_TYPE_TRANSFORM);
		Unit.register_component_type(OBJECT_TYPE_ACTOR,                   OBJECT_TYPE_TRANSFORM);
		Unit.register_component_type(OBJECT_TYPE_SCRIPT,                  "");
		Unit.register_component_type(OBJECT_TYPE_ANIMATION_STATE_MACHINE, "");

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
		_level_layers_treeview = new LevelLayersTreeView(_database, _level);
		_properties_view = new PropertiesView(_database, _project_store);
		_level.selection_changed.connect(_properties_view.on_selection_changed);

		_project_stack = new Gtk.Stack();
		_project_stack.add(_project_browser);
		_project_stack_compiling_data_label = compiling_data_label();
		_project_stack.add(_project_stack_compiling_data_label);
		_project_stack_connecting_to_data_compiler_label = connecting_to_data_compiler_label();
		_project_stack.add(_project_stack_connecting_to_data_compiler_label);
		_project_stack_compiler_crashed_label = compiler_crashed_label();
		_project_stack.add(_project_stack_compiler_crashed_label);
		_project_stack_compiler_failed_compilation_label = compiler_failed_compilation_label();
		_project_stack.add(_project_stack_compiler_failed_compilation_label);
		_project_stack_stopping_backend_label = stopping_backend_label();
		_project_stack.add(_project_stack_stopping_backend_label);

		_editor_stack = new Gtk.Stack();
		_editor_stack_compiling_data_label = compiling_data_label();
		_editor_stack.add(_editor_stack_compiling_data_label);
		_editor_stack_connecting_to_data_compiler_label = connecting_to_data_compiler_label();
		_editor_stack.add(_editor_stack_connecting_to_data_compiler_label);
		_editor_stack_compiler_crashed_label = compiler_crashed_label();
		_editor_stack.add(_editor_stack_compiler_crashed_label);
		_editor_stack_compiler_failed_compilation_label = compiler_failed_compilation_label();
		_editor_stack.add(_editor_stack_compiler_failed_compilation_label);
		_editor_stack_disconnected_label = new Gtk.Label("Disconnected.");
		_editor_stack.add(_editor_stack_disconnected_label);
		_editor_stack_oops_label = new Gtk.Label(null);
		_editor_stack_oops_label.track_visited_links = false;
		_editor_stack_oops_label.set_markup("Something went wrong.\rTry to <a href=\"restart\">restart this view</a>.");
		_editor_stack_oops_label.activate_link.connect(() => {
				activate_action("restart-editor-view", null);
				return true;
			});
		_editor_stack.add(_editor_stack_oops_label);
		_editor_stack_stopping_backend_label = stopping_backend_label();
		_editor_stack.add(_editor_stack_stopping_backend_label);

		_resource_preview_stack = new Gtk.Stack();
		_resource_preview_no_preview_label = new Gtk.Label("No Preview");
		_resource_preview_no_preview_label.set_size_request(300, 300);
		_resource_preview_stack.add(_resource_preview_no_preview_label);
		_resource_preview_disconnected_label = new Gtk.Label("Disconnected");
		_resource_preview_stack.add(_resource_preview_disconnected_label);
		_resource_preview_oops_label = new Gtk.Label(null);
		_resource_preview_oops_label.track_visited_links = false;
		_resource_preview_oops_label.set_markup("Something went wrong.\rTry to <a href=\"restart\">restart this view</a>.");
		_resource_preview_oops_label.activate_link.connect(() => {
				restart_resource_preview.begin((obj, res) => {
						restart_resource_preview.end(res);
					});
				return true;
			});
		_resource_preview_stack.add(_resource_preview_oops_label);

		_inspector_stack = new Gtk.Stack();
		_inspector_stack_compiling_data_label = compiling_data_label();
		_inspector_stack.add(_inspector_stack_compiling_data_label);
		_inspector_stack_connecting_to_data_compiler_label = connecting_to_data_compiler_label();
		_inspector_stack.add(_inspector_stack_connecting_to_data_compiler_label);
		_inspector_stack_compiler_crashed_label = compiler_crashed_label();
		_inspector_stack.add(_inspector_stack_compiler_crashed_label);
		_inspector_stack_compiler_failed_compilation_label = compiler_failed_compilation_label();
		_inspector_stack.add(_inspector_stack_compiler_failed_compilation_label);
		_inspector_stack_stopping_backend_label = stopping_backend_label();
		_inspector_stack.add(_inspector_stack_stopping_backend_label);

		Gtk.Builder builder = new Gtk.Builder.from_resource("/org/crownengine/Crown/ui/toolbar.ui");
		_toolbar = builder.get_object("toolbar") as Gtk.Toolbar;
		_toolbar_run = builder.get_object("run") as Gtk.ToolButton;

		_editor_view_overlay = new Gtk.Overlay();
		_editor_view_overlay.add_overlay(_toolbar);

		_resource_popover = new Gtk.Popover(_toolbar);
		_resource_popover.key_press_event.connect((ev) => {
				if (ev.keyval == Gdk.Key.Escape) {
					// Do not transition-animate (i.e. call hide() instead of popdown()).
					_resource_popover.hide();
					return Gdk.EVENT_STOP;
				}

				return Gdk.EVENT_PROPAGATE;
			});
		_resource_popover.button_press_event.connect((ev) => {
				// Do not transition-animate (i.e. call hide() instead of popdown()).
				// See: https://gitlab.gnome.org/GNOME/gtk/-/blob/3.22.30/gtk/gtkpopover.c
				Gtk.Widget child = _resource_popover.get_child();
				Gtk.Widget event_widget = Gtk.get_event_widget(ev);

				if (child != null && ev.window == event_widget.get_window()) {
					Gtk.Allocation child_alloc;
					child.get_allocation(out child_alloc);

					if ((int)ev.x < child_alloc.x
						|| (int)ev.x > child_alloc.x + child_alloc.width
						|| (int)ev.y < child_alloc.y
						|| (int)ev.y > child_alloc.y + child_alloc.height
						) {
						_resource_popover.hide();
					}
				} else if (!event_widget.is_ancestor(_resource_popover)) {
					_resource_popover.hide();
				}

				return Gdk.EVENT_PROPAGATE;
			});
		_resource_popover.events |= Gdk.EventMask.STRUCTURE_MASK; // unmap_event
		_resource_popover.unmap_event.connect(() => {
				// Redraw the editor view when the popover is not on-screen anymore.
				device_frame_delayed(16, _editor);
				return Gdk.EVENT_PROPAGATE;
			});
		_resource_popover.delete_event.connect(_resource_popover.hide_on_delete);
		_resource_popover.modal = true;

		_resource_chooser = new ResourceChooser(_project, _project_store, _resource_preview_stack, _resource_preview);
		_resource_chooser.resource_selected.connect(on_resource_browser_resource_selected);
		_resource_chooser.destroy.connect(() => {
				stop_resource_preview.begin((obj, res) => {
						stop_resource_preview.end(res);
					});
			});

		_resource_popover.add(_resource_chooser);

		_level_tree_view_notebook = new Notebook();
		_level_tree_view_notebook.show_border = false;
		_level_tree_view_notebook.append_page(_level_treeview, new Gtk.Image.from_icon_name("level-tree", IconSize.SMALL_TOOLBAR));
		_level_tree_view_notebook.append_page(_level_layers_treeview, new Gtk.Image.from_icon_name("level-layers", IconSize.SMALL_TOOLBAR));

		_console_notebook = new Notebook();
		_console_notebook.show_border = false;
		_console_notebook.append_page(_console_view, new Gtk.Label.with_mnemonic("Console"));

		_project_notebook = new Notebook();
		_project_notebook.show_border = false;
		_project_notebook.append_page(_project_stack, new Gtk.Label.with_mnemonic("Project"));

		_inspector_notebook = new Notebook();
		_inspector_notebook.show_border = false;
		_inspector_notebook.append_page(_properties_view, new Gtk.Label.with_mnemonic("Properties"));

		_editor_pane = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
		_editor_pane.pack1(_project_notebook, false, false);
		_editor_pane.pack2(_editor_stack, true, false);

		_content_pane = new Gtk.Paned(Gtk.Orientation.VERTICAL);
		_content_pane.pack1(_editor_pane, true, false);
		_content_pane.pack2(_console_notebook, false, false);

		_inspector_pane = new Gtk.Paned(Gtk.Orientation.VERTICAL);
		_inspector_pane.pack1(_level_tree_view_notebook, true, false);
		_inspector_pane.pack2(_inspector_notebook, false, false);
		_inspector_stack.add(_inspector_pane);

		_main_pane = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
		_main_pane.pack1(_content_pane, true, false);
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
		_panel_new_project = new PanelNewProject(_user, _project);
		_panel_new_project.fill_templates_list(_templates_dir.get_path());

		_panel_welcome = new PanelWelcome();
		_panel_projects_list = new PanelProjectsList(_user);
		_panel_welcome.pack_start(_panel_projects_list);
		_panel_welcome.set_visible(true); // To make Gtk.Stack work...

		_main_stack = new Gtk.Stack();
		_main_stack.add_named(_panel_welcome, "panel_welcome");
		_main_stack.add_named(_panel_new_project, "panel_new_project");
		_main_stack.add_named(_main_vbox, "main_vbox");

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

		if (_source_dir == null) {
			show_panel("panel_welcome");
		} else {
			show_panel("main_vbox");
			restart_backend.begin(_source_dir, _level_resource);
		}
	}

	protected override void activate()
	{
		if (this.active_window == null) {
			LevelEditorWindow win = new LevelEditorWindow(this);
			if (_window_state.has_key("level_editor_window"))
				win.decode((Hashtable)_window_state["level_editor_window"]);
			win.add(_main_stack);

			try {
				win.icon = IconTheme.get_default().load_icon(CROWN_EDITOR_ICON_NAME, 256, 0);
			} catch (Error e) {
				loge(e.message);
			}
		}

		this.active_window.show_all();
	}

	protected override bool local_command_line(ref unowned string[] args, out int exit_status)
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

	protected override int command_line(ApplicationCommandLine command_line)
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

	private void on_resource_browser_resource_selected(string type, string name)
	{
		activate_action("set-placeable", new GLib.Variant.tuple({ type, name }));
	}

	private void on_runtime_connected(RuntimeInstance ri, string address, int port)
	{
		logi("Connected to %s@%s:%d".printf(ri._name, address, port));
	}

	private void on_runtime_disconnected(RuntimeInstance ri)
	{
		logi("Disconnected from %s".printf(ri._name));
	}

	private void on_runtime_disconnected_unexpected(RuntimeInstance ri)
	{
		logw("Disconnected from %s unexpectedly".printf(ri._name));
	}

	private async void on_data_compiler_disconnected_unexpected(RuntimeInstance ri)
	{
		on_runtime_disconnected_unexpected(ri);

		yield stop_heads();

		// Reset the callback
		_data_compiler.compile_finished(false, 0);

		_project_stack.set_visible_child(_project_stack_compiler_crashed_label);
		_editor_stack.set_visible_child(_editor_stack_compiler_crashed_label);
		_inspector_stack.set_visible_child(_inspector_stack_compiler_crashed_label);
	}

	private void on_data_compiler_start()
	{
		_statusbar.set_status("Compiling data...");
	}

	private void on_data_compiler_finished(bool success)
	{
		_statusbar.clear_status();

		if (!success) {
			_statusbar.set_temporary_message("Failed to compile data");
			return;
		}

		refresh_all_clients.begin((obj, res) => {
				refresh_all_clients.end(res);
				_project.data_compiled();
				_project_browser.queue_draw();
			});
	}

	private void on_editor_connected(RuntimeInstance ri, string address, int port)
	{
		on_runtime_connected(ri, address, port);

		// Update editor view with current editor state.
		_level.send_level();
		send_state();
		_preferences_dialog.apply();
		_editor.send(DeviceApi.frame());
	}

	private void on_editor_disconnected_unexpected(RuntimeInstance ri)
	{
		on_runtime_disconnected_unexpected(ri);

		_editor_stack.set_visible_child(_editor_stack_oops_label);
	}

	private void on_resource_preview_disconnected_unexpected(RuntimeInstance ri)
	{
		on_runtime_disconnected_unexpected(ri);

		_resource_preview_stack.set_visible_child(_resource_preview_oops_label);
	}

	private void on_game_connected(RuntimeInstance ri, string address, int port)
	{
		on_runtime_connected(ri, address, port);

		_combo.set_active_id("game");
	}

	private void on_game_disconnected(RuntimeInstance ri)
	{
		on_runtime_disconnected(ri);

		_combo.set_active_id("editor");
		_toolbar_run.icon_name = "game-run";
	}

	private void on_message_received(RuntimeInstance ri, ConsoleClient client, uint8[] json)
	{
		try {
			Hashtable msg = JSON.decode(json) as Hashtable;
			handle_message(ri, client, (string)msg["type"], msg);
		} catch (JsonSyntaxError e) {
			loge(e.message);
		}
	}

	private void handle_message(RuntimeInstance ri, ConsoleClient client, string msg_type, Hashtable msg)
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

			_project.change_file(path, size, mtime);
		} else if (msg_type == "compile") {
			_data_compiler.message(msg);
		} else if (msg_type == "refresh") {
			ri.refresh_finished((bool)msg["success"]);
		} else if (msg_type == "refresh_list") {
			_data_compiler.refresh_list_finished((ArrayList<Value?>)msg["list"]);
		} else if (msg_type == "unit_spawned") {
			Guid id               = Guid.parse((string)msg["id"]);
			string name           = (string)msg["name"];
			ArrayList<Value?> pos = (ArrayList<Value?>)msg["position"];
			ArrayList<Value?> rot = (ArrayList<Value?>)msg["rotation"];
			ArrayList<Value?> scl = (ArrayList<Value?>)msg["scale"];

			_level.on_unit_spawned(id
				, name
				, Vector3.from_array(pos)
				, Quaternion.from_array(rot)
				, Vector3.from_array(scl)
				);
			_database.add_restore_point((int)ActionType.SPAWN_UNIT, new Guid?[] { id }, ActionTypeFlags.FROM_SERVER);
		} else if (msg_type == "sound_spawned") {
			Guid id               = Guid.parse((string)msg["id"]);
			string name           = (string)msg["name"];
			ArrayList<Value?> pos = (ArrayList<Value?>)msg["position"];
			ArrayList<Value?> rot = (ArrayList<Value?>)msg["rotation"];
			ArrayList<Value?> scl = (ArrayList<Value?>)msg["scale"];
			double range          = (double)msg["range"];
			double volume         = (double)msg["volume"];
			bool loop             = (bool)msg["loop"];

			_level.on_sound_spawned(id
				, name
				, Vector3.from_array(pos)
				, Quaternion.from_array(rot)
				, Vector3.from_array(scl)
				, range
				, volume
				, loop
				);
			_database.add_restore_point((int)ActionType.SPAWN_SOUND, new Guid?[] { id }, ActionTypeFlags.FROM_SERVER);
		} else if (msg_type == "move_objects") {
			Hashtable ids           = (Hashtable)msg["ids"];
			Hashtable new_positions = (Hashtable)msg["new_positions"];
			Hashtable new_rotations = (Hashtable)msg["new_rotations"];
			Hashtable new_scales    = (Hashtable)msg["new_scales"];

			ArrayList<string> keys = new ArrayList<string>.wrap(ids.keys.to_array());
			keys.sort(Gee.Functions.get_compare_func_for(typeof(string)));

			Guid?[] n_ids            = new Guid?[keys.size];
			Vector3[] n_positions    = new Vector3[keys.size];
			Quaternion[] n_rotations = new Quaternion[keys.size];
			Vector3[] n_scales       = new Vector3[keys.size];

			for (int i = 0; i < keys.size; ++i) {
				string k = keys[i];

				n_ids[i]       = Guid.parse((string)ids[k]);
				n_positions[i] = Vector3.from_array((ArrayList<Value?>)(new_positions[k]));
				n_rotations[i] = Quaternion.from_array((ArrayList<Value?>)new_rotations[k]);
				n_scales[i]    = Vector3.from_array((ArrayList<Value?>)new_scales[k]);
			}

			_level.on_move_objects(n_ids, n_positions, n_rotations, n_scales);
			_database.add_restore_point((int)ActionType.MOVE_OBJECTS, n_ids, ActionTypeFlags.FROM_SERVER);
		} else if (msg_type == "selection") {
			Hashtable objects = (Hashtable)msg["objects"];

			ArrayList<string> keys = new ArrayList<string>.wrap(objects.keys.to_array());
			keys.sort(Gee.Functions.get_compare_func_for(typeof(string)));

			Guid[] ids = new Guid[keys.size];

			for (int i = 0; i < keys.size; ++i) {
				string k = keys[i];
				ids[i] = Guid.parse((string)objects[k]);
			}

			_level.on_selection(ids);
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

	private void append_editor_state(StringBuilder sb)
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

	private void append_project_state(StringBuilder sb)
	{
		// This state is not guaranteed to be applicable to any project.
		if (_placeable_type != "")
			sb.append(LevelEditorApi.set_placeable(_placeable_type, _placeable_name));
	}

	private void send_state()
	{
		StringBuilder sb = new StringBuilder();
		append_editor_state(sb);
		append_project_state(sb);
		_editor.send_script(sb.str);
	}

	private void on_objects_created(Guid?[] object_ids)
	{
		StringBuilder sb = new StringBuilder();
		_level.generate_spawn_objects(sb, object_ids);
		_editor.send_script(sb.str);
		_editor.send(DeviceApi.frame());
		_level.selection_changed(_level._selection);
	}

	private void on_objects_destroyed(Guid?[] object_ids)
	{
		StringBuilder sb = new StringBuilder();
		_level.generate_destroy_objects(sb, object_ids);
		_editor.send_script(sb.str);
		_editor.send(DeviceApi.frame());
		_level.selection_changed(_level._selection);
	}

	private void on_objects_changed(Guid?[] object_ids)
	{
		StringBuilder sb = new StringBuilder();
		_level.generate_change_objects(sb, object_ids);
		_editor.send_script(sb.str);
		_editor.send(DeviceApi.frame());
	}

	private void on_restore_point_added(int id, Guid?[] data, uint32 flags)
	{
		switch (id) {
		case ActionType.SPAWN_UNIT:
		case ActionType.SPAWN_SOUND:
		case ActionType.DUPLICATE_OBJECTS:
		case ActionType.UNIT_ADD_COMPONENT:
			if ((flags & ActionTypeFlags.FROM_SERVER) == 0)
				on_objects_created(data);
			break;

		case ActionType.DESTROY_OBJECTS:
		case ActionType.UNIT_REMOVE_COMPONENT:
			if ((flags & ActionTypeFlags.FROM_SERVER) == 0)
				on_objects_destroyed(data);
			break;

		case ActionType.MOVE_OBJECTS:
		case ActionType.SET_TRANSFORM:
		case ActionType.SET_LIGHT:
		case ActionType.SET_MESH:
		case ActionType.SET_SPRITE:
		case ActionType.SET_CAMERA:
		case ActionType.SET_COLLIDER:
		case ActionType.SET_ACTOR:
		case ActionType.SET_SCRIPT:
		case ActionType.SET_ANIMATION_STATE_MACHINE:
		case ActionType.SET_SOUND:
			if ((flags & ActionTypeFlags.FROM_SERVER) == 0)
				on_objects_changed(data);
			break;

		case ActionType.OBJECT_SET_EDITOR_NAME:
			on_objects_changed(data);
			_level.object_editor_name_changed(data[0], _level.object_editor_name(data[0]));
			break;

		default:
			logw("Unknown action type %d".printf(id));
			break;
		}

		_properties_view.show_or_hide_properties();
	}

	private void on_undo_redo(bool undo, uint32 id, Guid?[] data)
	{
		switch (id) {
		case ActionType.SPAWN_UNIT:
		case ActionType.SPAWN_SOUND:
		case ActionType.DUPLICATE_OBJECTS:
		case ActionType.UNIT_ADD_COMPONENT:
			if (undo)
				on_objects_destroyed(data);
			else
				on_objects_created(data);
			break;

		case ActionType.DESTROY_OBJECTS:
		case ActionType.UNIT_REMOVE_COMPONENT:
			if (undo)
				on_objects_created(data);
			else
				on_objects_destroyed(data);
			break;

		case ActionType.MOVE_OBJECTS:
		case ActionType.SET_TRANSFORM:
		case ActionType.SET_LIGHT:
		case ActionType.SET_MESH:
		case ActionType.SET_SPRITE:
		case ActionType.SET_CAMERA:
		case ActionType.SET_COLLIDER:
		case ActionType.SET_ACTOR:
		case ActionType.SET_SCRIPT:
		case ActionType.SET_ANIMATION_STATE_MACHINE:
		case ActionType.SET_SOUND:
			on_objects_changed(data);
			break;

		case ActionType.OBJECT_SET_EDITOR_NAME:
			on_objects_changed(data);
			_level.object_editor_name_changed(data[0], _level.object_editor_name(data[0]));
			break;

		default:
			logw("Unknown action type %u".printf(id));
			break;
		}

		_properties_view.show_or_hide_properties();
	}

	private bool on_button_press(Gdk.EventButton ev)
	{
		return Gdk.EVENT_STOP;
	}

	private bool on_button_release(Gdk.EventButton ev)
	{
		return Gdk.EVENT_STOP;
	}

	Gtk.Label compiling_data_label()
	{
		return new Gtk.Label("Compiling resources, please wait...");
	}

	Gtk.Label connecting_to_data_compiler_label()
	{
		return new Gtk.Label("Connecting to Data Compiler...");
	}

	Gtk.Label compiler_crashed_label()
	{
		Gtk.Label label = new Gtk.Label(null);
		label.track_visited_links = false;
		label.set_markup("Data Compiler disconnected.\rTry to <a href=\"restart\">restart the compiler</a> to continue.");
		label.activate_link.connect(() => {
				restart_backend.begin(_project.source_dir(), _level._name != null ? _level._name : "");
				return true;
			});

		return label;
	}

	Gtk.Label compiler_failed_compilation_label()
	{
		Gtk.Label label = new Gtk.Label(null);
		label.track_visited_links = false;
		label.set_markup("Data compilation failed.\rFix errors and <a href=\"restart\">restart the compiler</a> to continue.");
		label.activate_link.connect(() => {
				restart_backend.begin(_project.source_dir(), _level._name != null ? _level._name : "");
				return true;
			});

		return label;
	}

	Gtk.Label stopping_backend_label()
	{
		return new Gtk.Label("Stopping Backend...");
	}

	public async void restart_backend(string source_dir, string level_name)
	{
		string sd = source_dir.dup();
		string ln = level_name.dup();
		yield stop_backend();

		// Reset project state.
		_placeable_type = "unit";
		_placeable_name = "core/units/primitives/cube";

		// Load project and level if any.
		logi("Loading project: `%s`...".printf(sd));
		_project.load(sd);

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

		_project_stack.set_visible_child(_project_stack_connecting_to_data_compiler_label);
		_editor_stack.set_visible_child(_editor_stack_connecting_to_data_compiler_label);
		_inspector_stack.set_visible_child(_inspector_stack_connecting_to_data_compiler_label);

		int tries = yield _compiler.connect_async(DATA_COMPILER_ADDRESS
			, DATA_COMPILER_TCP_PORT
			, DATA_COMPILER_CONNECTION_TRIES
			, DATA_COMPILER_CONNECTION_INTERVAL
			);
		if (tries == DATA_COMPILER_CONNECTION_TRIES) {
			loge("Cannot connect to data_compiler");
			return;
		}

		_project_stack.set_visible_child(_project_stack_compiling_data_label);
		_editor_stack.set_visible_child(_editor_stack_compiling_data_label);
		_inspector_stack.set_visible_child(_inspector_stack_compiling_data_label);

		// Compile data.
		bool success = yield _data_compiler.compile(_project.data_dir(), _project.platform());

		if (!success) {
			_project_stack.set_visible_child(_project_stack_compiler_failed_compilation_label);
			_editor_stack.set_visible_child(_editor_stack_compiler_failed_compilation_label);
			_inspector_stack.set_visible_child(_inspector_stack_compiler_failed_compilation_label);
			return;
		}

		// If successful, start the level editor.
		load_level(ln);
		yield restart_editor();

		_project_stack.set_visible_child(_project_browser);
		_inspector_stack.set_visible_child(_inspector_pane);
		_project_browser.select_project_root();
		return;
	}

	public async void stop_heads()
	{
		yield stop_game();
		yield stop_editor();
	}

	public async void stop_backend()
	{
		_project_stack.set_visible_child(_project_stack_stopping_backend_label);
		_editor_stack.set_visible_child(_editor_stack_stopping_backend_label);
		_inspector_stack.set_visible_child(_inspector_stack_stopping_backend_label);

		yield stop_heads();
		yield stop_data_compiler();

		_level.reset();
		_project.reset();

		this.active_window.title = CROWN_EDITOR_MAIN_WINDOW_TITLE;
	}

	private async void stop_data_compiler()
	{
		yield _compiler.stop();
	}

	private async void start_editor(uint window_xid, int width, int height)
	{
		if (window_xid == 0)
			return;

		// Spawn the level editor.
		string args[] =
		{
			ENGINE_EXE,
			"--data-dir",
			_project.data_dir(),
			"--boot-dir",
			LEVEL_EDITOR_BOOT_DIR,
			"--parent-window",
			window_xid.to_string(),
			"--wait-console",
			"--pumped",
			"--window-rect", "0", "0", width.to_string(), height.to_string()
		};

		try {
			_editor._process_id = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			_editor._revision = _data_compiler._revision;
		} catch (Error e) {
			loge(e.message);
		}

		// Try to connect to the level editor.
		int tries = yield _editor.connect_async(EDITOR_ADDRESS
			, EDITOR_TCP_PORT
			, EDITOR_CONNECTION_TRIES
			, EDITOR_CONNECTION_INTERVAL
			);
		if (tries == EDITOR_CONNECTION_TRIES) {
			loge("Cannot connect to level_editor");
			return;
		}
	}

	private async void start_resource_preview(uint window_xid, int width, int height)
	{
		if (window_xid == 0)
			return;

		// Spawn unit_preview.
		string args[] =
		{
			ENGINE_EXE,
			"--data-dir",
			_project.data_dir(),
			"--boot-dir",
			UNIT_PREVIEW_BOOT_DIR,
			"--parent-window",
			window_xid.to_string(),
			"--console-port",
			UNIT_PREVIEW_TCP_PORT.to_string(),
			"--wait-console",
			"--pumped",
			"--window-rect", "0", "0", width.to_string(), height.to_string()
		};

		try {
			_resource_preview._process_id = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			_resource_preview._revision = _data_compiler._revision;
		} catch (Error e) {
			loge(e.message);
		}

		// Try to connect to unit_preview.
		int tries = yield _resource_preview.connect_async(UNIT_PREVIEW_ADDRESS
			, UNIT_PREVIEW_TCP_PORT
			, EDITOR_CONNECTION_TRIES
			, EDITOR_CONNECTION_INTERVAL
			);
		if (tries == EDITOR_CONNECTION_TRIES) {
			loge("Cannot connect to unit_preview.");
			return;
		}

		// FIXME: This should be done in ResourceChooser.
		_resource_chooser._tree_view.set_cursor(new Gtk.TreePath.first(), null, false);
	}

	private async void stop_editor()
	{
		yield stop_thumbnail();
		yield stop_resource_preview();
		yield _editor.stop();
		_editor_stack.set_visible_child(_editor_stack_disconnected_label);
	}

	public async void stop_resource_preview()
	{
		yield _resource_preview.stop();
		_resource_preview_stack.set_visible_child(_resource_preview_disconnected_label);
	}

	private async void restart_editor()
	{
		yield stop_editor();

		if (_editor_view != null) {
			_editor_view_overlay.remove(_editor_view);
			_editor_stack.remove(_editor_view_overlay);
			_editor_view = null;
		}

		_editor_view = new EditorView(_editor);
		_editor_view.native_window_ready.connect(on_editor_view_realized);
		_editor_view.button_press_event.connect(on_button_press);
		_editor_view.button_release_event.connect(on_button_release);

		_editor_view_overlay.add(_editor_view);
		_editor_view_overlay.show_all();

		_editor_stack.add(_editor_view_overlay);
		_editor_stack.set_visible_child(_editor_view_overlay);

		yield restart_resource_preview();
		yield start_thumbnail();
	}

	private async void restart_resource_preview()
	{
		yield stop_resource_preview();

		if (_resource_preview_view != null) {
			_resource_preview_stack.remove(_resource_preview_view);
			_resource_preview_view = null;
		}

		_resource_preview_view = new EditorView(_resource_preview, false);
		_resource_preview_view.set_size_request(300, 300);
		_resource_preview_view.native_window_ready.connect(on_resource_preview_view_realized);
		_resource_preview_view.show_all();

		_resource_preview_stack.add(_resource_preview_view);
		_resource_preview_stack.set_visible_child(_resource_preview_view);
	}

	private int dump_test_level()
	{
		try {
			// Save temporary package to reference test level.
			ArrayList<Value?> level = new ArrayList<Value?>();
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

	private async void start_game(StartGame sg)
	{
		if (sg == StartGame.TEST && dump_test_level() != 0)
			return;

		bool success = yield _data_compiler.compile(_project.data_dir(), _project.platform());
		_project.delete_garbage();

		if (!success) {
			_toolbar_run.icon_name = "game-run";
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
			_game._revision = _data_compiler._revision;
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

	private async void start_thumbnail()
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
			_thumbnail._revision = _data_compiler._revision;
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

	private async void stop_game()
	{
		yield _game.stop();
	}

	private async void stop_thumbnail()
	{
		yield _thumbnail.stop();
	}

	private async void on_editor_view_realized(uint window_id, int width, int height)
	{
		start_editor.begin(window_id, width, height);
	}

	private async void on_resource_preview_view_realized(uint window_id, int width, int height)
	{
		start_resource_preview.begin(window_id, width, height);
	}

	private void on_tool(GLib.SimpleAction action, GLib.Variant? param)
	{
		ToolType type = (ToolType)param.get_int32();

		if (type == ToolType.PLACE) {
			// Store previous tool for it to be restored later.
			if (_tool_type != ToolType.PLACE)
				_tool_type_prev = _tool_type;
		}

		_tool_type = type;

		_editor_view.grab_focus();
		send_state();
		_editor.send(DeviceApi.frame());
		action.set_state(param);
	}

	private void on_cancel_place(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_tool_type != ToolType.PLACE)
			return;

		activate_action("tool", new GLib.Variant.int32(_tool_type_prev));
	}

	private void on_snap(GLib.SimpleAction action, GLib.Variant? param)
	{
		_snap_mode = (SnapMode)param.get_int32();

		send_state();
		_editor.send(DeviceApi.frame());
		action.set_state(param);
	}

	private void on_reference_system(GLib.SimpleAction action, GLib.Variant? param)
	{
		_reference_system = (ReferenceSystem)param.get_int32();

		send_state();
		_editor.send(DeviceApi.frame());
		action.set_state(param);
	}

	private void on_grid_size(GLib.SimpleAction action, GLib.Variant? param)
	{
		int32 new_size = param.get_int32();

		if (new_size != 0) {
			_grid_size = (double)new_size / 10.0;
			send_state();
			_editor.send(DeviceApi.frame());
			action.set_state(param);
			return;
		}

		// Custom grid size.
		Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Grid size"
			, this.active_window
			, DialogFlags.MODAL
			, "Cancel"
			, ResponseType.CANCEL
			, "Ok"
			, ResponseType.OK
			, null
			);

		EntryDouble sb = new EntryDouble(_grid_size, 0.1, 1000);
		sb.activate.connect(() => { dg.response(ResponseType.OK); });
		dg.get_content_area().add(sb);
		dg.skip_taskbar_hint = true;
		dg.show_all();

		if (dg.run() == ResponseType.OK) {
			_grid_size = sb.value;
			send_state();
			_editor.send(DeviceApi.frame());
			action.set_state(param);
		}

		dg.destroy();
	}

	private void new_level()
	{
		_level.load_from_path(LEVEL_EMPTY);
		_level.send_level();
		_editor.send(DeviceApi.frame());
	}

	private void update_active_window_title()
	{
		string title = "";

		if (_level._name != null) {
			if (_database.changed())
				title += " • ";

			title += (_level._name == LEVEL_EMPTY) ? "untitled" : _level._name;
			title += " - ";
		}

		title += CROWN_EDITOR_MAIN_WINDOW_TITLE;

		if (this.active_window.title != title)
			this.active_window.title = title;
	}

	private void load_level(string name)
	{
		if (name == _level._name)
			return;

		string resource_name = name != "" ? name : LEVEL_EMPTY;
		if (_level.load_from_path(resource_name) != 0) {
			loge("Unable to load level %s".printf(resource_name));
			return;
		}

		if (_editor.is_connected()) {
			_level.send_level();
			send_state();
			_editor.send(DeviceApi.frame());
		}

		update_active_window_title();
	}

	private bool save_as(string? filename)
	{
		string path = filename;

		if (path == null) {
			Gtk.FileChooserDialog fcd = new Gtk.FileChooserDialog("Save As..."
				, this.active_window
				, FileChooserAction.SAVE
				, "Cancel"
				, ResponseType.CANCEL
				, "Save"
				, ResponseType.ACCEPT
				);
			fcd.add_filter(_file_filter);
			fcd.set_current_folder(_project.source_dir());

			int rt = ResponseType.CANCEL;
			do {
				// Select the file
				rt = fcd.run();
				if (rt != ResponseType.ACCEPT) {
					fcd.destroy();
					return false;
				}
				path = fcd.get_filename();

				// Append file extension
				if (!path.has_suffix(".level"))
					path += ".level";

				// Check if the file is within the source directory
				if (!_project.path_is_within_source_dir(path)) {
					Gtk.MessageDialog md = new Gtk.MessageDialog(fcd
						, DialogFlags.MODAL
						, MessageType.WARNING
						, Gtk.ButtonsType.OK
						, "The file must be within the source directory."
						);
					md.set_default_response(ResponseType.OK);

					md.run();
					md.destroy();
					fcd.set_current_folder(_project.source_dir());
					continue;
				}

				// Check if the file already exists
				rt = ResponseType.YES;
				if (GLib.FileUtils.test(path, FileTest.EXISTS)) {
					Gtk.MessageDialog md = new Gtk.MessageDialog(fcd
						, DialogFlags.MODAL
						, MessageType.QUESTION
						, Gtk.ButtonsType.NONE
						, "A file named `%s` already exists.\nOverwrite?".printf(GLib.Path.get_basename(path))
						);

					Gtk.Widget btn;
					md.add_button("_No", ResponseType.NO);
					btn = md.add_button("_Yes", ResponseType.YES);
					btn.get_style_context().add_class(Gtk.STYLE_CLASS_DESTRUCTIVE_ACTION);

					md.set_default_response(ResponseType.NO);

					rt = md.run();
					md.destroy();
				}
			} while (rt != ResponseType.YES);

			fcd.destroy();
		}

		// Save level
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
			md.add_button("_Ok", ResponseType.OK);
			md.set_default_response(ResponseType.OK);

			md.run();
			md.destroy();
			return false;
		}

		_statusbar.set_temporary_message("Saved %s".printf(_level._path));
		update_active_window_title();
		return true;
	}

	private bool save()
	{
		return save_as(_level._path);
	}

	private bool save_timeout()
	{
		if (_level._path != null)
			save();

		return GLib.Source.CONTINUE;
	}

	private Hashtable encode()
	{
		Hashtable json_obj = new Hashtable();
		json_obj["level_editor_window"] = ((LevelEditorWindow)this.active_window).encode();
		return json_obj;
	}

	protected override void shutdown()
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

		// Destroy widgets.
		if (_resource_chooser != null)
			_resource_chooser.destroy();

		if (_preferences_dialog != null)
			_preferences_dialog.destroy();

		base.shutdown();
	}

	// Returns true if the level has been saved or the user decided it
	// should be discarded.
	public bool should_quit()
	{
		int rt = ResponseType.YES;

		if (_database.changed())
			rt = run_level_changed_dialog(this.active_window);

		if (!_database.changed() || rt == ResponseType.YES && save() || rt == ResponseType.NO)
			return true;

		return false;
	}

	private void on_new_level(GLib.SimpleAction action, GLib.Variant? param)
	{
		int rt = ResponseType.YES;

		if (_database.changed())
			rt = run_level_changed_dialog(this.active_window);

		if (!_database.changed() || rt == ResponseType.YES && save() || rt == ResponseType.NO) {
			new_level();
			send_state();
			_editor.send(DeviceApi.frame());
		}
	}

	private void on_open_level_from_menubar(GLib.SimpleAction action, GLib.Variant? param)
	{
		string path = "";

		Gtk.FileChooserDialog fcd = new Gtk.FileChooserDialog("Open Level..."
			, this.active_window
			, FileChooserAction.OPEN
			, "Cancel"
			, ResponseType.CANCEL
			, "Open"
			, ResponseType.ACCEPT
			);
		fcd.add_filter(_file_filter);
		fcd.set_current_folder(_project.source_dir());

		int err = 1;
		int rt = ResponseType.CANCEL;
		do {
			// Select the file
			rt = fcd.run();
			if (rt != ResponseType.ACCEPT) {
				fcd.destroy();
				return;
			}
			path = fcd.get_filename();
			err = 0;

			// Append file extension
			if (!path.has_suffix(".level"))
				path += ".level";

			if (!_project.path_is_within_source_dir(path)) {
				Gtk.MessageDialog md = new Gtk.MessageDialog(fcd
					, DialogFlags.MODAL
					, MessageType.WARNING
					, Gtk.ButtonsType.OK
					, "The file must be within the source directory."
					);
				md.set_default_response(ResponseType.OK);

				md.run();
				md.destroy();
				fcd.set_current_folder(_project.source_dir());
				err = 1;
				continue;
			}
		} while (err != 0);

		fcd.destroy();

		assert(path != "");

		// Load level
		string resource_filename = _project.resource_filename(path);
		string resource_path     = ResourceId.normalize(resource_filename);
		string resource_name     = ResourceId.name(resource_path);

		load_level(resource_name);
	}

	private void on_open_level(GLib.SimpleAction action, GLib.Variant? param)
	{
		int rt = ResponseType.YES;

		string level_name = param.get_string();
		if (level_name != "" && level_name == _level._name)
			return;

		if (_database.changed())
			rt = run_level_changed_dialog(this.active_window);

		if (!_database.changed() || rt == ResponseType.YES && save() || rt == ResponseType.NO) {
			if (level_name != "")
				load_level(level_name);
			else // Action invoked from menubar File > Open Level...
				on_open_level_from_menubar(action, param);
		}
	}

	private void on_open_project(GLib.SimpleAction action, GLib.Variant? param)
	{
		int rt = ResponseType.YES;

		if (_database.changed())
			rt = run_level_changed_dialog(this.active_window);

		if (!_database.changed() || rt == ResponseType.YES && save() || rt == ResponseType.NO) {
			string source_dir;
			source_dir = param.get_string();
			if (source_dir == "") {
				// Project opened from menubar.
				rt = run_open_project_dialog(out source_dir, this.active_window);
				if (rt != ResponseType.ACCEPT)
					return;
			}

			if (_project.source_dir() == source_dir)
				return;

			// Naively check whether the selected folder contains a Crown project.
			if (!GLib.File.new_for_path(GLib.Path.build_filename(source_dir, "boot.config")).query_exists()
				|| !GLib.File.new_for_path(GLib.Path.build_filename(source_dir, "global.physics_config")).query_exists()
				) {
				Gtk.MessageDialog md = new Gtk.MessageDialog(this.active_window
					, Gtk.DialogFlags.MODAL
					, Gtk.MessageType.INFO
					, Gtk.ButtonsType.OK
					, "The selected folder does not appear to be a valid Crown project."
					);

				md.set_default_response(ResponseType.OK);
				md.run();
				md.destroy();
				return;
			}

			this.show_panel("main_vbox", Gtk.StackTransitionType.NONE);
			_user.add_or_touch_recent_project(source_dir, source_dir);
			_console_view.reset();

			restart_backend.begin(source_dir, LEVEL_NONE, (obj, res) => {
					restart_backend.end(res);
				});
		}
	}

	private void on_new_project(GLib.SimpleAction action, GLib.Variant? param)
	{
		int rt = ResponseType.YES;

		if (_database.changed())
			rt = run_level_changed_dialog(this.active_window);

		if (!_database.changed() || rt == ResponseType.YES && save() || rt == ResponseType.NO) {
			stop_backend.begin((obj, res) => {
					stop_backend.end(res);
					show_panel("panel_new_project");
				});
		}
	}

	private void on_add_project(GLib.SimpleAction action, GLib.Variant? param)
	{
		string source_dir;
		int rt = run_open_project_dialog(out source_dir, this.active_window);
		if (rt != ResponseType.ACCEPT)
			return;

		_user.add_or_touch_recent_project(source_dir, source_dir);
	}

	private void on_save(GLib.SimpleAction action, GLib.Variant? param)
	{
		save();
	}

	private void on_save_as(GLib.SimpleAction action, GLib.Variant? param)
	{
		save_as(null);
	}

	private void on_import_result(ImportResult result)
	{
		if (result == ImportResult.ERROR) {
			loge("Failed to import resource(s)");
			return;
		} else if (result == ImportResult.SUCCESS) {
			_data_compiler.compile.begin(_project.data_dir(), _project.platform(), (obj, res) => {
					_data_compiler.compile.end(res);
				});
		}

		// FIXME: hack to force PropertiesView to update.
		_level.selection_changed(_level._selection);
	}

	private void on_import(GLib.SimpleAction action, GLib.Variant? param)
	{
		string? destination_dir = param == null ? null : param.get_string();

		ImportResult ec = _project.import(destination_dir
			, on_import_result
			, _project_store
			, this.active_window
			);
		if (ec != ImportResult.CALLBACK)
			on_import_result(ec);
	}

	private void on_preferences(GLib.SimpleAction action, GLib.Variant? param)
	{
		_preferences_dialog.set_transient_for(this.active_window);
		_preferences_dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT);
		_preferences_dialog.show_all();
		_preferences_dialog.present();
	}

	private void on_deploy(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_deploy_dialog == null) {
			_deploy_dialog = new DeployDialog(_project, _editor);
			_deploy_dialog.set_transient_for(this.active_window);
			_deploy_dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT);
			_deploy_dialog.delete_event.connect(_deploy_dialog.hide_on_delete);
		}

		_deploy_dialog.show_all();
		_deploy_dialog.present();
	}

	private void on_texture_settings(GLib.SimpleAction action, GLib.Variant? param)
	{
		string texture_name = param.get_string();

		if (_texture_settings_dialog == null) {
			_texture_settings_dialog = new TextureSettingsDialog(_project, _project_store, _database);
			_texture_settings_dialog.set_transient_for(this.active_window);
			_texture_settings_dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT);
			_texture_settings_dialog.delete_event.connect(_texture_settings_dialog.hide_on_delete);
			_texture_settings_dialog.texture_saved.connect(() => {
						_data_compiler.compile.begin(_project.data_dir(), _project.platform());
					});
		}

		_texture_settings_dialog.set_texture(texture_name);
		_texture_settings_dialog.show_all();
		_texture_settings_dialog.present();
	}

	private void on_reveal(GLib.SimpleAction action, GLib.Variant? param)
	{
		string type = (string)param.get_child_value(0);
		string name = (string)param.get_child_value(1);

		if (!_project_notebook.is_visible())
			_project_notebook.show_all();

		_project_browser.reveal(type, name);
	}

	private int run_level_changed_dialog(Gtk.Window? parent)
	{
		Gtk.MessageDialog md = new Gtk.MessageDialog(parent
			, Gtk.DialogFlags.MODAL
			, Gtk.MessageType.WARNING
			, Gtk.ButtonsType.NONE
			, "Save changes to Level before closing?"
			);

		Gtk.Widget btn;
		btn = md.add_button("Close _without Saving", ResponseType.NO);
		btn.get_style_context().add_class(Gtk.STYLE_CLASS_DESTRUCTIVE_ACTION);
		md.add_button("_Cancel", ResponseType.CANCEL);
		md.add_button("_Save", ResponseType.YES);

		md.set_default_response(ResponseType.YES);

		int rt = md.run();
		md.destroy();
		return rt;
	}

	public int run_open_project_dialog(out string source_dir, Gtk.Window? parent)
	{
		Gtk.FileChooserDialog fcd = new Gtk.FileChooserDialog("Open Project..."
			, parent
			, FileChooserAction.SELECT_FOLDER
			, "Cancel"
			, ResponseType.CANCEL
			, "Open"
			, ResponseType.ACCEPT
			);
		int rt = fcd.run();
		source_dir = fcd.get_filename();
		fcd.destroy();
		return rt;
	}

	private void on_close_project(GLib.SimpleAction action, GLib.Variant? param)
	{
		int rt = ResponseType.YES;

		if (_database.changed())
			rt = run_level_changed_dialog(this.active_window);

		if (!_database.changed() || rt == ResponseType.YES && save() || rt == ResponseType.NO) {
			stop_backend.begin((obj, res) => {
					stop_backend.end(res);
					show_panel("panel_welcome");
				});
		}
	}

	public void stop_backend_and_quit()
	{
		stop_backend.begin((obj, res) => {
				stop_backend.end(res);
				this.quit();
			});
	}

	private void on_quit(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (should_quit())
			stop_backend_and_quit();
	}

	public static bool is_image_file(string path)
	{
		return path.has_suffix(".png")
			|| path.has_suffix(".tga")
			;
	}

	private void on_open_resource(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (param == null)
			return;

		string resource_path  = param.get_string();
		string? resource_type = ResourceId.type(resource_path);
		string? resource_name = ResourceId.name(resource_path);
		if (resource_type == null || resource_name == null)
			return;

		if (resource_type == "level") {
			activate_action("open-level", resource_name);
			return;
		} else if (resource_type == "texture") {
			activate_action("texture-settings", resource_name);
			return;
		}

		GLib.AppInfo? app = null;

		if (resource_type == "lua") {
			app = _preferences_dialog._lua_external_tool_button.get_app_info();
		} else if (is_image_file(resource_path)) {
			app = _preferences_dialog._image_external_tool_button.get_app_info();
		}

		try {
			GLib.File file = GLib.File.new_for_path(_project.absolute_path(resource_path));
			if (app == null)
				app = file.query_default_handler();

			GLib.List<GLib.File> files = new GLib.List<GLib.File>();
			files.append(file);
			app.launch(files, null);
		} catch (Error e) {
			loge(e.message);
		}
	}

	private void copy_string(string str)
	{
		var clip = Gtk.Clipboard.get_default(Gdk.Display.get_default());
		clip.set_text(str, str.length);
#if !CROWN_PLATFORM_WINDOWS
		clip.store();
#endif
	}

	private void on_copy_path(GLib.SimpleAction action, GLib.Variant? param)
	{
		string path  = param.get_string();
		copy_string(_project.absolute_path(path));
	}

	private void on_copy_name(GLib.SimpleAction action, GLib.Variant? param)
	{
		copy_string(param.get_string());
	}

	private void on_show_grid(GLib.SimpleAction action, GLib.Variant? param)
	{
		_show_grid = !action.get_state().get_boolean();
		send_state();
		_editor.send(DeviceApi.frame());
		action.set_state(new GLib.Variant.boolean(_show_grid));
	}

	private void on_rotation_snap_size(GLib.SimpleAction action, GLib.Variant? param)
	{
		int32 new_size = param.get_int32();

		if (new_size != 0) {
			_rotation_snap = (double)new_size;
			send_state();
			_editor.send(DeviceApi.frame());
			action.set_state(param);
			return;
		}

		// Custom rotation size.
		Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Rotation snap"
			, this.active_window
			, DialogFlags.MODAL
			, "Cancel"
			, ResponseType.CANCEL
			, "Ok"
			, ResponseType.OK
			, null
			);

		EntryDouble sb = new EntryDouble(_rotation_snap, 1.0, 180.0);
		sb.activate.connect(() => { dg.response(ResponseType.OK); });
		dg.get_content_area().add(sb);
		dg.skip_taskbar_hint = true;
		dg.show_all();

		if (dg.run() == ResponseType.OK) {
			_rotation_snap = sb.value;
			send_state();
			_editor.send(DeviceApi.frame());
			action.set_state(param);
		}

		dg.destroy();
	}

	private void on_spawn_primitive(GLib.SimpleAction action, GLib.Variant? param)
	{
		GLib.Variant[] paramz;

		if (action.name == "primitive-cube")
			paramz = { "unit", "core/units/primitives/cube" };
		else if (action.name == "primitive-sphere")
			paramz = { "unit", "core/units/primitives/sphere" };
		else if (action.name == "primitive-cone")
			paramz = { "unit", "core/units/primitives/cone" };
		else if (action.name == "primitive-cylinder")
			paramz = { "unit", "core/units/primitives/cylinder" };
		else if (action.name == "primitive-plane")
			paramz = { "unit", "core/units/primitives/plane" };
		else if (action.name == "camera")
			paramz = { "unit", "core/units/camera" };
		else if (action.name == "light")
			paramz = { "unit", "core/units/light" };
		else if (action.name == "sound-source")
			paramz = { "sound", "" };
		else
			paramz = { "unit", "core/units/primitives/cube" };

		activate_action("set-placeable", new GLib.Variant.tuple(paramz));
	}

	private void on_spawn_unit(GLib.SimpleAction action, GLib.Variant? param)
	{
		_level.spawn_empty_unit();
		_editor.send(DeviceApi.frame());
	}

	private void on_camera_view(GLib.SimpleAction action, GLib.Variant? param)
	{
		_level._camera_view_type = (CameraViewType)param.get_int32();

		_editor.send_script(LevelEditorApi.set_camera_view_type(_level._camera_view_type));
		_editor.send(DeviceApi.frame());
		action.set_state(param);
	}

	private void on_camera_frame_selected(GLib.SimpleAction action, GLib.Variant? param)
	{
		Guid?[] selected_objects = _level._selection.to_array();
		_editor.send_script(LevelEditorApi.frame_objects(selected_objects));
		_editor.send(DeviceApi.frame());
	}

	private void on_camera_frame_all(GLib.SimpleAction action, GLib.Variant? param)
	{
		Gee.ArrayList<Guid?> all_objects = new Gee.ArrayList<Guid?>();
		_level.objects(ref all_objects);
		_editor.send_script(LevelEditorApi.frame_objects(all_objects.to_array()));
		_editor.send(DeviceApi.frame());
	}

	private void on_resource_chooser(GLib.SimpleAction action, GLib.Variant? param)
	{
		_resource_popover.show_all();
	}

	private void on_project_browser(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_project_notebook.is_visible()) {
			_project_notebook.hide();
		} else {
			_project_notebook.show_all();
		}
	}

	private void on_console(GLib.SimpleAction action, GLib.Variant? param)
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

	private void on_statusbar(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_statusbar.is_visible()) {
			_statusbar.hide();
		} else {
			_statusbar.show_all();
		}
	}

	private void on_inspector(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_inspector_stack.is_visible()) {
			_inspector_stack.hide();
		} else {
			_inspector_stack.show_all();
		}
	}

	private void on_restart_editor_view(GLib.SimpleAction action, GLib.Variant? param)
	{
		restart_editor.begin((obj, res) => {
				restart_editor.end(res);
			});
	}

	private void on_build_data(GLib.SimpleAction action, GLib.Variant? param)
	{
		_data_compiler.compile.begin(_project.data_dir(), _project.platform(), (obj, res) => {
				_data_compiler.compile.end(res);
			});
	}

	private void on_refresh_lua(GLib.SimpleAction action, GLib.Variant? param)
	{
		_data_compiler.compile.begin(_project.data_dir(), _project.platform(), (obj, res) => {
				if (_data_compiler.compile.end(res)) {
					refresh_all_clients.begin();
				}
			});
	}

	private async bool refresh_all_clients()
	{
		RuntimeInstance[] runtimes = new RuntimeInstance[] { _editor, _resource_preview, _game, _thumbnail };
		bool success = true;

		foreach (var ri in runtimes)
			if (!yield ri.refresh(_data_compiler))
				success = false;

		return success;
	}

	private void on_snap_to_grid(GLib.SimpleAction action, GLib.Variant? param)
	{
		_snap_to_grid = !action.get_state().get_boolean();
		send_state();
		_editor.send(DeviceApi.frame());
		action.set_state(new GLib.Variant.boolean(_snap_to_grid));
	}

	private void on_debug_render_world(GLib.SimpleAction action, GLib.Variant? param)
	{
		_debug_render_world = !action.get_state().get_boolean();
		send_state();
		_editor.send(DeviceApi.frame());
		action.set_state(new GLib.Variant.boolean(_debug_render_world));
	}

	private void on_debug_physics_world(GLib.SimpleAction action, GLib.Variant? param)
	{
		_debug_physics_world = !action.get_state().get_boolean();
		send_state();
		_editor.send(DeviceApi.frame());
		action.set_state(new GLib.Variant.boolean(_debug_physics_world));
	}

	private void on_run_game(GLib.SimpleAction action, GLib.Variant? param)
	{
		// Trigger a 'focus_out_event' on the currently focused
		// widget within the active_window, if any. This will
		// force 'focus' to commit its pending changes to the
		// database so we do not miss any modifications before
		// launching the game.
		Gtk.Widget? focus = this.active_window.get_focus();
		_editor_view.grab_focus();
		if (focus != null)
			focus.grab_focus();

		string icon_name_displayed = _toolbar_run.icon_name;

		stop_game.begin((obj, res) => {
				stop_game.end(res);

				if (icon_name_displayed == "game-run") {
					// Always change icon state regardless of failures
					_toolbar_run.icon_name = "game-stop";

					start_game.begin(action.name == "test-level" ? StartGame.TEST : StartGame.NORMAL);
				}
			});
	}

	private void on_undo(GLib.SimpleAction action, GLib.Variant? param)
	{
		int id = _database.undo();
		if (id != -1) {
			_statusbar.set_temporary_message("Undo: " + ActionNames[id]);
			update_active_window_title();
		}
	}

	private void on_redo(GLib.SimpleAction action, GLib.Variant? param)
	{
		int id = _database.redo();
		if (id != -1) {
			_statusbar.set_temporary_message("Redo: " + ActionNames[id]);
			update_active_window_title();
		}
	}

	private void on_duplicate(GLib.SimpleAction action, GLib.Variant? param)
	{
		_level.duplicate_selected_objects();
		_editor.send(DeviceApi.frame());
	}

	private void on_delete(GLib.SimpleAction action, GLib.Variant? param)
	{
		_level.destroy_selected_objects();
		_editor.send(DeviceApi.frame());
	}

	private void on_manual(GLib.SimpleAction action, GLib.Variant? param)
	{
		try {
			AppInfo.launch_default_for_uri(CROWN_LATEST_DOCS_URL, null);
		} catch (Error e) {
			loge(e.message);
		}
	}

	private void on_report_issue(GLib.SimpleAction action, GLib.Variant? param)
	{
		try {
			AppInfo.launch_default_for_uri("https://github.com/crownengine/crown/issues", null);
		} catch (Error e) {
			loge(e.message);
		}
	}

	private void on_browse_logs(GLib.SimpleAction action, GLib.Variant? param)
	{
		open_directory(_logs_dir.get_path());
	}

	private void on_changelog(GLib.SimpleAction action, GLib.Variant? param)
	{
		try {
			AppInfo.launch_default_for_uri(CROWN_LATEST_CHANGELOG_URL, null);
		} catch (Error e) {
			loge(e.message);
		}
	}

	private void on_donate(GLib.SimpleAction action, GLib.Variant? param)
	{
		try {
			AppInfo.launch_default_for_uri(CROWN_FUND_URL, null);
		} catch (Error e) {
			loge(e.message);
		}
	}

	private void on_about(GLib.SimpleAction action, GLib.Variant? param)
	{
		Gtk.AboutDialog dlg = new Gtk.AboutDialog();
		dlg.set_destroy_with_parent(true);
		dlg.set_transient_for(this.active_window);
		dlg.set_modal(true);
		dlg.set_logo_icon_name(CROWN_EDITOR_ICON_NAME);

		dlg.program_name = CROWN_EDITOR_NAME;
		dlg.version = CROWN_VERSION;
		dlg.website = CROWN_WWW_URL;
		dlg.copyright = "Copyright (c) 2012-2025 Daniele Bartolini et al.";
		dlg.license_type = GPL_3_0;
		dlg.authors =
		{
			"Daniele Bartolini",
			"Simone Boscaratto",
			"Michele Rossi",
			"Raphael de Vasconcelos Nascimento"
		};
		dlg.artists =
		{
			"Michela Iacchelli - Pepper logo",
			"Giulia Gazzoli - Crown logo"
		};

		dlg.run();
		dlg.destroy();
	}

	private void on_delete_file(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (param == null)
			return;

		string resource_path  = param.get_string();
		string? resource_type = ResourceId.type(resource_path);
		string? resource_name = ResourceId.name(resource_path);
		bool do_delete = true;

		if (resource_type != null && resource_name != null) {
			if (resource_name == _level._name) {
				int rt = ResponseType.YES;

				if (_database.changed())
					rt = run_level_changed_dialog(this.active_window);

				if (!_database.changed() || rt == ResponseType.YES && save() || rt == ResponseType.NO) {
					new_level();
					send_state();
					_editor.send(DeviceApi.frame());
				} else {
					do_delete = false;
				}
			}
		}

		if (do_delete) {
			string path = _project.absolute_path(resource_path);
			try {
				GLib.File.new_for_path(path).delete();
			} catch (Error e) {
				loge(e.message);
			}
		}
	}

	private void on_delete_directory(GLib.SimpleAction action, GLib.Variant? param)
	{
		string dir_name = param.get_string();

		var path = _project.absolute_path(dir_name);
		try {
			_project.delete_tree(GLib.File.new_for_path(path));
		} catch (Error e) {
			loge(e.message);
		}
	}

	private void on_create_directory(GLib.SimpleAction action, GLib.Variant? param)
	{
		string parent_dir_name = (string)param.get_child_value(0);
		string dir_name = (string)param.get_child_value(1);

		var path = _project.absolute_path(GLib.Path.build_filename(parent_dir_name, dir_name));
		try {
			GLib.File.new_for_path(path).make_directory();
		} catch (Error e) {
			loge(e.message);
		}
	}

	private void on_create_script(GLib.SimpleAction action, GLib.Variant? param)
	{
		string dir_name = (string)param.get_child_value(0);
		string script_name = (string)param.get_child_value(1);
		bool empty = (bool)param.get_child_value(2);

		int ec = _project.create_script(dir_name, script_name, empty);
		if (ec < 0) {
			loge("Failed to create script %s".printf(script_name));
			return;
		}

		_data_compiler.compile.begin(_project.data_dir(), _project.platform(), (obj, res) => {
				_data_compiler.compile.end(res);
			});
	}

	private void on_create_unit(GLib.SimpleAction action, GLib.Variant? param)
	{
		string dir_name = (string)param.get_child_value(0);
		string unit_name = (string)param.get_child_value(1);

		int ec = _project.create_unit(dir_name, unit_name);
		if (ec < 0) {
			loge("Failed to create unit %s".printf(unit_name));
			return;
		}

		_data_compiler.compile.begin(_project.data_dir(), _project.platform(), (obj, res) => {
				_data_compiler.compile.end(res);
			});
	}

	private void on_create_state_machine(GLib.SimpleAction action, GLib.Variant? param)
	{
		string dir_name = (string)param.get_child_value(0);
		string state_machine_name = (string)param.get_child_value(1);
		string skeleton_name = (string)param.get_child_value(2);

		int ec = _project.create_state_machine(dir_name, state_machine_name, skeleton_name != "" ? skeleton_name : null);
		if (ec < 0) {
			loge("Failed to create state machine %s".printf(state_machine_name));
			return;
		}

		_data_compiler.compile.begin(_project.data_dir(), _project.platform(), (obj, res) => {
				_data_compiler.compile.end(res);
			});
	}

	private void on_create_material(GLib.SimpleAction action, GLib.Variant? param)
	{
		string dir_name = (string)param.get_child_value(0);
		string material_name = (string)param.get_child_value(1);

		int ec = _project.create_material(dir_name, material_name);
		if (ec < 0) {
			loge("Failed to create material %s".printf(material_name));
			return;
		}

		_data_compiler.compile.begin(_project.data_dir(), _project.platform(), (obj, res) => {
				_data_compiler.compile.end(res);
			});
	}

	private void on_open_containing(GLib.SimpleAction action, GLib.Variant? param)
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

	private int deploy_create_package_folder(out string config_path, out string package_path, string output_path, string app_identifier, TargetPlatform platform, TargetArch arch, TargetConfig config)
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
		package_path = Path.build_path(Path.DIR_SEPARATOR_S, config_path, app_identifier);

		try {
			int rt = ResponseType.CANCEL;
			if (GLib.File.new_for_path(package_path).query_exists()) {
				Gtk.MessageDialog md = new Gtk.MessageDialog(_deploy_dialog
					, DialogFlags.MODAL
					, MessageType.QUESTION
					, Gtk.ButtonsType.NONE
					, "A file named `%s` already exists.\nOverwrite?".printf(package_path)
					);

				Gtk.Widget btn;
				md.add_button("_No", ResponseType.NO);
				btn = md.add_button("_Yes", ResponseType.YES);
				btn.get_style_context().add_class(Gtk.STYLE_CLASS_DESTRUCTIVE_ACTION);

				md.set_default_response(ResponseType.NO);

				rt = md.run();
				md.destroy();

				if (rt == ResponseType.YES)
					delete_tree(GLib.File.new_for_path(package_path));
				else
					return -1;
			}

			GLib.File.new_for_path(package_path).make_directory_with_parents();
		} catch (Error e) {
			loge(e.message);
			return -1;
		}

		return 0;
	}

	private void on_create_package_android(GLib.SimpleAction action, GLib.Variant? param)
	{
		string config_name[] =
		{
			"release",
			"development",
#if CROWN_DEBUG
			"debug"
#endif
		};

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
		var activity_name = "MainActivity";

		AndroidDeployer android = new AndroidDeployer();
		android.check_config();

		string config_path;
		string package_path;
		int err = deploy_create_package_folder(out config_path
			, out package_path
			, output_path
			, apk_name
			, TargetPlatform.ANDROID
			, arch
			, (TargetConfig)config
			);
		if (err != 0)
			return;

		logi("Creating Android package (%s)...".printf(arch == TargetArch.ARM ? "ARMv7-A" : "ARMv8-A"));

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

			string tmp_data_dir = GLib.DirUtils.make_tmp("XXXXXX");

			// Populate Android assets folder with data.
			args = new string[]
			{
				ENGINE_EXE,
				"--source-dir",
				_project.source_dir(),
				"--map-source-dir",
				"core",
				_project.toolchain_dir(),
				"--data-dir",
				tmp_data_dir,
				"--bundle-dir",
				assets_path,
				"--compile",
				"--bundle",
				"--platform",
				dc_platform
			};

			uint32 pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			int exit_status = _subprocess_launcher.wait(pid);
			delete_tree(GLib.File.new_for_path(tmp_data_dir));

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
		android_activity += "\n    private void hideSystemUI() {";
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
		android_activity += "\n    private void showSystemUI() {";
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
						libcrown_path_relative,
						libcpp_path_relative
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

				logi("Done: #FILE(%s)".printf(package_path));
				return 0;
			});
	}

	private void on_create_package_html5(GLib.SimpleAction action, GLib.Variant? param)
	{
		string config_name[] =
		{
			"release",
			"development",
#if CROWN_DEBUG
			"debug"
#endif
		};

		var output_path = (string)param.get_child_value(0);
		var config = (int)param.get_child_value(1);
		var app_title = (string)param.get_child_value(2);

		var exe_name = app_title.replace(" ", "_").down();

		HTML5Deployer html5 = new HTML5Deployer();
		html5.check_config();

		string config_path;
		string package_path;
		int err = deploy_create_package_folder(out config_path
			, out package_path
			, output_path
			, exe_name
			, TargetPlatform.HTML5
			, TargetArch.WASM
			, (TargetConfig)config
			);
		if (err != 0)
			return;

		logi("Creating HTML5 package...");

		// Create data bundle.
		try {
			string[] args;
			string tmp_data_dir = GLib.DirUtils.make_tmp("XXXXXX");
			string tmp_bundle_dir = GLib.DirUtils.make_tmp("XXXXXX");

			args = new string[]
			{
				ENGINE_EXE,
				"--source-dir",
				_project.source_dir(),
				"--map-source-dir",
				"core",
				_project.toolchain_dir(),
				"--data-dir",
				tmp_data_dir,
				"--bundle-dir",
				tmp_bundle_dir,
				"--compile",
				"--bundle",
				"--platform",
				"html5"
			};

			var pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			var exit_status = _subprocess_launcher.wait(pid);
			delete_tree(GLib.File.new_for_path(tmp_data_dir));

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

	private void on_create_package_linux(GLib.SimpleAction action, GLib.Variant? param)
	{
		string config_name[] =
		{
			"release",
			"development",
#if CROWN_DEBUG
			"debug"
#endif
		};

		var output_path = (string)param.get_child_value(0);
		var config = (int)param.get_child_value(1);
		var app_title = (string)param.get_child_value(2);

		var exe_name = app_title.replace(" ", "_").down();

		string config_path;
		string package_path;
		int err = deploy_create_package_folder(out config_path
			, out package_path
			, output_path
			, exe_name
			, TargetPlatform.LINUX
			, TargetArch.X64
			, (TargetConfig)config
			);
		if (err != 0)
			return;

		logi("Creating Linux package...");

		// Create data bundle.
		try {
			string tmp_data_dir = GLib.DirUtils.make_tmp("XXXXXX");

			string args[] =
			{
				ENGINE_EXE,
				"--source-dir",
				_project.source_dir(),
				"--map-source-dir",
				"core",
				_project.toolchain_dir(),
				"--data-dir",
				tmp_data_dir,
				"--bundle-dir",
				package_path,
				"--compile",
				"--bundle",
				"--platform",
				"linux"
			};

			uint32 compiler = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			int exit_status = _subprocess_launcher.wait(compiler);
			delete_tree(GLib.File.new_for_path(tmp_data_dir));

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

	private void on_create_package_windows(GLib.SimpleAction action, GLib.Variant? param)
	{
		string config_name[] =
		{
			"release",
			"development",
#if CROWN_DEBUG
			"debug"
#endif
		};

		var output_path = (string)param.get_child_value(0);
		var config = (int)param.get_child_value(1);
		var app_title = (string)param.get_child_value(2);

		var exe_name = app_title.replace(" ", "_").down();

		string config_path;
		string package_path;
		int err = deploy_create_package_folder(out config_path
			, out package_path
			, output_path
			, exe_name
			, TargetPlatform.WINDOWS
			, TargetArch.X64
			, (TargetConfig)config
			);
		if (err != 0)
			return;

		logi("Creating Windows package");

		// Create data bundle.
		try {
			string tmp_data_dir = GLib.DirUtils.make_tmp("XXXXXX");

			string args[] =
			{
				ENGINE_EXE,
				"--source-dir",
				_project.source_dir(),
				"--map-source-dir",
				"core",
				_project.toolchain_dir(),
				"--data-dir",
				tmp_data_dir,
				"--bundle-dir",
				package_path,
				"--compile",
				"--bundle",
				"--platform",
				"windows"
			};

			uint32 compiler = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			int exit_status = _subprocess_launcher.wait(compiler);
			delete_tree(GLib.File.new_for_path(tmp_data_dir));

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

			string lua_name = "lua51.dll";
			GLib.File lua_dll_src = File.new_for_path(lua_name);
			GLib.File lua_dll_dst = File.new_for_path(Path.build_filename(package_path, lua_name));
			lua_dll_src.copy(lua_dll_dst, FileCopyFlags.OVERWRITE);
		} catch (Error e) {
			loge("%s".printf(e.message));
			loge("Failed to deploy '%s'".printf(app_title));
			return;
		}

		logi("Done: #FILE(%s)".printf(package_path));
	}

	private void on_unit_add_component(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (param == null)
			return;

		string component_type = param.get_string();
		Guid unit_id = _level._selection.last();
		Unit unit = Unit(_database, unit_id);
		ArrayList<Guid?> components_added = new ArrayList<Guid?>();
		components_added.add(unit_id);
		unit.add_component_type_dependencies(ref components_added, component_type);

		_database.add_restore_point((int)ActionType.UNIT_ADD_COMPONENT, components_added.to_array());
	}

	private void on_unit_remove_component(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (param == null)
			return;

		string component_type = param.get_string();
		Guid unit_id = _level._selection.last();
		Unit unit = Unit(_database, unit_id);

		Guid component_id;
		if (!unit.has_component(out component_id, component_type))
			return;

		Gee.ArrayList<unowned string> dependents = new Gee.ArrayList<unowned string>();
		// Do not remove if any other component needs us.
		foreach (var entry in Unit._component_registry.entries) {
			Guid dummy;
			if (!unit.has_component(out dummy, entry.key))
				continue;

			string[] component_type_dependencies = ((string)entry.value).split(", ");
			if (component_type in component_type_dependencies)
				dependents.add(entry.key);
		}

		if (dependents.size > 0) {
			StringBuilder sb = new StringBuilder();
			sb.append("Cannot remove %s due to the following dependencies:\n\n".printf(component_type));
			foreach (var item in dependents)
				sb.append("• %s\n".printf(item));

			Gtk.MessageDialog md = new Gtk.MessageDialog(this.active_window
				, DialogFlags.MODAL
				, MessageType.WARNING
				, Gtk.ButtonsType.OK
				, sb.str
				);
			md.set_default_response(ResponseType.OK);

			md.run();
			md.destroy();
			return;
		} else {
			unit.remove_component_type(component_type);
		}
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

	private void set_conflicting_accels(bool on)
	{
		if (on) {
			this.set_accels_for_action("app.tool(0)", _tool_place_accels);
			this.set_accels_for_action("app.tool(1)", _tool_move_accels);
			this.set_accels_for_action("app.tool(2)", _tool_rotate_accels);
			this.set_accels_for_action("app.tool(3)", _tool_scale_accels);
			this.set_accels_for_action("app.delete", _delete_accels);
			this.set_accels_for_action("app.camera-view(0)", _camera_view_perspective_accels);
			this.set_accels_for_action("app.camera-view(1)", _camera_view_front_accels);
			this.set_accels_for_action("app.camera-view(2)", _camera_view_back_accels);
			this.set_accels_for_action("app.camera-view(3)", _camera_view_right_accels);
			this.set_accels_for_action("app.camera-view(4)", _camera_view_left_accels);
			this.set_accels_for_action("app.camera-view(5)", _camera_view_top_accels);
			this.set_accels_for_action("app.camera-view(6)", _camera_view_bottom_accels);
			this.set_accels_for_action("app.camera-frame-selected", _camera_frame_selected_accels);
			this.set_accels_for_action("app.camera-frame-all", _camera_frame_all_accels);
		} else {
			this.set_accels_for_action("app.tool(0)", {});
			this.set_accels_for_action("app.tool(1)", {});
			this.set_accels_for_action("app.tool(2)", {});
			this.set_accels_for_action("app.tool(3)", {});
			this.set_accels_for_action("app.delete", {});
			this.set_accels_for_action("app.camera-view(0)", {});
			this.set_accels_for_action("app.camera-view(1)", {});
			this.set_accels_for_action("app.camera-view(2)", {});
			this.set_accels_for_action("app.camera-view(3)", {});
			this.set_accels_for_action("app.camera-view(4)", {});
			this.set_accels_for_action("app.camera-view(5)", {});
			this.set_accels_for_action("app.camera-view(6)", {});
			this.set_accels_for_action("app.camera-frame-selected", {});
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

		if (name == "main_vbox") {
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
		} else if (name == "panel_welcome"
			|| name == "panel_new_project"
			|| name == "panel_projects_list"
			) {
			menu_set_enabled(false, action_entries_file, {"new-project", "add-project", "open-project", "quit"});
			menu_set_enabled(false, action_entries_edit);
			menu_set_enabled(false, action_entries_create);
			menu_set_enabled(false, action_entries_camera);
			menu_set_enabled(false, action_entries_view);
			menu_set_enabled(false, action_entries_debug);
			menu_set_enabled(true, action_entries_help);
		}
	}

	private void on_set_placeable(GLib.SimpleAction action, GLib.Variant? param)
	{
		_placeable_type = (string)param.get_child_value(0);
		_placeable_name = (string)param.get_child_value(1);

		_editor.send_script(LevelEditorApi.set_placeable(_placeable_type, _placeable_name));
		activate_action("tool", new GLib.Variant.int32(ToolType.PLACE));
	}

	private void on_project_reset()
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

	private void on_project_loaded()
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

private void device_frame_delayed(uint delay_ms, RuntimeInstance runtime)
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
		if (args[ii] == "--child") {
			break;
		}
	}

	if (ii == args.length) {
		_log_prefix = "launcher";
	} else {
		_log_prefix = "editor";

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

	_settings_file = GLib.File.new_for_path(GLib.Path.build_filename(_config_dir.get_path(), "settings.sjson"));
	_window_state_file = GLib.File.new_for_path(GLib.Path.build_filename(_data_dir.get_path(), "window.sjson"));
	_user_file = GLib.File.new_for_path(GLib.Path.build_filename(_data_dir.get_path(), "user.sjson"));
	_console_history_file = GLib.File.new_for_path(GLib.Path.build_filename(_data_dir.get_path(), "console_history.txt"));

	// Connect to SubprocessLauncher service.
	SubprocessLauncher subprocess_launcher;
	try {
		subprocess_launcher = GLib.Bus.get_proxy_sync(GLib.BusType.SESSION
			, "org.crownengine.SubprocessLauncher"
			, "/org/crownengine/subprocess_launcher"
			);
	} catch (IOError e) {
		loge(e.message);
		return 1;
	}

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
	LevelEditorApplication app = new LevelEditorApplication(subprocess_launcher);
	return app.run(args);
}

} /* namespace Crown */
