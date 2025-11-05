/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class DatabaseEditor
{
	public const GLib.ActionEntry[] actions =
	{
		{ "undo",   on_undo,   null,   null },
		{ "redo",   on_redo,   null,   null },
		{ "delete", on_delete, "(ss)", null },
		{ "rename", on_rename, "(ss)", null },
	};

	public UndoRedo _undo_redo;
	public Database _database;
	public GLib.SimpleActionGroup _action_group;

	public signal void undo(int action_id);
	public signal void redo(int action_id);

	public DatabaseEditor(Project project, uint32 undo_redo_size)
	{
		_action_group = new GLib.SimpleActionGroup();
		_action_group.add_action_entries(actions, this);

		_undo_redo = new UndoRedo(undo_redo_size);
		_database = new Database(project, _undo_redo);
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

	public void on_delete(GLib.SimpleAction action, GLib.Variant? param)
	{
		Guid object_id = Guid.parse((string)param.get_child_value(0));
		string set_name = (string)param.get_child_value(1);

		_database.remove_from_set(_database.object_owner(object_id), set_name, object_id);
		_database.destroy(object_id);
		_database.add_restore_point((int)ActionType.DESTROY_OBJECTS, { object_id });
	}

	public void do_rename(Guid object_id, string new_name)
	{
		if (new_name != "" && _database.object_name(object_id) != new_name) {
			_database.set_object_name(object_id, new_name);
			_database.add_restore_point((int)ActionType.CHANGE_OBJECTS, new Guid?[] { object_id });
		}
	}

	public void on_rename(GLib.SimpleAction action, GLib.Variant? param)
	{
		Guid object_id = Guid.parse((string)param.get_child_value(0));
		string new_name = (string)param.get_child_value(1);

		if (new_name != "") {
			do_rename(object_id, new_name);
		} else {
			Gtk.Dialog dg = new Gtk.Dialog.with_buttons("New Name"
				, null
				, Gtk.DialogFlags.MODAL
				, "Cancel"
				, Gtk.ResponseType.CANCEL
				, "Ok"
				, Gtk.ResponseType.OK
				, null
				);

			InputString sb = new InputString();
			sb.activate.connect(() => { dg.response(Gtk.ResponseType.OK); });
			sb.value = _database.object_name(object_id);

			dg.get_content_area().add(sb);
			dg.response.connect((response_id) => {
					if (response_id == Gtk.ResponseType.OK)
						do_rename(object_id, sb.text.strip());
					dg.destroy();
				});
			dg.show_all();
		}
	}
}

} /* namespace Crown */
