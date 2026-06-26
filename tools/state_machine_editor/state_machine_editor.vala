/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public delegate void StateMachineEditorSaveCallback();

public class StateMachineEditor : Gtk.ApplicationWindow
{
	public LevelEditorApplication _application;
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
	public Gtk.Box _events_box;
	public Gtk.Box _variables_box;
	public Gee.HashMap<string, Gtk.Button> _event_buttons;
	public Gee.HashMap<string, Gtk.Box> _variable_sliders;
	public InputResource _unit;

	public string _state_machine_name;
	public string? _state_machine_path;
	public Guid _state_machine_id;

	public signal void saved();

	public StateMachineEditor(LevelEditorApplication application
		, DataCompiler data_compiler
		, Project project
		, string boot_dir
		, string console_addr
		, uint16 console_port
		, uint32 undo_redo_size
		)
	{
		Object(application: application);

		_application = application;
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

		_editor_viewport = new EditorViewport("state_machine_editor"
			, data_compiler
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
		_runtime.message_received.connect(on_message_received);

		_statusbar = new Statusbar();

		_cancel = new Gtk.Button.with_label(_("Cancel"));
		_cancel.clicked.connect(() => {
				close();
			});
		_save = new Gtk.Button.with_label(_("Save & Reload"));
		_save.get_style_context().add_class("suggested-action");
		_save.clicked.connect(() => {
				save();
			});

		_header_bar = new Gtk.HeaderBar();
		_header_bar.title = _("State Machine Editor");
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

		mi = new GLib.MenuItem(_("Edit"), null);
		mi.set_submenu(make_database_editor_menu());
		menu.append_item(mi);

		mi = new GLib.MenuItem(_("Camera"), null);
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

		_events_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_events_box.halign = Gtk.Align.END;
		_events_box.valign = Gtk.Align.START;
		_events_box.margin_top = 8;
		_events_box.margin_end = 8;
		_editor_viewport._overlay.add_overlay(_events_box);

		_variables_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_variables_box.halign = Gtk.Align.END;
		_variables_box.valign = Gtk.Align.START;
		_variables_box.margin_top = 200;
		_variables_box.margin_end = 8;
		_variables_box.set_size_request(200, -1);
		_editor_viewport._overlay.add_overlay(_variables_box);

		_event_buttons = new Gee.HashMap<string, Gtk.Button>();
		_variable_sliders = new Gee.HashMap<string, Gtk.Box>();

		_unit = new InputResource(OBJECT_TYPE_UNIT, _database);
		_unit.halign = Gtk.Align.START;
		_unit.valign = Gtk.Align.START;
		_unit.margin_top = 8;
		_unit.margin_start = 8;
		_unit.value_changed.connect(on_unit_value_changed);
		_editor_viewport._overlay.add_overlay(_unit);

		reset();

		_editor_viewport.restart_runtime.begin();
	}

	public void update_window_title()
	{
		string title = "";

		if (_state_machine_name != null) {
			if (_database.changed())
				title += " • ";

			title += (_state_machine_name == LEVEL_EMPTY) ? _("untitled") : _state_machine_name;
			title += " - ";
		}

		title += CROWN_EDITOR_NAME;

		if (this.title != title)
			this.title = title;
	}

	public void send()
	{
		if (_state_machine_id == GUID_ZERO || !_database.has_object(_state_machine_id) || !_database.is_alive(_state_machine_id) || _unit.value == null)
			return;

		_runtime.send_script(StateMachineEditorApi.set_unit(_unit.value));
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
		_database.reset();
		_state_machine_name = "";
		_state_machine_path = null;
		_state_machine_id = GUID_ZERO;
		_unit.value = null;
		destroy_event_buttons();
		destroy_variable_sliders();
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
		_objects_properties.set_object(GUID_ZERO);
		update_window_title();
	}

	public bool do_save(string path)
	{
		if (_state_machine_id == GUID_ZERO)
			return false;

		string display_name      = _state_machine_name;
		string? resource_name    = ResourceId.name(ResourceId.normalize(_database._project.resource_filename(path)));
		if (resource_name != null)
			display_name = resource_name;

		if (_database.save(path, _state_machine_id) != 0) {
			_state_machine_path = null;
			Gtk.MessageDialog md = new Gtk.MessageDialog(this
				, Gtk.DialogFlags.MODAL
				, Gtk.MessageType.WARNING
				, Gtk.ButtonsType.NONE
				, _("Unable to save state machine '%s'").printf(display_name)
				);
			md.add_button(_("_Ok"), Gtk.ResponseType.OK);
			md.set_default_response(Gtk.ResponseType.OK);
			md.response.connect(() => { md.destroy(); });
			md.show_all();
			update_window_title();
			return false;
		}

		_state_machine_name = display_name;
		_state_machine_path = path;
		saved();
		update_window_title();
		return true;
	}

	public void save_as(string? filename, owned StateMachineEditorSaveCallback? on_save_success = null)
	{
		if (filename != null) {
			if (do_save(filename) && on_save_success != null)
				on_save_success();
			return;
		}

		string current_name = _state_machine_name == "" ? "" : GLib.Path.get_basename(_state_machine_name);
		SaveResourceDialog srd = new SaveResourceDialog(_("Save As...")
			, this
			, OBJECT_TYPE_STATE_MACHINE
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
		srd.show_all();
	}

	public void save(owned StateMachineEditorSaveCallback? on_save_success = null)
	{
		if (_state_machine_id == GUID_ZERO)
			return;

		save_as(_state_machine_path, (owned)on_save_success);
	}

	public void on_objects_created(Guid?[] object_ids, uint32 flags)
	{
		Guid last_created = object_ids[object_ids.length - 1];

		_objects_tree.set_object(_state_machine_id); // Force update the tree.
		_database_editor.selection_set({ last_created }); // Select the root object which must always exits.
		create_event_buttons();
		create_variable_sliders();
		update_window_title();
	}

	public void on_objects_destroyed(Guid?[] object_ids, uint32 flags = 0)
	{
		_objects_tree.set_object(_state_machine_id); // Force update the tree.
		_database_editor.selection_set({ _state_machine_id }); // Select the root object which must always exits.
		create_event_buttons();
		create_variable_sliders();
		update_window_title();
	}

	public void on_objects_changed(Guid?[] object_ids, uint32 flags = 0)
	{
		Guid last_changed = object_ids[object_ids.length - 1];

		_objects_tree.set_object(_state_machine_id); // Force update the tree.
		_database_editor.selection_set({ last_changed });
		create_event_buttons();
		create_variable_sliders();
		update_window_title();
	}

	public void do_set_state_machine(string state_machine_name)
	{
		reset();

		string resource_path = ResourceId.path(OBJECT_TYPE_STATE_MACHINE, state_machine_name);
		string path = _database._project.absolute_path(resource_path);

		_state_machine_name = state_machine_name;
		_state_machine_path = path;

		LoadError err = _database.load_from_path(out _state_machine_id, path, resource_path);
		if (err == LoadError.NOT_FOUND) {
			UndoRedo? undo_redo = _database.disable_undo();
			_state_machine_id = Guid.new_guid();
			StateMachineResource.sprite(_database, _state_machine_id, null);
			_database.set_name(_state_machine_id, GLib.Path.get_basename(state_machine_name));
			_database.restore_undo(undo_redo);
		} else if (err != LoadError.SUCCESS) {
			return;
		}

		_unit.value = state_machine_name;
		_objects_tree.set_object(_state_machine_id);
		_database_editor.selection_set({ _state_machine_id });
		create_event_buttons();
		create_variable_sliders();
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
						this.save(() => {
							do_set_state_machine(state_machine_name);
						});
					}
					dlg.destroy();
				});
			dlg.show_all();
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

