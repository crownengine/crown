/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class ObjectTree : Gtk.Box
{
#if CROWN_GTK3
	public const Gtk.TargetEntry[] DND_TARGETS =
	{
		{ "GUID", Gtk.TargetFlags.SAME_APP, TargetInfo.GUID },
	};
#endif
	public enum ItemType
	{
		OBJECT,
		OBJECTS_SET
	}

	public struct SetRowKey
	{
		public Guid id;
		public int set_index;

		public SetRowKey(Guid id, int set_index)
		{
			this.id = id;
			this.set_index = set_index;
		}

		public static uint hash_func(SetRowKey? key)
		{
			SetRowKey k = (SetRowKey)key;
			return Guid.hash_func(k.id) ^ ((uint)k.set_index * 0x9e3779b9u);
		}

		public static bool equal_func(SetRowKey? a, SetRowKey? b)
		{
			SetRowKey ka = (SetRowKey)a;
			SetRowKey kb = (SetRowKey)b;
			return Guid.equal_func(ka.id, kb.id) && ka.set_index == kb.set_index;
		}
	}

	public enum Column
	{
		ITEM_TYPE,   ///< enum ItemType.
		OBJECT_ID,   ///< Object ID that owns the property.
		OBJECT_NAME, ///< Object name for displaying.
		SET_INDEX,   ///< Index of the set property.
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
				return _("Name A-Z");
			case NAME_ZA:
				return _("Name Z-A");
			case TYPE_AZ:
				return _("Type A-Z");
			case TYPE_ZA:
				return _("Type Z-A");
			default:
				return _("Unknown");
			}
		}
	}

	public DatabaseEditor _database_editor;
	public Database _database;
	public Guid _object_id;
	public GLib.HashTable<Guid?, Gtk.TreeIter?> _object_rows; // Gtk.TreeStore guarantees persistent iters while their rows exist.
	public GLib.HashTable<SetRowKey?, Gtk.TreeIter?> _set_rows;
	public string _needle;
	public EntrySearch _filter_entry;
	public Gtk.TreeStore _tree_store;
	public Gtk.TreeModelFilter _tree_filter;
	public Gtk.TreeModelSort _tree_sort;
	public Gtk.TreeView _tree_view;
	public Gtk.TreeSelection _tree_selection;
	public ulong _database_selection_changed_id;
	public ulong _selection_changed_id;
	public GLib.List<Gtk.TreePath>? _saved_paths;
	public bool _drag_started;
	public bool _selection_changed_blocked;
	public bool _search_selection_changed;
	public Gtk.TreePath? _drag_path;
	public double _press_x;
	public double _press_y;
	public Gtk.ScrolledWindow _scrolled_window;
	public Gtk.Box _sort_items_box;
	public Gtk.Popover _sort_items_popover;
	public Gtk.MenuButton _sort_items;
	public Gtk.GestureSingle _gesture_click;
#if !CROWN_GTK3
	public Gtk.DragSource _drag_source;
#endif

	public ObjectTree(DatabaseEditor database_editor)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

		_database_editor = database_editor;
		_database_selection_changed_id = _database_editor.selection_changed.connect(on_database_selection_changed);
		_database = database_editor._database;
		_object_id = GUID_ZERO;
		_object_rows = new GLib.HashTable<Guid?, Gtk.TreeIter?>(Guid.hash_func, Guid.equal_func);
		_set_rows = new GLib.HashTable<SetRowKey?, Gtk.TreeIter?>(SetRowKey.hash_func, SetRowKey.equal_func);

		_needle = "";
		_filter_entry = new EntrySearch();
		_filter_entry.set_placeholder_text(_("Search..."));
		_filter_entry.search_changed.connect(on_filter_entry_text_changed);
		_filter_entry._entry.stop_search.connect(on_stop_search);

		_tree_store = new_tree_store();
		create_models(Column.OBJECT_NAME, Gtk.SortType.ASCENDING);

		Gtk.TreeViewColumn column = new Gtk.TreeViewColumn();
		Gtk.CellRendererPixbuf cell_pixbuf = new Gtk.CellRendererPixbuf();
		Gtk.CellRendererText cell_text = new Gtk.CellRendererText();
		column.pack_start(cell_pixbuf, false);
		column.pack_start(cell_text, true);
		column.set_cell_data_func(cell_pixbuf, render_item_icon);
		column.add_attribute(cell_text, "text", Column.OBJECT_NAME);
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

