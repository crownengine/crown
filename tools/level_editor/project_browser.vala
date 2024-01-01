/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;
using Gee;

namespace Crown
{
private Gtk.Menu? menu_create(string type, string name)
{
	Gtk.Menu? menu;

	if (type == "<folder>") {
		menu = new Gtk.Menu();

		Gtk.MenuItem mi;

		mi = new Gtk.MenuItem.with_label("Import...");
		mi.activate.connect(() => {
				GLib.Application.get_default().activate_action("import", new GLib.Variant.string((string)name));
			});
		menu.add(mi);

		mi = new Gtk.SeparatorMenuItem();
		menu.add(mi);

		mi = new Gtk.MenuItem.with_label("New Script...");
		mi.activate.connect(() => {
				Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Script Name"
					, ((Gtk.Application)GLib.Application.get_default()).active_window
					, DialogFlags.MODAL
					, "Cancel"
					, ResponseType.CANCEL
					, "Ok"
					, ResponseType.OK
					, null
					);

				EntryText sb = new EntryText();
				sb.activate.connect(() => { dg.response(ResponseType.OK); });
				dg.get_content_area().add(sb);
				dg.skip_taskbar_hint = true;
				dg.show_all();

				if (dg.run() == (int)ResponseType.OK) {
					if (sb.text.strip() == "") {
						dg.destroy();
						return;
					}

					var tuple = new GLib.Variant.tuple({(string)name, sb.text, true});
					GLib.Application.get_default().activate_action("create-script", tuple);
				}

				dg.destroy();
			});
		menu.add(mi);

		mi = new Gtk.MenuItem.with_label("New Script (Unit)...");
		mi.activate.connect(() => {
				Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Script Name"
					, ((Gtk.Application)GLib.Application.get_default()).active_window
					, DialogFlags.MODAL
					, "Cancel"
					, ResponseType.CANCEL
					, "Ok"
					, ResponseType.OK
					, null
					);

				EntryText sb = new EntryText();
				sb.activate.connect(() => { dg.response(ResponseType.OK); });
				dg.get_content_area().add(sb);
				dg.skip_taskbar_hint = true;
				dg.show_all();

				if (dg.run() == (int)ResponseType.OK) {
					if (sb.text.strip() == "") {
						dg.destroy();
						return;
					}

					var tuple = new GLib.Variant.tuple({(string)name, sb.text, false});
					GLib.Application.get_default().activate_action("create-script", tuple);
				}

				dg.destroy();
			});
		menu.add(mi);

		mi = new Gtk.SeparatorMenuItem();
		menu.add(mi);

