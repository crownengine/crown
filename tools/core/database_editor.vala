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
}
}
