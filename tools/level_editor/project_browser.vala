/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;
using Gee;

namespace Crown
{
	// Returns true if the row should be hidden
	private bool row_should_be_hidden(string type, string name)
	{
		return type == "<folder>" && name == "core"
			|| type == "wav"
			|| type == "png"
			;
	}

	public class ProjectBrowser : Gtk.Box
	{
		// Data
		public Project _project;
		public Gtk.TreeStore _tree_store;

		// Widgets
		public Gtk.TreeModelFilter _tree_filter;
		public Gtk.TreeModelSort _tree_sort;
		public Gtk.TreeView _tree_view;
		public Gtk.TreeSelection _tree_selection;
		public Gtk.ScrolledWindow _scrolled_window;

		// Signals
		public signal void resource_selected(string type, string name);

		public ProjectBrowser(Project? project, ProjectStore project_store)
		{
			Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

			// Data
			_project = project;
			_tree_store = project_store._tree_store;
			_tree_store.row_inserted.connect((path, iter) => {
					Value name;
					_tree_store.get_value(iter, ProjectStore.Column.NAME, out name);
					// _tree_view.expand_row(new Gtk.TreePath.first(), false);
				});

			// Widgets
			_tree_filter = new Gtk.TreeModelFilter(_tree_store, null);
			_tree_filter.set_visible_func(filter_tree);

			_tree_sort = new Gtk.TreeModelSort.with_model(_tree_filter);

			_tree_sort.set_default_sort_func((model, iter_a, iter_b) => {
				Value type_a;
				Value type_b;
				model.get_value(iter_a, ProjectStore.Column.TYPE, out type_a);
				model.get_value(iter_b, ProjectStore.Column.TYPE, out type_b);

				if ((string)type_a == "<folder>" && (string)type_b != "<folder>")
				{
					return -1;
				}
				else if ((string)type_a != "<folder>" && (string)type_b == "<folder>")
				{
					return 1;
				}
				else
				{
					Value id_a;
					Value id_b;
					model.get_value(iter_a, ProjectStore.Column.NAME, out id_a);
					model.get_value(iter_b, ProjectStore.Column.NAME, out id_b);
					return strcmp((string)id_a, (string)id_b);
				}
			});

			_tree_view = new Gtk.TreeView();

			Gtk.TreeViewColumn column = new Gtk.TreeViewColumn();
			Gtk.CellRendererPixbuf cell_pixbuf = new Gtk.CellRendererPixbuf();
			Gtk.CellRendererText cell_text = new Gtk.CellRendererText();
			column.pack_start(cell_pixbuf, false);
			column.pack_start(cell_text, true);
			column.set_cell_data_func(cell_pixbuf, (cell_layout, cell, model, iter) => {
				Value type;
				model.get_value(iter, ProjectStore.Column.TYPE, out type);

				// https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html
				if ((string)type == "<folder>")
					cell.set_property("icon-name", "folder-symbolic");
				else if ((string)type == "state_machine")
					cell.set_property("icon-name", "text-x-generic-symbolic");
				else if ((string)type == "config")
					cell.set_property("icon-name", "text-x-generic-symbolic");
				else if ((string)type == "font")
					cell.set_property("icon-name", "font-x-generic-symbolic");
				else if ((string)type == "unit")
					cell.set_property("icon-name", "text-x-generic-symbolic");
				else if ((string)type == "level")
					cell.set_property("icon-name", "text-x-generic-symbolic");
				else if ((string)type == "material")
					cell.set_property("icon-name", "text-x-generic-symbolic");
				else if ((string)type == "mesh")
					cell.set_property("icon-name", "text-x-generic-symbolic");
				else if ((string)type == "package")
					cell.set_property("icon-name", "package-x-generic-symbolic");
				else if ((string)type == "physics_config")
					cell.set_property("icon-name", "text-x-generic-symbolic");
				else if ((string)type == "lua")
					cell.set_property("icon-name", "text-x-script-symbolic");
				else if ((string)type == "shader")
					cell.set_property("icon-name", "text-x-script-symbolic");
				else if ((string)type == "sound")
					cell.set_property("icon-name", "audio-x-generic-symbolic");
				else if ((string)type == "sprite_animation")
					cell.set_property("icon-name", "text-x-generic-symbolic");
				else if ((string)type == "sprite")
					cell.set_property("icon-name", "text-x-generic-symbolic");
				else if ((string)type == "texture")
					cell.set_property("icon-name", "image-x-generic-symbolic");
				else
					cell.set_property("icon-name", "text-x-generic-symbolic");
			});
			column.set_cell_data_func(cell_text, (cell_layout, cell, model, iter) => {
				Value segment;
				Value type;
				model.get_value(iter, ProjectStore.Column.SEGMENT, out segment);
				model.get_value(iter, ProjectStore.Column.TYPE, out type);

				if ((string)type == "<folder>")
					cell.set_property("text", (string)segment);
				else
					cell.set_property("text", (string)segment + "." + (string)type);
			});
			_tree_view.append_column(column);

/*
			// This is for debugging only
			_tree_view.insert_column_with_attributes(-1
				, "Segment"
				, new Gtk.CellRendererText()
				, "text"
				, ProjectStore.Column.SEGMENT
				, null
				);
			_tree_view.insert_column_with_attributes(-1
				, "Name"
				, new Gtk.CellRendererText()
				, "text"
				, ProjectStore.Column.NAME
				, null
				);
			_tree_view.insert_column_with_attributes(-1
				, "Type"
				, new Gtk.CellRendererText()
				, "text"
				, ProjectStore.Column.TYPE
				, null
				);
*/

			_tree_view.model = _tree_sort;
			_tree_view.headers_visible = false;
			_tree_view.can_focus = false;
			_tree_view.button_press_event.connect(on_button_pressed);

			_tree_selection = _tree_view.get_selection();
			_tree_selection.set_mode(Gtk.SelectionMode.BROWSE);

			_scrolled_window = new Gtk.ScrolledWindow(null, null);
			_scrolled_window.add(_tree_view);

			this.pack_start(_scrolled_window, true, true, 0);
		}

