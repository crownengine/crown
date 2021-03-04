/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;
using Gee;

namespace Crown
{
[GtkTemplate (ui = "/org/crown/level_editor/ui/panel_projects_list.ui")]
public class PanelProjectsList : Gtk.ScrolledWindow
{
	// Data
	LevelEditorApplication _application;
	User _user;

	// Widgets
	[GtkChild]
	unowned Gtk.ListBox _list_projects;

	[GtkChild]
	unowned Gtk.Button _button_new_project;

	[GtkChild]
	unowned Gtk.Button _button_import_project;

	public PanelProjectsList(LevelEditorApplication app, User user)
	{
		// Data
		_application = app;
		_user = user;

		_list_projects.set_sort_func((row1, row2) => {
			int64 mtime1 = int64.parse(row1.get_data("mtime"));
			int64 mtime2 = int64.parse(row2.get_data("mtime"));
			return mtime1 > mtime2 ? -1 : 1; // LRU
		});

		_button_new_project.clicked.connect(() => {
			_application.show_panel("panel_new_project", Gtk.StackTransitionType.SLIDE_DOWN);
		});

		_button_import_project.clicked.connect(() => {
			string source_dir;
			int rt = _application.run_open_project_dialog(out source_dir, (Gtk.Window)this.get_toplevel());
			if (rt != ResponseType.ACCEPT)
				return;

			_user.add_recent_project(source_dir, source_dir);
		});

		_user.recent_project_added.connect(on_recent_project_added);
		// _user.recent_project_removed.connect(on_recent_project_remove);
	}

	public void on_recent_project_added(string source_dir, string name, string time)
	{
		Gtk.Widget widget;

		// Add row
		Gtk.ListBoxRow row = new Gtk.ListBoxRow();
		row.set_data("source_dir", source_dir);
		row.set_data("mtime", time);
		Gtk.Box vbox = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);

		widget = new Gtk.Label(null);
		widget.set_margin_start(12);
		widget.set_margin_end(12);
		widget.set_margin_top(8);
		widget.set_margin_bottom(8);
		((Gtk.Label)widget).set_markup("<b>%s</b>".printf(name));
		((Gtk.Label)widget).set_xalign(0.0f);
		vbox.pack_start(widget);

		widget = new Gtk.Label(null);
		widget.set_margin_start(12);
		widget.set_margin_end(12);
		widget.set_margin_bottom(8);
		((Gtk.Label)widget).set_markup("<small>%s</small>".printf(source_dir));
		((Gtk.Label)widget).set_xalign(0.0f);
		vbox.pack_start(widget);

		Gtk.Box hbox = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
		hbox.pack_start(vbox);

		Gtk.Button remove_button = new Gtk.Button.from_icon_name("list-remove-symbolic");
		remove_button.get_style_context().add_class("flat");
		remove_button.get_style_context().add_class("destructive-action");
		remove_button.set_halign(Gtk.Align.CENTER);
		remove_button.set_valign(Gtk.Align.CENTER);
		remove_button.set_margin_end(12);
		remove_button.clicked.connect(() => {
			Gtk.MessageDialog md = new Gtk.MessageDialog((Gtk.Window)this.get_toplevel()
				, Gtk.DialogFlags.MODAL
				, Gtk.MessageType.WARNING
				, Gtk.ButtonsType.NONE
				, "Remove \"%s\" from the list?\n\nThis action removes the project from the list only, files on disk will not be deleted.".printf(source_dir)
				);
			md.add_button("_Cancel", ResponseType.CANCEL);
			md.add_button("_Remove", ResponseType.YES);
			md.set_default_response(ResponseType.CANCEL);
			int rt = md.run();
			md.destroy();

			if (rt == ResponseType.CANCEL)
				return;

			_user.remove_recent_project(row.get_data("source_dir"));
			_list_projects.remove(row);
		});
		hbox.pack_end(remove_button, false, false, 0);

		Gtk.Button button_open = new Gtk.Button.with_label("Open");
		button_open.get_style_context().add_class("flat");
		button_open.set_halign(Gtk.Align.CENTER);
		button_open.set_valign(Gtk.Align.CENTER);
		// button_open.set_margin_end(12);
		button_open.clicked.connect(() => {
			string mtime = new GLib.DateTime.now_utc().to_unix().to_string();
			row.set_data("mtime", mtime);
			_list_projects.invalidate_sort();
			_user.touch_recent_project(row.get_data("source_dir"), mtime);
			_application.show_panel("main_vbox", Gtk.StackTransitionType.NONE);
			_application.restart_backend.begin(source_dir, LEVEL_NONE);
		});
		hbox.pack_end(button_open, false, false, 0);

		row.add(hbox);
		_list_projects.add(row);
		_list_projects.show_all(); // Otherwise the list is not always updated...

		if (!GLib.FileUtils.test(source_dir, FileTest.EXISTS))
			button_open.sensitive = false;
	}
}

}
