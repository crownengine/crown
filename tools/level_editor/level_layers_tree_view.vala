/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class LevelLayersTreeView : Gtk.Box
{
	public enum ItemFlags
	{
		VISIBLE = 1,
		LOCKED  = 2
	}

	// Data
	public Level _level;
	public Database _db;

	// Widgets
	public EntrySearch _filter_entry;
	public Gtk.ListStore _list_store;
	public Gtk.TreeModelFilter _tree_filter;
	public Gtk.TreeView _tree_view;
	public Gtk.GestureMultiPress _tree_view_gesture_click;
	public Gtk.TreeSelection _tree_selection;
	public Gtk.ScrolledWindow _scrolled_window;

	public LevelLayersTreeView(Database db, Level level)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

		// Data
		_level = level;
		_db = db;

		// Widgets
		_filter_entry = new EntrySearch();
		_filter_entry.set_placeholder_text("Search...");
		_filter_entry.search_changed.connect(on_filter_entry_text_changed);

		_list_store = new Gtk.ListStore(3, typeof(string), typeof(string), typeof(string));
		_list_store.insert_with_values(null, -1
			, 0, "layer-visible"
			, 1, "layer-locked"
			, 2, "Background"
			, -1
			);
		_list_store.insert_with_values(null, -1
			, 0, "layer-visible"
			, 1, "layer-locked"
			, 2, "Default"
			, -1
			);

		_tree_filter = new Gtk.TreeModelFilter(_list_store, null);
		_tree_filter.set_visible_func(filter_tree);

		_tree_view = new Gtk.TreeView();
		_tree_view.insert_column_with_attributes(-1, "Visible", new Gtk.CellRendererPixbuf(), "icon-name", 0, null);
		_tree_view.insert_column_with_attributes(-1, "Locked",  new Gtk.CellRendererPixbuf(), "icon-name", 1, null);
		_tree_view.insert_column_with_attributes(-1, "Name",    new Gtk.CellRendererText(),   "text",      2, null);

		_tree_view.headers_clickable = false;
		_tree_view.headers_visible = false;
		_tree_view.model = _tree_filter;

		_tree_view_gesture_click = new Gtk.GestureMultiPress(_tree_view);
		_tree_view_gesture_click.set_button(0);
		_tree_view_gesture_click.pressed.connect(on_button_pressed);

		_tree_selection = _tree_view.get_selection();
		_tree_selection.set_mode(Gtk.SelectionMode.MULTIPLE);

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.add(_tree_view);

		this.pack_start(_filter_entry, false, true, 0);
		this.pack_start(_scrolled_window, true, true, 0);
	}

	public void on_button_pressed(int n_press, double x, double y)
	{
		return;
	}

	public bool filter_tree(Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value val;
		model.get_value(iter, 2, out val);

		_tree_view.expand_all();

		string layer_name = ((string)val).down();
		string filter_text = _filter_entry.text.down();
		if (filter_text == "" || layer_name.index_of(filter_text) > -1)
			return true;

		return false;
	}

	public void on_filter_entry_text_changed()
	{
		_tree_filter.refilter();
	}
}

} /* namespace Crown */
