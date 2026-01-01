/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class LevelTreeView : Gtk.Box
{
	public enum ItemType
	{
		FOLDER,
		CAMERA,
		LIGHT,
		SOUND,
		UNIT
	}

	public enum Column
	{
		TYPE,
		GUID,
		NAME,
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
	public Level _level;
	public Database _db;

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
	public Gtk.TreeRowReference _units_root;
	public Gtk.TreeRowReference _sounds_root;

	public signal void selection_changed(Gee.ArrayList<Guid?> selection);

	public LevelTreeView(Database db, Level level)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

		// Data
		_level = level;

		_db = db;

		// Widgets
		_needle = "";
		_filter_entry = new EntrySearch();
		_filter_entry.set_placeholder_text("Search...");
		_filter_entry.search_changed.connect(on_filter_entry_text_changed);
		_filter_entry._entry.stop_search.connect(on_stop_search);

		_tree_store = new Gtk.TreeStore(Column.COUNT
			, typeof(int)    // Column.TYPE
			, typeof(Guid)   // Column.GUID
			, typeof(string) // Column.NAME
			, typeof(bool)   // Column.VISIBLE
			, typeof(uint32) // Column.SAVED_STATE
			);

		_tree_filter = new Gtk.TreeModelFilter(_tree_store, null);
		_tree_filter.set_visible_column(Column.VISIBLE);

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
			mi.set_action_and_target_value("database.duplicate", null);
			menu_model.append_item(mi);

			mi = new GLib.MenuItem("Delete", null);
			mi.set_action_and_target_value("database.delete", null);
			menu_model.append_item(mi);

			if (_tree_selection.count_selected_rows() == 1) {
				GLib.List<Gtk.TreePath> selected_paths = _tree_selection.get_selected_rows(null);

				Gtk.TreeIter iter;
				if (_tree_view.model.get_iter(out iter, selected_paths.nth(0).data)) {
					Value val;
					_tree_view.model.get_value(iter, Column.GUID, out val);
					Guid object_id = (Guid)val;
					_tree_view.model.get_value(iter, Column.NAME, out val);
					string name = (string)val;

					if (_db.object_type(object_id) == OBJECT_TYPE_UNIT) {
						mi = new GLib.MenuItem("Save as Prefab...", null);
						mi.set_action_and_target_value("app.unit-save-as-prefab", new GLib.Variant.tuple({ object_id.to_string(), name }));
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
		} else if (_gesture_click.get_current_button() == Gdk.BUTTON_PRIMARY && n_press == 2) {
			GLib.Application.get_default().activate_action("camera-frame-selected", null);
		}
	}

	public void on_tree_selection_changed()
	{
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

		selection_changed(ids);
	}

	public void read_selection(Guid?[] selection)
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

	public void on_objects_changed(Guid?[] object_ids, uint32 flags)
	{
		foreach (var id in object_ids) {
			_tree_sort.foreach ((model, path, iter) => {
					Value type;
					model.get_value(iter, Column.TYPE, out type);
					if ((int)type == ItemType.FOLDER)
						return false;

					Value guid;
					model.get_value(iter, Column.GUID, out guid);
					Guid guid_model = (Guid)guid;

					if (guid_model == id) {
						Gtk.TreeIter iter_filter;
						Gtk.TreeIter iter_model;
						_tree_sort.convert_iter_to_child_iter(out iter_filter, iter);
						_tree_filter.convert_iter_to_child_iter(out iter_model, iter_filter);

						_tree_store.set(iter_model
							, Column.NAME
							, _db.name(id)
							, -1
							);
						return true;
					}

					return false;
				});
		}
	}

	public ItemType item_type(Unit u)
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
			, Column.VISIBLE
			, true
			, Column.SAVE_STATE
			, 0u
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
			, Column.VISIBLE
			, true
			, Column.SAVE_STATE
			, 0u
			, -1
			);
		_sounds_root = new Gtk.TreeRowReference(_tree_store, _tree_store.get_path(iter));

		_tree_view.model = _tree_sort;

		_level = level;
		on_objects_created(_db.get_set(_level._id, "units", new Gee.HashSet<Guid?>()).to_array());
		on_objects_created(_db.get_set(_level._id, "sounds", new Gee.HashSet<Guid?>()).to_array());

		_tree_view.expand_all();
	}

	public int insert_units(Guid?[] object_ids)
	{
		Gtk.TreeIter iter;
		int i;

		if (object_ids.length > 1 && Unit.is_component(object_ids[1], _db)) {
			for (i = 1; i < object_ids.length; ++i) {
				if (!Unit.is_component(object_ids[i], _db))
					break;
			}
		} else {
			for (i = 0; i < object_ids.length; ++i) {
				if (_db.object_type(object_ids[i]) != OBJECT_TYPE_UNIT)
					break;

				Unit u = Unit(_level._db, object_ids[i]);
				Gtk.TreeIter units_iter;

				_tree_store.get_iter(out units_iter, _units_root.get_path());
				_tree_store.insert_with_values(out iter
					, units_iter
					, -1
					, Column.TYPE
					, item_type(u)
					, Column.GUID
					, u._id
					, Column.NAME
					, _db.name(u._id)
					, Column.VISIBLE
					, true
					, Column.SAVE_STATE
					, 0u
					, -1
					);
			}
		}

		return i;
	}

	public int insert_sounds(Guid?[] object_ids)
	{
		int i;

		for (i = 0; i < object_ids.length; ++i) {
			if (_db.object_type(object_ids[i]) != OBJECT_TYPE_SOUND_SOURCE)
				break;

			Gtk.TreeIter sounds_iter;
			Gtk.TreeIter iter;

			_tree_store.get_iter(out sounds_iter, _sounds_root.get_path());
			_tree_store.insert_with_values(out iter
				, sounds_iter
				, -1
				, Column.TYPE
				, ItemType.SOUND
				, Column.GUID
				, object_ids[i]
				, Column.NAME
				, _db.name(object_ids[i])
				, Column.VISIBLE
				, true
				, Column.SAVE_STATE
				, 0u
				, -1
				);
		}

		return i;
	}

	public void on_objects_created(Guid?[] object_ids, uint32 flags = 0)
	{
		int i = 0;
		while (i < object_ids.length) {
			if (_db.object_type(object_ids[i]) == OBJECT_TYPE_UNIT) {
				i += insert_units(object_ids[i:object_ids.length]);
			} else if (_db.object_type(object_ids[i]) == OBJECT_TYPE_SOUND_SOURCE) {
				i += insert_sounds(object_ids[i:object_ids.length]);
			} else {
				++i; // Skip object.
			}
		}

		_tree_filter.refilter();
	}

	public int remove_units(Guid?[] object_ids)
	{
		int i;

		_tree_selection.changed.disconnect(on_tree_selection_changed);

		if (object_ids.length > 1 && Unit.is_component(object_ids[1], _db)) {
			for (i = 1; i < object_ids.length; ++i) {
				if (!Unit.is_component(object_ids[i], _db))
					break;
			}
		} else {
			for (i = 0; i < object_ids.length; ++i) {
				if (_db.object_type(object_ids[i]) != OBJECT_TYPE_UNIT)
					break;

				Gtk.TreeIter parent_iter;
				_tree_store.get_iter(out parent_iter, _units_root.get_path());
				remove_item(object_ids[i], parent_iter);
			}
		}

		_tree_selection.changed.connect(on_tree_selection_changed);
		on_tree_selection_changed();

		return i;
	}

	public int remove_sounds(Guid?[] object_ids)
	{
		int i;

		_tree_selection.changed.disconnect(on_tree_selection_changed);

		for (i = 0; i < object_ids.length; ++i) {
			if (_db.object_type(object_ids[i]) != OBJECT_TYPE_SOUND_SOURCE)
				break;

			Gtk.TreeIter parent_iter;
			_tree_store.get_iter(out parent_iter, _sounds_root.get_path());
			remove_item(object_ids[i], parent_iter);
		}

		_tree_selection.changed.connect(on_tree_selection_changed);
		on_tree_selection_changed();

		return i;
	}

	public void on_objects_destroyed(Guid?[] object_ids, uint32 flags = 0)
	{
		int i = 0;

		while (i < object_ids.length) {
			if (_db.object_type(object_ids[i]) == OBJECT_TYPE_UNIT) {
				i += remove_units(object_ids[i:object_ids.length]);
			} else if (_db.object_type(object_ids[i]) == OBJECT_TYPE_SOUND_SOURCE) {
				i += remove_sounds(object_ids[i:object_ids.length]);
			} else {
				++i; // Skip object.
			}
		}
	}

	public void remove_item(Guid id, Gtk.TreeIter parent_iter)
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

	public Gtk.RadioButton add_sort_item(Gtk.RadioButton? group, SortMode mode)
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
				int type;
				string name;

				Value val;
				model.get_value(iter, Column.TYPE, out val);
				type = (int)val;

				bool visible = false;

				// Always show the roots.
				if (type == ItemType.FOLDER) {
					visible = true;
				} else {
					model.get_value(iter, Column.NAME, out val);
					name = (string)val;

					visible = needle == "" || name.down().index_of(needle) > -1;
				}

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
