/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
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
	unowned Gtk.Button _button_back;

	[GtkChild]
	unowned Gtk.Button _button_create;

	[GtkChild]
	unowned EntryText _entry_name;

	[GtkChild]
	unowned Gtk.FileChooserButton _file_chooser_button_location;

	[GtkChild]
	unowned ComboBoxMap _combo_box_map_template;

	[GtkChild]
	unowned Gtk.Label _label_message;

	public PanelNewProject(LevelEditorApplication app, User user, Project project)
	{
		// Data
		_application = app;
		_user = user;
		_project = project;

		_file_chooser_button_location.set_current_folder(_documents_dir.get_path());

		_combo_box_map_template.append("", "None");
		_combo_box_map_template.value = "";

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

			if (_combo_box_map_template.value == "")
				_project.create_initial_files(source_dir);
			else
				copy_template_to_source_dir(source_dir, _combo_box_map_template.value);

			_application.restart_backend.begin(source_dir, LEVEL_NONE);
		});
	}

	public void fill_templates_list(string path)
	{
		GLib.File file = GLib.File.new_for_path(path);
		try
		{
			FileEnumerator enumerator = file.enumerate_children("standard::*"
				, FileQueryInfoFlags.NOFOLLOW_SYMLINKS
				);
			for (GLib.FileInfo? info = enumerator.next_file(); info != null ; info = enumerator.next_file())
			{
				GLib.File source_dir = GLib.File.new_for_path(GLib.Path.build_filename(path, info.get_name()));
				_combo_box_map_template.append(source_dir.get_path(), info.get_name());
			}
		}
		catch (GLib.Error e)
		{
			loge(e.message);
		}
	}

	public void copy_recursive(GLib.File dst, GLib.File src, GLib.FileCopyFlags flags = GLib.FileCopyFlags.NONE)
	{
		try
		{
			GLib.FileType src_type = src.query_file_type(GLib.FileQueryInfoFlags.NONE);
			if (src_type == GLib.FileType.DIRECTORY)
			{
				if (dst.query_exists() == false)
					dst.make_directory();
				src.copy_attributes(dst, flags);

				string dst_path = dst.get_path();
				string src_path = src.get_path();
				GLib.FileEnumerator enum = src.enumerate_children(GLib.FileAttribute.STANDARD_NAME, GLib.FileQueryInfoFlags.NONE);
				for (GLib.FileInfo? info = enum.next_file(); info != null; info = enum.next_file())
				{
					copy_recursive(GLib.File.new_for_path(GLib.Path.build_filename(dst_path, info.get_name()))
						, GLib.File.new_for_path(GLib.Path.build_filename(src_path, info.get_name()))
						, flags
						);
				}
			}
			else if (src_type == GLib.FileType.REGULAR)
			{
				src.copy(dst, flags);
			}
		}
		catch (Error e)
		{
			loge(e.message);
		}
	}

	public void copy_template_to_source_dir(string source_dir, string template_dir)
	{
		GLib.File dst = GLib.File.new_for_path(source_dir);
		GLib.File src = GLib.File.new_for_path(template_dir);
		copy_recursive(dst, src);
	}
}

}
