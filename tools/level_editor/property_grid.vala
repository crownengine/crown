/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
class ObjectsSetEditor : Gtk.Box
{
	public PropertyGrid _grid;
	public PropertyDefinition _definition;
	public Guid _object_id;
	public PropertyGrid? _editor_grid;
	public bool _refreshing;
	public Gtk.Button _add;
	public Gtk.Label _read_only_note;
	public Gtk.ListBox _list;
	public Gtk.Box _editor;

	public ObjectsSetEditor(PropertyGrid grid, PropertyDefinition definition)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 6);

		_grid = grid;
		_definition = definition;
		_object_id = GUID_ZERO;
		_editor_grid = null;
		_refreshing = false;

		_add = new Gtk.Button.from_icon_name("list-add-symbolic");
		_add.clicked.connect(on_add_clicked);

		_read_only_note = new Gtk.Label(_("Inherited sub-objects are read-only here. Edit the unit in Unit Editor."));
		_read_only_note.wrap = true;
		_read_only_note.xalign = 0.0f;
		_read_only_note.visible = false;
#if CROWN_GTK3
		_read_only_note.get_style_context().add_class("dim-label");
#else
		_read_only_note.add_css_class("dim-label");
#endif

		_list = new Gtk.ListBox();
		_list.selection_mode = Gtk.SelectionMode.SINGLE;
		_list.row_selected.connect(on_row_selected);

		_editor = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);

#if CROWN_GTK3
		this.pack_start(_add, false, false);
		this.pack_start(_read_only_note, false, false);
		this.pack_start(_list, false, true);
		this.pack_start(_editor, false, true);
#else
		this.append(_add);
		this.append(_read_only_note);
		this.append(_list);
		this.append(_editor);
#endif

		_grid._db.objects_created.connect((object_ids, flags) => { read(); });
		_grid._db.objects_destroyed.connect((object_ids, flags) => { read(); });
		_grid._db.objects_changed.connect((object_ids, flags) => { read(); });
	}

	public bool is_read_only()
	{
		return _grid._db.object_type(_grid._id) == OBJECT_TYPE_UNIT
			&& _grid._component_id != GUID_ZERO
			&& _grid._id != _grid._db.owner(_grid._component_id)
			;
	}

	public string object_name(Guid id)
	{
		StringId64 object_type = StringId64(_grid._db.object_type(id));
		Aspect? name_aspect = _grid._db.get_aspect(object_type, StringId64("name"));
		if (name_aspect == null)
			name_aspect = default_name_aspect;

		string name;
		name_aspect(out name, _grid._db, id);
		return name;
	}

	public void on_add_clicked()
	{
		if (is_read_only())
			return;

		string object_type = _grid._db.type_name(_definition.object_type);
		Guid object_id = Guid.new_guid();
		Guid owner_id = _grid._component_id != GUID_ZERO ? _grid._component_id : _grid._id;
		_object_id = object_id;

		_grid._db.create(object_id, object_type);
		_grid._db.add_to_set(owner_id, _definition.name, object_id);
		_grid._db.add_restore_point((int)ActionType.CREATE_OBJECTS, { object_id });
	}

	public void on_delete_clicked(Guid object_id)
	{
		if (is_read_only())
			return;

		if (Guid.equal_func(_object_id, object_id))
			_object_id = GUID_ZERO;

		_grid._db.destroy(object_id);
		_grid._db.add_restore_point((int)ActionType.DESTROY_OBJECTS, { object_id });
	}

	public void on_row_selected(Gtk.ListBoxRow? row)
	{
		if (_refreshing)
			return;

		if (row == null) {
			_object_id = GUID_ZERO;
			_editor_grid = null;
#if CROWN_GTK3
			_editor.foreach((widget) => {
					widget.destroy();
				});
#else
			clear_box(_editor);
#endif
			return;
		}

		_object_id = Guid.parse(row.get_data<string>("id"));
		if (_editor_grid == null || !Guid.equal_func(_editor_grid._id, _object_id)) {
#if CROWN_GTK3
			_editor.foreach((widget) => {
					widget.destroy();
				});
#else
			clear_box(_editor);
#endif

			Guid selection_anchor_id = _grid._selection_anchor_id != GUID_ZERO ? _grid._selection_anchor_id : _grid._id;
			_editor_grid = new PropertyGrid.from_object(_object_id, _grid._db, _grid._database_editor, selection_anchor_id);
#if CROWN_GTK3
			_editor.pack_start(_editor_grid, false, true);
#else
			_editor.append(_editor_grid);
#endif
		}
		_editor_grid.sensitive = !is_read_only();
		_editor_grid.read_properties();
#if CROWN_GTK3
		_editor.show_all();
#endif
	}

#if !CROWN_GTK3
	public void clear_box(Gtk.Box box)
	{
		Gtk.Widget? child = box.get_first_child();
		while (child != null) {
			Gtk.Widget? next = child.get_next_sibling();
			box.remove(child);
			child = next;
		}
	}
#endif

