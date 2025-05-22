/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class PropertyGrid : Gtk.Grid
{
	public Database? _db;
	public Guid _id;
	public Guid _component_id;
	public int _rows;

	public ProjectStore _store;
	public Gee.HashMap<string, InputField> _widgets;
	public Gee.HashMap<InputField, PropertyDefinition?> _definitions;

	public PropertyGrid(Database? db = null, ProjectStore? store = null)
	{
		this.row_spacing = 4;
		this.row_homogeneous = true;
		this.column_spacing = 12;

		// Data
		_db = db;
		_id = GUID_ZERO;
		_component_id = GUID_ZERO;
		_rows = 0;
		_store = store;
		_widgets = new Gee.HashMap<string, InputField>();
		_definitions = new Gee.HashMap<InputField, PropertyDefinition?>();
	}

	public Gtk.Widget add_row(string label, Gtk.Widget w)
	{
		Gtk.Label l = new Gtk.Label(label);
		l.width_chars = 13;
		l.set_alignment(1.0f, 0.5f);

		w.hexpand = true;

		this.attach(l, 0, (int)_rows);
		this.attach(w, 1, (int)_rows);
		++_rows;

		return l;
	}

	public void add_object_type(PropertyDefinition[] properties)
	{
		foreach (PropertyDefinition def in properties) {
			InputField? p = null;

			switch (def.type) {
			case PropertyType.BOOL:
				if (def.deffault == null)
					def.deffault = false;

				assert(def.deffault.holds(typeof(bool)));

				p = new InputBool();
				break;
			case PropertyType.DOUBLE:
				if (def.deffault == null)
					def.deffault = 0.0;
				if (def.min == null)
					def.min = double.MIN;
				if (def.max == null)
					def.max = double.MAX;

				assert(def.deffault.holds(typeof(double)));
				assert(def.min.holds(typeof(double)));
				assert(def.max.holds(typeof(double)));

				if (def.editor == PropertyEditorType.ANGLE)
					p = new InputAngle((double)def.deffault, (double)def.min, (double)def.max);
				else
					p = new InputDouble((double)def.deffault, (double)def.min, (double)def.max);
				break;
			case PropertyType.STRING:
				if (def.deffault == null) {
					if (def.enum_values.length > 0)
						def.deffault = def.enum_values[0];
					else
						def.deffault = "";
				}

				assert(def.deffault.holds(typeof(string)));

				if (def.editor == PropertyEditorType.ENUM)
					p = new InputEnum((string)def.deffault, def.enum_labels, def.enum_values);
				else if (def.editor == PropertyEditorType.RESOURCE)
					p = new InputResource(_store, def.resource_type);
				else
					p = new InputString();
				break;
			case PropertyType.GUID:
				p = new InputString();
				break;
			case PropertyType.VECTOR3:
				if (def.deffault == null)
					def.deffault = VECTOR3_ZERO;
				if (def.min == null)
					def.min = VECTOR3_MIN;
				if (def.max == null)
					def.max = VECTOR3_MAX;

				assert(def.deffault.holds(typeof(Vector3)));
				assert(def.min.holds(typeof(Vector3)));
				assert(def.max.holds(typeof(Vector3)));

				if (def.editor == PropertyEditorType.COLOR)
					p = new InputColor3();
				else
					p = new InputVector3((Vector3)def.deffault, (Vector3)def.min, (Vector3)def.max);
				break;
			case PropertyType.QUATERNION:
				if (def.deffault == null)
					def.deffault = QUATERNION_IDENTITY;

				assert(def.deffault.holds(typeof(Quaternion)));

				p = new InputQuaternion();
				break;
			default:
			case PropertyType.NULL:
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

	public void on_property_value_changed(InputField p)
	{
		if (p.is_inconsistent())
			return;

		PropertyDefinition def = _definitions[p];
		bool changed = false;

		if (def.type == PropertyType.BOOL) {
			if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
				Unit u = Unit(_db, _id);
				if (u.get_component_property_bool(_component_id, def.name, (bool)def.deffault) != p.union_value()) {
					u.set_component_property_bool(_component_id, def.name, (bool)p.union_value());
					changed = true;
				}
			} else {
				if (_db.get_property_bool(_id, def.name, (bool)def.deffault) != p.union_value()) {
					_db.set_property_bool(_id, def.name, (bool)p.union_value());
					changed = true;
				}
			}
		} else if (def.type == PropertyType.DOUBLE) {
			if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
				Unit u = Unit(_db, _id);
				if (u.get_component_property_double(_component_id, def.name, (double)def.deffault) != p.union_value()) {
					u.set_component_property_double(_component_id, def.name, (double)p.union_value());
					changed = true;
				}
			} else {
				if (_db.get_property_double(_id, def.name, (double)def.deffault) != p.union_value()) {
					_db.set_property_double(_id, def.name, (double)p.union_value());
					changed = true;
				}
			}
		} else if (def.type == PropertyType.STRING) {
			if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
				Unit u = Unit(_db, _id);
				if (u.get_component_property_string(_component_id, def.name, (string)def.deffault) != (string)p.union_value()) {
					u.set_component_property_string(_component_id, def.name, (string)p.union_value());
					changed = true;
				}
			} else {
				if (_db.get_property_string(_id, def.name, (string)def.deffault) != (string)p.union_value()) {
					_db.set_property_string(_id, def.name, (string)p.union_value());
					changed = true;
				}
			}
		} else if (def.type == PropertyType.GUID) {
			if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
				Unit u = Unit(_db, _id);
				if (Guid.equal_func(u.get_component_property_guid(_component_id, def.name, (Guid)def.deffault), Guid.parse((string)p.union_value())) == false) {
					u.set_component_property_guid(_component_id, def.name, Guid.parse((string)p.union_value()));
					changed = true;
				}
			} else {
				if (Guid.equal_func(_db.get_property_guid(_id, def.name, (Guid)def.deffault), Guid.parse((string)p.union_value())) == false) {
					_db.set_property_guid(_id, def.name, Guid.parse((string)p.union_value()));
					changed = true;
				}
			}
		} else if (def.type == PropertyType.VECTOR3) {
			if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
				Unit u = Unit(_db, _id);
				if (Vector3.equal_func(u.get_component_property_vector3(_component_id, def.name, (Vector3)def.deffault), (Vector3)p.union_value()) == false) {
					u.set_component_property_vector3(_component_id, def.name, (Vector3)p.union_value());
					changed = true;
				}
			} else {
				if (Vector3.equal_func(_db.get_property_vector3(_id, def.name, (Vector3)def.deffault), (Vector3)p.union_value()) == false) {
					_db.set_property_vector3(_id, def.name, (Vector3)p.union_value());
					changed = true;
				}
			}
		} else if (def.type == PropertyType.QUATERNION) {
			if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
				Unit u = Unit(_db, _id);
				if (Quaternion.equal_func(u.get_component_property_quaternion(_component_id, def.name, (Quaternion)def.deffault), (Quaternion)p.union_value()) == false) {
					u.set_component_property_quaternion(_component_id, def.name, (Quaternion)p.union_value());
					changed = true;
				}
			} else {
				if (Quaternion.equal_func(_db.get_property_quaternion(_id, def.name, (Quaternion)def.deffault), (Quaternion)p.union_value()) == false) {
					_db.set_property_quaternion(_id, def.name, (Quaternion)p.union_value());
					changed = true;
				}
			}
		} else {
			loge("Unknown property type");
		}

		foreach (var e in _definitions) {
			PropertyDefinition other_def = e.value;

			if (other_def.enum_property == def.name) {
				if (other_def.enum_callback != null)
					other_def.enum_callback(p, (InputEnum)_widgets[other_def.name], _store._project);
				if (other_def.resource_callback != null)
					other_def.resource_callback(p, (InputResource)_widgets[other_def.name], _store._project);
			}
		}

		if (changed) {
			if (_db.object_type(_id) == OBJECT_TYPE_UNIT)
				_db.add_restore_point(ActionType.CHANGE_OBJECTS, new Guid?[] { _id, _component_id });
			else
				_db.add_restore_point(ActionType.CHANGE_OBJECTS, new Guid?[] { _id });
		}
	}

	public virtual void update()
	{
		foreach (var e in _definitions) {
			InputField p = e.key;
			PropertyDefinition def = e.value;

			if (def.type == PropertyType.BOOL) {
				if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
					Unit u = Unit(_db, _id);
					p.set_union_value(u.get_component_property_bool(_component_id, def.name, (bool)def.deffault));
				} else {
					p.set_union_value(_db.get_property_bool(_id, def.name, (bool)def.deffault));
				}
			} else if (def.type == PropertyType.DOUBLE) {
				if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
					Unit u = Unit(_db, _id);
					p.set_union_value(u.get_component_property_double(_component_id, def.name, (double)def.deffault));
				} else {
					p.set_union_value(_db.get_property_double(_id, def.name, (double)def.deffault));
				}
			} else if (def.type == PropertyType.STRING) {
				if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
					Unit u = Unit(_db, _id);
					p.set_union_value(u.get_component_property_string(_component_id, def.name, (string)def.deffault));
				} else {
					p.set_union_value(_db.get_property_string(_id, def.name, (string)def.deffault));
				}
			} else if (def.type == PropertyType.GUID) {
				if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
					Unit u = Unit(_db, _id);
					p.set_union_value(u.get_component_property_guid(_component_id, def.name, (Guid)def.deffault));
				} else {
					p.set_union_value(_db.get_property_guid(_id, def.name, (Guid)def.deffault));
				}
			} else if (def.type == PropertyType.VECTOR3) {
				if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
					Unit u = Unit(_db, _id);
					p.set_union_value(u.get_component_property_vector3(_component_id, def.name, (Vector3)def.deffault));
				} else {
					p.set_union_value(_db.get_property_vector3(_id, def.name, (Vector3)def.deffault));
				}
			} else if (def.type == PropertyType.QUATERNION) {
				if (_db.object_type(_id) == OBJECT_TYPE_UNIT) {
					Unit u = Unit(_db, _id);
					p.set_union_value(u.get_component_property_quaternion(_component_id, def.name, (Quaternion)def.deffault));
				} else {
					p.set_union_value(_db.get_property_quaternion(_id, def.name, (Quaternion)def.deffault));
				}
			} else {
				loge("Unknown property value type");
			}
		}
	}
}

public class PropertyGridSet : Gtk.Box
{
	public PropertyGridSet()
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);
	}

	public Expander add_property_grid(PropertyGrid cv, string label)
	{
		Gtk.Label l = new Gtk.Label(null);
		l.set_markup("<b>%s</b>".printf(label));
		l.set_alignment(0.0f, 0.5f);

		Expander e = new Expander();
		e.custom_header = l;
		e.expanded = true;
		e.add(cv);
		this.pack_start(e, false, true, 0);

		return e;
	}

	public Expander add_property_grid_optional(PropertyGrid cv, string label, InputBool InputBool)
	{
		Gtk.Label l = new Gtk.Label(null);
		l.set_markup("<b>%s</b>".printf(label));
		l.set_alignment(0.0f, 0.5f);

		Gtk.Box b = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
		b.pack_start(InputBool, false, false);
		b.pack_start(l, false, false);

		Expander e = new Expander();
		e.custom_header = b;
		e.expanded = true;
		e.add(cv);
		this.pack_start(e, false, true, 0);

		return e;
	}
}

} /* namespace Crown */
