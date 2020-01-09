/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;
using Gee;

namespace Crown
{
	// Returns true if the item should be filtered out
	private bool user_filter(string type, string name)
	{
		return (type == "unit" || type == "sound") && !name.has_prefix("core/");
	}

	public delegate bool UserFilter(string type, string name);

	public class ResourceBrowser : Gtk.Box
	{
		// Data
		public Project _project;
		public GLib.Subprocess _engine_process;
		public ConsoleClient _console_client;
		public Gtk.TreeStore _tree_store;
		public bool _preview;
		public unowned UserFilter _user_filter;
		public string _name;

		// Widgets
		public Gtk.Entry _filter_entry;
		public Gtk.TreeModelFilter _tree_filter;
		public Gtk.TreeModelSort _tree_sort;
		public Gtk.TreeView _tree_view;
		public Gtk.TreeSelection _tree_selection;
		public Gtk.ScrolledWindow _scrolled_window;

		public EngineView _engine_view;

		// Signals
		public signal void resource_selected(string type, string name);

		public ResourceBrowser(Project? project, ProjectStore project_store, bool preview)
		{
			Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

			// Data
			_project = project;

			_console_client = new ConsoleClient();
			_tree_store = project_store._tree_store;
			_preview = preview;
			_user_filter = user_filter;

			// Widgets
			this.destroy.connect(on_destroy);

			_filter_entry = new Gtk.SearchEntry();
			_filter_entry.set_placeholder_text("Search...");
			_filter_entry.changed.connect(on_filter_entry_text_changed);
			_filter_entry.key_press_event.connect(on_filter_entry_key_pressed);

			_tree_filter = new Gtk.TreeModelFilter(_tree_store, null);
			_tree_filter.set_visible_func(filter_tree);

			_tree_sort = new Gtk.TreeModelSort.with_model(_tree_filter);
			_tree_sort.set_default_sort_func((model, iter_a, iter_b) => {
				Value id_a;
				Value id_b;
				model.get_value(iter_a, 0, out id_a);
				model.get_value(iter_b, 0, out id_b);
				return strcmp((string)id_a, (string)id_b);
			});

			_tree_view = new Gtk.TreeView();
			_tree_view.insert_column_with_attributes(-1, "Name", new Gtk.CellRendererText(), "text", 0, null);
			_tree_view.insert_column_with_attributes(-1, "Type", new Gtk.CellRendererText(), "text", 1, null); // Debug
			_tree_view.model = _tree_sort;
			_tree_view.headers_visible = false;
			_tree_view.can_focus = false;
			_tree_view.row_activated.connect(on_row_activated);
			_tree_view.button_release_event.connect(on_button_released);

			_tree_selection = _tree_view.get_selection();
			_tree_selection.set_mode(Gtk.SelectionMode.BROWSE);
			_tree_selection.changed.connect(on_tree_selection_changed);

			_scrolled_window = new Gtk.ScrolledWindow(null, null);
			_scrolled_window.add(_tree_view);
			_scrolled_window.set_size_request(300, 400);

			if (_preview)
			{
				_engine_view = new EngineView(_console_client, false);
				_engine_view.realized.connect(on_engine_view_realized);
				_engine_view.set_size_request(300, 300);
			}

			this.pack_start(_filter_entry, false, false, 0);
			if (_preview)
			{
				this.pack_start(_engine_view, true, true, 0);
			}
			this.pack_start(_scrolled_window, false, false, 0);
		}

		private void on_row_activated(Gtk.TreePath path, TreeViewColumn column)
		{
			Gtk.TreePath filter_path = _tree_sort.convert_path_to_child_path(path);
			Gtk.TreePath child_path = _tree_filter.convert_path_to_child_path(filter_path);
			Gtk.TreeIter iter;
			if (_tree_store.get_iter(out iter, child_path))
			{
				Value name;
				Value type;
				_tree_store.get_value(iter, 0, out name);
				_tree_store.get_value(iter, 1, out type);
				_name = (string)name;
				resource_selected((string)type, (string)name);
			}
		}

