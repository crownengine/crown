/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
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
		|| type == "importer_settings"
		|| name == Project.LEVEL_EDITOR_TEST_NAME
		;
}

public class ProjectBrowser : Gtk.Box
{
	// Data
	public Project _project;
	public ProjectStore _project_store;

	// Widgets
	public Gtk.TreeModelFilter _tree_filter;
	public Gtk.TreeModelSort _tree_sort;
	public Gtk.TreeView _tree_view;
	public Gtk.TreeSelection _tree_selection;
	public Gtk.ScrolledWindow _scrolled_window;

	// Signals
	public signal void resource_selected(string type, string name);

	public static string join(string type, string name)
	{
		return type == "" ? name : name + "." + type;
	}

	public static string filename(Project project, string type, string name)
	{
		string bn = ProjectBrowser.join(type, name);
		return Path.build_filename(project.source_dir(), bn);
	}

	public ProjectBrowser(Gtk.Application app, Project? project, ProjectStore project_store)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

		// Data
		_project = project;
		_project_store = project_store;

		// Widgets
		_tree_filter = new Gtk.TreeModelFilter(_project_store._tree_store, null);
		_tree_filter.set_visible_func((model, iter) => {
			_tree_view.expand_row(new Gtk.TreePath.first(), false);

			Value type;
			Value name;
			model.get_value(iter, ProjectStore.Column.TYPE, out type);
			model.get_value(iter, ProjectStore.Column.NAME, out name);

			return (string)type != null
				&& (string)name != null
				&& !row_should_be_hidden((string)type, (string)name)
				;
		});