	public bool on_close_request(Gdk.EventAny event)
	{
		if (!_database.changed()) {
			close_and_unload();
			return Gdk.EVENT_STOP;
		}

		Gtk.Dialog dlg = new_resource_changed_dialog(this, _state_machine_name);
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
		dlg.show_all();
		return Gdk.EVENT_STOP;
	}

	public void destroy_event_buttons()
	{
		foreach (var button in _event_buttons)
			button.value.destroy();
		_event_buttons.clear();
	}

	public void create_event_buttons()
	{
		destroy_event_buttons();

		Guid?[] all_states = _database.all_objects_of_type(StringId64(OBJECT_TYPE_STATE_MACHINE_NODE));
		Guid?[] all_transitions = _database.all_objects_of_type(StringId64(OBJECT_TYPE_NODE_TRANSITION));

		foreach (var state in all_states) {
			if (!_database.is_subobject_of(state, _state_machine_id, "states"))
				continue;

			foreach (var transition in all_transitions) {
				if (!_database.is_subobject_of(transition, state, "transitions"))
					continue;

				string event_name = _database.get_string(transition, "event");
				if (!_event_buttons.has_key(event_name))
					_event_buttons.set(event_name, create_trigger_event_button(transition, event_name));
			}
		}

		foreach (var button in _event_buttons)
			_events_box.pack_start(button.value);
		_events_box.show_all();
	}

