/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class ObjectTree : Gtk.Box
{
	public enum ItemType
	{
		OBJECT,
		OBJECTS_SET
	}

	public enum Column
	{
		ITEM_TYPE,   ///< enum ItemType.
		OBJECT_ID,   ///< Object ID that owns the property.
		OBJECT_NAME, ///< Object name for displaying.
		SET_NAME,    ///< Name of the set property.
		VISIBLE,
		SAVE_STATE,

		COUNT
	}

	public enum SortMode
	{
		NAME_AZ,
		NAME_ZA,
		TYPE_AZ,
		TYPE_ZA,

		COUNT;

		public string to_label()
		{
			switch (this) {
			case NAME_AZ:
				return "Name A-Z";
			case NAME_ZA:
				return "Name Z-A";
			case TYPE_AZ:
				return "Type A-Z";
			case TYPE_ZA:
				return "Type Z-A";
			default:
				return "Unknown";
			}
		}
	}

	// Data
	public DatabaseEditor _database_editor;
	public Database _database;
	public Guid _object_id;

	// Widgets
	public string _needle;
	public EntrySearch _filter_entry;
	public Gtk.TreeStore _tree_store;
	public Gtk.TreeModelFilter _tree_filter;
	public Gtk.TreeModelSort _tree_sort;
	public Gtk.TreeView _tree_view;
	public Gtk.TreeSelection _tree_selection;
	public Gtk.ScrolledWindow _scrolled_window;
	public Gtk.Box _sort_items_box;
	public Gtk.Popover _sort_items_popover;
	public Gtk.MenuButton _sort_items;
	public Gtk.GestureMultiPress _gesture_click;

	public ObjectTree(DatabaseEditor database_editor)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

		_database_editor = database_editor;
		_database_editor.selection_changed.connect(on_database_selection_changed);
		_database = database_editor._database;
		_object_id = GUID_ZERO;

		// Widgets
		_needle = "";
		_filter_entry = new EntrySearch();
		_filter_entry.set_placeholder_text("Search...");
		_filter_entry.search_changed.connect(on_filter_entry_text_changed);
		_filter_entry._entry.stop_search.connect(on_stop_search);

		_tree_store = new Gtk.TreeStore(Column.COUNT
			, typeof(ItemType) // Column.ITEM_TYPE
			, typeof(Guid)     // Column.OBJECT_ID
			, typeof(string)   // Column.OBJECT_NAME
			, typeof(string)   // Column.SET_NAME
			, typeof(bool)     // Column.VISIBLE
			, typeof(uint32)   // Column.SAVED_STATE
			);

		_tree_filter = new Gtk.TreeModelFilter(_tree_store, null);
		_tree_filter.set_visible_column(Column.VISIBLE);

		_tree_sort = new Gtk.TreeModelSort.with_model(_tree_filter);
		_tree_sort.set_sort_column_id(Column.OBJECT_NAME, Gtk.SortType.ASCENDING);

		Gtk.TreeViewColumn column = new Gtk.TreeViewColumn();
		Gtk.CellRendererPixbuf cell_pixbuf = new Gtk.CellRendererPixbuf();
		Gtk.CellRendererText cell_text = new Gtk.CellRendererText();
		column.pack_start(cell_pixbuf, false);
		column.pack_start(cell_text, true);
		column.set_cell_data_func(cell_pixbuf, (cell_layout, cell, model, iter) => {
				Value type;
				model.get_value(iter, Column.ITEM_TYPE, out type);

				if ((ItemType)type == ItemType.OBJECTS_SET)
					cell.set_property("icon-name", "browser-folder-symbolic");
				else if ((ItemType)type == ItemType.OBJECT)
					cell.set_property("icon-name", "level-object-unit");
				else
					cell.set_property("icon-name", "level-object-unknown");
			});
		column.set_cell_data_func(cell_text, (cell_layout, cell, model, iter) => {
				Value name;
				model.get_value(iter, Column.OBJECT_NAME, out name);

				cell.set_property("text", (string)name);
			});
		_tree_view = new Gtk.TreeView();
		_tree_view.append_column(column);
#if 0
		// For debugging.
		_tree_view.insert_column_with_attributes(-1
			, "Guids"
			, new gtk.CellRendererText()
			, "text"
			, Column.OBJECT_ID
			, null
			);
#endif
		_tree_view.headers_clickable = false;
		_tree_view.headers_visible = false;
		_tree_view.model = _tree_sort;

		_gesture_click = new Gtk.GestureMultiPress(_tree_view);
		_gesture_click.set_propagation_phase(Gtk.PropagationPhase.CAPTURE);
		_gesture_click.set_button(0);
		_gesture_click.pressed.connect(on_button_pressed);

		_tree_selection = _tree_view.get_selection();
		_tree_selection.set_mode(Gtk.SelectionMode.SINGLE);
		_tree_selection.changed.connect(on_tree_selection_changed);

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.add(_tree_view);

		// Setup sort menu button popover.
		_sort_items_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);

		Gtk.RadioButton? button = null;
		for (int i = 0; i < SortMode.COUNT; ++i)
			button = add_sort_item(button, (SortMode)i);

		_sort_items_box.show_all();
		_sort_items_popover = new Gtk.Popover(null);
		_sort_items_popover.add(_sort_items_box);
		_sort_items = new Gtk.MenuButton();
		_sort_items.add(new Gtk.Image.from_icon_name("list-sort", Gtk.IconSize.SMALL_TOOLBAR));
		_sort_items.get_style_context().add_class("flat");
		_sort_items.get_style_context().add_class("image-button");
		_sort_items.can_focus = false;
		_sort_items.set_popover(_sort_items_popover);

		var tree_control = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		tree_control.pack_start(_filter_entry, true, true);
		tree_control.pack_end(_sort_items, false, false);

		this.pack_start(tree_control, false, true, 0);
		this.pack_start(_scrolled_window, true, true, 0);
	}

	public void on_button_pressed(int n_press, double x, double y)
	{
		if (_gesture_click.get_current_button() == Gdk.BUTTON_SECONDARY) {
			int bx;
			int by;
			Gtk.TreePath path;
			Gtk.TreeViewColumn column;
			_tree_view.convert_widget_to_bin_window_coords((int)x, (int)y, out bx, out by);
			if (!_tree_view.get_path_at_pos(bx, by, out path, out column, null, null))
				return; // Clicked on empty space.

			if (!_tree_selection.path_is_selected(path)) {
				_tree_selection.unselect_all();
				_tree_selection.select_path(path);
			}

			// Create menu.
			GLib.Menu menu_model = new GLib.Menu();
			GLib.MenuItem mi;

			if (_tree_selection.count_selected_rows() == 1) {
				GLib.List<Gtk.TreePath> selected_paths = _tree_selection.get_selected_rows(null);

				Gtk.TreeIter iter;
				if (_tree_view.model.get_iter(out iter, selected_paths.nth(0).data)) {
					Value val;
					_tree_view.model.get_value(iter, Column.ITEM_TYPE, out val);

					if ((ItemType)val == ItemType.OBJECT) {
						_tree_view.model.get_value(iter, Column.OBJECT_ID, out val);
						Guid object_id = (Guid)val;

						if (object_id != _object_id) {
							mi = new GLib.MenuItem("Duplicate", null);
							mi.set_action_and_target_value("database.duplicate", null);
							menu_model.append_item(mi);

							mi = new GLib.MenuItem("Delete", null);
							mi.set_action_and_target_value("database.delete", null);
							menu_model.append_item(mi);
						}
					} else if ((ItemType)val == ItemType.OBJECTS_SET) {
						_tree_view.model.get_value(iter, Column.OBJECT_ID, out val);
						Guid object_id = (Guid)val;
						_tree_view.model.get_value(iter, Column.SET_NAME, out val);
						string set_name = (string)val;

						mi = new GLib.MenuItem("Add", null);
						mi.set_action_and_target_value("database.add"
							, new GLib.Variant.tuple({ object_id.to_string(), set_name })
							);
						menu_model.append_item(mi);
					}
				}
			}

			Gtk.Popover menu = new Gtk.Popover.from_model(null, menu_model);
			menu.set_relative_to(_tree_view);
			menu.set_pointing_to({ (int)x, (int)y, 1, 1 });
			menu.set_position(Gtk.PositionType.BOTTOM);
			menu.popup();

			_gesture_click.set_state(Gtk.EventSequenceState.CLAIMED);
		}
	}

	public void on_tree_selection_changed()
	{
		_database_editor.selection_changed.disconnect(on_tree_selection_changed);

		ItemType item_type = ItemType.OBJECT;

		Gee.ArrayList<Guid?> ids = new Gee.ArrayList<Guid?>();
		_tree_selection.selected_foreach((model, path, iter) => {
				Value val;

				model.get_value(iter, Column.ITEM_TYPE, out val);
				item_type = (ItemType)val;

				if (item_type == ItemType.OBJECT) {
					model.get_value(iter, Column.OBJECT_ID, out val);
					ids.add((Guid)val);
				}
			});

		if (ids.size > 0)
			_database_editor.selection_set(ids.to_array());

		_database_editor.selection_changed.connect(on_tree_selection_changed);
	}

	public Gtk.RadioButton add_sort_item(Gtk.RadioButton? group, SortMode mode)
	{
		var button = new Gtk.RadioButton.with_label_from_widget(group, mode.to_label());
		button.toggled.connect(() => {
				if (mode == SortMode.NAME_AZ)
					_tree_sort.set_sort_column_id(Column.OBJECT_NAME, Gtk.SortType.ASCENDING);
				else if (mode == SortMode.NAME_ZA)
					_tree_sort.set_sort_column_id(Column.OBJECT_NAME, Gtk.SortType.DESCENDING);
				else if (mode == SortMode.TYPE_AZ)
					_tree_sort.set_sort_column_id(Column.ITEM_TYPE, Gtk.SortType.ASCENDING);
				else if (mode == SortMode.TYPE_ZA)
					_tree_sort.set_sort_column_id(Column.ITEM_TYPE, Gtk.SortType.DESCENDING);

				_tree_filter.refilter();
				_sort_items_popover.popdown();
			});
		_sort_items_box.pack_start(button, false, false);
		return button;
	}

	public void add_object_set(Gtk.TreeIter parent_iter, Guid id)
	{
		StringId64 object_type = StringId64(_database.object_type(id));
		unowned PropertyDefinition[] object_definition = _database.object_definition(object_type);

		foreach (var def in object_definition) {
			if (def.type != PropertyType.OBJECTS_SET)
				continue;

			// Insert set itself.
			Gtk.TreeIter iter;
			_tree_store.insert_with_values(out iter
				, parent_iter
				, -1
				, Column.ITEM_TYPE
				, ItemType.OBJECTS_SET
				, Column.OBJECT_ID
				, id
				, Column.OBJECT_NAME
				, camel_case(def.label)
				, Column.SET_NAME
				, def.name
				, Column.VISIBLE
				, true
				, Column.SAVE_STATE
				, 0u
				, -1
				);

			// Insert the set's content.
			if (_database.has_property(id, def.name)) {
				Gee.HashSet<Guid?> deffault_set = new Gee.HashSet<Guid?>();
				Gee.HashSet<Guid?> children = _database.get_set(id, def.name, deffault_set);
				foreach (var child_id in children) {
					if (!_database.is_alive(child_id))
						continue;

					StringId64 child_type = StringId64(_database.object_type(child_id));
					Aspect? name_aspect = _database.get_aspect(child_type, StringId64("name"));
					if (name_aspect == null)
						name_aspect = default_name_aspect;

					string object_name;
					name_aspect(out object_name, _database, child_id);

					Gtk.TreeIter child_iter;
					_tree_store.insert_with_values(out child_iter
						, iter
						, -1
						, Column.ITEM_TYPE
						, ItemType.OBJECT
						, Column.OBJECT_ID
						, child_id
						, Column.OBJECT_NAME
						, object_name
						, Column.SET_NAME
						, def.name
						, Column.VISIBLE
						, true
						, Column.SAVE_STATE
						, 0u
						, -1
						);
					add_object_set(child_iter, child_id);
				}
			}
		}
	}

	public void set_object(Guid id)
	{
		_tree_store.clear();

		_object_id = id;

		ObjectTypeInfo info = _database.type_info(StringId64(_database.object_type(id)));

		Gtk.TreeIter object_iter;
		_tree_store.insert_with_values(out object_iter
			, null
			, -1
			, Column.ITEM_TYPE
			, ItemType.OBJECT
			, Column.OBJECT_ID
			, id
			, Column.OBJECT_NAME
			, info.ui_name
			, Column.SET_NAME
			, ""
			, Column.VISIBLE
			, true
			, Column.SAVE_STATE
			, 0u
			, -1
			);
		add_object_set(object_iter, id);

		_tree_view.expand_all();
	}

	public void on_database_selection_changed()
	{
		_tree_selection.changed.disconnect(on_tree_selection_changed);
		_tree_selection.unselect_all();

		Gee.ArrayList<Guid?> selection = _database_editor._selection;
		Gtk.TreePath? last_selected = null;

		_tree_sort.foreach ((model, path, iter) => {
				Value type;
				model.get_value(iter, Column.ITEM_TYPE, out type);
				if ((ItemType)type != ItemType.OBJECT)
					return false;

				Value id;
				model.get_value(iter, Column.OBJECT_ID, out id);

				foreach (Guid? guid in selection) {
					if ((Guid)id == guid) {
						_tree_selection.select_iter(iter);
						last_selected = path;
						return false;
					}
				}

				return false;
			});

		if (last_selected != null)
			_tree_view.scroll_to_cell(last_selected, null, false, 0.0f, 0.0f);

		_tree_selection.changed.connect(on_tree_selection_changed);
	}

	public bool save_tree_state(Gtk.TreeModel model, Gtk.TreePath path, Gtk.TreeIter iter)
	{
		Gtk.TreePath filter_path = _tree_filter.convert_child_path_to_path(path);
		if (filter_path == null) {
			// Either the path is not valid or points to a non-visible row in the model.
			return false;
		}

		Gtk.TreePath sort_path = _tree_sort.convert_child_path_to_path(filter_path);
		if (sort_path == null) {
			// The path is not valid.
			assert(false);
			return false;
		}

		bool expanded = _tree_view.is_row_expanded(sort_path);
		bool selected = _tree_view.get_selection().path_is_selected(sort_path);

		uint32 user_data = 0;
		user_data |= (uint32)expanded << 0;
		user_data |= (uint32)selected << 1;
		_tree_store.set(iter, Column.SAVE_STATE, user_data, -1);

		return false; // Continue iterating.
	}

	public bool restore_tree_state(Gtk.TreeModel model, Gtk.TreePath path, Gtk.TreeIter iter)
	{
		uint32 user_data;
		Value val;
		_tree_store.get_value(iter, Column.SAVE_STATE, out val);
		user_data = (uint32)val;

		bool expanded = (bool)((user_data & 0x1) >> 0);
		bool selected = (bool)((user_data & 0x2) >> 1);

		Gtk.TreePath filter_path = _tree_filter.convert_child_path_to_path(path);
		if (filter_path == null) {
			// Either the path is not valid or points to a non-visible row in the model.
			return false;
		}

		Gtk.TreePath sort_path = _tree_sort.convert_child_path_to_path(filter_path);
		if (sort_path == null) {
			// The path is not valid.
			return false;
		}

		if (expanded)
			_tree_view.expand_to_path(sort_path);
		else
			_tree_view.collapse_row(sort_path);

		if (selected)
			_tree_view.get_selection().select_path(sort_path);

		return false; // Continue iterating.
	}

	public void make_visible(bool visible)
	{
		_tree_store.foreach((model, path, iter) => {
				_tree_store.set(iter, Column.VISIBLE, visible, -1);
				return false; // Continue iterating.
			});
	}

	public void filter(string needle)
	{
		make_visible(false);
		_tree_filter.refilter();

		_tree_store.foreach((model, path, iter) => {
				ItemType type;
				string name;

				Value val;
				model.get_value(iter, Column.ITEM_TYPE, out val);
				type = (ItemType)val;

				bool visible = false;

				// Always show the roots.
				model.get_value(iter, Column.OBJECT_NAME, out val);
				name = (string)val;
				visible = needle == "" || name.down().index_of(needle) > -1;

				if (visible) {
					// Make this iter and all its ancestors visible.
					Gtk.TreeIter it = iter;
					_tree_store.set(it, Column.VISIBLE, true, -1);
					while (_tree_store.iter_parent(out it, it))
						_tree_store.set(it, Column.VISIBLE, true, -1);
				}

				return false; // Continue iterating.
			});

		_tree_view.expand_all();
	}

	public void on_search_started()
	{
		_tree_selection.changed.disconnect(on_tree_selection_changed);
		// Save the current tree state (expanded branches + selection)
		// to restore it later when the search is done.
		_tree_store.foreach(save_tree_state);
		filter(_needle);
		_tree_selection.changed.connect(on_tree_selection_changed);
	}

	public void on_search_changed()
	{
		_tree_selection.changed.disconnect(on_tree_selection_changed);
		filter(_needle);
		_tree_selection.changed.connect(on_tree_selection_changed);
	}

	public void on_search_stopped()
	{
		// Only restore the old selection if it has not been
		// modified while searching (i.e. nothing is selected
		// because entering search clears it).
		Gtk.TreeModel selected_model;
		GLib.List<Gtk.TreePath> selected_rows = _tree_view.get_selection().get_selected_rows(out selected_model);
		Gtk.TreeRowReference[] selected_refs = {};
		for (uint i = 0, n = selected_rows.length(); i < n; ++i)
			selected_refs += new Gtk.TreeRowReference(selected_model, selected_rows.nth(i).data);

		_tree_selection.changed.disconnect(on_tree_selection_changed);
		make_visible(true);
		_tree_filter.refilter();
		// Restore the previous tree state (old expanded branches + old selection).
		_tree_view.get_selection().unselect_all();
		_tree_store.foreach(restore_tree_state);
		_tree_selection.changed.connect(on_tree_selection_changed);

		// If the selection changed while searching, restore it as well.
		for (int i = 0; i < selected_refs.length; ++i) {
			Gtk.TreePath path = selected_refs[i].get_path();
			_tree_view.expand_to_path(path);
			_tree_view.get_selection().select_path(path);
			_tree_view.scroll_to_cell(path, null, false, 0.0f, 0.0f);
		}
	}

	public void on_stop_search()
	{
		_filter_entry._entry.set_text("");
	}

	public void on_filter_entry_text_changed()
	{
		string old_needle = _needle;
		_needle = _filter_entry.text.strip().down();

		if (old_needle == "" && _needle != "") {
			on_search_started();
		} else if (old_needle != "" && _needle == "") {
			on_search_stopped();
		} else if (_needle != "") {
			on_search_changed();
		}
	}
}

} /* namespace Crown */