		_tree_sort = new Gtk.TreeModelSort.with_model(_tree_filter);
		_tree_sort.set_default_sort_func((model, iter_a, iter_b) => {
			Value type_a;
			Value type_b;
			model.get_value(iter_a, ProjectStore.Column.TYPE, out type_a);
			model.get_value(iter_b, ProjectStore.Column.TYPE, out type_b);

			if ((string)type_a == "<folder>")
			{
				if ((string)type_b != "<folder>")
					return -1;
			}
			else if ((string)type_b == "<folder>")
			{
				if ((string)type_a != "<folder>")
					return 1;
			}

			Value id_a;
			Value id_b;
			model.get_value(iter_a, ProjectStore.Column.SEGMENT, out id_a);
			model.get_value(iter_b, ProjectStore.Column.SEGMENT, out id_b);
			return strcmp((string)id_a, (string)id_b);
		});

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
				cell.set_property("icon-name", "level-object-unit");
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
				cell.set_property("icon-name", "x-office-document-symbolic");
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
				cell.set_property("text", ProjectBrowser.join((string)type, (string)segment));
		});
		_tree_view = new Gtk.TreeView();
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
		_tree_view.button_release_event.connect(on_button_released);

		_tree_selection = _tree_view.get_selection();
		_tree_selection.set_mode(Gtk.SelectionMode.BROWSE);

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.add(_tree_view);

		this.pack_start(_scrolled_window, true, true, 0);

		// Actions.
		GLib.ActionEntry[] action_entries =
		{
			{ "project-browser-reveal", on_reveal, "s", null }
		};
		app.add_action_entries(action_entries, this);
	}

	private void on_reveal(GLib.SimpleAction action, GLib.Variant? param)
	{
		string resource_path = param.get_string();
		string? type = resource_type(resource_path);
		string? name = resource_name(type, resource_path);
		if (type == null || name == null)
			return;

		Gtk.TreePath store_path;
		if (_project_store.path_for_resource_type_name(out store_path, type, name))
		{
			Gtk.TreePath filter_path = _tree_filter.convert_child_path_to_path(store_path);
			if (filter_path == null) // Either the path is not valid or points to a non-visible row in the model.
				return;
			Gtk.TreePath sort_path = _tree_sort.convert_child_path_to_path(filter_path);
			if (sort_path == null) // The path is not valid.
				return;

			_tree_view.expand_to_path(sort_path);
			_tree_view.get_selection().select_path(sort_path);
		}
	}

	private bool on_button_pressed(Gdk.EventButton ev)
	{
		if (ev.button == Gdk.BUTTON_SECONDARY)
		{
			Gtk.TreePath path;
			if (_tree_view.get_path_at_pos((int)ev.x, (int)ev.y, out path, null, null, null))
			{
				Gtk.TreeIter iter;
				_tree_view.model.get_iter(out iter, path);

				Value type;
				Value name;
				_tree_view.model.get_value(iter, ProjectStore.Column.TYPE, out type);
				_tree_view.model.get_value(iter, ProjectStore.Column.NAME, out name);

				if (type == "<folder>")
				{
					Gtk.Menu menu = new Gtk.Menu();
					Gtk.MenuItem mi;

					mi = new Gtk.MenuItem.with_label("Import...");
					mi.activate.connect(() => {
						Gtk.Application app = ((Gtk.Window)this.get_toplevel()).application;
						app.activate_action("import", new GLib.Variant.string((string)name));
					});
					menu.add(mi);

					mi = new Gtk.SeparatorMenuItem();
					menu.add(mi);

					mi = new Gtk.MenuItem.with_label("New Script...");
					mi.activate.connect(() => {
						Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Script Name"
							, (Gtk.Window)this.get_toplevel()
							, DialogFlags.MODAL
							, "Cancel"
							, ResponseType.CANCEL
							, "Ok"
							, ResponseType.OK
							, null
							);

						EntryText sb = new EntryText();
						sb.activate.connect(() => { dg.response(ResponseType.OK); });
						dg.get_content_area().add(sb);
						dg.skip_taskbar_hint = true;
						dg.show_all();

						if (dg.run() == (int)ResponseType.OK)
						{
							if (sb.text.strip() == "")
							{
								dg.destroy();
								return;
							}

							_project.create_script((string)name, sb.text, true);
						}

						dg.destroy();
					});
					menu.add(mi);

					mi = new Gtk.MenuItem.with_label("New Script (Unit)...");
					mi.activate.connect(() => {
						Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Script Name"
							, (Gtk.Window)this.get_toplevel()
							, DialogFlags.MODAL
							, "Cancel"
							, ResponseType.CANCEL
							, "Ok"
							, ResponseType.OK
							, null
							);

						EntryText sb = new EntryText();
						sb.activate.connect(() => { dg.response(ResponseType.OK); });
						dg.get_content_area().add(sb);
						dg.skip_taskbar_hint = true;
						dg.show_all();

						if (dg.run() == (int)ResponseType.OK)
						{
							if (sb.text.strip() == "")
							{
								dg.destroy();
								return;
							}

							_project.create_script((string)name, sb.text, false);
						}

						dg.destroy();
					});
					menu.add(mi);

					mi = new Gtk.SeparatorMenuItem();
					menu.add(mi);

					mi = new Gtk.MenuItem.with_label("New Unit...");
					mi.activate.connect(() => {
						Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Unit Name"
							, (Gtk.Window)this.get_toplevel()
							, DialogFlags.MODAL
							, "Cancel"
							, ResponseType.CANCEL
							, "Ok"
							, ResponseType.OK
							, null
							);

						EntryText sb = new EntryText();
						sb.activate.connect(() => { dg.response(ResponseType.OK); });
						dg.get_content_area().add(sb);
						dg.skip_taskbar_hint = true;
						dg.show_all();

						if (dg.run() == (int)ResponseType.OK)
						{
							if (sb.text.strip() == "")
							{
								dg.destroy();
								return;
							}

							_project.create_unit((string)name, sb.text);
						}

						dg.destroy();
					});
					menu.add(mi);

					mi = new Gtk.SeparatorMenuItem();
					menu.add(mi);

					mi = new Gtk.MenuItem.with_label("New Folder...");
					mi.activate.connect(() => {
						Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Folder Name"
							, (Gtk.Window)this.get_toplevel()
							, DialogFlags.MODAL
							, "Cancel"
							, ResponseType.CANCEL
							, "Ok"
							, ResponseType.OK
							, null
							);

						EntryText sb = new EntryText();
						sb.activate.connect(() => { dg.response(ResponseType.OK); });
						dg.get_content_area().add(sb);
						dg.skip_taskbar_hint = true;
						dg.show_all();

						if (dg.run() == (int)ResponseType.OK)
						{
							if (sb.text.strip() == "")
							{
								dg.destroy();
								return;
							}

							GLib.File file = GLib.File.new_for_path(GLib.Path.build_filename(_project.source_dir(), (string)name, sb.text));
							try
							{
								file.make_directory();
							}
							catch (Error e)
							{
								loge(e.message);
							}
						}

						dg.destroy();
					});
					menu.add(mi);

					if ((string)name != ProjectStore.ROOT_FOLDER)
					{
						mi = new Gtk.MenuItem.with_label("Delete Folder");
						mi.activate.connect(() => {
							Gtk.MessageDialog md = new Gtk.MessageDialog((Gtk.Window)this.get_toplevel()
								, Gtk.DialogFlags.MODAL
								, Gtk.MessageType.WARNING
								, Gtk.ButtonsType.NONE
								, "Delete Folder " + (string)name + "?"
								);
							md.add_button("_Cancel", ResponseType.CANCEL);
							md.add_button("_Delete", ResponseType.YES);
							md.set_default_response(ResponseType.CANCEL);
							int rt = md.run();
							md.destroy();

							if (rt == (int)ResponseType.CANCEL)
								return;

							GLib.File file = GLib.File.new_for_path(GLib.Path.build_filename(_project.source_dir(), (string)name));
							try
							{
								_project.delete_tree(file);
							}
							catch (Error e)
							{
								loge(e.message);
							}
						});
						menu.add(mi);
					}

					menu.show_all();
					menu.popup(null, null, null, ev.button, ev.time);
				}
				else // If file
				{
					Gtk.Menu menu = new Gtk.Menu();
					Gtk.MenuItem mi;

					mi = new Gtk.MenuItem.with_label("Delete File");
					mi.activate.connect(() => {
						GLib.File file = GLib.File.new_for_path(ProjectBrowser.filename(_project, (string)type, (string)name));
						try
						{
							file.delete();
						}
						catch (Error e)
						{
							loge(e.message);
						}
					});
					menu.add(mi);

					mi = new Gtk.MenuItem.with_label("Open Containing Folder...");
					mi.activate.connect(() => {
						Gtk.TreeIter parent;
						if (_tree_view.model.iter_parent(out parent, iter))
						{
							Value parent_name;
							_tree_view.model.get_value(parent, ProjectStore.Column.NAME, out parent_name);

							GLib.File file = GLib.File.new_for_path(GLib.Path.build_filename(_project.source_dir(), (string)parent_name));
							open_directory(file.get_path());
						}
					});
					menu.add(mi);

					menu.show_all();
					menu.popup(null, null, null, ev.button, ev.time);
				}
			}
		}
		else if (ev.button == Gdk.BUTTON_PRIMARY)
		{
			if (ev.type == Gdk.EventType.2BUTTON_PRESS)
			{
				Gtk.TreePath path;
				if (_tree_view.get_path_at_pos((int)ev.x, (int)ev.y, out path, null, null, null))
				{
					Gtk.TreeIter iter;
					_tree_view.model.get_iter(out iter, path);

					Value type;
					_tree_view.model.get_value(iter, ProjectStore.Column.TYPE, out type);
					if ((string)type == "<folder>")
						return Gdk.EVENT_PROPAGATE;

					Value name;
					_tree_view.model.get_value(iter, ProjectStore.Column.NAME, out name);

					Gtk.Application app = ((Gtk.Window)this.get_toplevel()).application;
					app.activate_action("open-resource", ProjectBrowser.join((string)type, (string)name));
				}
			}
		}

		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_button_released(Gdk.EventButton ev)
	{
		if (ev.button == Gdk.BUTTON_PRIMARY)
		{
			Gtk.TreePath path;
			if (_tree_view.get_path_at_pos((int)ev.x, (int)ev.y, out path, null, null, null))
			{
				Gtk.TreeIter iter;
				_tree_view.model.get_iter(out iter, path);

				Value type;
				_tree_view.model.get_value(iter, ProjectStore.Column.TYPE, out type);
				if ((string)type != "<folder>")
					return Gdk.EVENT_PROPAGATE;

				if (_tree_view.is_row_expanded(path))
					_tree_view.collapse_row(path);
				else
					_tree_view.expand_row(path, /*open_all = */false);

				return Gdk.EVENT_STOP;
			}
		}

		return Gdk.EVENT_PROPAGATE;
	}
}

}
