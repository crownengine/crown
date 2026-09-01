/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class UnitComponentRow : Gtk.ListBoxRow
{
	public string _component_type;
	public string _ui_name;
	public string _ui_category;
	public double _ui_category_order;
	public double _ui_order;

	public UnitComponentRow(ObjectTypeInfo info, string ui_category, double ui_category_order)
	{
		_component_type = info.name;
		_ui_name = info.ui_name;
		_ui_category = ui_category;
		_ui_category_order = ui_category_order;
		_ui_order = info.ui_order;

		Gtk.Label label = new Gtk.Label(_ui_name);
		label.xalign = 0.0f;
		label.margin_start = 12;
		label.margin_end = 6;
		label.margin_top = 3;
		label.margin_bottom = 3;
#if CROWN_GTK3
		this.add(label);
#else
		this.set_child(label);
#endif
	}
}

public class UnitView : PropertyGrid
{
	public InputResource _prefab;
	public Gtk.Button _open_prefab;
	public Gtk.MenuButton _component_add;
	public Gtk.Box _components;
	public Gtk.Popover _add_popover;
	public Gtk.EventControllerKey _component_search_controller_key;

	public const GLib.ActionEntry[] actions =
	{
		{ "add-component", on_add_component, "s", null },
	};

	public void on_add_component(GLib.SimpleAction action, GLib.Variant? param)
	{
		string component_type = param.get_string();

		Guid unit_id = _id;
		Unit unit = Unit(_db, unit_id);

		if (unit.add_component_type_dependencies(component_type))
			_db.add_restore_point((int)ActionType.CHANGE_OBJECTS, { unit_id });
	}

	public bool select_component_if_visible(Gtk.ListBox component_list, Gtk.Widget child)
	{
		if (!child.get_child_visible())
			return false;

		component_list.select_row((Gtk.ListBoxRow)child);
		return true;
	}

	public void select_first_visible_component(Gtk.ListBox component_list)
	{
#if CROWN_GTK3
		foreach (Gtk.Widget child in component_list.get_children()) {
			if (select_component_if_visible(component_list, child))
				break;
		}
#else
		for (Gtk.Widget? child = component_list.get_first_child(); child != null; child = child.get_next_sibling()) {
			if (select_component_if_visible(component_list, child))
				break;
		}
#endif
	}

