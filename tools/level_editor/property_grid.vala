/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class PropertyGrid : Gtk.Grid
{
	GLib.ActionEntry[] actions =
	{
		{ "remove", on_remove, null, null },
	};

	public Expander? _expander;
	public Gtk.GestureMultiPress _controller_click;
	public GLib.SimpleActionGroup _action_group;
	public Database? _db;
	public StringId64 _type;
	public Guid _id;
	public Guid _component_id;
	public int _rows;
	public double _order;
	public bool _visible;

	public Gee.HashMap<string, InputField> _widgets;
	public Gee.HashMap<InputField, PropertyDefinition?> _definitions;

	public void on_remove(GLib.SimpleAction action, GLib.Variant? param)
	{
		string component_type = _db.type_name(_type);
		Guid unit_id = _id;
		Unit unit = Unit(_db, unit_id);

		Guid component_id;
		if (!unit.has_component(out component_id, component_type))
			return;

		Gee.ArrayList<unowned string> dependents = new Gee.ArrayList<unowned string>();
		// Do not remove if any other component needs us.
		foreach (var entry in Unit._component_registry.entries) {
			Guid dummy;
			if (!unit.has_component(out dummy, entry.key))
				continue;

			string[] component_type_dependencies = ((string)entry.value).split(", ");
			if (component_type in component_type_dependencies)
				dependents.add(entry.key);
		}

		if (dependents.size > 0) {
			StringBuilder sb = new StringBuilder();
			sb.append("Cannot remove %s due to the following dependencies:\n\n".printf(component_type));
			foreach (var item in dependents)
				sb.append("• %s\n".printf(item));

			Gtk.MessageDialog md = new Gtk.MessageDialog(null
				, Gtk.DialogFlags.MODAL
				, Gtk.MessageType.WARNING
				, Gtk.ButtonsType.OK
				, sb.str
				);
			md.set_default_response(Gtk.ResponseType.OK);

			md.response.connect(() => { md.destroy(); });
			md.show_all();
			return;
		} else {
			unit.remove_component_type(component_type);
		}
	}

	public void on_expander_button_released(int n_press, double x, double y)
	{
		if (_controller_click.get_current_button() == Gdk.BUTTON_SECONDARY) {
			GLib.Menu menu = new GLib.Menu();
			GLib.MenuItem mi;

			if (_db != null && _db.object_type(_id) == OBJECT_TYPE_UNIT && _component_id != GUID_ZERO) {
				mi = new GLib.MenuItem("Remove Component", null);
				mi.set_action_and_target_value("object.remove", null);
				menu.append_item(mi);
			}

			if (menu.get_n_items() > 0) {
				Gtk.Popover popover = new Gtk.Popover.from_model(null, menu);
				popover.set_relative_to(this);
				popover.set_pointing_to({ (int)x, (int)y, 1, 1 });
				popover.set_position(Gtk.PositionType.BOTTOM);
				popover.popup();
			}
		}
	}

	public PropertyGrid(Database? db = null)
	{
		this.row_spacing = 4;
		this.row_homogeneous = true;
		this.column_spacing = 12;

		// Data
		_expander = null;
		_db = db;
		_id = GUID_ZERO;
		_component_id = GUID_ZERO;
		_rows = 0;
		_order = 0.0;
		_visible = true;

		_widgets = new Gee.HashMap<string, InputField>();
		_definitions = new Gee.HashMap<InputField, PropertyDefinition?>();

		_action_group = new GLib.SimpleActionGroup();
		_action_group.add_action_entries(actions, this);
		this.insert_action_group("object", _action_group);
	}

	public PropertyGrid.from_object_type(StringId64 type, Database db)
	{
		this(db);

		_order = db.type_info(type).ui_order;
		_type = type;
		add_object_type(db.object_definition(type));
	}

	public PropertyGrid.from_object(Guid id, Database db)
	{
		this.from_object_type(StringId64(db.object_type(id)), db);
		_id = id;
	}

	public void set_expander(Expander e)
	{
		assert(_expander == null);

		_expander = e;

		_controller_click = new Gtk.GestureMultiPress(e);
		_controller_click.set_button(0);
		_controller_click.released.connect(on_expander_button_released);
	}

	public Gtk.Widget add_row(string label, Gtk.Widget w)
	{
		Gtk.Label l = new Gtk.Label(label);
		l.width_chars = 13;
		l.xalign = 1.0f;
		l.yalign = 0.5f;

		w.hexpand = true;

		this.attach(l, 0, (int)_rows);
		this.attach(w, 1, (int)_rows);
		++_rows;

		return l;
	}

	public void add_object_type(PropertyDefinition[] properties)
	{
		foreach (PropertyDefinition def in properties) {
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
					p = new InputDouble((double)def.deffault, (double)def.min, (double)def.max);
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
				continue;
			default:
				assert(false);
				break;
			}

			p.value_changed.connect(on_property_value_changed);

			_widgets[def.name] = p;
			_definitions[p] = def;

			if (!def.hidden)
				add_row(def.label, p);
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

	public void on_property_value_changed(InputField p)
	{
		if (p.is_inconsistent())
			return;
		if (_id == GUID_ZERO)
			return;

		PropertyDefinition def = _definitions[p];
		Gee.ArrayList<PropertyDefinition?> dynamic_properties = new Gee.ArrayList<PropertyDefinition?>();
		Gee.ArrayList<GLib.Value?> dynamic_values = new Gee.ArrayList<GLib.Value?>();
		bool changed = false;

		save_dynamic_properties_values(ref dynamic_properties, ref dynamic_values);
		read_dynamic_properties_ranges_except({ def });
		changed = restore_dynamic_properties_values_except(dynamic_properties, dynamic_values, { def }) || changed;
		changed = write_property_if_changed(def, p.union_value()) || changed;

		if (changed)
			_db.add_restore_point(ActionType.CHANGE_OBJECTS, new Guid?[] { _id });
	}

	public void read_all_properties()
	{
		foreach (var e in _definitions) {
			InputField p = e.key;
			PropertyDefinition def = e.value;

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
	}

	public void read_dynamic_properties_ranges_except(PropertyDefinition[] excluded)
	{
		foreach (var e in _definitions) {
			PropertyDefinition def = e.value;
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
				InputField p = _widgets[def.name];
				InputField parent_p = _widgets[def.enum_property];

				p.value_changed.disconnect(on_property_value_changed);
				def.enum_callback(parent_p, (InputEnum)p, _db._project);
				p.value_changed.connect(on_property_value_changed);
			} else if (def.resource_callback != null) {
				InputField p = _widgets[def.name];
				InputField parent_p = _widgets[def.enum_property];

				p.value_changed.disconnect(on_property_value_changed);
				def.resource_callback(parent_p, (InputResource)p, _db._project);
				p.value_changed.connect(on_property_value_changed);
			}
		}
	}

	public void read_dynamic_properties_ranges()
	{
		read_dynamic_properties_ranges_except({});
	}

	public void save_dynamic_properties_values(ref Gee.ArrayList<PropertyDefinition?> properties, ref Gee.ArrayList<GLib.Value?> values)
	{
		foreach (var e in _definitions) {
			PropertyDefinition def = e.value;

			if (def.enum_callback != null) {
				InputField p = _widgets[def.name];

				properties.add(def);
				values.add(p.union_value());
			} else if (def.resource_callback != null) {
				InputField p = _widgets[def.name];

				properties.add(def);
				values.add(p.union_value());
			}
		}
	}

	public bool restore_dynamic_properties_values_except(Gee.ArrayList<PropertyDefinition?> properties, Gee.ArrayList<GLib.Value?> values, PropertyDefinition[] excluded)
	{
		bool changed = false;

		for (int i = 0; i < properties.size; ++i) {
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

	public bool restore_dynamic_properties_values(Gee.ArrayList<PropertyDefinition?> properties, Gee.ArrayList<GLib.Value?> values)
	{
		return restore_dynamic_properties_values_except(properties, values, {});
	}
}

public class PropertyGridSet : Gtk.Box
{
	public Gtk.ListBox _list_box;

	public PropertyGridSet()
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

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

		this.pack_start(_list_box);
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

	public Expander add_property_grid(PropertyGrid cv, string label)
	{
		Gtk.Label l = new Gtk.Label(null);
		l.set_markup("<b>%s</b>".printf(label));
		l.xalign = 0.0f;
		l.yalign = 0.5f;

		Expander e = new Expander();
		e.custom_header = l;
		e.expanded = true;
		e.add(cv);
		cv.set_expander(e);

		Gtk.ListBoxRow row = new Gtk.ListBoxRow();
		row.can_focus = false;
		row.add(e);

		_list_box.add(row);

		return e;
	}

	public Expander add_property_grid_optional(PropertyGrid cv, string label, InputBool InputBool)
	{
		Gtk.Label l = new Gtk.Label(null);
		l.set_markup("<b>%s</b>".printf(label));
		l.xalign = 0.0f;
		l.yalign = 0.5f;

		Gtk.Box b = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
		b.pack_start(InputBool, false, false);
		b.pack_start(l, false, false);

		Expander e = new Expander();
		e.custom_header = b;
		e.expanded = true;
		e.add(cv);
		cv.set_expander(e);

		Gtk.ListBoxRow row = new Gtk.ListBoxRow();
		row.can_focus = false;
		row.add(e);

		_list_box.add(row);

		return e;
	}
}

} /* namespace Crown */
