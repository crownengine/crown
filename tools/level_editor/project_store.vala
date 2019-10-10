/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;
using Gee;

namespace Crown
{
	public class ProjectStore
	{
		// Data
		public Project _project;
		public Gtk.TreeStore _tree_store;

		public ProjectStore(Project project)
		{
			// Data
			_project = project;
			_project.changed.connect(on_project_changed);

			_tree_store = new Gtk.TreeStore(2
				, typeof(string) // resource name
				, typeof(string) // resource type
				);

			read_project();
		}

		private void read_project()
		{
			_tree_store.clear();

			Database db = _project.files();
			HashSet<Guid?> files = db.get_property_set(GUID_ZERO, "data", new Gee.HashSet<Guid?>());
			files.foreach((id) => {
				Gtk.TreeIter resource_iter;
				_tree_store.append(out resource_iter, null);
				string name = db.get_property_string(id, "name");
				string type = db.get_property_string(id, "type");
				_tree_store.set(resource_iter, 0, name, 1, type, -1);
				return true;
			});
		}

		private void on_project_changed()
		{
			read_project();
		}
	}
}