#if CROWN_GTK3
		Gtk.drag_source_set(_tree_view, Gdk.ModifierType.BUTTON1_MASK, DND_TARGETS, Gdk.DragAction.COPY);
		_tree_view.drag_begin.connect(on_drag_begin);
		_tree_view.drag_data_get.connect(on_drag_data_get);

		_gesture_click = new Gtk.GestureMultiPress(_tree_view);
		((Gtk.GestureMultiPress)_gesture_click).pressed.connect(on_button_pressed);
		((Gtk.GestureMultiPress)_gesture_click).released.connect(on_button_released);
#else
		_drag_source = new Gtk.DragSource();
		_drag_source.actions = Gdk.DragAction.COPY;
		_drag_source.prepare.connect(on_drag_source_prepare);
		_tree_view.add_controller(_drag_source);

		_gesture_click = new Gtk.GestureClick();
		((Gtk.GestureClick)_gesture_click).pressed.connect(on_button_pressed);
		((Gtk.GestureClick)_gesture_click).released.connect(on_button_released);
		_tree_view.add_controller(_gesture_click);
#endif /* if CROWN_GTK3 */
		_gesture_click.set_propagation_phase(Gtk.PropagationPhase.CAPTURE);
		_gesture_click.set_button(0);

		_tree_selection = _tree_view.get_selection();
		_tree_selection.set_mode(Gtk.SelectionMode.MULTIPLE);
		_selection_changed_id = _tree_selection.changed.connect(on_tree_selection_changed);
		_selection_changed_blocked = false;
		_search_selection_changed = false;

#if CROWN_GTK3
		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.add(_tree_view);
#else
		_scrolled_window = new Gtk.ScrolledWindow();
		_scrolled_window.set_child(_tree_view);
		_scrolled_window.vexpand = true;
#endif

		// Setup sort menu button popover.
		_sort_items_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
#if CROWN_GTK3
		_sort_items_popover = new Gtk.Popover(null);
		_sort_items_popover.add(_sort_items_box);
#else
		_sort_items_popover = new Gtk.Popover();
		_sort_items_popover.set_child(_sort_items_box);
#endif
		_sort_items = new Gtk.MenuButton();
		_sort_items.set_tooltip_text(_("Sort items."));
#if CROWN_GTK3
		_sort_items.add(new Gtk.Image.from_icon_name(IconTheme.LIST_SORT, Gtk.IconSize.SMALL_TOOLBAR));
		_sort_items.get_style_context().add_class("flat");
		_sort_items.get_style_context().add_class("image-button");
		_sort_items.can_focus = false;
#else
		_sort_items.set_child(new Gtk.Image.from_icon_name(IconTheme.LIST_SORT));
		_sort_items.add_css_class("flat");
		_sort_items.add_css_class("image-button");
		_sort_items.focusable = false;
#endif
		_sort_items.set_popover(_sort_items_popover);

#if CROWN_GTK3
		Gtk.RadioButton? button = null;
#else
		Gtk.CheckButton? button = null;
#endif
		for (int i = 0; i < SortMode.COUNT; ++i) {
			button = add_sort_item(button, (SortMode)i);
#if !CROWN_GTK3
			if (i == SortMode.NAME_AZ)
				button.set_active(true);
#endif
		}

#if CROWN_GTK3
		_sort_items_box.show_all();
#endif

		var tree_control = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
#if CROWN_GTK3
		tree_control.pack_start(_filter_entry, true, true);
		tree_control.pack_end(_sort_items, false, false);

		this.pack_start(tree_control, false, true, 0);
		this.pack_start(_scrolled_window, true, true, 0);
#else
		tree_control.append(_filter_entry);
		tree_control.append(_sort_items);

		this.append(tree_control);
		this.append(_scrolled_window);
