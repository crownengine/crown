/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;
using Gee;

namespace Crown
{
public class ProjectStore
{
	public const string ROOT_FOLDER = "";

	public enum Column
	{
		NAME,
		TYPE,
		SIZE,
		MTIME,

		COUNT
	}

	// Data
	public Project _project;
	public Gtk.ListStore _list_store;
	public Gtk.TreeStore _tree_store;
	public Gee.HashMap<string, Gtk.TreeRowReference> _folders;
	public Gtk.TreeRowReference _favorites_root;

	public ProjectStore(Project project)
	{
		// Data
		_project = project;
		_project.file_added.connect(on_project_file_added);
		_project.file_changed.connect(on_project_file_changed);
		_project.file_removed.connect(on_project_file_removed);
		_project.tree_added.connect(on_project_tree_added);
		_project.tree_removed.connect(on_project_tree_removed);
		_project.project_reset.connect(on_project_reset);
		_project.project_loaded.connect(on_project_loaded);

		_list_store = new Gtk.ListStore(Column.COUNT
			, typeof(string) // resource name
			, typeof(string) // resource type
			, typeof(uint64) // resource size
			, typeof(uint64) // resource mtime
			);
		_tree_store = new Gtk.TreeStore(Column.COUNT
			, typeof(string) // resource name
			, typeof(string) // resource type
			, typeof(uint64) // resource size
			, typeof(uint64) // resource mtime
			);

		_folders = new Gee.HashMap<string, Gtk.TreeRowReference>();

		reset();
	}

	public void reset()
	{
		_folders.clear();
		_tree_store.clear();
		_list_store.clear();

		// Add favorites root.
		Gtk.TreeIter iter;
		_tree_store.insert_with_values(out iter
			, null
			, -1
			, Column.NAME
			, ROOT_FOLDER
			, Column.TYPE
			, "<favorites>"
			, Column.SIZE
			, 0u
			, Column.MTIME
			, 0u
			, -1
			);
		_favorites_root = new Gtk.TreeRowReference(_tree_store, _tree_store.get_path(iter));
	}

	public bool path_for_resource_type_name(out Gtk.TreePath path, string type, string name)
	{
		if (type == "<folder>") {
			path = _folders[name].get_path();
			return true;
		}

		string parent_folder = ResourceId.parent_folder(name);

		if (_folders.has_key(parent_folder)) {
			// Find the name inside the folder.
			Gtk.TreeIter parent_iter;
			_tree_store.get_iter(out parent_iter, _folders[parent_folder].get_path());

			Gtk.TreeIter child;
			if (_tree_store.iter_children(out child, parent_iter)) {
				Value iter_name;
				Value iter_type;

				while (true) {
					_tree_store.get_value(child, Column.NAME, out iter_name);
					_tree_store.get_value(child, Column.TYPE, out iter_type);
					if ((string)iter_name == name && (string)iter_type == type) {
						path = _tree_store.get_path(child);
						return true;
					}

					if (!_tree_store.iter_next(ref child))
						break;
				}
			}
		}

		path = _folders[ROOT_FOLDER].get_path();
		return false;
	}

	public Gtk.TreePath? project_root_path()
	{
		if (!_folders.has_key(ROOT_FOLDER))
			return null;

		return _folders[ROOT_FOLDER].get_path();
	}

	public Gtk.TreePath? favorites_root_path()
	{
		return _favorites_root.get_path();
	}

	public void add_to_favorites(string type, string name)
	{
		Gtk.TreeIter favorites_root_iter;
		_tree_store.get_iter(out favorites_root_iter, favorites_root_path());

		// Avoid duplicates.
		Gtk.TreeIter child;
		if (_tree_store.iter_children(out child, favorites_root_iter)) {
			Value iter_name;
			Value iter_type;

			while (true) {
				_tree_store.get_value(child, Column.NAME, out iter_name);
				_tree_store.get_value(child, Column.TYPE, out iter_type);
				if ((string)iter_name == name && (string)iter_type == type)
					return;

				if (!_tree_store.iter_next(ref child))
					break;
			}
		}

		// Add to favorites.
		Gtk.TreeIter iter;
		_tree_store.insert_with_values(out iter
			, favorites_root_iter
			, -1
			, Column.NAME
			, name
			, Column.TYPE
			, type
			, Column.SIZE
			, 0u
			, Column.MTIME
			, 0u
			, -1
			);
	}

