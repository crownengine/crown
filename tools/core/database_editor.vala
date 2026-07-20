/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class DatabaseEditor
{
	public const GLib.ActionEntry[] actions =
	{
		{ "undo",        on_undo,        null,   null },
		{ "redo",        on_redo,        null,   null },
		{ "duplicate",   on_duplicate,   null,   null },
		{ "delete",      on_delete,      null,   null },
		{ "add",         on_add,         "(ss)", null },
		{ "select_none", on_select_none, null,   null },
	};

	public UndoRedo _undo_redo;
	public Database _database;
	public GLib.GenericArray<Guid?> _selection;
	public GLib.SimpleActionGroup _action_group;

	public signal void undo(int action_id);
	public signal void redo(int action_id);
	public signal void selection_changed();

	public DatabaseEditor(uint32 undo_redo_size, Database database)
	{
		_action_group = new GLib.SimpleActionGroup();
		_action_group.add_action_entries(actions, this);

		_undo_redo = new UndoRedo(undo_redo_size);
		_database = database;
		_database._undo_redo = _undo_redo;
		_selection = new GLib.GenericArray<Guid?>();
	}

	public void load_types()
	{
		create_object_types(_database);
	}

	public void on_undo(GLib.SimpleAction action, GLib.Variant? param)
	{
		int id = _database.undo();
		if (id != -1)
			undo(id);
	}

	public void on_redo(GLib.SimpleAction action, GLib.Variant? param)
	{
		int id = _database.redo();
		if (id != -1)
			redo(id);
	}

	public void on_duplicate(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_selection.length == 0)
			return;

		Guid?[] new_ids = new Guid?[_selection.length];
		for (int i = 0; i < new_ids.length; ++i)
			new_ids[i] = Guid.new_guid();

		_database.duplicate_and_add_to_set(_selection.data, new_ids);
		_database.add_restore_point((int)ActionType.CREATE_OBJECTS, new_ids);

		selection_set(new_ids);
	}

	public void on_delete(GLib.SimpleAction action, GLib.Variant? param)
	{
		if (_selection.length == 0)
			return;

		Guid?[] ids = new Guid?[_selection.length];
		for (int i = 0; i < _selection.length; ++i)
			ids[i] = _selection[i];
		foreach (Guid id in ids) {
			uint index;
			if (_selection.find_with_equal_func(id, Guid.equal_func, out index))
				_selection.remove_index(index);
			_database.destroy(id);
		}
		selection_changed();
		_database.add_restore_point((int)ActionType.DESTROY_OBJECTS, ids);
	}

	public void on_add(GLib.SimpleAction action, GLib.Variant? param)
	{
		Guid object_id = Guid.parse((string)param.get_child_value(0));
		string set_name = (string)param.get_child_value(1);

		StringId64 object_type = StringId64(_database.object_type(object_id));
		unowned PropertyDefinition[] properties = _database.object_definition(object_type);
		int i;
		for (i = 0; i < properties.length; ++i) {
			unowned PropertyDefinition p = properties[i];
			if (p.name == set_name)
				break;
		}

		if (i != properties.length) {
			string obj_type_name = _database.type_name(properties[i].object_type);
			Guid new_obj = Guid.new_guid();
			_database.create(new_obj, obj_type_name);
			_database.add_to_set(object_id, properties[i].name, new_obj);
			_database.add_restore_point((int)ActionType.CREATE_OBJECTS, { new_obj });
		}
	}

	public void on_select_none(GLib.SimpleAction action, GLib.Variant? param)
	{
		clear_selection();
	}

	public void clear_selection()
	{
		_selection.length = 0;
		selection_changed();
	}

	public void selection_read(Guid?[] ids)
	{
		_selection.length = 0;
		foreach (Guid? id in ids)
			_selection.add(id);
	}

	public void selection_set(Guid?[] ids)
	{
		selection_read(ids);
		selection_changed();
	}

	public void send_selection(RuntimeInstance runtime)
	{
		runtime.send_script(LevelEditorApi.selection_set(_selection.data));
	}
}

} /* namespace Crown */
