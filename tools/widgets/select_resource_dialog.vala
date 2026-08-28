/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class SelectResourceDialog : Gtk.Window
{
	public enum View
	{
		ICON,
		LIST
	}

	// Data
	public string _resource_type;
	public string _needle;
	public Gtk.ListStore _view_store;
	public ThumbnailCache _thumbnail_cache;

	// Widgets
	public Gtk.EventControllerKey _controller_key;
	public EntrySearch _filter_entry;
	public Gtk.EventControllerKey _filter_entry_controller_key;
	public Gtk.TreeModelFilter _tree_filter;
	public Gtk.TreeModelSort _tree_sort;
	public Gtk.TreeView _tree_view;
	public Gtk.GestureMultiPress _tree_view_gesture_click;
	public Gtk.TreeSelection _tree_selection;
	public Gtk.ScrolledWindow _tree_view_window;
	public Gtk.IconView _icon_view;
	public Gtk.TreePath? _first_visible_path;
	public Gtk.TreePath? _last_visible_path;
	public Gtk.GestureMultiPress _icon_view_gesture_click;
	public Gtk.ScrolledWindow _icon_view_window;
	public Gtk.Stack _view_stack;
	public Gtk.Button _view_toggle;
	public Gtk.Image _view_toggle_image;

	// Signals
	public signal void resource_selected(string type, string name);

	public SelectResourceDialog(string resource_type
		, ProjectStore project_store
		, Gtk.Window? parent
		, ThumbnailCache thumbnail_cache
		, View default_view
		)
	{
		_resource_type = resource_type;
		_needle = "";
		_thumbnail_cache = thumbnail_cache;

		this.set_icon_name(CROWN_EDITOR_ICON_NAME);
		this.set_default_size(516, 650);

		if (parent != null) {
			this.set_transient_for(parent);
			this.set_modal(true);
		}
		this.delete_event.connect(on_close);

		_controller_key = new Gtk.EventControllerKey(this);
		_controller_key.set_propagation_phase(Gtk.PropagationPhase.CAPTURE);
		_controller_key.key_pressed.connect(on_key_pressed);

		Gtk.HeaderBar header_bar = new Gtk.HeaderBar();
		header_bar.show_close_button = true;
		this.set_titlebar(header_bar);

		_filter_entry = new EntrySearch();
		_filter_entry.set_placeholder_text(_("Search %s...").printf(resource_type));
		_filter_entry.hexpand = true;
		_filter_entry.search_changed.connect(refilter);
		header_bar.custom_title = _filter_entry;

		_filter_entry_controller_key = new Gtk.EventControllerKey(_filter_entry._entry);
		_filter_entry_controller_key.key_pressed.connect(on_filter_entry_key_pressed);

		_tree_filter = new Gtk.TreeModelFilter(project_store._list_store, null);
		_tree_filter.set_visible_func(filter_visible_func);

		_tree_sort = new Gtk.TreeModelSort.with_model(_tree_filter);
		_tree_sort.set_default_sort_func(sort_func);

		_tree_view = new Gtk.TreeView();
		_tree_view.model = _tree_sort;
		_tree_view.headers_visible = false;
		_tree_view.can_focus = false;
		_tree_view.row_activated.connect(on_tree_view_row_activated);

		Gtk.TreeViewColumn name_column = new Gtk.TreeViewColumn();
		var thumbnail_renderer = new Gtk.CellRendererPixbuf();
		thumbnail_renderer.stock_size = Gtk.IconSize.DND;
		thumbnail_renderer.set_fixed_size(32, 32);
		name_column.pack_start(thumbnail_renderer, false);
		name_column.set_cell_data_func(thumbnail_renderer, list_view_thumbnail_func);

		var name_renderer = new Gtk.CellRendererText();
		name_column.pack_start(name_renderer, true);
		name_column.add_attribute(name_renderer, "text", ProjectStore.Column.NAME);
		_tree_view.append_column(name_column);

		_tree_view_gesture_click = new Gtk.GestureMultiPress(_tree_view);
		_tree_view_gesture_click.set_button(0);
		_tree_view_gesture_click.released.connect(on_tree_view_button_released);

		_tree_selection = _tree_view.get_selection();
		_tree_selection.set_mode(Gtk.SelectionMode.BROWSE);

		_tree_view_window = new Gtk.ScrolledWindow(null, null);
		_tree_view_window.add(_tree_view);
		_tree_view_window.set_size_request(300, 400);

		_view_store = new Gtk.ListStore(5
			, typeof(string)               // ProjectStore.Column.TYPE
			, typeof(string)               // ProjectStore.Column.NAME
			, typeof(uint64)               // ProjectStore.Column.SIZE
			, typeof(uint64)               // ProjectStore.Column.MTIME
			, typeof(ProjectStore.RowKind) // ProjectStore.Column.KIND
			);
		_tree_view.model = _view_store;

		_icon_view = new Gtk.IconView();
		_icon_view.model = _view_store;
		_icon_view.set_item_width(80);
		_icon_view.set_item_padding(2);
		_icon_view.set_column_spacing(0);
		_icon_view.set_row_spacing(0);
		_icon_view.set_selection_mode(Gtk.SelectionMode.BROWSE);
		_icon_view.set_tooltip_column(ProjectStore.Column.NAME);
		_icon_view.draw.connect(on_icon_view_draw);
		_icon_view.item_activated.connect(confirm_path);

		var icon_renderer = new Gtk.CellRendererPixbuf();
		icon_renderer.stock_size = Gtk.IconSize.DIALOG;
		icon_renderer.set_fixed_size(64, 64);
		_icon_view.pack_start(icon_renderer, false);
		_icon_view.set_cell_data_func(icon_renderer, icon_view_thumbnail_func);

		var basename_renderer = new Gtk.CellRendererText();
		basename_renderer.set("wrap-mode", Pango.WrapMode.WORD_CHAR);
		basename_renderer.set("alignment", Pango.Alignment.CENTER);
		basename_renderer.set("xalign", 0.5);
		basename_renderer.set("yalign", 0.0);
		int wrap_width = _icon_view.item_width;
		wrap_width -= 2 * _icon_view.item_padding * 2;
		basename_renderer.set("wrap-width", wrap_width);
		basename_renderer.set("width", wrap_width);
		_icon_view.pack_end(basename_renderer, false);
		_icon_view.set_cell_data_func(basename_renderer, icon_view_text_func);

		_icon_view_gesture_click = new Gtk.GestureMultiPress(_icon_view);
		_icon_view_gesture_click.set_button(0);
		_icon_view_gesture_click.released.connect(on_icon_view_button_released);

		_icon_view_window = new Gtk.ScrolledWindow(null, null);
		_icon_view_window.add(_icon_view);
		_icon_view_window.set_size_request(300, 400);

		_view_stack = new Gtk.Stack();
		_view_stack.add_named(_icon_view_window, "icon-view");
		_view_stack.add_named(_tree_view_window, "list-view");
		_view_stack.set_visible_child_full("icon-view", Gtk.StackTransitionType.NONE);
		this.add(_view_stack);

		_view_toggle_image = new Gtk.Image.from_icon_name(IconTheme.BROWSER_LIST_VIEW, Gtk.IconSize.SMALL_TOOLBAR);
		_view_toggle = new Gtk.Button();
		_view_toggle.set_tooltip_text(_("List view."));
		_view_toggle.add(_view_toggle_image);
		_view_toggle.get_style_context().add_class("flat");
		_view_toggle.get_style_context().add_class("image-button");
		_view_toggle.can_focus = false;
		_view_toggle.clicked.connect(on_view_toggle_clicked);
		header_bar.pack_end(_view_toggle);

		_thumbnail_cache.changed.connect(on_thumbnail_cache_changed);
		project_store._project.file_added.connect(on_project_files_changed);
		project_store._project.file_changed.connect(on_project_files_changed);
		project_store._project.file_removed.connect(on_project_files_changed);
		project_store.reset_started.connect(on_project_reset_started);
		project_store.reset_finished.connect(rebuild_view_store);

		rebuild_view_store();
		set_view(default_view);
	}

	public bool on_close()
	{
		this.hide();
		return Gdk.EVENT_STOP;
	}

	public bool on_key_pressed(uint keyval, uint keycode, Gdk.ModifierType state)
	{
		if (keyval == Gdk.Key.Escape) {
			this.close();
			return Gdk.EVENT_STOP;
		}

		if (keyval == Gdk.Key.Tab
			|| keyval == Gdk.Key.KP_Tab
			|| keyval == Gdk.Key.ISO_Left_Tab) {
			_filter_entry._entry.grab_focus_without_selecting();
			return Gdk.EVENT_STOP;
		}

		if (!_filter_entry._entry.has_focus) {
			Gdk.Event? event = Gtk.get_current_event();
			if (event != null && _filter_entry._entry.handle_event(event)) {
				_filter_entry._entry.grab_focus_without_selecting();
				return Gdk.EVENT_STOP;
			}
		}

		return Gdk.EVENT_PROPAGATE;
	}

	public bool filter_visible_func(Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value type;
		Value name;
		model.get_value(iter, ProjectStore.Column.TYPE, out type);
		model.get_value(iter, ProjectStore.Column.NAME, out name);

		string type_str = (string)type;
		string name_str = (string)name;

		return type_str != null
			&& name_str != null
			&& type_str == _resource_type
			&& (_needle.length == 0 || name_str.down().index_of(_needle) > -1)
			;
	}

	public int sort_func(Gtk.TreeModel model, Gtk.TreeIter iter_a, Gtk.TreeIter iter_b)
	{
		Value name_a;
		Value name_b;
		model.get_value(iter_a, ProjectStore.Column.NAME, out name_a);
		model.get_value(iter_b, ProjectStore.Column.NAME, out name_b);
		return strcmp((string)name_a, (string)name_b);
	}

	public void on_tree_view_row_activated(Gtk.TreePath path, Gtk.TreeViewColumn column)
	{
		confirm_path(path);
	}

	public void list_view_thumbnail_func(Gtk.CellLayout layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		set_thumbnail_for_iter(cell, model, iter, 32);
	}

	public void icon_view_thumbnail_func(Gtk.CellLayout layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Gtk.TreePath path = model.get_path(iter);
		if (_first_visible_path != null
			&& path.compare(_first_visible_path) >= 0
			&& path.compare(_last_visible_path) <= 0) {
			set_thumbnail_for_iter(cell, model, iter, 64);
		} else {
			set_fallback_icon_for_iter(cell, model, iter);
		}
	}

	public void icon_view_text_func(Gtk.CellLayout layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value name;
		model.get_value(iter, ProjectStore.Column.NAME, out name);
		cell.set_property("text", GLib.Path.get_basename((string)name));
	}

	public void on_tree_view_button_released(int n_press, double x, double y)
	{
		if (_tree_view_gesture_click.get_current_button() != Gdk.BUTTON_PRIMARY)
			return;

		int bx;
		int by;
		Gtk.TreePath path;
		_tree_view.convert_widget_to_bin_window_coords((int)x, (int)y, out bx, out by);
		if (_tree_view.get_path_at_pos(bx, by, out path, null, null, null)
			&& _tree_selection.path_is_selected(path))
			confirm_path(path);
	}

	public void on_icon_view_button_released(int n_press, double x, double y)
	{
		if (_icon_view_gesture_click.get_current_button() != Gdk.BUTTON_PRIMARY)
			return;

		int bx;
		int by;
		_icon_view.convert_widget_to_bin_window_coords((int)x, (int)y, out bx, out by);
		Gtk.TreePath? path = _icon_view.get_path_at_pos(bx, by);
		if (path != null && _icon_view.path_is_selected(path))
			confirm_path(path);
	}

	public void on_view_toggle_clicked()
	{
		set_view(_view_stack.get_visible_child() == _icon_view_window
			? View.LIST
			: View.ICON
			);
	}

	public void on_thumbnail_cache_changed()
	{
		_icon_view.queue_draw();
		_tree_view.queue_draw();
	}

	public bool on_icon_view_draw(Cairo.Context cr)
	{
		Gtk.TreePath first_path;
		Gtk.TreePath last_path;
		if (_icon_view.get_visible_range(out first_path, out last_path)) {
			_first_visible_path = first_path;
			_last_visible_path = last_path;
		} else {
			_first_visible_path = null;
			_last_visible_path = null;
		}

		return Gdk.EVENT_PROPAGATE;
	}

	public void on_project_files_changed()
	{
		rebuild_view_store();
	}

	public void on_project_reset_started()
	{
		detach_view_store();
		_view_store.clear();
		attach_view_store();
	}

	public void resource_at_path(Gtk.TreePath path, out string type, out string name)
	{
		Gtk.TreeIter iter;
		Gtk.TreeModel model = _tree_view.model;
		bool found = model.get_iter(out iter, path);
		assert(found);

		Value type_value;
		Value name_value;
		model.get_value(iter, ProjectStore.Column.TYPE, out type_value);
		model.get_value(iter, ProjectStore.Column.NAME, out name_value);
		type = (string)type_value;
		name = (string)name_value;
	}

	public void set_thumbnail_for_iter(Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter, int size)
	{
		Value kind;
		Value type;
		Value name;
		model.get_value(iter, ProjectStore.Column.KIND, out kind);
		model.get_value(iter, ProjectStore.Column.TYPE, out type);
		model.get_value(iter, ProjectStore.Column.NAME, out name);

		set_thumbnail(cell
			, (ProjectStore.RowKind)kind
			, (string)type
			, (string)name
			, size
			, _thumbnail_cache
			);
	}

	public void set_fallback_icon_for_iter(Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value kind;
		Value type;
		model.get_value(iter, ProjectStore.Column.KIND, out kind);
		model.get_value(iter, ProjectStore.Column.TYPE, out type);

		set_fallback_icon(cell
			, (ProjectStore.RowKind)kind
			, (string)type
			, _thumbnail_cache
			);
	}

	public bool selected_path(out Gtk.TreePath path)
	{
		if (_view_stack.get_visible_child() == _icon_view_window) {
			GLib.List<Gtk.TreePath> paths = _icon_view.get_selected_items();
			if (paths.length() == 0) {
				path = null;
				return false;
			}

			path = paths.data;
			return true;
		}

		Gtk.TreeModel model;
		Gtk.TreeIter iter;
		if (!_tree_selection.get_selected(out model, out iter)) {
			path = null;
			return false;
		}

		path = model.get_path(iter);
		return true;
	}

	public void select_path(Gtk.TreePath path, bool scroll = true)
	{
		if (_view_stack.get_visible_child() == _icon_view_window) {
			_icon_view.set_cursor(path, null, false);
			_icon_view.select_path(path);
			if (scroll)
				_icon_view.scroll_to_path(path, true, 0.5f, 0.0f);
		} else {
			_tree_selection.select_path(path);
			if (scroll)
				_tree_view.scroll_to_cell(path, null, false, 0.0f, 0.0f);
		}
	}

	public void select_first_result()
	{
		_tree_selection.unselect_all();
		_icon_view.unselect_all();

		Gtk.TreeModel model = _tree_view.model;
		Gtk.TreeIter iter;
		if (model.get_iter_first(out iter)) {
			Gtk.TreePath path = model.get_path(iter);
			_tree_selection.select_path(path);
			_icon_view.set_cursor(path, null, false);
			_icon_view.select_path(path);
		}
	}

	public void confirm_path(Gtk.TreePath path)
	{
		string type;
		string name;
		resource_at_path(path, out type, out name);
		resource_selected(type, name);
	}

	public void refilter()
	{
		_needle = _filter_entry.text.strip().down();
		_tree_filter.refilter();
		rebuild_view_store();
	}

	public void attach_view_store()
	{
		_tree_view.model = _view_store;
		_icon_view.model = _view_store;
	}

	public void detach_view_store()
	{
		_tree_view.model = null;
		_icon_view.model = null;
	}

	public void rebuild_view_store()
	{
		detach_view_store();
		_view_store.clear();
		_tree_sort.foreach(copy_view_row);
		attach_view_store();
		select_first_result();
	}

	public bool copy_view_row(Gtk.TreeModel model, Gtk.TreePath path, Gtk.TreeIter iter)
	{
		Value type;
		Value name;
		Value size;
		Value mtime;
		Value kind;
		model.get_value(iter, ProjectStore.Column.TYPE, out type);
		model.get_value(iter, ProjectStore.Column.NAME, out name);
		model.get_value(iter, ProjectStore.Column.SIZE, out size);
		model.get_value(iter, ProjectStore.Column.MTIME, out mtime);
		model.get_value(iter, ProjectStore.Column.KIND, out kind);

		Gtk.TreeIter view_iter;
		_view_store.insert_with_values(out view_iter
			, -1
			, ProjectStore.Column.TYPE
			, (string)type
			, ProjectStore.Column.NAME
			, (string)name
			, ProjectStore.Column.SIZE
			, (uint64)size
			, ProjectStore.Column.MTIME
			, (uint64)mtime
			, ProjectStore.Column.KIND
			, (ProjectStore.RowKind)kind
			, -1
			);
		return false;
	}

	public bool on_filter_entry_key_pressed(uint keyval, uint keycode, Gdk.ModifierType state)
	{
		if (keyval == Gdk.Key.Return) {
			Gtk.TreePath selected;
			if (selected_path(out selected))
				confirm_path(selected);
			return Gdk.EVENT_STOP;
		}

		if (_view_stack.get_visible_child() == _icon_view_window) {
			bool horizontal = keyval == Gdk.Key.Left
				|| keyval == Gdk.Key.KP_Left
				|| keyval == Gdk.Key.Right
				|| keyval == Gdk.Key.KP_Right
				;
			bool vertical = keyval == Gdk.Key.Up
				|| keyval == Gdk.Key.KP_Up
				|| keyval == Gdk.Key.Down
				|| keyval == Gdk.Key.KP_Down
				;
			if (!horizontal && !vertical)
				return Gdk.EVENT_PROPAGATE;

			Gtk.MovementStep step = horizontal
				? Gtk.MovementStep.VISUAL_POSITIONS
				: Gtk.MovementStep.DISPLAY_LINES;
			int count = keyval == Gdk.Key.Left
				|| keyval == Gdk.Key.KP_Left
				|| keyval == Gdk.Key.Up
				|| keyval == Gdk.Key.KP_Up
				? -1
				: 1;

			_icon_view.grab_focus();
			_icon_view.move_cursor(step, count);
			_filter_entry._entry.grab_focus();
			return Gdk.EVENT_STOP;
		}

		if (keyval != Gdk.Key.Down && keyval != Gdk.Key.Up)
			return Gdk.EVENT_PROPAGATE;

		Gtk.TreePath path;
		if (!selected_path(out path))
			return Gdk.EVENT_STOP;

		Gtk.TreeModel model = _tree_view.model;
		Gtk.TreeIter iter;
		if (!model.get_iter(out iter, path))
			return Gdk.EVENT_STOP;

		bool moved = keyval == Gdk.Key.Down
			? model.iter_next(ref iter)
			: model.iter_previous(ref iter);
		if (moved)
			select_path(model.get_path(iter));

		return Gdk.EVENT_STOP;
	}

	public void set_view(View view)
	{
		Gtk.TreePath path;
		bool has_selection = selected_path(out path);

		if (view == View.LIST) {
			_tree_view_window.set_visible(true);
			_view_stack.set_visible_child_full("list-view", Gtk.StackTransitionType.NONE);
			_view_toggle_image.set_from_icon_name(IconTheme.BROWSER_ICON_VIEW, Gtk.IconSize.SMALL_TOOLBAR);
			_view_toggle.set_tooltip_text(_("Icon view."));
		} else if (view == View.ICON) {
			_icon_view_window.set_visible(true);
			_view_stack.set_visible_child_full("icon-view", Gtk.StackTransitionType.NONE);
			_view_toggle_image.set_from_icon_name(IconTheme.BROWSER_LIST_VIEW, Gtk.IconSize.SMALL_TOOLBAR);
			_view_toggle.set_tooltip_text(_("List view."));
		}

		if (has_selection)
			select_path(path);
	}
}

} /* namespace Crown */