	public void remove_from_favorites(string type, string name)
	{
		// Remove from tree store.
		Gtk.TreeIter parent_iter;
		_tree_store.get_iter(out parent_iter, favorites_root_path());
		Gtk.TreeIter child;
		if (_tree_store.iter_children(out child, parent_iter)) {
			Value iter_name;
			Value iter_type;

			while (true) {
				_tree_store.get_value(child, Column.NAME, out iter_name);
				_tree_store.get_value(child, Column.TYPE, out iter_type);
				if ((string)iter_name == name && (string)iter_type == type) {
					_tree_store.remove(ref child);
					break;
				}

				if (!_tree_store.iter_next(ref child))
					break;
			}
		}
	}

	private Gtk.TreeIter make_tree_internal(string folder, int start_index, Gtk.TreeRowReference parent)
	{
		// Folder can be:
		// "", root folder
		// "folder", one word
		// "folder/another_folder", any number of words concatenated by '/'
		int first_slash = folder.index_of_char('/', start_index);
		if (first_slash == -1) {
			Gtk.TreeIter parent_iter;
			_tree_store.get_iter(out parent_iter, parent.get_path());
			Gtk.TreeIter iter;
			_tree_store.insert_with_values(out iter
				, parent_iter
				, -1
				, Column.NAME
				, folder
				, Column.TYPE
				, "<folder>"
				, Column.SIZE
				, 0u
				, Column.MTIME
				, 0u
				, -1
				);

			_folders[folder] = new Gtk.TreeRowReference(_tree_store, _tree_store.get_path(iter));
			return iter;
		} else {
			if (_folders.has_key(folder.substring(0, first_slash))) {
				return make_tree_internal(folder, first_slash + 1, _folders[folder.substring(0, first_slash)]);
			} else {
				Gtk.TreeIter parent_iter;
				_tree_store.get_iter(out parent_iter, parent.get_path());
				Gtk.TreeIter iter;
				_tree_store.insert_with_values(out iter
					, parent_iter
					, -1
					, Column.NAME
					, folder.substring(0, first_slash)
					, Column.TYPE
					, "<folder>"
					, Column.SIZE
					, 0u
					, Column.MTIME
					, 0u
					, -1
					);

				Gtk.TreeRowReference trr = new Gtk.TreeRowReference(_tree_store, _tree_store.get_path(iter));
				_folders[folder.substring(0, first_slash)] = trr;

				return make_tree_internal(folder, first_slash + 1, trr);
			}
		}
	}

	private Gtk.TreeIter find_or_make_tree(string folder)
	{
		if (_folders.has_key(folder)) {
			Gtk.TreeIter iter;
			_tree_store.get_iter(out iter, _folders[folder].get_path());
			return iter;
		}

		return make_tree_internal(folder, 0, _folders[ROOT_FOLDER]);
	}

	private void on_project_file_added(string type, string name, uint64 size, uint64 mtime)
	{
		string parent_folder = ResourceId.parent_folder(name);

		Gtk.TreeIter parent = find_or_make_tree(parent_folder);
		Gtk.TreeIter iter;
		_list_store.insert_with_values(out iter
			, -1
			, Column.NAME
			, name
			, Column.TYPE
			, type
			, Column.SIZE
			, size
			, Column.MTIME
			, mtime
			, -1
			);
		_tree_store.insert_with_values(out iter
			, parent
			, -1
			, Column.NAME
			, name
			, Column.TYPE
			, type
			, Column.SIZE
			, size
			, Column.MTIME
			, mtime
			, -1
			);
	}

	private void on_project_file_changed(string type, string name, uint64 size, uint64 mtime)
	{
		on_project_file_removed(type, name);
		on_project_file_added(type, name, size, mtime);
	}

	private void on_project_file_removed(string type, string name)
	{
		string parent_folder = ResourceId.parent_folder(name);
		Gtk.TreeIter child;

		if (!_folders.has_key(parent_folder))
			return;

		remove_from_favorites(type, name);

		// Remove from list store.
		if (_list_store.iter_children(out child, null)) {
			Value iter_name;
			Value iter_type;

			while (true) {
				_list_store.get_value(child, Column.NAME, out iter_name);
				_list_store.get_value(child, Column.TYPE, out iter_type);
				if ((string)iter_name == name && (string)iter_type == type) {
					_list_store.remove(ref child);
					break;
				}

				if (!_list_store.iter_next(ref child))
					break;
			}
		}

		// Remove from tree store.
		Gtk.TreeIter parent_iter;
		_tree_store.get_iter(out parent_iter, _folders[parent_folder].get_path());
		if (_tree_store.iter_children(out child, parent_iter)) {
			Value iter_name;
			Value iter_type;

			while (true) {
				_tree_store.get_value(child, Column.NAME, out iter_name);
				_tree_store.get_value(child, Column.TYPE, out iter_type);
				if ((string)iter_name == name && (string)iter_type == type) {
					_tree_store.remove(ref child);
					break;
				}

				if (!_tree_store.iter_next(ref child))
					break;
			}
		}
	}