		private bool on_button_pressed(Gdk.EventButton ev)
		{
			if (ev.button == 3) // Right
			{
				Gtk.TreePath path;
				int cell_x;
				int cell_y;
				if (_tree_view.get_path_at_pos((int)ev.x, (int)ev.y, out path, null, out cell_x, out cell_y))
				{
					Gtk.TreeIter iter;
					_tree_view.model.get_iter(out iter, path);

					Value type;
					_tree_view.model.get_value(iter, ProjectStore.Column.TYPE, out type);

					if (type == "<folder>")
					{
						Gtk.Menu menu = new Gtk.Menu();
						Gtk.MenuItem mi;

						mi = new Gtk.MenuItem.with_label("New Folder...");
						mi.activate.connect(() => {
							Value name;
							_tree_view.model.get_value(iter, ProjectStore.Column.NAME, out name);

							Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Folder Name"
								, null
								, DialogFlags.MODAL
								, "Cancel"
								, ResponseType.CANCEL
								, "Ok"
								, ResponseType.OK
								, null
								);

							Gtk.Entry sb = new Gtk.Entry();
							sb.activate.connect(() => { dg.response(ResponseType.OK); });
							dg.get_content_area().add(sb);
							dg.skip_taskbar_hint = true;
							dg.show_all();

							if (dg.run() == (int)ResponseType.OK)
							{
								try
								{
									GLib.File file = GLib.File.new_build_filename(_project.source_dir(), (string)name, sb.text);
									file.make_directory();
								}
								catch (Error e)
								{
									// _console_view.loge("editor", e.message);
								}
							}

							dg.destroy();
						});
						menu.add(mi);

						mi = new Gtk.MenuItem.with_label("Import...");
						mi.activate.connect(() => {
							Value name;
							_tree_view.model.get_value(iter, ProjectStore.Column.NAME, out name);

							GLib.File file = GLib.File.new_build_filename(_project.source_dir(), (string)name);
							_project.import(file.get_path(), (Gtk.Window)this.get_toplevel());
						});
						menu.add(mi);

						menu.show_all();
						menu.popup(null, null, null, ev.button, ev.time);
					}
					else
					{
						Gtk.Menu menu = new Gtk.Menu();
						Gtk.MenuItem mi;

						mi = new Gtk.MenuItem.with_label("Open Containing Folder...");
						mi.activate.connect(() => {
							Gtk.TreeIter parent;
							if (_tree_view.model.iter_parent(out parent, iter))
							{
								Value parent_name;
								_tree_view.model.get_value(parent, ProjectStore.Column.NAME, out parent_name);

								try
								{
									GLib.File file = GLib.File.new_build_filename(_project.source_dir(), (string)parent_name);
									GLib.AppInfo.launch_default_for_uri("file://" + file.get_path(), null);
								}
								catch (Error e)
								{
									// _console_view.loge("editor", e.message);
								}
							}
						});
						menu.add(mi);

						menu.show_all();
						menu.popup(null, null, null, ev.button, ev.time);
					}
				}
			}
			else if (ev.button == 1) // Left
			{
				if (ev.type == Gdk.EventType.2BUTTON_PRESS)
				{
					Gtk.TreePath path;
					int cell_x;
					int cell_y;
					if (_tree_view.get_path_at_pos((int)ev.x, (int)ev.y, out path, null, out cell_x, out cell_y))
					{
						Gtk.TreeIter iter;
						_tree_view.model.get_iter(out iter, path);

						Value type;
						_tree_view.model.get_value(iter, ProjectStore.Column.TYPE, out type);
						if ((string)type == "<folder>")
							return false;

						Value name;
						_tree_view.model.get_value(iter, ProjectStore.Column.NAME, out name);

						GLib.AppInfo? app = GLib.AppInfo.get_default_for_type("text/plain", false);
						if (app != null)
						{
							GLib.File file = GLib.File.new_build_filename(_project.source_dir(), (string)name + "." + (string)type);
							GLib.List<GLib.File> files = new GLib.List<GLib.File>();
							files.append(file);
							try
							{
								app.launch(files, null);
							}
							catch (Error e)
							{
								// _console_view.loge("editor", e.message);
							}
						}
					}
				}
			}

			return false;
		}

		private bool filter_tree(Gtk.TreeModel model, Gtk.TreeIter iter)
		{
			Value type;
			Value name;
			model.get_value(iter, ProjectStore.Column.TYPE, out type);
			model.get_value(iter, ProjectStore.Column.NAME, out name);

			return (string)type != null
				&& (string)name != null
				&& !row_should_be_hidden((string)type, (string)name)
				;
		}
	}
}