		mi = new Gtk.MenuItem.with_label("New Unit...");
		mi.activate.connect(() => {
				Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Unit Name"
					, ((Gtk.Application)GLib.Application.get_default()).active_window
					, DialogFlags.MODAL
					, "Cancel"
					, ResponseType.CANCEL
					, "Ok"
					, ResponseType.OK
					, null
					);

				EntryText sb = new EntryText();
				sb.activate.connect(() => { dg.response(ResponseType.OK); });
				dg.get_content_area().add(sb);
				dg.skip_taskbar_hint = true;
				dg.show_all();

				if (dg.run() == (int)ResponseType.OK) {
					if (sb.text.strip() == "") {
						dg.destroy();
						return;
					}
				}

				var tuple = new GLib.Variant.tuple({(string)name, sb.text});
				GLib.Application.get_default().activate_action("create-unit", tuple);

				dg.destroy();
			});
		menu.add(mi);

		mi = new Gtk.SeparatorMenuItem();
		menu.add(mi);

		mi = new Gtk.MenuItem.with_label("New Folder...");
		mi.activate.connect(() => {
				Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Folder Name"
					, ((Gtk.Application)GLib.Application.get_default()).active_window
					, DialogFlags.MODAL
					, "Cancel"
					, ResponseType.CANCEL
					, "Ok"
					, ResponseType.OK
					, null
					);

				EntryText sb = new EntryText();
				sb.activate.connect(() => { dg.response(ResponseType.OK); });
				dg.get_content_area().add(sb);
				dg.skip_taskbar_hint = true;
				dg.show_all();

				if (dg.run() == (int)ResponseType.OK) {
					if (sb.text.strip() == "") {
						dg.destroy();
						return;
					}

					var tuple = new GLib.Variant.tuple({(string)name, sb.text});
					GLib.Application.get_default().activate_action("create-directory", tuple);
				}

				dg.destroy();
			});
		menu.add(mi);

		if ((string)name != ProjectStore.ROOT_FOLDER) {
			mi = new Gtk.MenuItem.with_label("Delete Folder");
			mi.activate.connect(() => {
					Gtk.MessageDialog md = new Gtk.MessageDialog(((Gtk.Application)GLib.Application.get_default()).active_window
						, Gtk.DialogFlags.MODAL
						, Gtk.MessageType.WARNING
						, Gtk.ButtonsType.NONE
						, "Delete Folder " + (string)name + "?"
						);

					Gtk.Widget btn;
					md.add_button("_Cancel", ResponseType.CANCEL);
					btn = md.add_button("_Delete", ResponseType.YES);
					btn.get_style_context().add_class(Gtk.STYLE_CLASS_DESTRUCTIVE_ACTION);

					md.set_default_response(ResponseType.CANCEL);

					int rt = md.run();
					md.destroy();

					if (rt != (int)ResponseType.YES)
						return;

					GLib.Application.get_default().activate_action("delete-directory", new GLib.Variant.string((string)name));
				});
			menu.add(mi);
		}
	} else { // If file
		menu = new Gtk.Menu();

		Gtk.MenuItem mi;

		mi = new Gtk.MenuItem.with_label("Delete File");
		mi.activate.connect(() => {
				GLib.Application.get_default().activate_action("delete-file", new GLib.Variant.string(ResourceId.path((string)type, (string)name)));
			});
		menu.add(mi);

		mi = new Gtk.MenuItem.with_label("Open Containing Folder...");
		mi.activate.connect(() => {
				GLib.Application.get_default().activate_action("open-containing", new GLib.Variant.string(name));
			});
		menu.add(mi);
	}

	// Add shared menu items.
	Gtk.MenuItem mi;

	mi = new Gtk.SeparatorMenuItem();
	menu.add(mi);

	mi = new Gtk.MenuItem.with_label("Copy Path");
	mi.activate.connect(() => {
			string path;
			if (type == "<folder>")
				path = name;
			else
				path = ResourceId.path(type, name);

			GLib.Application.get_default().activate_action("copy-path", new GLib.Variant.string(path));
		});
	menu.add(mi);

	return menu;
}

public class ProjectBrowser : Gtk.Box
{
	// Data
	public ProjectStore _project_store;

	// Widgets
	public Gtk.TreeModelFilter _tree_filter;
	public Gtk.TreeModelSort _tree_sort;
	public Gtk.TreeView _tree_view;
	public Gtk.TreeSelection _tree_selection;
	public Gtk.ScrolledWindow _scrolled_window;

	public bool _hide_core_resources;

	// Signals
	public signal void resource_selected(string type, string name);

	public ProjectBrowser(ProjectStore project_store)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

		// Data
		_project_store = project_store;

		// Widgets
		_tree_filter = new Gtk.TreeModelFilter(_project_store._tree_store, null);
		_tree_filter.set_visible_func((model, iter) => {
				_tree_view.expand_row(new Gtk.TreePath.first(), false);

				Value type;
				Value name;
				model.get_value(iter, ProjectStore.Column.TYPE, out type);
				model.get_value(iter, ProjectStore.Column.NAME, out name);

				return (string)type != null
					&& (string)name != null
					&& !row_should_be_hidden((string)type, (string)name)
					;
			});

		_tree_sort = new Gtk.TreeModelSort.with_model(_tree_filter);
		_tree_sort.set_default_sort_func((model, iter_a, iter_b) => {
				Value type_a;
				Value type_b;
				model.get_value(iter_a, ProjectStore.Column.TYPE, out type_a);
				model.get_value(iter_b, ProjectStore.Column.TYPE, out type_b);

				if ((string)type_a == "<folder>") {
					if ((string)type_b != "<folder>")
						return -1;
				} else if ((string)type_b == "<folder>") {
					if ((string)type_a != "<folder>")
						return 1;
				}

				Value id_a;
				Value id_b;
				model.get_value(iter_a, ProjectStore.Column.NAME, out id_a);
				model.get_value(iter_b, ProjectStore.Column.NAME, out id_b);
				return strcmp(GLib.Path.get_basename((string)id_a), GLib.Path.get_basename((string)id_b));
			});

