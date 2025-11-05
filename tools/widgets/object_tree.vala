/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
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
	public Database _database;
	public Guid _object_id;

	// Widgets
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

	public signal void selection_changed(Guid object_id, ItemType item_type);

	public ObjectTree(Database db)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

		_database = db;
		_object_id = GUID_ZERO;

		// Widgets
		_filter_entry = new EntrySearch();
		_filter_entry.set_placeholder_text("Search...");
		_filter_entry.search_changed.connect(on_filter_entry_text_changed);

		_tree_store = new Gtk.TreeStore(Column.COUNT
			, typeof(ItemType) // Column.ITEM_TYPE
			, typeof(Guid)     // Column.OBJECT_ID
			, typeof(string)   // Column.OBJECT_NAME
			, typeof(string)   // Column.SET_NAME
			);

		_tree_filter = new Gtk.TreeModelFilter(_tree_store, null);
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

						mi = new GLib.MenuItem("Rename...", null);
						mi.set_action_and_target_value("database.rename"
							, new GLib.Variant.tuple({ object_id.to_string(), "" })
							);
						menu_model.append_item(mi);

						if (object_id != _object_id) {
							_tree_view.model.get_value(iter, Column.SET_NAME, out val);
							string set_name = (string)val;

							mi = new GLib.MenuItem("Delete", null);
							mi.set_action_and_target_value("database.delete"
								, new GLib.Variant.tuple({ object_id.to_string(), set_name })
								);
							menu_model.append_item(mi);
						}
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
		ItemType item_type = ItemType.OBJECT;

		Gee.ArrayList<Guid?> ids = new Gee.ArrayList<Guid?>();
		_tree_selection.selected_foreach((model, path, iter) => {
				Value val;

				model.get_value(iter, Column.ITEM_TYPE, out val);
				item_type = (ItemType)val;
				model.get_value(iter, Column.OBJECT_ID, out val);
				ids.add((Guid)val);
			});

		if (ids.size > 0) {
			selection_changed(ids[0], item_type);
		}
	}

	public void on_filter_entry_text_changed()
	{
		_tree_selection.changed.disconnect(on_tree_selection_changed);
		_tree_filter.refilter();
		_tree_selection.changed.connect(on_tree_selection_changed);
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
				, -1
				);

			// Insert the set's content.
			if (_database.has_property(id, def.name)) {
				Gee.HashSet<Guid?> deffault_set = new Gee.HashSet<Guid?>();
				Gee.HashSet<Guid?> children = _database.get_property_set(id, def.name, deffault_set);
				foreach (var child_id in children) {
					Gtk.TreeIter child_iter;
					_tree_store.insert_with_values(out child_iter
						, iter
						, -1
						, Column.ITEM_TYPE
						, ItemType.OBJECT
						, Column.OBJECT_ID
						, child_id
						, Column.OBJECT_NAME
						, _database.object_name(child_id)
						, Column.SET_NAME
						, def.name
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

		Gtk.TreeIter object_iter;
		_tree_store.insert_with_values(out object_iter
			, null
			, -1
			, Column.ITEM_TYPE
			, ItemType.OBJECT
			, Column.OBJECT_ID
			, id
			, Column.OBJECT_NAME
			, _database.object_name(id)
			, Column.SET_NAME
			, ""
			, -1
			);
		add_object_set(object_iter, id);

		_tree_view.expand_all();
	}
}

} /* namespace Crown */
