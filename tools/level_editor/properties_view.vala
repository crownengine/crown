/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class UnitView : PropertyGrid
{
	public InputResource _prefab;
	public Gtk.MenuButton _component_add;
	public Gtk.Box _components;
	public Gtk.Popover _add_popover;

	public const GLib.ActionEntry[] actions =
	{
		{ "add-component", on_add_component, "s", null },
	};

	public void on_add_component(GLib.SimpleAction action, GLib.Variant? param)
	{
		string component_type = param.get_string();

		Guid unit_id = _id;
		Unit unit = Unit(_db, unit_id);

		Gee.ArrayList<Guid?> components_added = new Gee.ArrayList<Guid?>();
		components_added.add(unit_id);
		unit.add_component_type_dependencies(ref components_added, component_type);

		_db.add_restore_point((int)ActionType.CREATE_OBJECTS, components_added.to_array());
	}

	public UnitView(Database db)
	{
		base(db);

		_action_group.add_action_entries(actions, this);

		_order = -1.0;

		// Widgets
		_prefab = new InputResource(OBJECT_TYPE_UNIT, db);
		_prefab._selector.sensitive = false;

		// Construct 'add components' button.
		GLib.Menu menu_model = new GLib.Menu();
		GLib.MenuItem mi;

		foreach (var entry in Unit._component_registry.entries) {
			mi = new GLib.MenuItem(camel_case(entry.key), null);
			mi.set_action_and_target_value("object.add-component"
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

	public void on_open_prefab_clicked()
	{
		GLib.Application.get_default().activate_action("unit-open", new GLib.Variant.string(_prefab.value));
	}
}

public class PropertiesView : Gtk.Box
{
	public const string NOTHING_TO_SHOW = "nothing-to-show";
	public const string UNKNOWN_OBJECT_TYPE = "unknown-object-type";
	public const string PROPERTIES = "properties";

	public Database _db;
	public Gee.HashMap<string, bool> _expander_states;
	public Gee.HashMap<string, PropertyGrid> _objects;
	public Gee.ArrayList<Guid?>? _selection;
	public Gtk.Viewport _viewport;
	public Gtk.ScrolledWindow _scrolled_window;
	public PropertyGridSet _object_view;
	public Gtk.Stack _stack;

	public PropertiesView(Database db)
	{
		Object(orientation: Gtk.Orientation.VERTICAL);

		// Data
		_db = db;

		_expander_states = new Gee.HashMap<string, bool>();
		_objects = new Gee.HashMap<string, PropertyGrid>();
		_selection = null;

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

		db._project.project_reset.connect(on_project_reset);
	}

	public void register_object_type(string object_type, PropertyGrid? cv = null)
	{
		PropertyGrid? grid = cv;
		if (grid == null)
			grid = new PropertyGrid.from_object_type(StringId64(object_type), _db);

		_object_view.add_property_grid(grid, camel_case(object_type));
		_objects[object_type] = grid;
	}

	public void show_unit(Guid id)
	{
		foreach (var entry in _objects)
			_expander_states[entry.key] = entry.value._expander.expanded;

		_stack.set_visible_child_name(PROPERTIES);

		foreach (var entry in _objects) {
			string type = entry.key;
			PropertyGrid cv = entry.value;

			bool was_expanded = _expander_states.has_key(type) ? _expander_states[type] : false;

			Unit unit = Unit(_db, id);
			Guid component_id;
			Guid owner_id;
			if (unit.has_component_with_owner(out component_id, out owner_id, type) || type == OBJECT_TYPE_UNIT) {
				cv._id = id;
				cv._component_id = component_id;
				cv._visible = true;
				cv.update();

				if (id == owner_id)
					cv._expander.get_style_context().remove_class("inherited");
				else
					cv._expander.get_style_context().add_class("inherited");

				cv._expander.expanded = was_expanded;
			} else {
				cv._visible = false;
			}
		}

		_object_view._list_box.invalidate_filter();
		_object_view._list_box.invalidate_sort();
	}

	public void show_sound_source(Guid id)
	{
		foreach (var entry in _objects)
			_expander_states[entry.key] = entry.value._expander.expanded;

		_stack.set_visible_child_name(PROPERTIES);

		foreach (var entry in _objects) {
			string type = entry.key;
			PropertyGrid cv = entry.value;

			if (type == OBJECT_TYPE_SOUND_SOURCE) {
				bool was_expanded = _expander_states.has_key(type) ? _expander_states[type] : false;

				cv._id = id;
				cv._visible = true;
				cv.update();

				cv._expander.show();
				cv._expander.expanded = was_expanded;
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
			_stack.set_visible_child_name(NOTHING_TO_SHOW);
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
			_stack.set_visible_child_name(UNKNOWN_OBJECT_TYPE);
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
