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

	public delegate bool ObjectVisibleDelegate(Guid id);
	public delegate void SetObjectVisibleDelegate(Guid id, bool visible);
	public delegate bool SelectionLockedDelegate(Guid id);
	public delegate void SetSelectionLockedDelegate(Guid id, bool locked);
	public delegate void ObjectAspectDelegate(Guid id, out int kind, out string icon_name);
	public delegate void ContextMenuDelegate(GLib.Menu menu, Guid?[] selection);

	public struct FlattenedSetKey
	{
		public StringId64 object_type;
		public int set_index;

		public FlattenedSetKey(StringId64 object_type, int set_index)
		{
			this.object_type = object_type;
			this.set_index = set_index;
		}
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
		ITEM_TYPE,        ///< enum ItemType.
		OBJECT_ID,        ///< Object ID that owns the property.
		OBJECT_NAME,      ///< Object name for displaying.
		OBJECT_KIND,      ///< Optional kind key for finer sorting.
		OBJECT_ICON,      ///< Optional icon name.
		SET_INDEX,        ///< Index of the set property.
		VISIBLE,          ///< Search-filter visibility.
		OBJECT_VISIBLE,   ///< Optional object visibility state.
		SELECTION_LOCKED, ///< Optional selection-lock state.
		SAVE_STATE,

		COUNT
	}

	public enum SortMode
	{
		NAME,
		TYPE,

		COUNT;

		public string to_label()
		{
			switch (this) {
			case NAME:
				return _("Name");
			case TYPE:
				return _("Type");
			default:
				return _("Unknown");
			}
		}
	}

	public DatabaseEditor _database_editor;
	public Database _database;
	public Guid _object_id;
	public GLib.HashTable<Guid?, Gtk.TreeIter?> _object_rows; // Gtk.TreeStore guarantees persistent iters while their rows exist.
	public GLib.HashTable<Guid?, SetRowKey?> _object_sets;
	public GLib.HashTable<SetRowKey?, Gtk.TreeIter?> _set_rows;
	public unowned ObjectVisibleDelegate? _object_visible_func;
	public unowned SetObjectVisibleDelegate? _set_object_visible_func;
	public unowned SelectionLockedDelegate? _selection_locked_func;
	public unowned SetSelectionLockedDelegate? _set_selection_locked_func;
	public unowned ObjectAspectDelegate? _object_aspect_func;
	public unowned ContextMenuDelegate? _context_menu_func;
	private FlattenedSetKey[] _flattened_sets;
	public bool show_root;
	public bool expand_all_on_load;
	public string _needle;
	public EntrySearch _filter_entry;
	public Gtk.TreeStore _tree_store;
	public Gtk.TreeModelFilter _tree_filter;
	public Gtk.TreeModelSort _tree_sort;
	public Gtk.TreeView _tree_view;
	public Gtk.TreeSelection _tree_selection;
	public Gtk.TreeViewColumn _visibility_column;
	public Gtk.TreeViewColumn _lock_column;
	public Gtk.TreeViewColumn? _toggle_drag_column;
	public bool _toggle_drag_state;
	public GLib.GenericArray<Guid?> _toggle_drag_changed;
	public double _toggle_drag_last_x;
	public double _toggle_drag_last_y;
	public ulong _database_selection_changed_id;
	public ulong _selection_changed_id;
	private bool _setting_database_selection;
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
	public Gtk.CheckButton _reverse_sort;
	public Gtk.GestureSingle _gesture_click;
#if !CROWN_GTK3
	public Gtk.DragSource _drag_source;