#if !CROWN_GTK3
	public void clear_list()
	{
		Gtk.Widget? child = _list.get_first_child();
		while (child != null) {
			Gtk.Widget? next = child.get_next_sibling();
			_list.remove(child);
			child = next;
		}
	}
#endif

	public void read()
	{
		_refreshing = true;
#if CROWN_GTK3
		_list.foreach((widget) => {
				widget.destroy();
			});
#else
		clear_list();
#endif

		if (_grid._id == GUID_ZERO || !_grid._db.is_alive(_grid._id)) {
			_editor_grid = null;
#if CROWN_GTK3
			_editor.foreach((widget) => {
					widget.destroy();
				});
#else
			clear_box(_editor);
#endif
			_refreshing = false;
			return;
		}

		bool read_only = is_read_only();
		_add.sensitive = !read_only;
		_read_only_note.visible = read_only;

		Guid owner_id = _grid._component_id != GUID_ZERO ? _grid._component_id : _grid._id;
		Guid?[] children;
		if (_grid._db.object_type(_grid._id) == OBJECT_TYPE_UNIT && _grid._component_id != GUID_ZERO) {
			Unit unit = Unit(_grid._db, _grid._id);
			GLib.GenericSet<Guid?> component_children = (GLib.GenericSet<Guid?>)unit.get_component_property(_grid._component_id, _definition.name, guid_set_new());
			GLib.GenericArray<Guid?> live_children = new GLib.GenericArray<Guid?>();
			foreach (unowned Guid? child_id in component_children) {
				if (_grid._db.is_alive(child_id))
					live_children.add(child_id);
			}
			children = live_children.steal();
		} else {
			children = _grid._db.get_set(owner_id, _definition.name);
		}

		GLib.qsort_with_data<Guid?>(children, sizeof(Guid?), (a, b) => {
				return strcmp(object_name(a), object_name(b));
			});

		Gtk.ListBoxRow? row_to_select = null;
		foreach (unowned Guid? item_id in children) {
			Guid child_id = item_id;
			Gtk.ListBoxRow row = new Gtk.ListBoxRow();
			row.set_data("id", child_id.to_string());

			Gtk.Box box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
			Gtk.Label label = new Gtk.Label(object_name(child_id));
			label.hexpand = true;
			label.xalign = 0.0f;
#if CROWN_GTK3
			box.pack_start(label, true, true);
#else
			box.append(label);
#endif

			Gtk.Button remove = new Gtk.Button.from_icon_name("list-remove-symbolic");
			remove.sensitive = !read_only;
			remove.clicked.connect(() => {
					on_delete_clicked(child_id);
				});
#if CROWN_GTK3
			box.pack_end(remove, false);

			row.add(box);
			_list.add(row);
#else
			box.append(remove);

			row.set_child(box);
			_list.append(row);
#endif

			if (Guid.equal_func(_object_id, child_id))
				row_to_select = row;
		}

#if CROWN_GTK3
		_list.show_all();
#endif
		_refreshing = false;
		if (row_to_select != null) {
			_list.select_row(row_to_select);
			on_row_selected(row_to_select);
		} else {
			_object_id = GUID_ZERO;
			_editor_grid = null;
#if CROWN_GTK3
			_editor.foreach((widget) => {
					widget.destroy();
				});
#else
			clear_box(_editor);
#endif
		}
	}
}

public class PropertyGrid : Gtk.Grid
{
	GLib.ActionEntry[] actions =
	{
		{ "remove",         on_remove,         null, null },
		{ "reset_property", on_reset_property, "s",  null },
	};

	public Expander? _expander;
	public Gtk.GestureSingle _controller_click;
	public GLib.SimpleActionGroup _action_group;
	public Database? _db;
	public StringId64 _type;
	public Guid _id;
	public Guid _component_id;
	public int _rows;
	public double _order;
	public bool _visible;
	public Gtk.SizeGroup? _label_size_group;
	public InputBool? _expander_input_bool;
	public Gtk.Widget? _expander_input_label;
	public int _expander_input_row;
	public DatabaseEditor? _database_editor;
	public Guid _selection_anchor_id;

	public GLib.HashTable<string, Gtk.GestureSingle> _gestures;
	public GLib.HashTable<string, InputField> _widgets;
	public GLib.HashTable<InputField, PropertyDefinition?> _definitions;
	GLib.GenericArray<ObjectsSetEditor> _object_sets;