		Gtk.CellRendererPixbuf cell_pixbuf = new Gtk.CellRendererPixbuf();
		Gtk.CellRendererText cell_text = new Gtk.CellRendererText();
		Gtk.TreeViewColumn column = new Gtk.TreeViewColumn();
		column.pack_start(cell_pixbuf, false);
		column.pack_start(cell_text, true);
		column.set_cell_data_func(cell_pixbuf, pixbuf_func);
		column.set_cell_data_func(cell_text, text_func);
		_tree_view = new Gtk.TreeView();
		_tree_view.append_column(column);
#if 0
		// For debugging.
		_tree_view.insert_column_with_attributes(-1
			, "Segment"
			, new Gtk.CellRendererText()
			, "text"
			, ProjectStore.Column.SEGMENT
			, null
			);
		_tree_view.insert_column_with_attributes(-1
			, "Name"
			, new Gtk.CellRendererText()
			, "text"
			, ProjectStore.Column.NAME
			, null
			);
		_tree_view.insert_column_with_attributes(-1
			, "Type"
			, new Gtk.CellRendererText()
			, "text"
			, ProjectStore.Column.TYPE
			, null
			);
#endif /* if 0 */
		_tree_view.model = _tree_sort;
		_tree_view.headers_visible = false;
		_tree_view.button_press_event.connect(on_button_pressed);
		_tree_view.button_release_event.connect(on_button_released);

		_tree_selection = _tree_view.get_selection();
		_tree_selection.set_mode(Gtk.SelectionMode.BROWSE);

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.add(_tree_view);

		_hide_core_resources = true;

		this.pack_start(_scrolled_window, true, true, 0);

