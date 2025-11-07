/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputObject : InputEnum
{
	public StringId64 _type;
	public Database _database;

	public override GLib.Value union_value()
	{
		return this.value;
	}

	public override void set_union_value(GLib.Value v)
	{
		this.value = (Guid)v;
	}

	public Guid value
	{
		get
		{
			return Guid.parse(this.get_active_id());
		}
		set
		{
			_filter.refilter();
			bool success = this.set_active_id(value.to_string());
			set_inconsistent(!success);
		}
	}

	public bool filter_visible_func(Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value id_val;
		model.get_value(iter, 0, out id_val);

		if (!_inconsistent && (string)id_val == INCONSISTENT_ID)
			return false;

		// TODO: filter based on type.
		return true;
	}

	public string object_name(Guid id, ref int num_unnamed)
	{
		string name = _database.object_name(id);

		if (name == OBJECT_NAME_UNNAMED) {
			if (num_unnamed > 0) {
				return "%s (%d)".printf(OBJECT_NAME_UNNAMED, num_unnamed++);
			} else {
				++num_unnamed;
				return name;
			}
		}

		return name;
	}

	public InputObject(StringId64 type, Database database)
	{
		_type = type;
		_database = database;

		int num_unnamed = 0;
		Gee.HashMap<Guid?, Gee.HashMap<string, Value?>> all_objects = database._data;
		foreach (var o in all_objects) {
			if (StringId64(database.object_type(o.key)) == type)
				append(o.key.to_string(), object_name(o.key, ref num_unnamed));
		}
	}
}

} /* namespace Crown */
