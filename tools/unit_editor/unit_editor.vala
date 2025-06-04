/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public delegate void UnitEditorSaveCallback();

public class UnitEditor : Gtk.ApplicationWindow
{
	public LevelEditorApplication _application;
	public DatabaseEditor _database_editor;
	public Database _database;
	public EditorViewport _editor_viewport;
	public RuntimeInstance _runtime;
	public ObjectTree _objects_tree;
	public PropertiesView _properties_view;
	public Level _level;
	public Statusbar _statusbar;
	public PreferencesDialog _preferences;

	public Gtk.Paned _paned_object;
	public Gtk.Paned _paned_inspector;
	public Gtk.Button _save;
	public Gtk.HeaderBar _header_bar;
	public Gtk.Box _box;

	public string _unit_name;
	public string? _unit_path;
	public Guid _unit_id;
	public string _renderer;

	public signal void saved();

	public UnitEditor(LevelEditorApplication application
		, DataCompiler data_compiler
		, Project project
		, string boot_dir
		, string console_addr
		, uint16 console_port
		, PreferencesDialog preferences
		)
	{
		Object(application: application);
		_application = application;
		_preferences = preferences;

		_database = new Database(project);
		_database_editor = new DatabaseEditor((uint32)preferences._undo_redo_max_size.value * 1024 * 1024, _database);
		_database_editor.undo.connect(on_undo);
		_database_editor.redo.connect(on_redo);
		this.insert_action_group("database", _database_editor._action_group);

		_database.objects_created.connect(on_objects_created);
		_database.objects_destroyed.connect(on_objects_destroyed);
		_database.objects_changed.connect(on_objects_changed);
		_database.object_type_added.connect(on_object_type_added);

		_objects_tree = new ObjectTree(_database_editor);
		_properties_view = new PropertiesView(_database_editor);
		_database_editor.load_types();
		_properties_view.register_object_type(OBJECT_TYPE_UNIT, new UnitView(_database));

		_editor_viewport = new EditorViewport("unit_editor"
			, data_compiler
			, _database_editor
			, project
			, boot_dir
			, console_addr
			, console_port
			, preferences
			);
		this.insert_action_group("viewport", _editor_viewport._action_group);
		_runtime = _editor_viewport._runtime;
		_runtime.connected.connect(on_editor_connected);
		_runtime.disconnected.connect(on_editor_disconnected);
		_runtime.disconnected_unexpected.connect(on_editor_disconnected_unexpected);
		_runtime.message_received.connect(on_message_received);

		_level = new Level(_database, _runtime);

		_statusbar = new Statusbar();

		_save = new Gtk.Button.with_label(_("Save & Reload"));
#if CROWN_GTK3
		_save.get_style_context().add_class("suggested-action");
		_save.clicked.connect(() => {
				save();
			});
#else
		_save.add_css_class("suggested-action");
		_save.clicked.connect(() => save());
#endif

		_header_bar = new Gtk.HeaderBar();
#if CROWN_GTK3
		_header_bar.title = _("Unit Editor");
		_header_bar.show_close_button = true;
#else
		_header_bar.show_title_buttons = true;
#endif
		_header_bar.pack_end(_save);
		this.title = _("Unit Editor");

		_paned_inspector = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
#if CROWN_GTK3
		_paned_inspector.pack1(_editor_viewport, false, false);
		_paned_inspector.pack2(_properties_view, false, false);
#else
		_paned_inspector.set_start_child(_editor_viewport);
		_paned_inspector.set_end_child(_properties_view);
		_paned_inspector.resize_start_child = true;
		_paned_inspector.shrink_start_child = false;
		_paned_inspector.resize_end_child = false;
		_paned_inspector.shrink_end_child = false;
#endif

		_paned_object = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
#if CROWN_GTK3
		_paned_object.pack1(_objects_tree, false, false);
		_paned_object.pack2(_paned_inspector, true, false);
#else
		_paned_object.set_start_child(_objects_tree);
		_paned_object.set_end_child(_paned_inspector);
		_paned_object.resize_start_child = false;
		_paned_object.shrink_start_child = false;
		_paned_object.resize_end_child = true;
		_paned_object.shrink_end_child = false;
		_paned_object.vexpand = true;
#endif

		this.set_titlebar(_header_bar);
		this.set_default_size(1280, 720);
		int win_w;
		int win_h;
#if CROWN_GTK3
		this.get_size(out win_w, out win_h);
#else
		this.get_default_size(out win_w, out win_h);
#endif
		int object_panel_width = (int)(win_w * (220.0 / 1280.0));
		int inspector_panel_width = int.max((int)(win_w * (375.0 / 2560.0)), 232);
		_paned_object.set_position(object_panel_width);
		_paned_inspector.set_position(win_w - object_panel_width - inspector_panel_width);

		GLib.Menu menu = new GLib.Menu();
		GLib.MenuItem mi = null;

		mi = new GLib.MenuItem(_("Edit"), null);
		mi.set_submenu(make_database_editor_menu());
		menu.append_item(mi);

		mi = new GLib.MenuItem(_("Camera"), null);
		mi.set_submenu(make_camera_view_menu());
		menu.append_item(mi);

#if CROWN_GTK3
		Gtk.MenuBar menubar = new Gtk.MenuBar.from_model(menu);
		_header_bar.pack_start(menubar);
#else
		this.show_menubar = false;
		Gtk.PopoverMenuBar menubar = new Gtk.PopoverMenuBar.from_model(menu);
#endif

		_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
#if CROWN_GTK3
		_box.pack_start(_paned_object);
		_box.pack_start(_statusbar, false);

		this.delete_event.connect(on_close_request);
		this.add(_box);
#else
		_box.append(menubar);
		_box.append(_paned_object);
		_box.append(_statusbar);

		this.close_request.connect(on_close_request);
		this.set_child(_box);
#endif

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
		if (_unit_id == GUID_ZERO || !_database.has_object(_unit_id) || !_database.is_alive(_unit_id))
			return;

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

	public void on_message_received(RuntimeInstance ri, ConsoleClient client, uint8[] json)
	{
		_application.on_message_received(ri, client, json);
	}

	public void reset()
	{
		_level.reset();
		_unit_name = "";
		_unit_path = null;
		_unit_id = GUID_ZERO;
	}

	public void unload()
	{
		if (_runtime.is_connected()) {
			_runtime.send_script(LevelEditorApi.reset());
			_editor_viewport.frame();
		}

		reset();
		_database_editor.selection_read({});
		_objects_tree.set_object(GUID_ZERO);
		_properties_view.set_objects({});
		update_window_title();
	}

	public bool do_save(string path)
	{
		if (_unit_id == GUID_ZERO)
			return false;

		string display_name      = _unit_name;
		string? resource_name    = ResourceId.name(ResourceId.normalize(_database._project.resource_filename(path)));
		if (resource_name != null)
			display_name = resource_name;

		if (_database.save(path, _unit_id) != 0) {
			_unit_path = null;
			Gtk.MessageDialog md = new Gtk.MessageDialog(this
				, Gtk.DialogFlags.MODAL
				, Gtk.MessageType.WARNING
				, Gtk.ButtonsType.NONE
				, _("Unable to save unit '%s'").printf(display_name)
				);
			md.add_button(_("_Ok"), Gtk.ResponseType.OK);
			md.set_default_response(Gtk.ResponseType.OK);
			md.response.connect(() => { md.destroy(); });
#if CROWN_GTK3
			md.show_all();
#else
			md.show();
#endif
			update_window_title();
			return false;
		}

		_unit_name = display_name;
		_unit_path = path;
		saved();
		update_window_title();
		return true;
	}

	public void save_as(string? filename, owned UnitEditorSaveCallback? on_save_success = null)
	{
		if (filename != null) {
			if (do_save(filename) && on_save_success != null)
				on_save_success();
			return;
		}

		string current_name = _unit_name == "" ? "" : GLib.Path.get_basename(_unit_name);
		SaveResourceDialog srd = new SaveResourceDialog(_("Save As...")
			, this
			, OBJECT_TYPE_UNIT
			, current_name
			, _database._project
			);
		srd.safer_response.connect((response_id, path) => {
				if (response_id == Gtk.ResponseType.ACCEPT && path != null) {
					if (do_save(path) && on_save_success != null)
						on_save_success();
				}
				srd.destroy();
			});
#if CROWN_GTK3
		srd.show_all();
#else
		srd.show();
#endif
	}

	public void save(owned UnitEditorSaveCallback? on_save_success = null)
	{
		if (_unit_id == GUID_ZERO)
			return;

		save_as(_unit_path, (owned)on_save_success);
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
			bool respawn_objects = (flags& ActionTypeFlags.RESPAWN_OBJECTS) != 0;
			_level.generate_change_objects(sb, object_ids, respawn_objects);
			if (sb.len > 0) {
				_runtime.send_script(sb.str);
				if (respawn_objects)
					_runtime.send_script(LevelEditorApi.selection_set(_database_editor._selection.data));
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

		string path = _database._project.absolute_path(unit_name + "." + OBJECT_TYPE_UNIT);

		_unit_name = unit_name;
		_unit_path = path;

		LoadError err = Unit.load_unit(out _unit_id, _database, unit_name);
		if (err == LoadError.NOT_FOUND) {
			UndoRedo? undo_redo = _database.disable_undo();
			_unit_id = Guid.new_guid();
			Unit new_unit = Unit(_database, _unit_id);
			new_unit.create_empty();
			_database.set_name(_unit_id, GLib.Path.get_basename(unit_name));
			_database.restore_undo(undo_redo);
		} else if (err != LoadError.SUCCESS) {
			return;
		}

		Unit unit = Unit(_database, _unit_id);
		UndoRedo? undo_redo = _database.disable_undo();
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
						this.save(() => {
							do_set_unit(unit_name);
						});
					}
					dlg.destroy();
				});
#if CROWN_GTK3
			dlg.show_all();
#else
			dlg.show();
#endif
		}
	}

	public void on_undo(int action_id)
	{
		_statusbar.set_temporary_message(_("Undo: %s").printf(_(ActionNames[action_id])));
	}

	public void on_redo(int action_id)
	{
		_statusbar.set_temporary_message(_("Redo: %s").printf(_(ActionNames[action_id])));
	}

	public void close_and_unload()
	{
		this.hide();
		unload();
	}

	public bool handle_close_request()
	{
		if (!_database.changed()) {
			close_and_unload();
			return Gdk.EVENT_STOP;
		}

		Gtk.Dialog dlg = new_resource_changed_dialog(this, _unit_name);
		dlg.response.connect((response_id) => {
				if (response_id == Gtk.ResponseType.NO) {
					close_and_unload();
				} else if (response_id == Gtk.ResponseType.YES) {
					save(() => {
						close_and_unload();
					});
				}
				dlg.destroy();
			});
#if CROWN_GTK3
		dlg.show_all();
#else
		dlg.present();
#endif
		return Gdk.EVENT_STOP;
	}

#if CROWN_GTK3
	public bool on_close_request(Gdk.EventAny event)
	{
		return handle_close_request();
	}
#endif

#if !CROWN_GTK3
	public bool on_close_request()
	{
		return handle_close_request();
	}
#endif
}

} /* namespace Crown */
