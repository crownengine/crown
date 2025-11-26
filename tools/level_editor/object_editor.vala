/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
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

	public string _object_name;
	public Guid _object_id;

	public signal void saved();

	public ObjectEditor(Gtk.Application application
		, Project project
		, uint32 undo_redo_size
		)
	{
		Object(application: application);

		_object_id = GUID_ZERO;

		_database_editor = new DatabaseEditor(project, undo_redo_size);
		_database_editor.undo.connect(on_undo);
		_database_editor.redo.connect(on_redo);
		this.insert_action_group("database", _database_editor._action_group);

		_database = _database_editor._database;
		_database.objects_created.connect(on_objects_created);
		_database.objects_destroyed.connect(on_objects_destroyed);
		_database.objects_changed.connect(on_objects_changed);

		_objects_tree = new ObjectTree(_database);
		_objects_tree.selection_changed.connect(on_objects_tree_selection_changed);

		_objects_properties = new ObjectProperties(_database);

		_database_editor.load_types();

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

	public void save()
	{
		assert(_object_id != GUID_ZERO);

		if (_database.save(_database._project.absolute_path(_object_name) + "." + OBJECT_TYPE_STATE_MACHINE, _object_id) == 0)
			saved();
	}

	public void on_objects_created(Guid?[] object_ids, uint32 flags)
	{
		Guid last_created = object_ids[object_ids.length - 1];

		_objects_tree.set_object(_object_id); // Force update the tree.
		_objects_tree.select_objects({ last_created }); // Select the objects just created.
		update_window_title();
	}

	public void on_objects_destroyed(Guid?[] object_ids, uint32 flags = 0)
	{
		_objects_tree.set_object(_object_id); // Force update the tree.
		_objects_tree.select_objects({ _object_id }); // Select the root object which must always exits.
		update_window_title();
	}

	public void on_objects_changed(Guid?[] object_ids, uint32 flags = 0)
	{
		_objects_tree.on_tree_selection_changed(); // Force update any tree listener.
		update_window_title();
	}

	public void set_object(string type, string name)
	{
		string resource_path = ResourceId.path(type, name);
		string path = _database._project.absolute_path(resource_path);

		if (_database.load_from_path(out _object_id, path, resource_path) != 0)
			return;

		_object_name = name;
		_objects_tree.set_object(_object_id);
		update_window_title();
	}

	public void on_objects_tree_selection_changed(Guid object_id, ObjectTree.ItemType item_type)
	{
		if (item_type == ObjectTree.ItemType.OBJECT)
			_objects_properties.set_object(object_id);
		else
			_objects_properties.set_object(GUID_ZERO);
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
		if (!_database.changed()) {
			this.hide();
		} else {
			Gtk.Dialog dlg = new_resource_changed_dialog(this, _object_name);
			dlg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.NO) {
						this.hide();
					} else if (response_id == Gtk.ResponseType.YES) {
						this.save();
						this.hide();
					}
					dlg.destroy();
				});
			dlg.show_all();
		}

		return Gdk.EVENT_STOP;
	}
}

} /* namespace Crown */
