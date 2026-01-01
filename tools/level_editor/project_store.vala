/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class ProjectStore
{
	public const string ROOT_FOLDER = "";

	public enum Column
	{
		TYPE,
		NAME,
		SIZE,
		MTIME,
		VISIBLE,
		USER_DATA,

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
		_project.files_reset.connect(on_project_reset);
		_project.project_reset.connect(on_project_reset);

		_list_store = new Gtk.ListStore(Column.COUNT
			, typeof(string) // resource type
			, typeof(string) // resource name
			, typeof(uint64) // resource size
			, typeof(uint64) // resource mtime
			, typeof(bool)   // visible
			, typeof(uint32) // user data
			);
		_tree_store = new Gtk.TreeStore(Column.COUNT
			, typeof(string) // resource type
			, typeof(string) // resource name
			, typeof(uint64) // resource size
			, typeof(uint64) // resource mtime
			, typeof(bool)   // visible
			, typeof(uint32) // user data
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
			, Column.TYPE
			, "<favorites>"
			, Column.NAME
			, ROOT_FOLDER
			, Column.SIZE
			, 0u
			, Column.MTIME
			, 0u
			, Column.VISIBLE
			, true
			, -1
			);
		_favorites_root = new Gtk.TreeRowReference(_tree_store, _tree_store.get_path(iter));

		_tree_store.insert_with_values(out iter
			, null
			, -1
			, Column.TYPE
			, "<folder>"
			, Column.NAME
			, ROOT_FOLDER
			, Column.SIZE
			, 0u
			, Column.MTIME
			, 0u
			, Column.VISIBLE
			, true
			, -1
			);

		_folders[ROOT_FOLDER] = new Gtk.TreeRowReference(_tree_store, _tree_store.get_path(iter));
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
				Value iter_type;
				Value iter_name;

				while (true) {
					_tree_store.get_value(child, Column.TYPE, out iter_type);
					_tree_store.get_value(child, Column.NAME, out iter_name);
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
			Value iter_type;
			Value iter_name;

			while (true) {
				_tree_store.get_value(child, Column.TYPE, out iter_type);
				_tree_store.get_value(child, Column.NAME, out iter_name);
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
			, Column.TYPE
			, type
			, Column.NAME
			, name
			, Column.SIZE
			, 0u
			, Column.MTIME
			, 0u
			, Column.VISIBLE
			, true
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
			Value iter_type;
			Value iter_name;

			while (true) {
				_tree_store.get_value(child, Column.TYPE, out iter_type);
				_tree_store.get_value(child, Column.NAME, out iter_name);
				if ((string)iter_name == name && (string)iter_type == type) {
					_tree_store.remove(ref child);
					break;
				} else {
					if (!_tree_store.iter_next(ref child))
						break;
				}
			}
		}
	}

	public Gtk.TreeIter make_tree_internal(string folder, int start_index, Gtk.TreeRowReference parent)
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
				, Column.TYPE
				, "<folder>"
				, Column.NAME
				, folder
				, Column.SIZE
				, 0u
				, Column.MTIME
				, 0u
				, Column.VISIBLE
				, true
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
					, Column.TYPE
					, "<folder>"
					, Column.NAME
					, folder.substring(0, first_slash)
					, Column.SIZE
					, 0u
					, Column.MTIME
					, 0u
					, Column.VISIBLE
					, true
					, -1
					);

				Gtk.TreeRowReference trr = new Gtk.TreeRowReference(_tree_store, _tree_store.get_path(iter));
				_folders[folder.substring(0, first_slash)] = trr;

				return make_tree_internal(folder, first_slash + 1, trr);
			}
		}
	}

	public Gtk.TreeIter find_or_make_tree(string folder)
	{
		if (_folders.has_key(folder)) {
			Gtk.TreeIter iter;
			_tree_store.get_iter(out iter, _folders[folder].get_path());
			return iter;
		}

		return make_tree_internal(folder, 0, _folders[ROOT_FOLDER]);
	}

	public void on_project_file_added(string type, string name, uint64 size, uint64 mtime)
	{
		string parent_folder = ResourceId.parent_folder(name);

		Gtk.TreeIter parent = find_or_make_tree(parent_folder);
		Gtk.TreeIter iter;
		_list_store.insert_with_values(out iter
			, -1
			, Column.TYPE
			, type
			, Column.NAME
			, name
			, Column.SIZE
			, size
			, Column.MTIME
			, mtime
			, Column.VISIBLE
			, true
			, -1
			);
		_tree_store.insert_with_values(out iter
			, parent
			, -1
			, Column.TYPE
			, type
			, Column.NAME
			, name
			, Column.SIZE
			, size
			, Column.MTIME
			, mtime
			, Column.VISIBLE
			, true
			, -1
			);
	}

	public void on_project_file_changed(string type, string name, uint64 size, uint64 mtime)
	{
		string parent_folder = ResourceId.parent_folder(name);
		Gtk.TreeIter child;

		if (!_folders.has_key(parent_folder))
			return;

		// Update the list store entry.
		if (_list_store.iter_children(out child, null)) {
			Value iter_type;
			Value iter_name;

			while (true) {
				_list_store.get_value(child, Column.TYPE, out iter_type);
				_list_store.get_value(child, Column.NAME, out iter_name);
				if ((string)iter_name == name && (string)iter_type == type) {
					_list_store.set(child
						, Column.TYPE
						, type
						, Column.NAME
						, name
						, Column.SIZE
						, size
						, Column.MTIME
						, mtime
						, Column.VISIBLE
						, true
						, -1
						);
					break;
				}

				if (!_list_store.iter_next(ref child))
					break;
			}
		}

		// Update the tree store entry.
		Gtk.TreeIter parent_iter;
		_tree_store.get_iter(out parent_iter, _folders[parent_folder].get_path());
		if (_tree_store.iter_children(out child, parent_iter)) {
			Value iter_name;
			Value iter_type;

			while (true) {
				_tree_store.get_value(child, Column.TYPE, out iter_type);
				_tree_store.get_value(child, Column.NAME, out iter_name);
				if ((string)iter_name == name && (string)iter_type == type) {
					_tree_store.set(child
						, Column.TYPE
						, type
						, Column.NAME
						, name
						, Column.SIZE
						, size
						, Column.MTIME
						, mtime
						, Column.VISIBLE
						, true
						, -1
						);
					break;
				}

				if (!_tree_store.iter_next(ref child))
					break;
			}
		}
	}

	public void on_project_file_removed(string type, string name)
	{
		string parent_folder = ResourceId.parent_folder(name);
		Gtk.TreeIter child;

		if (!_folders.has_key(parent_folder))
			return;

		remove_from_favorites(type, name);

		// Remove from list store.
		if (_list_store.iter_children(out child, null)) {
			Value iter_type;
			Value iter_name;

			while (true) {
				_list_store.get_value(child, Column.TYPE, out iter_type);
				_list_store.get_value(child, Column.NAME, out iter_name);
				if ((string)iter_name == name && (string)iter_type == type) {
					_list_store.remove(ref child);
					break;
				} else {
					if (!_list_store.iter_next(ref child))
						break;
				}
			}
		}

		// Remove from tree store.
		Gtk.TreeIter parent_iter;
		_tree_store.get_iter(out parent_iter, _folders[parent_folder].get_path());
		if (_tree_store.iter_children(out child, parent_iter)) {
			Value iter_type;
			Value iter_name;

			while (true) {
				_tree_store.get_value(child, Column.TYPE, out iter_type);
				_tree_store.get_value(child, Column.NAME, out iter_name);
				if ((string)iter_name == name && (string)iter_type == type) {
					_tree_store.remove(ref child);
					break;
				} else {
					if (!_tree_store.iter_next(ref child))
						break;
				}
			}
		}
	}

	public void on_project_tree_added(string name)
	{
		Gtk.TreeIter iter;
		_list_store.insert_with_values(out iter
			, -1
			, Column.TYPE
			, "<folder>"
			, Column.NAME
			, name
			, Column.SIZE
			, 0u
			, Column.MTIME
			, 0u
			, Column.VISIBLE
			, true
			, -1
			);

		find_or_make_tree(name);
	}

	public void on_project_tree_removed(string name)
	{
		if (name == ROOT_FOLDER)
			return;

		if (!_folders.has_key(name))
			return;

		remove_from_favorites("<folder>", name);

		// Remove from list store.
		Gtk.TreeIter child;
		if (_list_store.iter_children(out child, null)) {
			Value iter_type;
			Value iter_name;

			while (true) {
				_list_store.get_value(child, Column.TYPE, out iter_type);
				_list_store.get_value(child, Column.NAME, out iter_name);
				if (((string)iter_name == name && (string)iter_type == "<folder>")
					|| ((string)iter_name).has_prefix(name + "/")
					) {
					if (!_list_store.remove(ref child))
						break;
				} else {
					if (!_list_store.iter_next(ref child))
						break;
				}
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

	public void on_project_reset()
	{
		reset();
	}

	public Gee.ArrayList<Value?> encode_favorites()
	{
		Gee.ArrayList<Value?> favorites = new Gee.ArrayList<Value?>();

		Gtk.TreeIter parent_iter;
		_tree_store.get_iter(out parent_iter, favorites_root_path());
		Gtk.TreeIter child;
		if (_tree_store.iter_children(out child, parent_iter)) {
			Value iter_type;
			Value iter_name;

			while (true) {
				_tree_store.get_value(child, Column.TYPE, out iter_type);
				_tree_store.get_value(child, Column.NAME, out iter_name);

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

	public void make_visible(bool visible)
	{
		_tree_store.foreach((model, path, iter) => {
				_tree_store.set(iter, Column.VISIBLE, visible, -1);
				return false; // Continue iterating.
			});

		_list_store.foreach((model, path, iter) => {
				_list_store.set(iter, Column.VISIBLE, visible, -1);
				return false; // Continue iterating.
			});
	}

	public bool is_visible(string type, string name, string needle)
	{
		// Always show the roots.
		if ((type == "<folder>" && name == "") || type == "<favorites>")
			return true;

		string basename = GLib.Path.get_basename(name);
		return basename != null
			&& (needle == ""
			|| basename.down().index_of(needle) > -1
			|| type.down().index_of(needle) > -1
			);
	}

	public void filter(string needle)
	{
		_tree_store.foreach((model, path, iter) => {
				string type;
				string name;

				Value val;
				model.get_value(iter, ProjectStore.Column.TYPE, out val);
				type = (string)val;
				model.get_value(iter, ProjectStore.Column.NAME, out val);
				name = (string)val;

				if (is_visible(type, name, needle)) {
					// Make this iter and all its ancestors visible.
					Gtk.TreeIter it = iter;
					_tree_store.set(it, Column.VISIBLE, true, -1);
					while (_tree_store.iter_parent(out it, it))
						_tree_store.set(it, Column.VISIBLE, true, -1);
				}

				return false; // Continue iterating.
			});

		_list_store.foreach((model, path, iter) => {
				string type;
				string name;

				Value val;
				model.get_value(iter, ProjectStore.Column.TYPE, out val);
				type = (string)val;
				model.get_value(iter, ProjectStore.Column.NAME, out val);
				name = (string)val;

				_list_store.set(iter, ProjectStore.Column.VISIBLE, is_visible(type, name, needle), -1);
				return false; // Continue iterating.
			});
	}
}

} /* namespace Crown */
