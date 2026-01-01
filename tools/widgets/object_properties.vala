/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class ObjectProperties : Gtk.Box
{
	public const string NOTHING_TO_SHOW = "nothing-to-show";
	public const string UNKNOWN_OBJECT_TYPE = "unknown-object-type";
	public const string PROPERTIES = "properties";

	public DatabaseEditor _database_editor;
	public Database _database;
	public Gee.ArrayList<bool> _expanders_states;
	public Gee.ArrayList<PropertyGrid> _grids;
	public Gtk.Viewport _viewport;
	public Gtk.ScrolledWindow _scrolled_window;
	public PropertyGridSet _object_view;
	public Gtk.Stack _stack;

	public ObjectProperties(DatabaseEditor database_editor)
	{
		// Data
		_database_editor = database_editor;
		_database_editor.selection_changed.connect(on_database_selection_changed);
		_database = database_editor._database;

		_expanders_states = new Gee.ArrayList<bool>();
		_grids = new Gee.ArrayList<PropertyGrid>();

		// Widgets
		_object_view = new PropertyGridSet();
		_object_view.margin_bottom
			= _object_view.margin_end
			= _object_view.margin_start
			= _object_view.margin_top
			= 6
			;

		_viewport = new Gtk.Viewport(null, null);
		_viewport.add(_object_view);

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.add(_viewport);

		_stack = new Gtk.Stack();
		_stack.add_named(new Gtk.Label("Select an object to start editing"), NOTHING_TO_SHOW);
		_stack.add_named(new Gtk.Label("Unknown object type"), UNKNOWN_OBJECT_TYPE);
		_stack.add_named(_scrolled_window, PROPERTIES);

		this.pack_start(_stack);
		this.get_style_context().add_class("properties-view");
	}

	public void set_object(Guid id)
	{
		if (id == GUID_ZERO) {
			_stack.set_visible_child_name(NOTHING_TO_SHOW);
			return;
		}

		if (!_database.has_object(id) || !_database.is_alive(id)) {
			loge("Object does not exist");
			return;
		}

		_stack.set_visible_child_name(PROPERTIES);

		int num_found = 0;
		for (int i = 0; i < _grids.size; ++i) {
			PropertyGrid grid = _grids[i];

			if (Guid.equal_func(grid._id, id)) {
				grid._visible = true;
				grid.read_properties();
				++num_found;
			} else {
				grid._visible = false;
			}
		}

		if (num_found == 0) {
			PropertyGrid grid = new PropertyGrid.from_object(id, _database);
			grid._visible = true;
			grid.read_properties();

			_object_view.add_property_grid(grid, "General");
			_grids.add(grid);
		}

		_object_view._list_box.invalidate_filter();
		_object_view._list_box.invalidate_sort();
		_object_view.show_all();
	}

	public void on_database_selection_changed()
	{
		Gee.ArrayList<Guid?> selection = _database_editor._selection;
		if (selection.size == 0)
			set_object(GUID_ZERO);
		else
			set_object(selection.last());
	}
}

} /* namespace Crown */