	public void on_remove(GLib.SimpleAction action, GLib.Variant? param)
	{
		string component_type = _db.type_name(_type);
		Guid unit_id = _id;
		Unit unit = Unit(_db, unit_id);

		Guid component_id;
		if (!unit.has_component(out component_id, component_type))
			return;

		GLib.GenericArray<unowned string> dependents = new GLib.GenericArray<unowned string>();
		// Do not remove if any other component needs us.
		Unit._component_registry.foreach((registered_type, dependencies_value) => {
				Guid dummy;
				if (!unit.has_component(out dummy, registered_type))
					return;

				string[] component_type_dependencies = ((string)dependencies_value).split(", ");
				if (component_type in component_type_dependencies)
					dependents.add(registered_type);
			});

		if (dependents.length > 0) {
			StringBuilder sb = new StringBuilder();
			sb.append(_("Cannot remove %s due to the following dependencies:\n\n").printf(component_type));
			for (int i = 0; i < dependents.length; ++i)
				sb.append("• %s\n".printf(dependents[i]));

			Gtk.MessageDialog md = new Gtk.MessageDialog(null
				, Gtk.DialogFlags.MODAL
				, Gtk.MessageType.WARNING
				, Gtk.ButtonsType.OK
				, sb.str
				);
			md.set_default_response(Gtk.ResponseType.OK);

			md.response.connect(() => { md.destroy(); });
#if CROWN_GTK3
			md.show_all();
#else
			md.show();
#endif
			return;
		} else {
			unit.remove_component_type(component_type);
		}
	}

	public void on_reset_property(GLib.SimpleAction action, GLib.Variant? param)
	{
		string property_name = param.get_string();

		InputField field = _widgets[property_name];
		PropertyDefinition? def = _definitions[field];
		field.set_union_value(def.deffault);
	}

	public void on_expander_button_released(int n_press, double x, double y)
	{
		if (_controller_click.get_current_button() == Gdk.BUTTON_SECONDARY) {
			GLib.Menu menu = new GLib.Menu();
			GLib.MenuItem mi;

			if (_db != null && _db.object_type(_id) == OBJECT_TYPE_UNIT && _component_id != GUID_ZERO) {
				mi = new GLib.MenuItem(_("Remove Component"), null);
				mi.set_action_and_target_value("object.remove", null);
				menu.append_item(mi);
			}

			if (menu.get_n_items() > 0) {
#if CROWN_GTK3
				Gtk.Popover popover = new Gtk.Popover.from_model(null, menu);
				popover.set_relative_to(this);
#else
				Gtk.PopoverMenu popover = new Gtk.PopoverMenu.from_model(menu);
				popover.set_parent(_expander);
#endif
				popover.set_pointing_to({ (int)x, (int)y, 1, 1 });
				popover.set_position(Gtk.PositionType.BOTTOM);
				popover.popup();
			}
		}
	}

	public PropertyGrid(Database? db = null, DatabaseEditor? database_editor = null, Guid selection_anchor_id = GUID_ZERO)
	{
		this.row_spacing = 4;
		this.row_homogeneous = true;
		this.column_spacing = 12;

		// Data
		_expander = null;
		_db = db;
		_database_editor = database_editor;
		_id = GUID_ZERO;
		_component_id = GUID_ZERO;
		_rows = 0;
		_order = 0.0;
		_visible = true;
		_label_size_group = null;
		_expander_input_bool = null;
		_expander_input_label = null;
		_expander_input_row = -1;
		_selection_anchor_id = selection_anchor_id;

		_gestures = new GLib.HashTable<string, Gtk.GestureSingle>(GLib.str_hash, GLib.str_equal);
		_widgets = new GLib.HashTable<string, InputField>(GLib.str_hash, GLib.str_equal);
		_definitions = new GLib.HashTable<InputField, PropertyDefinition?>(GLib.direct_hash, GLib.direct_equal);
		_object_sets = new GLib.GenericArray<ObjectsSetEditor>();

		_action_group = new GLib.SimpleActionGroup();
		_action_group.add_action_entries(actions, this);
		this.insert_action_group("object", _action_group);
	}

	public PropertyGrid.from_object_type(StringId64 type, Database db, DatabaseEditor? database_editor = null, Guid selection_anchor_id = GUID_ZERO)
	{
		this(db, database_editor, selection_anchor_id);

		_order = db.type_info(type).ui_order;
		_type = type;
		add_object_type(db.object_definition(type));
	}

	public PropertyGrid.from_object(Guid id, Database db, DatabaseEditor? database_editor = null, Guid selection_anchor_id = GUID_ZERO)
	{
		this.from_object_type(StringId64(db.object_type(id)), db, database_editor, selection_anchor_id);
		_id = id;
	}

	public void set_expander(Expander e)
	{
		assert(_expander == null);

		_expander = e;

#if CROWN_GTK3
		_controller_click = new Gtk.GestureMultiPress(e);
		((Gtk.GestureMultiPress)_controller_click).released.connect(on_expander_button_released);
#else
		_controller_click = new Gtk.GestureClick();
		((Gtk.GestureClick)_controller_click).released.connect(on_expander_button_released);
		e.add_controller(_controller_click);
#endif
		_controller_click.set_button(0);
	}

	public void set_label_size_group(Gtk.SizeGroup size_group)
	{
		assert(_label_size_group == null);

		_label_size_group = size_group;
		add_labels_to_size_group();
	}

	private void add_labels_to_size_group()
	{
		if (_label_size_group == null)
			return;

		for (int row = 0; row < _rows; ++row) {
			Gtk.Widget? child = this.get_child_at(0, row);
			if (child != null)
				_label_size_group.add_widget(child);
		}
	}

