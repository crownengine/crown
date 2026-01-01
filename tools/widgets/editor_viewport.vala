/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public enum ViewportRenderMode
{
	CONTINUOUS,
	PUMPED,

	COUNT
}

public class EditorViewport : Gtk.Bin
{
	public const string EDITOR_DISCONNECTED = "editor-disconnected";
	public const string EDITOR_OOPS = "editor-oops";

	public const GLib.ActionEntry[] actions =
	{
		{ "camera-view",           on_camera_view,           "i",  "0" },  // See: Crown.CameraViewType
		{ "camera-frame-selected", on_camera_frame_selected, null, null },
	};

	public DatabaseEditor _database_editor;
	public Project _project;
	public string _boot_dir;
	public string _console_address;
	public uint16 _console_port;
	public ViewportRenderMode _render_mode;
	public bool _input_enabled;
	public RuntimeInstance _runtime;
	public EditorView _editor_view;
	public Gtk.Overlay _overlay;
	public Gtk.Stack _stack;
	public GLib.SimpleActionGroup _action_group;

	public EditorViewport(string name
		, DatabaseEditor database_editor
		, Project project
		, string boot_dir
		, string console_addr
		, uint16 console_port
		, ViewportRenderMode render_mode = ViewportRenderMode.PUMPED
		, bool input_enabled = true
		)
	{
		_database_editor = database_editor;
		_project = project;
		_boot_dir = boot_dir;
		_console_address = console_addr;
		_console_port = console_port;
		_render_mode = render_mode;
		_input_enabled = input_enabled;

		_runtime = new RuntimeInstance(name);
		_runtime.disconnected_unexpected.connect(on_editor_disconnected_unexpected);

		_overlay = new Gtk.Overlay();

		_stack = new Gtk.Stack();
		_stack.add_named(editor_disconnected(), EDITOR_DISCONNECTED);
		_stack.add_named(editor_oops(() => { restart_runtime.begin(); }), EDITOR_OOPS);

		_stack.set_visible_child_name(EDITOR_DISCONNECTED);

		_action_group = new GLib.SimpleActionGroup();
		_action_group.add_action_entries(actions, this);
		this.insert_action_group("viewport", _action_group);

		this.can_focus = true;
		this.add(_stack);
	}

	public void on_editor_disconnected_unexpected(RuntimeInstance ri)
	{
		_stack.set_visible_child_name(EDITOR_OOPS);
	}

	public async void start_runtime(uint window_xid, int width, int height)
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
			_boot_dir,
			"--parent-window",
			window_xid.to_string(),
			"--console-port",
			_console_port.to_string(),
			"--wait-console",
			_render_mode == ViewportRenderMode.PUMPED ? "--pumped" : "",
			"--window-rect", "0", "0", width.to_string(), height.to_string(),
		};

		try {
			_runtime._process_id = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
		} catch (Error e) {
			loge(e.message);
		}

		// Try to connect to the level editor.
		int tries = yield _runtime.connect_async(_console_address
			, _console_port
			, EDITOR_CONNECTION_TRIES
			, EDITOR_CONNECTION_INTERVAL
			);
		if (tries == EDITOR_CONNECTION_TRIES) {
			loge("Cannot connect to %s".printf(_runtime._name));
			return;
		}
	}

	public async void stop_runtime()
	{
		yield _runtime.stop();
		_stack.set_visible_child_name(EDITOR_DISCONNECTED);
	}

	public async void restart_runtime()
	{
		yield stop_runtime();

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
		Guid?[] selected_objects = _database_editor._selection.to_array();
		_runtime.send_script(LevelEditorApi.frame_objects(selected_objects));
		frame();
	}
}

} /* namespace Crown */
