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
		ColorButtonVector3 _grid_color_button;

		[GtkChild]
		ColorButtonVector3 _grid_disabled_color_button;

		[GtkChild]
		ColorButtonVector3 _axis_x_color_button;

		[GtkChild]
		ColorButtonVector3 _axis_y_color_button;

		[GtkChild]
		ColorButtonVector3 _axis_z_color_button;

		[GtkChild]
		ColorButtonVector3 _axis_selected_color_button;

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

		[GtkCallback]
		private void on_color_set()
		{
			_application._editor.send_script(LevelEditorApi.set_color("grid", _grid_color_button.value));
			_application._editor.send_script(LevelEditorApi.set_color("grid_disabled", _grid_disabled_color_button.value));
			_application._editor.send_script(LevelEditorApi.set_color("axis_x", _axis_x_color_button.value));
			_application._editor.send_script(LevelEditorApi.set_color("axis_y", _axis_y_color_button.value));
			_application._editor.send_script(LevelEditorApi.set_color("axis_z", _axis_z_color_button.value));
			_application._editor.send_script(LevelEditorApi.set_color("axis_selected", _axis_selected_color_button.value));
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
