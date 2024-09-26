/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;
using Gee;

namespace Crown
{
public class PanelNewProject : Gtk.Viewport
{
	// Data
	User _user;
	Project _project;

	// Widgets
	public Gtk.Label _new_project_label;
	public Gtk.Label _name_label;
	public EntryText _entry_name;
	public Gtk.Label _location_label;
	public Gtk.FileChooserButton _file_chooser_button_location;
	public Gtk.Label _template_label;
	public ComboBoxMap _combo_box_map_template;
	public Gtk.Label _label_message;
	public Gtk.Button _button_back;
	public Gtk.Button _button_create;
	public Gtk.Box _buttons_box;
	public Gtk.Box _box;
	public Gtk.Grid _grid;
	public Clamp _clamp;

	public PanelNewProject(User user, Project project)
	{
		this.shadow_type = Gtk.ShadowType.NONE;

		// Data
		_user = user;
		_project = project;

		_new_project_label = new Gtk.Label("New Project");
		_new_project_label.xalign = 0;
		_new_project_label.set_attributes(Pango.AttrList.from_string("0 -1 weight bold, 0 -1 scale 1.5"));

		_name_label = new Gtk.Label("Name");
		_name_label.xalign = 1;
		_entry_name = new EntryText();

		_location_label = new Gtk.Label("Location");
		_location_label.xalign = 1;
		_file_chooser_button_location = new Gtk.FileChooserButton("Select Folder", Gtk.FileChooserAction.SELECT_FOLDER);
		_file_chooser_button_location.set_current_folder(_documents_dir.get_path());

		_template_label = new Gtk.Label("Template");
		_template_label.xalign = 1;
		_combo_box_map_template = new ComboBoxMap();
		_combo_box_map_template.hexpand = true;
		_combo_box_map_template.append("", "None");
		_combo_box_map_template.value = "";

		_label_message = new Gtk.Label("");
		_label_message.xalign = 1;

		_button_back = new Gtk.Button.with_label("Back");
		_button_back.clicked.connect(() => {
				var app = (LevelEditorApplication)GLib.Application.get_default();
				app.show_panel("panel_welcome", StackTransitionType.SLIDE_UP);
			});

		_button_create = new Gtk.Button.with_label("Create");
		_button_create.get_style_context().add_class(Gtk.STYLE_CLASS_SUGGESTED_ACTION);
		_button_create.clicked.connect(() => {
				if (_entry_name.text == "") {
					_label_message.label = "Choose project name";
					return;
				}

				GLib.File location = _file_chooser_button_location.get_file();
				string? source_dir = location.get_path();

				if (source_dir == null) {
					_label_message.label = "Location is not valid";
					return;
				}
				if (GLib.FileUtils.test(source_dir, FileTest.IS_REGULAR)) {
					_label_message.label = "Location must be an empty directory";
					return;
				}

				if (!is_directory_empty(source_dir)) {
					_label_message.label = "Location must be an empty directory";
					return;
				}

				_label_message.label = "";

				_user.add_or_touch_recent_project(source_dir, _entry_name.text);
				var app = (LevelEditorApplication)GLib.Application.get_default();
				app.show_panel("main_vbox");

				if (_combo_box_map_template.value == "")
					_project.create_initial_files(source_dir);
				else
					copy_template_to_source_dir(source_dir, _combo_box_map_template.value);

				app.restart_backend.begin(source_dir, LEVEL_NONE);
			});

		_buttons_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_buttons_box.spacing = 6;
		_buttons_box.pack_end(_button_create, false, true);
		_buttons_box.pack_end(_button_back, false, true);

		_grid = new Gtk.Grid();
		_grid.hexpand = true;
		_grid.row_spacing = 6;
		_grid.column_spacing = 12;
		_grid.attach(_name_label, 0, 0);
		_grid.attach(_entry_name, 1, 0);
		_grid.attach(_location_label, 0, 1);
		_grid.attach(_file_chooser_button_location, 1, 1);
		_grid.attach(_template_label, 0, 2);
		_grid.attach(_combo_box_map_template, 1, 2);
		_grid.attach(_buttons_box, 1, 3);

		_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_box.margin_start = 12;
		_box.margin_end = 12;
		_box.margin_top = 32;
		_box.margin_bottom = 32;
		_box.spacing = 12;
		_box.pack_start(_new_project_label, false, true);
		_box.pack_start(_grid, false, true);
		_box.pack_start(_label_message, false, true);

		_clamp = new Clamp();
		_clamp.add(_box);

		this.add(_clamp);
	}

	public void fill_templates_list(string path)
	{
		GLib.File file = GLib.File.new_for_path(path);
		try {
			FileEnumerator enumerator = file.enumerate_children("standard::*"
				, FileQueryInfoFlags.NOFOLLOW_SYMLINKS
				);
			for (GLib.FileInfo? info = enumerator.next_file(); info != null ; info = enumerator.next_file()) {
				GLib.File source_dir = GLib.File.new_for_path(GLib.Path.build_filename(path, info.get_name()));
				_combo_box_map_template.append(source_dir.get_path(), info.get_name());
			}
		} catch (GLib.Error e) {
			loge(e.message);
		}
	}

	public void copy_recursive(GLib.File dst, GLib.File src, GLib.FileCopyFlags flags = GLib.FileCopyFlags.NONE)
	{
		try {
			GLib.FileType src_type = src.query_file_type(GLib.FileQueryInfoFlags.NONE);
			if (src_type == GLib.FileType.DIRECTORY) {
				if (dst.query_exists() == false)
					dst.make_directory();

				string dst_path = dst.get_path();
				string src_path = src.get_path();
				GLib.FileEnumerator enum = src.enumerate_children(GLib.FileAttribute.STANDARD_NAME, GLib.FileQueryInfoFlags.NONE);
				for (GLib.FileInfo? info = enum.next_file(); info != null; info = enum.next_file()) {
					copy_recursive(GLib.File.new_for_path(GLib.Path.build_filename(dst_path, info.get_name()))
						, GLib.File.new_for_path(GLib.Path.build_filename(src_path, info.get_name()))
						, flags
						);
				}
			} else if (src_type == GLib.FileType.REGULAR) {
				src.copy(dst, flags);
			}
		} catch (Error e) {
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

} /* namespace Crown */