#endif
	}

	public Gtk.TreeStore new_tree_store()
	{
		return new Gtk.TreeStore(Column.COUNT
			, typeof(ItemType) // Column.ITEM_TYPE
			, typeof(Guid)     // Column.OBJECT_ID
			, typeof(string)   // Column.OBJECT_NAME
			, typeof(int)      // Column.SET_INDEX
			, typeof(bool)     // Column.VISIBLE
			, typeof(uint32)   // Column.SAVE_STATE
			);
	}

	public void create_models(int sort_column_id, Gtk.SortType sort_order)
	{
		_tree_filter = new Gtk.TreeModelFilter(_tree_store, null);
		_tree_filter.set_visible_column(Column.VISIBLE);
		_tree_sort = new Gtk.TreeModelSort.with_model(_tree_filter);
		_tree_sort.set_sort_column_id(sort_column_id, sort_order);
	}

	public void render_item_icon(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value val;
		model.get_value(iter, Column.ITEM_TYPE, out val);

		if ((ItemType)val == ItemType.OBJECTS_SET)
			cell.set_property("icon-name", IconTheme.BROWSER_FOLDER);
		else if ((ItemType)val == ItemType.OBJECT)
			cell.set_property("icon-name", IconTheme.LEVEL_OBJECT_UNIT);
		else
			cell.set_property("icon-name", IconTheme.LEVEL_OBJECT_UNKNOWN);
	}

	public bool block_tree_selection_changed()
	{
		if (_selection_changed_blocked)
			return false;

		GLib.SignalHandler.block(_tree_selection, _selection_changed_id);
		_selection_changed_blocked = true;
		return true;
	}

	public void unblock_tree_selection_changed()
	{
		assert(_selection_changed_blocked);
		GLib.SignalHandler.unblock(_tree_selection, _selection_changed_id);
		_selection_changed_blocked = false;
	}

	public void on_button_pressed(int n_press, double x, double y)
	{
		uint button = _gesture_click.get_current_button();

		if (button == Gdk.BUTTON_PRIMARY) {
			_saved_paths = _tree_selection.get_selected_rows(null);
			_press_x = x;
			_press_y = y;
			_drag_started = false;
			block_tree_selection_changed();
		}

		if (button == Gdk.BUTTON_SECONDARY) {
			int bx;
			int by;
			Gtk.TreePath path;
			_tree_view.convert_widget_to_bin_window_coords((int)x, (int)y, out bx, out by);
			if (!_tree_view.get_path_at_pos(bx, by, out path, null, null, null))
				return; // Clicked on empty space.

			if (!_tree_selection.path_is_selected(path)) {
				_tree_selection.unselect_all();
				_tree_selection.select_path(path);
			}

			GLib.Menu menu_model = new GLib.Menu();
			GLib.List<Gtk.TreePath> selected_paths = _tree_selection.get_selected_rows(null);
			uint selected_count = selected_paths.length();
			bool can_modify_objects = selected_count > 0;
			Guid set_owner_id = GUID_ZERO;
			int set_index = -1;

			foreach (Gtk.TreePath selected_path in selected_paths) {
				Gtk.TreeIter iter;
				if (!_tree_view.model.get_iter(out iter, selected_path)) {
					can_modify_objects = false;
					continue;
				}

				Value val;
				_tree_view.model.get_value(iter, Column.ITEM_TYPE, out val);
				if ((ItemType)val == ItemType.OBJECT) {
					_tree_view.model.get_value(iter, Column.OBJECT_ID, out val);
					if (Guid.equal_func((Guid)val, _object_id))
						can_modify_objects = false;
				} else {
					can_modify_objects = false;
					if (selected_count == 1 && (ItemType)val == ItemType.OBJECTS_SET) {
						_tree_view.model.get_value(iter, Column.OBJECT_ID, out val);
						set_owner_id = (Guid)val;
						_tree_view.model.get_value(iter, Column.SET_INDEX, out val);
						set_index = (int)val;
					}
				}
			}

			if (can_modify_objects) {
				GLib.MenuItem mi = new GLib.MenuItem(_("Duplicate"), null);
				mi.set_action_and_target_value("database.duplicate", null);
				menu_model.append_item(mi);

				mi = new GLib.MenuItem(_("Delete"), null);
				mi.set_action_and_target_value("database.delete", null);
				menu_model.append_item(mi);
			} else if (set_index >= 0) {
				StringId64 object_type = StringId64(_database.object_type(set_owner_id));
				unowned PropertyDefinition[] object_definition = _database.object_definition(object_type);
				assert(set_index < object_definition.length);
				assert(object_definition[set_index].type == PropertyType.OBJECTS_SET);

				GLib.MenuItem mi = new GLib.MenuItem(_("Add"), null);
				mi.set_action_and_target_value("database.add"
					, new GLib.Variant.tuple({ set_owner_id.to_string(), object_definition[set_index].name })
					);
				menu_model.append_item(mi);
			}

#if CROWN_GTK3
			Gtk.Popover menu = new Gtk.Popover.from_model(null, menu_model);
			menu.set_relative_to(_tree_view);
#else
			Gtk.PopoverMenu menu = new Gtk.PopoverMenu.from_model(menu_model);
			menu.set_parent(_tree_view);
#endif
			menu.set_pointing_to({ (int)x, (int)y, 1, 1 });
			menu.set_position(Gtk.PositionType.BOTTOM);
			menu.popup();
			_gesture_click.set_state(Gtk.EventSequenceState.CLAIMED);
		}
	}

	public void on_button_released(int n_press, double x, double y)
	{
		if (!_drag_started) {
			_saved_paths = null;
			if (_selection_changed_blocked)
				unblock_tree_selection_changed();
			_tree_selection.changed();
		}
	}

	public void prepare_drag()
	{
		_drag_started = true;

		int bx;
		int by;
		Gtk.TreePath? path;
		_tree_view.convert_widget_to_bin_window_coords((int)_press_x, (int)_press_y, out bx, out by);
		if (!_tree_view.get_path_at_pos(bx, by, out path, null, null, null))
			path = null;
		_drag_path = path;

		// Defer selection restore so it does not interfere with drag setup.
		GLib.Idle.add(restore_drag_selection);
	}

	public bool restore_drag_selection()
	{
		_tree_selection.unselect_all();
		if (_saved_paths != null) {
			foreach (Gtk.TreePath saved_path in _saved_paths)
				_tree_selection.select_path(saved_path);
		}
		_saved_paths = null;

		if (_selection_changed_blocked)
			unblock_tree_selection_changed();

		return GLib.Source.REMOVE;
	}

	public Guid? drag_guid()
	{
		if (_drag_path == null)
			return null;

		Gtk.TreeIter iter;
		if (_tree_view.model.get_iter(out iter, _drag_path)) {
			Gtk.TreeIter iter_filter;
			Gtk.TreeIter iter_model;
			_tree_sort.convert_iter_to_child_iter(out iter_filter, iter);
			_tree_filter.convert_iter_to_child_iter(out iter_model, iter_filter);

			Value guid_val;
			_tree_store.get_value(iter_model, Column.OBJECT_ID, out guid_val);
			return (Guid)guid_val;
		}

		return null;
	}