	public UnitView(Database db)
	{
		base(db);

		_action_group.add_action_entries(actions, this);

		_order = -1.0;

		// Widgets
		_prefab = new InputResource(OBJECT_TYPE_UNIT, db);
		_prefab._nullable = true;
		_prefab.value_changed.connect(on_prefab_value_changed);

		_open_prefab = new Gtk.Button.with_label(_("Open Prefab"));
		Gtk.Label open_prefab_label = (Gtk.Label)_open_prefab.get_child();
		open_prefab_label.ellipsize = Pango.EllipsizeMode.END;
		_open_prefab.sensitive = false;
		_open_prefab.clicked.connect(on_open_prefab_clicked);

		// Construct 'add components' button.
		EntrySearch search_entry = new EntrySearch();
		search_entry.set_placeholder_text(_("Search..."));

		Gtk.ListBox component_list = new Gtk.ListBox();
		component_list.selection_mode = Gtk.SelectionMode.SINGLE;

		GLib.HashTable<string, double?> category_orders = new GLib.HashTable<string, double?>(GLib.str_hash, GLib.str_equal);
		Unit._component_registry.foreach((component_type, _value) => {
				ObjectTypeInfo info = db.type_info(StringId64(component_type));
				string? category = info.ui_category;
				if (category != null
				&& (!category_orders.contains(category) || info.ui_order < category_orders[category])
				)
					category_orders[category] = info.ui_order;
			});

		Unit._component_registry.foreach((component_type, _value) => {
				ObjectTypeInfo info = db.type_info(StringId64(component_type));
				string category = info.ui_category != null ? info.ui_category : _("Other");
				double category_order = info.ui_category != null
				? category_orders[info.ui_category]
				: double.MAX;

#if CROWN_GTK3
				component_list.add(new UnitComponentRow(info, category, category_order));
#else
				component_list.append(new UnitComponentRow(info, category, category_order));
#endif
			});

		component_list.set_sort_func((row_a, row_b) => {
				UnitComponentRow a = (UnitComponentRow)row_a;
				UnitComponentRow b = (UnitComponentRow)row_b;

				if (a._ui_category_order != b._ui_category_order)
					return a._ui_category_order < b._ui_category_order ? -1 : 1;

				int result = a._ui_category.collate(b._ui_category);
				if (result != 0)
					return result;

				if (a._ui_order != b._ui_order)
					return a._ui_order < b._ui_order ? -1 : 1;

				result = a._ui_name.collate(b._ui_name);
				return result != 0 ? result : a._component_type.collate(b._component_type);
			});

		component_list.set_filter_func((row) => {
				UnitComponentRow component_row = (UnitComponentRow)row;
				string query = search_entry.text.casefold();
				return query == ""
				|| component_row._ui_name.casefold().contains(query)
				|| component_row._ui_category.casefold().contains(query);
			});

		component_list.set_header_func((row, before) => {
				UnitComponentRow component_row = (UnitComponentRow)row;
				UnitComponentRow? before_row = before != null ? (UnitComponentRow)before : null;

				if (before_row == null || component_row._ui_category != before_row._ui_category) {
					Gtk.Box header = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
					if (before != null)
#if CROWN_GTK3
						header.pack_start(new Gtk.Separator(Gtk.Orientation.HORIZONTAL), false, false, 0);
#else
						header.append(new Gtk.Separator(Gtk.Orientation.HORIZONTAL));
#endif

					Gtk.Label label = new Gtk.Label(component_row._ui_category);
					label.xalign = 0.0f;
					label.margin_start = 6;
					label.margin_top = 3;
					label.margin_bottom = 3;
#if CROWN_GTK3
					label.get_style_context().add_class(Gtk.STYLE_CLASS_DIM_LABEL);
					header.pack_start(label, false, false, 0);
#else
					label.add_css_class("dim-label");
					header.append(label);
#endif

					row.set_header(header);
#if CROWN_GTK3
					header.show_all();
#endif
				} else {
					row.set_header(null);
				}
			});

		component_list.row_activated.connect((row) => {
				UnitComponentRow component_row = (UnitComponentRow)row;
				_action_group.activate_action("add-component", new GLib.Variant.string(component_row._component_type));
				_add_popover.popdown();
			});

#if CROWN_GTK3
		Gtk.ScrolledWindow component_scroller = new Gtk.ScrolledWindow(null, null);
#else
		Gtk.ScrolledWindow component_scroller = new Gtk.ScrolledWindow();
#endif
		component_scroller.hscrollbar_policy = Gtk.PolicyType.NEVER;
		component_scroller.vscrollbar_policy = Gtk.PolicyType.AUTOMATIC;
		component_scroller.set_min_content_width(300);
		component_scroller.set_max_content_height(400);
		component_scroller.set_propagate_natural_height(true);
#if CROWN_GTK3
		component_scroller.add(component_list);
#else
		component_scroller.set_child(component_list);
#endif
		component_list.set_adjustment(component_scroller.get_vadjustment());

		search_entry.search_changed.connect(() => {
				component_list.invalidate_filter();
				component_list.invalidate_headers();
				component_list.unselect_all();
				select_first_visible_component(component_list);
			});
		search_entry._entry.activate.connect(() => {
				Gtk.ListBoxRow? row = component_list.get_selected_row();
				if (row != null)
					component_list.row_activated(row);
			});

#if CROWN_GTK3
		_component_search_controller_key = new Gtk.EventControllerKey(search_entry._entry);
#else
		_component_search_controller_key = new Gtk.EventControllerKey();
#endif
		_component_search_controller_key.key_pressed.connect((keyval, keycode, state) => {
				if (keyval == Gdk.Key.Down || keyval == Gdk.Key.Up) {
					int delta = keyval == Gdk.Key.Down ? 1 : -1;
					Gtk.ListBoxRow? row = component_list.get_selected_row();
					if (row == null)
						return Gdk.EVENT_STOP;

					int index = row.get_index();
					do {
						row = component_list.get_row_at_index(index += delta);
					} while (row != null && !row.get_child_visible());

					if (row != null) {
						component_list.select_row(row);
						row.grab_focus();
#if CROWN_GTK3
						search_entry._entry.grab_focus_without_selecting();
#else
						search_entry._entry.grab_focus();
#endif
					}
					return Gdk.EVENT_STOP;
				}

				if (keyval == Gdk.Key.Escape) {
					_add_popover.popdown();
					return Gdk.EVENT_STOP;
				}

				return Gdk.EVENT_PROPAGATE;
			});
#if !CROWN_GTK3
		search_entry._entry.add_controller(_component_search_controller_key);
#endif

		Gtk.Box popover_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 6);
#if CROWN_GTK3
		popover_box.margin = 6;
		popover_box.pack_start(search_entry, false, false);
		popover_box.pack_start(component_scroller);