		// Actions.
		GLib.ActionEntry[] action_entries =
		{
			{ "reveal-resource", on_reveal, "(ss)", null }
		};
		GLib.Application.get_default().add_action_entries(action_entries, this);
	}

	// Returns true if the row should be hidden.
	private bool row_should_be_hidden(string type, string name)
	{
		return type == "<folder>" && name == "core" && _hide_core_resources
			|| type == "importer_settings"
			|| name == Project.LEVEL_EDITOR_TEST_NAME
			;
	}

	private void on_reveal(GLib.SimpleAction action, GLib.Variant? param)
	{
		string type = (string)param.get_child_value(0);
		string name = (string)param.get_child_value(1);

		if (name.has_prefix("core/")) {
			_hide_core_resources = false;
			_tree_filter.refilter();
		}

		Gtk.TreePath store_path;
		if (_project_store.path_for_resource_type_name(out store_path, type, name)) {
			Gtk.TreePath filter_path = _tree_filter.convert_child_path_to_path(store_path);
			if (filter_path == null) // Either the path is not valid or points to a non-visible row in the model.
				return;
			Gtk.TreePath sort_path = _tree_sort.convert_child_path_to_path(filter_path);
			if (sort_path == null) // The path is not valid.
				return;

			_tree_view.expand_to_path(sort_path);
			_tree_view.get_selection().select_path(sort_path);
		}
	}

	private bool on_button_pressed(Gdk.EventButton ev)
	{
		if (ev.button == Gdk.BUTTON_SECONDARY) {
			Gtk.TreePath path;
			if (_tree_view.get_path_at_pos((int)ev.x, (int)ev.y, out path, null, null, null)) {
				Gtk.TreeIter iter;
				_tree_view.model.get_iter(out iter, path);

				Value type;
				Value name;
				_tree_view.model.get_value(iter, ProjectStore.Column.TYPE, out type);
				_tree_view.model.get_value(iter, ProjectStore.Column.NAME, out name);

				Gtk.Menu? menu = menu_create((string)type, (string)name);
				if (menu != null) {
					menu.show_all();
					menu.popup_at_pointer(ev);
				}
			}
		} else if (ev.button == Gdk.BUTTON_PRIMARY) {
			if (ev.type == Gdk.EventType.@2BUTTON_PRESS) {
				Gtk.TreePath path;
				if (_tree_view.get_path_at_pos((int)ev.x, (int)ev.y, out path, null, null, null)) {
					Gtk.TreeIter iter;
					_tree_view.model.get_iter(out iter, path);

					Value type;
					_tree_view.model.get_value(iter, ProjectStore.Column.TYPE, out type);
					if ((string)type == "<folder>")
						return Gdk.EVENT_PROPAGATE;

					Value name;
					_tree_view.model.get_value(iter, ProjectStore.Column.NAME, out name);

					GLib.Application.get_default().activate_action("open-resource", ResourceId.path((string)type, (string)name));
				}
			}
		}

		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_button_released(Gdk.EventButton ev)
	{
		if (ev.button == Gdk.BUTTON_PRIMARY) {
			Gtk.TreePath path;
			if (_tree_view.get_path_at_pos((int)ev.x, (int)ev.y, out path, null, null, null)) {
				Gtk.TreeIter iter;
				_tree_view.model.get_iter(out iter, path);

				Value type;
				_tree_view.model.get_value(iter, ProjectStore.Column.TYPE, out type);
				if ((string)type != "<folder>")
					return Gdk.EVENT_PROPAGATE;

				if (_tree_view.is_row_expanded(path))
					_tree_view.collapse_row(path);
				else
					_tree_view.expand_row(path, /*open_all = */ false);

				return Gdk.EVENT_STOP;
			}
		}

		return Gdk.EVENT_PROPAGATE;
	}

	public void select_project_root()
	{
		_tree_selection.select_path(new Gtk.TreePath.first());
	}

	private void pixbuf_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value type;
		model.get_value(iter, ProjectStore.Column.TYPE, out type);

		// https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html
		if ((string)type == "<folder>")
			cell.set_property("icon-name", "folder-symbolic");
		else if ((string)type == "state_machine")
			cell.set_property("icon-name", "text-x-generic-symbolic");
		else if ((string)type == "config")
			cell.set_property("icon-name", "text-x-generic-symbolic");
		else if ((string)type == "font")
			cell.set_property("icon-name", "font-x-generic-symbolic");
		else if ((string)type == "unit")
			cell.set_property("icon-name", "level-object-unit");
		else if ((string)type == "level")
			cell.set_property("icon-name", "text-x-generic-symbolic");
		else if ((string)type == "material")
			cell.set_property("icon-name", "text-x-generic-symbolic");
		else if ((string)type == "mesh")
			cell.set_property("icon-name", "text-x-generic-symbolic");
		else if ((string)type == "package")
			cell.set_property("icon-name", "package-x-generic-symbolic");
		else if ((string)type == "physics_config")
			cell.set_property("icon-name", "text-x-generic-symbolic");
		else if ((string)type == "lua")
			cell.set_property("icon-name", "x-office-document-symbolic");
		else if ((string)type == "shader")
			cell.set_property("icon-name", "text-x-script-symbolic");
		else if ((string)type == "sound")
			cell.set_property("icon-name", "audio-x-generic-symbolic");
		else if ((string)type == "sprite_animation")
			cell.set_property("icon-name", "text-x-generic-symbolic");
		else if ((string)type == "sprite")
			cell.set_property("icon-name", "text-x-generic-symbolic");
		else if ((string)type == "texture")
			cell.set_property("icon-name", "image-x-generic-symbolic");
		else
			cell.set_property("icon-name", "text-x-generic-symbolic");
	}

	private void text_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value name;
		Value type;
		model.get_value(iter, ProjectStore.Column.NAME, out name);
		model.get_value(iter, ProjectStore.Column.TYPE, out type);

		string basename = GLib.Path.get_basename((string)name);

		if ((string)type == "<folder>") {
			if ((string)name == "")
				cell.set_property("text", _project_store._project.name());
			else
				cell.set_property("text", basename);
		} else {
			cell.set_property("text", ResourceId.path((string)type, basename));
		}
	}
}

} /* namespace Crown */
