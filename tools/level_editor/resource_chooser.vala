/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
// Returns true if the item should be filtered out
public bool user_filter(string type, string name)
{
	return (type == OBJECT_TYPE_UNIT || type == OBJECT_TYPE_SOUND) && !name.has_prefix("core/");
}

public delegate bool UserFilter(string type, string name);

public class ResourceChooser : Gtk.Box
{
	// Data
	public Project _project;
	public Gtk.ListStore _list_store;
	public Gtk.Stack? _editor_stack;
	public RuntimeInstance? _resource_preview;
	public unowned UserFilter _user_filter;
	public string _name;

	// Widgets
	public EntrySearch _filter_entry;
	public Gtk.EventControllerKey _filter_entry_controller_key;
	public Gtk.TreeModelFilter _tree_filter;
	public Gtk.TreeModelSort _tree_sort;
	public Gtk.TreeView _tree_view;
	public Gtk.GestureMultiPress _tree_view_gesture_click;
	public Gtk.TreeSelection _tree_selection;
	public Gtk.ScrolledWindow _scrolled_window;

	// Signals
	public signal void resource_selected(string type, string name);

	public ResourceChooser(Project? project
		, ProjectStore project_store
		, Gtk.Stack? editor_stack = null
		, RuntimeInstance? resource_preview = null
		)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

		// Data
		_project = project;

		_list_store = project_store._list_store;
		_editor_stack = editor_stack;
		_resource_preview = resource_preview;
		_user_filter = user_filter;

		// Widgets
		_filter_entry = new EntrySearch();
		_filter_entry.set_placeholder_text("Search...");
		_filter_entry.search_changed.connect(on_filter_entry_text_changed);

		_filter_entry_controller_key = new Gtk.EventControllerKey(_filter_entry);
		_filter_entry_controller_key.key_pressed.connect(on_filter_entry_key_pressed);

		_tree_filter = new Gtk.TreeModelFilter(_list_store, null);
		_tree_filter.set_visible_func((model, iter) => {
				Value type;
				Value name;
				model.get_value(iter, ProjectStore.Column.TYPE, out type);
				model.get_value(iter, ProjectStore.Column.NAME, out name);

				string type_str = (string)type;
				string name_str = (string)name;

				return type_str != null
					&& name_str != null
					&& _user_filter(type_str, name_str)
					&& (_filter_entry.text.length == 0 || name_str.index_of(_filter_entry.text) > -1)
					;
			});

		_tree_sort = new Gtk.TreeModelSort.with_model(_tree_filter);
		_tree_sort.set_default_sort_func((model, iter_a, iter_b) => {
				Value id_a;
				Value id_b;
				model.get_value(iter_a, ProjectStore.Column.NAME, out id_a);
				model.get_value(iter_b, ProjectStore.Column.NAME, out id_b);
				return strcmp((string)id_a, (string)id_b);
			});

		_tree_view = new Gtk.TreeView();
		_tree_view.insert_column_with_attributes(-1
			, "Name"
			, new Gtk.CellRendererText()
			, "text"
			, ProjectStore.Column.NAME
			, null
			);
#if 0
		// For debugging.
		_tree_view.insert_column_with_attributes(-1
			, "Type"
			, new Gtk.CellRendererText()
			, "text"
			, ProjectStore.Column.TYPE
			, null
			);
#endif
		_tree_view.model = _tree_sort;
		_tree_view.headers_visible = false;
		_tree_view.can_focus = false;
		_tree_view.row_activated.connect(on_row_activated);

		_tree_view_gesture_click = new Gtk.GestureMultiPress(_tree_view);
		_tree_view_gesture_click.set_button(0);
		_tree_view_gesture_click.released.connect(on_button_released);

		_tree_selection = _tree_view.get_selection();
		_tree_selection.set_mode(Gtk.SelectionMode.BROWSE);
		_tree_selection.changed.connect(on_tree_selection_changed);

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.add(_tree_view);
		_scrolled_window.set_size_request(300, 400);

		this.pack_start(_filter_entry, false, true, 0);
		if (_editor_stack != null)
			this.pack_start(_editor_stack, true, true, 0);
		this.pack_start(_scrolled_window, true, true, 0);