	public Gtk.Button create_trigger_event_button(Guid object_id, string event_name)
	{
		Gtk.Button button = new Gtk.Button.with_label(event_name);
		button.clicked.connect(() => {
				_runtime.send_script(StateMachineEditorApi.trigger_animation_event(event_name));
			});
		return button;
	}

	public void destroy_variable_sliders()
	{
		foreach (var slider in _variable_sliders)
			slider.value.destroy();
		_variable_sliders.clear();
	}

	public void create_variable_sliders()
	{
		destroy_variable_sliders();

		Guid?[] all_variables = _database.all_objects_of_type(StringId64(OBJECT_TYPE_STATE_MACHINE_VARIABLE));

		foreach (var variable in all_variables) {
			if (!_database.is_subobject_of(variable, _state_machine_id, "variables"))
				continue;

			string variable_name = _database.get_string(variable, "name");
			if (!_variable_sliders.has_key(variable_name)) {
				double min_value = _database.get_double(variable, "min");
				double max_value = _database.get_double(variable, "max");
				double current_value = _database.get_double(variable, "value");

				_variable_sliders.set(variable_name, create_variable_slider(variable, variable_name, min_value, max_value, current_value));
			}
		}

		foreach (var slider in _variable_sliders)
			_variables_box.pack_start(slider.value);
		_variables_box.show_all();
	}

	public Gtk.Box create_variable_slider(Guid object_id, string variable_name, double min_value, double max_value, double current_value)
	{
		// Create label.
		Gtk.Label label = new Gtk.Label(variable_name);
		label.ellipsize = Pango.EllipsizeMode.END;
		label.halign = Gtk.Align.START;
		label.set_size_request(1, -1);

		// Create slider.
		Gtk.Adjustment adjustment = new Gtk.Adjustment(current_value, min_value, max_value, 0.01, 0.1, 0.0);
		Gtk.Scale scale = new Gtk.Scale(Gtk.Orientation.HORIZONTAL, adjustment);
		scale.set_draw_value(true);
		scale.set_value_pos(Gtk.PositionType.RIGHT);
		scale.set_digits(2);
		scale.set_size_request(150, -1);

		scale.value_changed.connect(() => {
				double value = scale.get_value();
				_runtime.send_script(StateMachineEditorApi.set_variable(variable_name, value));
			});

		Gtk.Box slider_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
		slider_box.pack_start(label, false, false, 0);
		slider_box.pack_start(scale, true, true, 0);
		slider_box.margin_bottom = 4;
		slider_box.set_size_request(180, -1);

		return slider_box;
	}

	public void on_unit_value_changed()
	{
		send();
	}
}

} /* namespace Crown */