	private void on_project_tree_added(string name)
	{
		Gtk.TreeIter iter;
		_list_store.insert_with_values(out iter
			, -1
			, Column.NAME
			, name
			, Column.TYPE
			, "<folder>"
			, Column.SIZE
			, 0u
			, Column.MTIME
			, 0u
			, -1
			);

		find_or_make_tree(name);
	}

	private void on_project_tree_removed(string name)
	{
		if (name == ROOT_FOLDER)
			return;

		if (!_folders.has_key(name))
			return;

		remove_from_favorites("<folder>", name);

		// Remove from list store.
		Gtk.TreeIter child;
		if (_list_store.iter_children(out child, null)) {
			Value iter_name;
			Value iter_type;

			while (true) {
				_list_store.get_value(child, Column.NAME, out iter_name);
				_list_store.get_value(child, Column.TYPE, out iter_type);
				if ((string)iter_name == name && (string)iter_type == "<folder>") {
					_list_store.remove(ref child);
					break;
				}

				if (!_list_store.iter_next(ref child))
					break;
			}
		}

		// Remove the tree.
		Gtk.TreeIter iter;
		_tree_store.get_iter(out iter, _folders[name].get_path());
		_tree_store.remove(ref iter);

		// Remove any stale TreeRowRerefence
		var it = _folders.map_iterator();
		for (var has_next = it.next(); has_next; has_next = it.next()) {
			string ff = it.get_key();
			if (ff.has_prefix(name + "/"))
				it.unset();
		}
		_folders.unset(name);
	}

	private void on_project_reset()
	{
		reset();
	}

	private void on_project_loaded()
	{
		Gtk.TreeIter iter;
		_tree_store.insert_with_values(out iter
			, null
			, -1
			, Column.NAME
			, ROOT_FOLDER
			, Column.TYPE
			, "<folder>"
			, Column.SIZE
			, 0u
			, Column.MTIME
			, 0u
			, -1
			);

		_folders[ROOT_FOLDER] = new Gtk.TreeRowReference(_tree_store, _tree_store.get_path(iter));
	}

	public Gee.ArrayList<Value?> encode_favorites()
	{
		Gee.ArrayList<Value?> favorites = new Gee.ArrayList<Value?>();

		Gtk.TreeIter parent_iter;
		_tree_store.get_iter(out parent_iter, favorites_root_path());
		Gtk.TreeIter child;
		if (_tree_store.iter_children(out child, parent_iter)) {
			Value iter_name;
			Value iter_type;

			while (true) {
				_tree_store.get_value(child, Column.NAME, out iter_name);
				_tree_store.get_value(child, Column.TYPE, out iter_type);

				Hashtable resource = new Hashtable();
				resource["type"] = (string)iter_type;
				resource["name"] = (string)iter_name;
				favorites.add(resource);

				if (!_tree_store.iter_next(ref child))
					break;
			}
		}

		return favorites;
	}

	public Hashtable encode()
	{
		Hashtable h = new Hashtable();
		h["favorites"] = encode_favorites();
		return h;
	}

	public void decode_favorites(Gee.ArrayList<Value?> favorites)
	{
		foreach (var entry in favorites) {
			if (entry == null || !entry.holds(typeof(Hashtable)))
				continue;

			Hashtable resource = (Hashtable)entry;
			if (!resource.has_key("type") || !resource.has_key("name"))
				continue;

			Value type = resource["type"];
			if (!type.holds(typeof(string)))
				continue;
			Value name = resource["name"];
			if (!name.holds(typeof(string)))
				continue;

			add_to_favorites((string)type, (string)name);
		}
	}

	public void decode(Hashtable h)
	{
		if (h.has_key("favorites")) {
			Value favorites = h["favorites"];
			if (favorites.holds(typeof(Gee.ArrayList)))
				decode_favorites((Gee.ArrayList<Value?>)favorites);
		}
	}
}

} /* namespace Crown */