		_add_popover = new Gtk.Popover(null);
		_add_popover.add(popover_box);
#else
		popover_box.margin_bottom = 6;
		popover_box.margin_end = 6;
		popover_box.margin_start = 6;
		popover_box.margin_top = 6;
		popover_box.append(search_entry);
		popover_box.append(component_scroller);

		_add_popover = new Gtk.Popover();
		_add_popover.set_child(popover_box);
#endif /* if CROWN_GTK3 */
		_add_popover.map.connect(() => {
#if CROWN_GTK3
				search_entry._entry.grab_focus_without_selecting();
				search_entry.search_changed();
#else
				search_entry._entry.grab_focus();
				search_entry.search_changed(search_entry);
#endif
			});
		_add_popover.closed.connect(() => {
				search_entry.text = "";
				component_list.unselect_all();
			});
#if CROWN_GTK3
		popover_box.show_all();
#endif

		_component_add = new Gtk.MenuButton();
		_component_add.label = _("Add Component");
		Gtk.Label component_add_label = (Gtk.Label)_component_add.get_child();
		component_add_label.ellipsize = Pango.EllipsizeMode.END;
		_component_add.set_popover(_add_popover);

		_components = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
		_components.homogeneous = true;
#if CROWN_GTK3
		_components.pack_start(_component_add);
#else
		_components.append(_component_add);
#endif

		add_row(_("Prefab"), _prefab);
		add_row(_("Prefab"), _open_prefab);
		add_row(_("Components"), _components);
	}

	public override void read_properties()
	{
		string? prefab = _db.get_resource(_id, "prefab");
		_prefab.value_changed.disconnect(on_prefab_value_changed);
		_prefab.value = prefab;
		_prefab.value_changed.connect(on_prefab_value_changed);
		_open_prefab.sensitive = prefab != null;
	}

	public void on_prefab_value_changed(InputField p, int undo_redo)
	{
		if (_id == GUID_ZERO)
			return;

		Unit unit = Unit(_db, _id);
		if (unit.set_prefab(_prefab.value) != 0) {
			read_properties();
			return;
		}

		_db.add_restore_point((int)ActionType.CHANGE_OBJECTS
			, new Guid?[] { _id }
			, ActionTypeFlags.RESPAWN_OBJECTS
			);
	}

	public void on_open_prefab_clicked()
	{
		GLib.Application.get_default().activate_action("open-unit", new GLib.Variant.string(_prefab.value));
	}
}

public class PropertiesView : Gtk.Box
{
	public const string NOTHING_TO_SHOW = "nothing-to-show";
	public const string UNKNOWN_OBJECT_TYPE = "unknown-object-type";
	public const string PROPERTIES = "properties";

	public DatabaseEditor _database_editor;
	public Database _database;
	public GLib.HashTable<string, bool> _expander_states;
	public GLib.HashTable<string, PropertyGrid> _objects;
	public Gtk.Viewport _viewport;
	public Gtk.ScrolledWindow _scrolled_window;
	public PropertyGridSet _object_view;
	public Gtk.Stack _stack;
	public Guid _id;

