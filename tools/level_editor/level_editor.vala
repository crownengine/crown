/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gdk; // Pixbuf
using Gee;
using Gtk;

namespace Crown
{
const int WINDOW_DEFAULT_WIDTH = 1280;
const int WINDOW_DEFAULT_HEIGHT = 720;
const string LEVEL_EDITOR_WINDOW_TITLE = "Crown Editor";
const string CROWN_ICON_NAME = "crown";

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

		this.title = LEVEL_EDITOR_WINDOW_TITLE;
		this.key_press_event.connect(this.on_key_press);
		this.key_release_event.connect(this.on_key_release);
		this.window_state_event.connect(this.on_window_state_event);
		this.delete_event.connect(this.on_delete_event);
		this.focus_out_event.connect(this.on_focus_out);

		_fullscreen = false;
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
		return Gdk.EVENT_STOP;
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
		{ "open-project",  on_open_project,  "s",  null },
		{ "save",          on_save,          null, null },
		{ "save-as",       on_save_as,       null, null },
		{ "import",        on_import,        "s",  null },
		{ "preferences",   on_preferences,   null, null },
		{ "deploy",        on_deploy,        null, null },
		{ "close",         on_close,         null, null },
		{ "quit",          on_quit,          null, null },
		{ "open-resource", on_open_resource, "s",  null }
	};

	private const GLib.ActionEntry[] action_entries_edit =
	{
		{ "menu-edit",          null,                  null, null   },
		{ "undo",               on_undo,               null, null   },
		{ "redo",               on_redo,               null, null   },
		{ "duplicate",          on_duplicate,          null, null   },
		{ "delete",             on_delete,             null, null   },
		{ "tool",               on_tool,               "i",  "1"    }, // See: Crown.ToolType
		{ "snap",               on_snap,               "i",  "0"    }, // See: Crown.SnapMode
		{ "reference-system",   on_reference_system,   "i",  "0"    }, // See: Crown.ReferenceSystem
		{ "snap-to-grid",       on_snap_to_grid,       null, "true" },
		{ "menu-grid",          null,                  null, null   },
		{ "grid-show",          on_show_grid,          null, "true" },
		{ "grid-size",          on_grid_size,          "i",  "10"   }, // 10*meters.
		{ "menu-rotation-snap", null,                  null, null   },
		{ "rotation-snap-size", on_rotation_snap_size, "i",  "15"   }
	};

	private const GLib.ActionEntry[] action_entries_create =
	{
		{ "menu-create",        null,                null, null },
		{ "menu-primitives",    null,                null, null },
		{ "primitive-cube",     on_create_primitive, null, null },
		{ "primitive-sphere",   on_create_primitive, null, null },
		{ "primitive-cone",     on_create_primitive, null, null },
		{ "primitive-cylinder", on_create_primitive, null, null },
		{ "primitive-plane",    on_create_primitive, null, null },
		{ "camera",             on_create_primitive, null, null },
		{ "light",              on_create_primitive, null, null },
		{ "sound-source",       on_create_primitive, null, null },
		{ "unit-empty",         on_create_unit,      null, null }
	};

	private const GLib.ActionEntry[] action_entries_camera =
	{
		{ "menu-camera", null,           null, null },
		{ "camera-view", on_camera_view, "i",  "0"  } // See: Crown.CameraViewType
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
		{ "about",        on_about,        null, null }
	};

	private const GLib.ActionEntry[] action_entries_project =
	{
		{ "delete-file", on_delete_file, "s", null }
	};

	// Command line options
	private string? _source_dir = null;
	private string _level_resource = "";
	private User _user;
	private Hashtable _settings;

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
	private CameraViewType _camera_view_type;

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

	// Engine connections
	private uint32 _compiler_process;
	private uint32 _editor_process;
	private uint32 _resource_preview_process;
	private uint32 _game_process;
	private GLib.SourceFunc _stop_data_compiler_callback = null;
	private GLib.SourceFunc _stop_editor_callback = null;
	private GLib.SourceFunc _stop_game_callback = null;
	private GLib.SourceFunc _stop_resource_preview_callback = null;
	private ConsoleClient _compiler;
	public ConsoleClient _editor;
	private ConsoleClient _resource_preview;
	private ConsoleClient _game;

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
	private ResourceChooser _resource_chooser;
	private Gtk.Popover _resource_popover;
	private Gtk.Overlay _editor_view_overlay;

	private Gtk.Stack _project_stack;
	private Gtk.Label _project_stack_compiling_data_label;
	private Gtk.Label _project_stack_connecting_to_data_compiler_label;
	private Gtk.Label _project_stack_compiler_crashed_label;
	private Gtk.Label _project_stack_compiler_failed_compilation_label;

	private Gtk.Stack _editor_stack;
	private Gtk.Label _editor_stack_compiling_data_label;
	private Gtk.Label _editor_stack_connecting_to_data_compiler_label;
	private Gtk.Label _editor_stack_compiler_crashed_label;
	private Gtk.Label _editor_stack_compiler_failed_compilation_label;
	private Gtk.Label _editor_stack_disconnected_label;
	private Gtk.Label _editor_stack_oops_label;

	public Gtk.Stack _resource_preview_stack;
	public Gtk.Label _resource_preview_disconnected_label;
	public Gtk.Label _resource_preview_oops_label;
	public Gtk.Label _resource_preview_no_preview_label;

	private Gtk.Stack _inspector_stack;
	private Gtk.Label _inspector_stack_compiling_data_label;
	private Gtk.Label _inspector_stack_connecting_to_data_compiler_label;
	private Gtk.Label _inspector_stack_compiler_crashed_label;
	private Gtk.Label _inspector_stack_compiler_failed_compilation_label;

	private Gtk.Toolbar _toolbar;
	private Gtk.ToolButton _toolbar_run;
	private Gtk.Notebook _level_tree_view_notebook;
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
		Object(application_id: "org.crown.level_editor"
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

		string css = "/org/crown/level_editor/theme/Adwaita/gtk%s.css".printf(theme == Theme.DARK ? "-dark" : "");
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

		_settings = SJSON.load_from_path(_settings_file.get_path());

		// Set theme.
		set_theme(Theme.DARK);
		if (_settings.has_key("preferences")) {
			Hashtable preferences = (Hashtable)_settings["preferences"];
			if (preferences.has_key("theme"))
				set_theme_from_name((string)preferences["theme"]);
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

		_compiler = new ConsoleClient();
		_compiler.connected.connect(on_data_compiler_connected);
		_compiler.message_received.connect(on_message_received);

		_data_compiler = new DataCompiler(_compiler);

		_project = new Project(_data_compiler);
		_project.set_toolchain_dir(_toolchain_dir.get_path());
		_project.register_importer("Sprite", { "png" }, SpriteResource.import, 0.0);
		_project.register_importer("Mesh", { "mesh" }, MeshResource.import, 1.0);
		_project.register_importer("Sound", { "wav" }, SoundResource.import, 2.0);
		_project.register_importer("Texture", { "png", "tga", "dds", "ktx", "pvr" }, TextureResource.import, 2.0);

		uint32 undo_redo_size = DEFAULT_UNDO_REDO_MAX_SIZE;
		if (_settings.has_key("preferences")) {
			Hashtable preferences = (Hashtable)_settings["preferences"];
			if (preferences.has_key("undo_redo_max_size"))
				undo_redo_size = (uint32)(double)preferences["undo_redo_max_size"];
		}
		_undo_redo = new UndoRedo(undo_redo_size*1024*1024);
		_database = new Database(_project, _undo_redo);
		_database.key_changed.connect(() => { update_active_window_title(); });

		_editor = new ConsoleClient();
		_editor.connected.connect(on_editor_connected);
		_editor.message_received.connect(on_message_received);

		_resource_preview = new ConsoleClient();
		_resource_preview.connected.connect(on_resource_preview_connected);
		_resource_preview.message_received.connect(on_resource_preview_message_received);

		_game = new ConsoleClient();
		_game.connected.connect(on_game_connected);
		_game.message_received.connect(on_message_received);

		_level = new Level(_database, _editor, _project);

		// Editor state
		_grid_size = 1.0;
		_rotation_snap = 15.0;
		_show_grid = true;
		_snap_to_grid = true;
		_debug_render_world = false;
		_debug_physics_world = false;
		_tool_type = ToolType.MOVE;
		_tool_type_prev = _tool_type;
		_snap_mode = SnapMode.RELATIVE;
		_reference_system = ReferenceSystem.LOCAL;
		_camera_view_type = CameraViewType.PERSPECTIVE;

		// Project state
		_placeable_type = "";
		_placeable_name = "";

		// Engine connections
		_compiler_process = uint32.MAX;
		_editor_process = uint32.MAX;
		_resource_preview_process = uint32.MAX;
		_game_process = uint32.MAX;

		_project_store = new ProjectStore(_project);

		// Widgets
		_preferences_dialog = new PreferencesDialog(this);
		_preferences_dialog.delete_event.connect(_preferences_dialog.hide_on_delete);

		_combo = new Gtk.ComboBoxText();
		_combo.append("editor", "Editor");
		_combo.append("game", "Game");
		_combo.set_active_id("editor");

		_console_view = new ConsoleView(_project, _combo, _preferences_dialog);
		_project_browser = new ProjectBrowser(this, _project, _project_store);
		_level_treeview = new LevelTreeView(_database, _level);
		_level_layers_treeview = new LevelLayersTreeView(_database, _level);
		_properties_view = new PropertiesView(_level, _project_store);

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

		Gtk.Builder builder = new Gtk.Builder.from_resource("/org/crown/level_editor/ui/toolbar.ui");
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

		_editor_pane = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
		_editor_pane.pack1(_project_stack, false, false);
		_editor_pane.pack2(_editor_stack, true, false);

		_content_pane = new Gtk.Paned(Gtk.Orientation.VERTICAL);
		_content_pane.pack1(_editor_pane, true, false);
		_content_pane.pack2(_console_view, false, false);

		_inspector_pane = new Gtk.Paned(Gtk.Orientation.VERTICAL);
		_inspector_pane.pack1(_level_tree_view_notebook, true, false);
		_inspector_pane.pack2(_properties_view, false, false);
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
		_panel_new_project = new PanelNewProject(this, _user, _project);
		_panel_new_project.fill_templates_list(_templates_dir.get_path());

		_panel_welcome = new PanelWelcome();
		_panel_projects_list = new PanelProjectsList(this, _user);
		_panel_welcome.pack_start(_panel_projects_list);
		_panel_welcome.set_visible(true); // To make Gtk.Stack work...

		_main_stack = new Gtk.Stack();
		_main_stack.add_named(_panel_welcome, "panel_welcome");
		_main_stack.add_named(_panel_new_project, "panel_new_project");
		_main_stack.add_named(_main_vbox, "main_vbox");

		_preferences_dialog.decode(_settings);

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
			}
			catch (GLib.Error e) {
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
			win.set_default_size(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);
			win.add(_main_stack);

			try {
				win.icon = IconTheme.get_default().load_icon(CROWN_ICON_NAME, 256, 0);
			}
			catch (Error e) {
				loge(e.message);
			}
		}

		this.active_window.show_all();
		this.active_window.maximize();
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

	public ConsoleClient? current_selected_client()
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
		set_placeable(type, name);
		activate_action("tool", new GLib.Variant.int32(ToolType.PLACE));
	}

	private void on_data_compiler_connected(string address, int port)
	{
		logi("Connected to data_compiler@%s:%d".printf(address, port));
		_compiler.receive_async();
	}

	private void on_data_compiler_disconnected()
	{
		logi("Disconnected from data_compiler");

		if (_stop_data_compiler_callback != null)
			_stop_data_compiler_callback();
	}

	private async void on_data_compiler_disconnected_unexpected()
	{
		logw("Disconnected from data_compiler unexpectedly");
		try {
			if (_compiler_process != uint32.MAX)
				_subprocess_launcher.wait(_compiler_process);
			_compiler_process = uint32.MAX;
		} catch (GLib.Error e) {
			loge(e.message);
		}

		yield stop_heads();

		// Reset the callback
		_data_compiler.finished(false);

		_project_stack.set_visible_child(_project_stack_compiler_crashed_label);
		_editor_stack.set_visible_child(_editor_stack_compiler_crashed_label);
		_inspector_stack.set_visible_child(_inspector_stack_compiler_crashed_label);
	}

	private void on_editor_connected(string address, int port)
	{
		logi("Connected to level_editor@%s:%d".printf(address, port));

		// Start receiving data from the editor view.
		_editor.receive_async();

		// Update editor view with current editor state.
		_level.send_level();
		send_state();
		_preferences_dialog.apply();
		_editor.send(DeviceApi.frame());
	}

	private void on_editor_disconnected()
	{
		logi("Disconnected from editor");

		if (_stop_editor_callback != null)
			_stop_editor_callback();
	}

	private void on_editor_disconnected_unexpected()
	{
		logw("Disconnected from editor unexpectedly");
		try {
			if (_editor_process != uint32.MAX)
				_subprocess_launcher.wait(_editor_process);
			_editor_process = uint32.MAX;
		} catch (GLib.Error e) {
			loge(e.message);
		}

		_editor_stack.set_visible_child(_editor_stack_oops_label);
	}

	private void on_resource_preview_connected(string address, int port)
	{
		logi("Connected to preview@%s:%d".printf(address, port));

		// Start receiving data from the editor view.
		_resource_preview.receive_async();
	}

	private void on_resource_preview_disconnected()
	{
		logi("Disconnected from preview");

		if (_stop_resource_preview_callback != null)
			_stop_resource_preview_callback();
	}

	private void on_resource_preview_disconnected_unexpected()
	{
		logw("Disconnected from preview unexpectedly");
		try {
			if (_resource_preview_process != uint32.MAX)
				_subprocess_launcher.wait(_resource_preview_process);
			_resource_preview_process = uint32.MAX;
		} catch (GLib.Error e) {
			loge(e.message);
		}

		_resource_preview_stack.set_visible_child(_resource_preview_oops_label);
	}

	private void on_resource_preview_message_received(ConsoleClient client, uint8[] json)
	{
		// Ignore the message content.
		client.receive_async();
	}

	private void on_game_connected(string address, int port)
	{
		logi("Connected to game@%s:%d".printf(address, port));
		_game.receive_async();
		_combo.set_active_id("game");
	}

	private void on_game_disconnected()
	{
		logi("Disconnected from game");

		_project.delete_garbage();
		_combo.set_active_id("editor");
		_toolbar_run.icon_name = "game-run";

		if (_stop_game_callback != null)
			_stop_game_callback();
	}

	private void on_game_disconnected_externally()
	{
		on_game_disconnected();
		try {
			if (_game_process != uint32.MAX)
				_subprocess_launcher.wait(_game_process);
			_game_process = uint32.MAX;
		} catch (GLib.Error e) {
			loge(e.message);
		}
	}

	private void on_message_received(ConsoleClient client, uint8[] json)
	{
		Hashtable msg = JSON.decode(json) as Hashtable;
		string msg_type = msg["type"] as string;

		if (msg_type == "message") {
			log((string)msg["system"], (string)msg["severity"], (string)msg["message"]);
		} else if (msg_type == "add_file") {
			string path = (string)msg["path"];

			_project.add_file(path);
		} else if (msg_type == "remove_file") {
			string path = (string)msg["path"];

			_project.remove_file(path);
		} else if (msg_type == "add_tree") {
			string path = (string)msg["path"];

			_project.add_tree(path);
		} else if (msg_type == "remove_tree") {
			string path = (string)msg["path"];

			_project.remove_tree(path);
		} else if (msg_type == "compile") {
			// Guid id = Guid.parse((string)msg["id"]);

			if (msg.has_key("start")) {
				// FIXME
			} else if (msg.has_key("success")) {
				_data_compiler.finished((bool)msg["success"]);
			}
		} else if (msg_type == "unit_spawned") {
			string id             = (string)msg["id"];
			string name           = (string)msg["name"];
			ArrayList<Value?> pos = (ArrayList<Value?>)msg["position"];
			ArrayList<Value?> rot = (ArrayList<Value?>)msg["rotation"];
			ArrayList<Value?> scl = (ArrayList<Value?>)msg["scale"];

			_level.on_unit_spawned(Guid.parse(id)
				, name
				, Vector3.from_array(pos)
				, Quaternion.from_array(rot)
				, Vector3.from_array(scl)
				);
		} else if (msg_type == "sound_spawned") {
			string id             = (string)msg["id"];
			string name           = (string)msg["name"];
			ArrayList<Value?> pos = (ArrayList<Value?>)msg["position"];
			ArrayList<Value?> rot = (ArrayList<Value?>)msg["rotation"];
			ArrayList<Value?> scl = (ArrayList<Value?>)msg["scale"];
			double range          = (double)msg["range"];
			double volume         = (double)msg["volume"];
			bool loop             = (bool)msg["loop"];

			_level.on_sound_spawned(Guid.parse(id)
				, name
				, Vector3.from_array(pos)
				, Quaternion.from_array(rot)
				, Vector3.from_array(scl)
				, range
				, volume
				, loop
				);
		} else if (msg_type == "move_objects") {
			Hashtable ids           = (Hashtable)msg["ids"];
			Hashtable new_positions = (Hashtable)msg["new_positions"];
			Hashtable new_rotations = (Hashtable)msg["new_rotations"];
			Hashtable new_scales    = (Hashtable)msg["new_scales"];

			ArrayList<string> keys = new ArrayList<string>.wrap(ids.keys.to_array());
			keys.sort(Gee.Functions.get_compare_func_for(typeof(string)));

			Guid[] n_ids             = new Guid[keys.size];
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
		} else if (msg_type == "error") {
			loge((string)msg["message"]);
		} else {
			loge("Unknown message type: " + msg_type);
		}

		// Receive next message
		client.receive_async();
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
		sb.append(LevelEditorApi.set_camera_view_type(_camera_view_type));
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
			_compiler_process = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
		} catch (Error e) {
			loge(e.message);
		}

		// It is an error if the data compiler. disconnects after here.
		_compiler.disconnected.disconnect(on_data_compiler_disconnected);
		_compiler.disconnected.connect(on_data_compiler_disconnected_unexpected);

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
		_data_compiler.compile.begin(_project.data_dir(), _project.platform(), (obj, res) => {
				if (_data_compiler.compile.end(res)) {
					load_level(ln);

					// If successful, start the level editor.
					restart_editor.begin((obj, res) => {
							restart_editor.end(res);

							_project_stack.set_visible_child(_project_browser);
							_inspector_stack.set_visible_child(_inspector_pane);
						});
				} else {
					_project_stack.set_visible_child(_project_stack_compiler_failed_compilation_label);
					_editor_stack.set_visible_child(_editor_stack_compiler_failed_compilation_label);
					_inspector_stack.set_visible_child(_inspector_stack_compiler_failed_compilation_label);
				}
			});
	}

	public async void stop_heads()
	{
		yield stop_game();
		yield stop_editor();
	}

	public async void stop_backend()
	{
		yield stop_heads();
		yield stop_data_compiler();

		_level.reset();
		_project.reset();

		this.active_window.title = LEVEL_EDITOR_WINDOW_TITLE;
	}

	private async void stop_data_compiler()
	{
		if (_compiler != null) {
			// Reset "disconnected" signal.
			_compiler.disconnected.disconnect(on_data_compiler_disconnected);
			_compiler.disconnected.disconnect(on_data_compiler_disconnected_unexpected);

			// Explicit call to this function should not produce error messages.
			_compiler.disconnected.connect(on_data_compiler_disconnected);

			if (_compiler.is_connected()) {
				_stop_data_compiler_callback = stop_data_compiler.callback;
				_compiler.send(DataCompilerApi.quit());
				yield; // Wait for ConsoleClient to disconnect.
				_stop_data_compiler_callback = null;
			}
		}

		try {
			if (_compiler_process != uint32.MAX)
				_subprocess_launcher.wait(_compiler_process);
			_compiler_process = uint32.MAX;
		} catch (GLib.Error e) {
			loge(e.message);
		}
	}

	private async void start_editor(uint window_xid)
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
			"--pumped"
		};

		try {
			_editor_process = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
		} catch (Error e) {
			loge(e.message);
		}

		// It is an error if the level editor disconnects after here.
		_editor.disconnected.disconnect(on_editor_disconnected);
		_editor.disconnected.connect(on_editor_disconnected_unexpected);

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

	private async void start_resource_preview(uint window_xid)
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
			"--pumped"
		};

		try {
			_resource_preview_process = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
		} catch (Error e) {
			loge(e.message);
		}

		// It is an error if the unit_preview disconnects after here.
		_resource_preview.disconnected.disconnect(on_resource_preview_disconnected);
		_resource_preview.disconnected.connect(on_resource_preview_disconnected_unexpected);

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
		yield stop_resource_preview();

		if (_editor != null) {
			// Reset "disconnected" signal.
			_editor.disconnected.disconnect(on_editor_disconnected);
			_editor.disconnected.disconnect(on_editor_disconnected_unexpected);

			// Explicit call to this function should not produce error messages.
			_editor.disconnected.connect(on_editor_disconnected);

			if (_editor.is_connected()) {
				_stop_editor_callback = stop_editor.callback;
				_editor.send_script("Device.quit()");
				yield; // Wait for ConsoleClient to disconnect.
				_stop_editor_callback = null;
				_editor_stack.set_visible_child(_editor_stack_disconnected_label);
			}
		}

		try {
			if (_editor_process != uint32.MAX)
				_subprocess_launcher.wait(_editor_process);
			_editor_process = uint32.MAX;
		} catch (GLib.Error e) {
			loge(e.message);
		}
	}

	public async void stop_resource_preview()
	{
		if (_resource_preview != null) {
			// Reset "disconnected" signal.
			_resource_preview.disconnected.disconnect(on_resource_preview_disconnected);
			_resource_preview.disconnected.disconnect(on_resource_preview_disconnected_unexpected);

			// Explicit call to this function should not produce error messages.
			_resource_preview.disconnected.connect(on_resource_preview_disconnected);

			if (_resource_preview.is_connected()) {
				_stop_resource_preview_callback = stop_resource_preview.callback;
				_resource_preview.send_script("Device.quit()");
				yield; // Wait for ConsoleClient to disconnect.
				_stop_resource_preview_callback = null;
				_resource_preview_stack.set_visible_child(_resource_preview_disconnected_label);
			}
		}

		try {
			if (_resource_preview_process != uint32.MAX)
				_subprocess_launcher.wait(_resource_preview_process);
			_resource_preview_process = uint32.MAX;
		} catch (GLib.Error e) {
			loge(e.message);
		}
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
		_editor_view.realized.connect(on_editor_view_realized);
		_editor_view.button_press_event.connect(on_button_press);
		_editor_view.button_release_event.connect(on_button_release);

		_editor_view_overlay.add(_editor_view);
		_editor_view_overlay.show_all();

		_editor_stack.add(_editor_view_overlay);
		_editor_stack.set_visible_child(_editor_view_overlay);

		yield restart_resource_preview();
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
		_resource_preview_view.realized.connect(on_resource_preview_view_realized);
		_resource_preview_view.show_all();

		_resource_preview_stack.add(_resource_preview_view);
		_resource_preview_stack.set_visible_child(_resource_preview_view);
	}

	private async void start_game(StartGame sg)
	{
		// Save test level to file
		_database.dump(_project._level_editor_test_level.get_path(), _level._id);

		// Save temporary package to reference test level
		ArrayList<Value?> level = new ArrayList<Value?>();
		level.add("_level_editor_test");
		Hashtable package = new Hashtable();
		package["level"] = level;
		SJSON.save(package, _project._level_editor_test_package.get_path());

		bool success = yield _data_compiler.compile(_project.data_dir(), _project.platform());
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
			_game_process = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
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

	private async void stop_game()
	{
		if (_game != null) {
			// Reset "disconnected" signal.
			_game.disconnected.disconnect(on_game_disconnected);
			_game.disconnected.disconnect(on_game_disconnected_externally);

			// Explicit call to this function should not produce error messages.
			_game.disconnected.connect(on_game_disconnected);

			if (_game.is_connected()) {
				_stop_game_callback = stop_game.callback;
				_game.send_script("Device.quit()");
				yield; // Wait for SocketClient to disconnect.
				_stop_game_callback = null;
			}
		}

		try {
			if (_game_process != uint32.MAX)
				_subprocess_launcher.wait(_game_process);
			_game_process = uint32.MAX;
		} catch (GLib.Error e) {
			loge(e.message);
		}
	}

	private void deploy_game()
	{
		Gtk.FileChooserDialog fcd = new Gtk.FileChooserDialog("Select destination directory..."
			, this.active_window
			, FileChooserAction.SELECT_FOLDER
			, "Cancel"
			, ResponseType.CANCEL
			, "Open"
			, ResponseType.ACCEPT
			);

		if (fcd.run() == ResponseType.ACCEPT) {
			GLib.File data_dir = File.new_for_path(fcd.get_filename());

			string args[] =
			{
				ENGINE_EXE,
				"--source-dir",
				_project.source_dir(),
				"--map-source-dir",
				"core",
				_project.toolchain_dir(),
				"--data-dir",
				data_dir.get_path(),
				"--compile"
			};

			try {
				uint32 compiler = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
				int exit_status = _subprocess_launcher.wait(compiler);
				if (exit_status == 0) {
					string game_name = DEPLOY_DEFAULT_NAME;
					GLib.File engine_exe_src = File.new_for_path(DEPLOY_EXE);
					GLib.File engine_exe_dst = File.new_for_path(Path.build_filename(data_dir.get_path(), game_name + EXE_SUFFIX));
					engine_exe_src.copy(engine_exe_dst, FileCopyFlags.OVERWRITE);

#if CROWN_PLATFORM_WINDOWS
					string lua51_name = "lua51.dll";
					GLib.File lua51_dll_src = File.new_for_path(lua51_name);
					GLib.File lua51_dll_dst = File.new_for_path(Path.build_filename(data_dir.get_path(), lua51_name));
					lua51_dll_src.copy(lua51_dll_dst, FileCopyFlags.OVERWRITE);

					string openal_name = "openal-release.dll";
					GLib.File openal_dll_src = File.new_for_path(openal_name);
					GLib.File openal_dll_dst = File.new_for_path(Path.build_filename(data_dir.get_path(), openal_name));
					openal_dll_src.copy(openal_dll_dst, FileCopyFlags.OVERWRITE);
#endif

					logi("Project deployed to `%s`".printf(data_dir.get_path()));
				}
			}
			catch (Error e) {
				loge("%s".printf(e.message));
				loge("Failed to deploy project");
			}
		}

		fcd.destroy();
	}

	private async void on_editor_view_realized()
	{
		start_editor.begin(_editor_view.window_id);
	}

	private async void on_resource_preview_view_realized()
	{
		start_resource_preview.begin(_resource_preview_view.window_id);
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
			title += (_level._name == LEVEL_EMPTY) ? "untitled" : _level._name;

			if (_database.changed())
				title += " • ";

			title += " - ";
		}

		title += LEVEL_EDITOR_WINDOW_TITLE;

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
						, Gtk.ButtonsType.YES_NO
						, "A file named `%s` already exists.\nOverwrite?".printf(_project.basename(path))
						);
					md.set_default_response(ResponseType.NO);

					rt = md.run();
					md.destroy();
				}
			} while (rt != ResponseType.YES);

			fcd.destroy();
		}

		// Save level
		string resource_filename = _project.absolute_path_to_resource_filename(path);
		string resource_path     = _project.resource_filename_to_resource_path(resource_filename);
		string resource_name     = _project.resource_path_to_resource_name(resource_path);

		_level.save(resource_name);
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

	protected override void shutdown()
	{
		// Disable auto-save.
		if (_save_timer_id > 0)
			GLib.Source.remove(_save_timer_id);

		// Save editor settings.
		_user.save(_user_file.get_path());
		_preferences_dialog.encode(_settings);
		SJSON.save(_settings, _settings_file.get_path());
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
		string resource_filename = _project.absolute_path_to_resource_filename(path);
		string resource_path     = _project.resource_filename_to_resource_path(resource_filename);
		string resource_name     = _project.resource_path_to_resource_name(resource_path);

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

			this.show_panel("main_vbox", Gtk.StackTransitionType.NONE);
			_user.add_or_touch_recent_project(source_dir, source_dir);
			_console_view.reset();
			restart_backend.begin(source_dir, LEVEL_NONE);
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

	private void on_save(GLib.SimpleAction action, GLib.Variant? param)
	{
		save();
	}

	private void on_save_as(GLib.SimpleAction action, GLib.Variant? param)
	{
		save_as(null);
	}

	private void on_import(GLib.SimpleAction action, GLib.Variant? param)
	{
		string destination_dir = param.get_string();
		_project.import(destination_dir != "" ? destination_dir : null, this.active_window);
	}

	private void on_preferences(GLib.SimpleAction action, GLib.Variant? param)
	{
		_preferences_dialog.set_transient_for(this.active_window);
		_preferences_dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT);
		_preferences_dialog.show_all();
	}

	private void on_deploy(GLib.SimpleAction action, GLib.Variant? param)
	{
		deploy_game();
	}

	private int run_level_changed_dialog(Gtk.Window? parent)
	{
		Gtk.MessageDialog md = new Gtk.MessageDialog(parent
			, Gtk.DialogFlags.MODAL
			, Gtk.MessageType.WARNING
			, Gtk.ButtonsType.NONE
			, "Save changes to Level before closing?"
			);
		md.add_button("Close _without Saving", ResponseType.NO);
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

	private void on_close(GLib.SimpleAction action, GLib.Variant? param)
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

		string resource_path = param.get_string();
		string? type = Crown.resource_type(resource_path);
		string? name = Crown.resource_name(type, resource_path);
		if (type == null || name == null)
			return;

		if (type == "level") {
			activate_action("open-level", name);
			return;
		}

		GLib.AppInfo? app = null;

		if (type == "lua") {
			app = _preferences_dialog._lua_external_tool_button.get_app_info();
		} else if (is_image_file(resource_path)) {
			app = _preferences_dialog._image_external_tool_button.get_app_info();
		}

		try {
			GLib.File file = GLib.File.new_for_path(_project.resource_path_to_absolute_path(resource_path));
			if (app == null)
				app = file.query_default_handler();

			GLib.List<GLib.File> files = new GLib.List<GLib.File>();
			files.append(file);
			app.launch(files, null);
		}
		catch (Error e) {
			loge(e.message);
		}
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

	private void on_create_primitive(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (action.name == "primitive-cube")
			set_placeable("unit", "core/units/primitives/cube");
		else if (action.name == "primitive-sphere")
			set_placeable("unit", "core/units/primitives/sphere");
		else if (action.name == "primitive-cone")
			set_placeable("unit", "core/units/primitives/cone");
		else if (action.name == "primitive-cylinder")
			set_placeable("unit", "core/units/primitives/cylinder");
		else if (action.name == "primitive-plane")
			set_placeable("unit", "core/units/primitives/plane");
		else if (action.name == "camera")
			set_placeable("unit", "core/units/camera");
		else if (action.name == "light")
			set_placeable("unit", "core/units/light");
		else if (action.name == "sound-source")
			set_placeable("sound", "");

		activate_action("tool", new GLib.Variant.int32(ToolType.PLACE));
	}

	private void on_create_unit(GLib.SimpleAction action, GLib.Variant? param)
	{
		_level.spawn_empty_unit();
		_editor.send(DeviceApi.frame());
	}

	private void on_camera_view(GLib.SimpleAction action, GLib.Variant? param)
	{
		_camera_view_type = (CameraViewType)param.get_int32();

		send_state();
		_editor.send(DeviceApi.frame());
		action.set_state(param);
	}

	private void on_resource_chooser(GLib.SimpleAction action, GLib.Variant? param)
	{
		_resource_popover.show_all();
	}

	private void on_project_browser(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_project_stack.is_visible()) {
			_project_stack.hide();
		} else {
			_project_stack.show_all();
		}
	}

	private void on_console(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_console_view.is_visible()) {
			if (_console_view._entry.has_focus)
				_console_view.hide();
			else
				_console_view._entry.grab_focus_without_selecting();
		} else {
			_console_view.show_all();
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
					_editor.send(DeviceApi.refresh());
					_editor.send(DeviceApi.frame());
					_game.send(DeviceApi.refresh());
					_game.send(DeviceApi.frame());
				}
			});
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
		string icon_name_displayed = _toolbar_run.icon_name;

		stop_game.begin((obj, res) => {
				stop_game.end(res);

				if (icon_name_displayed == "game-run") {
					// Always change icon state regardless of failures
					_toolbar_run.icon_name = "game-stop";

					//
					_game.disconnected.disconnect(on_game_disconnected);
					_game.disconnected.connect(on_game_disconnected_externally);

					start_game.begin(action.name == "test-level" ? StartGame.TEST : StartGame.NORMAL);
				}
			});
	}

	private void on_undo(GLib.SimpleAction action, GLib.Variant? param)
	{
		int id = _database.undo();
		if (id != -1)
			_statusbar.set_temporary_message("Undo: " + ActionNames[id]);
	}

	private void on_redo(GLib.SimpleAction action, GLib.Variant? param)
	{
		int id = _database.redo();
		if (id != -1)
			_statusbar.set_temporary_message("Redo: " + ActionNames[id]);
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
			AppInfo.launch_default_for_uri("https://docs.crownengine.org/html/v" + CROWN_VERSION, null);
		}
		catch (Error e) {
			loge(e.message);
		}
	}

	private void on_report_issue(GLib.SimpleAction action, GLib.Variant? param)
	{
		try {
			AppInfo.launch_default_for_uri("https://github.com/crownengine/crown/issues", null);
		}
		catch (Error e) {
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
			AppInfo.launch_default_for_uri("https://docs.crownengine.org/html/v" + CROWN_VERSION + "/changelog.html", null);
		}
		catch (Error e) {
			loge(e.message);
		}
	}

	private void on_about(GLib.SimpleAction action, GLib.Variant? param)
	{
		Gtk.AboutDialog dlg = new Gtk.AboutDialog();
		dlg.set_destroy_with_parent(true);
		dlg.set_transient_for(this.active_window);
		dlg.set_modal(true);
		dlg.set_logo_icon_name(CROWN_ICON_NAME);

		dlg.program_name = LEVEL_EDITOR_WINDOW_TITLE;
		dlg.version = CROWN_VERSION;
		dlg.website = "https://www.crownengine.org";
		dlg.copyright = "Copyright (c) 2012-2023 Daniele Bartolini et al.";
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

		string resource_path = param.get_string();
		string? type = Crown.resource_type(resource_path);
		string? name = Crown.resource_name(type, resource_path);
		if (type == null || name == null)
			return;

		if (name == _level._name) {
			int rt = ResponseType.YES;

			if (_database.changed())
				rt = run_level_changed_dialog(this.active_window);

			if (!_database.changed() || rt == ResponseType.YES && save() || rt == ResponseType.NO) {
				new_level();
				send_state();
				_editor.send(DeviceApi.frame());

				_project.delete_resource(type, name);
			}
		} else {
			_project.delete_resource(type, name);
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
			_editor_pane.set_position(210);
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
			menu_set_enabled(false, action_entries_file, {"new-project", "open-project", "quit"});
			menu_set_enabled(false, action_entries_edit);
			menu_set_enabled(false, action_entries_create);
			menu_set_enabled(false, action_entries_camera);
			menu_set_enabled(false, action_entries_view);
			menu_set_enabled(false, action_entries_debug);
			menu_set_enabled(true, action_entries_help);
		}
	}

	public void set_placeable(string type, string name)
	{
		_placeable_type = type;
		_placeable_name = name;
		_editor.send_script(LevelEditorApi.set_placeable(type, name));
	}

	public void activate_last_tool_before_place()
	{
		if (_tool_type != ToolType.PLACE)
			return;

		activate_action("tool", new GLib.Variant.int32(_tool_type_prev));
	}
}