#endif

	public signal void selection_changed(Guid?[] selection);
	public signal void object_activated(Guid id);
	public signal void objects_edited(Guid?[] object_ids);

	public ObjectTree(DatabaseEditor database_editor)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

		_database_editor = database_editor;
		_database_selection_changed_id = _database_editor.selection_changed.connect(on_database_selection_changed);
		_database = database_editor._database;
		_object_id = GUID_ZERO;
		_object_rows = new GLib.HashTable<Guid?, Gtk.TreeIter?>(Guid.hash_func, Guid.equal_func);
		_object_sets = new GLib.HashTable<Guid?, SetRowKey?>(Guid.hash_func, Guid.equal_func);
		_set_rows = new GLib.HashTable<SetRowKey?, Gtk.TreeIter?>(SetRowKey.hash_func, SetRowKey.equal_func);
		_object_visible_func = null;
		_set_object_visible_func = null;
		_selection_locked_func = null;
		_set_selection_locked_func = null;
		_object_aspect_func = null;
		_context_menu_func = null;
		_flattened_sets = {};
		show_root = true;
		expand_all_on_load = false;
		_toggle_drag_column = null;
		_toggle_drag_state = false;
		_toggle_drag_changed = new GLib.GenericArray<Guid?>();
		_toggle_drag_last_x = 0.0;
		_toggle_drag_last_y = 0.0;
		_setting_database_selection = false;

		_needle = "";
		_filter_entry = new EntrySearch();
		_filter_entry.set_placeholder_text(_("Search..."));
		_filter_entry.search_changed.connect(on_filter_entry_text_changed);
		_filter_entry._entry.stop_search.connect(on_stop_search);

		_tree_store = new_tree_store();
		create_models(Column.OBJECT_NAME, Gtk.SortType.ASCENDING);

		Gtk.TreeViewColumn column = new Gtk.TreeViewColumn();
		column.expand = true;
		Gtk.CellRendererPixbuf cell_pixbuf = new Gtk.CellRendererPixbuf();
		Gtk.CellRendererText cell_text = new Gtk.CellRendererText();
		column.pack_start(cell_pixbuf, false);
		column.pack_start(cell_text, true);
		column.set_cell_data_func(cell_pixbuf, render_item_icon);
		column.add_attribute(cell_text, "text", Column.OBJECT_NAME);
		_tree_view = new Gtk.TreeView();
		_tree_view.append_column(column);

		_visibility_column = new Gtk.TreeViewColumn();
		_visibility_column.sizing = Gtk.TreeViewColumnSizing.FIXED;
		_visibility_column.fixed_width = 18;
		_visibility_column.visible = false;
		Gtk.CellRendererPixbuf cell_visibility = new Gtk.CellRendererPixbuf();
		cell_visibility.set_property("xpad", 0u);
		_visibility_column.pack_start(cell_visibility, false);
		_visibility_column.set_cell_data_func(cell_visibility, render_object_visibility);
		_tree_view.append_column(_visibility_column);

		_lock_column = new Gtk.TreeViewColumn();
		_lock_column.sizing = Gtk.TreeViewColumnSizing.FIXED;
		_lock_column.fixed_width = 34;
		_lock_column.visible = false;
		Gtk.CellRendererPixbuf cell_lock = new Gtk.CellRendererPixbuf();
		cell_lock.set_property("xpad", 0u);
		cell_lock.set_property("xalign", 0.0f);
		_lock_column.pack_start(cell_lock, false);
		_lock_column.set_cell_data_func(cell_lock, render_selection_lock);
		_tree_view.append_column(_lock_column);
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
		_tree_view.has_tooltip = true;
		_tree_view.query_tooltip.connect(on_query_tooltip);

#if CROWN_GTK3
		Gtk.drag_source_set(_tree_view, Gdk.ModifierType.BUTTON1_MASK, DND_TARGETS, Gdk.DragAction.COPY);
		_tree_view.drag_begin.connect(on_drag_begin);
		_tree_view.drag_data_get.connect(on_drag_data_get);

		_gesture_click = new Gtk.GestureMultiPress(_tree_view);
		((Gtk.GestureMultiPress)_gesture_click).pressed.connect(on_button_pressed);
		((Gtk.GestureMultiPress)_gesture_click).released.connect(on_button_released);
		((Gtk.GestureMultiPress)_gesture_click).update.connect(on_button_update);
#else
		_drag_source = new Gtk.DragSource();
		_drag_source.actions = Gdk.DragAction.COPY;
		_drag_source.prepare.connect(on_drag_source_prepare);
		_tree_view.add_controller(_drag_source);

		_gesture_click = new Gtk.GestureClick();
		((Gtk.GestureClick)_gesture_click).pressed.connect(on_button_pressed);
		((Gtk.GestureClick)_gesture_click).released.connect(on_button_released);
		((Gtk.GestureClick)_gesture_click).update.connect(on_button_update);
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
		for (int i = 0; i < SortMode.COUNT; ++i)
			button = add_sort_item(button, (SortMode)i);

		_reverse_sort = new Gtk.CheckButton.with_label(_("Reverse"));
		_reverse_sort.set_active(false);
		_reverse_sort.toggled.connect(on_reverse_sort_toggled);

#if CROWN_GTK3
		_sort_items_box.pack_start(_reverse_sort, false, false);
		_sort_items_box.show_all();
