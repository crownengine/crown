/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;

namespace Crown
{
	[GtkTemplate (ui = "/org/crown/level_editor/ui/preferences_dialog.ui")]
	public class PreferencesDialog : Gtk.Dialog
	{
		// Data
		LevelEditorApplication _application;

		// Widgets
		[GtkChild]
		Gtk.ColorButton _grid_color_button;

		[GtkChild]
		Gtk.ColorButton _grid_disabled_color_button;

		[GtkChild]
		Gtk.ColorButton _axis_x_color_button;

		[GtkChild]
		Gtk.ColorButton _axis_y_color_button;

		[GtkChild]
		Gtk.ColorButton _axis_z_color_button;

		[GtkChild]
		Gtk.ColorButton _axis_selected_color_button;

		[GtkChild]
		Gtk.SpinButton _gizmo_size_spin_button;

		[GtkChild]
		Gtk.SpinButton _level_autosave_spin_button;

		public PreferencesDialog(LevelEditorApplication app)
		{
			// Data
			_application = app;

			this.title = "Preferences";
		}

		private static Vector3 rgba_to_vector3(Gdk.RGBA rgba)
		{
			double r = rgba.red;
			double g = rgba.green;
			double b = rgba.blue;
			return Vector3(r, g, b);
		}

		[GtkCallback]
		private void on_color_set()
		{
			_application._editor.send_script(LevelEditorApi.set_color("grid", rgba_to_vector3(_grid_color_button.get_rgba())));
			_application._editor.send_script(LevelEditorApi.set_color("grid_disabled", rgba_to_vector3(_grid_disabled_color_button.get_rgba())));
			_application._editor.send_script(LevelEditorApi.set_color("axis_x", rgba_to_vector3(_axis_x_color_button.get_rgba())));
			_application._editor.send_script(LevelEditorApi.set_color("axis_y", rgba_to_vector3(_axis_y_color_button.get_rgba())));
			_application._editor.send_script(LevelEditorApi.set_color("axis_z", rgba_to_vector3(_axis_z_color_button.get_rgba())));
			_application._editor.send_script(LevelEditorApi.set_color("axis_selected", rgba_to_vector3(_axis_selected_color_button.get_rgba())));
		}

		[GtkCallback]
		private void on_gizmo_size_value_changed()
		{
			_application._editor.send_script("Gizmo.size = %f".printf(_gizmo_size_spin_button.value));
		}

		[GtkCallback]
		private void on_level_autosave_value_changed()
		{
			_application.set_autosave_timer((uint)_level_autosave_spin_button.value);
		}
	}
}