// Global paths
public static GLib.File _toolchain_dir;
public static GLib.File _templates_dir;
public static GLib.File _config_dir;
public static GLib.File _logs_dir;
public static GLib.File _cache_dir;
public static GLib.File _documents_dir;
public static GLib.File _log_file;
public static GLib.File _settings_file;
public static GLib.File _user_file;
public static GLib.File _console_history_file;

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
		string line = "%s.%06d  %s: %s\n".printf(now_str
			, now_us
			, system
			, message
			);

		_console_view.log(severity, line);
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
	}
	catch (Error e) {
		loge(e.message);
	}
#else
	GLib.SubprocessLauncher sl = new GLib.SubprocessLauncher(subprocess_flags());
	try {
		sl.spawnv({ "explorer.exe", directory, null });
	}
	catch (Error e) {
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
	}
	catch (GLib.Error e) {
		loge(e.message);
	}

	return false;
}

private void device_frame_delayed(uint delay_ms, ConsoleClient client)
{
	// FIXME: find a way to time exactly when it is effective to queue a redraw.
	// See: https://blogs.gnome.org/jnelson/2010/10/13/those-realize-map-widget-signals/
	GLib.Timeout.add_full(GLib.Priority.DEFAULT, delay_ms, () => {
			client.send(DeviceApi.frame());
			return GLib.Source.REMOVE;
		});
}