	private void remove_labels_from_size_group()
	{
		if (_label_size_group == null)
			return;

		for (int row = 0; row < _rows; ++row) {
			Gtk.Widget? child = this.get_child_at(0, row);
			if (child != null)
				_label_size_group.remove_widget(child);
		}
	}

	public void show_grid()
	{
		if (_visible)
			return;

		_visible = true;
		add_labels_to_size_group();
	}

	public void hide_grid()
	{
		if (!_visible)
			return;

		remove_labels_from_size_group();
		_visible = false;
	}

	public Gtk.Widget add_row(string label, Gtk.Widget w, string? tooltip = null)
	{
		Gtk.Label l = new Gtk.Label(label);
		l.ellipsize = Pango.EllipsizeMode.END;
		l.xalign = 1.0f;
		l.yalign = 0.5f;
		l.set_tooltip_text(tooltip);
		if (_visible && _label_size_group != null)
			_label_size_group.add_widget(l);

		w.hexpand = true;

		this.attach(l, 0, (int)_rows);
		this.attach(w, 1, (int)_rows);
		++_rows;

		return l;
	}

	public InputBool? steal_enabled_input_bool()
	{
		if (_expander_input_bool == null)
			return null;

		assert(_expander_input_label != null);
		assert(_expander_input_row >= 0);

		InputBool input_bool = (InputBool)_expander_input_bool;
		Gtk.Widget input_label = (Gtk.Widget)_expander_input_label;
		if (_visible && _label_size_group != null)
			_label_size_group.remove_widget(input_label);

		this.remove(input_bool);
		this.remove(input_label);
		this.remove_row(_expander_input_row);
		--_rows;

		_expander_input_bool = null;
		_expander_input_label = null;
		_expander_input_row = -1;

		return input_bool;
	}

