/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;
using Gee;

namespace Crown
{
	[GtkTemplate (ui = "/org/crown/level_editor/ui/panel_new_project.ui")]
	public class PanelNewProject : Gtk.Viewport
	{
		// Data
		LevelEditorApplication _application;
		User _user;
		Project _project;

		// Widgets
		[GtkChild]
		Gtk.Button _button_back;

		[GtkChild]
		Gtk.Button _button_create;

		[GtkChild]
		Gtk.Entry _entry_name;

		[GtkChild]
		Gtk.FileChooserButton _file_chooser_button_location;

		[GtkChild]
		Gtk.Label _label_message;

		public PanelNewProject(LevelEditorApplication app, User user, Project project)
		{
			// Data
			_application = app;
			_user = user;
			_project = project;

			_file_chooser_button_location.set_current_folder(_documents_dir.get_path());

			_button_back.clicked.connect(() => {
				_application.show_panel("panel_welcome", StackTransitionType.SLIDE_UP);
			});

			_button_create.clicked.connect(() => {
				if (_entry_name.text == "")
				{
					_label_message.label = "Choose project name";
					return;
				}

				GLib.File location = _file_chooser_button_location.get_file();
				string? source_dir = location.get_path();

				if (source_dir == null)
				{
					_label_message.label = "Location is not valid";
					return;
				}
				if (GLib.FileUtils.test(source_dir, FileTest.IS_REGULAR))
				{
					_label_message.label = "Location must be an empty directory";
					return;
				}

				if (!is_directory_empty(source_dir))
				{
					_label_message.label = "Location must be an empty directory";
					return;
				}

				_label_message.label = "";

				_user.add_recent_project(source_dir, _entry_name.text);
				_application.show_panel("main_vbox");
				_project.create_initial_files(source_dir);
				_application.restart_compiler(source_dir, null);
			});
		}
	}
}
