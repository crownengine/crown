/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class ObjectProperties : Gtk.Box
{
	public delegate bool ComponentDelegate(out Guid component_id
		, StringId64 component_type
		, Database database
		, Guid object_id
		);

	public const string NOTHING_TO_SHOW = "nothing-to-show";
	public const string PROPERTIES = "properties";

	public DatabaseEditor _database_editor;
	public Database _database;
	public unowned ComponentDelegate? _component_func;
	public GLib.HashTable<StringId64?, PropertyGrid> _grids;
	public Gtk.Viewport _viewport;
	public Gtk.ScrolledWindow _scrolled_window;
	public PropertyGridSet _object_view;
	public Gtk.Stack _stack;

	public ObjectProperties(DatabaseEditor database_editor)
	{
		Object(orientation: Gtk.Orientation.VERTICAL);

		// Data
		_database_editor = database_editor;
		_database_editor.selection_changed.connect(on_database_selection_changed);
		_database = database_editor._database;
		_database._project.project_reset.connect(on_project_reset);
		_component_func = null;

		_grids = new GLib.HashTable<StringId64?, PropertyGrid>(StringId64.hash_func, StringId64.equal_func);

		// Widgets
		_object_view = new PropertyGridSet();
		_object_view.margin_bottom
			= _object_view.margin_end
			= _object_view.margin_start
			= _object_view.margin_top
			= 6
			;
		_object_view.vexpand = true;

		_viewport = new Gtk.Viewport(null, null);
#if CROWN_GTK3
		_viewport.add(_object_view);

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.add(_viewport);
#else
		_viewport.set_child(_object_view);

		_scrolled_window = new Gtk.ScrolledWindow();
		_scrolled_window.set_child(_viewport);
#endif

		_stack = new Gtk.Stack();
		_stack.hhomogeneous = false;
		Gtk.Label nothing_to_show = new Gtk.Label(_("Select an object to start editing"));
		nothing_to_show.ellipsize = Pango.EllipsizeMode.END;
		_stack.add_named(nothing_to_show, NOTHING_TO_SHOW);
		_stack.add_named(_scrolled_window, PROPERTIES);
		_stack.vexpand = true;

#if CROWN_GTK3
		this.pack_start(_stack);
		this.get_style_context().add_class("properties-view");
#else
		this.append(_stack);
		this.add_css_class("properties-view");
#endif
	}

	public void set_component_func(ComponentDelegate? component_func)
	{
		_component_func = component_func;
	}

	public void register_object_type(StringId64 object_type, PropertyGrid? custom_grid = null)
	{
		assert(!_grids.contains(object_type));

		PropertyGrid grid = custom_grid != null
			? custom_grid
			: new PropertyGrid.from_object_type(object_type, _database, _database_editor)
			;

		_object_view.add_property_grid(grid, camel_case(_database.type_name(object_type)));
		grid.hide_grid();
		_grids[object_type] = grid;
	}

	private void set_inherited(PropertyGrid grid, bool inherited)
	{
#if CROWN_GTK3
		if (inherited)
			grid._expander.get_style_context().add_class("inherited");
		else
			grid._expander.get_style_context().remove_class("inherited");
#else
		if (inherited)
			grid._expander.add_css_class("inherited");
		else
			grid._expander.remove_css_class("inherited");
#endif
	}

	private void clear_grids()
	{
		GLib.HashTableIter<StringId64?, PropertyGrid> iter = GLib.HashTableIter<StringId64?, PropertyGrid>(_grids);
		unowned StringId64? type;
		unowned PropertyGrid grid;
		while (iter.next(out type, out grid)) {
			grid.hide_grid();
			grid._id = GUID_ZERO;
			grid._component_id = GUID_ZERO;
			set_inherited(grid, false);
		}

		_object_view._list_box.invalidate_filter();
		_object_view._list_box.invalidate_sort();
	}

	public void set_object(Guid id)
	{
		if (id == GUID_ZERO || !_database.has_object(id) || !_database.is_alive(id)) {
			clear_grids();
			_stack.set_visible_child_name(NOTHING_TO_SHOW);
			return;
		}

		StringId64 object_type = StringId64(_database.object_type(id));
		PropertyGrid? direct_grid = _grids[object_type];
		if (direct_grid == null) {
			direct_grid = new PropertyGrid.from_object_type(object_type, _database, _database_editor);
			_object_view.add_property_grid(direct_grid, _("General"));
			_grids[object_type] = direct_grid;
		}

		_stack.set_visible_child_name(PROPERTIES);

		GLib.List<unowned StringId64?> types = _grids.get_keys();
		foreach (unowned StringId64? type in types) {
			PropertyGrid grid = _grids[type];
			Guid component_id = GUID_ZERO;
			bool visible = grid == direct_grid;

			if (!visible && _component_func != null)
				visible = _component_func(out component_id, (StringId64)type, _database, id);

			if (visible) {
				grid._id = id;
				grid._component_id = component_id;
				grid.show_grid();
				grid.read_properties();
				set_inherited(grid
					, component_id != GUID_ZERO && id != _database.owner(component_id)
					);
			} else {
				grid.hide_grid();
				grid._id = GUID_ZERO;
				grid._component_id = GUID_ZERO;
				set_inherited(grid, false);
			}
		}

		_object_view._list_box.invalidate_filter();
		_object_view._list_box.invalidate_sort();
#if CROWN_GTK3
		_object_view.show_all();
#endif
	}

	public void read_selection(Guid?[] selection)
	{
		if (selection.length == 0)
			set_object(GUID_ZERO);
		else
			set_object(selection[selection.length - 1]);
	}

	public void on_project_reset()
	{
		set_object(GUID_ZERO);
	}

	public void on_database_selection_changed()
	{
		read_selection(_database_editor._selection.data);
	}
}

} /* namespace Crown */