	public void add_object_type(PropertyDefinition[] properties)
	{
		int[] property_groups = new int[properties.length];
		uint[] group_sizes = new uint[properties.length];
		Gtk.Box?[] group_boxes = new Gtk.Box?[properties.length];
		Gtk.SizeGroup?[] group_input_size_groups = new Gtk.SizeGroup?[properties.length];
		int pi;

		// Assign matching properties to a group led by its first property.
		for (pi = 0; pi < properties.length; ++pi) {
			property_groups[pi] = -1;
			PropertyDefinition def = properties[pi];
			if (def.hidden || def.type == PropertyType.OBJECTS_SET)
				continue;

			int dot = def.name.last_index_of_char('.');
			int underscore = def.name.last_index_of_char('_');
			if (underscore <= dot + 1 || underscore == def.name.length - 1)
				continue;

			int group_index = pi;
			string group = def.name.substring(0, underscore);
			for (int ii = 0; ii < pi; ++ii) {
				PropertyDefinition other = properties[ii];
				if (property_groups[ii] == -1 || other.type != def.type)
					continue;

				int other_underscore = other.name.last_index_of_char('_');
				if (group == other.name.substring(0, other_underscore)) {
					group_index = property_groups[ii];
					break;
				}
			}

			property_groups[pi] = group_index;
			group_sizes[group_index]++;
		}

		pi = 0;
		foreach (PropertyDefinition def in properties) {
			int group_index = property_groups[pi++];

			// Create input field.
			InputField? p = null;

			switch (def.type) {
			case PropertyType.BOOL:
				p = new InputBool();
				break;
			case PropertyType.DOUBLE:
				if (def.editor == PropertyEditorType.ANGLE)
					p = new InputAngle((double)def.deffault, (double)def.min, (double)def.max);
				else
					p = new InputDouble((double)def.deffault
						, (double)def.min
						, (double)def.max
						, InputDouble.DEFAULT_PREVIEW_DECIMALS
						, InputDouble.DEFAULT_EDIT_DECIMALS
						, def.input_double_flags
						);
				break;
			case PropertyType.STRING:
				if (def.editor == PropertyEditorType.ENUM)
					p = new InputEnum((string)def.deffault, def.enum_labels, def.enum_values);
				else
					p = new InputString();
				break;
			case PropertyType.VECTOR3:
				if (def.editor == PropertyEditorType.COLOR)
					p = new InputColor3();
				else if (def.editor == PropertyEditorType.ANGLE)
					p = new InputAngle3((Vector3)def.deffault, (Vector3)def.min, (Vector3)def.max);
				else
					p = new InputVector3((Vector3)def.deffault, (Vector3)def.min, (Vector3)def.max);
				break;
			case PropertyType.QUATERNION:
				p = new InputQuaternion();
				break;
			case PropertyType.RESOURCE:
				p = new InputResource(def.resource_type, _db);
				break;
			case PropertyType.REFERENCE:
				p = new InputObject(def.object_type, _db);
				break;
			case PropertyType.OBJECTS_SET:
				if (_database_editor == null)
					continue;
				this.row_homogeneous = false;
				ObjectsSetEditor set_editor = new ObjectsSetEditor(this, def);
				_object_sets.add(set_editor);

				if (!def.hidden) {
					Gtk.Widget label = add_row(def.label, set_editor, def.tooltip);
					label.valign = Gtk.Align.START;
				}
				continue;
				default:
				assert(false);
				break;
			}

			p.value_changed.connect(on_property_value_changed);

			Gtk.GestureSingle click;
#if CROWN_GTK3
			click = new Gtk.GestureMultiPress(p);
#else
			click = new Gtk.GestureClick();
#endif
			click.set_propagation_phase(Gtk.PropagationPhase.CAPTURE);
			click.set_button(Gdk.BUTTON_SECONDARY);
#if CROWN_GTK3
			((Gtk.GestureMultiPress)click).pressed.connect((n_press, x, y) => {
#else
			((Gtk.GestureClick)click).pressed.connect((n_press, x, y) => {
#endif
					if (click.get_current_button() != Gdk.BUTTON_SECONDARY)
						return;

					GLib.Menu menu = new GLib.Menu();
					GLib.MenuItem mi;

					if (_db != null) {
						mi = new GLib.MenuItem(_("Reset to default"), null);
						mi.set_action_and_target_value("object.reset_property", new GLib.Variant.string(def.name));
						menu.append_item(mi);
					}

					if (menu.get_n_items() > 0) {
#if CROWN_GTK3
						Gtk.Popover popover = new Gtk.Popover.from_model(null, menu);
						popover.set_relative_to(p);
#else
						Gtk.PopoverMenu popover = new Gtk.PopoverMenu.from_model(menu);
#endif
						popover.set_position(Gtk.PositionType.BOTTOM);
						popover.popup();
					}
				});
#if !CROWN_GTK3
			p.add_controller(click);
#endif

			_gestures[def.name] = click;
			_widgets[def.name] = p;
			_definitions[p] = def;

			if (!def.hidden) {
				bool grouped = group_index != -1 && group_sizes[group_index] > 1;

				if (grouped) {
					// Create or retrieve the group's row and input size group.
					int underscore = def.name.last_index_of_char('_');
					string group = def.name.substring(0, underscore);
					string suffix = def.name.substring(underscore + 1);
					bool first = group_boxes[group_index] == null;
					Gtk.Box box;
					Gtk.SizeGroup input_size_group;
					if (first) {
						box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 4);
						group_boxes[group_index] = box;
						input_size_group = new Gtk.SizeGroup(Gtk.SizeGroupMode.HORIZONTAL);
						group_input_size_groups[group_index] = input_size_group;
					} else {
						box = (Gtk.Box)group_boxes[group_index];
						input_size_group = (Gtk.SizeGroup)group_input_size_groups[group_index];
					}

					bool axis = def.type == PropertyType.DOUBLE
						&& (suffix == "x" || suffix == "y" || suffix == "z" || suffix == "w")
						;

					// Merge the first suffix into the row label; label later fields inline.
					Gtk.Widget member = p;
					if (first) {
						string label = camel_case(group.substring(group.last_index_of_char('.') + 1));
						if (!axis)
							label += " " + camel_case(suffix);
						add_row(label, box, def.tooltip);
					} else if (!axis) {
						Gtk.Box labeled_field = new Gtk.Box(Gtk.Orientation.HORIZONTAL, (int)this.column_spacing);
						Gtk.Label label = new Gtk.Label(camel_case(suffix));
						label.xalign = 1.0f;
						label.yalign = 0.5f;
						label.set_tooltip_text(def.tooltip);
#if CROWN_GTK3
						labeled_field.pack_start(label, false, false);
						labeled_field.pack_start(p, true, true);
#else
						labeled_field.append(label);
						labeled_field.append(p);
#endif
						member = labeled_field;
					}

					if (axis) {
#if CROWN_GTK3
						p.get_style_context().add_class("axis");
						p.get_style_context().add_class(suffix);
#else
						p.add_css_class("axis");
						p.add_css_class(suffix);
#endif
					}

					input_size_group.add_widget(p);
					p.set_tooltip_text(def.tooltip);
					p.hexpand = true;
					member.hexpand = true;
#if CROWN_GTK3
					box.pack_start(member, true, true);
#else
					box.append(member);
#endif
				} else {
					Gtk.Widget label = add_row(def.label, p, def.tooltip);

					if (def.type == PropertyType.BOOL
						&& (def.name == "enabled" || def.name.has_suffix(".enabled"))
						) {
						assert(_expander_input_bool == null);
						_expander_input_bool = (InputBool)p;
						_expander_input_label = label;
						_expander_input_row = _rows - 1;
						p.set_tooltip_text(def.tooltip);
					}
				}
			}
		}
	}

	// Returns true if the property was written.
	// The property is written to database only if its value
	// differs than the value stored in the database.
	public bool write_property_if_changed(PropertyDefinition def, GLib.Value? new_value)
	{
		bool changed = false;

		if (def.type == PropertyType.BOOL) {
			if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
				Unit u = Unit(_db, _id);
				if (u.get_component_bool(_component_id, def.name, (bool)def.deffault) != new_value) {
					u.set_component_bool(_component_id, def.name, (bool)new_value);
					changed = true;
				}
			} else {
				if (_db.get_bool(_id, def.name, (bool)def.deffault) != new_value) {
					_db.set_bool(_id, def.name, (bool)new_value);
					changed = true;
				}
			}
		} else if (def.type == PropertyType.DOUBLE) {
			if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
				Unit u = Unit(_db, _id);
				if (u.get_component_double(_component_id, def.name, (double)def.deffault) != new_value) {
					u.set_component_double(_component_id, def.name, (double)new_value);
					changed = true;
				}
			} else {
				if (_db.get_double(_id, def.name, (double)def.deffault) != new_value) {
					_db.set_double(_id, def.name, (double)new_value);
					changed = true;
				}
			}
		} else if (def.type == PropertyType.STRING) {
			if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
				Unit u = Unit(_db, _id);
				if (u.get_component_string(_component_id, def.name, (string)def.deffault) != (string)new_value) {
					u.set_component_string(_component_id, def.name, (string)new_value);
					changed = true;
				}
			} else {
				if (_db.get_string(_id, def.name, (string)def.deffault) != (string)new_value) {
					_db.set_string(_id, def.name, (string)new_value);
					changed = true;
				}
			}
		} else if (def.type == PropertyType.VECTOR3) {
			if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
				Unit u = Unit(_db, _id);
				if (Vector3.equal_func(u.get_component_vector3(_component_id, def.name, (Vector3)def.deffault), (Vector3)new_value) == false) {
					u.set_component_vector3(_component_id, def.name, (Vector3)new_value);
					changed = true;
				}
			} else {
				if (Vector3.equal_func(_db.get_vector3(_id, def.name, (Vector3)def.deffault), (Vector3)new_value) == false) {
					_db.set_vector3(_id, def.name, (Vector3)new_value);
					changed = true;
				}
			}
		} else if (def.type == PropertyType.QUATERNION) {
			if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
				Unit u = Unit(_db, _id);
				if (Quaternion.equal_func(u.get_component_quaternion(_component_id, def.name, (Quaternion)def.deffault), (Quaternion)new_value) == false) {
					u.set_component_quaternion(_component_id, def.name, (Quaternion)new_value);
					changed = true;
				}
			} else {
				if (Quaternion.equal_func(_db.get_quaternion(_id, def.name, (Quaternion)def.deffault), (Quaternion)new_value) == false) {
					_db.set_quaternion(_id, def.name, (Quaternion)new_value);
					changed = true;
				}
			}
		} else if (def.type == PropertyType.RESOURCE) {
			if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
				Unit u = Unit(_db, _id);
				if (u.get_component_resource(_component_id, def.name, (string?)def.deffault) != (string?)new_value) {
					u.set_component_resource(_component_id, def.name, (string?)new_value);
					changed = true;
				}
			} else {
				if (_db.get_resource(_id, def.name, (string?)def.deffault) != (string?)new_value) {
					_db.set_resource(_id, def.name, (string?)new_value);
					changed = true;
				}
			}
		} else if (def.type == PropertyType.REFERENCE) {
			if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
				Unit u = Unit(_db, _id);
				if (Guid.equal_func(u.get_component_reference(_component_id, def.name, (Guid)def.deffault), (Guid)new_value) == false) {
					u.set_component_reference(_component_id, def.name, (Guid)new_value);
					changed = true;
				}
			} else {
				if (Guid.equal_func(_db.get_reference(_id, def.name, (Guid)def.deffault), (Guid)new_value) == false) {
					_db.set_reference(_id, def.name, (Guid)new_value);
					changed = true;
				}
			}
		} else {
			loge("Unknown property type");
		}

		return changed;
	}

	public void on_property_value_changed(InputField p, int undo_redo)
	{
		if (p.is_inconsistent())
			return;
		if (_id == GUID_ZERO)
			return;

		PropertyDefinition def = _definitions[p];
		GLib.GenericArray<PropertyDefinition?> dynamic_properties = new GLib.GenericArray<PropertyDefinition?>();
		GLib.GenericArray<GLib.Value?> dynamic_values = new GLib.GenericArray<GLib.Value?>();
		bool changed = false;

		save_dynamic_properties_values(ref dynamic_properties, ref dynamic_values);
		read_dynamic_properties_ranges_except({ def });

		UndoRedo? ur = null;
		if (undo_redo == 0 || undo_redo == -1)
			ur = _db.disable_undo();

		changed = restore_dynamic_properties_values_except(dynamic_properties, dynamic_values, { def }) || changed;
		changed = write_property_if_changed(def, p.union_value()) || changed;

		if (changed && undo_redo != -1)
			_db.add_restore_point(ActionType.CHANGE_OBJECTS, new Guid?[] { _id });

		if (undo_redo == 0 || undo_redo == -1)
			_db.restore_undo(ur);

		if (_database_editor != null && _selection_anchor_id != GUID_ZERO)
			_database_editor.selection_set({ _selection_anchor_id });
	}

	public void read_all_properties()
	{
		GLib.HashTableIter<InputField, PropertyDefinition?> iter = GLib.HashTableIter<InputField, PropertyDefinition?>(_definitions);
		unowned InputField p;
		unowned PropertyDefinition? def;
		while (iter.next(out p, out def)) {
				p.value_changed.disconnect(on_property_value_changed);

				if (def.type == PropertyType.BOOL) {
					if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
						Unit u = Unit(_db, _id);
						p.set_union_value(u.get_component_bool(_component_id, def.name, (bool)def.deffault));
					} else {
						p.set_union_value(_db.get_bool(_id, def.name, (bool)def.deffault));
					}
				} else if (def.type == PropertyType.DOUBLE) {
					if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
						Unit u = Unit(_db, _id);
						p.set_union_value(u.get_component_double(_component_id, def.name, (double)def.deffault));
					} else {
						p.set_union_value(_db.get_double(_id, def.name, (double)def.deffault));
					}
				} else if (def.type == PropertyType.STRING) {
					if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
						Unit u = Unit(_db, _id);
						p.set_union_value(u.get_component_string(_component_id, def.name, (string)def.deffault));
					} else {
						p.set_union_value(_db.get_string(_id, def.name, (string)def.deffault));
					}
				} else if (def.type == PropertyType.VECTOR3) {
					if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
						Unit u = Unit(_db, _id);
						p.set_union_value(u.get_component_vector3(_component_id, def.name, (Vector3)def.deffault));
					} else {
						p.set_union_value(_db.get_vector3(_id, def.name, (Vector3)def.deffault));
					}
				} else if (def.type == PropertyType.QUATERNION) {
					if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
						Unit u = Unit(_db, _id);
						p.set_union_value(u.get_component_quaternion(_component_id, def.name, (Quaternion)def.deffault));
					} else {
						p.set_union_value(_db.get_quaternion(_id, def.name, (Quaternion)def.deffault));
					}
				} else if (def.type == PropertyType.RESOURCE) {
					if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
						Unit u = Unit(_db, _id);
						p.set_union_value(u.get_component_resource(_component_id, def.name, (string?)def.deffault));
					} else {
						p.set_union_value(_db.get_resource(_id, def.name, (string?)def.deffault));
					}
				} else if (def.type == PropertyType.REFERENCE) {
					if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
						Unit u = Unit(_db, _id);
						p.set_union_value(u.get_component_reference(_component_id, def.name, (Guid)def.deffault));
					} else {
						p.set_union_value(_db.get_reference(_id, def.name, (Guid)def.deffault));
					}
				} else {
					loge("Unknown property value type");
				}

			p.value_changed.connect(on_property_value_changed);
		}
	}

	public virtual void read_properties()
	{
		read_all_properties();
		read_dynamic_properties_ranges();
		read_all_properties();
		for (int i = 0; i < _object_sets.length; ++i)
			_object_sets[i].read();
	}

	public void read_dynamic_properties_ranges_except(PropertyDefinition[] excluded)
	{
		GLib.HashTableIter<InputField, PropertyDefinition?> iter = GLib.HashTableIter<InputField, PropertyDefinition?>(_definitions);
		unowned InputField p;
		unowned PropertyDefinition? def;
		while (iter.next(out p, out def)) {
				int i;

				// Skip if excluded.
				for (i = 0; i < excluded.length; ++i) {
					if (excluded[i].name == def.name)
						break;
				}
				if (i != excluded.length)
					continue;

				// Read range.
				if (def.enum_callback != null) {
					InputField field = _widgets[def.name];
					InputField parent_p = _widgets[def.enum_property];

					field.value_changed.disconnect(on_property_value_changed);
					def.enum_callback(parent_p, (InputEnum)field, _db._project);
					field.value_changed.connect(on_property_value_changed);
				} else if (def.resource_callback != null) {
					InputField field = _widgets[def.name];
					InputField parent_p = _widgets[def.enum_property];

					field.value_changed.disconnect(on_property_value_changed);
					def.resource_callback(parent_p, (InputResource)field, _db._project);
					field.value_changed.connect(on_property_value_changed);
			}
		}
	}

	public void read_dynamic_properties_ranges()
	{
		read_dynamic_properties_ranges_except({});
	}

	public void save_dynamic_properties_values(ref GLib.GenericArray<PropertyDefinition?> properties, ref GLib.GenericArray<GLib.Value?> values)
	{
		GLib.GenericArray<PropertyDefinition?> properties_local = properties;
		GLib.GenericArray<GLib.Value?> values_local = values;

		GLib.HashTableIter<InputField, PropertyDefinition?> iter = GLib.HashTableIter<InputField, PropertyDefinition?>(_definitions);
		unowned InputField p;
		unowned PropertyDefinition? def;
		while (iter.next(out p, out def)) {
				if (def.enum_callback != null) {
					InputField field = _widgets[def.name];

					properties_local.add(def);
					values_local.add(field.union_value());
				} else if (def.resource_callback != null) {
					InputField field = _widgets[def.name];

					properties_local.add(def);
					values_local.add(field.union_value());
			}
		}
	}

	public bool restore_dynamic_properties_values_except(GLib.GenericArray<PropertyDefinition?> properties, GLib.GenericArray<GLib.Value?> values, PropertyDefinition[] excluded)
	{
		bool changed = false;

		for (int i = 0; i < properties.length; ++i) {
			PropertyDefinition def = properties[i];
			GLib.Value val = values[i];
			InputField p = _widgets[def.name];
			int j;

			// Skip if excluded.
			for (j = 0; j < excluded.length; ++j) {
				if (excluded[j].name == def.name)
					break;
			}
			if (j != excluded.length)
				continue;

			// Restore value.
			p.value_changed.disconnect(on_property_value_changed);

			if (def.enum_callback != null) {
				p.set_union_value(val);

				if (p.is_inconsistent() || !p.is_inconsistent() && (string)p.union_value() != (string)val)
					p.set_union_value(((InputEnum)p).any_valid_id());
			} else if (def.resource_callback != null) {
				p.set_union_value(val);
			}

			p.value_changed.connect(on_property_value_changed);

			changed = write_property_if_changed(def, p.union_value()) || changed;
		}

		return changed;
	}

	public bool restore_dynamic_properties_values(GLib.GenericArray<PropertyDefinition?> properties, GLib.GenericArray<GLib.Value?> values)
	{
		return restore_dynamic_properties_values_except(properties, values, {});
	}
}