public static int main(string[] args)
{
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
	_config_dir = GLib.File.new_for_path(GLib.Path.build_filename(GLib.Environment.get_user_config_dir(), "crown"));
	try {
		_config_dir.make_directory();
	} catch (Error e) {
		/* Nobody cares */
	}
	_logs_dir = GLib.File.new_for_path(GLib.Path.build_filename(_config_dir.get_path(), "logs"));
	try {
		_logs_dir.make_directory();
	} catch (Error e) {
		/* Nobody cares */
	}
	_documents_dir = GLib.File.new_for_path(GLib.Environment.get_user_special_dir(GLib.UserDirectory.DOCUMENTS));
	_cache_dir = GLib.File.new_for_path(GLib.Path.build_filename(GLib.Environment.get_user_cache_dir(), "crown"));
	try {
		_cache_dir.make_directory();
	} catch (Error e) {
		/* Nobody cares */
	}

	_log_file = GLib.File.new_for_path(GLib.Path.build_filename(_logs_dir.get_path(), new GLib.DateTime.now_local().format("%Y-%m-%d") + ".log"));
	_log_stream = GLib.FileStream.open(_log_file.get_path(), "a");

	// If args does not contain --child, spawn the launcher.
	int ii;
	for (ii = 0; ii < args.length; ++ii) {
		if (args[ii] == "--child")
			break;
	}

	if (ii == args.length) {
		_log_prefix = "launcher";
		return launcher_main(args);
	}

	_log_prefix = "editor";
	// Remove --child from args for backward compatibility.
	if (args.length > 1)
		args = args[0 : args.length - 1];

	_settings_file = GLib.File.new_for_path(GLib.Path.build_filename(_config_dir.get_path(), "settings.sjson"));
	_user_file = GLib.File.new_for_path(GLib.Path.build_filename(_config_dir.get_path(), "user.sjson"));
	_console_history_file = GLib.File.new_for_path(GLib.Path.build_filename(_cache_dir.get_path(), "console_history.txt"));

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
		".",
		"../..",
		"../../../samples"
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
		".",
		"../..",
		"../../.."
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
