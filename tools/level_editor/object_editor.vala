/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public delegate void ObjectEditorSaveCallback();

public class ObjectEditor : Gtk.ApplicationWindow
{
	public DatabaseEditor _database_editor;
	public Database _database;
	public ObjectTree _objects_tree;
	public ObjectProperties _objects_properties;
	public Gtk.Paned _paned;
	public Statusbar _statusbar;

	public Gtk.Button _cancel;
	public Gtk.Button _save;
	public Gtk.HeaderBar _header_bar;
	public Gtk.Box _box;

	public string _object_type;
	public string _object_name;
	public string? _object_path;
	public Guid _object_id;

	public signal void saved();

	public ObjectEditor(Gtk.Application application
		, Project project
		, uint32 undo_redo_size
		)
	{
		Object(application: application);

		_object_id = GUID_ZERO;

		_database = new Database(project);
		_database_editor = new DatabaseEditor(undo_redo_size, _database);
		_database_editor.undo.connect(on_undo);
		_database_editor.redo.connect(on_redo);
		this.insert_action_group("database", _database_editor._action_group);

		_database.objects_created.connect(on_objects_created);
		_database.objects_destroyed.connect(on_objects_destroyed);
		_database.objects_changed.connect(on_objects_changed);

		_objects_tree = new ObjectTree(_database_editor);
		_objects_properties = new ObjectProperties(_database_editor);

		_database_editor.load_types();

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
		_header_bar.title = _("Object Editor");
		_header_bar.show_close_button = true;
		_header_bar.pack_start(_cancel);
		_header_bar.pack_end(_save);

		_paned = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
		_paned.pack1(_objects_tree, true, false);
		_paned.pack2(_objects_properties, true, false);

		this.set_titlebar(_header_bar);
		this.set_size_request(1000, 600);

		int win_w;
		int win_h;
		this.get_size(out win_w, out win_h);
		_paned.set_position(win_w/2);

		GLib.Menu menu = new GLib.Menu();
		GLib.MenuItem mi = null;

		mi = new GLib.MenuItem(_("Edit"), null);
		mi.set_submenu(make_database_editor_menu());
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
	}

	public void update_window_title()
	{
		string title = "";

		if (_database.changed())
			title += " • ";

		title += _object_id == GUID_ZERO ? "unnamed" : _object_name;
		title += " - ";
		title += CROWN_EDITOR_NAME;

		if (this.title != title)
			this.title = title;
	}

	public void reset()
	{
		_database.reset();
		_object_type = "";
		_object_name = "";
		_object_path = null;
		_object_id = GUID_ZERO;
	}

	public void unload()
	{
		reset();
		_database_editor.selection_read({});
		_objects_tree.set_object(GUID_ZERO);
		_objects_properties.set_object(GUID_ZERO);
		update_window_title();
	}

	public bool do_save(string path)
	{
		if (_object_id == GUID_ZERO)
			return false;

		string display_name      = _object_name;
		string? resource_name    = ResourceId.name(ResourceId.normalize(_database._project.resource_filename(path)));
		if (resource_name != null)
			display_name = resource_name;

		if (_database.save(path, _object_id) != 0) {
			_object_path = null;
			Gtk.MessageDialog md = new Gtk.MessageDialog(this
				, Gtk.DialogFlags.MODAL
				, Gtk.MessageType.WARNING
				, Gtk.ButtonsType.NONE
				, _("Unable to save object '%s'").printf(display_name)
				);
			md.add_button(_("_Ok"), Gtk.ResponseType.OK);
			md.set_default_response(Gtk.ResponseType.OK);
			md.response.connect(() => { md.destroy(); });
			md.show_all();
			update_window_title();
			return false;
		}

		_object_name = display_name;
		_object_path = path;
		saved();
		update_window_title();
		return true;
	}

	public void save_as(string? filename, owned ObjectEditorSaveCallback? on_save_success = null)
	{
		if (filename != null) {
			if (do_save(filename) && on_save_success != null)
				on_save_success();
			return;
		}

		string current_name = _object_name == "" ? "" : GLib.Path.get_basename(_object_name);
		SaveResourceDialog srd = new SaveResourceDialog(_("Save As...")
			, this
			, _object_type
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

	public void save(owned ObjectEditorSaveCallback? on_save_success = null)
	{
		if (_object_id == GUID_ZERO)
			return;

		save_as(_object_path, (owned)on_save_success);
	}

	public void on_objects_created(Guid?[] object_ids, uint32 flags)
	{
		Guid last_created = object_ids[object_ids.length - 1];

		_objects_tree.set_object(_object_id); // Force update the tree.
		_database_editor.selection_set({ last_created }); // Select the objects just created.
		update_window_title();
	}

	public void on_objects_destroyed(Guid?[] object_ids, uint32 flags = 0)
	{
		_objects_tree.set_object(_object_id); // Force update the tree.
		_database_editor.selection_set({ _object_id }); // Select the root object which must always exits.
		update_window_title();
	}

	public void on_objects_changed(Guid?[] object_ids, uint32 flags = 0)
	{
		Guid last_changed = object_ids[object_ids.length - 1];

		_objects_tree.set_object(_object_id); // Force update the tree.
		_database_editor.selection_set({ last_changed });
		update_window_title();
	}

	public void do_set_object(string type, string name)
	{
		reset();

		string resource_path = ResourceId.path(type, name);
		string path = _database._project.absolute_path(resource_path);

		_object_type = type;
		_object_name = name;
		_object_path = path;

		LoadError err = _database.load_from_path(out _object_id, path, resource_path);
		if (err == LoadError.NOT_FOUND) {
			UndoRedo? undo_redo = _database.disable_undo();
			_object_id = Guid.new_guid();
			_database.create(_object_id, type);
			_database.set_name(_object_id, GLib.Path.get_basename(name));
			_database.restore_undo(undo_redo);
		} else if (err != LoadError.SUCCESS) {
			return;
		}

		_objects_tree.set_object(_object_id);
		_database_editor.selection_set({ _object_id });
		update_window_title();
	}

	public void set_object(string type, string name)
	{
		if (_object_type == type && _object_name == name)
			return;

		if (!_database.changed()) {
			this.do_set_object(type, name);
		} else {
			Gtk.Dialog dlg = new_resource_changed_dialog(this, _object_name);
			dlg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.NO) {
						this.do_set_object(type, name);
					} else if (response_id == Gtk.ResponseType.YES) {
						this.save(() => {
							do_set_object(type, name);
						});
					}
					dlg.destroy();
				});
			dlg.show_all();
		}
	}

	public void on_objects_tree_selection_changed(Guid?[] objects)
	{
		_database_editor.selection_read(objects);
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

		Gtk.Dialog dlg = new_resource_changed_dialog(this, _object_name);
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
}

} /* namespace Crown */