public class PropertyGridSet : Gtk.Box
{
	public Gtk.ListBox _list_box;
	public Gtk.SizeGroup _label_size_group;

	public PropertyGridSet()
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

		_label_size_group = new Gtk.SizeGroup(Gtk.SizeGroupMode.HORIZONTAL);

		_list_box = new Gtk.ListBox();
		_list_box.selection_mode = Gtk.SelectionMode.NONE;
		_list_box.margin_bottom
			= this.margin_end
			= this.margin_start
			= this.margin_top
			= 12
			;
		_list_box.set_sort_func(sort_function);
		_list_box.set_filter_func(filter_function);

#if CROWN_GTK3
		this.pack_start(_list_box);
#else
		this.append(_list_box);
#endif
	}

	public static int sort_function(Gtk.ListBoxRow row1, Gtk.ListBoxRow row2)
	{
		Expander e1 = (Expander)row1.get_child();
		Expander e2 = (Expander)row2.get_child();
		double order = ((PropertyGrid)e1._child)._order - ((PropertyGrid)e2._child)._order;
		return (int)order;
	}

	public static bool filter_function(Gtk.ListBoxRow row)
	{
		Expander e = (Expander)row.get_child();
		return ((PropertyGrid)e._child)._visible;
	}

	private void set_header(Expander expander, Gtk.Label label, InputBool? input_bool)
	{
		expander.custom_header = label;
		if (input_bool == null)
			return;

		InputBool field = (InputBool)input_bool;
		field.hexpand = false;
#if CROWN_GTK3
		expander._header_box.pack_start(field, false, false);
		expander._header_box.reorder_child(field, 1);
#else
		expander._header_box.insert_child_after(field, expander._arrow_image);
#endif
	}

	public Expander add_property_grid(PropertyGrid cv, string label, string? tooltip = null)
	{
		InputBool? input_bool = cv.steal_enabled_input_bool();
		Gtk.Label l = new Gtk.Label(null);
		l.set_markup("<b>%s</b>".printf(label));
		l.xalign = 0.0f;
		l.yalign = 0.5f;
		l.set_tooltip_text(label);

		Expander e = new Expander();
		set_header(e, l, input_bool);
		e.expanded = true;
		e.add(cv);
		cv.set_expander(e);
		cv.set_label_size_group(_label_size_group);

		Gtk.ListBoxRow row = new Gtk.ListBoxRow();
#if CROWN_GTK3
		row.can_focus = false;
		row.add(e);

		_list_box.add(row);
#else
		row.focusable = false;
		row.set_child(e);

		_list_box.append(row);
#endif

		return e;
	}

	public Expander add_property_grid_optional(PropertyGrid cv, string label, InputBool InputBool, string? tooltip = null)
	{
		Gtk.Label l = new Gtk.Label(null);
		l.set_markup("<b>%s</b>".printf(label));
		l.xalign = 0.0f;
		l.yalign = 0.5f;
		l.set_tooltip_text(tooltip);

		Expander e = new Expander();
		set_header(e, l, InputBool);
		e.expanded = true;
		e.add(cv);
		cv.set_expander(e);
		cv.set_label_size_group(_label_size_group);

		Gtk.ListBoxRow row = new Gtk.ListBoxRow();
#if CROWN_GTK3
		row.can_focus = false;
		row.add(e);

		_list_box.add(row);
#else
		row.focusable = false;
		row.set_child(e);

		_list_box.append(row);
#endif

		return e;
	}
}

} /* namespace Crown */