#if CROWN_GTK3
	public void on_drag_begin(Gdk.DragContext ctx)
	{
		prepare_drag();
	}
#endif

#if !CROWN_GTK3
	public Gdk.ContentProvider? on_drag_source_prepare(double x, double y)
	{
		prepare_drag();
		Guid? guid = drag_guid();
		if (guid == null)
			return null;
		Guid drag_id = (Guid)guid;

		GLib.Value value = GLib.Value(typeof(string));
		value.set_string(drag_id.to_string());
		return new Gdk.ContentProvider.for_value(value);
	}
#endif

#if CROWN_GTK3
	public void on_drag_data_get(Gdk.DragContext ctx, Gtk.SelectionData data, uint info, uint time_)
	{
		Guid? guid = drag_guid();
		if (guid == null)
			return;
		Guid drag_id = (Guid)guid;

		uint8[] buf = new uint8[sizeof(Guid)];
		Memory.copy(buf, &drag_id, sizeof(Guid));
		data.set(data.get_target(), 8, buf);
	}
#endif

	public void on_tree_selection_changed()
	{
		if (_needle != "")
			_search_selection_changed = true;

		GLib.GenericArray<Guid?> ids = new GLib.GenericArray<Guid?>();
		GLib.List<Gtk.TreePath> selected_paths = _tree_selection.get_selected_rows(null);
		foreach (Gtk.TreePath path in selected_paths) {
			Gtk.TreeIter iter;
			if (!_tree_view.model.get_iter(out iter, path))
				continue;

			Value val;
			_tree_view.model.get_value(iter, Column.ITEM_TYPE, out val);
			if ((ItemType)val != ItemType.OBJECT)
				continue;

			_tree_view.model.get_value(iter, Column.OBJECT_ID, out val);
			ids.add((Guid)val);
		}

		GLib.SignalHandler.block(_database_editor, _database_selection_changed_id);
		_database_editor.selection_set(ids.data);
		GLib.SignalHandler.unblock(_database_editor, _database_selection_changed_id);
	}

#if CROWN_GTK3
	public Gtk.RadioButton add_sort_item(Gtk.RadioButton? group, SortMode mode)
#else
	public Gtk.CheckButton add_sort_item(Gtk.CheckButton? group, SortMode mode)
