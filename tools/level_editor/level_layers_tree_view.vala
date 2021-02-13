/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gee;
using Gtk;

namespace Crown
{
public class LevelLayersTreeView : Gtk.Box
{
	private enum ItemFlags
	{
		VISIBLE = 1,
		LOCKED = 2
	}

	// Data
	private Level _level;
	private Database _db;

	// Widgets
	private EntrySearch _filter_entry;
	private Gtk.ListStore _list_store;
	private Gtk.TreeModelFilter _tree_filter;
	private Gtk.TreeView _tree_view;
	private Gtk.TreeSelection _tree_selection;
	private Gtk.ScrolledWindow _scrolled_window;

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
		_tree_view.button_press_event.connect(on_button_pressed);

		_tree_selection = _tree_view.get_selection();
		_tree_selection.set_mode(Gtk.SelectionMode.MULTIPLE);

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.add(_tree_view);

		this.pack_start(_filter_entry, false, true, 0);
		this.pack_start(_scrolled_window, true, true, 0);

		this.get_style_context().add_class("level-layers-view");
	}

	private bool on_button_pressed(Gdk.EventButton ev)
	{
		return Gdk.EVENT_PROPAGATE;
	}

	private bool filter_tree(Gtk.TreeModel model, Gtk.TreeIter iter)
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

	private void on_filter_entry_text_changed()
	{
		_tree_filter.refilter();
	}
}

}
