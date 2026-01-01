/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class UnitEditor : Gtk.ApplicationWindow
{
	public DatabaseEditor _database_editor;
	public Database _database;
	public EditorViewport _editor_viewport;
	public RuntimeInstance _runtime;
	public ObjectTree _objects_tree;
	public PropertiesView _properties_view;
	public Level _level;
	public Statusbar _statusbar;

	public Gtk.Paned _paned_object;
	public Gtk.Paned _paned_inspector;
	public Gtk.Button _cancel;
	public Gtk.Button _save;
	public Gtk.HeaderBar _header_bar;
	public Gtk.Box _box;

	public string _unit_name;
	public Guid _unit_id;

	public signal void saved();

	public UnitEditor(Gtk.Application application
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
		_database.object_type_added.connect(on_object_type_added);

		_objects_tree = new ObjectTree(_database_editor);
		_properties_view = new PropertiesView(_database_editor);
		_properties_view.register_object_type(OBJECT_TYPE_UNIT, new UnitView(_database));

		_database_editor.load_types();

		_editor_viewport = new EditorViewport("unit_editor"
			, _database_editor
			, project
			, boot_dir
			, console_addr
			, console_port
			);
		this.insert_action_group("viewport", _editor_viewport._action_group);
		_runtime = _editor_viewport._runtime;
		_runtime.connected.connect(on_editor_connected);
		_runtime.disconnected.connect(on_editor_disconnected);
		_runtime.disconnected_unexpected.connect(on_editor_disconnected_unexpected);

		_level = new Level(_database, _runtime);

		_statusbar = new Statusbar();

		_cancel = new Gtk.Button.with_label("Cancel");
		_cancel.clicked.connect(() => {
				close();
			});
		_save = new Gtk.Button.with_label("Save & Reload");
		_save.get_style_context().add_class("suggested-action");
		_save.clicked.connect(save);

		_header_bar = new Gtk.HeaderBar();
		_header_bar.title = "Unit Editor";
		_header_bar.show_close_button = true;
		_header_bar.pack_start(_cancel);
		_header_bar.pack_end(_save);

		_paned_inspector = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
		_paned_inspector.pack1(_editor_viewport, false, false);
		_paned_inspector.pack2(_properties_view, false, false);

		_paned_object = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
		_paned_object.pack1(_objects_tree, false, false);
		_paned_object.pack2(_paned_inspector, true, false);

		this.set_titlebar(_header_bar);
		this.set_size_request(1280, 720);

		int win_w;
		int win_h;
		this.get_size(out win_w, out win_h);
		_paned_inspector.set_position(530);
		_paned_object.set_position(340);

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
		_box.pack_start(_paned_object);
		_box.pack_start(_statusbar, false);

		this.delete_event.connect(on_close_request);
		this.add(_box);

		reset();

		_editor_viewport.restart_runtime.begin();
	}

	public void update_window_title()
	{
		string title = "";

		if (_database.changed())
			title += " • ";

		title += _unit_name;
		title += " - ";
		title += CROWN_EDITOR_NAME;

		if (this.title != title)
			this.title = title;
	}

	public void send()
	{
		StringBuilder sb = new StringBuilder();

		_level.send_level();

		Unit.generate_spawn_unit_commands(sb, { _unit_id, }, _database);

		if (sb.len > 0)
			_runtime.send_script(sb.str);

		_editor_viewport.frame();

		sb.erase();
		sb.append(LevelEditorApi.frame_objects({ _unit_id }));
		if (sb.len > 0)
			_runtime.send_script(sb.str);
		_editor_viewport.frame();
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
		_unit_name = "";
		_unit_id = GUID_ZERO;
	}

	public void save()
	{
		if (_unit_id == GUID_ZERO)
			return;

		if (_database.save(_database._project.absolute_path(_unit_name) + "." + OBJECT_TYPE_UNIT, _unit_id) == 0)
			saved();

		update_window_title();
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

	public void on_objects_created(Guid?[] object_ids, uint32 flags)
	{
		if ((flags& ActionTypeFlags.FROM_SERVER) == 0) {
			StringBuilder sb = new StringBuilder();
			_level.generate_spawn_objects(sb, object_ids);
			if (sb.len > 0) {
				_runtime.send_script(sb.str);
				_editor_viewport.frame();
			}
		}

		Guid last = object_ids[object_ids.length - 1];

		_objects_tree.set_object(_unit_id); // Force update the tree.

		if (_database.object_type(last) == OBJECT_TYPE_UNIT) {
			_database_editor.selection_set({ last }); // Select the objects just created.
			_properties_view.set_objects({ last });
		} else if ((_database.type_flags(StringId64(_database.object_type(last))) & ObjectTypeFlags.UNIT_COMPONENT) != 0) {
			_database_editor.selection_set({ _database.owner(last) });
			_properties_view.set_objects({ _database.owner(last) });
		}

		update_window_title();
	}

	public void on_objects_destroyed(Guid?[] object_ids, uint32 flags = 0)
	{
		_objects_tree.set_object(_unit_id); // Force update the tree.

		Guid last = object_ids[object_ids.length - 1];

		if (_database.object_type(last) == OBJECT_TYPE_UNIT) {
			// Select the root object which must always exits.
			_database_editor.selection_set({ _unit_id });
			_properties_view.set_objects({ _unit_id });
		} else if ((_database.type_flags(StringId64(_database.object_type(last))) & ObjectTypeFlags.UNIT_COMPONENT) != 0) {
			Guid owner_id = _database.owner(last);

			if (_database.is_alive(owner_id)) {
				_database_editor.selection_set({ owner_id });
				_properties_view.set_objects({ owner_id });
			} else {
				// Select the root object which must always exits.
				_database_editor.selection_set({ _unit_id });
				_properties_view.set_objects({ _unit_id });
			}
		}

		update_window_title();

		if ((flags& ActionTypeFlags.FROM_SERVER) == 0) {
			StringBuilder sb = new StringBuilder();
			_level.generate_destroy_objects(sb, object_ids);
			if (sb.len > 0) {
				_runtime.send_script(sb.str);
				_editor_viewport.frame();
			}
		}
	}

	public void on_objects_changed(Guid?[] object_ids, uint32 flags = 0)
	{
		if ((flags& ActionTypeFlags.FROM_SERVER) == 0) {
			StringBuilder sb = new StringBuilder();
			_level.generate_change_objects(sb, object_ids);
			if (sb.len > 0) {
				_runtime.send_script(sb.str);
				_editor_viewport.frame();
			}
		}

		Guid last_changed = object_ids[object_ids.length - 1];

		_objects_tree.set_object(_unit_id); // Force update the tree.
		_database_editor.selection_set({ last_changed });
		update_window_title();
	}

	public void do_set_unit(string unit_name)
	{
		reset();

		_level.load(LEVEL_EMPTY);

		if (Unit.load_unit(out _unit_id, _database, unit_name) != 0)
			return;

		_unit_name = unit_name;
		Unit unit = Unit(_database, _unit_id);
		UndoRedo undo_redo = _database.disable_undo();
		unit.set_local_position(VECTOR3_ZERO);
		_database.restore_undo(undo_redo);

		_objects_tree.set_object(_unit_id);
		_database_editor.selection_set({ _unit_id });
		update_window_title();
		send();
	}

	public void set_unit(string unit_name)
	{
		if (_unit_name == unit_name)
			return;

		if (!_database.changed()) {
			this.do_set_unit(unit_name);
		} else {
			Gtk.Dialog dlg = new_resource_changed_dialog(this, _unit_name);
			dlg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.NO) {
						this.do_set_unit(unit_name);
					} else if (response_id == Gtk.ResponseType.YES) {
						this.save();
						this.do_set_unit(unit_name);
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
