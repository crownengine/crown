/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class LevelTreeView : Gtk.Box
{
	private enum ItemType
	{
		FOLDER,
		CAMERA,
		LIGHT,
		SOUND,
		UNIT
	}

	private enum Column
	{
		TYPE,
		GUID,
		NAME,

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
	private Level _level;
	private Database _db;

	// Widgets
	private EntrySearch _filter_entry;
	private Gtk.TreeStore _tree_store;
	private Gtk.TreeModelFilter _tree_filter;
	private Gtk.TreeModelSort _tree_sort;
	private Gtk.TreeView _tree_view;
	private Gtk.TreeSelection _tree_selection;
	private Gtk.ScrolledWindow _scrolled_window;
	private Gtk.Box _sort_items_box;
	private Gtk.Popover _sort_items_popover;
	private Gtk.MenuButton _sort_items;
	private Gtk.GestureMultiPress _gesture_click;
	private Gtk.TreeRowReference _units_root;
	private Gtk.TreeRowReference _sounds_root;

	public LevelTreeView(Database db, Level level)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

		// Data
		_level = level;
		_level.selection_changed.connect(on_level_selection_changed);
		_level.object_editor_name_changed.connect(on_object_editor_name_changed);

		_db = db;

		// Widgets
		_filter_entry = new EntrySearch();
		_filter_entry.set_placeholder_text("Search...");
		_filter_entry.search_changed.connect(on_filter_entry_text_changed);

		_tree_store = new Gtk.TreeStore(Column.COUNT
			, typeof(int)    // Column.TYPE
			, typeof(Guid)   // Column.GUID
			, typeof(string) // Column.NAME
			);

		_tree_filter = new Gtk.TreeModelFilter(_tree_store, null);
		_tree_filter.set_visible_func((model, iter) => {
				_tree_view.expand_all();

				Value type;
				Value name;
				model.get_value(iter, Column.TYPE, out type);
				model.get_value(iter, Column.NAME, out name);

				if ((int)type == ItemType.FOLDER)
					return true;

				string name_str = (string)name;
				string filter_text = _filter_entry.text.down();

				return name_str != null
					&& (filter_text == "" || name_str.down().index_of(filter_text) > -1)
					;
			});

		_tree_sort = new Gtk.TreeModelSort.with_model(_tree_filter);
		_tree_sort.set_sort_column_id(Column.NAME, Gtk.SortType.ASCENDING);

		Gtk.TreeViewColumn column = new Gtk.TreeViewColumn();
		Gtk.CellRendererPixbuf cell_pixbuf = new Gtk.CellRendererPixbuf();
		Gtk.CellRendererText cell_text = new Gtk.CellRendererText();
		column.pack_start(cell_pixbuf, false);
		column.pack_start(cell_text, true);
		column.set_cell_data_func(cell_pixbuf, (cell_layout, cell, model, iter) => {
				Value type;
				model.get_value(iter, LevelTreeView.Column.TYPE, out type);

				if ((int)type == LevelTreeView.ItemType.FOLDER)
					cell.set_property("icon-name", "browser-folder-symbolic");
				else if ((int)type == LevelTreeView.ItemType.UNIT)
					cell.set_property("icon-name", "level-object-unit");
				else if ((int)type == LevelTreeView.ItemType.SOUND)
					cell.set_property("icon-name", "level-object-sound");
				else if ((int)type == LevelTreeView.ItemType.LIGHT)
					cell.set_property("icon-name", "level-object-light");
				else if ((int)type == LevelTreeView.ItemType.CAMERA)
					cell.set_property("icon-name", "level-object-camera");
				else
					cell.set_property("icon-name", "level-object-unknown");
			});
		column.set_cell_data_func(cell_text, (cell_layout, cell, model, iter) => {
				Value name;
				model.get_value(iter, LevelTreeView.Column.NAME, out name);

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
			, Column.GUID
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
		_tree_selection.set_mode(Gtk.SelectionMode.MULTIPLE);
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

	private void on_button_pressed(int n_press, double x, double y)
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

			GLib.Menu menu_model = new GLib.Menu();
			GLib.MenuItem mi;

			if (_tree_selection.count_selected_rows() == 1) {
				GLib.List<Gtk.TreePath> selected_paths = _tree_selection.get_selected_rows(null);

				Gtk.TreeIter iter;
				if (_tree_view.model.get_iter(out iter, selected_paths.nth(0).data)) {
					Value val;
					_tree_view.model.get_value(iter, Column.TYPE, out val);

					if ((int)val != ItemType.FOLDER) {
						_tree_view.model.get_value(iter, Column.GUID, out val);
						Guid object_id = (Guid)val;

						mi = new GLib.MenuItem("Rename...", null);
						mi.set_action_and_target_value("app.rename", new GLib.Variant.tuple({ object_id.to_string(), "" }));
						menu_model.append_item(mi);
					}
				}
			}

			mi = new GLib.MenuItem("Duplicate", null);
			mi.set_action_and_target_value("app.duplicate", null);
			menu_model.append_item(mi);

			mi = new GLib.MenuItem("Delete", null);
			mi.set_action_and_target_value("app.delete", null);
			menu_model.append_item(mi);

			if (_tree_selection.count_selected_rows() == 1) {
				GLib.List<Gtk.TreePath> selected_paths = _tree_selection.get_selected_rows(null);

				Gtk.TreeIter iter;
				if (_tree_view.model.get_iter(out iter, selected_paths.nth(0).data)) {
					Value val;
					_tree_view.model.get_value(iter, Column.GUID, out val);
					Guid object_id = (Guid)val;
					_tree_view.model.get_value(iter, Column.NAME, out val);
					string object_name = (string)val;

					if (_db.object_type(object_id) == OBJECT_TYPE_UNIT) {
						mi = new GLib.MenuItem("Save as Prefab...", null);
						mi.set_action_and_target_value("app.unit-save-as-prefab", new GLib.Variant.tuple({ object_id.to_string(), object_name }));
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

	private void on_tree_selection_changed()
	{
		_level.selection_changed.disconnect(on_level_selection_changed);

		Gee.ArrayList<Guid?> ids = new Gee.ArrayList<Guid?>();
		_tree_selection.selected_foreach((model, path, iter) => {
				Value type;
				model.get_value(iter, Column.TYPE, out type);
				if ((int)type == ItemType.FOLDER)
					return;

				Value id;
				model.get_value(iter, Column.GUID, out id);
				ids.add((Guid)id);
			});

		_level.selection_set(ids.to_array());
		_level.selection_changed.connect(on_level_selection_changed);
	}

	private void on_level_selection_changed(Gee.ArrayList<Guid?> selection)
	{
		_tree_selection.changed.disconnect(on_tree_selection_changed);
		_tree_selection.unselect_all();

		Gtk.TreePath? last_selected = null;

		_tree_sort.foreach ((model, path, iter) => {
				Value type;
				model.get_value(iter, Column.TYPE, out type);
				if ((int)type == ItemType.FOLDER)
					return false;

				Value id;
				model.get_value(iter, Column.GUID, out id);

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

	private void on_object_editor_name_changed(Guid object_id, string name)
	{
		_tree_sort.foreach ((model, path, iter) => {
				Value type;
				model.get_value(iter, Column.TYPE, out type);
				if ((int)type == ItemType.FOLDER)
					return false;

				Value guid;
				model.get_value(iter, Column.GUID, out guid);
				Guid guid_model = (Guid)guid;

				if (guid_model == object_id) {
					Gtk.TreeIter iter_filter;
					Gtk.TreeIter iter_model;
					_tree_sort.convert_iter_to_child_iter(out iter_filter, iter);
					_tree_filter.convert_iter_to_child_iter(out iter_model, iter_filter);

					_tree_store.set(iter_model
						, Column.NAME
						, name
						, -1
						);

					return true;
				}

				return false;
			});
	}

	private ItemType item_type(Unit u)
	{
		if (u.is_light())
			return ItemType.LIGHT;
		else if (u.is_camera())
			return ItemType.CAMERA;
		else
			return ItemType.UNIT;
	}

	// Sets the level object to show.
	public void set_level(Level level)
	{
		Gtk.TreeIter iter;

		_tree_view.model = null;
		_tree_store.clear();

		_tree_store.insert_with_values(out iter
			, null
			, -1
			, Column.TYPE
			, ItemType.FOLDER
			, Column.GUID
			, GUID_ZERO
			, Column.NAME
			, "Units"
			, -1
			);
		_units_root = new Gtk.TreeRowReference(_tree_store, _tree_store.get_path(iter));

		_tree_store.insert_with_values(out iter
			, null
			, -1
			, Column.TYPE
			, ItemType.FOLDER
			, Column.GUID
			, GUID_ZERO
			, Column.NAME
			, "Sounds"
			, -1
			);
		_sounds_root = new Gtk.TreeRowReference(_tree_store, _tree_store.get_path(iter));

		_tree_view.model = _tree_sort;
		_tree_view.expand_all();

		_level = level;
		on_objects_created(_db.get_property_set(_level._id, "units", new Gee.HashSet<Guid?>()).to_array());
		on_objects_created(_db.get_property_set(_level._id, "sounds", new Gee.HashSet<Guid?>()).to_array());
	}

	public void on_objects_created(Guid?[] object_ids)
	{
		Gee.HashSet<Guid?> units  = _db.get_property_set(_level._id, "units", new Gee.HashSet<Guid?>());
		Gee.HashSet<Guid?> sounds = _db.get_property_set(_level._id, "sounds", new Gee.HashSet<Guid?>());
		Gtk.TreeIter iter;

		foreach (Guid id in object_ids) {
			if (_db.object_type(id) == OBJECT_TYPE_UNIT && units.contains(id)) {
				Unit u = Unit(_level._db, id);
				Gtk.TreeIter units_iter;

				_tree_store.get_iter(out units_iter, _units_root.get_path());
				_tree_store.insert_with_values(out iter
					, units_iter
					, -1
					, Column.TYPE
					, item_type(u)
					, Column.GUID
					, id
					, Column.NAME
					, _level.object_editor_name(id)
					, -1
					);
			} else if (_db.object_type(id) == OBJECT_TYPE_SOUND_SOURCE && sounds.contains(id)) {
				Gtk.TreeIter sounds_iter;

				_tree_store.get_iter(out sounds_iter, _sounds_root.get_path());
				_tree_store.insert_with_values(out iter
					, sounds_iter
					, -1
					, Column.TYPE
					, ItemType.SOUND
					, Column.GUID
					, id
					, Column.NAME
					, _level.object_editor_name(id)
					, -1
					);
			}
		}
	}

	public void on_objects_destroyed(Guid?[] object_ids)
	{
		foreach (Guid id in object_ids) {
			Gtk.TreeIter parent_iter;
			if (_db.object_type(id) == OBJECT_TYPE_UNIT)
				_tree_store.get_iter(out parent_iter, _units_root.get_path());
			else if (_db.object_type(id) == OBJECT_TYPE_SOUND_SOURCE)
				_tree_store.get_iter(out parent_iter, _sounds_root.get_path());
			else
				continue;

			remove_item(id, parent_iter);
		}
	}

	private void remove_item(Guid id, Gtk.TreeIter parent_iter)
	{
		Gtk.TreeIter child;

		if (_tree_store.iter_children(out child, parent_iter)) {
			Value column_id;

			while (true) {
				_tree_store.get_value(child, Column.GUID, out column_id);
				if (Guid.equal_func((Guid)column_id, id)) {
					_tree_store.remove(ref child);
					break;
				} else {
					if (!_tree_store.iter_next(ref child))
						break;
				}
			}
		}
	}

	private void on_filter_entry_text_changed()
	{
		_tree_selection.changed.disconnect(on_tree_selection_changed);
		_tree_filter.refilter();
		_tree_selection.changed.connect(on_tree_selection_changed);
	}

	private Gtk.RadioButton add_sort_item(Gtk.RadioButton? group, SortMode mode)
	{
		var button = new Gtk.RadioButton.with_label_from_widget(group, mode.to_label());
		button.toggled.connect(() => {
				if (mode == SortMode.NAME_AZ)
					_tree_sort.set_sort_column_id(Column.NAME, Gtk.SortType.ASCENDING);
				else if (mode == SortMode.NAME_ZA)
					_tree_sort.set_sort_column_id(Column.NAME, Gtk.SortType.DESCENDING);
				else if (mode == SortMode.TYPE_AZ)
					_tree_sort.set_sort_column_id(Column.TYPE, Gtk.SortType.ASCENDING);
				else if (mode == SortMode.TYPE_ZA)
					_tree_sort.set_sort_column_id(Column.TYPE, Gtk.SortType.DESCENDING);

				_tree_filter.refilter();
				_sort_items_popover.popdown();
			});
		_sort_items_box.pack_start(button, false, false);
		return button;
	}
}

} /* namespace Crown */
