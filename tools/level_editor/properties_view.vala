/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class UnitView : PropertyGrid
{
	// Widgets
	public InputResource _prefab;
	public Gtk.MenuButton _component_add;
	public Gtk.Box _components;
	public Gtk.Popover _add_popover;

	public static GLib.Menu component_menu(string object_type)
	{
		GLib.Menu menu = new GLib.Menu();
		GLib.MenuItem mi;

		mi = new GLib.MenuItem("Remove Component", null);
		mi.set_action_and_target_value("app.unit-remove-component", new GLib.Variant.string(object_type));
		menu.append_item(mi);

		return menu;
	}

	public UnitView(Database db)
	{
		base(db);

		_order = -1.0;

		// Widgets
		_prefab = new InputResource(OBJECT_TYPE_UNIT, db);
		_prefab._selector.sensitive = false;

		// Construct 'add components' button.
		GLib.Menu menu_model = new GLib.Menu();
		GLib.MenuItem mi;

		foreach (var entry in Unit._component_registry.entries) {
			mi = new GLib.MenuItem(camel_case(entry.key), null);
			mi.set_action_and_target_value("app.unit-add-component"
				, new GLib.Variant.string(entry.key)
				);
			menu_model.append_item(mi);
		}

		_add_popover = new Gtk.Popover.from_model(null, menu_model);

		_component_add = new Gtk.MenuButton();
		_component_add.label = "Add Component";
		_component_add.set_popover(_add_popover);

		_components = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
		_components.homogeneous = true;
		_components.pack_start(_component_add);

		add_row("Prefab", _prefab);
		add_row("Components", _components);
	}

	public override void update()
	{
		if (_db.has_property(_id, "prefab")) {
			_prefab.value = _db.get_property_string(_id, "prefab");
		} else {
			_prefab.value = "<none>";
		}
	}
}

public class PropertiesView : Gtk.Box
{
	public Database _db;
	public Gee.HashMap<string, Expander> _expanders;
	public Gee.HashMap<string, bool> _expander_states;
	public Gee.HashMap<string, PropertyGrid> _objects;
	public Gee.ArrayList<string> _entries;
	public Gee.ArrayList<Guid?>? _selection;

	// Widgets
	public Gtk.Label _nothing_to_show;
	public Gtk.Label _unknown_object_type;
	public Gtk.Viewport _viewport;
	public Gtk.ScrolledWindow _scrolled_window;
	public PropertyGridSet _object_view;
	public Gtk.Stack _stack;

	[CCode (has_target = false)]
	public delegate GLib.Menu ContextMenu(string object_type);

	public PropertiesView(Database db)
	{
		Object(orientation: Gtk.Orientation.VERTICAL);

		// Data
		_db = db;

		_expanders = new Gee.HashMap<string, Expander>();
		_expander_states = new Gee.HashMap<string, bool>();
		_objects = new Gee.HashMap<string, PropertyGrid>();
		_entries = new Gee.ArrayList<string>();
		_selection = null;

		// Widgets
		_object_view = new PropertyGridSet();
		_object_view.margin_bottom
			= _object_view.margin_end
			= _object_view.margin_start
			= _object_view.margin_top
			= 6
			;

		_nothing_to_show = new Gtk.Label("Select an object to start editing");
		_unknown_object_type = new Gtk.Label("Unknown object type");

		_viewport = new Gtk.Viewport(null, null);
		_viewport.add(_object_view);

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.add(_viewport);

		_stack = new Gtk.Stack();
		_stack.add(_nothing_to_show);
		_stack.add(_scrolled_window);
		_stack.add(_unknown_object_type);

		this.pack_start(_stack);
		this.get_style_context().add_class("properties-view");

		db._project.project_reset.connect(on_project_reset);
	}

	public void register_object_type(string object_type, PropertyGrid? cv = null, ContextMenu? context_menu = null)
	{
		PropertyGrid? grid = cv;
		if (grid == null)
			grid = new PropertyGrid.from_object_type(StringId64(object_type), _db);

		Expander expander = _object_view.add_property_grid(grid, camel_case(object_type));
		if (context_menu != null) {
			Gtk.GestureMultiPress _controller_click = new Gtk.GestureMultiPress(expander);
			_controller_click.set_button(0);
			_controller_click.released.connect((n_press, x, y) => {
					if (_controller_click.get_current_button() == Gdk.BUTTON_SECONDARY) {
						Gtk.Popover menu = new Gtk.Popover.from_model(null, context_menu(object_type));
						menu.set_relative_to(expander);
						menu.set_pointing_to({ (int)x, (int)y, 1, 1 });
						menu.set_position(Gtk.PositionType.BOTTOM);
						menu.popup();
					}
				});
		}

		_objects[object_type] = grid;
		_expanders[object_type] = expander;
		_entries.add(object_type);
	}

	public void show_unit(Guid id)
	{
		foreach (var type in _entries) {
			Expander expander = _expanders[type];
			_expander_states[type] = expander.expanded;
		}
		_stack.set_visible_child(_scrolled_window);

		foreach (var type in _entries) {
			Expander expander = _expanders[type];
			bool was_expanded = _expander_states.has_key(type) ? _expander_states[type] : false;
			PropertyGrid cv = _objects[type];

			Unit unit = Unit(_db, id);
			Guid component_id;
			Guid owner_id;
			if (unit.has_component_with_owner(out component_id, out owner_id, type) || type == OBJECT_TYPE_UNIT) {
				cv._id = id;
				cv._component_id = component_id;
				cv._visible = true;
				cv.update();

				if (id == owner_id)
					expander.get_style_context().remove_class("inherited");
				else
					expander.get_style_context().add_class("inherited");

				expander.expanded = was_expanded;
			} else {
				cv._visible = false;
			}
		}

		_object_view._list_box.invalidate_filter();
		_object_view._list_box.invalidate_sort();
	}

	public void show_sound_source(Guid id)
	{
		foreach (var type in _entries) {
			Expander expander = _expanders[type];
			_expander_states[type] = expander.expanded;
		}

		_stack.set_visible_child(_scrolled_window);

		foreach (var type in _entries) {
			Expander expander = _expanders[type];
			PropertyGrid cv = _objects[type];

			if (type == OBJECT_TYPE_SOUND_SOURCE) {
				bool was_expanded = _expander_states.has_key(type) ? _expander_states[type] : false;

				cv._id = id;
				cv._visible = true;
				cv.update();

				expander.show();
				expander.expanded = was_expanded;
			} else {
				cv._visible = false;
			}
		}

		_object_view._list_box.invalidate_filter();
		_object_view._list_box.invalidate_sort();
	}

	public void show_or_hide_properties()
	{
		if (_selection == null || _selection.size != 1) {
			_stack.set_visible_child(_nothing_to_show);
			return;
		}

		Guid id = _selection[_selection.size - 1];
		if (!_db.has_object(id))
			return;

		if (_db.object_type(id) == OBJECT_TYPE_UNIT)
			show_unit(id);
		else if (_db.object_type(id) == OBJECT_TYPE_SOUND_SOURCE)
			show_sound_source(id);
		else
			_stack.set_visible_child(_unknown_object_type);
	}

	public void on_selection_changed(Gee.ArrayList<Guid?> selection)
	{
		_selection = selection;
		show_or_hide_properties();
	}

	public override void map()
	{
		base.map();
		show_or_hide_properties();
	}

	public void on_project_reset()
	{
		foreach (var obj in _objects) {
			PropertyGrid cv = obj.value;
			cv._id = GUID_ZERO;
			cv._component_id = GUID_ZERO;
		}
	}
}

} /* namespace Crown */