#else
		_sort_items_box.append(_reverse_sort);
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
			, typeof(int)      // Column.OBJECT_KIND
			, typeof(string)   // Column.OBJECT_ICON
			, typeof(int)      // Column.SET_INDEX
			, typeof(bool)     // Column.VISIBLE
			, typeof(bool)     // Column.OBJECT_VISIBLE
			, typeof(bool)     // Column.SELECTION_LOCKED
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

		if ((ItemType)val == ItemType.OBJECTS_SET) {
			cell.set_property("icon-name", IconTheme.BROWSER_FOLDER);
			return;
		}

		model.get_value(iter, Column.OBJECT_ICON, out val);
		cell.set_property("icon-name", (string)val);
	}

	public void render_object_visibility(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value val;
		model.get_value(iter, Column.ITEM_TYPE, out val);
		if ((ItemType)val != ItemType.OBJECT) {
			cell.set_property("icon-name", "");
			cell.set_property("sensitive", false);
			return;
		}

		model.get_value(iter, Column.OBJECT_VISIBLE, out val);
		cell.set_property("icon-name", IconTheme.LAYER_VISIBLE);
		cell.set_property("sensitive", (bool)val);
	}

	public void render_selection_lock(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value val;
		model.get_value(iter, Column.ITEM_TYPE, out val);
		if ((ItemType)val != ItemType.OBJECT) {
			cell.set_property("icon-name", "");
			cell.set_property("sensitive", false);
			return;
		}

		model.get_value(iter, Column.SELECTION_LOCKED, out val);
		cell.set_property("icon-name", IconTheme.LAYER_LOCKED);
		cell.set_property("sensitive", (bool)val);
	}

	public bool refresh_object_visibility(Gtk.TreeModel model, Gtk.TreePath path, Gtk.TreeIter iter)
	{
		Value val;
		model.get_value(iter, Column.ITEM_TYPE, out val);
		if ((ItemType)val != ItemType.OBJECT)
			return false;

		model.get_value(iter, Column.OBJECT_ID, out val);
		Guid id = (Guid)val;
		bool visible = _object_visible_func == null ? true : _object_visible_func(id);
		_tree_store.set(iter, Column.OBJECT_VISIBLE, visible, -1);
		return false;
	}

	public void set_object_visibility_func(ObjectVisibleDelegate? object_visible_func, SetObjectVisibleDelegate? set_object_visible_func)
	{
		_object_visible_func = object_visible_func;
		_set_object_visible_func = set_object_visible_func;
		_visibility_column.visible = object_visible_func != null;
		_tree_store.foreach(refresh_object_visibility);
	}

	public bool refresh_selection_lock(Gtk.TreeModel model, Gtk.TreePath path, Gtk.TreeIter iter)
	{
		Value val;
		model.get_value(iter, Column.ITEM_TYPE, out val);
		if ((ItemType)val != ItemType.OBJECT)
			return false;

		model.get_value(iter, Column.OBJECT_ID, out val);
		Guid id = (Guid)val;
		bool locked = _selection_locked_func == null ? false : _selection_locked_func(id);
		_tree_store.set(iter, Column.SELECTION_LOCKED, locked, -1);
		return false;
	}

	public void set_selection_lock_func(SelectionLockedDelegate? selection_locked_func, SetSelectionLockedDelegate? set_selection_locked_func)
	{
		_selection_locked_func = selection_locked_func;
		_set_selection_locked_func = set_selection_locked_func;
		_lock_column.visible = selection_locked_func != null;
		_tree_store.foreach(refresh_selection_lock);
	}

	public void object_aspect(Guid id, out int kind, out string icon_name)
	{
		kind = 0;
		icon_name = IconTheme.LEVEL_OBJECT_UNIT;
		if (_object_aspect_func != null)
			_object_aspect_func(id, out kind, out icon_name);
	}

	public bool refresh_object_aspect(Gtk.TreeModel model, Gtk.TreePath path, Gtk.TreeIter iter)
	{
		Value val;
		model.get_value(iter, Column.ITEM_TYPE, out val);
		if ((ItemType)val != ItemType.OBJECT)
			return false;

		model.get_value(iter, Column.OBJECT_ID, out val);
		Guid id = (Guid)val;
		int kind;
		string icon_name;
		object_aspect(id, out kind, out icon_name);
		_tree_store.set(iter
			, Column.OBJECT_KIND
			, kind
			, Column.OBJECT_ICON
			, icon_name
			, -1
			);
		return false;
	}

	public void set_object_aspect_func(ObjectAspectDelegate? object_aspect_func)
	{
		_object_aspect_func = object_aspect_func;
		_tree_store.foreach(refresh_object_aspect);
	}

	public void set_context_menu_func(ContextMenuDelegate? context_menu_func)
	{
		_context_menu_func = context_menu_func;
	}

	public bool objects_set_flattened(StringId64 object_type, int set_index)
	{
		for (int i = 0; i < _flattened_sets.length; ++i) {
			FlattenedSetKey key = _flattened_sets[i];
			if (key.object_type == object_type && key.set_index == set_index)
				return true;
		}

		return false;
	}

	/// Displays @a property_name children directly under objects of @a object_type.
	public void flatten_objects_set(StringId64 object_type, string property_name)
	{
		unowned PropertyDefinition[] object_definition = _database.object_definition(object_type);
		for (int set_index = 0; set_index < object_definition.length; ++set_index) {
			var def = object_definition[set_index];
			if (def.name != property_name)
				continue;

			assert(def.type == PropertyType.OBJECTS_SET);

			for (int i = 0; i < _flattened_sets.length; ++i) {
				FlattenedSetKey key = _flattened_sets[i];
				if (key.object_type == object_type && key.set_index == set_index)
					return;
			}

			_flattened_sets += FlattenedSetKey(object_type, set_index);

			if (_object_id != GUID_ZERO) {
				Guid id = _object_id;
				_object_id = GUID_ZERO;
				set_object(id);
				if (_needle != "")
					filter();
				on_database_selection_changed();
			}
			return;
		}

		assert(false);
	}

	public bool on_query_tooltip(int x, int y, bool keyboard_tooltip, Gtk.Tooltip tooltip)
	{
		if (keyboard_tooltip)
			return false;

		int bx;
		int by;
		Gtk.TreePath path;
		Gtk.TreeViewColumn column;
		_tree_view.convert_widget_to_bin_window_coords(x, y, out bx, out by);
		if (!_tree_view.get_path_at_pos(bx, by, out path, out column, null, null))
			return false;

		bool visibility = column == _visibility_column
			&& _object_visible_func != null
			&& _set_object_visible_func != null
			;
		bool lock = column == _lock_column
			&& _selection_locked_func != null
			&& _set_selection_locked_func != null
			;
		if (!visibility && !lock)
			return false;

		Gtk.TreeIter iter;
		if (!_tree_view.model.get_iter(out iter, path))
			return false;

		Value val;
		_tree_view.model.get_value(iter, Column.ITEM_TYPE, out val);
		if ((ItemType)val != ItemType.OBJECT)
			return false;

		if (visibility)
			tooltip.set_text(_("Toggle visibility."));
		else
			tooltip.set_text(_("Toggle selection lock."));
		return true;
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
			int bx;
			int by;
			Gtk.TreePath path;
			Gtk.TreeViewColumn hit_column;
			_tree_view.convert_widget_to_bin_window_coords((int)x, (int)y, out bx, out by);
			if (_tree_view.get_path_at_pos(bx, by, out path, out hit_column, null, null)) {
				bool toggle_visibility = hit_column == _visibility_column
					&& _object_visible_func != null
					&& _set_object_visible_func != null
					;
				bool toggle_lock = hit_column == _lock_column
					&& _selection_locked_func != null
					&& _set_selection_locked_func != null
					;
				if (toggle_visibility || toggle_lock) {
					Gtk.TreeIter iter;
					if (_tree_view.model.get_iter(out iter, path)) {
						Value val;
						_tree_view.model.get_value(iter, Column.ITEM_TYPE, out val);
						if ((ItemType)val == ItemType.OBJECT) {
							_toggle_drag_column = hit_column;
							if (toggle_visibility)
								_tree_view.model.get_value(iter, Column.OBJECT_VISIBLE, out val);
							else
								_tree_view.model.get_value(iter, Column.SELECTION_LOCKED, out val);
							_toggle_drag_state = !(bool)val;
							_toggle_drag_changed.length = 0;
							_toggle_drag_last_x = x;
							_toggle_drag_last_y = y;
#if CROWN_GTK3
							Gtk.drag_source_unset(_tree_view);
#else
							_drag_source.actions = 0;
#endif
							toggle_drag_at(x, y);
							_gesture_click.set_state(Gtk.EventSequenceState.CLAIMED);
							return;
						}
					}
				}
			}

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
			GLib.GenericArray<Guid?> selected_object_ids = new GLib.GenericArray<Guid?>();
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
					Guid selected_id = (Guid)val;
					selected_object_ids.add(selected_id);
					if (selected_id == _object_id)
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

			if (_context_menu_func != null)
				_context_menu_func(menu_model, selected_object_ids.data);

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

		if (button == Gdk.BUTTON_PRIMARY && n_press == 2) {
			int bx;
			int by;
			Gtk.TreePath path;
			_tree_view.convert_widget_to_bin_window_coords((int)x, (int)y, out bx, out by);
			if (_tree_view.get_path_at_pos(bx, by, out path, null, null, null)) {
				Gtk.TreeIter iter;
				if (_tree_view.model.get_iter(out iter, path)) {
					Value val;
					_tree_view.model.get_value(iter, Column.ITEM_TYPE, out val);
					if ((ItemType)val == ItemType.OBJECT) {
						_tree_view.model.get_value(iter, Column.OBJECT_ID, out val);
						object_activated((Guid)val);
					}
				}
			}
		}
	}

	public void on_button_released(int n_press, double x, double y)
	{
		if (_toggle_drag_column != null) {
			if (_toggle_drag_changed.length > 0)
				objects_edited(_toggle_drag_changed.data);
			_toggle_drag_column = null;
			_toggle_drag_changed.length = 0;
#if CROWN_GTK3
			Gtk.drag_source_set(_tree_view, Gdk.ModifierType.BUTTON1_MASK, DND_TARGETS, Gdk.DragAction.COPY);
#else
			_drag_source.actions = Gdk.DragAction.COPY;
#endif
			return;
		}

		if (!_drag_started) {
			_saved_paths = null;
			if (_selection_changed_blocked)
				unblock_tree_selection_changed();
			_tree_selection.changed();
		}
	}

	public void on_button_update(Gdk.EventSequence? sequence)
	{
		if (_toggle_drag_column == null)
			return;

		double x;
		double y;
		if (!_gesture_click.get_point(sequence, out x, out y))
			return;

		double dy = y - _toggle_drag_last_y;
		int steps = int.max(1, (int)((dy < 0.0 ? -dy : dy) / 4.0));
		for (int i = 1; i <= steps; ++i) {
			double t = (double)i / (double)steps;
			toggle_drag_at(_toggle_drag_last_x + (x - _toggle_drag_last_x) * t
				, _toggle_drag_last_y + dy * t
				);
		}

		_toggle_drag_last_x = x;
		_toggle_drag_last_y = y;
	}

	public void toggle_drag_at(double x, double y)
	{
		Gtk.Allocation alloc;
		_tree_view.get_allocation(out alloc);
		if (x < 0.0 || x >= alloc.width || y < 0.0 || y >= alloc.height)
			return;

		int bx;
		int by;
		Gtk.TreePath path;
		_tree_view.convert_widget_to_bin_window_coords((int)x, (int)y, out bx, out by);
		if (!_tree_view.get_path_at_pos(bx, by, out path, null, null, null)
			&& !_tree_view.get_path_at_pos(1, by, out path, null, null, null))
			return;

		Gtk.TreeIter iter;
		if (!_tree_view.model.get_iter(out iter, path))
			return;

		Value val;
		_tree_view.model.get_value(iter, Column.ITEM_TYPE, out val);
		if ((ItemType)val != ItemType.OBJECT)
			return;

		_tree_view.model.get_value(iter, Column.OBJECT_ID, out val);
		Guid id = (Guid)val;

		Gtk.TreeIter filter_iter;
		Gtk.TreeIter store_iter;
		_tree_sort.convert_iter_to_child_iter(out filter_iter, iter);
		_tree_filter.convert_iter_to_child_iter(out store_iter, filter_iter);

		if (_toggle_drag_column == _visibility_column) {
			if (_set_object_visible_func == null)
				return;

			_tree_store.get_value(store_iter, Column.OBJECT_VISIBLE, out val);
			if ((bool)val == _toggle_drag_state)
				return;

			_tree_store.set(store_iter, Column.OBJECT_VISIBLE, _toggle_drag_state, -1);
			_set_object_visible_func(id, _toggle_drag_state);
		} else if (_toggle_drag_column == _lock_column) {
			if (_set_selection_locked_func == null)
				return;

			_tree_store.get_value(store_iter, Column.SELECTION_LOCKED, out val);
			if ((bool)val == _toggle_drag_state)
				return;

			_tree_store.set(store_iter, Column.SELECTION_LOCKED, _toggle_drag_state, -1);
			_set_selection_locked_func(id, _toggle_drag_state);
		} else {
			return;
		}

		_toggle_drag_changed.add(id);
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

	private void update_delete_action()
	{
		GLib.SimpleAction action = (GLib.SimpleAction)_database_editor._action_group.lookup_action("delete");
		action.set_enabled(!_database_editor._selection.find_with_equal_func(_object_id, Guid.equal_func));
	}

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

		selection_changed(ids.data);

		_setting_database_selection = true;
		GLib.SignalHandler.block(_database_editor, _database_selection_changed_id);
		_database_editor.selection_set(ids.data);
		GLib.SignalHandler.unblock(_database_editor, _database_selection_changed_id);
		_setting_database_selection = false;
		update_delete_action();
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
		if (mode == SortMode.NAME)
			button.set_active(true);
#endif
		button.toggled.connect(() => {
			if (!button.get_active())
				return;

			set_sort(mode);
		});

#if CROWN_GTK3
		_sort_items_box.pack_start(button, false, false);
#else
		_sort_items_box.append(button);
#endif
		return button;
	}

	public void set_sort(SortMode mode)
	{
		int column = mode == SortMode.NAME ? Column.OBJECT_NAME : Column.OBJECT_KIND;
		Gtk.SortType order = _reverse_sort.get_active() ? Gtk.SortType.DESCENDING : Gtk.SortType.ASCENDING;
		_tree_sort.set_sort_column_id(column, order);
	}

	public void on_reverse_sort_toggled()
	{
		int column;
		Gtk.SortType order;
		_tree_sort.get_sort_column_id(out column, out order);
		_tree_sort.set_sort_column_id(column, order == Gtk.SortType.ASCENDING ? Gtk.SortType.DESCENDING : Gtk.SortType.ASCENDING);
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
			_object_sets.remove(id);
		} else if (item_type == ItemType.OBJECTS_SET) {
			_tree_store.get_value(iter, Column.SET_INDEX, out val);
			_set_rows.remove(SetRowKey(id, (int)val));
		}
	}

	public void insert_object_child(Gtk.TreeIter? parent_iter, Guid child_id, SetRowKey set_key)
	{
		if (_object_rows.lookup(child_id) != null)
			return;

		int object_kind;
		string object_icon;
		object_aspect(child_id, out object_kind, out object_icon);

		Gtk.TreeIter child_iter;
		_tree_store.insert_with_values(out child_iter
			, parent_iter
			, 0 // GtkTreeStore append is quadratic for many siblings. Prepend instead.
			, Column.ITEM_TYPE
			, ItemType.OBJECT
			, Column.OBJECT_ID
			, child_id
			, Column.OBJECT_NAME
			, object_display_name(child_id)
			, Column.OBJECT_KIND
			, object_kind
			, Column.OBJECT_ICON
			, object_icon
			, Column.VISIBLE
			, true
			, Column.OBJECT_VISIBLE
			, _object_visible_func == null ? true : _object_visible_func(child_id)
			, Column.SELECTION_LOCKED
			, _selection_locked_func == null ? false : _selection_locked_func(child_id)
			, -1
			);
		_object_rows[child_id] = child_iter;
		_object_sets[child_id] = set_key;
		add_object_set(child_iter, child_id);
	}

	public void insert_object_in_set(SetRowKey set_key, Guid child_id)
	{
		StringId64 object_type = StringId64(_database.object_type(set_key.id));
		if (objects_set_flattened(object_type, set_key.set_index)) {
			if (set_key.id == _object_id && !show_root) {
				insert_object_child(null, child_id, set_key);
				return;
			}

			unowned Gtk.TreeIter? owner_iter_ptr = _object_rows.lookup(set_key.id);
			if (owner_iter_ptr != null)
				insert_object_child((Gtk.TreeIter)owner_iter_ptr, child_id, set_key);
			return;
		}

		unowned Gtk.TreeIter? set_iter_ptr = _set_rows.lookup(set_key);
		if (set_iter_ptr != null)
			insert_object_child((Gtk.TreeIter)set_iter_ptr, child_id, set_key);
	}

	// Synchronize one object's logical set and visual parent without scanning the tree.
	public void sync_object_placement(Guid id)
	{
		if (id == _object_id)
			return;

		SetRowKey desired = SetRowKey(GUID_ZERO, -1);
		bool has_desired = false;
		Guid owner_id = _database.owner(id);
		if (owner_id != GUID_ZERO
			&& (owner_id == _object_id || _object_rows.lookup(owner_id) != null)
			) {
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

		unowned SetRowKey? current_ptr = _object_sets.lookup(id);
		if (current_ptr != null && has_desired && SetRowKey.equal_func(current_ptr, desired))
			return;

		unowned Gtk.TreeIter? iter_ptr = _object_rows.lookup(id);
		if (iter_ptr != null) {
			Gtk.TreeIter iter = (Gtk.TreeIter)iter_ptr;
			unregister_subtree(iter);
			_tree_store.remove(ref iter);
		}

		if (has_desired)
			insert_object_in_set(desired, id);
	}

	public void reconcile_objects_set(Guid owner_id, StringId64 object_type, int set_index, PropertyDefinition def)
	{
		SetRowKey set_key = SetRowKey(owner_id, set_index);
		Guid?[] children = {};
		var desired = new GLib.HashTable<Guid?, bool?>(Guid.hash_func, Guid.equal_func);
		if (_database.has_property(owner_id, def.name)) {
			children = _database.get_set(owner_id, def.name);
			foreach (unowned Guid? child_id in children)
				desired.insert(child_id, true);
		}

		Gtk.TreeIter child_iter;
		bool child_valid;
		if (objects_set_flattened(object_type, set_index)) {
			if (owner_id == _object_id && !show_root) {
				child_valid = _tree_store.iter_children(out child_iter, null);
			} else {
				unowned Gtk.TreeIter? owner_iter_ptr = _object_rows.lookup(owner_id);
				if (owner_iter_ptr == null)
					return;
				child_valid = _tree_store.iter_children(out child_iter, (Gtk.TreeIter)owner_iter_ptr);
			}
		} else {
			unowned Gtk.TreeIter? set_iter_ptr = _set_rows.lookup(set_key);
			assert(set_iter_ptr != null);
			child_valid = _tree_store.iter_children(out child_iter, (Gtk.TreeIter)set_iter_ptr);
		}

		while (child_valid) {
			Value val;
			_tree_store.get_value(child_iter, Column.ITEM_TYPE, out val);
			if ((ItemType)val == ItemType.OBJECT) {
				_tree_store.get_value(child_iter, Column.OBJECT_ID, out val);
				Guid child_id = (Guid)val;
				unowned SetRowKey? current_ptr = _object_sets.lookup(child_id);
				if (current_ptr != null
					&& SetRowKey.equal_func(current_ptr, set_key)
					&& desired.lookup(child_id) == null
					) {
					unregister_subtree(child_iter);
					child_valid = _tree_store.remove(ref child_iter);
					continue;
				}
			}
			child_valid = _tree_store.iter_next(ref child_iter);
		}

		foreach (unowned Guid? child_id in children) {
			unowned SetRowKey? current_ptr = _object_sets.lookup(child_id);
			if (current_ptr != null && !SetRowKey.equal_func(current_ptr, set_key))
				sync_object_placement(child_id);
			else
				insert_object_in_set(set_key, child_id);
		}
	}

	public void on_objects_changed(Guid?[] object_ids, uint32 flags = 0)
	{
		if (_object_id == GUID_ZERO)
			return;

		bool selection_blocked = block_tree_selection_changed();

		foreach (unowned Guid? id in object_ids) {
			sync_object_placement(id);

			unowned Gtk.TreeIter? object_iter_ptr = _object_rows.lookup(id);
			if (object_iter_ptr != null) {
				Gtk.TreeIter object_iter = (Gtk.TreeIter)object_iter_ptr;
				if (_object_visible_func != null) {
					Value val;
					_tree_store.get_value(object_iter, Column.OBJECT_VISIBLE, out val);
					bool visible = _object_visible_func(id);
					if ((bool)val != visible)
						_tree_store.set(object_iter, Column.OBJECT_VISIBLE, visible, -1);
				}
				if (_selection_locked_func != null) {
					Value val;
					_tree_store.get_value(object_iter, Column.SELECTION_LOCKED, out val);
					bool locked = _selection_locked_func(id);
					if ((bool)val != locked)
						_tree_store.set(object_iter, Column.SELECTION_LOCKED, locked, -1);
				}
				if (_object_aspect_func != null) {
					int kind;
					string icon_name;
					object_aspect(id, out kind, out icon_name);

					Value val;
					_tree_store.get_value(object_iter, Column.OBJECT_KIND, out val);
					int old_kind = (int)val;
					_tree_store.get_value(object_iter, Column.OBJECT_ICON, out val);
					string old_icon_name = (string)val;
					if (old_kind != kind || old_icon_name != icon_name) {
						_tree_store.set(object_iter
							, Column.OBJECT_KIND
							, kind
							, Column.OBJECT_ICON
							, icon_name
							, -1
							);
					}
				}

				if (id != _object_id) {
					Value val;
					_tree_store.get_value(object_iter, Column.OBJECT_NAME, out val);
					string name = object_display_name(id);
					if ((string)val != name)
						_tree_store.set(object_iter, Column.OBJECT_NAME, name, -1);
				}
			} else if (id != _object_id) {
				continue;
			}

			StringId64 object_type = StringId64(_database.object_type(id));
			unowned PropertyDefinition[] object_definition = _database.object_definition(object_type);
			for (int set_index = 0; set_index < object_definition.length; ++set_index) {
				var def = object_definition[set_index];
				if (def.type == PropertyType.OBJECTS_SET)
					reconcile_objects_set(id, object_type, set_index, def);
			}
		}

		if (_needle != "")
			filter();

		select_rows(_database_editor._selection.data, false);
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

		select_rows(_database_editor._selection.data, false);
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

	public void add_object_set(Gtk.TreeIter? parent_iter, Guid id)
	{
		StringId64 object_type = StringId64(_database.object_type(id));
		unowned PropertyDefinition[] object_definition = _database.object_definition(object_type);

		for (int set_index = 0; set_index < object_definition.length; ++set_index) {
			var def = object_definition[set_index];
			if (def.type != PropertyType.OBJECTS_SET)
				continue;

			SetRowKey set_key = SetRowKey(id, set_index);
			Gtk.TreeIter? children_parent = parent_iter;
			if (!objects_set_flattened(object_type, set_index)) {
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
					, Column.OBJECT_KIND
					, 0
					, Column.OBJECT_ICON
					, ""
					, Column.SET_INDEX
					, set_index
					, Column.VISIBLE
					, true
					, Column.OBJECT_VISIBLE
					, true
					, Column.SELECTION_LOCKED
					, false
					, -1
					);
				_set_rows[set_key] = set_iter;
				children_parent = set_iter;
			}

			if (!_database.has_property(id, def.name))
				continue;

			Guid?[] children = _database.get_set(id, def.name);
			foreach (unowned Guid? child_id in children)
				insert_object_child(children_parent, child_id, set_key);
		}
	}

	/// Setting @a id to GUID_ZERO resets the tree.
	public void set_object(Guid id)
	{
		if (id == _object_id)
			return;

		int sort_column_id = Column.OBJECT_NAME;
		Gtk.SortType sort_order = Gtk.SortType.ASCENDING;
		_tree_sort.get_sort_column_id(out sort_column_id, out sort_order);

		// Populate a bare TreeStore.
		_tree_view.model = null;
		_object_rows.remove_all();
		_object_sets.remove_all();
		_set_rows.remove_all();
		_tree_store = new_tree_store();
		_object_id = id;
		update_delete_action();

		if (id != GUID_ZERO) {
			if (show_root) {
				ObjectTypeInfo info = _database.type_info(StringId64(_database.object_type(id)));
				int object_kind;
				string object_icon;
				object_aspect(id, out object_kind, out object_icon);

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
					, Column.OBJECT_KIND
					, object_kind
					, Column.OBJECT_ICON
					, object_icon
					, Column.VISIBLE
					, true
					, Column.OBJECT_VISIBLE
					, _object_visible_func == null ? true : _object_visible_func(id)
					, Column.SELECTION_LOCKED
					, _selection_locked_func == null ? false : _selection_locked_func(id)
					, -1
					);
				_object_rows[id] = object_iter;
				add_object_set(object_iter, id);
			} else {
				add_object_set(null, id);
			}
		}

		create_models(sort_column_id, sort_order);
		_tree_view.model = _tree_sort;

		if (expand_all_on_load) {
			_tree_view.expand_all();
		} else if (id != GUID_ZERO && show_root) {
			unowned Gtk.TreeIter? object_iter_ptr = _object_rows.lookup(id);
			assert(object_iter_ptr != null);
			Gtk.TreePath child_path = _tree_store.get_path((Gtk.TreeIter)object_iter_ptr);
			Gtk.TreePath filter_path = _tree_filter.convert_child_path_to_path(child_path);
			Gtk.TreePath sort_path = _tree_sort.convert_child_path_to_path(filter_path);
			_tree_view.expand_to_path(sort_path);
		}
	}

	public void select_rows(Guid?[] selection, bool scroll_to_selection)
	{
		Gtk.TreePath? last_selected = null;

		foreach (Guid? id in selection) {
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

	public void set_selection(Guid?[] selection, bool scroll_to_selection)
	{
		if (_needle != "")
			_search_selection_changed = true;

		bool selection_blocked = block_tree_selection_changed();
		_tree_selection.unselect_all();
		select_rows(selection, scroll_to_selection);
		if (selection_blocked)
			unblock_tree_selection_changed();
		update_delete_action();
	}

	public void read_selection(Guid?[] selection)
	{
		if (_setting_database_selection)
			return;

		set_selection(selection, false);
	}

	public void on_database_selection_changed()
	{
		set_selection(_database_editor._selection.data, true);
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
				select_rows(_database_editor._selection.data, true);
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
