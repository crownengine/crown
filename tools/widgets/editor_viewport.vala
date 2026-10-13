/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
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
			return _("Perspective");
		case FRONT:
			return _("View Front");
		case BACK:
			return _("View Back");
		case RIGHT:
			return _("View Right");
		case LEFT:
			return _("View Left");
		case TOP:
			return _("View Top");
		case BOTTOM:
			return _("View Bottom");
		default:
			return _("View Unknown");
		}
	}
}

public enum ViewportRenderMode
{
	CONTINUOUS,
	PUMPED,

	COUNT
}

#if CROWN_GTK3
public class EditorViewport : Gtk.Bin
#else
public class EditorViewport : Gtk.Box
#endif
{
	public const string EDITOR_DISCONNECTED = "editor-disconnected";
	public const string EDITOR_OOPS = "editor-oops";
#if !CROWN_GTK3
	public const string EDITOR_VIEWPORT = "editor-viewport";
#endif

	public const GLib.ActionEntry[] actions =
	{
		{ "camera-view",           on_camera_view,           "i",  "0" },  // See: Crown.CameraViewType
		{ "camera-frame-selected", on_camera_frame_selected, null, null },
	};

	public DatabaseEditor _database_editor;
	public Project _project;
	public string _boot_dir;
	public string? _render_config;
	public string _console_address;
	public uint16 _console_port;
	public PreferencesDialog _preferences;
	public ViewportRenderMode _render_mode;
	public bool _input_enabled;
	public RuntimeInstance _runtime;
	public EditorView _editor_view;
	public Gtk.Overlay _overlay;
	public Gtk.Stack _stack;
	public GLib.SimpleActionGroup _action_group;
#if !CROWN_GTK3
	public Gtk.Label _export_error_label;
	public uint _size_idle_id;
	public uint _size_wait_id;
	public uint _size_poll_id;
	public int _last_pixel_width;
	public int _last_pixel_height;
	public bool _start_requested;
	public bool _runtime_starting;
#endif

	public EditorViewport(string name
		, DataCompiler data_compiler
		, DatabaseEditor database_editor
		, Project project
		, string boot_dir
		, string console_addr
		, uint16 console_port
		, PreferencesDialog preferences
		, ViewportRenderMode render_mode = ViewportRenderMode.PUMPED
		, bool input_enabled = true
		)
	{
		_database_editor = database_editor;
		_project = project;
		_boot_dir = boot_dir;
		_console_address = console_addr;
		_console_port = console_port;
		_preferences = preferences;
		_render_mode = render_mode;
		_input_enabled = input_enabled;

		_runtime = new RuntimeInstance(name, data_compiler);
		_runtime.disconnected_unexpected.connect(on_editor_disconnected_unexpected);
#if CROWN_GTK3
		_overlay = new Gtk.Overlay();
#else
		_runtime.connected.connect(on_editor_connected);
#endif

		_stack = new Gtk.Stack();
		_stack.halign = Gtk.Align.FILL;
		_stack.valign = Gtk.Align.FILL;
		_stack.add_named(editor_disconnected(), EDITOR_DISCONNECTED);
		_stack.add_named(editor_oops(() => { restart_runtime.begin(); }), EDITOR_OOPS);

		_stack.set_visible_child_name(EDITOR_DISCONNECTED);

		_action_group = new GLib.SimpleActionGroup();
		_action_group.add_action_entries(actions, this);
		this.insert_action_group("viewport", _action_group);

#if CROWN_GTK3
		this.can_focus = true;
		this.add(_stack);
#else
		_editor_view = new EditorView(_runtime, _input_enabled);
		_editor_view.hexpand = true;
		_editor_view.vexpand = true;
		_editor_view.show.connect(on_editor_view_show);
		_editor_view.notify["width"].connect(on_editor_view_size_changed);
		_editor_view.notify["height"].connect(on_editor_view_size_changed);
		_editor_view.notify["scale-factor"].connect(on_editor_view_size_changed);
		_editor_view.export_error.connect(on_export_error);
		_editor_view.export_ready.connect(on_export_ready);
		_overlay = new Gtk.Overlay();
		_overlay.hexpand = true;
		_overlay.vexpand = true;
		_overlay.set_child(_editor_view);
		_export_error_label = new Gtk.Label(null);
		_export_error_label.wrap = true;
		_export_error_label.halign = Gtk.Align.CENTER;
		_export_error_label.valign = Gtk.Align.CENTER;
		_export_error_label.add_css_class("error");
		_export_error_label.can_target = false;
		_export_error_label.visible = false;
		_overlay.add_overlay(_export_error_label);
		_stack.add_named(_overlay, EDITOR_VIEWPORT);
		_stack.set_visible_child_name(EDITOR_VIEWPORT);
		_stack.hexpand = true;
		_stack.vexpand = true;
		_size_idle_id = 0;
		_size_wait_id = 0;
		_size_poll_id = 0;
		_last_pixel_width = 0;
		_last_pixel_height = 0;
		_start_requested = true;
		_runtime_starting = false;

		this.focusable = true;
		this.hexpand = true;
		this.vexpand = true;
		this.append(_stack);
#endif /* if CROWN_GTK3 */
	}

	public void on_editor_disconnected_unexpected(RuntimeInstance ri)
	{
#if !CROWN_GTK3
		_editor_view.close_export();
		_runtime_starting = false;
		if (_size_poll_id != 0) {
			GLib.Source.remove(_size_poll_id);
			_size_poll_id = 0;
		}
#endif
		_stack.set_visible_child_name(EDITOR_OOPS);
	}

#if !CROWN_GTK3
	public void on_export_error(string message)
	{
		_export_error_label.label = message;
		_export_error_label.visible = true;
	}

	public void on_export_ready()
	{
		_export_error_label.visible = false;
	}

	public void on_editor_view_show()
	{
		if (_runtime_starting || _runtime.is_connected())
			return;
		_start_requested = true;
		on_editor_view_size_changed();
	}

	public void on_editor_view_size_changed()
	{
		if (_size_idle_id == 0)
			_size_idle_id = GLib.Idle.add(on_apply_view_size);
	}

	public bool on_wait_for_view_size()
	{
		if (!_start_requested || _editor_view.get_width() > 0 && _editor_view.get_height() > 0) {
			_size_wait_id = 0;
			on_editor_view_size_changed();
			return GLib.Source.REMOVE;
		}
		return GLib.Source.CONTINUE;
	}

	public bool on_poll_view_size()
	{
		int width = _editor_view.get_width() * _editor_view.get_scale_factor();
		int height = _editor_view.get_height() * _editor_view.get_scale_factor();
		if (width != _last_pixel_width || height != _last_pixel_height)
			on_editor_view_size_changed();
		return GLib.Source.CONTINUE;
	}

	public bool on_apply_view_size()
	{
		_size_idle_id = 0;
		int width = _editor_view.get_width() * _editor_view.get_scale_factor();
		int height = _editor_view.get_height() * _editor_view.get_scale_factor();
		if (width < 1 || height < 1) {
			if (_start_requested && _size_wait_id == 0)
				_size_wait_id = GLib.Timeout.add(50, on_wait_for_view_size);
			return GLib.Source.REMOVE;
		}
		if (_start_requested && !_runtime_starting) {
			_runtime_starting = true;
			_start_requested = false;
			_last_pixel_width = width;
			_last_pixel_height = height;
			start_runtime.begin(1, width, height);
		} else if (_runtime.is_connected() && (width != _last_pixel_width || height != _last_pixel_height)) {
			_last_pixel_width = width;
			_last_pixel_height = height;
			_runtime.send(DeviceApi.resize(width, height));
			frame();
		}
		return GLib.Source.REMOVE;
	}

	public void on_editor_connected()
	{
		if (_size_poll_id == 0)
			_size_poll_id = GLib.Timeout.add(100, on_poll_view_size);
		_export_error_label.visible = false;
		int width = _editor_view.get_width() * _editor_view.get_scale_factor();
		int height = _editor_view.get_height() * _editor_view.get_scale_factor();
		if (width > 0 && height > 0) {
			_last_pixel_width = width;
			_last_pixel_height = height;
			_runtime.send(DeviceApi.resize(width, height));
		}
		_runtime.send(DeviceApi.frame());
		_runtime.send(DeviceApi.frame());
		_stack.set_visible_child_name(EDITOR_VIEWPORT);
	}
#endif /* if !CROWN_GTK3 */
	public async void start_runtime(uint window_xid, int width, int height)
	{
#if CROWN_GTK3
		if (window_xid == 0)
			return;
#endif
		// Spawn the level editor.
		PortPipe? port_pipe = PortPipe.create();
		if (port_pipe == null)
			return;

#if !CROWN_GTK3 && !CROWN_PLATFORM_LINUX && !CROWN_PLATFORM_WINDOWS
		on_export_error(_("Viewport export is unavailable on this platform."));
		_runtime_starting = false;
		return;
#endif

#if !CROWN_GTK3
		if (!_editor_view.prepare_export()) {
			_runtime_starting = false;
			return;
		}
#endif

		string[] args =
		{
			ENGINE_EXE,
			_preferences._editor_renderer.value != "default" ? "--renderer" : "",
			_preferences._editor_renderer.value != "default" ? _preferences._editor_renderer.value : "",
#if CROWN_PLATFORM_LINUX && CROWN_GTK3
			"--display-server", "x11",
#endif
			"--data-dir",
			_project.data_dir(),
			"--boot-dir",
			_boot_dir,
#if CROWN_GTK3
			"--parent-window",
			window_xid.to_string(),
#endif
			"--port-file",
			port_pipe._path,
			"--wait-console",
			_render_mode == ViewportRenderMode.PUMPED ? "--pumped" : "",
			"--window-rect", "0", "0", width.to_string(), height.to_string(),
#if !CROWN_GTK3
			"--headless",
			"--export",
			_editor_view._export_path,
#endif
		};

		if (_render_config != null) {
			args += "--render-config";
			args += _render_config;
		}

		try {
			_runtime._process_id = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
		} catch (Error e) {
			loge(e.message);
#if !CROWN_GTK3
			_runtime_starting = false;
			_editor_view.close_export();
#endif
			return;
		}

		uint32 process_id = _runtime._process_id;
		uint16 console_port = yield port_pipe.read_async(EDITOR_PORT_TIMEOUT_MS);
		port_pipe = null;
		if (_runtime._process_id != process_id)
			return;
		if (console_port == 0) {
			loge("Cannot read console port for %s".printf(_runtime._name));
#if !CROWN_GTK3
			_runtime_starting = false;
			_editor_view.close_export();
#endif
			return;
		}

		bool connected = yield _runtime.connect_async(_console_address, console_port);
		if (_runtime._process_id != process_id)
			return;
		if (!connected) {
			loge("Cannot connect to %s".printf(_runtime._name));
#if !CROWN_GTK3
			_runtime_starting = false;
			_editor_view.close_export();
#endif
			return;
		}
	}

	public async void stop_runtime()
	{
#if !CROWN_GTK3
		_start_requested = false;
		if (_size_poll_id != 0) {
			GLib.Source.remove(_size_poll_id);
			_size_poll_id = 0;
		}
		_editor_view.close_export();
#endif
		yield _runtime.stop();
		_stack.set_visible_child_name(EDITOR_DISCONNECTED);
#if !CROWN_GTK3
		_runtime_starting = false;
		_last_pixel_width = 0;
		_last_pixel_height = 0;
#endif
	}

	public async void restart_runtime()
	{
		yield stop_runtime();
#if CROWN_GTK3
		if (_editor_view != null) {
			_overlay.remove(_editor_view);
			_stack.remove(_overlay);
			_editor_view = null;
		}

		_editor_view = new EditorView(_runtime, _input_enabled);
		_editor_view.native_window_ready.connect(on_editor_view_realized);

		_overlay.add(_editor_view);
		_overlay.show_all();

		_stack.add(_overlay);
		_stack.set_visible_child(_overlay);
	}

	public async void on_editor_view_realized(uint window_id, int width, int height)
	{
		start_runtime.begin(window_id, width, height);
#else
		_stack.set_visible_child_name(EDITOR_VIEWPORT);
		_start_requested = true;
		on_editor_view_size_changed();
#endif /* if CROWN_GTK3 */
	}

	public void frame()
	{
		if (_render_mode != ViewportRenderMode.PUMPED)
			return;

		_runtime.send(DeviceApi.frame());
	}

	public void on_camera_view(GLib.SimpleAction action, GLib.Variant? param)
	{
		CameraViewType view_type = (CameraViewType)param.get_int32();

		_runtime.send_script(LevelEditorApi.set_camera_view_type(view_type));
		frame();

		action.set_state(param);
	}

	public void on_camera_frame_selected(GLib.SimpleAction action, GLib.Variant? param)
	{
		_runtime.send_script(LevelEditorApi.frame_objects(_database_editor._selection.data));
		frame();
	}
}

} /* namespace Crown */