#endif
	{
#if CROWN_GTK3
		var button = new Gtk.RadioButton.with_label_from_widget(group, mode.to_label());
#else
		var button = new Gtk.CheckButton.with_label(mode.to_label());
		button.set_group(group);
#endif
		switch (mode) {
		case SortMode.NAME_AZ:
			button.toggled.connect(on_sort_name_az_toggled);
			break;
		case SortMode.NAME_ZA:
			button.toggled.connect(on_sort_name_za_toggled);
			break;
		case SortMode.TYPE_AZ:
			button.toggled.connect(on_sort_type_az_toggled);
			break;
		case SortMode.TYPE_ZA:
			button.toggled.connect(on_sort_type_za_toggled);
			break;
			default:
			assert(false);
			break;
		}

#if CROWN_GTK3
		_sort_items_box.pack_start(button, false, false);
#else
		_sort_items_box.append(button);
#endif
		return button;
	}

	public void set_sort(int column, Gtk.SortType order)
	{
		_tree_sort.set_sort_column_id(column, order);
		_sort_items_popover.popdown();
	}

	public void on_sort_name_az_toggled()
	{
		set_sort(Column.OBJECT_NAME, Gtk.SortType.ASCENDING);
	}

	public void on_sort_name_za_toggled()
	{
		set_sort(Column.OBJECT_NAME, Gtk.SortType.DESCENDING);
	}

	public void on_sort_type_az_toggled()
	{
		set_sort(Column.ITEM_TYPE, Gtk.SortType.ASCENDING);
	}

	public void on_sort_type_za_toggled()
	{
		set_sort(Column.ITEM_TYPE, Gtk.SortType.DESCENDING);
	}

	public string object_display_name(Guid id)
	{
		StringId64 object_type = StringId64(_database.object_type(id));
		Aspect? name_aspect = _database.get_aspect(object_type, StringId64("name"));
		if (name_aspect == null)
			name_aspect = default_name_aspect;

		string object_name;
		name_aspect(out object_name, _database, id);
		return object_name;
	}

	public void unregister_subtree(Gtk.TreeIter iter)
	{
		Gtk.TreeIter child;
		if (_tree_store.iter_children(out child, iter)) {
			do {
				unregister_subtree(child);
			} while (_tree_store.iter_next(ref child));
		}

		Value val;
		_tree_store.get_value(iter, Column.ITEM_TYPE, out val);
		ItemType item_type = (ItemType)val;

		_tree_store.get_value(iter, Column.OBJECT_ID, out val);
		Guid id = (Guid)val;

		if (item_type == ItemType.OBJECT) {
			_object_rows.remove(id);
		} else if (item_type == ItemType.OBJECTS_SET) {
			_tree_store.get_value(iter, Column.SET_INDEX, out val);
			_set_rows.remove(SetRowKey(id, (int)val));
		}
	}

	public void insert_object_child(Gtk.TreeIter set_iter, Guid child_id)
	{
		if (_object_rows.lookup(child_id) != null)
			return;

		Gtk.TreeIter child_iter;
		_tree_store.insert_with_values(out child_iter
			, set_iter
			, 0 // GtkTreeStore append is quadratic for many siblings. Prepend instead.
			, Column.ITEM_TYPE
			, ItemType.OBJECT
			, Column.OBJECT_ID
			, child_id
			, Column.OBJECT_NAME
			, object_display_name(child_id)
			, Column.VISIBLE
			, true
			, -1
			);
		_object_rows[child_id] = child_iter;
		add_object_set(child_iter, child_id);
	}

	// Synchronize one object's parent without scanning the displayed tree.
	public void sync_object_placement(Guid id)
	{
		if (id == _object_id)
			return;

		SetRowKey desired = SetRowKey(GUID_ZERO, -1);
		bool has_desired = false;
		Guid owner_id = _database.owner(id);
		if (owner_id != GUID_ZERO && _object_rows.lookup(owner_id) != null) {
			StringId64 owner_type = StringId64(_database.object_type(owner_id));
			unowned PropertyDefinition[] object_definition = _database.object_definition(owner_type);
			for (int set_index = 0; set_index < object_definition.length && !has_desired; ++set_index) {
				var def = object_definition[set_index];
				if (def.type != PropertyType.OBJECTS_SET || !_database.has_property(owner_id, def.name))
					continue;

				Guid?[] children = _database.get_set(owner_id, def.name);
				foreach (unowned Guid? child_id in children) {
					if (!Guid.equal_func(child_id, id))
						continue;

					desired = SetRowKey(owner_id, set_index);
					has_desired = true;
					break;
				}
			}
		}

		unowned Gtk.TreeIter? iter_ptr = _object_rows.lookup(id);
		if (iter_ptr != null) {
			Gtk.TreeIter set_iter;
			bool has_parent = _tree_store.iter_parent(out set_iter, (Gtk.TreeIter)iter_ptr);
			assert(has_parent); // The only parentless object row is _object_id.

			Value val;
			_tree_store.get_value(set_iter, Column.OBJECT_ID, out val);
			Guid current_owner = (Guid)val;
			_tree_store.get_value(set_iter, Column.SET_INDEX, out val);
			SetRowKey current = SetRowKey(current_owner, (int)val);

			if (has_desired && SetRowKey.equal_func(current, desired))
				return;

			Gtk.TreeIter iter = (Gtk.TreeIter)iter_ptr;
			unregister_subtree(iter);
			_tree_store.remove(ref iter);
		}

		if (!has_desired)
			return;

		unowned Gtk.TreeIter? set_iter_ptr = _set_rows.lookup(desired);
		if (set_iter_ptr != null)
			insert_object_child((Gtk.TreeIter)set_iter_ptr, id);
	}

	public void on_objects_changed(Guid?[] object_ids, uint32 flags = 0)
	{
		if (_object_id == GUID_ZERO)
			return;

		bool selection_blocked = block_tree_selection_changed();

		foreach (unowned Guid? id in object_ids) {
			sync_object_placement(id);

			unowned Gtk.TreeIter? object_iter_ptr = _object_rows.lookup(id);
			if (object_iter_ptr == null)
				continue;

			StringId64 object_type = StringId64(_database.object_type(id));
			unowned PropertyDefinition[] object_definition = _database.object_definition(object_type);
			for (int set_index = 0; set_index < object_definition.length; ++set_index) {
				var def = object_definition[set_index];
				if (def.type != PropertyType.OBJECTS_SET)
					continue;

				unowned Gtk.TreeIter? set_iter_ptr = _set_rows.lookup(SetRowKey(id, set_index));
				assert(set_iter_ptr != null);
				Gtk.TreeIter set_iter = (Gtk.TreeIter)set_iter_ptr;

				Guid?[] children = {};
				var desired = new GLib.HashTable<Guid?, bool?>(Guid.hash_func, Guid.equal_func);
				if (_database.has_property(id, def.name)) {
					children = _database.get_set(id, def.name);
					foreach (unowned Guid? child_id in children)
						desired.insert(child_id, true);
				}

				Gtk.TreeIter child_iter;
				bool child_valid = _tree_store.iter_children(out child_iter, set_iter);
				while (child_valid) {
					Value val;
					_tree_store.get_value(child_iter, Column.OBJECT_ID, out val);
					if (desired.lookup((Guid)val) == null) {
						unregister_subtree(child_iter);
						child_valid = _tree_store.remove(ref child_iter);
					} else {
						child_valid = _tree_store.iter_next(ref child_iter);
					}
				}

				foreach (unowned Guid? child_id in children)
					insert_object_child(set_iter, child_id);
			}

			if (id != _object_id) {
				Gtk.TreeIter object_iter = (Gtk.TreeIter)object_iter_ptr;
				Value val;
				_tree_store.get_value(object_iter, Column.OBJECT_NAME, out val);
				string name = object_display_name(id);
				if ((string)val != name)
					_tree_store.set(object_iter, Column.OBJECT_NAME, name, -1);
			}
		}

		if (_needle != "")
			filter();

		select_database_rows(false);
		if (selection_blocked)
			unblock_tree_selection_changed();
	}

	public void on_objects_created(Guid?[] object_ids, uint32 flags = 0)
	{
		if (_object_id == GUID_ZERO)
			return;

		bool selection_blocked = block_tree_selection_changed();
		foreach (unowned Guid? id in object_ids)
			sync_object_placement(id);

		if (_needle != "")
			filter();

		select_database_rows(false);
		if (selection_blocked)
			unblock_tree_selection_changed();
	}

	public void on_objects_destroyed(Guid?[] object_ids, uint32 flags = 0)
	{
		if (_object_id == GUID_ZERO)
			return;

		foreach (unowned Guid? id in object_ids) {
			if (id == _object_id) {
				set_object(GUID_ZERO);
				return;
			}
		}

		bool selection_blocked = block_tree_selection_changed();
		foreach (unowned Guid? id in object_ids) {
			unowned Gtk.TreeIter? iter_ptr = _object_rows.lookup(id);
			if (iter_ptr == null)
				continue;

			Gtk.TreeIter iter = (Gtk.TreeIter)iter_ptr;
			unregister_subtree(iter);
			_tree_store.remove(ref iter);
		}

		if (_needle != "")
			filter();

		if (selection_blocked)
			unblock_tree_selection_changed();
	}

	public void add_object_set(Gtk.TreeIter parent_iter, Guid id)
	{
		StringId64 object_type = StringId64(_database.object_type(id));
		unowned PropertyDefinition[] object_definition = _database.object_definition(object_type);

		for (int set_index = 0; set_index < object_definition.length; ++set_index) {
			var def = object_definition[set_index];
			if (def.type != PropertyType.OBJECTS_SET)
				continue;

			Gtk.TreeIter set_iter;
			_tree_store.insert_with_values(out set_iter
				, parent_iter
				, 0
				, Column.ITEM_TYPE
				, ItemType.OBJECTS_SET
				, Column.OBJECT_ID
				, id
				, Column.OBJECT_NAME
				, camel_case(def.label)
				, Column.SET_INDEX
				, set_index
				, Column.VISIBLE
				, true
				, -1
				);
			_set_rows[SetRowKey(id, set_index)] = set_iter;

			if (!_database.has_property(id, def.name))
				continue;

			Guid?[] children = _database.get_set(id, def.name);
			foreach (unowned Guid? child_id in children)
				insert_object_child(set_iter, child_id);
		}
	}

	/// Setting @a id to GUID_ZERO resets the tree.
	public void set_object(Guid id)
	{
		if (id != GUID_ZERO && id == _object_id && _object_rows.lookup(id) != null)
			return;

		int sort_column_id = Column.OBJECT_NAME;
		Gtk.SortType sort_order = Gtk.SortType.ASCENDING;
		_tree_sort.get_sort_column_id(out sort_column_id, out sort_order);

		// Populate a bare TreeStore.
		_tree_view.model = null;
		_object_rows.remove_all();
		_set_rows.remove_all();
		_tree_store = new_tree_store();
		_object_id = id;

		if (id != GUID_ZERO) {
			ObjectTypeInfo info = _database.type_info(StringId64(_database.object_type(id)));

			Gtk.TreeIter object_iter;
			_tree_store.insert_with_values(out object_iter
				, null
				, 0
				, Column.ITEM_TYPE
				, ItemType.OBJECT
				, Column.OBJECT_ID
				, id
				, Column.OBJECT_NAME
				, info.ui_name
				, Column.VISIBLE
				, true
				, -1
				);
			_object_rows[id] = object_iter;
			add_object_set(object_iter, id);
		}

		create_models(sort_column_id, sort_order);
		_tree_view.model = _tree_sort;

		if (id != GUID_ZERO) {
			unowned Gtk.TreeIter? object_iter_ptr = _object_rows.lookup(id);
			assert(object_iter_ptr != null);
			Gtk.TreePath child_path = _tree_store.get_path((Gtk.TreeIter)object_iter_ptr);
			Gtk.TreePath filter_path = _tree_filter.convert_child_path_to_path(child_path);
			Gtk.TreePath sort_path = _tree_sort.convert_child_path_to_path(filter_path);
			_tree_view.expand_to_path(sort_path);
		}
	}

	public void select_database_rows(bool scroll_to_selection = true)
	{
		GLib.GenericArray<Guid?> selection = _database_editor._selection;
		Gtk.TreePath? last_selected = null;

		for (int i = 0; i < selection.length; ++i) {
			Guid? id = selection[i];
			unowned Gtk.TreeIter? iter_ptr = _object_rows.lookup(id);
			if (iter_ptr == null)
				continue;

			Gtk.TreeIter iter = (Gtk.TreeIter)iter_ptr;
			Gtk.TreePath child_path = _tree_store.get_path(iter);
			Gtk.TreePath filter_path = _tree_filter.convert_child_path_to_path(child_path);
			if (filter_path == null)
				continue;

			Gtk.TreePath sort_path = _tree_sort.convert_child_path_to_path(filter_path);
			if (sort_path == null)
				continue;

			_tree_selection.select_path(sort_path);
			last_selected = sort_path;
		}

		if (scroll_to_selection && last_selected != null)
			_tree_view.scroll_to_cell(last_selected, null, false, 0.0f, 0.0f);
	}

	public void on_database_selection_changed()
	{
		if (_needle != "")
			_search_selection_changed = true;

		bool selection_blocked = block_tree_selection_changed();
		_tree_selection.unselect_all();
		select_database_rows();
		if (selection_blocked)
			unblock_tree_selection_changed();
	}

	public bool save_tree_state(Gtk.TreeModel model, Gtk.TreePath path, Gtk.TreeIter iter)
	{
		Gtk.TreePath filter_path = _tree_filter.convert_child_path_to_path(path);
		if (filter_path == null)
			return false;

		Gtk.TreePath sort_path = _tree_sort.convert_child_path_to_path(filter_path);
		if (sort_path == null)
			return false;

		uint32 state = 0u;
		if (_tree_view.is_row_expanded(sort_path))
			state |= 0x1;
		if (_tree_selection.path_is_selected(sort_path))
			state |= 0x2;
		_tree_store.set(iter, Column.SAVE_STATE, state, -1);
		return false;
	}

	public bool restore_tree_state(Gtk.TreeModel model, Gtk.TreePath path, Gtk.TreeIter iter)
	{
		Value val;
		_tree_store.get_value(iter, Column.SAVE_STATE, out val);
		uint32 state = (uint32)val;
		if (state == 0u)
			return false;

		Gtk.TreePath filter_path = _tree_filter.convert_child_path_to_path(path);
		if (filter_path == null)
			return false;

		Gtk.TreePath sort_path = _tree_sort.convert_child_path_to_path(filter_path);
		if (sort_path == null)
			return false;

		if ((state & 0x1) != 0)
			_tree_view.expand_to_path(sort_path);
		if ((state & 0x2) != 0)
			_tree_selection.select_path(sort_path);
		return false;
	}

	public bool hide_tree_row(Gtk.TreeModel model, Gtk.TreePath path, Gtk.TreeIter iter)
	{
		_tree_store.set(iter, Column.VISIBLE, false, -1);
		return false;
	}

	public bool show_tree_row(Gtk.TreeModel model, Gtk.TreePath path, Gtk.TreeIter iter)
	{
		_tree_store.set(iter, Column.VISIBLE, true, -1);
		return false;
	}

	public bool filter_tree_row(Gtk.TreeModel model, Gtk.TreePath path, Gtk.TreeIter iter)
	{
		Value val;
		model.get_value(iter, Column.OBJECT_NAME, out val);
		if (((string)val).down().index_of(_needle) < 0)
			return false;

		Gtk.TreeIter visible_iter = iter;
		_tree_store.set(visible_iter, Column.VISIBLE, true, -1);
		while (_tree_store.iter_parent(out visible_iter, visible_iter))
			_tree_store.set(visible_iter, Column.VISIBLE, true, -1);
		return false;
	}

	public void filter()
	{
		assert(_needle != "");
		_tree_store.foreach(hide_tree_row);
		_tree_store.foreach(filter_tree_row);
		_tree_filter.refilter();
		_tree_view.expand_all();
	}

	public void on_filter_entry_text_changed()
	{
		string old_needle = _needle;
		_needle = _filter_entry.text.strip().down();

		if (old_needle == "" && _needle != "") {
			_search_selection_changed = false;
			bool selection_blocked = block_tree_selection_changed();
			_tree_store.foreach(save_tree_state);
			filter();
			if (selection_blocked)
				unblock_tree_selection_changed();
			return;
		}

		if (old_needle != "" && _needle == "") {
			bool selection_blocked = block_tree_selection_changed();
			_tree_store.foreach(show_tree_row);
			_tree_filter.refilter();
			_tree_selection.unselect_all();
			_tree_store.foreach(restore_tree_state);

			if (_search_selection_changed) {
				_tree_selection.unselect_all();
				select_database_rows();
			}
			_search_selection_changed = false;

			if (selection_blocked)
				unblock_tree_selection_changed();
			return;
		}

		if (_needle != "") {
			bool selection_blocked = block_tree_selection_changed();
			filter();
			if (selection_blocked)
				unblock_tree_selection_changed();
		}
	}

	public void on_stop_search()
	{
		_filter_entry._entry.set_text("");
	}
}

} /* namespace Crown */
