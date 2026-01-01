/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class ProjectRow : Gtk.ListBoxRow
{
	public Gtk.Box _vbox;
	public Gtk.Box _hbox;
	public Gtk.Label _name;
	public Gtk.Label _source_dir;
	public Gtk.Button _remove_button;
	public Gtk.Button _open_button;
	public ProjectsList _projects_list;

	public ProjectRow(string source_dir, string time, string name, ProjectsList pl)
	{
		this.set_data("source_dir", source_dir);
		this.set_data("mtime", time);
		_projects_list = pl;

		_vbox = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);

		_name = new Gtk.Label(null);
		_name.set_margin_start(12);
		_name.set_margin_end(12);
		_name.set_margin_top(8);
		_name.set_margin_bottom(8);
		_name.set_markup("<b>%s</b>".printf(name));
		_name.set_xalign(0.0f);
		_vbox.pack_start(_name);

		_source_dir = new Gtk.Label(null);
		_source_dir.set_margin_start(12);
		_source_dir.set_margin_end(12);
		_source_dir.set_margin_bottom(8);
		_source_dir.set_markup("<small>%s</small>".printf(source_dir));
		_source_dir.set_xalign(0.0f);
		_vbox.pack_start(_source_dir);

		_hbox = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
		_hbox.pack_start(_vbox);

		_remove_button = new Gtk.Button.from_icon_name("list-remove-symbolic");
		_remove_button.get_style_context().add_class("flat");
		_remove_button.get_style_context().add_class("destructive-action");
		_remove_button.set_halign(Gtk.Align.CENTER);
		_remove_button.set_valign(Gtk.Align.CENTER);
		_remove_button.set_margin_end(12);
		_hbox.pack_end(_remove_button, false, false, 0);

		_open_button = new Gtk.Button.with_label("Open");
		_open_button.get_style_context().add_class("flat");
		_open_button.set_halign(Gtk.Align.CENTER);
		_open_button.set_valign(Gtk.Align.CENTER);
		// _open_button.set_margin_end(12);
		_hbox.pack_end(_open_button, false, false, 0);

		_remove_button.clicked.connect(on_remove_button_clicked);
		_open_button.clicked.connect(on_open_button_clicked);

		this.add(_hbox);
	}

	public void on_remove_button_clicked()
	{
		GLib.Application.get_default().activate_action("remove-project"
			, new GLib.Variant.string(this.get_data("source_dir"))
			);
	}

	public void on_open_button_clicked()
	{
		GLib.Application.get_default().activate_action("open-project"
			, new GLib.Variant.tuple({this.get_data<string>("source_dir"), LEVEL_NONE})
			);
	}
}

public class ProjectsList : Gtk.Box
{
	// Data
	public User _user;

	// Widgets
	public Gtk.Label _projects_list_label;
	public Gtk.Label _local_label;
	public Gtk.Box _project_list_empty;
	public Gtk.ListBox _list_projects;
	public Gtk.Button _button_import_project;
	public Gtk.Button _button_new_project;
	public Gtk.Box _buttons_box;
	public Gtk.Box _projects_box;
	public Clamp _clamp;

	public ProjectsList(User user)
	{
		Object(orientation: Gtk.Orientation.VERTICAL);

		// Data
		_user = user;

		_projects_list_label = new Gtk.Label(null);
		_projects_list_label.xalign = 0;
		_projects_list_label.set_markup("<span font_weight=\"bold\" size=\"x-large\">Projects</span>");

		_local_label = new Gtk.Label("Local");
		_local_label.set_markup("<span font_weight=\"bold\">Local</span>");

		_project_list_empty = new Gtk.Box(Gtk.Orientation.VERTICAL, 6);
		_project_list_empty.margin_top = 12;
		_project_list_empty.margin_bottom = 12;
		var label = new Gtk.Label(null);
		label.set_markup("<span font_size=\"large\"><b>No projects found</b></span>");
		_project_list_empty.pack_start(label, false, false);
		label = new Gtk.Label(null);
		label.set_markup("Use the buttons above to create a new project or import an already existing one.");
		_project_list_empty.pack_start(label, false, false);
		_project_list_empty.show_all();

		_list_projects = new Gtk.ListBox();
		_list_projects.set_placeholder(_project_list_empty);
		_list_projects.set_sort_func((row1, row2) => {
				int64 mtime1 = int64.parse(row1.get_data("mtime"));
				int64 mtime2 = int64.parse(row2.get_data("mtime"));
				return mtime1 > mtime2 ? -1 : 1; // LRU
			});

		_button_import_project = new Gtk.Button.with_label("Import...");
		_button_import_project.action_name = "app.add-project";

		_button_new_project = new Gtk.Button.with_label("Create New");
		_button_new_project.get_style_context().add_class("suggested-action");
		_button_new_project.action_name = "app.new-project";

		_buttons_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_buttons_box.spacing = 6;
		_buttons_box.pack_start(_local_label, false, true);
		_buttons_box.pack_end(_button_new_project, false, true);
		_buttons_box.pack_end(_button_import_project, false, true);

		_projects_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_projects_box.margin_start = 12;
		_projects_box.margin_end = 12;
		_projects_box.margin_top = 32;
		_projects_box.margin_bottom = 32;
		_projects_box.spacing = 12;
		_projects_box.pack_start(_projects_list_label, false, true);
		_projects_box.pack_start(_buttons_box, false, true);
		_projects_box.pack_start(_list_projects, false, true);

		_clamp = new Clamp();
		_clamp.set_child(_projects_box);

		this.add(_clamp);

		_user.recent_project_added.connect(on_recent_project_added);
		_user.recent_project_touched.connect(on_recent_project_touched);
		_user.recent_project_removed.connect(on_recent_project_removed);
	}

	public void on_recent_project_added(string source_dir, string name, string time)
	{
		// Add project row.
		var row = new ProjectRow(source_dir, time, name, this);

		_list_projects.add(row);
		_list_projects.show_all(); // Otherwise the list is not always updated...

		if (!GLib.FileUtils.test(source_dir, FileTest.EXISTS))
			row._open_button.sensitive = false;

		invalidate_sort();
	}

	public void on_recent_project_touched(string source_dir, string mtime)
	{
		_list_projects.foreach((row) => {
				if (row.get_data<string>("source_dir") == source_dir) {
					row.set_data("mtime", mtime);
					return;
				}
			});

		invalidate_sort();
	}

	public void on_recent_project_removed(string source_dir)
	{
		_list_projects.foreach((row) => {
				if (row.get_data<string>("source_dir") == source_dir) {
					_list_projects.remove(row);
					return;
				}
			});
	}

	public void invalidate_sort()
	{
		_list_projects.invalidate_sort();

		// Give focus to most recent project's open button.
		ProjectRow? first_row = (ProjectRow?)_list_projects.get_row_at_index(0);
		if (first_row != null)
			first_row._open_button.has_focus = true;
	}
}

} /* namespace Crown */