		private bool on_button_released(Gdk.EventButton ev)
		{
			if (ev.button == 1)
			{
				Gtk.TreePath path;
				int cell_x;
				int cell_y;
				if (_tree_view.get_path_at_pos((int)ev.x, (int)ev.y, out path, null, out cell_x, out cell_y))
				{
					if (_tree_view.get_selection().path_is_selected(path))
					{
						Gtk.TreePath filter_path = _tree_sort.convert_path_to_child_path(path);
						Gtk.TreePath child_path = _tree_filter.convert_path_to_child_path(filter_path);
						Gtk.TreeIter iter;
						if (_tree_store.get_iter(out iter, child_path))
						{
							Value name;
							Value type;
							_tree_store.get_value(iter, 0, out name);
							_tree_store.get_value(iter, 1, out type);
							_name = (string)name;
							resource_selected((string)type, (string)name);
						}
					}
				}
			}

			return false;
		}

		private void on_destroy()
		{
			stop_engine();
		}

		private void start_engine(uint window_xid)
		{
			if (window_xid == 0)
				return;

			string args[] =
			{
				ENGINE_EXE,
				"--data-dir", _project.data_dir(),
				"--boot-dir", UNIT_PREVIEW_BOOT_DIR,
				"--parent-window", window_xid.to_string(),
				"--console-port", "10002",
				"--wait-console",
				null
			};

			GLib.SubprocessLauncher sl = new GLib.SubprocessLauncher(SubprocessFlags.NONE);
			sl.set_cwd(ENGINE_DIR);
			try
			{
				_engine_process = sl.spawnv(args);
			}
			catch (Error e)
			{
				GLib.stderr.printf("%s\n", e.message);
			}

			while (!_console_client.is_connected())
			{
				_console_client.connect("127.0.0.1", 10002);
				GLib.Thread.usleep(100*1000);
			}

			_tree_view.set_cursor(new Gtk.TreePath.first(), null, false);
		}

		private void stop_engine()
		{
			_console_client.close();

			if (_engine_process != null)
			{
				_engine_process.force_exit();
				try
				{
					_engine_process.wait();
				}
				catch (Error e)
				{
					stderr.printf("Error: %s\n", e.message);
				}
			}
		}

		public void restart_engine()
		{
			stop_engine();
			start_engine(_engine_view.window_id);
		}

		private void on_engine_view_realized()
		{
			start_engine(_engine_view.window_id);
		}

		private void on_filter_entry_text_changed()
		{
			_tree_selection.changed.disconnect(on_tree_selection_changed);
			_tree_filter.refilter();
			_tree_selection.changed.connect(on_tree_selection_changed);
			_tree_view.set_cursor(new Gtk.TreePath.first(), null, false);
		}

		private bool filter_tree(Gtk.TreeModel model, Gtk.TreeIter iter)
		{
			Value type;
			Value name;
			model.get_value(iter, 1, out type);
			model.get_value(iter, 0, out name);

			string type_str = (string)type;
			string name_str = (string)name;

			return type_str != null
				&& name_str != null
				&& _user_filter(type_str, name_str)
				&& (_filter_entry.text.length == 0 || name_str.index_of(_filter_entry.text) > -1)
				;
		}

		private bool on_filter_entry_key_pressed(Gdk.EventKey ev)
		{
			Gtk.TreeModel model;
			Gtk.TreeIter iter;
			bool selected = _tree_selection.get_selected(out model, out iter);

			if (ev.keyval == Gdk.Key.Down)
			{
				if (selected && model.iter_next(ref iter))
				{
					_tree_selection.select_iter(iter);
					_tree_view.scroll_to_cell(model.get_path(iter), null, true, 1.0f, 0.0f);
				}
			}
			else if (ev.keyval == Gdk.Key.Up)
			{
				if (selected && model.iter_previous(ref iter))
				{
					_tree_selection.select_iter(iter);
					_tree_view.scroll_to_cell(model.get_path(iter), null, true, 1.0f, 0.0f);
				}
			}
			else if (ev.keyval == 65293) // Enter
			{
				if (selected)
				{
					Value name;
					Value type;
					model.get_value(iter, 0, out name);
					model.get_value(iter, 1, out type);
					_name = (string)name;
					resource_selected((string)type, (string)name);
				}
			}
			else
				return false;

			return true;
		}

		private void on_tree_selection_changed()
		{
			if (_preview)
			{
				Gtk.TreeModel model;
				Gtk.TreeIter iter;
				if (_tree_selection.get_selected(out model, out iter))
				{
					Value name;
					Value type;
					model.get_value(iter, 0, out name);
					model.get_value(iter, 1, out type);
					_console_client.send_script(UnitPreviewApi.set_preview_resource((string)type, (string)name));
				}
			}
		}

		public void set_type_filter(owned UserFilter filter)
		{
			_user_filter = filter;
			_tree_filter.refilter();
		}
	}
}