	public PropertiesView(DatabaseEditor database_editor)
	{
		Object(orientation: Gtk.Orientation.VERTICAL);

		// Data
		_database_editor = database_editor;
		_database_editor.selection_changed.connect(on_database_selection_changed);
		_database = database_editor._database;

		_expander_states = new GLib.HashTable<string, bool>(GLib.str_hash, GLib.str_equal);
		_objects = new GLib.HashTable<string, PropertyGrid>(GLib.str_hash, GLib.str_equal);

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
		Gtk.Label unknown_object_type = new Gtk.Label(_("Unknown object type"));
		unknown_object_type.ellipsize = Pango.EllipsizeMode.END;
		_stack.add_named(unknown_object_type, UNKNOWN_OBJECT_TYPE);
		_stack.add_named(_scrolled_window, PROPERTIES);
		_stack.vexpand = true;

		_id = GUID_ZERO;

#if CROWN_GTK3
		this.pack_start(_stack);
		this.get_style_context().add_class("properties-view");
#else
		this.append(_stack);
		this.add_css_class("properties-view");
#endif

		_database._project.project_reset.connect(on_project_reset);
	}

	public void register_object_type(string object_type, PropertyGrid? cv = null)
	{
		PropertyGrid? grid = cv;
		if (grid == null)
			grid = new PropertyGrid.from_object_type(StringId64(object_type), _database, _database_editor);

		_object_view.add_property_grid(grid, camel_case(object_type));
		_objects[object_type] = grid;
	}

	public void show_unit(Guid id)
	{
		_objects.foreach((type, cv) => _expander_states[type] = cv._expander.expanded);

		_stack.set_visible_child_name(PROPERTIES);

		_objects.foreach((type, cv) => {
				bool was_expanded = _expander_states.contains(type) ? _expander_states[type] : false;

				Unit unit = Unit(_database, id);
				Guid component_id;
				if (unit.has_component(out component_id, type) || type == OBJECT_TYPE_UNIT) {
					cv._id = id;
					cv._component_id = component_id;
					cv.show_grid();
					cv.read_properties();

					if (component_id != GUID_ZERO) {
						if (id == _database.owner(component_id))
#if CROWN_GTK3
							cv._expander.get_style_context().remove_class("inherited");
#else
							cv._expander.remove_css_class("inherited");
#endif
						else
#if CROWN_GTK3
							cv._expander.get_style_context().add_class("inherited");
#else
							cv._expander.add_css_class("inherited");
#endif
					}

					cv._expander.expanded = was_expanded;
				} else {
					cv.hide_grid();
					cv._id = GUID_ZERO;
					cv._component_id = GUID_ZERO;
				}
			});

		_object_view._list_box.invalidate_filter();
		_object_view._list_box.invalidate_sort();
	}

	public void show_sound_source(Guid id)
	{
		_objects.foreach((type, cv) => _expander_states[type] = cv._expander.expanded);

		_stack.set_visible_child_name(PROPERTIES);

		_objects.foreach((type, cv) => {
				if (type == OBJECT_TYPE_SOUND_SOURCE) {
					bool was_expanded = _expander_states.contains(type) ? _expander_states[type] : false;

					cv._id = id;
					cv.show_grid();
					cv.read_properties();

#if CROWN_GTK3
					cv._expander.show();
#endif
					cv._expander.expanded = was_expanded;
				} else {
					cv.hide_grid();
					cv._id = GUID_ZERO;
					cv._component_id = GUID_ZERO;
				}
			});

		_object_view._list_box.invalidate_filter();
		_object_view._list_box.invalidate_sort();
	}

	public void set_objects(Guid?[] objects)
	{
		if (objects.length == 0) {
			_stack.set_visible_child_name(NOTHING_TO_SHOW);
			_objects.foreach((type, cv) => {
					cv._id = GUID_ZERO;
					cv._component_id = GUID_ZERO;
				});
			return;
		}

		Guid id = objects[objects.length - 1];
		if (id == _id || !_database.has_object(id) || !_database.is_alive(id))
			return;

		_id = id;

		if (_database.object_type(id) == OBJECT_TYPE_UNIT)
			show_unit(id);
		else if (_database.object_type(id) == OBJECT_TYPE_SOUND_SOURCE)
			show_sound_source(id);
		else
			_stack.set_visible_child_name(UNKNOWN_OBJECT_TYPE);
	}

	public void on_project_reset()
	{
		set_objects({});
	}

	public void on_database_selection_changed()
	{
		GLib.GenericArray<Guid?> selection = _database_editor._selection;

		if (selection.length == 0)
			set_objects({ GUID_ZERO });
		else
			set_objects({ selection[selection.length - 1] });
	}
}

} /* namespace Crown */
