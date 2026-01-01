/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
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

	public new Guid value
	{
		get
		{
			Guid id;
			string active_id = this.get_active_id();

			if (active_id == null)
				return GUID_ZERO;
			return Guid.try_parse(active_id, out id) ? id : GUID_ZERO;
		}
		set
		{
			_filter.refilter();
			bool success = this.set_active_id(value.to_string());
			set_inconsistent(!success);
		}
	}

	public new bool filter_visible_func(Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value id_val;
		model.get_value(iter, 0, out id_val);

		if (!_inconsistent && (string)id_val == INCONSISTENT_ID)
			return false;

		// TODO: filter based on type.
		return true;
	}

	public InputObject(StringId64 type, Database database)
	{
		_type = type;
		_database = database;
		_database.objects_created.connect(on_objects_created);
		_database.objects_destroyed.connect(on_objects_destroyed);
		_database.objects_changed.connect(on_objects_changed);

		append_objects();
	}

	public void append_objects()
	{
		Guid?[] all_of_type = _database.all_objects_of_type(_type);
		Guid previous_value = this.value;

		clear();

		foreach (Guid? id in all_of_type) {
			StringId64 object_type = StringId64(_database.object_type(id));
			Aspect? name_aspect = _database.get_aspect(object_type, StringId64("name"));
			if (name_aspect == null)
				name_aspect = default_name_aspect;

			string object_name;
			name_aspect(out object_name, _database, id);

			append(id.to_string(), object_name);
		}

		this.value = previous_value;
	}

	public void on_objects_created(Guid?[] object_ids, uint32 flags)
	{
		append_objects();
	}

	public void on_objects_destroyed(Guid?[] object_ids, uint32 flags)
	{
		append_objects();
	}

	public void on_objects_changed(Guid?[] object_ids, uint32 flags)
	{
		append_objects();
	}
}

} /* namespace Crown */
