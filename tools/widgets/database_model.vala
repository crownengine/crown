/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class DatabaseModel : GLib.Object, Gtk.TreeModel
{
	public enum Column
	{
		NAME,
		GUID,
		TYPE,

		COUNT
	}

	public Database _db;
	public Gee.ArrayList<Guid?> _objects;
	public int _stamp;

	public DatabaseModel(Database db)
	{
		assert(db != null);

		_db = db;
		_objects = new Gee.ArrayList<Guid?>(Guid.equal_func);
		_stamp = (int)GLib.Random.next_int() | 1;

		populate();

		_db.objects_created.connect(on_objects_created);
		_db.objects_destroyed.connect(on_objects_destroyed);
		_db.objects_changed.connect(on_objects_changed);
	}

	public void populate()
	{
		_objects.clear();
		_objects.add_all_array(_db.all_objects_of_type(OBJECT_TYPE_ANY));
	}

	public Gtk.TreeModelFlags get_flags()
	{
		return Gtk.TreeModelFlags.LIST_ONLY;
	}

	public int get_n_columns()
	{
		return Column.COUNT;
	}

	public GLib.Type get_column_type(int index)
	{
		switch (index) {
		case Column.NAME: return typeof(string);
		case Column.GUID: return typeof(Guid);
		case Column.TYPE: return typeof(StringId64);
		default: assert_not_reached();
		}
	}

	public bool get_iter(out Gtk.TreeIter iter, Gtk.TreePath path)
	{
		iter = Gtk.TreeIter();

		if (path.get_depth() != 1)
			return false;

		unowned int[] indices = path.get_indices();
		int idx = indices[0];

		if (idx < 0 || idx >= _objects.size)
			return false;

		iter.stamp = _stamp;
		iter.user_data = (void*)((ulong)idx);
		return true;
	}

	public Gtk.TreePath? get_path(Gtk.TreeIter iter)
	{
		if (iter.stamp != _stamp)
			return null;

		int idx = (int)((ulong)iter.user_data);
		if (idx < 0 || idx >= _objects.size)
			return null;

		var path = new Gtk.TreePath();
		path.append_index(idx);
		return path;
	}

	public void get_value(Gtk.TreeIter iter, int column, out Value value)
	{
		value = Value(typeof(GLib.Value));

		if (iter.stamp != _stamp)
			return;

		int idx = (int)((ulong)iter.user_data);
		if (idx < 0 || idx >= _objects.size)
			return;

		Guid id = _objects[idx];
		assert(_db.is_alive(id));

		switch (column) {
		case Column.NAME:
			Aspect? name_aspect = _db.get_aspect(StringId64(_db.object_type(id)), StringId64("name"));
			if (name_aspect == null)
				name_aspect = default_name_aspect;
			string object_name;
			name_aspect(out object_name, _db, id);
			value = object_name;
			break;

		case Column.GUID:
			value = id;
			break;

		case Column.TYPE:
			value = StringId64(_db.object_type(id));
			break;

		default:
			break;
		}
	}

	public bool iter_next(ref Gtk.TreeIter iter)
	{
		if (iter.stamp != _stamp) {
			iter = Gtk.TreeIter();
			return false;
		}

		int idx = (int)((ulong)iter.user_data);
		if (idx + 1 >= _objects.size) {
			iter = Gtk.TreeIter();
			return false;
		}

		iter.user_data = (void*)((ulong)(idx + 1));
		return true;
	}

	public bool iter_previous(ref Gtk.TreeIter iter)
	{
		if (iter.stamp != _stamp) {
			iter = Gtk.TreeIter();
			return false;
		}

		int idx = (int)((ulong)iter.user_data);
		if (idx - 1 < 0) {
			iter = Gtk.TreeIter();
			return false;
		}

		iter.user_data = (void*)((ulong)(idx - 1));
		return true;
	}

	public bool iter_children(out Gtk.TreeIter iter, Gtk.TreeIter? parent)
	{
		iter = Gtk.TreeIter();
		if (parent != null)
			return false;

		if (_objects.size == 0)
			return false;

		iter.stamp = _stamp;
		iter.user_data = (void*)0UL;
		return true;
	}

	public bool iter_has_child(Gtk.TreeIter iter)
	{
		return false;
	}

	public int iter_n_children(Gtk.TreeIter? iter)
	{
		return (iter == null) ? _objects.size : 0;
	}

	public bool iter_nth_child(out Gtk.TreeIter iter, Gtk.TreeIter? parent, int n)
	{
		iter = Gtk.TreeIter();
		if (parent != null)
			return false;

		if (n < 0 || n >= _objects.size)
			return false;

		iter.stamp = _stamp;
		iter.user_data = (void*)((ulong)n);
		return true;
	}

	public bool iter_parent(out Gtk.TreeIter iter, Gtk.TreeIter child)
	{
		iter = Gtk.TreeIter();
		return false;
	}

	public void ref_node(Gtk.TreeIter iter)
	{
	}

	public void unref_node(Gtk.TreeIter iter)
	{
	}

	public void append_new_alive_objects()
	{
		int old_size = _objects.size;
		Guid?[] ids = _db.all_objects_of_type(OBJECT_TYPE_ANY);

		foreach (var id in ids) {
			if (id != null && !_objects.contains(id))
				_objects.add(id);
		}

		if (old_size == _objects.size)
			return;

		for (int i = old_size; i < _objects.size; i++) {
			Gtk.TreePath path = new Gtk.TreePath.from_indices(i);
			Gtk.TreeIter iter = Gtk.TreeIter();
			iter.stamp = _stamp;
			iter.user_data = (void*)((ulong)i);
			row_inserted(path, iter);
		}
	}

	public void remove_dead_objects()
	{
		var indices = new Gee.ArrayList<int>();
		var alive = new Gee.HashSet<Guid?>(Guid.hash_func, Guid.equal_func);
		foreach (var id in _db.all_objects_of_type(OBJECT_TYPE_ANY))
			alive.add(id);

		for (int i = 0; i < _objects.size; ++i) {
			if (!alive.contains(_objects[i]))
				indices.add(i);
		}

		indices.sort((a, b) => b - a);

		foreach (int idx in indices) {
			_objects.remove_at(idx);
			Gtk.TreePath path = new Gtk.TreePath.from_indices(idx);
			row_deleted(path);
		}
	}

	public void on_objects_created(Guid?[] ids, uint32 flags)
	{
		append_new_alive_objects();
	}

	public void on_objects_destroyed(Guid?[] ids, uint32 flags)
	{
		remove_dead_objects();
	}

	public void on_objects_changed(Guid?[] ids, uint32 flags)
	{
		foreach (var id in ids) {
			if (id == null)
				continue;

			int idx = _objects.index_of(id);
			if (idx >= 0) {
				Gtk.TreePath path = new Gtk.TreePath.from_indices(idx);
				Gtk.TreeIter iter = Gtk.TreeIter();
				iter.stamp = _stamp;
				iter.user_data = (void*)((ulong)idx);
				row_changed(path, iter);
			}
		}
	}
}

} /* namespace Crown */
