/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
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
		SEGMENT,

		COUNT
	}

	// Data
	public Project _project;
	public Gtk.TreeStore _tree_store;
	public Gtk.ListStore _list_store;
	public Gee.HashMap<string, Gtk.TreeRowReference> _folders;

	public ProjectStore(Project project)
	{
		// Data
		_project = project;
		_project.file_added.connect(on_project_file_added);
		_project.file_removed.connect(on_project_file_removed);
		_project.tree_added.connect(on_project_tree_added);
		_project.tree_removed.connect(on_project_tree_removed);
		_project.project_reset.connect(on_project_reset);
		_project.project_loaded.connect(on_project_loaded);

		_tree_store = new Gtk.TreeStore(Column.COUNT
			, typeof(string) // resource name
			, typeof(string) // resource type
			, typeof(string) // resource segment
			);
		_list_store = new Gtk.ListStore(2
			, typeof(string) // resource name
			, typeof(string) // resource type
			);

		_folders = new Gee.HashMap<string, Gtk.TreeRowReference>();

		reset();
	}

	public void reset()
	{
		_folders.clear();
		_tree_store.clear();
		_list_store.clear();
	}

	private string folder(string name)
	{
		int last_slash = name.last_index_of_char('/');
		if (last_slash == -1)
			return ROOT_FOLDER;
		return name.substring(0, last_slash);
	}

	private string segment(string name)
	{
		int last_slash = name.last_index_of_char('/');
		if (last_slash == -1)
			return name;
		return name.substring(last_slash + 1);
	}

	public bool path_for_resource_type_name(out Gtk.TreePath path, string type, string name)
	{
		string f = folder(name);
		if (_folders.has_key(f))
		{
			// Find the name inside the folder.
			Gtk.TreeIter parent_iter;
			_tree_store.get_iter(out parent_iter, _folders[f].get_path());

			Gtk.TreeIter child;
			if (_tree_store.iter_children(out child, parent_iter))
			{
				Value iter_name;
				Value iter_type;

				while (true)
				{
					_tree_store.get_value(child, Column.NAME, out iter_name);
					_tree_store.get_value(child, Column.TYPE, out iter_type);
					if ((string)iter_name == name && (string)iter_type == type)
					{
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

	private Gtk.TreeIter make_tree_internal(string folder, int start_index, Gtk.TreeRowReference parent)
	{
		// Folder can be:
		// "", root folder
		// "folder", one word
		// "folder/another_folder", any number of words concatenated by '/'
		int first_slash = folder.index_of_char('/', start_index);
		if (first_slash == -1)
		{
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
				, Column.SEGMENT
				, folder.substring(start_index)
				, -1
				);

			_folders[folder] = new Gtk.TreeRowReference(_tree_store, _tree_store.get_path(iter));
			return iter;
		}
		else
		{
			if (_folders.has_key(folder.substring(0, first_slash)))
			{
				return make_tree_internal(folder, first_slash + 1, _folders[folder.substring(0, first_slash)]);
			}
			else
			{
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
					, Column.SEGMENT
					, folder.substring(start_index, first_slash - start_index)
					, -1
					);

				Gtk.TreeRowReference trr = new Gtk.TreeRowReference(_tree_store, _tree_store.get_path(iter));
				_folders[folder.substring(0, first_slash)] = trr;

				return make_tree_internal(folder, first_slash + 1, trr);
			}
		}
	}

	private Gtk.TreeIter make_tree(string folder)
	{
		if (_folders.has_key(folder))
		{
			Gtk.TreeIter iter;
			_tree_store.get_iter(out iter, _folders[folder].get_path());
			return iter;
		}

		return make_tree_internal(folder, 0, _folders[ROOT_FOLDER]);
	}

	private void on_project_file_added(string type, string name)
	{
		string f = folder(name);
		string s = segment(name);
		Gtk.TreeIter parent = make_tree(f);
		Gtk.TreeIter iter;
		_tree_store.insert_with_values(out iter
			, parent
			, -1
			, Column.NAME
			, name
			, Column.TYPE
			, type
			, Column.SEGMENT
			, s
			, -1
			);
		_list_store.insert_with_values(out iter
			, -1
			, Column.NAME
			, name
			, Column.TYPE
			, type
			, -1
			);
	}

	private void on_project_file_removed(string type, string name)
	{
		string f = folder(name);
		if (!_folders.has_key(f))
			return;

		// Remove from tree store
		Gtk.TreeIter parent_iter;
		_tree_store.get_iter(out parent_iter, _folders[f].get_path());
		Gtk.TreeIter child;
		if (_tree_store.iter_children(out child, parent_iter))
		{
			Value iter_name;
			Value iter_type;

			while (true)
			{
				_tree_store.get_value(child, Column.NAME, out iter_name);
				_tree_store.get_value(child, Column.TYPE, out iter_type);
				if ((string)iter_name == name && (string)iter_type == type)
				{
					_tree_store.remove(ref child);
					return;
				}

				if (!_tree_store.iter_next(ref child))
					break;
			}
		}

		// Remove from list store
		if (_list_store.iter_children(out child, null))
		{
			Value iter_name;
			Value iter_type;

			while (true)
			{
				_list_store.get_value(child, Column.NAME, out iter_name);
				_list_store.get_value(child, Column.TYPE, out iter_type);
				if ((string)iter_name == name && (string)iter_type == type)
				{
					_list_store.remove(ref child);
					return;
				}

				if (!_list_store.iter_next(ref child))
					break;
			}
		}
	}

	private void on_project_tree_added(string name)
	{
		make_tree(name);
	}

	private void on_project_tree_removed(string name)
	{
		if (name == ROOT_FOLDER)
			return;

		if (!_folders.has_key(name))
			return;

		// Remove the tree
		Gtk.TreeIter iter;
		_tree_store.get_iter(out iter, _folders[name].get_path());
		_tree_store.remove(ref iter);

		// Remove any stale TreeRowRerefence
		var it = _folders.map_iterator();
		for (var has_next = it.next(); has_next; has_next = it.next())
		{
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
			, Column.SEGMENT
			, _project.name()
			, -1
			);

		_folders[ROOT_FOLDER] = new Gtk.TreeRowReference(_tree_store, _tree_store.get_path(iter));
	}
}

}