		this.unmap.connect(on_unmap);
	}

	public void on_row_activated(Gtk.TreePath path, Gtk.TreeViewColumn column)
	{
		Gtk.TreePath filter_path = _tree_sort.convert_path_to_child_path(path);
		Gtk.TreePath child_path = _tree_filter.convert_path_to_child_path(filter_path);
		Gtk.TreeIter iter;
		if (_list_store.get_iter(out iter, child_path)) {
			Value name;
			Value type;
			_list_store.get_value(iter, ProjectStore.Column.NAME, out name);
			_list_store.get_value(iter, ProjectStore.Column.TYPE, out type);
			_name = (string)name;
			resource_selected((string)type, (string)name);
		}
	}

	public void on_button_released(int n_press, double x, double y)
	{
		uint button = _tree_view_gesture_click.get_current_button();

		if (button == Gdk.BUTTON_PRIMARY) {
			int bx;
			int by;
			Gtk.TreePath path;
			_tree_view.convert_widget_to_bin_window_coords((int)x, (int)y, out bx, out by);
			if (_tree_view.get_path_at_pos(bx, by, out path, null, null, null)) {
				if (_tree_view.get_selection().path_is_selected(path)) {
					Gtk.TreePath filter_path = _tree_sort.convert_path_to_child_path(path);
					Gtk.TreePath child_path = _tree_filter.convert_path_to_child_path(filter_path);
					Gtk.TreeIter iter;
					if (_list_store.get_iter(out iter, child_path)) {
						Value name;
						Value type;
						_list_store.get_value(iter, ProjectStore.Column.NAME, out name);
						_list_store.get_value(iter, ProjectStore.Column.TYPE, out type);
						_name = (string)name;
						resource_selected((string)type, (string)name);
					}
				}
			}
		}
	}

	public void on_unmap()
	{
		_filter_entry.text = "";
	}

	public void on_filter_entry_text_changed()
	{
		_tree_selection.changed.disconnect(on_tree_selection_changed);
		_tree_filter.refilter();
		_tree_selection.changed.connect(on_tree_selection_changed);
		_tree_view.set_cursor(new Gtk.TreePath.first(), null, false);
	}

	public bool on_filter_entry_key_pressed(uint keyval, uint keycode, Gdk.ModifierType state)
	{
		Gtk.TreeModel model;
		Gtk.TreeIter iter;
		bool selected = _tree_selection.get_selected(out model, out iter);

		if (keyval == Gdk.Key.Down) {
			if (selected && model.iter_next(ref iter)) {
				_tree_selection.select_iter(iter);
				_tree_view.scroll_to_cell(model.get_path(iter), null, true, 1.0f, 0.0f);
			}

			return Gdk.EVENT_STOP;
		} else if (keyval == Gdk.Key.Up) {
			if (selected && model.iter_previous(ref iter)) {
				_tree_selection.select_iter(iter);
				_tree_view.scroll_to_cell(model.get_path(iter), null, true, 1.0f, 0.0f);
			}

			return Gdk.EVENT_STOP;
		} else if (keyval == Gdk.Key.Return) {
			if (selected) {
				Value name;
				Value type;
				model.get_value(iter, ProjectStore.Column.NAME, out name);
				model.get_value(iter, ProjectStore.Column.TYPE, out type);
				_name = (string)name;
				resource_selected((string)type, (string)name);
			}

			return Gdk.EVENT_STOP;
		}

		return Gdk.EVENT_PROPAGATE;
	}

	public void on_tree_selection_changed()
	{
		if (_editor_stack != null) {
			Gtk.TreeModel model;
			Gtk.TreeIter iter;
			if (_tree_selection.get_selected(out model, out iter)) {
				Value name;
				Value type;
				model.get_value(iter, ProjectStore.Column.NAME, out name);
				model.get_value(iter, ProjectStore.Column.TYPE, out type);
				_resource_preview.send_script(UnitPreviewApi.set_preview_resource((string)type, (string)name));
				_resource_preview.send(DeviceApi.frame());
			}
		}
	}

	public void set_type_filter(owned UserFilter filter)
	{
		_user_filter = filter;
		_tree_filter.refilter();
	}
}

} /* namespace Crown */
