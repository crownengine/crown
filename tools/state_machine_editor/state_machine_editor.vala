/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class StateMachineEditor : Gtk.ApplicationWindow
{
	public DatabaseEditor _database_editor;
	public Database _database;
	public EditorViewport _editor_viewport;
	public RuntimeInstance _runtime;
	public ObjectTree _objects_tree;
	public ObjectProperties _objects_properties;
	public Gtk.Paned _inspector_paned;
	public Statusbar _statusbar;

	public Gtk.Paned _paned;
	public Gtk.Button _cancel;
	public Gtk.Button _save;
	public Gtk.HeaderBar _header_bar;
	public Gtk.Box _box;

	public string _state_machine_name;
	public Guid _state_machine_id;

	public signal void saved();

	public StateMachineEditor(Gtk.Application application
		, Project project
		, string boot_dir
		, string console_addr
		, uint16 console_port
		, uint32 undo_redo_size
		)
	{
		Object(application: application);

		_database_editor = new DatabaseEditor(project, undo_redo_size);
		_database_editor.undo.connect(on_undo);
		_database_editor.redo.connect(on_redo);
		this.insert_action_group("database", _database_editor._action_group);

		_database = _database_editor._database;
		_database.objects_created.connect(on_objects_created);
		_database.objects_destroyed.connect(on_objects_destroyed);
		_database.objects_changed.connect(on_objects_changed);

		_objects_tree = new ObjectTree(_database_editor);
		_objects_properties = new ObjectProperties(_database_editor);

		_database_editor.load_types();

		_editor_viewport = new EditorViewport("unit_editor"
			, _database_editor
			, project
			, boot_dir
			, console_addr
			, console_port
			, ViewportRenderMode.CONTINUOUS
			);
		this.insert_action_group("viewport", _editor_viewport._action_group);
		_runtime = _editor_viewport._runtime;
		_runtime.connected.connect(on_editor_connected);
		_runtime.disconnected.connect(on_editor_disconnected);
		_runtime.disconnected_unexpected.connect(on_editor_disconnected_unexpected);

		_statusbar = new Statusbar();

		_cancel = new Gtk.Button.with_label("Cancel");
		_cancel.clicked.connect(() => {
				close();
			});
		_save = new Gtk.Button.with_label("Save & Reload");
		_save.get_style_context().add_class("suggested-action");
		_save.clicked.connect(save);

		_header_bar = new Gtk.HeaderBar();
		_header_bar.title = "State Machine Editor";
		_header_bar.show_close_button = true;
		_header_bar.pack_start(_cancel);
		_header_bar.pack_end(_save);

		_inspector_paned = new Gtk.Paned(Gtk.Orientation.VERTICAL);
		_inspector_paned.pack1(_objects_tree, true, false);
		_inspector_paned.pack2(_objects_properties, true, false);

		_paned = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
		_paned.pack1(_editor_viewport, true, false);
		_paned.pack2(_inspector_paned, false, false);

		this.set_titlebar(_header_bar);
		this.set_size_request(1280, 720);

		int win_w;
		int win_h;
		this.get_size(out win_w, out win_h);
		_paned.set_position(win_w - 360);

		GLib.Menu menu = new GLib.Menu();
		GLib.MenuItem mi = null;

		mi = new GLib.MenuItem("Edit", null);
		mi.set_submenu(make_database_editor_menu());
		menu.append_item(mi);

		mi = new GLib.MenuItem("Camera", null);
		mi.set_submenu(make_camera_view_menu());
		menu.append_item(mi);

		this.show_menubar = false;
		Gtk.MenuBar menubar = new Gtk.MenuBar.from_model(menu);

		_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_box.pack_start(menubar, false);
		_box.pack_start(_paned);
		_box.pack_start(_statusbar, false);

		this.delete_event.connect(on_close_request);
		this.add(_box);

		reset();

		_editor_viewport.restart_runtime.begin();
	}

	public void update_window_title()
	{
		string title = "";

		if (_state_machine_name != null) {
			if (_database.changed())
				title += " • ";

			title += (_state_machine_name == LEVEL_EMPTY) ? "untitled" : _state_machine_name;
			title += " - ";
		}

		title += CROWN_EDITOR_NAME;

		if (this.title != title)
			this.title = title;
	}

	public void send()
	{
	}

	public void on_editor_connected(RuntimeInstance ri, string address, int port)
	{
		send();
	}

	public void on_editor_disconnected(RuntimeInstance ri)
	{
	}

	public void on_editor_disconnected_unexpected(RuntimeInstance ri)
	{
	}

	public void reset()
	{
		_database.reset();
		_state_machine_name = "";
		_state_machine_id = GUID_ZERO;
	}

	public void save()
	{
		if (_database.save(_database._project.absolute_path(_state_machine_name) + "." + OBJECT_TYPE_STATE_MACHINE, _state_machine_id) == 0)
			saved();
	}

	public void on_objects_created(Guid?[] object_ids, uint32 flags)
	{
		Guid last_created = object_ids[object_ids.length - 1];

		_objects_tree.set_object(_state_machine_id); // Force update the tree.
		_database_editor.selection_set({ last_created }); // Select the objects just created.

		update_window_title();
	}

	public void on_objects_destroyed(Guid?[] object_ids, uint32 flags = 0)
	{
		_objects_tree.set_object(_state_machine_id); // Force update the tree.
		_database_editor.selection_set({ _state_machine_id }); // Select the root object which must always exits.
		update_window_title();
	}

	public void on_objects_changed(Guid?[] object_ids, uint32 flags = 0)
	{
		Guid last_changed = object_ids[object_ids.length - 1];

		_objects_tree.set_object(_state_machine_id); // Force update the tree.
		_database_editor.selection_set({ last_changed });
		update_window_title();
	}

	public void do_set_state_machine(string state_machine_name)
	{
		reset();

		string resource_path = ResourceId.path(OBJECT_TYPE_STATE_MACHINE, state_machine_name);
		string path = _database._project.absolute_path(resource_path);

		if (_database.load_from_path(out _state_machine_id, path, resource_path) != 0)
			return;

		_state_machine_name = state_machine_name;
		_objects_tree.set_object(_state_machine_id);
		_database_editor.selection_set({ _state_machine_id });
		update_window_title();
		send();
	}

	public void set_state_machine(string state_machine_name)
	{
		if (state_machine_name == _state_machine_name)
			return;

		if (!_database.changed()) {
			this.do_set_state_machine(state_machine_name);
		} else {
			Gtk.Dialog dlg = new_resource_changed_dialog(this, _state_machine_name);
			dlg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.NO) {
						this.do_set_state_machine(state_machine_name);
					} else if (response_id == Gtk.ResponseType.YES) {
						this.save();
						this.do_set_state_machine(state_machine_name);
					}
					dlg.destroy();
				});
			dlg.show_all();
		}
	}

	public void on_undo(int action_id)
	{
		_statusbar.set_temporary_message("Undo: " + ActionNames[action_id]);
	}

	public void on_redo(int action_id)
	{
		_statusbar.set_temporary_message("Redo: " + ActionNames[action_id]);
	}

	public bool on_close_request(Gdk.EventAny event)
	{
		this.hide();
		return Gdk.EVENT_STOP;
	}
}

} /* namespace Crown */
